/*
 * logfile.c
 * File ID: 5a6ffd88-3740-11e6-83c5-02010e0a6634
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

size_t MAX_GROWTH = 5; /* When converting from plain text to the XML format 
                        * used in the log file, the worst case is if the whole 
                        * string contains only ampersands, then it will grow by 
                        * a factor of five.
                        */

/*
 * valid_xml_chars() - Check that the string pointed to by s contains valid 
 * UTF-8 and no control chars. Return true if ok, false if invalid.
 */

bool valid_xml_chars(const char *s)
{
	unsigned char *p = (unsigned char *)s;

	assert(s);

	if (utf8_check(s))
		return false;
	while (*p) {
		if (*p < ' ' && !strchr("\n\t", *p))
			return false;
		if (*p == 127)
			return false;
		p++;
	}

	return true;
}

/*
 * suuid_xml() - Return pointer to allocated string where the data in the text 
 * argument is escaped for use in the XML file.
 */

char *suuid_xml(const char *text)
{
	char *retval;
	size_t size;
	const char *p;
	char *destp;

	assert(text);

	size = strlen(text);
	retval = mymalloc(size * MAX_GROWTH + 1);
	if (!retval)
		return NULL;

	destp = retval;
	for (p = text; *p; p++) {
		switch (*p) {
		case '&':
			strcpy(destp, "&amp;");
			destp += 5;
			break;
		case '<':
			strcpy(destp, "&lt;");
			destp += 4;
			break;
		case '>':
			strcpy(destp, "&gt;");
			destp += 4;
			break;
		case '\\':
			strcpy(destp, "\\\\");
			destp += 2;
			break;
		case '\n':
			strcpy(destp, "\\n");
			destp += 2;
			break;
		case '\t':
			strcpy(destp, "\\t");
			destp += 2;
			break;
		default:
			*destp++ = *p;
			break;
		}
	}
	*destp = '\0';

	return retval;
}

/*
 * init_xml_entry() - Initialise Entry struct at memory position e with initial 
 * values.
 */

void init_xml_entry(struct Entry *e)
{
	unsigned int i;

	assert(e);

	memset(e->date, 0, DATE_LENGTH + 1);
	memset(e->uuid, 0, UUID_LENGTH + 1);
	e->txt = NULL;
	e->host = NULL;
	e->cwd = NULL;
	e->user = NULL;
	e->tty = NULL;

	for (i = 0; i < MAX_TAGS; i++)
		e->tag[i] = NULL;
	for (i = 0; i < MAX_SESS; i++)
		e->sess[i].uuid = e->sess[i].desc = NULL;
}

/*
 * allocate_elem() - Allocate space and write the XML element to it.
 *   elem: char * to name of XML element
 *   src: char * to data source
 * Returns char * to allocated area, or NULL if error.
 */

char *allocate_elem(const char *elem, const char *src)
{
	char *retval, *ap;
	size_t size = 0;

	assert(elem);
	assert(strlen(elem));

	if (!src)
		return mystrdup("");

	size += strlen("<") + strlen(elem) + strlen(">") +
	        strlen(src) * MAX_GROWTH +
	        strlen("<") + strlen(elem) + strlen("/> ") + 1;

	retval = mymalloc(size + 1);
	if (!retval)
		return NULL;

	ap = suuid_xml(src);
	if (!ap)
		return NULL;

	if (strlen(ap))
		snprintf(retval, size, "<%s>%s</%s> ", elem, ap, elem);
	else
		retval[0] = '\0';

	free(ap);

	return retval;
}

/*
 * alloc_attr() - Return pointer to allocated string with XML attribute. 
 * Returns NULL on error.
 */

char *alloc_attr(const char *attr, const char *data)
{
	char *retval = NULL;
	int size;

	assert(attr);
	assert(strlen(attr));

	size = strlen(" ") + strlen(attr) + strlen("=\"") + strlen(data) +
	       strlen("\"") + 1;

	retval = mymalloc(size + 1);
	if (!retval)
		return NULL;

	snprintf(retval, size, " %s=\"%s\"", attr, data);

	return retval;
}

/*
 * get_xml_tags() - Return pointer to an allocated XML string with <tag> 
 * elements generated from the entry->tag[] array. If error, return NULL.
 */

