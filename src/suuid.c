
/*
 * suuid.c
 * File ID: 285082a4-2b93-11e6-90fb-02010e0a6634
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
 *
 */

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "suuid.h"
#include "version.h"

#define LOGDIR_MAXLEN 4096

/*
 * Function prototypes
 */

struct Entry {
	char *date;
	char *uuid;
	char *txt;
	char *host;
	char *cwd;
	char *user;
};

int add_to_logfile(char *, struct Entry *);
void create_logfile(char *);
char *generate_uuid(void);
char *get_hostname(void);
char *getpath(void);
void print_license(void);
void print_version(void);
void usage(int);
char *uuid_date(char *);
char *xml_entry(struct Entry);

/*
 * Global variables
 */

char *progname;
int  debug = 0;

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int c;
	int retval = EXIT_OK;
	char opt_logdir[LOGDIR_MAXLEN + 1];
	char *logfile;
	struct Entry entry;
	size_t fname_length; /* Total length of logfile name */

	progname = argv[0];

	strncpy(opt_logdir, getenv("SUUID_LOGDIR"), LOGDIR_MAXLEN);

	while (1) {
		int option_index = 0;
		static struct option long_options[] = {
			{  "debug", 0, 0,   0},
			{   "help", 0, 0, 'h'},
			{"license", 0, 0,   0},
			{ "logdir", 1, 0, 'l'},
			{"version", 0, 0, 'V'},
			{        0, 0, 0,   0}
		};

		/*
		 * long_options:
		 *
		 * 1. const char  *name;
		 * 2. int         has_arg;
		 * 3. int         *flag;
		 * 4. int         val;
		 *
		 */

		c = getopt_long (argc, argv, "hl:V",
			long_options, &option_index);

		if (c == -1)
			break;

		switch (c) {
		case 0 :
			if (!strcmp(long_options[option_index].name,
			    "debug"))
				debug = 1;

			else if (!strcmp(
				 long_options[option_index].name,
				 "license")
			) {
				print_license();
				return(EXIT_OK);
			}

#if 1
			fprintf(stddebug, "option %s",
				long_options[option_index].name);
			if (optarg)
				fprintf(stddebug, " with arg %s",
					optarg);
			fprintf(stddebug, "\n");
#endif /* if 0 */
			break;
		case 'h' :
			usage(EXIT_OK);
			break;
		case 'l' :
			strncpy(opt_logdir, optarg, LOGDIR_MAXLEN);
			break;
		case 'V' :
			print_version();
			return(EXIT_OK);
		default :
			debpr1("getopt_long() returned "
			       "character code %d\n", c);
			break;
		}
	}

	debpr1("debugging is set to level %d\n", debug);
	debpr1("opt_logdir = \"%s\"\n", opt_logdir);
	entry.host = get_hostname();
	debpr1("entry.host = \"%s\"\n", entry.host);
	entry.cwd = getpath();
	if (entry.cwd == NULL)
		err(0, "Could not get current directory");
	debpr1("entry.cwd = \"%s\"\n", entry.cwd);

	if (debug && optind < argc) {
		int t;

		debpr0("non-option args: ");
		for (t = optind; t < argc; t++)
			fprintf(stddebug, "%s ", argv[t]);

		fprintf(stddebug, "\n");
	}

	fname_length = strlen(opt_logdir) +
		       strlen("/") +
		       strlen(entry.host) +
		       strlen(".xml") +
		       1;
	logfile = malloc(fname_length + 1);
	if (logfile == NULL)
		err(1, "Could not allocate %lu bytes for logfile filename",
			fname_length + 1);
	/* fixme: Remove slash hardcoding */
	snprintf(logfile, fname_length, "%s/%s.xml", opt_logdir, entry.host);
	debpr1("logfile = \"%s\"\n", logfile);
	create_logfile(logfile);
	printf("%s\n", generate_uuid());
	free(logfile);
	free(entry.cwd);

	/*
	if (optind < argc) {
		int  t;

		for (t = optind; t < argc; t++)
			retval |= process_file(argv[t]);
	} else
		retval |= process_file("-");
	*/

	debpr1("Returning from main() with value %d\n", retval);

	return(retval);
} /* main() */

/*
 * print_license() - Display the program license
 */

void print_license(void)
{
	fprintf(stdout,
		"(C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>\n"
		"\n"
		"This program is free software; you can redistribute it "
		"and/or modify it \n"
		"under the terms of the GNU General Public License as "
		"published by the \n"
		"Free Software Foundation; either version 2 of the License, "
		"or (at your \n"
		"option) any later version.\n"
		"\n"
		"This program is distributed in the hope that it will be "
		"useful, but \n"
		"WITHOUT ANY WARRANTY; without even the implied warranty of \n"
		"MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  "
		"See the GNU \n"
		"General Public License for more details.\n"
		"\n"
		"You should have received a copy of "
		"the GNU General Public License along \n"
		"with this program; if not, write to "
		"the Free Software Foundation, Inc., \n"
		"59 Temple Place, Suite 330, Boston, MA  02111-1307  USA\n"
	);
} /* print_license() */

/*
 * print_version() - Print version information on stdout
 */

