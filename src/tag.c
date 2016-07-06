/*
 * tag.c
 * File ID: ee2458fc-3cf5-11e6-b8f8-9b274834a07e
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

unsigned int tag_count = 0;
unsigned int tag_list_ind = 0;

/*
 * rewind_tag() - Reset the index counter so get_next_tag() starts from the 
 * beginning.
 */

void rewind_tag(void)
{
	tag_list_ind = 0;
}

/*
 * tag_exists() - Return TRUE if tag already is added to the array, FALSE if 
 * not.
 */

bool tag_exists(char *tag)
{
	unsigned int i;

	for (i = 0; i < tag_count; i++)
		if (!strcmp(tag, entry.tag[i]))
			return TRUE;

	return FALSE;
}

/*
 * get_next_tag() - Return a pointer to a string with the next stored tag name. 
 * Returns NULL when the last tag has been found.
 */

char *get_next_tag(void)
{
	if (tag_list_ind < MAX_TAGS)
		return entry.tag[tag_list_ind++];
	else
		return NULL;
}

/*
 * store_tag() - Store a new tag in the array. If ok, return pointer to the tag 
 * name, otherwise NULL.
 */

char *store_tag(char *arg)
{
	char *tag, *p;

	tag = strdup(arg); /* Don't modify the source */
	if (!tag) {
		myerror("store_tag(): Could not duplicate arg string");
		return NULL;
	}

	while ((p = strchr(tag, ','))) {
		*p++ = '\0';
		store_tag(tag);
		tag = p;
	}
	trim_str_front(tag);
	trim_str_end(tag);
	if (tag_exists(tag) || !strlen(tag))
		return tag;
	if (utf8_check(tag)) {
		fprintf(stderr, "%s: Tags have to be in UTF-8\n", progname);
		return NULL;
	}
	entry.tag[tag_count++] = strdup(tag);

	return tag;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
