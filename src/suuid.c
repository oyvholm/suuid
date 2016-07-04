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
struct Rc rc;
struct Entry entry;
#if PERL_COMPAT
bool perlexit13 = FALSE; /* If it is set to TRUE, the program exits with 13 */
#endif

/*
 * msg() - Print a message prefixed with "[progname]: " to stddebug if 
 * opt.verbose is equal or higher than the first argument. The rest of the 
 * arguments are delivered to vfprintf().
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
		retval += fprintf(stddebug, "\n");
		va_end(ap);
	}

	return retval;
}

/*
 * myerror() - Print an error message to stderr using this format:
 *   a: b: c
 * where a is the name of the program (progname), b is the output from the 
 * printf-like string and optional arguments, and c is the error message from 
 * errno. Returns the number of characters written.
 */

int myerror(const char *format, ...)
{
	va_list ap;
	int retval = 0;
	int orig_errno = errno;

	retval = fprintf(stderr, "%s: ", progname);
	va_start(ap, format);
	retval += vfprintf(stderr, format, ap);
	va_end(ap);
	if (errno)
		retval += fprintf(stderr, ": %s", strerror(orig_errno));
	retval += fprintf(stderr, "\n");

	return retval;
}

/*
 * print_license() - Display the program license
 */

void print_license(void)
{
	puts("(C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>");
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
	if (retval != EXIT_OK) {
		fprintf(stderr, "\nType \"%s --help\" for help screen. "
		                "Returning with value %d.\n",
		                progname, retval);
		return;
	}

	puts("");
	if (opt.verbose >= 1) {
		print_version();
		puts("");
	}
	printf("Usage: suuid [options]\n");
	printf("\n");
	printf("Generates one or more UUIDs and stores it to a log file with "
	       "optional \n"
	       "comment or tag/category.\n");
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("  -c x, --comment x\n"
	       "    Store comment x in the log file. If \"-\" is specified as "
	       "comment, the \n"
	       "    program will read the comment from stdin. Two hyphens "
	       "(\"--\") as a \n"
	       "    comment opens the editor defined in the environment "
	       "variable \n"
	       "    %s to edit the message. If %s is "
	       "not defined, \n"
	       "    EDITOR is read, if not defined, \"%s\" is called, it "
	       "should exist \n"
	       "    everywhere. It may not, but it should.\n",
	       ENV_EDITOR, ENV_EDITOR, STD_EDITOR);
	printf("  -h, --help\n"
	       "    Show this help.\n");
	printf("  -l x, --logdir x\n"
	       "    Store log files in directory x.\n"
	       "    If the %s environment variable is defined, "
	       "that value is \n"
	       "    used. Otherwise the value \"$HOME/uuids\" is used.\n"
	       "    Current default: %s\n", ENV_LOGDIR, get_logdir());
	printf("  -m, --random-mac\n"
	       "    Don’t use the hardware MAC address, generate a random "
	       "address field.\n");
	printf("  -n x, --count x\n"
	       "    Print and store x UUIDs.\n");
	printf("  -q, --quiet\n"
	       "    Be more quiet. Can be repeated to increase silence.\n");
	printf("  --raw\n"
	       "    Don’t convert <txt> element to XML. When using this "
	       "option, it is \n"
	       "    expected that the value of the -c/--comment option is "
	       "valid XML, \n"
	       "    otherwise it will create corrupted log files.\n");
	printf("  --rcfile X\n"
	       "    Use file X instead of '%s/.suuidrc'.\n", getenv("HOME"));
	printf("  -t x, --tag x\n"
	       "    Use x as tag (category).\n");
	printf("  -v, --verbose\n"
	       "    Increase level of verbosity. Can be repeated.\n");
	printf("  --version\n"
	       "    Print version information.\n");
	printf("  -w x, --whereto x\n"
	       "    x is a string which decides where the UUID will be "
	       "written:\n"
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
	       "        Don’t output anything.\n"
	       "    Default: \"o\"\n");
	printf("\n");
	printf("If the %s environment variable is defined by "
	       "sess(1) or another \n"
	       "program, the value is logged if it is an UUID.\n", ENV_SESS);
	printf("\n");
	printf("A different hostname can be specified in the environment "
	       "variable \n"
	       "%s, or in the file %s/.suuidrc with "
	       "the format \n"
	       "\"hostname = xxx\".\n", ENV_HOSTNAME, getenv("HOME"));
	printf("\n");
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
		if (!strcmp(opts->name, "license"))
			dest->license = TRUE;
		else if (!strcmp(opts->name, "raw"))
			dest->raw = TRUE;
		else if (!strcmp(opts->name, "rcfile")) {
			dest->rcfile = strdup(optarg);
			if (!dest->rcfile) {
				myerror("choose_opt_action(): Cannot allocate "
				        "memory for --rcfile argument");
				retval = EXIT_ERROR;
			}
		} else if (!strcmp(opts->name, "version"))
			dest->version = TRUE;
		break;
	case 'c':
		dest->comment = strdup(optarg);
		if (!dest->comment) {
			myerror("choose_opt_action(): Cannot allocate memory "
			        "for -c/--comment argument");
			retval = EXIT_ERROR;
		}
		break;
	case 'h':
		dest->help = TRUE;
		break;
	case 'l':
		dest->logdir = strdup(optarg);
		if (!dest->logdir) {
			myerror("choose_opt_action(): Cannot allocate memory "
			        "for -l/--logdir argument");
			retval = EXIT_ERROR;
		}
		break;
	case 'm':
		dest->random_mac = TRUE;
		break;
	case 'n':
		if (!sscanf(optarg, "%u", &dest->count)) {
			myerror("Error in -n/--count argument");
			retval = EXIT_ERROR;
		}
		break;
	case 'q':
		dest->verbose--;
		break;
	case 't':
		if (!store_tag(optarg))
			return EXIT_ERROR;
		break;
	case 'v':
		dest->verbose++;
		break;
	case 'w':
		dest->whereto = strdup(optarg);
		if (!dest->whereto) {
			myerror("choose_opt_action(): Cannot allocate memory "
			        "for -w/--whereto argument");
			retval = EXIT_ERROR;
		}
		break;
	default:
		msg(3, "getopt_long() returned character code %d", c);
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
	dest->count = 1;
	dest->help = FALSE;
	dest->license = FALSE;
	dest->logdir = NULL;
	dest->random_mac = FALSE;
	dest->raw = FALSE;
	dest->rcfile = NULL;
	dest->verbose = 0;
	dest->version = FALSE;
	dest->whereto = NULL;

	while (retval == EXIT_OK) {
		int option_index = 0;
		static struct option long_options[] = {
			{"comment", required_argument, 0, 'c'},
			{"count", required_argument, 0, 'n'},
			{"help", no_argument, 0, 'h'},
			{"license", no_argument, 0, 0},
			{"logdir", required_argument, 0, 'l'},
			{"quiet", no_argument, 0, 'q'},
			{"random-mac", no_argument, 0, 'm'},
			{"raw", no_argument, 0, 0},
			{"rcfile", required_argument, 0, 0},
			{"tag", required_argument, 0, 't'},
			{"verbose", no_argument, 0, 'v'},
			{"version", no_argument, 0, 0},
			{"whereto", no_argument, 0, 'w'},
			{0, 0, 0, 0}
		};

		c = getopt_long(argc, argv,
		                "c:" /* --comment */
		                "h"  /* --help */
		                "l:" /* --logdir */
		                "m"  /* --random-mac */
		                "n:" /* --count */
		                "q"  /* --quiet */
		                "t:" /* --tag */
		                "v"  /* --verbose */
		                "w:" /* --whereto */
		                , long_options, &option_index);

		if (c == -1)
			break;

		retval = choose_opt_action(dest,
		                           c, &long_options[option_index]);
	}

	return retval;
}