char *get_xml_tags(const struct Entry *entry)
{
	char *p, *buf, *tmpbuf;
	size_t size = 0, tmpsize = 0;

	assert(entry);

	/*
	 * Loop through the tags and find the total size of the tags. Multiply 
	 * the size to make up for any worst case scenario (only ampersands) 
	 * and include space for the XML tags. Also find the length of the 
	 * longest tag, this is used to allocate a temporary work buffer where 
	 * each tag is converted to escaped XML before it's appended to the 
	 * returned string.
	 */

	rewind_tag();
	do {
		p = get_next_tag(entry);
		if (p) {
			size_t len = strlen(p);

			size += len * MAX_GROWTH + 16;
			if (len > tmpsize)
				tmpsize = len;
		}
	} while (p);

	if (!size)
		/*
		 * No tags found, return empty string,
		 */
		return mystrdup("");

	buf = mymalloc(size);
	if (!buf)
		return NULL;
	buf[0] = '\0';

	tmpsize = tmpsize * MAX_GROWTH + 16;
	tmpbuf = mymalloc(tmpsize);
	if (!tmpbuf) {
		free(buf);
		return NULL;
	}

	/*
	 * Loop through each tag, convert it to XML and write it to a temporary 
	 * buffer which is appended to the final string.
	 */

	rewind_tag();
	do {
		p = get_next_tag(entry);

		if (p) {
			char *ap;

			ap = suuid_xml(p);
			if (!ap) {
				free(tmpbuf);
				free(buf);
				return NULL;
			}

			snprintf(tmpbuf, tmpsize, "<tag>%s</tag> ", ap);
			free(ap);
			strncat(buf, tmpbuf, size - strlen(buf));
		}
	} while (p);

	free(tmpbuf);

	return buf;
}

/*
 * create_sess_xml() - Return pointer to allocated XML string generated from 
 * entry->sess, or NULL if error.
 */

char *create_sess_xml(const struct Entry *entry)
{
	unsigned int i;
	size_t size = 0, tmpsize = 0;
	char *buf, *tmpbuf;

	assert(entry);

	/*
	 * Loop through the sess array and find the total length of all sess 
	 * elements. Also find the length of the longest element, it's used to 
	 * allocate a temporary work buffer.
	 */

	i = 0;
	while (entry->sess[i].uuid) {
		size_t len;

		len = strlen(entry->sess[i].uuid) + 32;
		if (entry->sess[i].desc)
			len += strlen(entry->sess[i].desc) + 32;
		size += len;
		if (len > tmpsize)
			tmpsize = len;
		i++;
	}

	if (!size)
		/*
		 * No elements in the sess array, return empty string.
		 */
		return mystrdup("");

	/*
	 * Allocate space for the final string and a temporary work buffer.
	 */

	buf = mymalloc(size);
	if (!buf)
		return NULL;

	tmpbuf = mymalloc(tmpsize);
	if (!tmpbuf) {
		free(buf);
		return NULL;
	}

	buf[0] = '\0';
	i = 0;

	/*
	 * Loop through each element in the sess array, convert it to XML, 
	 * write it to a temporary buffer and append it to the final string.
	 */

	while (entry->sess[i].uuid) {
		char *u, *d;

		u = entry->sess[i].uuid;
		d = entry->sess[i].desc;
		if (d)
			snprintf(tmpbuf, tmpsize,
			         "<sess desc=\"%s\">%s</sess> ", d, u);
		else
			snprintf(tmpbuf, tmpsize, "<sess>%s</sess> ", u);
		strncat(buf, tmpbuf, size - strlen(buf));
		i++;
	}
	free(tmpbuf);

	return buf;
}

/*
 * xml_entry() - Return pointer to allocated string with one XML entry 
 * extracted from the entry struct, or NULL if error. If raw is true, insert 
 * the comment into the XML unmodified, no escaping is performed.
 */

