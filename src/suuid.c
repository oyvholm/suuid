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

/*
 * Global variables
 */

char *progname;
struct Options opt;

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
		fputc('\n', stddebug);
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
		puts("  -c x, --comment x\n"
		     "    Store comment x in the log file.");
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
	case 'c':
		dest->comment = malloc(strlen(optarg));
		if (dest->comment == NULL) {
			perror("choose_opt_action(): Cannot allocate "
			       "memory for -c/--comment argument");
			retval = EXIT_ERROR;
		} else
			strncpy(dest->comment, optarg, strlen(optarg));
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
		       "character code %d", c);
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

	dest->comment = NULL;
	dest->help = 0;
	dest->license = 0;
	dest->logdir = NULL;
	dest->verbose = 0;
	dest->version = 0;

	while (retval == EXIT_OK) {
		int option_index = 0;
		static struct option long_options[] = {
			{"comment", required_argument, 0, 'c'},
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

		c = getopt_long(argc, argv, "c:hl:qv", long_options,
				&option_index);

		if (c == -1)
			break;

		retval = choose_opt_action(dest,
					   c, &long_options[option_index]);
	}

	msg(3, "parse_options() returns %d", retval);
	return retval;
}

/*
 * get_hostname()
 */

char *get_hostname(void)
{
	static char buf[256];
	char *retval = buf;
	if (gethostname(buf, 255) == -1)
		return NULL;
#if FAKE_HOST
	retval = "fake"; /* Use "fake" as hostname to avoid conflicts
			    with files created by the Perl version */
#endif
	msg(3, "get_hostname() returns '%s'", retval);
	return retval;
}

/*
 * getpath()
 */

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
			return NULL;
		}
	}
	return retval;
}

/*
 * get_username() - Return pointer to string with login name.
 */

char *get_username(void)
{
	char *retval;
	struct passwd *pw;
	pw = getpwuid(getuid());
	if (pw == NULL)
		retval = NULL;
	else
		retval = pw->pw_name;
	msg(3, "get_username() returns \"%s\"", retval);
	return retval;
}

/*
 * get_tty() - Return pointer to string with name of current tty.
 */

char *get_tty(void)
{
	char *retval;

	retval = ttyname(STDIN_FILENO);
	msg(3, "get_tty() returns \"%s\"", retval);
	return retval;
}

/*
 * init_xml_entry() - Initialise Entry struct at memory position e with 
 * initial values.
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
	msg(3, "Entering allocate_entry(\"%s\", \"%s\")", elem, src);
	if (elem != NULL && src != NULL) {
		size += strlen("<") + strlen(elem) + strlen(">") +
			strlen(src) +
			strlen("<") + strlen(elem) + strlen("/> ") + 1;
		msg(3, "allocate_entry(): size = %lu", size);
		retval = malloc(size + 1);
		if (retval == NULL)
			perror("allocate_entry(): Cannot allocate memory");
		else
			snprintf(retval, size, "<%s>%s</%s> ",
					       elem, src, elem);
	} else
		retval = NULL;
	msg(3, "allocate_entry() returns '%s'", retval);
	return retval;
}

/*
 * alloc_attr() - Return pointer to string with XML attribute. Returns 
 * NULL on error.
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
	if (retval == NULL)
		perror("alloc_attr(): Cannot allocate memory");
	else
		snprintf(retval, size, " %s=\"%s\"", attr, data);
	msg(3, "alloc_attr() returns \"%s\"", retval);
	return retval;
}

/*
 * xml_entry() - Return pointer to string with one XML entry extracted 
 * from the entry struct, or NULL if error.
 */

