/*
 * genuuid.c
 * File ID: 34498cac-4661-11e6-9093-a75376a00eeb
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

bool should_terminate = FALSE;

/*
 * init_randomness() - Initialise the random number generator. Returns EXIT_OK 
 * or EXIT_ERROR.
 */

bool init_randomness(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1) {
		myerror("Could not initialiase randomness generator, "
		        "gettimeofday() failed");
		return EXIT_ERROR;
	}

	srandom((unsigned int)tv.tv_sec ^ (unsigned int)tv.tv_usec ^
	        (unsigned int)getpid());

	return EXIT_OK;
}

/*
 * init_opt() - Initialise dest with default start values.
 */

void init_opt(struct Options *dest)
{
	unsigned int i;

	assert(dest);

	dest->comment = NULL;
	dest->count = 1;
	dest->help = FALSE;
	dest->license = FALSE;
	dest->logdir = NULL;
	dest->random_mac = FALSE;
	dest->raw = FALSE;
	dest->rcfile = NULL;
	dest->self_test = FALSE;
	dest->uuid = NULL;
	dest->verbose = 0;
	dest->version = FALSE;
	dest->whereto = NULL;
	for (i = 0; i < MAX_TAGS; i++)
		dest->tag[i] = NULL;
}

/*
 * process_comment_option() - Receive the argument used with -c/--comment and 
 * decide what to do with it. Return pointer to allocated string with the 
 * comment, or NULL if anything failed.
 */

char *process_comment_option(const char *cmt)
{
	char *retval;

	assert(cmt);
	if (!strcmp(cmt, "-")) {
		retval = read_from_fp(stdin);
		if (!retval) {
			myerror("Could not read data from stdin");
			return NULL;
		}
	} else if (!strcmp(cmt, "--")) {
		char *e;

		e = get_editor();
		if (!e)
			return NULL;
		retval = read_from_editor(e);
		if (!retval) {
			free(e);
			return NULL;
		}
		free(e);
	} else {
		retval = strdup(cmt);
		if (!retval) {
			myerror("%s: Cannot allocate memory for comment, "
			        "strdup() failed");
			return NULL;
		}
	}
	if (!valid_xml_chars(retval)) {
		fprintf(stderr, "%s: Comment contains illegal characters or "
		                "is not valid UTF-8\n", progname);
		free(retval);
		return NULL;
	}

	/* fixme: This is how it's done in the Perl version. I'm not sure if 
	 * it's an ok thing to do, even though it looks nice in the log files 
	 * and has worked great for years. Maybe this behaviour should be 
	 * changed when the C version passes all tests in suuid.t .
	 */
	trim_str_front(retval);
	trim_str_end(retval);

	return retval;
}

/*
 * fill_entry_struct() - Fill the entry struct with information from the opt 
 * struct and the environment, like current directory, hostname, comment, etc.
 * Returns EXIT_OK if no errors, EXIT_ERROR if errors.
 */

int fill_entry_struct(struct Entry *entry, const struct Rc *rc,
                      const struct Options *opt)
{
	unsigned int i;

	entry->host = get_hostname(rc);
	if (!entry->host) {
		myerror("fill_entry_struct(): Cannot get hostname");
		return EXIT_ERROR;
	}
	if (!valid_hostname(entry->host)) {
		myerror("fill_entry_struct(): Got invalid hostname: \"%s\"",
		        entry->host);
		return EXIT_ERROR;
	}
	entry->cwd = getpath();
	entry->user = get_username();
	entry->tty = get_tty();

	for (i = 0; i < MAX_TAGS && opt->tag[i]; i++)
		if (!store_tag(entry, opt->tag[i]))
			return EXIT_ERROR;

	if (opt->comment) {
		entry->txt = process_comment_option(opt->comment);
		if (!entry->txt)
			return EXIT_ERROR;
	}

	if (get_sess_info(entry) == EXIT_ERROR) {
		free(entry->txt);
		return EXIT_ERROR;
	}

	return EXIT_OK;
}

/*
 * process_uuid() - Generate UUID and write it to the log file.
 * If no errors, send it to stdout and/or stderr and return a pointer to the 
 * UUID. Otherwise return NULL.
 */

