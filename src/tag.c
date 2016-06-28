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

void rewind_tag(void)
{
	tag_list_ind = 0;
}

char *get_next_tag(void) {
	if (tag_list_ind < MAX_TAGS)
		return entry.tag[tag_list_ind++];
	else
		return NULL;
}

char *store_tag(char *tag)
{
	if (utf8_check((unsigned char *)tag)) {
		fprintf(stderr, "%s: Tags have to be in UTF-8\n", progname);
		return(NULL);
	}
	entry.tag[tag_count++] = strdup(tag);

	return tag;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
