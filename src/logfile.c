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
 * init_xml_entry() - Initialise Entry struct at memory position e with initial 
 * values.
 */

void init_xml_entry(struct Entry *e)
{
	e->date = NULL;
	e->uuid = NULL;
	e->tag = NULL;
	e->txt = NULL;
	e->host = NULL;
	e->cwd = NULL;
	e->user = NULL;
	e->tty = NULL;
	e->sess = NULL;
}

/*
 * allocate_entry() - Allocate space and write the XML element to it.
 *   elem: char * to name of XML element
 *   src: char * to data source
 * Returns char * to allocated area, or NULL if error.
 */

char *allocate_entry(char *elem, char *src)
{
	char *retval;
	size_t size = 0;

	msg(4, "Entering allocate_entry(\"%s\", \"%s\")", elem, src);
	if (elem && src) {
		size += strlen("<") + strlen(elem) + strlen(">") +
		        strlen(src) * MAX_GROWTH +
		        strlen("<") + strlen(elem) + strlen("/> ") + 1;
		msg(3, "allocate_entry(): size = %lu", size);
		retval = malloc(size + 1);
		if (!retval)
			perror("allocate_entry(): Cannot allocate memory");
		else
			snprintf(retval, size, "<%s>%s</%s> ",
			                       elem, suuid_xml(src), elem);
	} else
		retval = NULL;
	msg(3, "allocate_entry() returns '%s'", retval);

	return retval;
}

/* suuid_xml() - Return pointer to string where the data in the text argument 
 * is escaped for use in the XML file.
 */

char *suuid_xml(char *text)
{
	char *retval;
	size_t size = strlen(text);
	char *p, *destp;

	retval = malloc(size * MAX_GROWTH + 1);
	if (!retval) {
		fprintf(stderr, "%s: Cannot allocate %lu bytes for XML\n",
		                progname,
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
			*destp = *p;
			destp++;
			break;
		}
	}
	*destp = '\0';

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

	msg(3, "Entering alloc_attr(\"%s\", \"%s\")", attr, data);
	size = strlen(" ") +
	       strlen(attr) +
	       strlen("=\"") +
	       strlen(data) +
	       strlen("\"") +
	       1;
	msg(3, "data size = %lu", size);
	retval = malloc(size + 1);
	if (!retval)
		perror("alloc_attr(): Cannot allocate memory");
	else
		snprintf(retval, size, " %s=\"%s\"", attr, data);
	msg(3, "alloc_attr() returns \"%s\"", retval);
	return retval;
}

/*
 * xml_entry() - Return pointer to string with one XML entry extracted from the 
 * entry struct, or NULL if error.
 */

char *xml_entry(struct Entry *entry)
{
#define XML_BUFSIZE  1000000 /* fixme: Temporary */
	static char buf[XML_BUFSIZE];
	struct Entry e;
	char *retval;

	msg(3, "Entering xml_entry()");
	init_xml_entry(&e);
	msg(3, "xml_entry(): After init_xml_entry()");

	msg(3, "xml_entry(): entry->date = '%s'", entry->date);
	msg(3, "xml_entry(): entry->uuid = '%s'", entry->uuid);
	msg(3, "xml_entry(): entry->tag = '%s'",  entry->tag);
	msg(4, "xml_entry(): entry->txt = '%s'",  entry->txt);
	msg(3, "xml_entry(): entry->host = '%s'", entry->host);
	msg(3, "xml_entry(): entry->cwd = '%s'",  entry->cwd);
	msg(3, "xml_entry(): entry->user = '%s'", entry->user);
	msg(3, "xml_entry(): entry->sess = '%s'", entry->sess);

	if (!entry->uuid) {
		msg(2, "xml_entry(): uuid is NULL");
		return NULL;
	} else
		e.uuid = alloc_attr("u", entry->uuid);

	if (entry->date)
		e.date = alloc_attr("t", entry->date);

	e.txt = allocate_entry("txt", entry->txt);
	e.host = allocate_entry("host", entry->host);
	e.cwd = allocate_entry("cwd", entry->cwd);
	e.user = allocate_entry("user", entry->user);
	e.tty = allocate_entry("tty", entry->tty);

	snprintf(buf, XML_BUFSIZE, /* fixme: length */
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
	         (e.tag) ? e.tag : "",
	         (e.txt) ? e.txt : "",
	         (e.host) ? e.host : "",
	         (e.cwd) ? e.cwd : "",
	         (e.user) ? e.user : "",
	         (e.tty) ? e.tty : "",
	         (e.sess) ? e.sess : "");
	msg(3, "xml_entry(): After snprintf()");
#if 0
	static char fake[] = "<suuid t=\"2016-06-07T04:18:40.9460630Z\" "
	                     "u=\"ea3beb96-2c66-11e6-aa54-02010e0a6634\"> "
	                     "<tag>ti</tag> "
	                     "<txt>Jepp</txt> "
	                     "<host>bellmann</host> "
	                     "<cwd>/home/sunny/uuids</cwd> "
	                     "<user>sunny</user> <tty>/dev/pts/7</tty> "
	                     "<sess desc=\"xterm\">"
	                     "8a390a22-2c2e-11e6-8ffb-02010e0a6634"
	                     "</sess> "
	                     "<sess desc=\"logging\">"
	                     "9ad18242-2c2e-11e6-b1f8-02010e0a6634"
	                     "</sess> "
	                     "<sess desc=\"screen\">"
	                     "9c4257a0-2c2e-11e6-b724-02010e0a6634"
	                     "</sess> "
	                     "</suuid>";
#endif
	retval = buf;
	msg(3, "xml_entry() returns '%s'", retval);

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
	else {
		if (!getenv("HOME")) {
			msg(3, "get_logdir(): HOME not found");
			fprintf(stderr, "%s: $%s and $HOME environment "
			                "variables are not defined, cannot "
			                "create logdir path",
			                progname, ENV_LOGDIR);
			return NULL;
		} else {
			int size = strlen(getenv("HOME")) +
			           strlen("/uuids") + 1;
			retval = malloc(size + 1);
			if (!retval) {
				perror("get_logdir(): Cannot allocate "
				       "memory");
				return NULL;
			}
			snprintf(retval, size, "%s/uuids",
			                       getenv("HOME"));
		}
	}
	msg(3, "get_logdir() returns \"%s\"", retval);

	return retval;
}

