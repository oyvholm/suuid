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
 * has_key() - Check if line contains the keyword at the beginning of the line. 
 * If it does, return pointer to the value of that keyword, otherwise return 
 * NULL.
 */

char *has_key(char *line, char *keyword)
{
	char *search_str;
	size_t size;
	char *retval;

	size = strlen(keyword) + 2;
	search_str = malloc(size);
	if (!search_str) {
		myerror("has_key(): Could not allocate %lu bytes", size);
		return NULL;
	}
	snprintf(search_str, size, "%s ", keyword);
	msg(3, "has_key(): search_str = \"%s\"", search_str);

	if (!strncmp(line, search_str, strlen(search_str))) {
		msg(3, "has_key(): Found \"%s\"", keyword);

		/* Move retval to the first character that is not a space 
		 * (ASCII 32) after the first equal sign.
		 */
		retval = strchr(line, '=');
		while (retval && (*retval == '=' || *retval == ' '))
			retval++;
	} else
		retval = NULL;

	msg(3, "has_key() returns \"%s\"", retval);
	return retval;
}

/*
 * parse_rc_line() - Receive a line from the rcfile and check for each keyword 
 * by sending it to check_rc() whom will set the struct variable accordingly.
 */

void parse_rc_line(char *line, struct Rc *rc)
{
	msg(3, "Entering parse_rc_line(\"%s\", ...)", line);
	msg(3, "rc->uuidcmd before has_key(): \"%s\"", rc->uuidcmd);
	if (has_key(line, "hostname"))
		rc->hostname = has_key(line, "hostname");
	if (has_key(line, "uuidcmd"))
		rc->uuidcmd = has_key(line, "uuidcmd");
	msg(3, "rc->uuidcmd after has_key(): \"%s\"", rc->uuidcmd);
}

/*
 * Read contents of rcfile into rc. Return EXIT_OK or EXIT_ERROR.
 */

int read_rcfile(char *rcfile, struct Rc *rc)
{
	FILE *fp;
	char buf[BUFSIZ];

	msg(3, "Entering read_rcfile(\"%s\", ...)", rcfile);
	msg(3, "read_rcfile(): rc->uuidcmd = \"%s\"", rc->uuidcmd);

	rc->hostname = NULL;
	rc->uuidcmd = NULL;

	if (!rcfile) {
		msg(3, "rcfile is NULL, return EXIT_OK from read_rcfile()");
		return EXIT_OK;
	}

	fp = fopen(rcfile, "r");
	if (!fp)
		return EXIT_OK; /* It's perfectly fine if it's not readable, 
		                 * that probably means it doesn't exist.
		                 */

	do {
		if (!fgets(buf, BUFSIZ, fp) && errno) {
			msg(3, "read_rcfile(): if part 1: buf = \"%s\"", buf);
			myerror("%s: Could not read from rcfile", rcfile);
			return EXIT_ERROR;
		}
		msg(3, "read_rcfile(): if part 2: buf = \"%s\"", buf);
		trim_str_front(buf);
		trim_str_end(buf);
		parse_rc_line(buf, rc);
		*buf = '\0';
	} while (!feof(fp));

	return EXIT_OK;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