char *xml_entry(struct Entry *entry)
{
	static char buf[65536]; /* fixme: Temporary */
	struct Entry e;
	char *retval;

	msg(3, "Entering xml_entry()");
	init_xml_entry(&e);
	msg(3, "xml_entry(): After init_xml_entry()");

	msg(3, "xml_entry(): entry->date = '%s'", entry->date);
	msg(3, "xml_entry(): entry->uuid = '%s'", entry->uuid);
	msg(3, "xml_entry(): entry->tag = '%s'",  entry->tag);
	msg(3, "xml_entry(): entry->txt = '%s'",  entry->txt);
	msg(3, "xml_entry(): entry->host = '%s'", entry->host);
	msg(3, "xml_entry(): entry->cwd = '%s'",  entry->cwd);
	msg(3, "xml_entry(): entry->user = '%s'", entry->user);
	msg(3, "xml_entry(): entry->sess = '%s'", entry->sess);

	if (entry->uuid == NULL) {
		msg(2, "xml_entry(): uuid is NULL");
		return NULL;
	} else
		e.uuid = alloc_attr("u", entry->uuid);

	if (entry->date != NULL)
		e.date = alloc_attr("t", entry->date);

	e.txt = allocate_entry("txt", entry->txt);
	e.host = allocate_entry("host", entry->host);
	e.cwd = allocate_entry("cwd", entry->cwd);
	e.user = allocate_entry("user", entry->user);
	e.tty = allocate_entry("tty", entry->tty);

	snprintf(buf, 65535, /* fixme: length */
		"<suuid%s%s> " /* date, uuid */
			"%s" /* tag */
			"%s" /* txt */
			"%s" /* host */
			"%s" /* cwd */
			"%s" /* user */
			"%s" /* tty */
			"%s" /* sess */
		"</suuid>",
		(e.date == NULL) ? "" : e.date,
		(e.uuid == NULL) ? "" : e.uuid,
		(e.tag == NULL) ? "" : e.tag,
		(e.txt == NULL) ? "" : e.txt,
		(e.host == NULL) ? "" : e.host,
		(e.cwd == NULL) ? "" : e.cwd,
		(e.user == NULL) ? "" : e.user,
		(e.tty == NULL) ? "" : e.tty,
		(e.sess == NULL) ? "" : e.sess);
	msg(3, "xml_entry(): After snprintf()");
#if 0
	static char fake[] = "<suuid t=\"2016-06-07T04:18:40.9460630Z\" "
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
#endif
	retval = buf;
	msg(3, "xml_entry() returns '%s'", retval);
	return retval;
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_OK;
	char *logdir;
	char *logfile;
	struct Entry entry;
	size_t fname_length; /* Total length of logfile name */
	int i;

	progname = argv[0];
	progname = "suuid"; /* fixme: Temporary kludge to make it 
			       compatible with the Perl version. */

	retval = parse_options(&opt, argc, argv);
	msg(3, "retval after parse_options(): %d", retval);
	if (retval != EXIT_OK) {
		fprintf(stderr, "%s: Option error\n", progname);
		return EXIT_ERROR;
	}

	msg(2, "Using verbose level %d", opt.verbose);

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

	logdir = get_logdir(&opt);
	msg(3, "logdir = '%s'", logdir);
	if (logdir == NULL) {
		fprintf(stderr, "%s: Unable to find logdir location\n",
				progname);
		return(EXIT_ERROR);
	}

	init_xml_entry(&entry);
	entry.uuid = generate_uuid();
	if (!valid_uuid(entry.uuid)) {
		fprintf(stderr, "%s: Got invalid UUID: \"%s\"",
				progname, entry.uuid);
		return(EXIT_ERROR);
	}
	entry.date = uuid_date(entry.uuid);
	entry.host = get_hostname();
	msg(2, "entry.host = \"%s\"", entry.host);

	entry.cwd = getpath();
	msg(2, "entry.cwd = \"%s\"", entry.cwd);

	entry.user = get_username();
	entry.tty = get_tty();

	if (opt.comment != NULL)
		entry.txt = opt.comment;

	if (opt.comment != NULL && !valid_xml_chars(entry.txt)) {
		fprintf(stderr, "%s: Comment contains illegal characters or "
				"is not valid UTF-8\n", progname);
		return EXIT_ERROR;
	}
	msg(3, "After utf8_check()");

	fname_length = strlen(logdir) +
		       strlen("/") +
		       strlen(entry.host) +
		       strlen(".xml") +
		       1;
	logfile = malloc(fname_length + 1);
	if (logfile == NULL)
		err(1, "Could not allocate %lu bytes for logfile filename",
			fname_length + 1);
	/* fixme: Remove slash hardcoding */
	snprintf(logfile, fname_length, "%s/%s.xml", logdir, entry.host);
	msg(2, "logfile = \"%s\"", logfile);
	create_logfile(logfile);
	if (opt.verbose > 2) {
		i = system("(echo; echo After create_logfile:; "
			   "cat /home/sunny/uuids/fake.xml; "
			   "echo; echo) >&2");
		i = i; /* Get rid of gcc warning */
	}
	add_to_logfile(logfile, &entry);
	puts(entry.uuid);
	free(logfile);
	free(entry.cwd);

	if (optind < argc) {
		int t;

		for (t = optind; t < argc; t++)
			msg(2, "Non-option arg: %s", argv[t]);
	}

	msg(2, "Returning from main() with value %d", retval);
	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
