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

bool valid_xml_chars(char *s)
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

char *suuid_xml(char *text)
{
	char *retval;
	size_t size = strlen(text);
	char *p, *destp;

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

char *allocate_elem(char *elem, char *src)
{
	char *retval, *ap;
	size_t size = 0;

	if (!elem || !src) {
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
 * alloc_attr() - Return pointer to string with XML attribute. Returns NULL on 
 * error.
 */

char *alloc_attr(char *attr, char *data)
{
	char *retval = NULL;
	int size;

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
 * get_xml_tags() - Return pointer to an XML string with <tag> elements 
 * generated from the entry.tag[] array. If error, return NULL.
 */

char *get_xml_tags(void)
{
	char *p,
	     *buf,
	     *tmpbuf;
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
 * create_sess_xml() - Return pointer to XML string generated from entry->sess, 
 * or NULL if error.
 */

char *create_sess_xml(struct Entry *entry)
{
#define CSX_BUFSIZE 20000 /* fixme: Temporary, use dynamic allocation later */
#define CSX_TMPBUFSIZE 1000 /* Another temporary fixme */
	static char buf[CSX_BUFSIZE],
	            tmpbuf[CSX_TMPBUFSIZE];
	unsigned int i = 0;

	buf[0] = '\0';
	while (entry->sess[i].uuid) {
		char *u, *d;

		u = entry->sess[i].uuid;
		d = entry->sess[i].desc;
		if (d)
			snprintf(tmpbuf, CSX_TMPBUFSIZE,
			         "<sess desc=\"%s\">%s</sess> ", d, u);
		else
			snprintf(tmpbuf, CSX_TMPBUFSIZE, "<sess>%s</sess> ",
			                                 u);
		strncat(buf, tmpbuf, CSX_BUFSIZE - strlen(buf));
		i++;
	}

	return buf;
#undef CSX_TMPBUFSIZE /* fixme */
#undef CSX_BUFSIZE /* fixme */
}

/*
 * xml_entry() - Return pointer to allocated string with one XML entry 
 * extracted from the entry struct, or NULL if error.
 */

char *xml_entry(struct Entry *entry)
{
	struct Entry e;
	char *retval;
	char *tag_xml, *sess_xml;
	size_t size;

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
		                      txt_space,
		                      entry->txt,
		                      txt_space);
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

	snprintf(retval, size,
	         "<suuid%s%s> " /* date, uuid */
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
	         (e.tty) ? e.tty : "",
	         sess_xml
	         );

	free(e.tty);
	free(e.user);
	free(e.cwd);
	free(e.host);
	free(e.txt);
	/* free(sess_xml); */ /* fixme: later */
	free(tag_xml);
	free(e.date);
	free(e.uuid);

	return retval;
}

/*
 * get_logdir() - Return pointer to string with location of the log directory. 
 * Use the value of -l/--logdir if it's defined, otherwise use the environment 
 * variable defined in ENV_LOGDIR, otherwise use "$HOME/uuids". If that also 
 * fails, return NULL.
 */

char *get_logdir()
{
	char *retval = NULL;

	if (opt.logdir)
		retval = opt.logdir;
	else if (getenv(ENV_LOGDIR))
		retval = getenv(ENV_LOGDIR);
	else if (getenv("HOME")) {
		int size = strlen(getenv("HOME")) +
		           strlen("/uuids") + 1;

		retval = malloc(size + 1);
		if (!retval) {
			myerror("get_logdir(): Cannot allocate %lu bytes",
			        size);
			return NULL;
		}
		snprintf(retval, size, "%s/uuids",
		                       getenv("HOME"));
	} else {
		fprintf(stderr, "%s: $%s and $HOME environment "
		                "variables are not defined, cannot "
		                "create logdir path",
		                progname, ENV_LOGDIR);
		return NULL;
	}

	return retval;
}

/*
 * create_logfile() - Create logfile with initial XML structure if it doesn't 
 * exist already. On success, return pointer to string with file name, or NULL 
 * if error.
 */

char *create_logfile(char *name)
{
	char *xml_header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char *xml_doctype = "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">";
	FILE *fp;

	if (access(name, F_OK) != -1)
		return name; /* File already exists */

	fp = fopen(name, "a");
	if (!fp) {
		myerror("%s: Could not create log file", name);
		return NULL;
	}
	fprintf(fp, "%s\n%s\n<suuids>\n</suuids>\n", xml_header, xml_doctype);
	fclose(fp);

	return name;
}

/*
 * set_up_logfile() - Determine log file name and create an initial XML log 
 * file if it doesn't exist.
 * Return pointer to name of log file, or NULL if error.
 */

char *set_up_logfile(struct Options *opt, char *hostname)
{
	char *logdir, *logfile;
	size_t fname_length; /* Total length of logfile name */

	logdir = get_logdir(&opt);
	if (!logdir) {
		fprintf(stderr, "%s: Unable to find logdir location\n",
		                progname);
		return NULL;
	}

	fname_length = strlen(logdir) + strlen("/") +
	               strlen(hostname) + strlen(".xml") + 1;
	logfile = malloc(fname_length + 1);
	if (!logfile) {
		myerror("Could not allocate %lu bytes for logfile filename",
		        fname_length + 1);
		return NULL;
	}
	/* fixme: Remove slash hardcoding */
	snprintf(logfile, fname_length, "%s/%s.xml", logdir, hostname);
	if (!create_logfile(logfile)) {
		myerror("%s: Error when creating log file", logfile);
		return NULL;
	}

	return logfile;
}

/*
 * Open log file fname for read+write, check that the end of file is OK, set 
 * the file position to the place where the new log entry shall be inserted. 
 * Return FILE pointer to the opened stream, ready for writing. If anything 
 * fails, NULL is returned.
 */

FILE *open_logfile(char *fname)
{
	FILE *fp;
	char check_line[12];
	long filepos;

	/* todo: Add file locking */
	fp = fopen(fname, "r+");
	if (!fp) {
#if PERL_COMPAT
		myerror("%s: Cannot open file for append", fname);
		perlexit13 = TRUE;
#else
		myerror("%s: Could not open file for read+write", fname);
#endif
		return NULL;
	}
	if (fseek(fp, -10, SEEK_END) == -1) {
		myerror("%s: Could not seek in log file", fname);
		return NULL;
	}
	filepos = ftell(fp);
	if (filepos == -1) {
		myerror("%s: Cannot read file position", fname);
		return NULL;
	}
	if (strcmp(fgets(check_line, 10, fp), "</suuids>"))
		fprintf(stderr, "%s: %s: Unknown end line, adding to "
		                "end of file\n", progname, fname);
	else
		if (fseek(fp, filepos, SEEK_SET) == -1) {
			myerror("%s: Cannot seek to position %lu",
			        fname, filepos);
			return NULL;
		}

	return fp;
}

/*
 * add_to_logfile() - Add the contents of *entry to the logfile stream. Return 
 * EXIT_OK or EXIT_ERROR.
 */

int add_to_logfile(FILE *fp, struct Entry *entry)
{
	char *ap;
	int retval = EXIT_OK;

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
	int i;

	if (fprintf(fp, "</suuids>\n") != 10)
		retval = EXIT_ERROR;
	if (fclose(fp) == EOF)
		retval = EXIT_ERROR;
	if (opt.verbose >= 3) {
		i = system("(echo; cat /home/sunny/uuids/fake.xml; "
		           "echo) >&2");
		i = i; /* Get rid of gcc warning */
	}

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
