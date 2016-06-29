/*
 * utf8.c
 * File ID: 8847e78c-376f-11e6-82a9-02010e0a6634
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
 * The utf8_check() function scans the '\0'-terminated string starting at s. It 
 * returns a pointer to the first byte of the first malformed or overlong UTF-8 
 * sequence found, or NULL if the string contains only correct UTF-8. It also 
 * spots UTF-8 sequences that could cause trouble if converted to UTF-16, 
 * namely surrogate characters (U+D800..U+DFFF) and non-Unicode positions 
 * (U+FFFE..U+FFFF). This routine is very likely to find a malformed sequence 
 * if the input uses any other encoding than UTF-8. It therefore can be used as 
 * a very effective heuristic for distinguishing between UTF-8 and other 
 * encodings.
 *
 * I wrote this code mainly as a specification of functionality; there are no 
 * doubt performance optimizations possible for certain CPUs.
 *
 * Markus Kuhn <http://www.cl.cam.ac.uk/~mgk25/> -- 2005-03-30
 * License: http://www.cl.cam.ac.uk/~mgk25/short-license.html
 *
 * Modified by Øyvind A. Holm <sunny@sunbase.org> 2016-06-29 to receive and 
 * return regular char * instead of unsigned char * to avoid casting 
 * everywhere.
 */

char *utf8_check(char *text)
{
	unsigned char *s = (unsigned char *)text;

	msg(4, "Entering utf8_check()");
	while (*s) {
		if (*s < 0x80)
			/* 0xxxxxxx */
			s++;
		else if ((s[0] & 0xe0) == 0xc0) {
			/* 110XXXXx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 ||
			    (s[0] & 0xfe) == 0xc0) /* overlong? */
				return (char *)s;
			else
				s += 2;
		} else if ((s[0] & 0xf0) == 0xe0) {
			/* 1110XXXX 10Xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 ||
			    (s[0] == 0xe0 &&
			    (s[1] & 0xe0) == 0x80) || /* overlong? */
			    (s[0] == 0xed &&
			    (s[1] & 0xe0) == 0xa0) || /* surrogate? */
			    (s[0] == 0xef && s[1] == 0xbf &&
			    (s[2] & 0xfe) == 0xbe)) /* U+FFFE or U+FFFF? */
				return (char *)s;
			else
				s += 3;
		} else if ((s[0] & 0xf8) == 0xf0) {
			/* 11110XXX 10XXxxxx 10xxxxxx 10xxxxxx */
			if ((s[1] & 0xc0) != 0x80 || (s[2] & 0xc0) != 0x80 ||
			    (s[3] & 0xc0) != 0x80 || (s[0] == 0xf0 &&
			    (s[1] & 0xf0) == 0x80) || /* overlong? */
			    (s[0] == 0xf4 && s[1] > 0x8f) ||
			    s[0] > 0xf4) /* > U+10FFFF? */
				return (char *)s;
			else
				s += 4;
		} else
			return (char *)s;
	}

	return NULL;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