void  print_version(void)
{
	fprintf(stdout, "%s %s\n", progname, VERSION);
} /* print_version() */

/*
 * usage() - Prints a help screen
 */

void usage(int retval)
{
	if (retval != EXIT_OK)
		fprintf(stderr, "\nType \"%s --help\" for help screen. "
			"Returning with value %d.\n", progname, retval);
	else {
		fprintf(stdout, "\n");
		print_version();
		fprintf(stdout,
			"Usage: %s [options] [file [files [...]]]\n"
			"\n"
			"Options:\n"
			"\n"
			"  -h, --help\n"
			"    Show this help.\n"
			"  --license\n"
			"    Print the software license\n"
			"  -l x, --logdir x\n"
			"    Store log files in directory x.\n"
			"    If the SUUID_LOGDIR environment variable is "
			"defined, that value is\n"
			"    used. Otherwise the value \"$HOME/uuids\" is "
			"used.\n"
			"    Current default: /home/sunny/uuids\n"
			"  -v, --verbose\n"
			"    Increase level of verbosity. Can be repeated.\n"
			"  --version\n"
			"    Print version information.\n"
			"  --debug\n"
			"    Print debugging messages.\n"
			"\n", progname
		);
	}

	exit(retval);
} /* usage() */

int add_to_logfile(char *fname, struct Entry *entry)
{
	int retval = 0;
	FILE *fp;
	char check_line[12];
	long filepos;
	/* todo: Add file locking */
	fp = fopen(fname, "r+");
	if (fp == NULL)
		err(1, "%s: Could not open file for read+write", fname);
	fseek(fp, -10, SEEK_END);
	filepos = ftell(fp);
	if (filepos == -1)
		err(1, "%s: Cannot read file position", fname);
	if (strcmp(fgets(check_line, 10, fp), "</suuid>\n")) {
		fprintf(stderr, "%s: %s: Unknown end line, adding to "
				"end of file\n", progname, fname);
	} else {
		if (fseek(fp, 0, SEEK_SET) == -1)
			err(1, "%s: Cannot seek to position %lu",
				fname, filepos);
	}
	if (fputs(xml_entry(*entry), fp) <= 0) {
		warn("fputs()");
		retval = -1;
	}
	fclose(fp);
	return(retval);
} /* add_to_logfile() */

char *generate_uuid(void)
{
	static char uuid[38];
	FILE *fp;
	/* FIXME: Generate it properly */
	fp = popen("/usr/bin/uuid", "r");
	if (fp == NULL) {
		fprintf(stderr, "%s: Could not exec /usr/bin/uuid", progname);
		return(NULL);
	}
	if (fgets(uuid, 37, fp) == NULL)
		fprintf(stderr, "%s: fgets() error", progname);
	uuid[36] = '\0';
	pclose(fp);
	return(uuid);
} /* generate_uuid() */

char *get_hostname(void)
{
	static char buf[256];
	char *retval = buf;
	if (gethostname(buf, 255) == -1) {
		perror("Could not get hostname");
		return(NULL);
	}
#if FAKE_HOST
	retval = "fake"; /* Use "fake" as hostname to avoid conflicts
			    with files created by the Perl version */
#endif
	return(retval);
} /* get_hostname() */

char *getpath(void)
{
	char *retval;
	char *p;
	size_t blksize = 1024;
	size_t size = blksize;
	retval = malloc(size);
	if (retval == NULL) {
		perror("getpath(): malloc() fail");
		return NULL;
	}
	for (p = getcwd(retval, size); p == NULL; ) {
		size += blksize;
		retval = realloc(retval, size);
		if (retval == NULL) {
			perror("getpath(): realloc() fail");
			return NULL;
		}
		p = getcwd(retval, size);
		if (p == NULL && errno != ERANGE) {
			/* Avoid infinite loop in case there's another 
			 * getcwd() problem that's not fixable by just 
			 * allocating more memory.
			 */
			perror("getcwd()");
			free(retval);
			return(NULL);
		}
	}
	return retval;
}

void create_logfile(char *name)
{
	char *xml_header = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
	char *xml_doctype = "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">";

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
} /* create_logfile() */

char *uuid_date(char *uuid)
{
	/* fixme */
	static char retval[32] = "2000-01-01T00:00:00.0000000Z";
	return(retval);
} /* uuid_date() */

char *xml_entry(struct Entry entry)
{
	/* fixme */
	static char retval[] = "<suuid t=\"2016-06-07T04:18:40.9460630Z\" "
		"u=\"ea3beb96-2c66-11e6-aa54-02010e0a6634\"> "
		"<tag>ti</tag> "
		"<txt>Jepp</txt> "
		"<host>bellmann</host> "
		"<cwd>/home/sunny/uuids</cwd> "
		"<user>sunny</user> <tty>/dev/pts/7</tty> "
		"<sess desc=\"xterm\">8a390a22-2c2e-11e6-8ffb-02010e0a6634"
		"</sess> "
		"<sess desc=\"logging\">9ad18242-2c2e-11e6-b1f8-02010e0a6634"
		"</sess> "
		"<sess desc=\"screen\">9c4257a0-2c2e-11e6-b724-02010e0a6634"
		"</sess> "
		"</suuid>";
	return(retval);
} /* xml_entry() */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
