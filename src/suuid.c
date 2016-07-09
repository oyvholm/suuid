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
	if (orig_errno)
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
	printf("%s %s (%s)\n", progname, VERSION, RELEASE_DATE);
#if FAKE_HOST || PERL_COMPAT || TEST_FUNC
	printf("\nThis version is compiled with the following conditional "
               "directives:\n");
#  if FAKE_HOST
	printf("\nFAKE_HOST: Always return \"fake\" as hostname. This is to "
	       "make sure it \n"
	       "doesn't write to the real log file.\n");
#  endif
#  if PERL_COMPAT
	printf("\nPERL_COMPAT: Suppress the new and better behaviour, behave "
	       "just like the \n"
	       "original Perl version to make the tests succeed.\n");
#  endif
#  if TEST_FUNC
	printf("\nTEST_FUNC: Send non-option command line arguments to "
	       "various functions \n"
	       "for testing. This doesn't break anything, as the program only "
	       "checks for \n"
	       "options. Non-option arguments are ignored.\n");
#  endif
#endif
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
	printf("Usage: %s [options]\n", progname);
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
	printf("  --license\n"
	       "    Print the software license\n");
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
	       "    Use file X instead of '%s/%s'.\n",
	       getenv("HOME"), STD_RCFILE);
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
	       "%s, or in the file %s/%s with the format \n"
	       "\"hostname = xxx\".\n",
	       ENV_HOSTNAME, getenv("HOME"), STD_RCFILE);
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
		else if (!strcmp(opts->name, "rcfile"))
			dest->rcfile = optarg;
		else if (!strcmp(opts->name, "version"))
			dest->version = TRUE;
		break;
	case 'c':
		dest->comment = optarg;
		break;
	case 'h':
		dest->help = TRUE;
		break;
	case 'l':
		dest->logdir = optarg;
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
		dest->whereto = optarg;
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
			{"whereto", required_argument, 0, 'w'},
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
 * process_comment_option() - Receive the argument used with -c/--comment and 
 * decide what to do with it. Return pointer to allocated string with the 
 * comment, or NULL if anything failed.
 */

char *process_comment_option(char *cmt)
{
	char *retval;

	assert(cmt);
	if (!strcmp(cmt, "-")) {
		retval = read_from_fp(stdin);
		if (!retval) {
			myerror("Could not read data from stdin");
			return NULL;
		}
	} else if (!strcmp(cmt, "--")) {
		char *e;

		e = get_editor();
		if (!e) {
			myerror("get_editor() failed");
			return NULL;
		}
		retval = read_from_editor(e);
		if (!retval) {
			myerror("Could not read data from editor \"%s\"", e);
			return NULL;
		}
		free(e);
	} else {
		retval = strdup(cmt);
		if (!retval) {
			myerror("%s: Cannot allocate memory for comment, "
			        "strdup() failed");
			return NULL;
		}
	}
	if (!valid_xml_chars(retval)) {
		fprintf(stderr, "%s: Comment contains illegal characters or "
		                "is not valid UTF-8\n", progname);
		free(retval);
		return NULL;
	}

	/* fixme: This is how it's done in the Perl version. I'm not sure if 
	 * it's an ok thing to do, even though it looks nice in the log files 
	 * and has worked great for years. Maybe this behaviour should be 
	 * changed when the C version passes all tests in suuid.t .
	 */
	trim_str_front(retval);
	trim_str_end(retval);

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
	if (!entry->host) {
		myerror("fill_entry_struct(): Cannot get hostname");
		return EXIT_ERROR;
	}
	if (!valid_hostname(entry->host)) {
		myerror("fill_entry_struct(): Got invalid hostname: \"%s\"",
		        entry->host);
		return EXIT_ERROR;
	}
	entry->cwd = getpath();
	entry->user = get_username();
	entry->tty = get_tty();

	if (opt->comment) {
		entry->txt = process_comment_option(opt->comment);
		if (!entry->txt)
			return EXIT_ERROR;
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
	if (!entry->uuid) {
#if PERL_COMPAT
		fprintf(stderr, "%s: '': Generated UUID is not in the "
		                "expected format\n", progname);
#else
		fprintf(stderr, "%s: UUID generation failed\n", progname);
#endif
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
 * init_randomness() - Initialise the random number generator. Returns EXIT_OK 
 * or EXIT_ERROR.
 */

bool init_randomness(void)
{
	struct timeval tv;

	if (gettimeofday(&tv, NULL) == -1)
		return EXIT_ERROR;

	srandom((unsigned int)tv.tv_sec ^ (unsigned int)tv.tv_usec ^
	        (unsigned int)getpid());

	return EXIT_OK;
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_OK;
	char *rcfile, *logfile = NULL;
	FILE *logfp;
	unsigned int i;

	progname = argv[0];
#if PERL_COMPAT
	progname = "suuid"; /* fixme: Temporary kludge to make it compatible 
	                     * with the Perl version.
	                     */
#endif

	if (init_randomness() == EXIT_ERROR) {
		myerror("Could not initialiase randomness generator");
		return EXIT_ERROR;
	}
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

#if TEST_FUNC
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

	rcfile = get_rcfilename();
	if (read_rcfile(rcfile, &rc) == EXIT_ERROR) {
		myerror("%s: Could not read rc file", opt.rcfile);
		retval = EXIT_ERROR;
		goto cleanup;
	}

	if (fill_entry_struct(&entry, &opt) == EXIT_ERROR) {
		retval = EXIT_ERROR;
		goto cleanup;
	}

	logfile = get_logfile_name();
	if (!logfile) {
		myerror("get_logfile_name() failed");
		retval = EXIT_ERROR;
		goto cleanup;
	}

	logfp = open_logfile(logfile);
	if (!logfp) {
		myerror("open_logfile() failed, cannot open log file");
		retval = EXIT_ERROR;
		goto cleanup;
	}

	for (i = 0; i < opt.count; i++) {
		if (!process_uuid(logfp, &entry)) {
			close_logfile(logfp);
			retval = EXIT_ERROR;
			goto cleanup;
		}
	}

	if (close_logfile(logfp) == EXIT_ERROR)
		myerror("close_logfile() failed");

	if (optind < argc) {
		int t;

		for (t = optind; t < argc; t++)
			msg(3, "Non-option arg: %s", argv[t]);
	}

cleanup:

	free(logfile);
	free(entry.date);
	free(entry.cwd);
	free(rcfile);

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
