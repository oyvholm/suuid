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

static char *progname;
static struct Options opt;

/*
 * opt_struct() - Returns a copy of `opt`. It can be used by functions outside 
 * this file who need to inspect the values of various members of `opt`. It is 
 * currently used by test_command() in the test suite for use with 
 * streams_exec().
 */

struct Options opt_struct(void)
{
	return opt;
}

/*
 * msg() - Print a message prefixed with "[progname]: " to stderr if the 
 * current verbose level is equal or higher than the first argument. The rest 
 * of the arguments are delivered to vfprintf().
 * Returns the number of characters written.
 */

int msg(const int verbose, const char *format, ...)
{
	int retval = 0;

	assert(format);
	assert(*format);

	if (opt.verbose >= verbose) {
		va_list ap;

		va_start(ap, format);
		retval = fprintf(stderr, "%s: ", progname);
		retval += vfprintf(stderr, format, ap);
		retval += fprintf(stderr, "\n");
		va_end(ap);
	}

	return retval;
}

/*
 * std_strerror() - Replacement for `strerror()` that returns a predictable 
 * error message on every platform so the tests work everywhere.
 */

const char *std_strerror(const int errnum)
{
	switch (errnum) {
	case EACCES:
		return "Permission denied";
	case EISDIR:
		return "Is a directory";
	case ENOENT:
		return "No such file or directory";
	case ENOTTY:
		return "Inappropriate ioctl for device";
	case EPIPE:
		return "Broken pipe";
	default: /* gncov */
		/*
		 * Should never happen. If this line is executed, an `errno` 
		 * value is missing from `std_strerror()`, and tests may fail 
		 * on other platforms.
		 */
#ifdef CHECK_ERRNO
		fprintf(stderr,
		        "\n%s: %s(): Unknown errno received: %d, \"%s\"\n",
		        progname, __func__, errnum, strerror(errnum));
#endif
		return strerror(errnum); /* gncov */
	}
}

/*
 * myerror() - Print an error message to stderr using this format:
 *
 *     a: b: c
 *
 * where `a` is the name of the program (the value of `progname`), `b` is the 
 * output from the printf-like string and optional arguments, and `c` is the 
 * error message from `errno`.
 *
 * If `errno` contained an error value (!0), it is reset to 0.
 *
 * If `errno` indicates no error, the ": c" part is not printed. Returns the 
 * number of characters written.
 */

int myerror(const char *format, ...)
{
	va_list ap;
	int retval = 0;
	const int orig_errno = errno;

	assert(format);
	assert(*format);

	retval = fprintf(stderr, "%s: ", progname);
	va_start(ap, format);
	retval += vfprintf(stderr, format, ap);
	va_end(ap);
	if (orig_errno) {
		retval += fprintf(stderr, ": %s",
		                          std_strerror(orig_errno));
		errno = 0;
	}
	retval += fprintf(stderr, "\n");

	return retval;
}

/*
 * print_license() - Display the program license. Returns `EXIT_SUCCESS`.
 */

static int print_license(void)
{
	puts("(C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>");
	puts("");
	puts("This program is free software; you can redistribute it"
	     " and/or modify it \n"
	     "under the terms of the GNU General Public License as"
	     " published by the \n"
	     "Free Software Foundation; either version 2 of the License,"
	     " or (at your \n"
	     "option) any later version.");
	puts("");
	puts("This program is distributed in the hope that it will be"
	     " useful, but \n"
	     "WITHOUT ANY WARRANTY; without even the implied warranty of \n"
	     "MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.");
	puts("See the GNU General Public License for more details.");
	puts("");
	puts("You should have received a copy of"
	     " the GNU General Public License along \n"
	     "with this program. If not, see <http://www.gnu.org/licenses/>.");

	return EXIT_SUCCESS;
}

/*
 * print_version() - Print version information on stdout. If `-q` is used, only 
 * the version number is printed. Returns `EXIT_SUCCESS`.
 */

