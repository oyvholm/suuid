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

/*
 * verbose_level() - Get or set the verbosity level. If action is 0, return the 
 * current level. If action is non-zero, set the level to argument 2 and return 
 * the new level.
 */

int verbose_level(const int action, ...)
{
	static int level = 0;

	if (action) {
		va_list ap;

		va_start(ap, action);
		level = va_arg(ap, int);
		va_end(ap);
	}

	return level;
}

/*
 * msg() - Print a message prefixed with "[progname]: " to stddebug if the 
 * current verbose level is equal or higher than the first argument. The rest 
 * of the arguments are delivered to vfprintf().
 * Returns the number of characters written.
 */

int msg(const int verbose, const char *format, ...)
{
	va_list ap;
	int retval = 0;

	if (verbose_level(0) >= verbose) {
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
#if FAKE_HOST || TEST_FUNC
	printf("\nThis version is compiled with the following conditional "
               "directives:\n");
#  if FAKE_HOST
	printf("\nFAKE_HOST: Always return \"fake\" as hostname. This is to "
	       "make sure it \n"
	       "doesn't write to the real log file.\n");
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

void usage(const int retval)
{
	if (retval != EXIT_OK) {
		fprintf(stderr, "\nType \"%s --help\" for help screen. "
		                "Returning with value %d.\n",
		                progname, retval);
		return;
	}

	puts("");
	if (verbose_level(0) >= 1) {
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
	       "    Current default: %s\n", ENV_LOGDIR, get_logdir(NULL));
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

int choose_opt_action(struct Options *dest,
                      const int c, const struct option *opts)
{
	int retval = EXIT_OK;
	static unsigned int tag_count = 0;

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
		dest->tag[tag_count++] = optarg;
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

int parse_options(struct Options *dest, const int argc, char * const argv[])
{
	int retval = EXIT_OK;
	int c;

	init_opt(dest);

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
	verbose_level(1, dest->verbose);

	return retval;
}

/*
 * main()
 */

int main(int argc, char *argv[])
{
	int retval = EXIT_OK;
	struct uuid_result result;
	struct Options opt;

	progname = argv[0];

	retval = parse_options(&opt, argc, argv);
	if (retval != EXIT_OK)
		return EXIT_ERROR;

	msg(3, "Using verbose level %d", verbose_level(0));

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

	result = create_and_log_uuids(&opt);
	if (!result.success)
		retval = EXIT_ERROR;

	msg(3, "Returning from main() with value %d", retval);
	return retval;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
