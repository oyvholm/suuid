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
 */

#include "suuid.h"
#include "version.h"

#define LOGDIR_MAXLEN 4096

/*
 * Global variables
 */

char *progname;
struct Options {
	int help;
	int license;
	char *logdir;
	int verbose;
	int version;
} opt;

/*
 * msg() - Print a message prefixed with "[progname]: " to stddebug if 
 * opt.verbose is equal or higher than the first argument. The rest of 
 * the arguments are delivered to vfprintf().
 * Returns the number of characters written.
 */

int msg(int verbose, const char *format, ...)
{
	va_list ap;
	int retval = 0;
	if (opt.verbose >= verbose) {
		va_start(ap, format);
		retval = fprintf(stddebug, "%s: ", progname);
		retval += vfprintf(stddebug, format, ap);
		va_end(ap);
	}
	return retval;
}

/*
 * print_license() - Display the program license
 */

void print_license(void)
{
	puts("(C)opyleft STDyearDTS- Øyvind A. Holm <sunny@sunbase.org>");
	puts("");
	puts("This program is free software; you can redistribute it "
	     "and/or modify it \n"
	     "under the terms of the GNU General Public License as "
	     "published by the \n"
	     "Free Software Foundation; either version 2 of the License, "
	     "or (at your \n"
	     "option) any later version.");
	puts("");
	puts("This program is distributed in the hope that it will be "
	     "useful, but \n"
	     "WITHOUT ANY WARRANTY; without even the implied warranty of \n"
	     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
	puts("See the GNU General Public License for more details.");
	puts("");
	puts("You should have received a copy of "
	     "the GNU General Public License along \n"
	     "with this program. If not, see "
	     "<http://www.gnu.org/licenses/>.");
}

/*
 * print_version() - Print version information on stdout
 */

void print_version(void)
{
	printf("%s %s\n", progname, VERSION);
}

/*
 * usage() - Prints a help screen
 */

void usage(int retval)
{
	if (retval != EXIT_OK)
		fprintf(stderr, "\nType \"%s --help\" for help screen. "
			"Returning with value %d.\n", progname, retval);
	else {
		puts("");
		if (opt.verbose >= 1) {
			print_version();
			puts("");
		}
		printf("Usage: %s [options] [file [files [...]]]\n", progname);
		puts("");
		puts("Options:");
		puts("");
		puts("  -h, --help\n"
		     "    Show this help.");
		puts("  --license\n"
		     "    Print the software license");
		puts("  -l x, --logdir x\n"
		     "    Store log files in directory x.\n"
		     "    If the SUUID_LOGDIR environment variable is "
		     "defined, that value is\n"
		     "    used. Otherwise the value \"$HOME/uuids\" is "
		     "used.\n"
		     "    Current default: /home/sunny/uuids");
		puts("  -q, --quiet\n"
		     "    Be more quiet. "
		     "Can be repeated to increase silence.");
		puts("  -v, --verbose\n"
		     "    Increase level of verbosity. Can be repeated.");
		puts("  --version\n"
		     "    Print version information.");
		puts("");
	}
}

/*
 * choose_opt_action() - Decide what to do when option c is found. Store 
 * changes in dest. opts is the struct with the definitions for the long 
 * options.
 * Return EXIT_OK if ok, EXIT_ERROR if c is unknown or anything fails.
 */

int choose_opt_action(struct Options *dest, int c, struct option *opts)
{
	int retval = EXIT_OK;

	switch (c) {
	case 0:
		if (!strcmp(opts->name, "license")) {
			dest->license = 1;
		} else if (!strcmp(opts->name, "version")) {
			dest->version = 1;
		}
		break;
	case 'h':
		dest->help = 1;
		break;
	case 'l':
		dest->logdir = malloc(strlen(optarg) + 1);
		if (dest->logdir == NULL) {
			perror("choose_opt_action(): malloc() error");
			retval = EXIT_ERROR;
		} else {
			strncpy(dest->logdir, optarg, strlen(optarg));
		}
		break;
	case 'q':
		dest->verbose--;
		break;
	case 'v':
		dest->verbose++;
		break;
	default:
		msg(2, "getopt_long() returned "
		       "character code %d\n", c);
		retval = EXIT_ERROR;
		break;
	}

	return retval;
}

/*
 * parse_options() - Parse command line options.
 * Returns EXIT_OK if ok, EXIT_ERROR if error.
 */

int parse_options(struct Options *dest, int argc, char *argv[])
{
	int retval = EXIT_OK;
	int c;

	dest->help = 0;
	dest->license = 0;
	dest->verbose = 0;
	dest->version = 0;

	while (retval == EXIT_OK) {
		int option_index = 0;
		static struct option long_options[] = {
			{"help", no_argument, 0, 'h'},
			{"license", no_argument, 0, 0},
			{"logdir", required_argument, 0, 'l'},
			{"quiet", no_argument, 0, 'q'},
			{"verbose", no_argument, 0, 'v'},
			{"version", no_argument, 0, 0},
			{0, 0, 0, 0}
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

		c = getopt_long(argc, argv, "hl:qv", long_options,
				&option_index);

		if (c == -1)
			break;

		retval = choose_opt_action(dest,
					   c, &long_options[option_index]);
	}

	msg(3, "parse_options() returns %d\n", retval);
	return retval;
}

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
}

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
}

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
}

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
}

char *uuid_date(char *uuid)
{
	/* fixme */
	static char retval[32] = "2000-01-01T00:00:00.0000000Z";
	return(retval);
}

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
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_OK;
	char opt_logdir[LOGDIR_MAXLEN + 1];
	char *logfile;
	struct Entry entry;
	size_t fname_length; /* Total length of logfile name */

	progname = argv[0];

	retval = parse_options(&opt, argc, argv);
	msg(3, "retval after parse_options(): %d\n", retval);
	if (retval != EXIT_OK) {
		fprintf(stderr, "%s: Option error\n", progname);
		return EXIT_ERROR;
	}

	msg(2, "Using verbose level %d\n", opt.verbose);

	if (opt.help) {
		usage(EXIT_OK);
		return EXIT_OK;
	}

	if (opt.version) {
		print_version();
		return EXIT_OK;
	}

	if (opt.license) {
		print_license();
		return EXIT_OK;
	}

	strncpy(opt_logdir, getenv("SUUID_LOGDIR"), LOGDIR_MAXLEN);

	if (strlen(opt.logdir)) {
		strncpy(opt_logdir, optarg, LOGDIR_MAXLEN);
		msg(2, "opt_logdir = \"%s\"\n", opt_logdir);
	}

	entry.host = get_hostname();
	msg(2, "entry.host = \"%s\"\n", entry.host);

	entry.cwd = getpath();
	if (entry.cwd == NULL)
		err(0, "Could not get current directory");
	msg(2, "entry.cwd = \"%s\"\n", entry.cwd);

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
	msg(2, "logfile = \"%s\"\n", logfile);
	create_logfile(logfile);
	printf("%s\n", generate_uuid());
	free(logfile);
	free(entry.cwd);

	if (optind < argc) {
		int t;

		for (t = optind; t < argc; t++)
			msg(2, "Non-option arg: %s\n", argv[t]);
	}

	msg(2, "Returning from main() with value %d\n", retval);
	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