static int print_version(const struct Options *o)
{
#ifdef FAKE_MEMLEAK
	char *p;

	p = malloc(100);
	if (p) { }
#endif

	assert(o);
	if (o->verbose < 0) {
		puts(EXEC_VERSION);
		return EXIT_SUCCESS;
	}
	printf("%s %s (%s)\n", progname, EXEC_VERSION, EXEC_DATE);
#ifdef CHECK_ERRNO
	printf("has CHECK_ERRNO\n");
#endif
#ifdef FAKE_HOST
	printf("has FAKE_HOST\n");
#endif
#ifdef FAKE_MEMLEAK
	printf("has FAKE_MEMLEAK\n");
#endif
#ifdef GCOV
	printf("has GCOV\n");
#endif
#ifdef NDEBUG
	printf("has NDEBUG\n");
#endif
#ifdef PROF
	printf("has PROF\n");
#endif
#ifdef TEST_FUNC
	printf("has TEST_FUNC\n");
#endif
#ifdef UNUSED
	printf("has UNUSED\n");
#endif
#ifdef USE_NEW
	printf("has USE_NEW\n");
#endif
#ifdef VERIFY_UUID
	printf("has VERIFY_UUID\n");
#endif

	return EXIT_SUCCESS;
}

/*
 * usage() - Prints a help screen. Returns `retval`.
 */

static int usage(const struct Options *o, const int retval)
{
	char *logdir;

	assert(o);

	if (retval != EXIT_SUCCESS) {
		myerror("Type \"%s --help\" for help screen."
		        " Returning with value %d.", progname, retval);
		return retval;
	}

	logdir = get_logdir(NULL);

	puts("");
	if (o->verbose >= 1) {
		print_version(o);
		puts("");
	}
	printf("Usage: %s [options]\n", progname);
	printf("\n");
	printf("Generates one or more UUIDs and stores it to a log file with"
	       " optional \n"
	       "comment or tag/category.\n");
	printf("\n");
	printf("Options:\n");
	printf("\n");
	printf("  -c x, --comment x\n"
	       "    Store comment x in the log file. If \"-\" is specified as"
	       " comment, the \n"
	       "    program will read the comment from stdin. Two hyphens"
	       " (\"--\") as a \n"
	       "    comment opens the editor defined in the environment"
	       " variable \n"
	       "    %s to edit the message. If %s is not defined, \n"
	       "    the value from EDITOR is used. If none of these variables"
	       " are \n"
	       "    defined, the program aborts.\n", ENV_EDITOR, ENV_EDITOR);
	printf("  -n x, --count x\n"
	       "    Print and store x UUIDs.\n");
	printf("  -h, --help\n"
	       "    Show this help.\n");
	printf("  --license\n"
	       "    Print the software license.\n");
	printf("  -l x, --logdir x\n"
	       "    Store log files in directory x.\n"
	       "    If the %s environment variable is defined,"
	       " that value is \n"
	       "    used. Otherwise the value \"$HOME/uuids\" is used.\n"
	       "    Current default: %s\n", ENV_LOGDIR, logdir);
	printf("  -q, --quiet\n"
	       "    Be more quiet. Can be repeated to increase silence.\n");
	printf("  -m, --random-mac\n"
	       "    Don't use the hardware MAC address, generate a random"
	       " address field.\n");
	printf("  --raw\n"
	       "    Don't convert the <txt> element to XML. When using this"
	       " option, it \n"
	       "    is expected that the value of the -c/--comment option is"
	       " valid XML, \n"
	       "    otherwise it will create corrupted log files.\n");
	printf("  --rcfile X\n"
	       "    Use file X instead of '%s/%s'.\n",
	       getenv("HOME"), STD_RCFILE);
	printf("  --selftest [arg]\n"
	       "    Run the built-in test suite. If specified, the argument"
	       " can contain \n"
	       "    one or more of these strings: \"exec\" (the tests use the"
	       " executable \n"
	       "    file), \"func\" (runs function tests), or \"all\"."
	       " Multiple strings \n"
	       "    should be separated by commas. If no argument is"
	       " specified, default \n"
	       "    is \"all\".\n");
	printf("  -t x, --tag x\n"
	       "    Use x as tag (category).\n");
	printf("  --valgrind [arg]\n"
	       "    Run the built-in test suite with Valgrind memory checking."
	       " Accepts \n"
	       "    the same optional argument as --selftest, with the same"
	       " defaults.\n");
	printf("  -v, --verbose\n"
	       "    Increase level of verbosity. Can be repeated.\n");
	printf("  --version\n"
	       "    Print version information.\n");
	printf("  -w x, --whereto x\n"
	       "    x is a string which decides where the UUID will be"
	       " written:\n"
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
	       "        Don't output anything.\n"
	       "    Default: \"o\"\n");
	printf("\n");
	printf("If the %s environment variable is defined by"
	       " sess(1) or another \n"
	       "program, the value is logged if it is an UUID.\n", ENV_SESS);
	printf("\n");
	printf("A different hostname can be specified in the environment"
	       " variable \n"
	       "%s, or in the rc file %s/%s with the format \n"
	       "\"hostname = xxx\".\n",
	       ENV_HOSTNAME, getenv("HOME"), STD_RCFILE);
	printf("To use a specific MAC address all the time, add it to the rc"
	       " file using \n"
	       "the format \"macaddr = xxxxxxxxxxxx\".\n");
	printf("\n");

	free(logdir);

	return retval;
}