char *process_uuid(FILE *logfp, const struct Rc *rc, const struct Options *opt,
                   struct Entry *entry)
{
	if (opt->uuid) {
		assert(valid_uuid(opt->uuid, TRUE));
		if (!valid_uuid(opt->uuid, TRUE)) {
			fprintf(stderr, "process_uuid(): UUID \"%s\" is not "
			                "valid.", opt->uuid);
			return NULL;
		}
		entry->uuid = opt->uuid;
	} else
		entry->uuid = generate_uuid(rc, opt->random_mac);
	if (!entry->uuid) {
		fprintf(stderr, "%s: UUID generation failed\n", progname);
		return NULL;
	}
	entry->date = malloc(DATE_LENGTH + 1);
	if (!entry->date) {
		myerror("process_uuid(): Could not allocate %lu bytes for "
		        "date string", DATE_LENGTH + 1);
		return NULL;
	}
	if (!uuid_date_from_uuid(entry->date, entry->uuid))
		return NULL;

	if (add_to_logfile(logfp, entry, opt->raw) == EXIT_ERROR)
		return NULL;

	if (!opt->whereto)
		puts(entry->uuid);
	else {
		if (strchr(opt->whereto, 'a') || strchr(opt->whereto, 'o'))
			fprintf(stdout, "%s\n", entry->uuid);
		if (strchr(opt->whereto, 'a') || strchr(opt->whereto, 'e'))
			fprintf(stderr, "%s\n", entry->uuid);
	}

	return entry->uuid;
}

/*
 * sighandler() - Called when it receives a termination signal. Set the 
 * variable should_terminate to indicate that the fun is over, but don't 
 * terminate until the log file has been closed properly.
 */

void sighandler(const int sig)
{
	if (sig == SIGHUP || sig == SIGINT || sig == SIGQUIT ||
	    sig == SIGPIPE || sig == SIGTERM) {
		fprintf(stderr, "%s: Termination signal (%s) received, "
		                "aborting\n", progname, strsignal(sig));
		should_terminate = TRUE;
	}
}

/*
 * create_and_log_uuids() - Do everything in one place; Initialise the random 
 * number generator, read values from the rc file, environment and command 
 * line, generate the UUID(s) and write it to the log file. Returns a struct 
 * uuid_result with the number of UUIDs generated and a value indicating 
 * success or not. If opt->uuid contains an UUID, set count to 1 to avoid 
 * duplicates in the log file.
 */

struct uuid_result create_and_log_uuids(const struct Options *opt)
{
	struct uuid_result retval;
	char *rcfile = NULL, *logfile = NULL;
	FILE *logfp;
	unsigned int i, count = opt->count;
	struct Rc rc;
	struct Entry entry;

	retval.count = 0;
	memset(retval.lastuuid, 0, UUID_LENGTH + 1);
	retval.success = TRUE;
	init_xml_entry(&entry);

	if (init_randomness() == EXIT_ERROR) {
		retval.success = FALSE;
		goto cleanup;
	}

	rcfile = get_rcfilename(opt);
	if (read_rcfile(rcfile, &rc) == EXIT_ERROR) {
		retval.success = FALSE;
		goto cleanup;
	}

	if (fill_entry_struct(&entry, &rc, opt) == EXIT_ERROR) {
		retval.success = FALSE;
		goto cleanup;
	}

	logfile = get_log_prefix(&rc, opt, ".xml");
	if (!logfile) {
		retval.success = FALSE;
		goto cleanup;
	}

	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGPIPE, sighandler);
	signal(SIGTERM, sighandler);

	logfp = open_logfile(logfile);
	if (!logfp) {
		retval.success = FALSE;
		goto cleanup;
	}

	if (opt->uuid)
		count = 1;
	for (i = 0; i < count; i++) {
		if (!process_uuid(logfp, &rc, opt, &entry)) {
			close_logfile(logfp);
			retval.success = FALSE;
			goto cleanup;
		}
		retval.count++;
		if (should_terminate)
			break;
	}
	if (valid_uuid(entry.uuid, TRUE))
		strncpy(retval.lastuuid, entry.uuid, UUID_LENGTH + 1);

	if (retval.count < opt->count)
		fprintf(stderr, "%s: Generated only %u of %u UUIDs\n",
		                progname, retval.count, opt->count);

	if (close_logfile(logfp) == EXIT_ERROR)
		retval.success = FALSE;

cleanup:
	free(logfile);
	free(rc.uuidcmd);
	free(rc.hostname);
	free(entry.txt);
	free(entry.date);
	free(entry.cwd);
	free(rcfile);

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
