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
 * UTF-8 and no control chars. Return TRUE if ok, FALSE if invalid.
 */

bool valid_xml_chars(const char *s)
{
	unsigned char *p = (unsigned char *)s;

	if (utf8_check(s))
		return FALSE;
	while (*p) {
		if (*p < ' ' && !strchr("\n\r\t", *p))
			return FALSE;
		if (*p == 127)
			return FALSE;
		p++;
	}

	return TRUE;
}

/*
 * suuid_xml() - Return pointer to allocated string where the data in the text 
 * argument is escaped for use in the XML file.
 */

char *suuid_xml(const char *text)
{
	char *retval;
	size_t size = strlen(text);
	const char *p;
	char *destp;

	retval = malloc(size * MAX_GROWTH + 1);
	if (!retval) {
		myerror("Cannot allocate %lu bytes for XML",
		        size + MAX_GROWTH + 1);
		return NULL;
	}

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
		case '\r':
			strcpy(destp, "\\r");
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

	e->date = NULL;
	e->uuid = NULL;
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

	if (!src) {
		retval = strdup("");
		if (!retval) {
			myerror("allocate_elem(): Could not duplicate empty "
			        "string");
			return NULL;
		}
		return retval;
	}

	size += strlen("<") + strlen(elem) + strlen(">") +
	        strlen(src) * MAX_GROWTH +
	        strlen("<") + strlen(elem) + strlen("/> ") + 1;

	retval = malloc(size + 1);
	if (!retval) {
		myerror("allocate_elem(): Cannot allocate %lu bytes",
		        size + 1);
		return NULL;
	}

	ap = suuid_xml(src);
	if (!ap) {
		myerror("allocate_elem(): suuid_xml() failed");
		return NULL;
	}
	snprintf(retval, size, "<%s>%s</%s> ", elem, ap, elem);
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

	retval = malloc(size + 1);
	if (!retval) {
		myerror("alloc_attr(): Cannot allocate %lu bytes", size + 1);
		return NULL;
	}

	snprintf(retval, size, " %s=\"%s\"", attr, data);

	return retval;
}

/*
 * get_xml_tags() - Return pointer to an allocated XML string with <tag> 
 * elements generated from the entry.tag[] array. If error, return NULL.
 */

