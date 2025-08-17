/*
 * rcfile.c
 * File ID: 9649c988-3c09-11e6-a523-5bef14de5976
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
 * init_rc() - Initialize a `struct Rc`. Returns nothing.
 */

void init_rc(struct Rc *rc)
{
	assert(rc);

	rc->hostname = NULL;
	rc->macaddr = NULL;
}

/*
 * free_rc() - Deallocate all members of a `struct Rc`. Returns nothing.
 */

void free_rc(struct Rc *rc)
{
	assert(rc);

	free(rc->hostname);
	free(rc->macaddr);
	init_rc(rc);
}

/*
 * create_rcfile() - Create rcfile `file`, using the values in `rc`. Returns 0 
 * if successful, or 1 if anything failed.
 */

int create_rcfile(const char *file, struct Rc *rc)
{
	FILE *fp = NULL;

	assert(file);
	assert(*file);
	assert(rc);

	fp = fopen(file, "w");
	if (!fp)
		return 1; /* gncov */
	if (rc->hostname)
		fprintf(fp, "hostname = %s\n", rc->hostname);
	if (rc->macaddr)
		fprintf(fp, "macaddr = %s\n", rc->macaddr);
	if (fclose(fp))
		return 1; /* gncov */

	return 0;
}

/*
 * get_rcfilename() - Return pointer to an allocated string with name of 
 * rcfile. If neither opts->rcfile or HOME is defined, return NULL.
 */

char *get_rcfilename(const struct Options *opts)
{
	char *retval = NULL, *env;
	size_t size;

	assert(opts);

	if (opts && opts->rcfile) {
		retval = mystrdup(opts->rcfile);
		if (!retval)
			failed("mystrdup()"); /* gncov */
		return retval;
	}
	env = getenv("HOME");
	if (!env) {
		myerror("HOME environment variable not defined, cannot"
		        " determine name of rcfile");
		return NULL;
	}
	size = strlen(env) + strlen(STD_RCFILE) + 32;
	retval = malloc(size);
	if (!retval) {
		failed("malloc()"); /* gncov */
		return NULL; /* gncov */
	}
	snprintf(retval, size, "%s/%s", env, STD_RCFILE);

	return retval;
}

/*
 * has_key() - Check if line contains the keyword at the beginning of the line. 
 * If it does, return pointer to the value of that keyword, otherwise return 
 * NULL.
 */

char *has_key(const char *line, const char *keyword)
{
	char *retval;

	assert(line);
	assert(keyword);
	assert(*keyword);

	if (strlen(keyword) >= strlen(line))
		return NULL;
	if (!strncmp(line, keyword, strlen(keyword))) {
		/*
		 * Verify that the first character after what looks like a 
		 * complete keyword is either a space or '='. This avoids 
		 * misinterpreting a longer keyword that starts with the 
		 * shorter one. E.g., "hostname_extra = value" shouldn't match 
		 * "hostname".
		 */
		if (!strchr(" =", line[strlen(keyword)]))
			return NULL;
		/*
		 * Find the first '=', then skip spaces (but not '=') after it 
		 * to preserve leading '=' in the value.
		 */
		retval = strchr(line, '=');
		if (retval) {
			retval++; /* Skip the single '=' */
			while (*retval == ' ') /* Skip only spaces after '=' */
				retval++;
		}
	} else {
		retval = NULL;
	}

	return retval;
}

/*
 * parse_rc_line() - Receive a line from the rcfile and check for each keyword 
 * by sending it to check_rc() which will set the struct variable accordingly. 
 * If ok, return 0. If mystrdup() failed, return 1.
 */

int parse_rc_line(const char *line, struct Rc *rc)
{
	assert(line);
	assert(rc);

	if (has_key(line, "hostname")) {
		rc->hostname = mystrdup(has_key(line, "hostname"));
		if (!rc->hostname) {
			failed("mystrdup()"); /* gncov */
			return 1; /* gncov */
		}
	}
	if (has_key(line, "macaddr")) {
		rc->macaddr = mystrdup(has_key(line, "macaddr"));
		if (!rc->macaddr) {
			failed("mystrdup()"); /* gncov */
			return 1; /* gncov */
		}
		string_to_lower(rc->macaddr);
	}

	return 0;
}

/*
 * read_rcfile() - Read contents of rcfile into rc. rcfile is allowed to be 
 * NULL, that means it wasn't found.
 * Returns 0 if success or 1 if error.
 */

int read_rcfile(const char *rcfile, struct Rc *rc)
{
	FILE *fp;
	char buf[BUFSIZ];

	assert(rc);

	free_rc(rc);

	if (!rcfile)
		return 0;

	fp = fopen(rcfile, "r");
	if (!fp) {
		/*
		 * It's perfectly fine if it's not readable, that probably 
		 * means it doesn't exist.
		 */
		errno = 0;
		return 0;
	}

	do {
		if (!fgets(buf, BUFSIZ, fp) && errno) {
			myerror("%s: Could not read from rcfile", /* gncov */
			        rcfile);
			fclose(fp); /* gncov */
			return 1; /* gncov */
		}
		trim_str_front(buf);
		trim_str_end(buf);
		if (parse_rc_line(buf, rc)) {
			fclose(fp); /* gncov */
			return 1; /* gncov */
		}
		*buf = '\0';
	} while (!feof(fp));

	fclose(fp);

	if (rc->macaddr && !*rc->macaddr) {
		/* Keyword with no value, that's ok */
		free(rc->macaddr);
		rc->macaddr = NULL;
	}
	if (rc->macaddr && !valid_macaddr(rc->macaddr)) {
		free(rc->macaddr);
		rc->macaddr = NULL;
		return 1;
	}

	return 0;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