char *xml_entry(const struct Entry *entry, const bool raw)
{
	struct Entry e;
	char *uuidp;
	char *datep = NULL;
	char *retval;
	char *tag_xml = NULL, *sess_xml = NULL;
	size_t size;

	assert(entry);
	assert(raw == false || raw == true);

	init_xml_entry(&e);

	if (!valid_uuid(entry->uuid, true))
		return NULL;

	/*
	 * Allocate space for the UUID and timestamp attributes.
	 */

	uuidp = alloc_attr("u", entry->uuid);
	if (!uuidp)
		return NULL;

	if (is_valid_date(entry->date, true)) {
		datep = alloc_attr("t", entry->date);
		if (!datep) {
			free(uuidp);
			return NULL;
		}
	}

	/*
	 * Allocate space for XML tags and sess elements.
	 */

	tag_xml = get_xml_tags(entry);
	if (!tag_xml) {
		retval = NULL;
		goto cleanup;
	}

	sess_xml = create_sess_xml(entry);
	if (!sess_xml) {
		retval = NULL;
		goto cleanup;
	}

	if (raw) {
		/*
		 * Write unescaped XML surrounded by <txt> elements to the 
		 * allocated buffer. This is for programs that need to output 
		 * an XML structure, and it's their responsibility to turn it 
		 * into valid XML. If the XML isn't well-formed, the XML log 
		 * file won't validate. The XML doesn't need to have a single 
		 * root, as it will be enclosed inside the <txt> element.
		 */
		int size;
		char *txt_space;

		size = strlen("<txt> ") + strlen(entry->txt) +
		       strlen(" </txt> ") + 1;
		e.txt = mymalloc(size);
		if (!e.txt) {
			retval = NULL;
			goto cleanup;
		}
		txt_space = entry->txt[0] == '<' ? " " : "";
		snprintf(e.txt, size, "<txt>%s%s%s</txt> ",
		                      txt_space, entry->txt, txt_space);
	} else
		/*
		 * Write escaped XML to the buffer.
		 */
		e.txt = allocate_elem("txt", entry->txt);

	e.host = allocate_elem("host", entry->host);
	e.cwd = allocate_elem("cwd", entry->cwd);
	e.user = allocate_elem("user", entry->user);
	e.tty = allocate_elem("tty", entry->tty);

	/*
	 * Allocate space for the final XML string.
	 */

	size = DATE_LENGTH + UUID_LENGTH + strlen(tag_xml) + strlen(e.txt) +
	       strlen(e.host) + strlen(e.cwd) + strlen(e.user) +
	       strlen(e.tty) + strlen(sess_xml) + 128;
	retval = mymalloc(size);
	if (!retval) {
		retval = NULL;
		goto cleanup;
	}

	/*
	 * Write the XML to the return buffer. Undefined values are skipped, 
	 * the only required data is the UUID and extracted timestamp.
	 */

	snprintf(retval, size, "<suuid%s%s> " /* date, uuid */
	                       "%s" /* tag */
	                       "%s" /* txt */
	                       "%s" /* host */
	                       "%s" /* cwd */
	                       "%s" /* user */
	                       "%s" /* tty */
	                       "%s" /* sess */
	                       "</suuid>",
	                       (datep) ? datep : "",
	                       (uuidp) ? uuidp : "",
	                       tag_xml ? tag_xml : "",
	                       (e.txt) ? e.txt : "",
	                       (e.host) ? e.host : "",
	                       (e.cwd) ? e.cwd : "",
	                       (e.user) ? e.user : "",
	                       (e.tty) ? e.tty : "", sess_xml);

cleanup:
	free(e.tty);
	free(e.user);
	free(e.cwd);
	free(e.host);
	free(e.txt);
	free(sess_xml);
	free(tag_xml);
	free(datep);
	free(uuidp);

	return retval;
}

/*
 * lock_file() - Lock file associated with fp. If locking succeeds, return FILE 
 * pointer to the stream, otherwise return NULL.
 */

FILE *lock_file(FILE *fp, const char *fname)
{
	assert(fp);
	assert(fname);
	assert(strlen(fname));

	if (flock(fileno(fp), LOCK_EX) == -1) {
		myerror("Could not lock file \"%s\"", fname);
		fclose(fp);
		return NULL;
	}

	return fp;
}

/*
 * write_xml_header() - Write the initial log file header to the log file 
 * stream.
 *
 * Return fp if success, NULL if FUBAR.
 */

FILE *write_xml_header(FILE *fp)
{
	char *header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	                "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">\n"
	                "<suuids>\n";

	assert(fp);

	if (fputs(header, fp) == EOF) {
		myerror("Cannot write header to the log file");
		return NULL;
	}

	return fp;
}

/*
 * seek_to_eof() - Seek to end of fp, and display error message about fname if 
 * it can't seek.
 *
 * Return fp if ok, NULL if error.
 */

FILE *seek_to_eof(FILE *fp, const char *fname)
{
	assert(fp);
	assert(fname);

	if (fseek(fp, 0, SEEK_END) == -1) {
		myerror("%s: Cannot seek to end of file", fname);
		return NULL;
	}

	return fp;
}

/*
 * unknown_end_line() - The last line in the logfile isn't "</suuids>\n". 
 * Complain about it and set the file position to EOF.
 *
 * Returns with value from seek_to_eof().
 */

FILE *unknown_end_line(FILE *fp, const char *fname)
{
	assert(fp);
	assert(fname);

	fprintf(stderr, "%s: %s: Unknown end line, adding to end of file\n",
	                progname, fname);

	return seek_to_eof(fp, fname);
}

/*
 * check_last_log_line() - Check that the last line of the logfile is ok, it 
 * should be identical to "</suuids>\n".
 *
 * If it isn't, return with value from unknown_end_line(). If it was ok, seek 
 * to the correct position and return fp.
 */

