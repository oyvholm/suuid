/*
 * suuid.c
 * File ID: 285082a4-2b93-11e6-90fb-02010e0a6634
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
 * Global variables
 */

char *progname;
struct Options opt;

/*
 * msg() - Print a message prefixed with "[progname]: " to stddebug if 
 * opt.verbose is equal or higher than the first argument. The rest of the 
 * arguments are delivered to vfprintf().
 * Returns the number of characters written, excluding the terminating \n.
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

int myerror(const char *format, ...)
{
	va_list ap;
	int retval = 0;

	va_start(ap, format);
	retval = fprintf(stderr, "%s: ", progname);
	retval += vfprintf(stderr, format, ap);
	retval += fprintf(stderr, ": %s\n", strerror(errno));
	va_end(ap);

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
		                "Returning with value %d.\n",
		                progname, retval);
	else {
		puts("");
		if (opt.verbose >= 1) {
			print_version();
			puts("");
		}
		printf("Usage: %s [options] [file [files [...]]]\n", progname);
		printf("\n");
		printf("Options:\n");
		printf("\n");
		printf("  -c x, --comment x\n"
		       "    Store comment x in the log file.\n");
		printf("  -h, --help\n"
		       "    Show this help.\n");
		printf("  --license\n"
		       "    Print the software license\n");
		printf("  -l x, --logdir x\n"
		       "    Store log files in directory x.\n"
		       "    If the %s environment variable is defined, that "
		       "value is\n"
		       "    used. Otherwise the value \"$HOME/uuids\" is "
		       "used.\n", ENV_LOGDIR);
		printf("  -q, --quiet\n"
		       "    Be more quiet. "
		       "Can be repeated to increase silence.\n");
		printf("  -v, --verbose\n"
		       "    Increase level of verbosity. Can be repeated.\n");
		printf("  --version\n"
		       "    Print version information.\n");
		printf("  -w x, --whereto x\n"
		       "    x is a string which decides where the UUID will "
		       "be written:\n"
		       "      The string contains 'e' - stderr\n"
		       "      The string contains 'o' - stdout\n"
		       "    All other characters will be ignored. Examples:\n"
		       "      e\n"
		       "        Send to stderr.\n"
		       "      eo\n"
		       "        Send to both stdout and stderr.\n"
		       "      a\n"
		       "        Synonym for eo.\n"
		       "      n\n"
		       "        Don’t output anything.\n");
		printf("\n");
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
		dest->comment = strdup(optarg);
		if (!dest->comment) {
			perror("choose_opt_action(): Cannot allocate "
			       "memory for -c/--comment argument");
			retval = EXIT_ERROR;
		}
		break;
	case 'h':
		dest->help = 1;
		break;
	case 'l':
		dest->logdir = strdup(optarg);
		if (!dest->logdir) {
			perror("choose_opt_action(): Cannot allocate "
			       "memory for -l/--logdir argument");
			retval = EXIT_ERROR;
		}
		break;
	case 'q':
		dest->verbose--;
		break;
	case 'v':
		dest->verbose++;
		break;
	case 'w':
		dest->whereto = strdup(optarg);
		if (!dest->whereto) {
			perror("Cannot allocate memory for -w argument");
			retval = EXIT_ERROR;
		}
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
	dest->whereto = NULL;

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
			{"whereto", no_argument, 0, 'w'},
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

		c = getopt_long(argc, argv, "c:hl:qvw:", long_options,
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
	bool did_read_from_stdin = FALSE;

	progname = argv[0];
	progname = "suuid"; /* fixme: Temporary kludge to make it compatible 
	                       with the Perl version. */

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
	if (!logdir) {
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

	if (opt.comment) {
		if (!strcmp(opt.comment, "-")) {
			entry.txt = read_from_fp(stdin);
			if (!entry.txt) {
				fprintf(stderr,
				        "%s: Could not read data from stdin\n",
				        progname);
				return EXIT_ERROR;
			}
			did_read_from_stdin = TRUE;
		} else {
			entry.txt = strdup(opt.comment);
			if (!entry.txt) {
				fprintf(stderr,
				        "%s: Cannot allocate memory for "
				        "comment, strdup() failed: %s\n",
				        progname, strerror(errno));
				return EXIT_ERROR;
			}
		}

		/* fixme: This is how it's done in the Perl version. I'm not 
		 * sure if it's an ok thing to do, even though it looks nice in 
		 * the log files and has worked great for years. Maybe this 
		 * behaviour should be changed when the C version passes all 
		 * tests in suuid.t .
		 */
		trim_str_front(entry.txt);
		trim_str_end(entry.txt);
	}

	if (opt.comment && !valid_xml_chars(entry.txt)) {
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
	if (!logfile) {
		myerror("Could not allocate %lu bytes for logfile filename",
		       fname_length + 1);
		return EXIT_ERROR;
	}
	/* fixme: Remove slash hardcoding */
	snprintf(logfile, fname_length, "%s/%s.xml", logdir, entry.host);
	msg(2, "logfile = \"%s\"", logfile);
	if (!create_logfile(logfile)) {
		fprintf(stderr, "%s: %s: Error when creating log file\n",
		                progname, logfile);
		return EXIT_ERROR;
	}
	if (opt.verbose >= 4) {
		i = system("(echo; echo After create_logfile:; "
		           "cat /home/sunny/uuids/fake.xml; "
		           "echo; echo) >&2");
		i = i; /* Get rid of gcc warning */
	}
	if (add_to_logfile(logfile, &entry) == EXIT_ERROR) {
		fprintf(stderr, "%s: %s: Error when adding entry "
		                "to log file\n", progname, logfile);
		return(EXIT_ERROR);
	}
	if (!opt.whereto)
		puts(entry.uuid);
	else {
		if (strchr(opt.whereto, 'a') || strchr(opt.whereto, 'o'))
			fprintf(stdout, "%s\n", entry.uuid);
		if (strchr(opt.whereto, 'a') || strchr(opt.whereto, 'e'))
			fprintf(stderr, "%s\n", entry.uuid);
	}
	free(logfile);
	if (did_read_from_stdin)
		free(entry.txt);
	free(entry.cwd);

	if (optind < argc) {
		int t;

		for (t = optind; t < argc; t++) {
			msg(2, "Non-option arg: %s", argv[t]);
#if 0
			puts(suuid_xml(argv[t])); /* For testing, remove when 
			                           * suuid_xml() is wonderful.
			                           */
#endif
		}
	}

	msg(2, "Returning from main() with value %d", retval);

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