/*
 * choose_opt_action() - Decide what to do when option `c` is found. Store 
 * changes in `dest`. Read definitions for long options from `opts`.
 * Returns 0 if ok, or 1 if `c` is unknown or anything fails.
 */

static int choose_opt_action(struct Options *dest,
                             const int c, const struct option *opts)
{
	static unsigned int tag_count = 0;

	assert(dest);
	assert(opts);

	switch (c) {
	case 0:
		if (!strcmp(opts->name, "license")) {
			dest->license = true;
		} else if (!strcmp(opts->name, "raw")) {
			dest->raw = true;
		} else if (!strcmp(opts->name, "rcfile")) {
			dest->rcfile = optarg;
		} else if (!strcmp(opts->name, "selftest")) {
			dest->selftest = true;
		} else if (!strcmp(opts->name, "valgrind")) {
			dest->valgrind = dest->selftest = true;
		} else if (!strcmp(opts->name, "version")) {
			dest->version = true;
		}
		break;
	case 'c':
		dest->comment = optarg;
		break;
	case 'h':
		dest->help = true;
		break;
	case 'l':
		dest->logdir = optarg;
		break;
	case 'm':
		dest->random_mac = true;
		break;
	case 'n':
		if (!sscanf(optarg, "%lu", &dest->count)) {
			myerror("Error in -n/--count argument");
			return 1;
		}
		break;
	case 'q':
		dest->verbose--;
		break;
	case 't':
		if (tag_count >= MAX_TAGS) {
			fprintf(stderr, "%s: Maximum number of tags (%d)"
			                " exceeded\n", progname, MAX_TAGS);
			return 1;
		}
		dest->tag[tag_count++] = optarg;
		break;
	case 'v':
		dest->verbose++;
		break;
	case 'w':
		dest->whereto = optarg;
		break;
	default:
		myerror("%s(): getopt_long() returned character code %d",
		        __func__, c);
		return 1;
		break;
	}

	return 0;
}

/*
 * init_opt() - Initializes a `struct Options` with default values. Returns 
 * nothing.
 */

void init_opt(struct Options *dest)
{
	unsigned int i;

	assert(dest);

	dest->comment = NULL;
	dest->count = 1;
	dest->help = false;
	dest->license = false;
	dest->logdir = NULL;
	dest->random_mac = false;
	dest->raw = false;
	dest->rcfile = NULL;
	dest->selftest = false;
	dest->testexec = false;
	dest->testfunc = false;
	dest->uuid = NULL;
	dest->valgrind = false;
	dest->verbose = 0;
	dest->version = false;
	dest->whereto = NULL;
	for (i = 0; i < MAX_TAGS; i++)
		dest->tag[i] = NULL;
}

