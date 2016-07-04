/*
 * uuid.c
 * File ID: 06472a8e-3744-11e6-8115-02010e0a6634
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
 * valid_uuid() - Check that the UUID pointed to by u is a valid UUID. If 
 * check_len is TRUE, also check that the string length is exactly the same as 
 * a standard UUID, UUID_LENGTH chars.
 * Return TRUE if valid, FALSE if not.
 */

bool valid_uuid(char *u, bool check_len)
{
	if (strlen(u) < UUID_LENGTH)
		return FALSE;
	if (check_len)
		if (strlen(u) != UUID_LENGTH)
			return FALSE;

	/* Check that it only contains lowercase hex and dashes at the right 
	 * places
	 */
	if (check_hex(u, 8) || u[8] != '-' || check_hex(u + 9, 4) ||
	    u[13] != '-' || check_hex(u + 14, 4) || u[18] != '-' ||
	    check_hex(u + 19, 4) || u[23] != '-' || check_hex(u + 24, 12))
		return FALSE;

	/* At the moment only v1 UUIDs are allowed
	 */
	if (u[14] != '1')
		return FALSE;

	return TRUE;
}

/*
 * generate_uuid() - Return a pointer to a string with a generated UUID v1, or 
 * NULL if error.
 */

char *generate_uuid(void)
{
	static char uuid[UUID_LENGTH + 2];
	char *cmd = "uuid";
	FILE *fp;

	if (rc.uuidcmd)
		cmd = rc.uuidcmd;

	/* fixme: Generate it properly */
	/* fixme: Make -m/--random-mac actually do something */
	fp = popen(cmd, "r");
	if (!fp) {
		myerror("generate_uuid(): Could not exec \"%s\"", cmd);
		return NULL;
	}
	if (!fgets(uuid, UUID_LENGTH + 1, fp)) {
		/* Nevermind read errors, valid_uuid() checks if it's valid 
		 * later.
		 */
	}
	uuid[UUID_LENGTH] = '\0';
	pclose(fp);

	return uuid;
}

/*
 * uuid_date() - Receive an UUID v1 and write the UUID date to dest, 29 bytes 
 * (ISO 8601 date plus terminating null byte). Return pointer to dest if ok, or 
 * NULL if it's not a valid v1 UUID.
 */

char *uuid_date(char *dest, char *uuid)
{
	/* fixme: unfinished */
	char hexbuf[16];

	if (!valid_uuid(uuid, FALSE))
		return NULL;
	if (uuid[14] != '1')
		return NULL; /* Not a v1 UUID, has no timestamp */

	memset(hexbuf, 0, 16);
	strncat(hexbuf, uuid + 15, 3);
	strncat(hexbuf, uuid + 9, 4);
	strncat(hexbuf, uuid, 8);

	if (1)
		strncpy(dest, "2000-01-01T00:00:00.0000000Z", DATE_LENGTH + 1);

	return dest;
}

/*
 * is_valid_date() - Check that the date pointed to by s is valid. If check_len 
 * is TRUE, also check that the string length is correct.
 * Return 1 if ok, 0 if invalid.
 */

bool is_valid_date(char *s, bool check_len)
{
	if (check_len && strlen(s) != DATE_LENGTH)
		return FALSE;

	if (s[0] != '2' || s[1] != '0' || /* Yay for Y2.1K */
	    !isdigit(s[2]) || !isdigit(s[3]) || /* Two last digits in year */
	    s[4] != '-' ||
	    !strchr("01", s[5]) || !isdigit(s[6]) || /* Month */
	    s[7] != '-' ||
	    !strchr("0123", s[8]) || !isdigit(s[9]) || /* Day */
	    s[10] != 'T' ||
	    !strchr("012", s[11]) || !isdigit(s[12]) || /* Hour */
	    s[13] != ':' ||
	    !strchr("012345", s[14]) || !isdigit(s[15]) || /* Minute */
	    s[16] != ':' ||
	    !strchr("0123456", s[17]) || !isdigit(s[18]) || /* Second */
	    s[19] != '.' ||
	    !isdigit(s[20]) || !isdigit(s[21]) || !isdigit(s[22]) ||
	    !isdigit(s[23]) || !isdigit(s[24]) || !isdigit(s[25]) ||
	    !isdigit(s[26]) || /* Nanoseconds */
	    s[27] != 'Z')
		return FALSE;
	else
		return TRUE;
}

/*
 * uuid_date_from_uuid() - Same functionality as uuid_date(), but use the 
 * uuid(1) program to calculate the date. Use until uuid_date() works.
 */

char *uuid_date_from_uuid(char *dest, char *uuid)
{
	FILE *fp;
	char cmd[50];
	char *ap, *p;

	assert(dest);

	if (!valid_uuid(uuid, FALSE))
		return NULL;
	if (uuid[14] != '1')
		return NULL; /* Not a v1 UUID, has no timestamp */

	memset(cmd, 0, 50);
	snprintf(cmd, 49, "uuid -d %s", uuid);

	fp = popen(cmd, "r");
	if (!fp) {
		myerror("uuid_date_from_uuid(): Could not exec \"%s\"", cmd);
		return NULL;
	}
	ap = read_from_fp(fp);
	pclose(fp);

	p = strstr(ap, "content: time:");
	if (!p) {
		fprintf(stderr, "%s: uuid_date_from_uuid(): Search string not "
		                 "found in uuid(1) output\n", progname);
		return NULL;
	}
	p = strstr(p, "20"); /* This is how you create Y2.1K problems, kids */
	if (!p) {
		fprintf(stderr, "%s: uuid_date_from_uuid(): Didn't find year "
		                "in uuid(1) output\n", progname);
		return NULL;
	}

	p[10] = 'T';
	p[26] = p[27];
	p[27] = 'Z';
	p[28] = '\0';

	strncpy(dest, p, DATE_LENGTH + 1);
	free(ap);
	if (!is_valid_date(dest, TRUE)) {
		fprintf(stderr, "uuid_date_from_uuid(): Generated date is "
		                "invalid: \"%s\"\n", dest);
		return NULL;
	}

	return dest;
}

/*
 * scan_for_uuid() - Return a pointer to the first UUID in the string s, or 
 * NULL if no UUID was found.
 */

char *scan_for_uuid(char *s)
{
	char *p = s;

	while (strlen(p) >= UUID_LENGTH) {
		if (valid_uuid(p, FALSE))
			return p;
		p++;
	}

	return NULL;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
