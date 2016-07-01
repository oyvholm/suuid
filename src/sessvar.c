/*
 * sessvar.c
 * File ID: a3d401d8-3f18-11e6-bafd-02010e0a6634
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

static unsigned int sessind;

bool is_legal_desc_char(unsigned char c)
{
	return strchr(DESC_LEGAL, c) ? TRUE : FALSE;
}

bool is_valid_desc_string(char *s)
{
	char *p = s;

	while (*p) {
		if (!is_legal_desc_char(*p))
			return FALSE;
		p++;
	}

	if (utf8_check(s))
		return FALSE;

	return TRUE;
}

int fill_sess(struct Sess *dest, char *desc, char *uuid)
{
	msg(2, "Entering fill_sess({uuid:\"%s\", uuid:\"%s\"}, "
	       "\"%s\", \"%s\")",
	       dest->uuid, dest->desc,
	       desc, uuid);
	if (desc && strlen(desc) && is_valid_desc_string(desc)) {
		dest->desc = strdup(desc);
		if (dest->desc) {
		}
	}

	if (uuid && valid_uuid(uuid, FALSE))
		dest->uuid = desc;
	else
		return EXIT_ERROR;

	return EXIT_OK;
}

/*
 * get_sess_info() - Read sess information from the environment variable and 
 * insert it into the entry.sess array. Returns EXIT_OK or EXIT_ERROR.
 */

int get_sess_info(struct Entry *entry)
{
	char *var, *s, *p, *desc_found, *desc_end;

	var = getenv(ENV_SESS);
	if (!var)
		return EXIT_OK;
	msg(2, "get_sess_info(): var = \"%s\"", var);

	s = strdup(var);
	if (!s) {
		myerror("get_sess_info(): Could not duplicate %s environment "
		        "variable", ENV_SESS);
		return EXIT_ERROR;
	}

	p = scan_for_uuid(s);
	if (!p)
		return EXIT_OK;

	desc_found = desc_end = NULL;
	p = s;
	while (*p) {
		if (valid_uuid(p, FALSE)) {
			struct Sess dest;
			char *auuid, *adesc;

			auuid = strndup(p, UUID_LENGTH);
			if (!auuid) {
				myerror("get_sess_info(): Could not "
					"duplicate UUID");
				return EXIT_ERROR;
			}

			adesc = strndup(desc_found, desc_end - desc_found);
			if (!adesc) {
				myerror("get_sess_info(): Could not "
					"duplicate desc");
				return EXIT_ERROR;
			}

			if (fill_sess(&dest, adesc, auuid) == EXIT_OK) {
				entry->sess[sessind]->uuid = auuid;
				entry->sess[sessind]->desc = adesc;
			}
			p += UUID_LENGTH;
		} else if (is_legal_desc_char(*p)) {
			msg(4, "get_sess_info(): '%c' is a legal desc char",
			       *p);
			if (!desc_found)
				desc_found = p;
		} else if (*p == '/') {
			msg(2, "get_sess_info(): Found slash");
			desc_end = p;
			p++;
		}
		p++;
	}

	return EXIT_OK;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
