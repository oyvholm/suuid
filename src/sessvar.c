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
 * is_legal_desc_char() - Return true if the character c is a valid char for 
 * use in the desc attribute in <sess> elements, false if not.
 */

bool is_legal_desc_char(const unsigned char c)
{
	return strchr(DESC_LEGAL, c) ? true : false;
}

#ifdef UNUSED
/*
 * is_valid_desc_string() - Return true if the string s is a valid desc name, 
 * return false if not.
 */

bool is_valid_desc_string(const char *s)
{
	const char *p = s;

	assert(s);

	while (*p) {
		if (!is_legal_desc_char(*p))
			return false;
		p++;
	}

	if (utf8_check(s))
		return false;

	return true;
}
#endif

#ifdef UNUSED
/*
 * get_desc_from_command() - Return pointer to allocated desc string extracted 
 * from the command, used for the desc attribute in the sess string. If cmd is 
 * NULL or empty, or if something fails or cmd is NULL or empty, return NULL.
 */

char *get_desc_from_command(const char *cmd)
{
	char *ap, *p, *p2;

	if (!cmd || !strlen(cmd))
		return NULL;
	ap = mystrdup(cmd);
	if (!ap)
		return NULL;
	p = ap;
	while (strchr("./", *p))
		p++;
	p2 = p;
	while (*p2 && !isspace((unsigned char)*p2))
		p2++;
	if (p2 > p)
		*p2 = '\0';
	if (p > ap)
		memmove(ap, p, strlen(p) + 1);

	return ap;
}
#endif

/*
 * fill_sess() - Fill the first available dest->sess element with uuid and desc 
 * and increase the local counter. Return 0 if everything is ok, 1 if something 
 * failed.
 */

int fill_sess(struct Entry *dest, const char *uuid,
              const char *desc, const size_t desclen)
{
	char *auuid = NULL, *adesc = NULL;
	static unsigned int sessind = 0;

	assert(dest);
	assert(valid_uuid(uuid, false));

	if (sessind >= MAX_SESS) {
		fprintf(stderr, "%s: Maximum number of sess entries (%u)"
		                " exceeded\n", progname, MAX_SESS);
		return 1;
	}

	auuid = strndup(uuid, UUID_LENGTH);
	if (!auuid) {
		myerror("%s(): Memory allcation error," /* gncov */
		        " could not duplicate UUID", __func__);
		return 1; /* gncov */
	}

	if (desc && desclen) {
		adesc = strndup(desc, desclen);
		if (!adesc) {
			myerror("%s(): Memory allocation error," /* gncov */
			        " could not duplicate desc", __func__);
			free(auuid); /* gncov */
			return 1; /* gncov */
		}
		dest->sess[sessind].desc = adesc;
	}
	dest->sess[sessind].uuid = auuid;
	sessind++;

	return 0;
}

/*
 * get_sess_info() - Read sess information from the environment variable and 
 * insert it into the entry.sess array. Returns 0 if ok or 1 if any error.
 */

int get_sess_info(struct Entry *entry)
{
	char *s, *p, *desc_found = NULL, *desc_end = NULL;

	assert(entry);

	if (!getenv(ENV_SESS))
		return 0;

	s = mystrdup(getenv(ENV_SESS));
	if (!s)
		return 1; /* gncov */

	if (!scan_for_uuid(s)) {
		/*
		 * The environment variable exists, but contains no valid 
		 * UUIDs. Not much to do about that, so just return gracefully.
		 */
		free(s);
		return 0;
	}

	p = s;
	while (*p) {
		if (valid_uuid(p, false)) {
			size_t desclen = 0;

			if (desc_found && !desc_end) {
				/*
				 * There was no slash between desc and uuid, so 
				 * desc_end hasn't been set.
				 */
				desc_end = p;
			}

			if (desc_end > desc_found)
				desclen = desc_end - desc_found;

			if (fill_sess(entry, p, desc_found, desclen)) {
				free(s);
				return 1;
			}

			p += UUID_LENGTH - 1;
			desc_found = desc_end = NULL;
		} else if (is_legal_desc_char(*p)) {
			if (!desc_found && p >= s)
				desc_found = p;
		} else if (*p == '/') {
			if (desc_found)
				desc_end = p;
		} else {
			desc_found = desc_end = NULL;
		}
		p++;
	}
	free(s);

	return 0;
}

/*
 * free_sess() - Deallocate all sess entries in the entry->sess[].{desc,uuid} 
 * arrays.
 */

void free_sess(struct Entry *entry)
{
	unsigned int i;

	assert(entry);

	for (i = 0; entry->sess[i].uuid && i < MAX_SESS; i++) {
		free(entry->sess[i].uuid);
		free(entry->sess[i].desc);
	}
}

