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
	msg(2, "%sEntering fill_sess({uuid:\"%s\", desc:\"%s\"}, "
	       "\"%s\", \"%s\")%s",
	       T_RED, dest->uuid, dest->desc,
	       desc, uuid, T_RESET);
	DEBL;
	if (desc && strlen(desc) && is_valid_desc_string(desc)) {
		DEBL;
		dest->desc = strdup(desc);
		if (!dest->desc) {
			myerror("fill_sess(): Could not duplicate desc");
			return EXIT_ERROR;
		}
		DEBL;
	}
	DEBL;

	if (uuid && valid_uuid(uuid, TRUE))
		dest->uuid = uuid;
	else
		msg(2, "fill_sess() received invalid UUID \"%s\", "
		       "should not happen", uuid);
	DEBL;

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
	DEBL;
	msg(2, "get_sess_info(): var = \"%s\"", var);

	s = strdup(var);
	if (!s) {
		myerror("get_sess_info(): Could not duplicate %s environment "
		        "variable", ENV_SESS);
		return EXIT_ERROR;
	}

	if (!scan_for_uuid(s))
		return EXIT_OK;

	DEBL;
	desc_found = desc_end = NULL;
	p = s;
	while (*p) {
		msg(6, "Loop: p = \"%s\"", p);
		if (valid_uuid(p, FALSE)) {
			struct Sess dest;
			char *auuid = NULL, *adesc = NULL;

			msg(2, "get_sess_info(): Found valid UUID, p = \"%s\"",
			       p);
			auuid = strndup(p, UUID_LENGTH);
			if (!auuid) {
				myerror("get_sess_info(): Could not "
					"duplicate UUID");
				return EXIT_ERROR;
			}
			msg(2, "get_sess_info(): auuid = \"%s\"", auuid);

			DEBL;
			msg(2, "desc_found = %p, desc_end = %p",
			       desc_found, desc_end);
			if (desc_end > desc_found) {
				DEBL;
				adesc = strndup(desc_found,
				                desc_end - desc_found);
				DEBL;
				if (!adesc) {
					myerror("get_sess_info(): Could not "
					        "duplicate desc");
					return EXIT_ERROR;
				}
				msg(2, "get_sess_info(): adesc = \"%s\"",
				       adesc);
			}

			if (fill_sess(&dest, adesc, auuid) == EXIT_ERROR) {
				myerror("get_sess_info(): fill_sess() failed");
				DEBL;
				return EXIT_ERROR;
			}
			msg(2, "%sget_sess_info(): dest.uuid after "
			        "fill_sess(): \"%s\"%s",
			        T_GREEN, dest.uuid, T_RESET);
			msg(2, "%sget_sess_info(): dest.desc after "
			        "fill_sess(): \"%s\"%s",
			        T_GREEN, dest.desc, T_RESET);

			entry->sess[sessind].uuid = dest.uuid;
			DEBL;
			entry->sess[sessind].desc = dest.desc;
			DEBL;
			p += UUID_LENGTH - 1;
			msg(2, "p after increasing with "
			       "UUID_LENGTH - 1 = \"%s\"", p);
			desc_found = desc_end = NULL;
			sessind++;
			DEBL;
		} else if (is_legal_desc_char(*p)) {
			if (!desc_found && p >= s) {
				desc_found = p;
				msg(2, "get_sess_info(): Set desc_found to p, "
				       "\"%s\"", desc_found);
			}
		} else if (*p == '/') {
			if (desc_found) {
				desc_end = p;
				msg(2, "get_sess_info(): Found slash, "
				       "desc_end is set to \"%s\"", desc_end);
			} else
				msg(2, "Found slash, but desc_found is not "
				       "defined, doing nothing");
		} else {
			msg(2, "Found invalid desc char, reset");
			desc_found = desc_end = NULL;
		}
		p++;
	}

	return EXIT_OK;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
