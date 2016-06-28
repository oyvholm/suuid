/*
 * string.c
 * File ID: bfaee004-3a99-11e6-a49d-e9fd2f75d24e
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
 * trim_str_front() - Modify dest by removing initial whitespace. Returns dest.
 */

char *trim_str_front(char *dest)
{
	char *p = dest;
	size_t size = strlen(dest);

	msg(5, "Entering trim_str_front(\"%s\")", dest);
	while (p < dest + size && isspace(*p)) {
		p++;
		msg(5, "trim_str_front(): p = \"%s\"", p);
	}
	if (p == dest)
		return dest; /* Nothing needs to be done */
	memmove(dest, p, strlen(p) + 1);

	return dest;
}

/*
 * trim_str_end() - Modify dest by removing whitespace from the end of the 
 * string. Returns dest.
 */

char *trim_str_end(char *dest)
{
	char *p;
	size_t size = strlen(dest);

	msg(5, "Entering trim_str_end(\"%s\")", dest);
	if (!size)
		return dest;
	p = dest + size - 1;
	msg(5, "trim_str_end(): init p to \"%s\"", p);
	while (p > dest && isspace(*p)) {
		*p-- = '\0';
		msg(5, "trim_str_end(): p = \"%s\"", p);
	}

	return dest;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
