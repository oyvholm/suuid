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
 * generate_uuid()
 */

char *generate_uuid(void)
{
	static char uuid[UUID_LENGTH + 2];
	char *cmd = "/usr/bin/uuid";
	FILE *fp;

	if (rc.uuidcmd)
		cmd = rc.uuidcmd;

	/* fixme: Generate it properly */
	/* fixme: Make -m/--random-mac actually do something */
	fp = popen(cmd, "r");
	if (!fp) {
		myerror("Could not exec /usr/bin/uuid");
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
 * check_hex() - Check that len bytes at the location pointed to by p are all 
 * legal lowercase hex chars. Return a pointer to the first invalid character 
 * or NULL if everything is ok.
 */

char *check_hex(char *hex, size_t len)
{
	char *p;

	for (p = hex; p < hex + len; p++)
		if (!strchr("0123456789abcdef", *p))
			return p;

	return NULL;
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

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