/*
 * add_to_logfile() - Add the contents of *entry to the XML file fname.
 */

int add_to_logfile(char *fname, struct Entry *entry)
{
	int retval = EXIT_OK;
	FILE *fp;
	char check_line[12];
	long filepos;
	int i;

	/* todo: Add file locking */
	fp = fopen(fname, "r+");
	if (!fp) {
		err(1, "%s: Could not open file for read+write", fname);
		return EXIT_ERROR;
	}
	if (fseek(fp, -10, SEEK_END) == -1) {
		fprintf(stderr, "%s: %s: Could not seek in log file: %s",
		                progname, fname, strerror(errno));
		return EXIT_ERROR;
	}
	filepos = ftell(fp);
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (filepos == -1) {
		err(1, "%s: Cannot read file position", fname);
		return EXIT_ERROR;
	}
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (strcmp(fgets(check_line, 10, fp), "</suuids>")) {
		msg(3, "add_to_logfile(): check_line = '%s'", check_line);
		fprintf(stderr, "%s: %s: Unknown end line, adding to "
		                "end of file\n", progname, fname);
	} else {
		msg(3, "add_to_logfile(): Seems as check_line is ok, "
		       "it is '%s'", check_line);
		if (fseek(fp, filepos, SEEK_SET) == -1) {
			err(1, "%s: Cannot seek to position %lu",
			       fname, filepos);
			return EXIT_ERROR;
		}
	}
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (fputs(xml_entry(entry), fp) < 0) {
		warn("fputs()");
		retval = EXIT_ERROR;
	}
	msg(3, "Before end tag is written");
	fprintf(fp, "\n</suuids>\n");
	fclose(fp);
	msg(3, "add_to_logfile(): fp is closed");
	if (opt.verbose > 2) {
		i = system("(echo; echo; cat /home/sunny/uuids/fake.xml; "
		           "echo; echo) >&2");
		i = i; /* Get rid of gcc warning */
	}
	msg(3, "add_to_logfile() returns %d", retval);

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

	msg(3, "Entering create_logfile(\"%s\")", name);
	if (access(name, F_OK) != -1)
		return name; /* File already exists */
	else {
		FILE *fp;
		fp = fopen(name, "a");
		if (!fp) {
			err(1, "%s: Could not create log file", name);
			return NULL;
		}
		fprintf(fp, "%s\n%s\n<suuids>\n</suuids>\n",
			xml_header, xml_doctype);
		fclose(fp);
	}
	return name;
}

/*
 * valid_xml_chars() - Check that the string pointed to by s contains valid 
 * UTF-8 and no control chars. Return 1 if ok, 0 if invalid.
 */

int valid_xml_chars(char *s)
{
	unsigned char *p = (unsigned char *)s;

	if (utf8_check((unsigned char *)s))
		return 0;
	while (*p) {
		if (*p < ' ' && *p != '\n' && *p != '\r' && *p != '\t')
			return 0;
		if (*p == 127)
			return 0;
		p++;
	}

	return 1;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
