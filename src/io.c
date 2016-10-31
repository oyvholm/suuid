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
	size_t bytes_read = 0, total_bytes_read = 0;
	size_t bufsize = BUFSIZ;

	do {
		char *new_mem = realloc(retval,
		                        bufsize + total_bytes_read + 1);
		if (!new_mem) {
			myerror("read_from_fp(): Cannot allocate memory for "
			        "stream buffer");
			free(retval);
			return NULL;
		}
		retval = new_mem;
		p = retval + total_bytes_read;
		bytes_read = fread(p, 1, bufsize, fp);
		total_bytes_read += bytes_read;
		p[bytes_read] = '\0';
		if (ferror(fp)) {
			myerror("read_from_fp(): Read error");
			free(retval);
			return NULL;
		}
	} while (!feof(fp));

	return retval;
}

/*
 * read_from_file() - Read contents of file fname and return a pointer to a 
 * allocated string with the contents, or NULL if error.
 */

char *read_from_file(const char *fname)
{
	FILE *fp;
	char *retval;

	fp = fopen(fname, "rb");
	if (!fp) {
		myerror("read_from_file(): Could not open file for read");
		return NULL;
	}
	retval = read_from_fp(fp);
	if (!retval)
		return NULL;
	fclose(fp);

	return retval;
}

/*
 * read_from_editor() - Open editor e on a temporary file and return the 
 * contents as an allocated string, or NULL if error.
 */

char *read_from_editor(const char *editor)
{
	char *retval,
	     tmpfile[] = ".tmp-suuid.XXXXXX",
	     *cmdbuf;
	size_t size;

	if (mkstemp(tmpfile) == -1) {
		myerror("read_from_editor(): Could not create file name for"
		        "temporary file");
		return NULL;
	}
	size = strlen(editor) + strlen(tmpfile) + 5;
	cmdbuf = malloc(size);
	if (!cmdbuf) {
		myerror("read_from_editor(): Could not allocate %lu bytes for "
		        "command buffer", size);
		return NULL;
	}
	snprintf(cmdbuf, size, "%s %s", editor, tmpfile);
	if (system(cmdbuf) == -1) {
		myerror("read_from_editor(): Cannot execute \"%s\"", cmdbuf);
		return NULL;
	}
	retval = read_from_file(tmpfile);
	if (!retval)
		return NULL;
	if (remove(tmpfile) == -1)
		myerror("Warning: Could not remove temporary file \"%s\"",
		        tmpfile);

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
