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
	static char uuid[38];
	char *cmd = "/usr/bin/uuid";
	FILE *fp;

	msg(3, "generate_uuid(): rc.uuidcmd = \"%s\"", rc.uuidcmd);
	if (rc.uuidcmd) {
		cmd = rc.uuidcmd;
		msg(3, "generate_uuid(): Setting cmd to \"%s\"", cmd);
	}
	msg(4, "generate_uuid(): cmd = \"%s\"", cmd);

	/* fixme: Generate it properly */
	/* fixme: Make -m/--random-mac actually do something */
	fp = popen(cmd, "r");
	if (!fp) {
		myerror("Could not exec /usr/bin/uuid");
		return NULL;
	}
	if (!fgets(uuid, 37, fp)) {
		/* Nevermind read errors, valid_uuid() checks if it's valid 
		 * later.
		 */
	}
	uuid[36] = '\0';
	pclose(fp);

	return uuid;
}

/*
 * uuid_date() - Return pointer to string with ISO date generated from UUID v1.
 */

char *uuid_date(char *uuid)
{
	/* fixme */
	static char retval[32];

	uuid = uuid; /* Disable gcc -Wextra warning */
	strcpy(retval, "2000-01-01T00:00:00.0000000Z");

	return retval;
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

	while (strlen(p) >= 36) {
		msg(2, "scan_for_uuid(): p = \"%s\"", p);
		if (valid_uuid(p, FALSE))
			return p;
		p++;
	}

	return NULL;
}

/*
 * valid_uuid() - Check that the UUID pointed to by u is a valid UUID. If 
 * check_len is TRUE, also check that the string length is exactly the same as 
 * a standard UUID, 36 chars.
 * Return TRUE if valid, FALSE if not.
 */

bool valid_uuid(char *u, bool check_len)
{
	if (strlen(u) < 36)
		return FALSE;
	if (check_len)
		if (strlen(u) != 36)
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