FILE *check_last_log_line(FILE *fp, const char *fname)
{
	long filepos;
	char check_line[12];

	assert(fp);
	assert(fname);

	if (fseek(fp, -10, SEEK_END) == -1) {
		myerror("%s: Could not seek in log file", fname);
		return NULL;
	}
	filepos = ftell(fp);
	if (filepos == -1) {
		myerror("%s: Cannot get file position of end line", fname);
		return NULL;
	}
	if (!fgets(check_line, 10, fp)) {
		myerror("Error when reading end line from log file \"%s\"",
		        fname);
		return NULL;
	}
	if (strcmp(check_line, "</suuids>"))
		return unknown_end_line(fp, fname);
	if (fseek(fp, filepos, SEEK_SET) == -1) {
		myerror("%s: Cannot seek to position %lu", fname, filepos);
		return NULL;
	}

	return fp;
}

/*
 * seek_to_entry_pos() - Check the size of the log file and call the 
 * appropriate function for setting the file position.
 *
 * Return NULL if error, otherwise return fp.
 */

FILE *seek_to_entry_pos(FILE *fp, const char *fname)
{
	long filepos;

	assert(fp);
	assert(fname);

	if (!seek_to_eof(fp, fname))
		return NULL;
	filepos = ftell(fp);
	if (filepos == -1) {
		myerror("%s: Cannot get file position at EOF", fname);
		return NULL;
	}
	if (filepos > 10)
		return check_last_log_line(fp, fname);
	if (filepos == 0)
		return write_xml_header(fp);

	return unknown_end_line(fp, fname);
}

/*
 * open_logfile() - Open log file fname for read+write if it exists, or create 
 * it if it doesn't. Then lock it and give control to seek_to_entry_pos().
 *
 * Return FILE pointer to the opened stream, ready for writing. If anything 
 * fails, NULL is returned.
 */

FILE *open_logfile(const char *fname)
{
	FILE *fp;

	assert(fname);
	assert(strlen(fname));

	/*
	 * Fixme: Make the existence check/file creation atomic. See the log 
	 * message for commit a0c635c3aba8 ("Get rid of create_logfile() and 
	 * set_up_logfile(), move into open_logfile()", 2016-07-06) for more 
	 * info. ... OK, because I feel generous today, here it is:
	 *
	 * Trying to get rid of a small race condition when the log file is 
	 * created. In some situations it can create an extra header when two 
	 * processes create the file at the same time. The creation isn't 
	 * atomic, there's a small gap between the existence check and when 
	 * it's opened and locked. It's hard to reproduce, and it's maybe a 
	 * non-problem because it only happens when the file is created and 
	 * lots of processes are hammering on it, that's an unlikely scenario. 
	 * Nevertheless, I'll look into it because it's kind of annoying.
	 */

	if (access(fname, F_OK) != -1) {
		/* File already exists */
		fp = fopen(fname, "r+");
		if (!fp) {
			myerror("%s: Could not open file for read+write",
			        fname);
			return NULL;
		}
	} else {
		/* File doesn't exist */
		fp = fopen(fname, "a");
		if (!fp) {
			myerror("%s: Could not create log file", fname);
			return NULL;
		}
	}
	if (!lock_file(fp, fname))
		return NULL;

	return seek_to_entry_pos(fp, fname);
}

/*
 * add_to_logfile() - Add the contents of *entry to the logfile stream. Return 
 * EXIT_SUCCESS or EXIT_FAILURE.
 */

int add_to_logfile(FILE *fp, const struct Entry *entry, const bool raw)
{
	char *ap;
	int retval = EXIT_SUCCESS;

	assert(fp);
	assert(entry);
	assert(raw == false || raw == true);

	ap = xml_entry(entry, raw);
	if (!ap)
		return EXIT_FAILURE;
	if (fputs(ap, fp) < 0)
		retval = EXIT_FAILURE;
	if (fputc('\n', fp) == EOF)
		retval = EXIT_FAILURE;
	if (retval == EXIT_FAILURE)
		myerror("add_to_logfile(): Cannot write to the log file");

	free(ap);

	return retval;
}

/*
 * close_logfile() - Do the finishing changes on FILE stream fp, add end tag 
 * and close the stream. Return EXIT_SUCCESS if no errors, if any errors were 
 * detected, return EXIT_FAILURE.
 */

int close_logfile(FILE *fp)
{
	int retval = EXIT_SUCCESS;

	assert(fp);

	if (fprintf(fp, "</suuids>\n") != 10)
		retval = EXIT_FAILURE;
	if (fflush(fp) == EOF)
		retval = EXIT_FAILURE;
	flock(fileno(fp), LOCK_UN);
	if (fclose(fp) == EOF)
		retval = EXIT_FAILURE;

	if (retval == EXIT_FAILURE)
		myerror("Error when closing log file");

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
