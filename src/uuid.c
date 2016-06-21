/*
 * uuid.c
 * File ID: 06472a8e-3744-11e6-8115-02010e0a6634
 *
 * (C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
	if (fp == NULL) {
		fprintf(stderr, "%s: Could not exec /usr/bin/uuid", progname);
		return NULL;
	}
	if (fgets(uuid, 37, fp) == NULL)
		fprintf(stderr, "%s: fgets() error", progname);
	uuid[36] = '\0';
	pclose(fp);
	return uuid;
}

/*
 * uuid_date() - Return pointer to string with ISO date generated from 
 * UUID v1.
 */

char *uuid_date(char *uuid)
{
	/* fixme */
	static char retval[32] = "2000-01-01T00:00:00.0000000Z";
	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