/*
 * fill_entry_struct() - Fill the entry struct with information from the opt 
 * struct and the environment, like current directory, hostname, comment, etc.
 * Returns EXIT_OK if no errors, EXIT_ERROR if errors.
 */

int fill_entry_struct(struct Entry *entry, struct Options *opt)
{
	entry->host = get_hostname();
	entry->cwd = getpath();
	entry->user = get_username();
	entry->tty = get_tty();

	if (opt->comment) {
		if (!strcmp(opt->comment, "-")) {
			entry->txt = read_from_fp(stdin);
			if (!entry->txt) {
				myerror("Could not read data from stdin");
				return EXIT_ERROR;
			}
		} else if (!strcmp(opt->comment, "--")) {
			char *e;

			e = get_editor();
			if (!e) {
				myerror("get_editor() failed");
				return EXIT_ERROR;
			}
			entry->txt = read_from_editor(e);
			if (!entry->txt) {
				myerror("Could not read data from editor "
				        "\"%s\"", e);
				return EXIT_ERROR;
			}
			free(e);
		} else {
			entry->txt = strdup(opt->comment);
			if (!entry->txt) {
				myerror("%s: Cannot allocate memory for "
				        "comment, strdup() failed");
				return EXIT_ERROR;
			}
		}
		if (!valid_xml_chars(entry->txt)) {
			fprintf(stderr, "%s: Comment contains illegal "
			                "characters or is not valid UTF-8\n",
			                progname);
			free(entry->txt);
			entry->txt = NULL;
			return EXIT_ERROR;
		}

		/* fixme: This is how it's done in the Perl version. I'm not 
		 * sure if it's an ok thing to do, even though it looks nice in 
		 * the log files and has worked great for years. Maybe this 
		 * behaviour should be changed when the C version passes all 
		 * tests in suuid.t .
		 */
		trim_str_front(entry->txt);
		trim_str_end(entry->txt);
	}
	if (get_sess_info(entry) == EXIT_ERROR) {
		myerror("fill_entry_struct(): get_sess_info() failed");
		free(entry->txt);
		return EXIT_ERROR;
	}

	return EXIT_OK;
}

