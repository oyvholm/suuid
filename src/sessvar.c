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

int fill_sess(struct Sess *dest, char *uuid, char *desc)
{
	char *auuid = NULL;

	msg(2, "%sEntering fill_sess({uuid:\"%s\", desc:\"%s\"}, "
	       "\"%s\", \"%s\")%s",
	       T_RED, dest->uuid, dest->desc,
	       uuid, desc, T_RESET);

	auuid = strndup(uuid, UUID_LENGTH);
	if (!auuid) {
		myerror("fill_sess(): Could not duplicate UUID");
		return EXIT_ERROR;
	}

	if (desc && strlen(desc) && is_valid_desc_string(desc)) {
		dest->desc = strdup(desc);
		if (!dest->desc) {
			myerror("fill_sess(): Could not duplicate desc");
			return EXIT_ERROR;
		}
	}

	if (valid_uuid(auuid, TRUE))
		dest->uuid = auuid;
	else
		msg(2, "fill_sess() received invalid UUID \"%s\", "
		       "should not happen", auuid);

	return EXIT_OK;
}

/*
 * get_sess_info() - Read sess information from the environment variable and 
 * insert it into the entry.sess array. Returns EXIT_OK or EXIT_ERROR.
 */

int get_sess_info(struct Entry *entry)
{
	char *s, *p, *desc_found, *desc_end;

	if (!getenv(ENV_SESS))
		return EXIT_OK;

	s = strdup(getenv(ENV_SESS));
	if (!s) {
		myerror("get_sess_info(): Could not duplicate %s environment "
		        "variable", ENV_SESS);
		return EXIT_ERROR;
	}

	if (!scan_for_uuid(s))
		return EXIT_OK;

	desc_found = desc_end = NULL;
	p = s;
	while (*p) {
		if (valid_uuid(p, FALSE)) {
			struct Sess dest;
			char *adesc = NULL;

			dest.uuid = dest.desc = NULL;
			if (desc_found && !desc_end)
				desc_end = p; /* There was no slash between 
				               * desc and uuid, so desc_end 
				               * hasn't been set.
				               */
			if (desc_end > desc_found) {
				adesc = strndup(desc_found,
				                desc_end - desc_found);
				if (!adesc) {
					myerror("get_sess_info(): Could not "
					        "duplicate desc");
					return EXIT_ERROR;
				}
			}

			if (fill_sess(&dest, p, adesc) == EXIT_ERROR) {
				myerror("get_sess_info(): fill_sess() failed");
				return EXIT_ERROR;
			}

			entry->sess[sessind].uuid = dest.uuid;
			entry->sess[sessind].desc = dest.desc;
			p += UUID_LENGTH - 1;
			desc_found = desc_end = NULL;
			sessind++;
		} else if (is_legal_desc_char(*p)) {
			if (!desc_found && p >= s)
				desc_found = p;
		} else if (*p == '/') {
			if (desc_found)
				desc_end = p;
		} else
			desc_found = desc_end = NULL;
		p++;
	}

	return EXIT_OK;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