char *get_xml_tags(void)
{
	char *p, *buf, *tmpbuf;
	size_t size = 0, tmpsize = 0;

	rewind_tag();
	do {
		p = get_next_tag();
		if (p) {
			size_t len = strlen(p);

			size += len * MAX_GROWTH + 16;
			if (len > tmpsize)
				tmpsize = len;
		}
	} while (p);

	buf = malloc(size);
	if (!buf) {
		myerror("get_xml_tags(): Could not allocate %lu bytes for buf",
		        size);
		return NULL;
	}
	buf[0] = '\0';

	tmpsize = tmpsize * MAX_GROWTH + 16;
	tmpbuf = malloc(tmpsize);
	if (!tmpbuf) {
		myerror("get_xml_tags(): Could not allocate %lu bytes for "
		        "tmpbuf", tmpsize);
		return NULL;
	}

	rewind_tag();
	do {
		p = get_next_tag();

		if (p) {
			char *ap;

			ap = suuid_xml(p);
			if (!ap) {
				myerror("get_xml_tags(): suuid_xml() failed");
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

	buf = malloc(size);
	if (!buf) {
		myerror("create_sess_xml(): Cannot allocate %lu bytes for buf",
		        size);
		return NULL;
	}
	tmpbuf = malloc(tmpsize);
	if (!tmpbuf) {
		myerror("create_sess_xml(): Cannot allocate %lu bytes for "
		        "tmpbuf", tmpsize);
		free(buf);
		return NULL;
	}
	buf[0] = '\0';
	i = 0;
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
 * extracted from the entry struct, or NULL if error.
 */

char *xml_entry(const struct Entry *entry)
{
	struct Entry e;
	char *retval;
	char *tag_xml, *sess_xml;
	size_t size;

	assert(entry);

	init_xml_entry(&e);

	if (!entry->uuid)
		return NULL;

	e.uuid = alloc_attr("u", entry->uuid);
	if (!e.uuid) {
		myerror("xml_entry(): alloc_attr() for uuid failed");
		return NULL;
	}

	if (entry->date) {
		e.date = alloc_attr("t", entry->date);
		if (!e.date) {
			myerror("xml_entry(): alloc_attr() for date failed");
			return NULL;
		}
	}

	tag_xml = get_xml_tags();
	if (!tag_xml) {
		myerror("xml_entry(): get_xml_tags() failed");
		return NULL;
	}

	sess_xml = create_sess_xml(entry);
	if (!sess_xml) {
		myerror("create_sess_xml() failed");
		return NULL;
	}

	if (opt.raw) {
		int size;
		char *txt_space;

		size = strlen("<txt> ") + strlen(entry->txt) +
		       strlen(" </txt> ") + 1;
		e.txt = malloc(size);
		if (!e.txt) {
			myerror("xml_entry(): Could not allocate %lu bytes "
			        "for raw XML <txt> string", size);
			return NULL;
		}
		txt_space = entry->txt[0] == '<' ? " " : "";
		snprintf(e.txt, size, "<txt>%s%s%s</txt> ",
		                      txt_space, entry->txt, txt_space);
	} else
		e.txt = allocate_elem("txt", entry->txt);
	e.host = allocate_elem("host", entry->host);
	e.cwd = allocate_elem("cwd", entry->cwd);
	e.user = allocate_elem("user", entry->user);
	e.tty = allocate_elem("tty", entry->tty);

	size = DATE_LENGTH + UUID_LENGTH + strlen(tag_xml) + strlen(e.txt) +
	       strlen(e.host) + strlen(e.cwd) + strlen(e.user) +
	       strlen(e.tty) + strlen(sess_xml) + 128;
	retval = malloc(size);
	if (!retval) {
		myerror("xml_entry(): Could not allocate %lu bytes for XML "
		        "string", size);
		return NULL;
	}

	snprintf(retval, size, "<suuid%s%s> " /* date, uuid */
	                       "%s" /* tag */
	                       "%s" /* txt */
	                       "%s" /* host */
	                       "%s" /* cwd */
	                       "%s" /* user */
	                       "%s" /* tty */
	                       "%s" /* sess */
	                       "</suuid>",
	                       (e.date) ? e.date : "",
	                       (e.uuid) ? e.uuid : "",
	                       tag_xml ? tag_xml : "",
	                       (e.txt) ? e.txt : "",
	                       (e.host) ? e.host : "",
	                       (e.cwd) ? e.cwd : "",
	                       (e.user) ? e.user : "",
	                       (e.tty) ? e.tty : "", sess_xml);

	free(e.tty);
	free(e.user);
	free(e.cwd);
	free(e.host);
	free(e.txt);
	free(sess_xml);
	free(tag_xml);
	free(e.date);
	free(e.uuid);

	return retval;
}

/*
 * get_logdir() - Return pointer to allocated string with location of the log 
 * directory. Use the value of -l/--logdir if it's defined, otherwise use the 
 * environment variable defined in ENV_LOGDIR, otherwise use "$HOME/uuids". If 
 * that also fails, return NULL.
 */

char *get_logdir(void)
{
	char *retval = NULL;

	if (opt.logdir) {
		retval = strdup(opt.logdir);
		if (!retval) {
			myerror("get_logdir(): Could not duplicate "
			        "-l/--logdir argument");
			return NULL;
		}
	} else if (getenv(ENV_LOGDIR)) {
		retval = strdup(getenv(ENV_LOGDIR));
		if (!retval) {
			myerror("get_logdir(): Could not duplicate %s "
			        "environment variable", ENV_LOGDIR);
			return NULL;
		}
	} else if (getenv("HOME")) {
		int size = strlen(getenv("HOME")) +
		           strlen("/uuids") + 1; /* fixme: slash */

		retval = malloc(size + 1);
		if (!retval) {
			myerror("get_logdir(): Cannot allocate %lu bytes",
			        size);
			return NULL;
		}
		snprintf(retval, size, "%s/uuids", /* fixme: slash */
		                       getenv("HOME"));
	} else {
		fprintf(stderr, "%s: $%s and $HOME environment "
		                "variables are not defined, cannot "
		                "create logdir path\n", progname, ENV_LOGDIR);
		return NULL;
	}

	return retval;
}

/*
 * get_logfile_name() - Return pointer to an allocated string with log file 
 * name, or NULL if it can't be determined.
 */

char *get_logfile_name(void)
{
	char *logdir, *logfile = NULL, *hostname;
	size_t fname_length; /* Total length of logfile name */

	logdir = get_logdir();
	if (!logdir) {
		fprintf(stderr, "%s: get_logfile_name(): Unable to find "
		                "logdir location\n", progname);
		return NULL;
	}

	hostname = get_hostname();
	if (!hostname) {
		myerror("get_logfile_name(): Cannot get hostname");
		goto cleanup;
	}
	if (!valid_hostname(hostname)) {
		myerror("get_logfile_name(): Got invalid hostname: \"%s\"",
		        hostname);
		goto cleanup;
	}

	fname_length = strlen(logdir) + strlen("/") + /* fixme: slash */
	               strlen(hostname) + strlen(".xml") + 1;
	logfile = malloc(fname_length + 1);
	if (!logfile) {
		myerror("get_logfile_name(): Could not allocate %lu bytes for "
		        "logfile filename", fname_length + 1);
		goto cleanup;
	}
	/* fixme: Remove slash hardcoding, use some portable solution */
	snprintf(logfile, fname_length, "%s/%s.xml", logdir, hostname);

cleanup:
	free(logdir);

	return logfile;
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
	 * fixme: Make the existence check/file creation atomic. See the log 
	 * message for commit a0c635c ("Get rid of create_logfile() and 
	 * set_up_logfile(), move into open_logfile()", 2016-07-06) for more 
	 * info.
	 */

	if (access(fname, F_OK) != -1) {
		/* File already exists */
		fp = fopen(fname, "r+");
		if (!fp) {
#if PERL_COMPAT
			myerror("%s: Cannot open file for append", fname);
			perlexit13 = TRUE;
#else
			myerror("%s: Could not open file for read+write",
			        fname);
#endif
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
 * EXIT_OK or EXIT_ERROR.
 */

int add_to_logfile(FILE *fp, const struct Entry *entry)
{
	char *ap;
	int retval = EXIT_OK;

	assert(entry);

	ap = xml_entry(entry);
	if (!ap) {
		myerror("add_to_logfile(): xml_entry() failed");
		return EXIT_ERROR;
	}
	if (fputs(ap, fp) < 0) {
		myerror("add_to_logfile(): fputs() failed");
		retval = EXIT_ERROR;
	}
	if (fputc('\n', fp) == EOF) {
		myerror("add_to_logfile(): fputc() failed");
		retval = EXIT_ERROR;
	}
	free(ap);

	return retval;
}

/*
 * close_logfile() - Do the finishing changes on FILE stream fp, add end tag 
 * and close the stream. Return EXIT_OK if no errors, if any errors were 
 * detected, return EXIT_ERROR.
 */

int close_logfile(FILE *fp)
{
	int retval = EXIT_OK;

	if (fprintf(fp, "</suuids>\n") != 10)
		retval = EXIT_ERROR;
	if (fflush(fp) == EOF)
		retval = EXIT_ERROR;
	flock(fileno(fp), LOCK_UN);
	if (fclose(fp) == EOF)
		retval = EXIT_ERROR;

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