/*
 * process_uuid() - Generate UUID and write it to the log file.
 * If no errors, send it to stdout and/or stderr and return a pointer to the 
 * UUID. Otherwise return NULL.
 */

char *process_uuid(FILE *logfp, struct Entry *entry)
{
	entry->uuid = generate_uuid();
	if (!valid_uuid(entry->uuid, TRUE)) {
		fprintf(stderr, "%s: '%s': Generated UUID is not in the "
		                "expected format\n",
		                progname, entry->uuid);
		return NULL;
	}
	entry->date = malloc(DATE_LENGTH + 1);
	if (!entry->date) {
		myerror("process_uuid(): Could not allocate %lu bytes for "
		        "date string", DATE_LENGTH + 1);
		return NULL;
	}
	if (!uuid_date_from_uuid(entry->date, entry->uuid))
		return NULL;

	if (opt.verbose >= 4) {
		int i;

		i = system("(echo; echo After create_logfile:; "
		           "cat /home/sunny/uuids/fake.xml; "
		           "echo) >&2");
		i = i; /* Get rid of gcc warning */
	}
	if (add_to_logfile(logfp, entry) == EXIT_ERROR) {
#if PERL_COMPAT
		perlexit13 = TRUE; /* errno EACCES from die() in Perl */
#else
		myerror("process_uuid(): add_to_logfile() failed");
#endif
		return NULL;
	}

	if (!opt.whereto)
		puts(entry->uuid);
	else {
		if (strchr(opt.whereto, 'a') || strchr(opt.whereto, 'o'))
			fprintf(stdout, "%s\n", entry->uuid);
		if (strchr(opt.whereto, 'a') || strchr(opt.whereto, 'e'))
			fprintf(stderr, "%s\n", entry->uuid);
	}

	return entry->uuid;
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_OK;
	char *logfile;
	FILE *logfp;
	unsigned int i;

	progname = argv[0];
#if PERL_COMPAT
	progname = "suuid"; /* fixme: Temporary kludge to make it compatible 
	                     * with the Perl version.
	                     */
#endif

	init_xml_entry(&entry);

	retval = parse_options(&opt, argc, argv);
	if (retval != EXIT_OK)
		return EXIT_ERROR;

	msg(3, "Using verbose level %d", opt.verbose);

	if (opt.help) {
		usage(EXIT_OK);
		return EXIT_OK;
	} else if (opt.version) {
		print_version();
		return EXIT_OK;
	} else if (opt.license) {
		print_license();
		return EXIT_OK;
	}

#if 1
	/*
	 * Send non-option command line arguments to various functions for 
	 * testing. This doesn't break anything, as the program only checks for 
	 * options. Non-option arguments are ignored.
	 */
	if (optind < argc) {
		int i;

		for (i = optind; i < argc; i++) {
			char *a = argv[i];
			char buf[DATE_LENGTH + 1];

			msg(4, "Checking arg %d \"%s\"", i, a);
			printf("uuid_date(\"%s\") = \"%s\"\n",
			       a, uuid_date_from_uuid(buf, a));
		}
		return EXIT_OK;
	}
#endif

	if (read_rcfile(opt.rcfile, &rc) == EXIT_ERROR) {
		myerror("%s: Could not read rc file", opt.rcfile);
		retval = EXIT_ERROR;
		goto cleanup;
	}

	if (fill_entry_struct(&entry, &opt) == EXIT_ERROR) {
		retval = EXIT_ERROR;
		goto cleanup;
	}

	logfile = set_up_logfile(&opt, entry.host);
	if (!logfile) {
		myerror("Could not initialise log file");
		retval = EXIT_ERROR;
		goto cleanup;
	}

	logfp = open_logfile(logfile);
	if (!logfp) {
		myerror("open_logfile() failed, cannot open log file");
		retval = EXIT_ERROR;
		goto cleanup;
	}

	for (i = 0; i < opt.count; i++)
		if (!process_uuid(logfp, &entry)) {
			retval = EXIT_ERROR;
			goto cleanup;
		}

	if (close_logfile(logfp) == EXIT_ERROR)
		myerror("close_logfile() failed");

	if (optind < argc) {
		int t;

		for (t = optind; t < argc; t++)
			msg(3, "Non-option arg: %s", argv[t]);
	}

cleanup:

	free(entry.date);
	free(entry.cwd);

#if PERL_COMPAT
	if (perlexit13)
		retval = 13; /* die() is used some places in the Perl version, 
		              * and it exits with errno 13 (EACCES).
		              */
#endif
	msg(3, "Returning from main() with value %d", retval);

	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
