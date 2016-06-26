/*
 * io.c
 * File ID: ada23776-3a67-11e6-8cbf-a50d0c0491ce
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
 * read_from_fp() - Read data from fp into an allocated buffer and return a 
 * pointer to the allocated memory or NULL if something failed.
 */

char *read_from_fp(FILE *fp)
{
	char *retval = NULL;
	char *p = NULL;
	size_t bytes_read = 0;

	do {
		char *new_mem = realloc(retval,
		                        BUFSIZ + bytes_read + 1);
		if (!new_mem) {
			myerror("Cannot allocate memory for stdin buffer");
			if (retval)
				free(retval);
			return NULL;
		}
		retval = new_mem;
		p = retval + bytes_read;
		bytes_read += fread(p, 1, BUFSIZ, fp);
		msg(4, "read_from_fp(): bytes_read = %lu", bytes_read);
		if (ferror(fp)) {
			myerror("Error when reading stdin");
			free(retval);
			return NULL;
		}
	} while (!feof(fp));

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
