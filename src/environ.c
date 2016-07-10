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
	char *retval;

	if (getenv(ENV_EDITOR)) {
		retval = strdup(getenv(ENV_EDITOR));
		if (!retval) {
			myerror("get_editor(): Could not duplicate "
			        "%s environment variable", ENV_EDITOR);
			return NULL;
		}
	} else if (getenv("EDITOR")) {
		retval = strdup(getenv("EDITOR"));
		if (!retval) {
			myerror("get_editor(): Could not duplicate "
			        "EDITOR environment variable");
			return NULL;
		}
	} else {
		retval = strdup(STD_EDITOR); /* In case it's free()'ed later */
		if (!retval) {
			myerror("get_editor(): Could not duplicate \"%s\"",
			        STD_EDITOR);
			return NULL;
		}
	}

	return retval;
}

/*
 * valid_hostname() - Check if hostname in string s is a somewhat valid 
 * hostname. Return TRUE if ok, FALSE if not.
 */

bool valid_hostname(const char *s)
{
	unsigned char *p;

	if (!s || !strlen(s) || strlen(s) > HOST_NAME_MAX ||
	    strstr(s, "..") || utf8_check(s))
		return FALSE;

	p = (unsigned char *)s;
	while (*p) {
		if (strchr("!\"#$%&'()*+,/:;<=>?@[\\]^`{|}~", *p) ||
		    *p < '!' || *p >= '\x7f')
			return FALSE;
		p++;
	}

	return TRUE;
}

/*
 * get_hostname() - Return pointer to string with hostname of the computer, or 
 * NULL if error.
 */

char *get_hostname(const struct Rc *rc)
{
	static char buf[HOST_NAME_MAX + 1];
	char *retval = buf;

	if (getenv(ENV_HOSTNAME))
		strncpy(buf, getenv(ENV_HOSTNAME), HOST_NAME_MAX);
	else if (rc->hostname)
		strncpy(buf, rc->hostname, HOST_NAME_MAX);
#if FAKE_HOST
	else if (1)
		retval = "fake";
#endif
	else if (gethostname(buf, HOST_NAME_MAX) == -1)
		return NULL;

	return retval;
}

/*
 * getpath() - Return pointer to string with full path to current directory, or 
 * NULL if error. Use free() on the pointer when it's not needed anymore.
 */

char *getpath(void)
{
	char *retval;
	char *p;
	size_t size = BUFSIZ;

	retval = malloc(size);
	if (!retval) {
		myerror("getpath(): Could not allocate %lu bytes in first "
		        "malloc()", size);
		return NULL;
	}
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
			/* Avoid infinite loop in case there's another getcwd() 
			 * problem that's not fixable by just allocating more 
			 * memory.
			 */
			myerror("getpath(): getcwd() failed");
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