#ifdef UNUSED
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

	assert(argv);

	for (t = optind; t < argc; t++) {
		msg(3, "Non-option arg: %s", argv[t]);
		cmdsize += strlen(argv[t]) + 1; /* Add one for space */
	}
	cmdsize += 1; /* Terminating '\0' */
	cmd = mymalloc(cmdsize);
	if (!cmd)
		return NULL;
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
#endif

#ifdef UNUSED
/*
 * clean_up_sessvar() - Modifies dest by removing trailing and repeated commas. 
 * Returns dest.
 */

char *clean_up_sessvar(char *dest)
{
	unsigned int i;

	assert(dest);

	i = strlen(dest);
	while (i && dest[i - 1] == ',') {
		dest[i - 1] = '\0';
		i--;
	}
	squeeze_chars(dest, ",");

	return dest;
}
#endif

#ifdef UNUSED
/*
 * add_to_sessvar() - Modify the session environment variable (defined in 
 * ENV_SESS) by adding ",desc/uuid," to the end of it. If desc is NULL or 
 * empty, only ",uuid," is added. Return 0 on success, or 1 if anything fails 
 * or uuid isn't a valid UUID.
 */

const char *add_to_sessvar(const char *desc, const char *uuid)
{
	size_t envlen; /* Length of the new string */
	char *sessvar; /* Copy of the original envvar */
	char *envbuf; /* Temporary buffer for the finished string */

	assert(valid_uuid(uuid, true));

	if (!is_valid_desc_string(desc))
		desc = NULL;

	if (getenv(ENV_SESS)) {
		char *ap;

		ap = mystrdup(getenv(ENV_SESS));
		clean_up_sessvar(ap);
		sessvar = mystrdup(ap);
		free(ap);
	} else {
		sessvar = mystrdup("");
	}
	if (!sessvar)
		return NULL;

	envlen = strlen(ENV_SESS) + 1 + strlen(sessvar) + 1
	         + strlen(desc) + 1 + UUID_LENGTH + 1 + 1;
	envbuf = mymalloc(envlen);
	if (!envbuf) {
		free(sessvar);
		return NULL;
	}

	snprintf(envbuf, envlen,
	         "%s=%s,%s%s%s,",
	         ENV_SESS,
	         sessvar ? sessvar : "",
	         desc ? desc : "",
	         desc ? "/" : "",
	         uuid);

	if (putenv(envbuf)) {
		myerror("Could not set %s environment variable", ENV_SESS);
		free(sessvar);
		return NULL;
	}

	free(sessvar);

	return getenv(ENV_SESS);
}
#endif

#ifdef UNUSED
/*
 * run_session() - Execute a shell command and log it with start time, end time 
 * and return value. If any error occurs, return -1. Otherwise, return with the 
 * value from system(), which by a nice coincidence also returns -1 on error or 
 * the return value from the program.
 */

int run_session(const struct Options *orig_opt,
                const int argc, char * const argv[])
{
	int retval = 0;
	struct Options opt = *orig_opt;
	char *cmd = NULL;
	char *start_uuid = NULL;
	char *cmd_desc = NULL;
	struct uuid_result result;

	assert(orig_opt);
	assert(argv);

	cmd = concat_cmd_string(argc, argv);
	if (!cmd)
		return -1;
	cmd_desc = get_desc_from_command(cmd);
	msg(2, "cmd_desc = \"%s\"", cmd_desc);

	opt.count = 1;
	opt.whereto = "e";
	result = create_and_log_uuids(&opt);
	if (!result.success) {
		myerror("Error generating UUID, session not started");
		retval = -1;
		goto cleanup;
	}
	start_uuid = mystrdup(result.lastuuid);
	if (!start_uuid) {
		retval = -1;
		goto cleanup;
	}
	assert(valid_uuid(start_uuid, true));
	msg(3, "old %s: \"%s\"", ENV_SESS, getenv(ENV_SESS));
	add_to_sessvar(cmd_desc, start_uuid);
	msg(3, "new %s: \"%s\"", ENV_SESS, getenv(ENV_SESS));

	msg(1, "Executing \"%s\"", cmd);
	/*
	 * FIXME: This value is shifted with 8 bits in main(). Check if it's 
	 * ok.
	 */
	retval = system(cmd);
	msg(2, "%s(): retval from system() = %d (0x%x)",
	       __func__, retval, retval);

cleanup:
	free(start_uuid);
	free(cmd_desc);
	free(cmd);

	return retval;
}
#endif

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
