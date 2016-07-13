/*
 * sessvar.c
 * File ID: a3d401d8-3f18-11e6-bafd-02010e0a6634
 *
 * (C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>
 *
 * This program is free software; you can redistribute it and/or modify it 
 * under the terms of the GNU General Public License as published by the Free 
 * Software Foundation; either version 2 of the License, or (at your option) 
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or 
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for 
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with 
 * this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "suuid.h"

/*
 * is_legal_desc_char() - Return TRUE if the character c is a valid char for 
 * use in the desc attribute in <sess> elements, FALSE if not.
 */

bool is_legal_desc_char(const unsigned char c)
{
	return strchr(DESC_LEGAL, c) ? TRUE : FALSE;
}

/*
 * is_valid_desc_string() - Return TRUE if the string s is a valid desc name, 
 * return FALSE if not.
 */

bool is_valid_desc_string(const char *s)
{
	const char *p = s;

	while (*p) {
		if (!is_legal_desc_char(*p))
			return FALSE;
		p++;
	}

	if (utf8_check(s))
		return FALSE;

	return TRUE;
}

/*
 * get_desc_from_command() - Return pointer to allocated desc string extracted 
 * from the command, used for the desc attribute in the sess string. If cmd is 
 * NULL or empty, or if something fails, return NULL.
 */

char *get_desc_from_command(const char *cmd)
{
	char *ap, *p, *p2;

	if (!cmd || !strlen(cmd))
		return NULL;
	ap = strdup(cmd);
	if (!ap) {
		myerror("get_desc_from_command(): Could not duplicate command "
		        "string");
		return NULL;
	}
	p = ap;
	while (strchr("./", *p))
		p++;
	p2 = p;
	while (*p2 && !isspace(*p2))
		p2++;
	if (p2 > p)
		*p2 = '\0';
	if (p > ap)
		memmove(ap, p, strlen(p) + 1);

	return ap;
}

/*
 * fill_sess() - Fill the first available dest->sess element with uuid and desc 
 * and increase the local counter. Return EXIT_OK if everything is ok, 
 * EXIT_ERROR if something failed.
 */

int fill_sess(struct Entry *dest, const char *uuid,
              const char *desc, const size_t desclen)
{
	char *auuid = NULL, *adesc = NULL;
	static unsigned int sessind = 0;

	assert(dest);
	assert(valid_uuid(uuid, FALSE));

	auuid = strndup(uuid, UUID_LENGTH);
	if (!auuid) {
		myerror("fill_sess(): Could not duplicate UUID");
		return EXIT_ERROR;
	}

	if (desc && desclen) {
		adesc = strndup(desc, desclen);
		if (!adesc) {
			myerror("get_sess_info(): Could not duplicate desc");
			free(auuid);
			return EXIT_ERROR;
		}
		if (!is_valid_desc_string(adesc))
			free(adesc);
		else
			dest->sess[sessind].desc = adesc;
	}
	dest->sess[sessind].uuid = auuid;
	sessind++;

	return EXIT_OK;
}

/*
 * get_sess_info() - Read sess information from the environment variable and 
 * insert it into the entry.sess array. Returns EXIT_OK or EXIT_ERROR.
 */

int get_sess_info(struct Entry *entry)
{
	char *s, *p, *desc_found = NULL, *desc_end = NULL;

	assert(entry);

	if (!getenv(ENV_SESS))
		return EXIT_OK;

	s = strdup(getenv(ENV_SESS));
	if (!s) {
		myerror("get_sess_info(): Could not duplicate %s environment "
		        "variable", ENV_SESS);
		return EXIT_ERROR;
	}

	if (!scan_for_uuid(s)) {
		free(s);
		return EXIT_OK;
	}

	p = s;
	while (*p) {
		if (valid_uuid(p, FALSE)) {
			size_t desclen = 0;

			if (desc_found && !desc_end)
				desc_end = p; /* There was no slash between 
				               * desc and uuid, so desc_end 
				               * hasn't been set.
				               */

			if (desc_end > desc_found)
				desclen = desc_end - desc_found;

			if (fill_sess(entry, p,
				      desc_found, desclen) == EXIT_ERROR) {
				myerror("get_sess_info(): fill_sess() failed");
				free(s);
				return EXIT_ERROR;
			}

			p += UUID_LENGTH - 1;
			desc_found = desc_end = NULL;
		} else if (is_legal_desc_char(*p)) {
			if (!desc_found && p >= s)
				desc_found = p;
		} else if (*p == '/') {
			if (desc_found)
				desc_end = p;
		} else
			desc_found = desc_end = NULL;
		p++;
	}
	free(s);

	return EXIT_OK;
}

/*
 * concat_cmd_string() - Concatenate the command line arguments received in 
 * argc and argv with a single space character between them. Return pointer to 
 * allocated string containing the command, or NULL if anything fails.
 */

char *concat_cmd_string(const int argc, char * const argv[])
{
	int t;
	size_t cmdsize = 0;
	char *cmd = NULL;

	for (t = optind; t < argc; t++) {
		msg(3, "Non-option arg: %s", argv[t]);
		cmdsize += strlen(argv[t]) + 1; /* Add one for space */
	}
	cmdsize += 1; /* Terminating '\0' */
	cmd = malloc(cmdsize);
	if (!cmd) {
		myerror("Could not allocate %lu bytes for command string",
		        cmdsize);
		return NULL;
	}
	memset(cmd, 0, cmdsize);

	for (t = optind; t < argc; t++) {
		strcat(cmd, argv[t]);
		strcat(cmd, " ");
	}
	if (strlen(cmd) && cmd[strlen(cmd) - 1] == ' ')
		cmd[strlen(cmd) - 1] = '\0'; /* Remove added space */
	if (!strlen(cmd)) {
		fprintf(stderr, "%s: Command is empty\n", progname);
		free(cmd);
		return NULL;
	}

	return cmd;
}

/*
 * run_session() - Execute a shell command and log it with start time, end time 
 * and return value. If any error occurs, return -1. Otherwise, return with the 
 * value from system(), which by a nice coincidence also return -1 on error or 
 * the return value from the program.
 */

int run_session(const struct Options *orig_opt,
                const int argc, char * const argv[])
{
	int retval = EXIT_OK;
	struct Options opt = *orig_opt;
	char *cmd = NULL;
	char *start_uuid = NULL;
	char *cmd_desc = NULL;
	struct uuid_result result;

	cmd = concat_cmd_string(argc, argv);
	if (!cmd)
		return -1;
	cmd_desc = get_desc_from_command(cmd);
	msg(2, "cmd_desc = \"%s\"", cmd_desc);

	opt.count = 1;
	result = create_and_log_uuids(&opt);
	if (!result.success) {
		myerror("Error generating UUID, session not started");
		retval = -1;
		goto cleanup;
	}
	start_uuid = strdup(result.lastuuid);
	if (!start_uuid) {
		myerror("Could not duplicate start UUID");
		goto cleanup;
	}
	assert(valid_uuid(start_uuid, TRUE));

	msg(1, "Executing \"%s\"", cmd);
	retval = system(cmd); /* fixme: This value is shifted with 8 bits in 
	                       * main(). Check if it's ok.
	                       */
	msg(2, "run_session(): retval from system() = %d (0x%x)",
	       retval, retval);

cleanup:
	free(start_uuid);
	free(cmd_desc);
	free(cmd);

	return(retval);
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
