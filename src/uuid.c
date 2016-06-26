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
	FILE *fp;

	/* FIXME: Generate it properly */
	fp = popen("/usr/bin/uuid", "r");
	if (!fp) {
		myerror("Could not exec /usr/bin/uuid");
		return NULL;
	}
	if (!fgets(uuid, 37, fp))
		myerror("generate_uuid(): fgets() error");
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
	static char retval[32] = "2000-01-01T00:00:00.0000000Z";

	return retval;
}

/*
 * is_hex() - Check that len bytes at the location pointed to by p are all 
 * legal lowercase hex chars. Return 1 if all characters are valid hex, 0 if 
 * not.
 */

int is_hex(char *p, unsigned int len)
{
	int retval = 1;
	int i;

	for (i = 0; i < len; i++) {
		char c = p[i];
		if (!in_range(c, '0', '9') && !in_range(c, 'a', 'f'))
			retval = 0;
	}

	return retval;
}

/*
 * valid_uuid() - Check that the UUID pointed to by u is a valid UUID. Return 1 
 * if valid, 0 if not.
 */

int valid_uuid(char *u)
{
	int retval = 1;

	if (strlen(u) != 36)
		retval = 0;

	/* Check that it only contains lowercase hex and dashes at the right 
	 * places
	 */
	if (!is_hex(u, 8) || u[8] != '-' || !is_hex(u + 9, 4) ||
	    u[13] != '-' || !is_hex(u + 14, 4) || u[18] != '-' ||
	    !is_hex(u + 19, 4) || u[23] != '-' || !is_hex(u + 24, 12))
		retval = 0;

	/* At the moment only v1 UUIDs are allowed
	 */
	if (u[14] != '1')
		retval = 0;

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
