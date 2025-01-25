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
 * tag_exists() - Return true if tag already is added to the array, false if 
 * not.
 */

bool tag_exists(const struct Entry *entry, const char *tag)
{
	unsigned int i;

	assert(entry);
	assert(entry->tag);
	assert(tag);

	for (i = 0; i < tag_count; i++) {
		if (!strcmp(tag, entry->tag[i]))
			return true;
	}

	return false;
}

/*
 * get_next_tag() - Return a pointer to a string with the next stored tag name. 
 * Returns NULL when the last tag has been found.
 */

char *get_next_tag(const struct Entry *entry)
{
	assert(entry);
	assert(entry->tag);

	if (tag_list_ind < MAX_TAGS)
		return entry->tag[tag_list_ind++];
	else
		return NULL;
}

/*
 * store_tag() - Store a new tag in the array. Return 0 if the tag was 
 * successfully added or it existed from before, or 1 if something failed.
 */

int store_tag(struct Entry *entry, const char *arg)
{
	char *tag, *p;
	int retval = 0;

	assert(entry);
	assert(arg);

	tag = mystrdup(arg); /* Don't modify the source */
	if (!tag)
		return 1; /* gncov */

	while ((p = strchr(tag, ','))) {
		*p++ = '\0';
		if (store_tag(entry, tag)) {
			retval = 1;
			goto cleanup;
		}
		if (p && strlen(p)) {
			/*
			 * This whole thing could be replaced by a single 
			 * strcpy(tag, p), but Valgrind complains about source 
			 * and destination overlap. It's better to uglify the 
			 * source a bit than having Valgrind errors, so live 
			 * with it for now.
			 */
			char *tag2;

			tag2 = mymalloc(strlen(p) + 1);
			if (!tag2) {
				retval = 1; /* gncov */
				goto cleanup; /* gncov */
			}
			strcpy(tag2, p);
			free(tag);
			tag = tag2;
		}
	}
	trim_str_front(tag);
	trim_str_end(tag);
	if (tag_exists(entry, tag) || !strlen(tag))
		goto cleanup;
	if (utf8_check(tag)) {
		fprintf(stderr, "%s: Tags have to be in UTF-8\n", progname);
		retval = 1;
		goto cleanup;
	}

	if (tag_count >= MAX_TAGS) {
		fprintf(stderr, "%s: Maximum number of tags (%d) exceeded\n",
		                progname, MAX_TAGS);
		retval = 1;
		goto cleanup;
	}

	if (!(entry->tag[tag_count++] = mystrdup(tag)))
		retval = 1; /* gncov */

cleanup:
	free(tag);

	return retval;
}

/*
 * free_tags() - Free all allocated strings in the tag array.
 */

void free_tags(struct Entry *entry)
{
	unsigned int i;

	for (i = 0; entry->tag[i]; i++)
		free(entry->tag[i]);
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
