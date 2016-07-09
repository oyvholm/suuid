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
 * check_hex() - Check that len bytes at the location pointed to by p are all 
 * legal lowercase hex chars. Return a pointer to the first invalid character 
 * or NULL if everything is ok.
 */

char *check_hex(char *hex, size_t len)
{
	char *p;

	assert(hex);
	for (p = hex; p < hex + len; p++)
		if (!strchr("0123456789abcdef", *p))
			return p;

	return NULL;
}

/*
 * trim_str_front() - Modify dest by removing initial whitespace. Returns dest.
 */

char *trim_str_front(char *dest)
{
	char *p = dest;
	size_t size;

	assert(dest);
	size = strlen(dest);
	while (p < dest + size && isspace(*p))
		p++;
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
	size_t size;

	assert(dest);
	size = strlen(dest);
	if (!size)
		return dest;
	p = dest + size - 1;
	while (p > dest && isspace(*p))
		*p-- = '\0';

	return dest;
}

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
 * Modifications by Øyvind A. Holm <sunny@sunbase.org>:
 *
 *   2016-06-29 - receive and return regular char * instead of unsigned char * 
 *   to avoid casting everywhere.
 *   2016-07-09 - Add const modifier to text argument.
 */

char *utf8_check(const char *text)
{
	unsigned char *s = (unsigned char *)text;

	assert(text);

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

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
