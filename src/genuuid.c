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

bool should_terminate = false;

/*
 * init_randomness() - Initialise the random number generator. Returns 0 if ok, 
 * or 1 if `gettimeofday()` fails.
 */

int init_randomness(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1) {
		failed("Could not initialise randomness" /* gncov */
		       " generator, gettimeofday()");
		return 1; /* gncov */
	}

	srandom((unsigned int)tv.tv_sec ^ (unsigned int)tv.tv_usec
		^ (unsigned int)getpid());

	return 0;
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
		/*
		 * Read comment from stdin.
		 */
		retval = read_from_fp(stdin, NULL);
		if (!retval) {
			myerror("Could not read data from stdin"); /* gncov */
			return NULL; /* gncov */
		}
	} else if (!strcmp(cmt, "--")) {
		/*
		 * Open the user's favourite editor and edit the comment there 
		 * in a temporary file.
		 */
		char *e;

		e = get_editor();
		if (!e)
			return NULL;
		retval = read_from_editor(e);
		free(e);
		if (!retval)
			return NULL;
	} else {
		/*
		 * The comment was stored as a plain string in the -c/--comment 
		 * argument.
		 */
		retval = mystrdup(cmt);
		if (!retval) {
			failed("mystrdup()"); /* gncov */
			return NULL; /* gncov */
		}
	}
	if (!valid_xml_chars(retval)) {
		myerror("Comment contains illegal characters or is not valid"
		        " UTF-8");
		free(retval);
		return NULL;
	}

	/*
	 * FIXME: This is how it's done in the Perl version. I'm not sure if 
	 * it's an ok thing to do, even though it looks nice in the log files 
	 * and has worked great for years. Maybe this behaviour should be 
	 * changed when the C version passes all tests in suuid.t .
	 */
	trim_str_front(retval);
	trim_str_end(retval);

	return retval;
}

/*
 * fill_entry_struct() - Fill the `entry` struct with information from the 
 * `opts` struct and the environment, like current directory, hostname, 
 * comment, etc.
 * Returns 0 if no errors, 1 if errors.
 */

int fill_entry_struct(struct Entry *entry, const struct Rc *rc,
                      const struct Options *opts)
{
	unsigned int i;

	assert(entry);
	assert(rc);
	assert(opts);

	/*
	 * Get information about the environment; hostname, current directory, 
	 * login name and tty.
	 *
	 * FIXME: Add check so this and the session info thing are run only 
	 * once. Only has some effect if creating many UUIDs.
	 */

	entry->host = get_hostname(rc);
	if (!entry->host)
		return 1;
	entry->cwd = getpath();
	entry->user = get_username();
	entry->tty = get_tty();

	/*
	 * Store tags and comment in entry.
	 */

	for (i = 0; i < MAX_TAGS && opts->tag[i]; i++) {
		if (store_tag(entry, opts->tag[i]))
			return 1;
	}

	if (opts->comment) {
		entry->txt = process_comment_option(opts->comment);
		if (!entry->txt)
			return 1;
	}

	/*
	 * Store session information from the environment variable.
	 */

	if (get_sess_info(entry)) {
		free(entry->txt);
		return 1;
	}

	return 0;
}

/*
 * process_uuid() - Generate one UUID and write it to the log file. If no 
 * errors, send it to stdout and/or stderr and return a pointer to the UUID. 
 * Otherwise return NULL.
 */

char *process_uuid(struct Logs *logs,
                   const struct Rc *rc, const struct Options *opts,
                   struct Entry *entry)
{
	int result;

	assert(logs);
	assert(logs->logfp);
	assert(rc);
	assert(opts);
	assert(entry);

	/*
	 * Generate the UUID or use an already generated UUID stored in 
	 * opts->uuid.
	 */

	if (opts->uuid) {
		if (!valid_uuid(opts->uuid, true)) {
			fprintf(stderr, "%s(): UUID \"%s\" is not valid.\n",
			                __func__, opts->uuid);
			return NULL;
		}
		memcpy(entry->uuid, opts->uuid, UUID_LENGTH + 1);
	} else {
		if (!generate_uuid(entry->uuid))
			return NULL; /* gncov */
		if (rc->macaddr) {
			memcpy(entry->uuid + 24, rc->macaddr,
			       MACADDR_LENGTH * 2);
		}
		if (opts->random_mac)
			scramble_mac_address(entry->uuid + 24);
	}
	if (!valid_uuid(entry->uuid, true)) {
		myerror("UUID generation failed");
		return NULL;
	}

	/*
	 * Extract the time stamp from the UUID and store it in an allocated 
	 * buffer.
	 */

