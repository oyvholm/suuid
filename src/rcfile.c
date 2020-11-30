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
 * get_rcfilename() - Return pointer to an allocated string with name of 
 * rcfile. If neither opt->rcfile or HOME is defined, return NULL.
 */

char *get_rcfilename(const struct Options *opt)
{
	char *retval = NULL, *env;
	size_t size;

	assert(opt);

	if (opt && opt->rcfile)
		return mystrdup(opt->rcfile);
	env = getenv("HOME");
	if (!env) {
		fprintf(stderr, "%s: HOME environment variable not defined,"
		                " cannot determine name of rcfile\n",
		                progname);
		return NULL;
	}
	size = strlen(env) + strlen(STD_RCFILE) + 32;
	retval = mymalloc(size);
	if (!retval)
		return NULL; /* gncov */
	snprintf(retval, size, "%s/%s", env, STD_RCFILE); /* FIXME: slash */

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
	assert(strlen(keyword));

	if (strlen(keyword) >= strlen(line))
		return NULL;
	if (!strncmp(line, keyword, strlen(keyword))) {
		if (!strchr(" =", line[strlen(keyword)]))
			return NULL;
		/*
		 * Move retval to the first character that is not a space 
		 * (ASCII 32) after the first equal sign.
		 */
		retval = strchr(line, '=');
		while (retval && (*retval == '=' || *retval == ' '))
			retval++;
	} else {
		retval = NULL;
	}

	return retval;
}

/*
 * parse_rc_line() - Receive a line from the rcfile and check for each keyword 
 * by sending it to check_rc() which will set the struct variable accordingly. 
 * If ok, return EXIT_SUCCESS. If strdup() failed, return EXIT_FAILURE.
 */

int parse_rc_line(const char *line, struct Rc *rc)
{
	assert(line);
	assert(rc);

	if (has_key(line, "hostname")) {
		rc->hostname = mystrdup(has_key(line, "hostname"));
		if (!rc->hostname)
			return EXIT_FAILURE; /* gncov */
	}
	if (has_key(line, "macaddr")) {
		rc->macaddr = mystrdup(has_key(line, "macaddr"));
		if (!rc->macaddr)
			return EXIT_FAILURE; /* gncov */
		string_to_lower(rc->macaddr);
	}

	return EXIT_SUCCESS;
}

/*
 * read_rcfile() - Read contents of rcfile into rc. rcfile is allowed to be 
 * NULL, that means it wasn't found.
 * Returns EXIT_SUCCESS or EXIT_FAILURE.
 */

int read_rcfile(const char *rcfile, struct Rc *rc)
{
	FILE *fp;
	char buf[BUFSIZ];

	assert(rc);

	rc->hostname = NULL;
	rc->macaddr = NULL;

	if (!rcfile)
		return EXIT_SUCCESS;

	fp = fopen(rcfile, "r");
	if (!fp) {
		/*
		 * It's perfectly fine if it's not readable, that probably 
		 * means it doesn't exist.
		 */
		errno = 0;
		return EXIT_SUCCESS;
	}

	do {
		if (!fgets(buf, BUFSIZ, fp) && errno) {
			myerror("%s: Could not read from rcfile", /* gncov */
			        rcfile);
			fclose(fp); /* gncov */
			return EXIT_FAILURE; /* gncov */
		}
		trim_str_front(buf);
		trim_str_end(buf);
		if (parse_rc_line(buf, rc) == EXIT_FAILURE) {
			fclose(fp); /* gncov */
			return EXIT_FAILURE; /* gncov */
		}
		*buf = '\0';
	} while (!feof(fp));

	fclose(fp);

	if (rc->macaddr && !strlen(rc->macaddr)) {
		/* Keyword with no value, that's ok */
		free(rc->macaddr);
		rc->macaddr = NULL;
	}
	if (rc->macaddr && !valid_macaddr(rc->macaddr))
		return EXIT_FAILURE;

	return EXIT_SUCCESS;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
