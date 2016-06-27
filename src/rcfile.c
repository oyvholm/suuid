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

char *has_key(char *line, char *keyword)
{
	char *search_str;
	size_t size;
	char *retval;

	size = strlen(keyword) + 2;
	search_str = malloc(size);
	if (!search_str)
	{
		myerror("has_key(): Could not allocate %lu bytes", size);
		return NULL;
	}
	snprintf(search_str, size, "%s ", keyword);
	msg(3, "has_key(): search_str = \"%s\"", search_str);

	if (!strncmp(line, search_str, strlen(search_str))) {
		msg(3, "has_key(): Found \"%s\"", keyword);
		retval = keyword;
	} else
		retval = NULL;
	msg(3, "has_key() returns \"%s\"", retval);

	return retval;
}

void check_rc(char *keyword, char *var, char *line)
{
	char *p;

	if (!has_key(line, keyword))
		return;
	msg(3, "check_rc(): Yo, found %s", keyword);
	p = strchr(line, '=');
	while (p && (*p == '=' || *p == ' '))
		p++;
	var = p;
	msg(3, "check_rc() set var = \"%s\"", var);
}

void parse_rc_line(char *line, struct Rc *rc)
{
	msg(3, "Entering parse_rc_line(\"%s\", ...)", line);

	check_rc("uuidcmd", rc->uuidcmd, line);
}

/*
 * Read contents of rcfile into rc. Return EXIT_OK or EXIT_ERROR.
 */

int read_rcfile(char *rcfile, struct Rc *rc)
{
	FILE *fp;
	char buf[BUFSIZ];

	msg(3, "Entering rcfile(\"%s\", ...)", rcfile);

	if (!rcfile)
		return EXIT_OK;

	fp = fopen(rcfile, "r");
	if (!fp)
		return EXIT_OK;

	do {
		if (!fgets(buf, BUFSIZ, fp) && errno) {
			myerror("%s: Could not read from rcfile", rcfile);
			return EXIT_ERROR;
		}
		trim_str_front(buf);
		trim_str_end(buf);
		parse_rc_line(buf, rc);
	} while(!feof(fp));

	return EXIT_OK;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
