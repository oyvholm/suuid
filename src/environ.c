/*
 * environ.c
 * File ID: d31b36f8-38a8-11e6-89ed-02010e0a6634
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
 * get_editor() - Return pointer to allocated string with the name of the 
 * user's favourite text editor. Return NULL if it can't find out.
 */

char *get_editor(void)
{
	char *e;

	e = getenv(ENV_EDITOR);
	if (!e || !strlen(e))
		e = getenv("EDITOR");
	if (e && strlen(e))
		return mystrdup(e);
	myerror("Environment variables %s and EDITOR aren't defined, "
	        "cannot start editor", ENV_EDITOR);

	return NULL;
}

/*
 * valid_hostname() - Check if hostname in string s is a somewhat valid 
 * hostname. Return true if ok, false if not.
 */

bool valid_hostname(const char *s)
{
	unsigned char *p;

	assert(s);

	if (!strlen(s) || strlen(s) > HOST_NAME_MAX ||
	    strstr(s, "..") || utf8_check(s))
		return false;

	p = (unsigned char *)s;
	while (*p) {
		/*
		 * Fixme: Check up on some RFC or something so the correct 
		 * rules are enforced, don't just invent something. Have only 
		 * included characters here that look like they shouldn't be 
		 * allowed. Like, in these wonderful modern times, are 
		 * characters above U+007F allowed? Or even if it isn't, should 
		 * it be allowed here? In the end it ends up as a file name 
		 * anyway.
		 */
		if (strchr("!\"#$%&'()*+,/:;<=>?@[\\]^`{|}~", *p) ||
		    *p < '!' || *p >= '\x7f')
			return false;
		p++;
	}

	return true;
}

/*
 * get_hostname() - Return pointer to string with hostname of the computer, or 
 * NULL if error.
 */

char *get_hostname(const struct Rc *rc)
{
	static char buf[HOST_NAME_MAX + 1];
	char *p;

	assert(rc);

	p = getenv(ENV_HOSTNAME);
	if (!p)
		p = rc->hostname;
	if (p) {
		if (!valid_hostname(p)) {
			myerror("Got invalid hostname: \"%s\"", p);
			return NULL;
		}
		strncpy(buf, p, HOST_NAME_MAX);
	} else {
		if (gethostname(buf, HOST_NAME_MAX) == -1) {
			myerror("Cannot get hostname");
			return NULL;
		}
	}

#ifdef FAKE_HOST
	strncpy(buf, "fake", HOST_NAME_MAX);
#endif
	return buf;
}

/*
 * get_logdir() - Return pointer to allocated string with location of the log 
 * directory. Use the value of opt->logdir if it's defined, otherwise use the 
 * environment variable defined in ENV_LOGDIR, otherwise use "$HOME/uuids". If 
 * that also fails, return NULL. opt is allowed to be NULL, it's called by 
 * usage().
 */

char *get_logdir(const struct Options *opt)
{
	char *p = NULL, *retval = NULL;

	if (opt && opt->logdir)
		p = opt->logdir;
	else if (getenv(ENV_LOGDIR))
		p = getenv(ENV_LOGDIR);
	else if (getenv("HOME")) {
		/*
		 * Use default hardcoded value.
		 */
		int size = strlen(getenv("HOME")) +
		           strlen("/uuids") + 1; /* fixme: slash */

		retval = mymalloc(size + 1);
		if (!retval)
			return NULL;
		snprintf(retval, size, "%s/uuids", /* fixme: slash */
		                       getenv("HOME"));
	} else {
		fprintf(stderr, "%s: $%s and $HOME environment "
		                "variables are not defined, cannot "
		                "create logdir path\n", progname, ENV_LOGDIR);
		return NULL;
	}
	if (p)
		retval = mystrdup(p);

	return retval;
}

/*
 * get_log_prefix() - Return pointer to an allocated string with log or 
 * database prefix (full path without the file extension), or NULL if it can't 
 * be determined. The third "ext" argument is a file extension that will be 
 * added to the returned string, it can be "", NULL or a string.
 */

char *get_log_prefix(const struct Rc *rc, const struct Options *opt, char *ext)
{
	char *logdir, *hostname;
	size_t prefix_length; /* Total length of prefix */
	char *prefix = NULL;

	assert(rc);
	assert(opt);

	/*
	 * Get full path to the log directory.
	 */

	logdir = get_logdir(opt);
	if (!logdir)
		return NULL;

	/*
	 * Get the host name from the environment, rc file, or use the real 
	 * hostname for the computer. This name is used as the part of the file 
	 * name before the file extension.
	 */

	hostname = get_hostname(rc);
	if (!hostname)
		goto cleanup;

	if (!ext)
		ext = "";

	/*
	 * Build the full log file path with extension if provided and 
	 * return it as an allocated string.
	 */

	prefix_length = strlen(logdir) + strlen("/") + /* fixme: slash */
	                strlen(hostname) + strlen(ext) + 1;
	prefix = mymalloc(prefix_length + 1);
	if (!prefix)
		goto cleanup;
	/* fixme: Remove slash hardcoding, use some portable solution */
	snprintf(prefix, prefix_length, "%s/%s%s", logdir, hostname, ext);

cleanup:
	free(logdir);

	return prefix;
}

/*
 * getpath() - Return pointer to allocated string with full path to current 
 * directory, or NULL if error. Use free() on the pointer when it's not needed 
 * anymore.
 */

char *getpath(void)
{
	char *retval;
	char *p;
	size_t size = BUFSIZ;

	retval = mymalloc(size);
	if (!retval)
		return NULL;
	for (p = getcwd(retval, size); !p;) {
		size += BUFSIZ;
		retval = realloc(retval, size);
		if (!retval) {
			myerror("getpath(): Could not reallocate %lu bytes",
			        size);
			return NULL;
		}
		p = getcwd(retval, size);
		if (!p && errno != ERANGE) {
			/*
			 * Avoid infinite loop in case there's another getcwd() 
			 * problem that's not fixable by just allocating more 
			 * memory.
			 */
			myerror("getpath(): Cannot get current directory");
			free(retval);
			return NULL;
		}
	}

	return retval;
}

/*
 * get_username() - Return pointer to string with login name, or NULL if error.
 */

char *get_username(void)
{
	char *retval;
	struct passwd *pw;

	pw = getpwuid(getuid());
	if (!pw)
		retval = NULL;
	else
		retval = pw->pw_name;

	return retval;
}

/*
 * get_tty() - Return pointer to string with name of current tty.
 */

char *get_tty(void)
{
	return ttyname(STDIN_FILENO);
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