/*
 * set_opt_valgrind() - Change the value of `opt.valgrind`. Used by 
 * test_valgrind_option() in case --valgrind is used and Valgrind isn't 
 * installed.
 */

void set_opt_valgrind(bool b) /* gncov */
{
	assert(b == false || b == true); /* gncov */
	opt.valgrind = b; /* gncov */
} /* gncov */

/*
 * parse_options() - Parse command line options and store the result in `dest`. 
 * Returns 0 if successful, or 1 if an error occurs.
 */

static int parse_options(struct Options *dest,
                         const int argc, char * const argv[])
{
	int retval = 0;

	assert(dest);
	assert(argv);

	init_opt(dest);

	while (!retval) {
		int c;
		int option_index = 0;
		static const struct option long_options[] = {
			{"comment", required_argument, NULL, 'c'},
			{"count", required_argument, NULL, 'n'},
			{"help", no_argument, NULL, 'h'},
			{"license", no_argument, NULL, 0},
			{"logdir", required_argument, NULL, 'l'},
			{"quiet", no_argument, NULL, 'q'},
			{"random-mac", no_argument, NULL, 'm'},
			{"raw", no_argument, NULL, 0},
			{"rcfile", required_argument, NULL, 0},
			{"selftest", no_argument, NULL, 0},
			{"tag", required_argument, NULL, 't'},
			{"valgrind", no_argument, NULL, 0},
			{"verbose", no_argument, NULL, 'v'},
			{"version", no_argument, NULL, 0},
			{"whereto", required_argument, NULL, 'w'},
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
 * setup_options() - Do necessary changes to `o` based on the user input.
 *
 * - Parse the optional argument to --selftest and set `o->testexec` and 
 *   `o->testfunc`.
 *
 * Returns 0 if everything is ok, otherwise it returns 1.
 */

static int setup_options(struct Options *o, const int argc, char *argv[])
{
	assert(o);
	assert(argv);

	if (o->selftest) {
		if (optind < argc) {
			const char *s = argv[optind];
			if (!s) {
				myerror("%s(): argv[optind] is" /* gncov */
				        " NULL", __func__);
				return 1; /* gncov */
			}
			if (strstr(s, "all"))
				o->testexec = o->testfunc = true;
			if (strstr(s, "exec"))
				o->testexec = true; /* gncov */
			if (strstr(s, "func"))
				o->testfunc = true; /* gncov */
		} else {
			o->testexec = o->testfunc = true;
		}
	}

	return 0;
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_SUCCESS;
	struct uuid_result result;

	progname = argv[0];
	errno = 0;

	if (parse_options(&opt, argc, argv)) {
		myerror("Option error");
		return usage(&opt, EXIT_FAILURE);
	}

	msg(4, "%s(): Using verbose level %d", __func__, opt.verbose);

	if (setup_options(&opt, argc, argv))
		return EXIT_FAILURE; /* gncov */

	if (opt.help)
		return usage(&opt, EXIT_SUCCESS);
	if (opt.selftest)
		return opt_selftest(progname, &opt);
	if (opt.version)
		return print_version(&opt);
	if (opt.license)
		return print_license();

#ifdef TEST_FUNC
	/*
	 * Send non-option command line arguments to various functions for 
	 * testing. This doesn't break anything, as the program only checks for 
	 * options. Non-option arguments are ignored.
	 */
	if (optind < argc) {
		int i;

		for (i = optind; i < argc; i++) {
			char *a = argv[i];
			char buf[1000];

			msg(3, "Checking arg %d \"%s\"", i, a);
			memset(buf, 0, 1000);
			strncpy(buf, a, 999);
			printf("squeeze_chars(\"%s\", \",\") = \"%s\"\n",
			       a, squeeze_chars(buf, "e"));
		}
		return EXIT_SUCCESS;
	}
#endif

	result = create_and_log_uuids(&opt);
	if (!result.success)
		retval = EXIT_FAILURE;

	check_errno;

	msg(4, "Returning from %s() with value %d", __func__, retval);
	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
