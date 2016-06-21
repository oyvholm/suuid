/*
 * logfile.c
 * File ID: 5a6ffd88-3740-11e6-83c5-02010e0a6634
 *
 * (C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "suuid.h"

/*
 * get_logdir() - Return pointer to string with location of the log 
 * directory. Use the value of -l/--logdir if it's defined, otherwise 
 * use the environment variable $SUUID_LOGDIR, otherwise use 
 * "$HOME/uuids". If that also fails, return NULL.
 */

char *get_logdir()
{
	char *retval;
	if (opt.logdir != NULL)
		retval = opt.logdir;
	else if (getenv("SUUID_LOGDIR") != NULL)
		retval = getenv("SUUID_LOGDIR");
	else {
		if (getenv("HOME") == NULL) {
			msg(3, "get_logdir(): HOME not found");
			fprintf(stderr, "%s: $SUUID_LOGDIR and $HOME "
					"environment variables "
					"are not defined, cannot create "
					"logdir path", progname);
			return NULL;
		} else {
			int size = strlen(getenv("HOME")) +
				   strlen("/uuids") + 1;
			retval = malloc(size + 1);
			if (retval == NULL) {
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
	if (fp == NULL)
		err(1, "%s: Could not open file for read+write", fname);
	fseek(fp, -10, SEEK_END);
	filepos = ftell(fp);
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (filepos == -1)
		err(1, "%s: Cannot read file position", fname);
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (strcmp(fgets(check_line, 10, fp), "</suuids>")) {
		msg(3, "add_to_logfile(): check_line = '%s'", check_line);
		fprintf(stderr, "%s: %s: Unknown end line, adding to "
				"end of file\n", progname, fname);
	} else {
		msg(3, "add_to_logfile(): Seems as check_line is ok, "
		       "it is '%s'", check_line);
		if (fseek(fp, filepos, SEEK_SET) == -1)
			err(1, "%s: Cannot seek to position %lu",
				fname, filepos);
	}
	msg(3, "ftell(fp) at line %u is %lu", __LINE__, ftell(fp));
	if (fputs(xml_entry(entry), fp) <= 0) {
		warn("fputs()");
		retval = -1;
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
 * create_logfile() - Create logfile with initial XML structure if it 
 * doesn't exist already.
 */

void create_logfile(char *name)
{
	char *xml_header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char *xml_doctype = "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">";

	msg(3, "Entering create_logfile(\"%s\")", name);
	if (access(name, F_OK) != -1)
		return; /* File already exists */
	else {
		FILE *fp;
		fp = fopen(name, "a");
		if (fp == NULL)
			err(1, "%s: Could not create log file", name);
		fprintf(fp, "%s\n%s\n<suuids>\n</suuids>\n",
			xml_header, xml_doctype);
		fclose(fp);
	}
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