	if (!uuid_date(entry->date, entry->uuid))
		return NULL;

	if (add_to_logfile(logs->logfp, entry, opts->raw))
		return NULL;

	/*
	 * Write the UUID to stdout and/or stderr, or not, depending on the 
	 * -w/--whereto argument.
	 */

	if (!opts->whereto) {
		result = puts(entry->uuid);
		if (result == EOF) {
			myerror("Cannot print UUID to stdout");
			return NULL;
		}
	} else {
		size_t len = strlen(entry->uuid) + 1;

		if (strchr(opts->whereto, 'a') || strchr(opts->whereto, 'o')) {
			result = fprintf(stdout, "%s\n", entry->uuid);
			if (result < 0 || (size_t)result != len) {
				myerror("Cannot print UUID to stdout");
				return NULL;
			}

		}
		if (strchr(opts->whereto, 'a') || strchr(opts->whereto, 'e')) {
			result = fprintf(stderr, "%s\n", entry->uuid);
			if (result < 0 || (size_t)result != len) {
				myerror("Cannot print UUID to stderr");
				return NULL;
			}
		}
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
	if (sig == SIGHUP || sig == SIGINT || sig == SIGQUIT
	    || sig == SIGPIPE || sig == SIGTERM) {
		myerror("Termination signal (%s) received, aborting",
		        strsignal(sig));
	} else {
		myerror("%s(): Unknown signal %d (%s) received,"
		        " should not happen", __func__, sig, strsignal(sig));
	}
	should_terminate = true;
}

/*
 * create_and_log_uuids() - Do everything in one place; Initialise the random 
 * number generator, read values from the rc file, environment and command 
 * line, generate the UUID(s) and write it to the log file.
 *
 * Returns a struct uuid_result with the number of UUIDs generated and a value 
 * indicating success or not. If opts->uuid contains an UUID, set count to 1 to 
 * avoid duplicates in the log file.
 */

struct uuid_result create_and_log_uuids(const struct Options *opts)
{
	struct uuid_result retval;
	char *rcfile = NULL;
	char *logfile = NULL;
	unsigned long l, count;
	struct Rc rc;
	struct Entry entry;
	struct Logs logs;

	assert(opts);

	memset(&rc, 0, sizeof(rc));
	logs.logfp = NULL;
	count = opts->count;
	retval.count = 0UL;
	memset(retval.lastuuid, 0, UUID_LENGTH + 1);
	retval.success = true;
	init_xml_entry(&entry);

	/*
	 * Get information about the environment; hostname, current directory, 
	 * tty, location of rc file and log directory, etc.
	 */

	if (init_randomness()) {
		retval.success = false; /* gncov */
		goto cleanup; /* gncov */
	}

	rcfile = get_rcfilename(opts);
	if (read_rcfile(rcfile, &rc)) {
		retval.success = false;
		goto cleanup;
	}

	if (fill_entry_struct(&entry, &rc, opts)) {
		retval.success = false;
		goto cleanup;
	}

	logfile = get_log_prefix(&rc, opts, LOGFILE_EXTENSION);
	if (!logfile) {
		retval.success = false;
		goto cleanup;
	}

	signal(SIGHUP, sighandler);
	signal(SIGINT, sighandler);
	signal(SIGQUIT, sighandler);
	signal(SIGPIPE, sighandler);
	signal(SIGTERM, sighandler);

	/*
	 * Open the log file. If it's missing, create it.
	 */

	logs.logfp = open_logfile(logfile);
	if (!logs.logfp) {
		retval.success = false;
		goto cleanup;
	}

	/*
	 * Generate the UUIDs and write them to the log file.
	 */

	if (opts->uuid)
		count = 1UL;
	for (l = 0UL; l < count; l++) {
		if (!process_uuid(&logs, &rc, opts, &entry)) {
			retval.success = false;
			/*
			 * Check that the correct amount of UUIDs were created.
			 */
			if (retval.count < opts->count) {
				myerror("Generated only %lu of %lu UUIDs",
				        retval.count, opts->count);
			}

			goto cleanup;
		}
		retval.count++;
		if (should_terminate)
			break;
	}
	if (valid_uuid(entry.uuid, true))
		memcpy(retval.lastuuid, entry.uuid, UUID_LENGTH + 1);

	/*
	 * Close up the shop and go home.
	 */

cleanup:
	if (logs.logfp && close_logfile(logs.logfp))
		retval.success = false; /* gncov */

	free(logfile);
	free_sess(&entry);
	free_tags(&entry);
	free(rc.macaddr);
	free(rc.hostname);
	free(entry.txt);
	free(entry.cwd);
	free(rcfile);

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
