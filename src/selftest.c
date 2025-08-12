/*
 * selftest.c
 * File ID: ee49f58e-9f61-11e6-b9e0-e6436a218c69
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
 * The functions in this file are supposed to be compatible with `Test::More` 
 * in Perl 5 as far as possible.
 */

#define EXECSTR  "__EXSTR__"
#define OPTION_ERROR_STR  EXECSTR ": Option error\n" \
                          EXECSTR ": Type \"" EXECSTR " --help\" for help screen." \
                          " Returning with value 1.\n"
#define chp  (char *[])
#define DIAG_DEBL  diag("DEBL: %s:%s():%d", __FILE__, __func__, __LINE__)
#define DIAG_VARS  do { \
	diag("%s = \"%s\"", "HOME", no_null(getenv("HOME"))); \
	diag("%s = \"%s\"", ENV_EDITOR, no_null(getenv(ENV_EDITOR))); \
	diag("%s = \"%s\"", ENV_HOSTNAME, no_null(getenv(ENV_HOSTNAME))); \
	diag("%s = \"%s\"", ENV_LOGDIR, no_null(getenv(ENV_LOGDIR))); \
	diag("%s = \"%s\"", ENV_SESS, no_null(getenv(ENV_SESS))); \
} while (0)

/*
 * Main test macros, meant to be a human-friendly frontend against ok(). Unlike 
 * most other testing frameworks that return 1 for success and 0 for error, 
 * these functions and macros return 0 for success and 1 if the test fails. The 
 * reasoning behind this is:
 *
 * We don't look for successful tests, but tests that fail. By returning 1 when 
 * the test fails, the return value can be used to increase a fail counter to 
 * find the total number of failed tests, or take special action based on the 
 * outcome of a single test or a series of previous tests. If the macros or 
 * ok() had returned 0 for failure and non-zero for success, we would need an 
 * additional counter to keep track of the number of executed tests and then 
 * use subtraction to see if any tests failed. Besides, C typically returns 0 
 * for success and non-zero for failure, so this convention should feel natural 
 * to C programmers. The `ok(expr, desc, ...)` function is a frontend to 
 * `ok_va()`, which evaluates `expr`, printing an "ok" line to stdout if `expr` 
 * is 0 (success, returning 0) or "not ok" if `expr` is non-zero (failure, 
 * returning 1), using `desc` with any additional arguments for the test 
 * description.
 *
 * All macros have a description parameter at the end which supports printf() 
 * sequences and additional optional arguments.
 *
 * In addition to the regular macros, the same macros exist as `OK_*_L()` 
 * variants. The functionality is identical, but they have an extra `linenum` 
 * parameter before the `desc` parameter. This is used to communicate to ok() 
 * and ok_va() the line number of the test. If the test is inside a helper 
 * function that's repeated many times, the line number inside the helper 
 * function is sent to ok(), and that's not what's usually wanted. By having a 
 * `const int linenumber` parameter in the helper function, the scopes above 
 * can deliver the actual line the test is executed from.
 *
 * This is a list of the various macros, with a description of which types of 
 * tests they're intended for, and the criteria for success:
 *
 * OK_EQUAL(a, b, desc, ...) - Verifies that the values `a` and `b` are 
 * identical. It uses `==` for comparison and is intended for variables of any 
 * type that supports the `==` operator.
 * Example: OK_EQUAL(num_found, expected, "Found %u elements", expected);
 *
 * OK_ERROR(msg, ...) - Generates a test failure with `msg` as the description. 
 * Used for unexpected errors that can't be ignored, incrementing the failure 
 * counter and failing the test run. Typically used in conditional checks.
 * Example: if (valgrind_lines(stderr_output))
 *                  OK_ERROR("Found Valgrind output in stderr");
 *
 * OK_FAILURE(func, desc, ...) - Used for functions that return 0 for success 
 * and non-zero for failure. Expects the function to fail (return non-zero). 
 * Example: OK_FAILURE(stat(file), "File is unreadable or doesn't exist");
 *
 * OK_FALSE(val, desc, ...) - Used for boolean values or expressions expected 
 * to be false. Negated expressions can be confusing, so `OK_TRUE` is usually a 
 * clearer choice for complex checks.
 * Examples: OK_FALSE(user_exists(user), "User %s doesn't exist", user);
 *           OK_FALSE(result == 5, "Result is not 5");
 *
 * OK_NOTEQUAL(a, b, desc, ...) - Expects the values `a` and `b` to be 
 * different. The `!=` operator is used for the comparison.
 * Example: OK_NOTEQUAL(userid1, userid2, "The users have different IDs");
 *
 * OK_NOTNULL(p, desc, ...) - Succeeds if the pointer `p` is non-NULL.
 * Examples: OK_NOTNULL(strstr(txt, substr), "Substring was found in text");
 *           OK_NOTNULL(fp, "File pointer is not NULL");
 *
 * OK_NULL(p, desc, ...) - Expects the pointer `p` to be NULL.
 * Examples: OK_NULL(getenv(var), "Environment variable %s is undefined", var);
 *           OK_NULL(strchr(file, '/'), "No slash in file name \"%s\"", file);
 *
 * OK_STRCMP(a, b, desc, ...) - Compares the strings `a` and `b` and succeeds 
 * if they're identical.
 * Example: OK_STRCMP(file, "index.html", "File name is correct");
 *
 * OK_STRNCMP(a, b, len, desc, ...) - Compares the first `len` characters of 
 * the strings `a` and `b` and succeeds if the substrings are identical.
 * Example: OK_STRNCMP(file, "tmp", 3, "File name has \"tmp\" prefix");
 *
 * OK_SUCCESS(func, desc, ...) - Used for functions that return 0 for success 
 * and non-zero for failure. Expects the function to succeed (return zero).
 * Example: OK_SUCCESS(rmdir(tempdir), "Delete temporary directory");
 *
 * OK_TRUE(val, desc, ...) - Expects the boolean value `val` to be true. This 
 * macro can also be used for comparisons or expressions not covered by other 
 * macros, like checking if a value is larger or smaller than another.
 * Examples: OK_TRUE(file_exists(file), "File %s was created", file);
 *           OK_TRUE(errcount < 10, "Error count %d is below 10", errcount);
 */

#define OK_EQUAL_L(a, b, linenum, desc, ...)  ok(!((a) == (b)), (linenum), (desc), ##__VA_ARGS__)
#define OK_ERROR_L(linenum, msg, ...)  ok(1, (linenum), (msg), ##__VA_ARGS__)
#define OK_FAILURE_L(func, linenum, desc, ...)  ok(!(func), (linenum), (desc), ##__VA_ARGS__)
#define OK_FALSE_L(val, linenum, desc, ...)  ok(!!(val), (linenum), (desc), ##__VA_ARGS__)
#define OK_NOTEQUAL_L(a, b, linenum, desc, ...)  ok(!((a) != (b)), (linenum), (desc), ##__VA_ARGS__)
#define OK_NOTNULL_L(p, linenum, desc, ...)  ok(!(p), (linenum), (desc), ##__VA_ARGS__)
#define OK_NULL_L(p, linenum, desc, ...)  ok(!!(p), (linenum), (desc), ##__VA_ARGS__)
#define OK_STRCMP_L(a, b, linenum, desc, ...)  ok(!!strcmp((a), (b)), (linenum), (desc), ##__VA_ARGS__)
#define OK_STRNCMP_L(a, b, len, linenum, desc, ...)  ok(!!strncmp((a), (b), (len)), (linenum), (desc), ##__VA_ARGS__)
#define OK_SUCCESS_L(func, linenum, desc, ...)  ok(!!(func), (linenum), (desc), ##__VA_ARGS__)
#define OK_TRUE_L(val, linenum, desc, ...)  ok(!(val), (linenum), (desc), ##__VA_ARGS__)

#define OK_EQUAL(a, b, desc, ...)  OK_EQUAL_L((a), (b), __LINE__, (desc), ##__VA_ARGS__)
#define OK_ERROR(msg, ...)  OK_ERROR_L(__LINE__, (msg), ##__VA_ARGS__)
#define OK_FAILURE(func, desc, ...)  OK_FAILURE_L((func), __LINE__, (desc), ##__VA_ARGS__)
#define OK_FALSE(val, desc, ...)  OK_FALSE_L((val), __LINE__, (desc), ##__VA_ARGS__)
#define OK_NOTEQUAL(a, b, desc, ...)  OK_NOTEQUAL_L((a), (b), __LINE__, (desc), ##__VA_ARGS__)
#define OK_NOTNULL(p, desc, ...)  OK_NOTNULL_L((p), __LINE__, (desc), ##__VA_ARGS__)
#define OK_NULL(p, desc, ...)  OK_NULL_L((p), __LINE__, (desc), ##__VA_ARGS__)
#define OK_STRCMP(a, b, desc, ...)  OK_STRCMP_L((a), (b), __LINE__, (desc), ##__VA_ARGS__)
#define OK_STRNCMP(a, b, len, desc, ...)  OK_STRNCMP_L((a), (b), (len), __LINE__, (desc), ##__VA_ARGS__)
#define OK_SUCCESS(func, desc, ...)  OK_SUCCESS_L((func), __LINE__, (desc), ##__VA_ARGS__)
#define OK_TRUE(val, desc, ...)  OK_TRUE_L((val), __LINE__, (desc), ##__VA_ARGS__)

#define failed_ok(a)  do { \
	if (errno) \
		OK_ERROR("%s():%d: %s failed: %s", \
		         __func__, __LINE__, (a), strerror(errno)); \
	else \
		OK_ERROR("%s():%d: %s failed", __func__, __LINE__, (a)); \
	errno = 0; \
} while (0)

/* 2025-04-26T21:02:32.1107790Z */
#define R_TIMESTAMP  "2[0-9]{3}-[01][0-9]-[0-3][0-9]" \
                     "T" \
                     "[0-2][0-9]:[0-5][0-9]:[0-6][0-9]\\.[0-9]{7}" \
                     "Z"

/* c5450b4e-22e1-11f0-bffd-83850402c3ce */
#define R_UUID  "[0-9a-f]{8}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{4}-[0-9a-f]{12}"

#define HNAME  "hname"
#define TMPDIR  ".suuid-test.tmp"

#define delete_logfile()  delete_logfile_func(__LINE__)
#define init_tempdir()  init_tempdir_func(__LINE__)
#define sc(cmd, num_stdout, num_stderr, desc, ...)  \
        sc_func(__LINE__, (cmd), (num_stdout), (num_stderr), \
                (desc), ##__VA_ARGS__);
#define set_env(name, val)  set_env_func(__LINE__, (name), (val))
#define tc(cmd, num_stdout, num_stderr, desc, ...)  \
        tc_func(__LINE__, (cmd), (num_stdout), (num_stderr), \
                (desc), ##__VA_ARGS__);
#define uc(cmd, num_stdout, num_stderr, desc, ...)  \
        uc_func(__LINE__, (cmd), (num_stdout), (num_stderr), \
                (desc), ##__VA_ARGS__);
#define unset_env(a)  unset_env_func(__LINE__, (a))
#define verify_logfile(entry, uuid_count, desc, ...)  \
        verify_logfile_func(__LINE__, (entry), (uuid_count), \
                            (desc), ##__VA_ARGS__);
#define verify_output_files(desc, exp_stdout, exp_stderr)  \
        verify_output_files_func(__LINE__, desc, exp_stdout, exp_stderr)

/* Used in chk_unique_macs() */
enum cum_mode {
	CHECK_UNIQUE,
	CHECK_EQUAL
};

/* Used in chk_rr_im() to specify which stderr output is expected. */
enum rr_msgs {
	RR_ILLEGAL_CHARS,
	RR_MULTICAST,
	RR_WRONG_LENGTH
};

static char *execname;
static char *logfile;
static const char *rcfile = TMPDIR "/" STD_RCFILE;
static int failcount = 0;
static int testnum = 0;

static const char *stderr_file = TMPDIR "/stderr.txt";
static const char *stdout_file = TMPDIR "/stdout.txt";
static int orig_stderr_fd = -1;
static int orig_stdout_fd = -1;

/******************************************************************************
                             --selftest functions
******************************************************************************/

/*
 * bail_out() - Aborts the test suite with an optional message written to 
 * stdout. To abort without a message, use NULL in `msg`. Returns nothing.
 */

static void bail_out(const char *msg, ...) /* gncov */
{
	va_list ap;

	fputs("Bail out!", stdout); /* gncov */
	if (msg) { /* gncov */
		fputs("  ", stdout); /* gncov */
		va_start(ap, msg); /* gncov */
		vprintf(msg, ap); /* gncov */
		va_end(ap); /* gncov */
	}
	fputc('\n', stdout); /* gncov */

	exit(EXIT_FAILURE); /* gncov */
}

/*
 * ok_va() - Print a log line to stdout. If `i` is 0, an "ok" line is printed, 
 * otherwise a "not ok" line is printed. `desc` is the test description and can 
 * use printf sequences.
 *
 * Returns 0 if `i` is 0, otherwise it returns 1.
 */

static int ok_va(const int i, const int linenum, const char *desc, va_list ap)
{
	va_list ap_copy;
	char *s, *s2;

	assert(desc);

	if (!desc)
		bail_out("%s(): desc is NULL", __func__); /* gncov */

	printf("%sok %d - %d: ", (i ? "not " : ""), ++testnum, linenum);
	va_copy(ap_copy, ap);
	s = allocstr_va(desc, ap_copy);
	va_end(ap_copy);
	if (!s)
		bail_out("allocstr_va() failed: %s", /* gncov */
		         strerror(errno)); /* gncov */
	s2 = str_replace(s, "\n", "\\n");
	if (!s2) {
		free(s); /* gncov */
		bail_out("str_replace() failed: %s", /* gncov */
		         strerror(errno)); /* gncov */
	}
	puts(s2);
	free(s2);
	free(s);
	fflush(stdout);
	failcount += !!i;

	return !!i;
}

/*
 * ok() - Frontend against ok_va(). Refer to the description for that function 
 * for more info. Returns 0 if `i` is 0, otherwise it returns 1.
 */

static int ok(const int i, const int linenum, const char *desc, ...)
{
	va_list ap;

	assert(desc);

	if (!desc)
		bail_out("%s(): desc is NULL", __func__); /* gncov */

	va_start(ap, desc);
	ok_va(i, linenum,  desc, ap);
	va_end(ap);

	return !!i;
}

/*
 * diag_output_va() - Receives a printf-like string and returns an allocated 
 * string, prefixed with "# " and all '\n' characters converted to "\n# ". 
 * Returns NULL if anything fails or `format` is NULL.
 */

static char *diag_output_va(const char *format, va_list ap)
{
	const char *src;
	char *buffer, *converted_buffer, *dst;
	size_t converted_size;

	assert(format);
	if (!format)
		return NULL; /* gncov */

	buffer = allocstr_va(format, ap);
	if (!buffer) {
		failed_ok("allocstr_va()"); /* gncov */
		return NULL; /* gncov */
	}

	/* Prepare for worst case, every char is a newline. */
	converted_size = strlen("# ") + strlen(buffer) * 3 + 1;
	converted_buffer = malloc(converted_size);
	if (!converted_buffer) {
		free(buffer); /* gncov */
		return NULL; /* gncov */
	}

	src = buffer;
	dst = converted_buffer;
	*dst++ = '#';
	*dst++ = ' ';
	while (*src) {
		if (*src == '\n') {
			*dst++ = '\n';
			*dst++ = '#';
			*dst++ = ' ';
		} else {
			*dst++ = *src;
		}
		src++;
	}
	*dst = '\0';
	free(buffer);

	return converted_buffer;
}

/*
 * diag_output() - Frontend against diag_output_va(), used by the tests. 
 * Returns the value from diag_output_va(); a pointer to the allocated string, 
 * or NULL if anything failed.
 */

static char *diag_output(const char *format, ...)
{
	va_list ap;
	char *result;

	if (!format)
		return NULL;

	va_start(ap, format);
	result = diag_output_va(format, ap);
	va_end(ap);

	return result;
}

/*
 * diag() - Prints a diagnostic message prefixed with "# " to stderr. `printf` 
 * sequences can be used. All `\n` characters are converted to "\n# ".
 *
 * A terminating `\n` is automatically added to the string. Returns 0 if 
 * successful, or 1 if `format` is NULL or diag_output_va() failed.
 */

static int diag(const char *format, ...)
{
	va_list ap;
	char *converted_buffer;

	if (!format)
		return 1;

	va_start(ap, format);
	converted_buffer = diag_output_va(format, ap);
	va_end(ap);
	if (!converted_buffer) {
		failed_ok("diag_output_va()"); /* gncov */
		return 1; /* gncov */
	}
	fprintf(stderr, "%s\n", converted_buffer);
	fflush(stderr);
	free(converted_buffer);

	return 0;
}

/*
 * gotexp_output() - Generate the output used by print_gotexp(). Returns NULL 
 * if allocstr() or mystrdup() fails. Otherwise, it returns a pointer to an 
 * allocated string with the output.
 */

static char *gotexp_output(const char *got, const char *exp)
{
	char *s;

	if (got == exp || (got && exp && !strcmp(got, exp))) {
		s = mystrdup("");
		if (!s)
			failed_ok("mystrdup()"); /* gncov */
	} else {
		s = allocstr("         got: '%s'\n"
		             "    expected: '%s'",
		             no_null(got), no_null(exp));
		if (!s)
			failed_ok("allocstr()"); /* gncov */
	}

	return s;
}

/*
 * print_gotexp() - Print the value of the actual and exepected data. Used when 
 * a test fails. Returns 1 if `got` or `exp` are different, otherwise 0.
 */

static int print_gotexp(const char *got, const char *exp)
{
	char *s;

	s = gotexp_output(got, exp);
	if (s && *s)
		diag(s); /* gncov */
	free(s);

	return 0;
}

/*
 * valgrind_lines() - Searches for Valgrind markers ("\n==DIGITS==") in `s`, 
 * used by test_command(). If a marker is found or `s` is NULL, it returns 1. 
 * Otherwise, it returns 0.
 */

static int valgrind_lines(const char *s)
{
	const char *p = s;

	if (!s)
		return OK_ERROR("%s(): s == NULL", __func__); /* gncov */

	while (*p) {
		p = strstr(p, "\n==");
		if (!p)
			return 0;
		p += 3;
		if (!*p)
			return 0;
		if (!isdigit((unsigned char)*p))
			continue;
		while (isdigit((unsigned char)*p))
			p++;
		if (!*p)
			return 0;
		if (!strncmp(p, "==", 2))
			return 1;
		p++;
	}

	return 0;
}

/*
 * tc_cmp() - Comparison function used by test_command(). There are 2 types of 
 * verification: One that demands that the whole output must be identical to 
 * the expected value, and the other is just a substring search. `got` is the 
 * actual output from the program, and `exp` is the expected output or 
 * substring.
 *
 * If `identical` is 0 (substring search) and `exp` is empty, the output in 
 * `got` must also be empty for the test to succeed.
 *
 * Returns 0 if the string was found, otherwise 1.
 */

static int tc_cmp(const int identical, const char *got, const char *exp)
{
	assert(got);
	assert(exp);
	if (!got || !exp)
		return 1; /* gncov */

	if (identical || !*exp)
		return !!strcmp(got, exp);

	return !strstr(got, exp);
}

/*
 * test_command() - Run the executable with arguments in `cmd` and verify 
 * stdout, stderr and the return value against `exp_stdout`, `exp_stderr` and 
 * `exp_retval`. Returns nothing.
 */

static void test_command(const int linenum, const char identical, char *cmd[],
                         const char *exp_stdout, const char *exp_stderr,
                         const int exp_retval, const char *desc, va_list ap)
{
	const struct Options o = opt_struct();
	struct streams ss;
	char *e_stdout, *e_stderr, *descbuf;

	assert(cmd);
	assert(desc);
	if (!cmd) {
		OK_ERROR_L(linenum, "%s(): cmd is NULL", __func__); /* gncov */
		return; /* gncov */
	}

	if (o.verbose >= 4) {
		int i = -1; /* gncov */
		fprintf(stderr, "# %s(", __func__); /* gncov */
		while (cmd[++i]) /* gncov */
			fprintf(stderr, "%s\"%s\"", /* gncov */
			                i ? ", " : "", cmd[i]); /* gncov */
		fprintf(stderr, ")\n"); /* gncov */
	}

	e_stdout = str_replace(exp_stdout, EXECSTR, execname);
	e_stderr = str_replace(exp_stderr, EXECSTR, execname);
	descbuf = allocstr_va(desc, ap);
	if (!descbuf) {
		failed_ok("allocstr_va()"); /* gncov */
		return; /* gncov */
	}
	streams_init(&ss);
	streams_exec(&o, &ss, cmd);
	if (e_stdout) {
		OK_FALSE_L(tc_cmp(identical, ss.out.buf, e_stdout), linenum,
		         "%s (stdout)", descbuf);
		if (tc_cmp(identical, ss.out.buf, e_stdout))
			print_gotexp(ss.out.buf, e_stdout); /* gncov */
	}
	if (e_stderr) {
		OK_FALSE_L(tc_cmp(identical, ss.err.buf, e_stderr), linenum,
		                  "%s (stderr)", descbuf);
		if (tc_cmp(identical, ss.err.buf, e_stderr))
			print_gotexp(ss.err.buf, e_stderr); /* gncov */
	}
	OK_EQUAL_L(ss.ret, exp_retval, linenum, "%s (retval)", descbuf);
	free(descbuf);
	free(e_stderr);
	free(e_stdout);
	if (ss.ret != exp_retval) {
		char *g = allocstr("%d", ss.ret), /* gncov */
		     *e = allocstr("%d", exp_retval); /* gncov */
		if (!g || !e) /* gncov */
			failed_ok("allocstr()"); /* gncov */
		else
			print_gotexp(g, e); /* gncov */
		free(e); /* gncov */
		free(g); /* gncov */
	}
	if (valgrind_lines(ss.err.buf))
		OK_ERROR_L(linenum, "Found valgrind output"); /* gncov */
	streams_free(&ss);
}

/*
 * sc_func() - Execute command `cmd` and verify that stdout, stderr and the 
 * return value corresponds to the expected values. The `exp_*` variables are 
 * substrings that must occur in the actual output. Not meant to be called 
 * directly, but via the uc() macro that logs the line number automatically. 
 * Returns nothing.
 */

static void sc_func(const int linenum, char *cmd[], const char *exp_stdout,
                    const char *exp_stderr, const int exp_retval,
                    const char *desc, ...)
{
	va_list ap;

	assert(cmd);
	assert(desc);

	va_start(ap, desc);
	test_command(linenum, 0, cmd, exp_stdout, exp_stderr, exp_retval,
	             desc, ap);
	va_end(ap);
}

/*
 * tc_func() - Execute command `cmd` and verify that stdout, stderr and the 
 * return value are identical to the expected values. The `exp_*` variables are 
 * strings that must be identical to the actual output. Not meant to be called 
 * directly, but via the tc() macro that logs the line number automatically. 
 * Returns nothing.
 */

static void tc_func(const int linenum, char *cmd[], const char *exp_stdout,
                    const char *exp_stderr, const int exp_retval,
                    const char *desc, ...)
{
	va_list ap;

	assert(cmd);
	assert(desc);

	va_start(ap, desc);
	test_command(linenum, 1, cmd, exp_stdout, exp_stderr, exp_retval,
	             desc, ap);
	va_end(ap);
}

/*
 * init_output_files() - Redirects stdout and stderr to files in TMPDIR. Used 
 * for testing functions that prints to stderr or stdout. Returns 0 on success, 
 * or 1 on failure.
 */

static int init_output_files(void)
{
	int stdout_fd, stderr_fd;

	assert(file_exists(TMPDIR));

	if (orig_stdout_fd != -1 || orig_stderr_fd != -1) {
		/*
		 * Already initialized, perhaps a previous test didn't restore 
		 * properly.
		 */
		OK_ERROR("%s(): Already initialized", __func__); /* gncov */
		return 1; /* gncov */
	}

	stdout_fd = open(stdout_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (stdout_fd == -1) {
		failed_ok("open() for stdout"); /* gncov */
		diag("stdout_file = \"%s\"", stdout_file); /* gncov */
		return 1; /* gncov */
	}

	stderr_fd = open(stderr_file, O_WRONLY | O_CREAT | O_TRUNC, 0666);
	if (stderr_fd == -1) {
		failed_ok("open() for stderr"); /* gncov */
		diag("stderr_file = \"%s\"", stderr_file); /* gncov */
		close(stdout_fd); /* gncov */
		return 1; /* gncov */
	}

	orig_stdout_fd = dup(1);
	if (orig_stdout_fd == -1) {
		failed_ok("dup() for stdout"); /* gncov */
		close(stdout_fd); /* gncov */
		close(stderr_fd); /* gncov */
		return 1; /* gncov */
	}

	orig_stderr_fd = dup(2);
	if (orig_stderr_fd == -1) {
		failed_ok("dup() for stderr"); /* gncov */
		close(stdout_fd); /* gncov */
		close(stderr_fd); /* gncov */
		close(orig_stdout_fd); /* gncov */
		return 1; /* gncov */
	}

	fflush(stdout);
	fflush(stderr);

	if (dup2(stdout_fd, 1) == -1) {
		failed_ok("dup2() for stdout"); /* gncov */
		close(stdout_fd); /* gncov */
		close(stderr_fd); /* gncov */
		close(orig_stdout_fd); /* gncov */
		close(orig_stderr_fd); /* gncov */
		orig_stdout_fd = -1; /* gncov */
		orig_stderr_fd = -1; /* gncov */
		return 1; /* gncov */
	}
	close(stdout_fd);

	if (dup2(stderr_fd, 2) == -1) {
		failed_ok("dup2() for stderr"); /* gncov */
		close(stderr_fd); /* gncov */
		close(orig_stdout_fd); /* gncov */
		close(orig_stderr_fd); /* gncov */
		/* Restore stdout partially if possible */
		dup2(orig_stdout_fd, 1); /* gncov */
		orig_stdout_fd = -1; /* gncov */
		orig_stderr_fd = -1; /* gncov */
		return 1; /* gncov */
	}
	close(stderr_fd);

	return 0;
}

/*
 * restore_output_files() - Restores stdout and stderr to their original file 
 * descriptors. Returns nothing, but logs failures.
 */

static void restore_output_files(void)
{
	if (orig_stdout_fd == -1 && orig_stderr_fd == -1) {
		/* Not initialized, nothing to restore */
		return; /* gncov */
	}

	fflush(stdout);
	fflush(stderr);

	if (orig_stdout_fd != -1) {
		if (dup2(orig_stdout_fd, 1) == -1) {
			failed_ok("dup2() (restore stdout)"); /* gncov */
		}
		close(orig_stdout_fd);
		orig_stdout_fd = -1;
	}

	if (orig_stderr_fd != -1) {
		if (dup2(orig_stderr_fd, 2) == -1) {
			failed_ok("dup2() (restore stderr)"); /* gncov */
		}
		close(orig_stderr_fd);
		orig_stderr_fd = -1;
	}
}

/*
 * verify_output_files_func() - Verify that the contents of stdout_file and 
 * stderr_file is equal to `exp_stdout` and `exp_stderr`, respectively. Returns 
 * nothing.
 */

static void verify_output_files_func(const int linenum, const char *desc,
                                     const char *exp_stdout,
                                     const char *exp_stderr)
{
	char *result;

	assert(exp_stdout);
	assert(exp_stderr);

	result = read_from_file(stdout_file);
	if (result) {
		OK_STRCMP_L(result, exp_stdout, linenum, "%s (stdout)", desc);
		print_gotexp(result, exp_stdout);
		free(result);
	} else {
		failed_ok("read_from_file(stdout_file)"); /* gncov */
	}

	result = read_from_file(stderr_file);
	if (result) {
		OK_STRCMP_L(result, exp_stderr, linenum, "%s (stderr)", desc);
		print_gotexp(result, exp_stderr);
		free(result);
	} else {
		failed_ok("read_from_file(stderr_file)"); /* gncov */
	}
}

/*
 * set_env_func() - Sets the environment variable `name` to `val`. It's 
 * normally not meant to be called directly, but via the set_env() macro that 
 * takes care of specifying `__LINE__` automatically.
 *
 * Returns 1 if unsetenv() failed, otherwise it returns 0.
 */

static int set_env_func(const int linenum, const char *name, const char *val)
{
	assert(name);
	assert(*name);
	assert(val);

	if (OK_SUCCESS_L(setenv(name, val, 1), linenum,
	                 "Set %s=\"%s\"", name, val)) {
		diag("Cannot set %s environment" /* gncov */
		     " variable to \"%s\": %s",
		     name, val, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return 1; /* gncov */
	}

	return 0;
}

/*
 * unset_env_func() - Unsets the environment variable `name` and prefixes the 
 * test description with `funcname` and `linenum` to avoid duplicated test 
 * descriptions. It's normally not meant to be called directly, but via the 
 * unset_env() macro that takes care of specifying `__LINE__` automatically.
 *
 * Returns 1 if unsetenv() failed, otherwise it returns 0.
 */

static int unset_env_func(const int linenum, const char *name)
{
	assert(name);
	assert(*name);

	if (!OK_SUCCESS_L(unsetenv(name), linenum, "Delete the %s envvar",
                                                   name))
		return 0;

	diag("Cannot delete %s: %s", name, strerror(errno)); /* gncov */
	errno = 0; /* gncov */

	return 1; /* gncov */
}

/*
 * is_root() - Returns 1 if the current user is root, otherwise it returns 0.
 */

static int is_root(void)
{
	return !getuid();
}

/******************************************************************************
                       suuid-specific selftest functions
******************************************************************************/

/*
 * count_uuids() - Returns the number of UUIDs followed by separator `sep` in 
 * the string `buf`. If an invalid UUID or separator is found, it returns the 
 * number of valid UUIDs and separators up until then.
 */

static unsigned long count_uuids(const char *buf, const char *sep)
{
	const char *p = buf, *endbuf;
	unsigned long count = 0L;
	size_t sep_len, both_len;

	assert(buf);
	assert(sep);

	sep_len = strlen(sep);
	both_len = UUID_LENGTH + sep_len;
	endbuf = p + strlen(buf);
	if ((size_t)(endbuf - p) < both_len)
		goto out;

	while ((size_t)(endbuf - p) >= both_len) {
		if (!valid_uuid(p, false))
			goto out;
		p += UUID_LENGTH;
		if (strncmp(p, sep, sep_len))
			goto out;
		p += sep_len;
		count++;
	}

out:
	return count;
}

/*
 * uc_check_va() - Verifies that `output` contains `num_exp` number of UUIDs. 
 * `name` describes if it's stdout or stderr, and the value there can be "out" 
 * or "err". `desc` is a short test description, and `ap` is printf-like 
 * arguments as a va_list. `linenum` is `__LINE__` from the caller function.
 */

static void uc_check_va(const int linenum, const char *name,
                        const char *output, const unsigned long num_exp,
                        const char *desc, va_list ap)
{
	va_list ap_copy;
	char *desc_str = NULL;
	size_t buflen;
	unsigned long found;

	assert(name);
	assert(strlen(name) == 3);
	assert(output);
	assert(desc);

	buflen = strlen(desc) + strlen(" (stdXXX)") + 1;
	desc_str = malloc(buflen);
	if (!desc_str) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	snprintf(desc_str, buflen, "%s (std%s)", desc, name);

	found = count_uuids(output, "\n");
	va_copy(ap_copy, ap);
	ok_va(!(found == num_exp), linenum, desc_str, ap_copy);
	if (found != num_exp) {
		diag("std%s: Found %lu UUID%s, expected %lu", /* gncov */
		     name, found, found == 1 ? "" : "s", num_exp);
		diag("%s(): std%s = \"%s\"", /* gncov */
		     __func__, name, output);
	}

	va_end(ap_copy);
	free(desc_str);
}

/*
 * uc_check() - Frontend against uc_check_va(). Read the description of that 
 * function for more info.
 */

static void uc_check(const int linenum, const char *name, const char *output,
                     const unsigned long num_exp, const char *desc, ...)
{
	va_list ap;

	assert(name);
	assert(strlen(name) == 3);
	assert(output);
	assert(desc);

	va_start(ap, desc);
	uc_check_va(linenum, name, output, num_exp, desc, ap);
	va_end(ap);
}

/*
 * uc_func() - Execute command `cmd` and verify that stdout and stderr contains 
 * the correct number of uuids. `desc` is a description of the test. Not meant 
 * to be called directly, but via the uc() macro that logs the line number 
 * automatically. Returns nothing.
 */

static void uc_func(const int linenum, char *cmd[],
                    const unsigned long num_stdout,
                    const unsigned long num_stderr, const char *desc, ...)
{
	struct streams ss;
	struct Options opt = opt_struct();
	va_list ap;
	char *s;

	assert(cmd);
	if (!cmd) {
		OK_ERROR("%s(): cmd is NULL", __func__); /* gncov */
		return; /* gncov */
	}
	assert(desc);

	streams_init(&ss);
	streams_exec(&opt, &ss, cmd);

	va_start(ap, desc);
	uc_check_va(linenum, "out", ss.out.buf, num_stdout, desc, ap);
	uc_check_va(linenum, "err", ss.err.buf, num_stderr, desc, ap);
	s = allocstr_va(desc, ap);
	if (s)
		OK_EQUAL_L(ss.ret, EXIT_SUCCESS, linenum, "%s (retval)", s);
	else
		failed_ok("allocstr_va()"); /* gncov */
	free(s);
	va_end(ap);

	streams_free(&ss);
}

/*
 * hostname_logfile() - Returns an allocated string to the log file when HNAME 
 * isn't used, i.e., when the value from get_hostname() is used. `dir` is the 
 * name of the log directory. If `dir` is NULL, the default name "uuids" is 
 * used. Returns NULL on error.
 */

static char *hostname_logfile(const char *dir)
{
	struct Rc rc;
	char *hostname, *hostname_log;

	init_rc(&rc);
	hostname = get_hostname(&rc);
	if (!hostname) {
		failed_ok("get_hostname()"); /* gncov */
		return NULL; /* gncov */
	}
	hostname_log = allocstr("%s/%s/%s%s",
	                        TMPDIR, dir ? dir : "uuids", hostname,
	                        LOGFILE_EXTENSION);
	if (!hostname_log)
		failed_ok("allocstr()"); /* gncov */

	return hostname_log;
}

/*
 * delete_logfile_func() - Deletes the log file. For the `funcname` value, 
 * `__func__` should be used, and `__LINE__` for `linenum`. This is to avoid 
 * duplicated test descriptions.
 *
 * Not meant to be called directly, but via the delete_logfile() macro that 
 * logs the line number automatically.
 *
 * Returns 1 if anything failed, otherwise 0.
 */

static int delete_logfile_func(const int linenum)
{
	int result;
	char *hostname_log;

	if (!strstr(getenv("HOME"), TMPDIR)) {
		bail_out("%s(): The string \"%s\" was not found" /* gncov */
		         " in the HOME environment variable, will not delete"
		         " the log file.", __func__, TMPDIR);
	}

	hostname_log = hostname_logfile(NULL);
	if (hostname_log) {
		if (file_exists(hostname_log))
			OK_SUCCESS_L(remove(hostname_log), linenum,
			             "Delete %s", hostname_log);
		free(hostname_log);
	} else {
		failed_ok("hostname_logfile()"); /* gncov */
	}

	if (!file_exists(logfile))
		return 0;

	result = remove(logfile);
	OK_SUCCESS_L(result, linenum, "Delete log file");
	if (result) {
		diag("test %d: errno = %d (%s)", /* gncov */
		     testnum, errno, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
	}

	return !!result;
}

/*
 * generate_tags_regexp() - Used by generate_line_regexp(). Returns an 
 * allocated string with all non-NULL `tag` elements in `entry` up to the first 
 * NULL element. Each element is surrounded with "<tag>" and "</tag>", and all 
 * "<tag>...</tag>" are surrounded by 1 space (" ") except the last one. 
 * Returns NULL if any error occurs.
 */

static char *generate_tags_regexp(const struct Entry *entry)
{
	unsigned int ui, tag_count = 0;
	size_t tag_lengths = 0, elem_len = strlen(" <tag></tag>"), bufsize;
	char *buf;

	assert(entry);

	if (!entry->tag[0])
		return mystrdup("");

	for (ui = 0; entry->tag[ui]; ui++) {
		tag_count++;
		tag_lengths += strlen(entry->tag[ui]);
	}
	bufsize = tag_count * elem_len + tag_lengths + 1;
	buf = malloc(bufsize);
	if (!buf)
		return NULL; /* gncov */
	memset(buf, 0, bufsize);

	for (ui = 0; ui < tag_count; ui++) {
		strcat(buf, " <tag>");
		strcat(buf, entry->tag[ui]);
		strcat(buf, "</tag>");
	}

	return buf;
}

/*
 * generate_line_regexp() - Returns a pointer to an allocated string with a 
 * `<suuid>...</suuid>` regexp generated from the data in `entry`. `count` is 
 * the expected number of lines and is used as a repetition count at the end of 
 * the regexp. Returns NULL if anything fails.
 */

static char *generate_line_regexp(const struct Entry *entry,
                                  const unsigned int count)
{
	char *tag_str = NULL, *txt_str = NULL, *sess_elems = NULL,
	     *r_entry = NULL;

	assert(entry);

	if (!count)
		return mystrdup("\n");

	tag_str = generate_tags_regexp(entry);
	if (!tag_str) {
		failed_ok("generate_tags_regexp()"); /* gncov */
		goto cleanup; /* gncov */
	}
	txt_str = entry->txt && *entry->txt
		? allocstr(" <txt>%s</txt>", entry->txt)
		: mystrdup("");
	if (!txt_str)
		goto cleanup; /* gncov */

	sess_elems = create_sess_xml(entry);

	r_entry = allocstr("(<suuid"
	                   " t=\"%s\""
	                   " u=\"%s\">"
	                   "%s"
	                   "%s"
	                   " <host>%s</host>"
	                   " <cwd>%s</cwd>"
	                   " <user>%s</user> "
	                   "%s"
	                   "</suuid>\n){%u}",
	                   *entry->date ? entry->date : R_TIMESTAMP,
	                   *entry->uuid ? entry->uuid : R_UUID,
	                   tag_str,
	                   txt_str,
	                   entry->host ? entry->host : "[^<]+",
	                   entry->cwd ? entry->cwd : "[^<]+",
	                   entry->user ? entry->user : "[^<]+",
	                   sess_elems,
	                   count);
	if (!r_entry) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}

cleanup:
	free(sess_elems);
	free(txt_str);
	free(tag_str);

	return r_entry;
}

/*
 * diag_sess_array/() - Print all non-NULL values of a `Struct sess` array. 
 * Returns nothing.
 */

static void diag_sess_array(const struct Sess *sess) /* gncov */
{
	size_t i;

	assert(sess); /* gncov */

	for (i = 0; i < MAX_SESS; i++) { /* gncov */
		if (sess[i].desc) /* gncov */
			diag("sess[%zu].desc: \"%s\"", /* gncov */
			     i, sess[i].desc); /* gncov */
		if (sess[i].uuid) /* gncov */
			diag("sess[%zu].uuid: \"%s\"", /* gncov */
			     i, sess[i].uuid); /* gncov */
	}
} /* gncov */

/*
 * verify_logfile_func() - Verify that the contents in the log file defined in 
 * the file-static variable `logfile` corresponds to the data in `entry` and 
 * that the number of entries is `uuid_count` is correct. `desc` is a test 
 * description, and can use printf escapes.
 *
 * Not meant to be called directly, but via the verify_logfile() macro that 
 * logs the line number automatically.
 *
 * Returns 0 if ok, or 1 if anything fails.
 */

static int verify_logfile_func(const int linenum, const struct Entry *entry,
                               const unsigned int uuid_count,
                               const char *desc, ...)
{
	char *r_entry = NULL, *r_file = NULL, *pattern = NULL,
	     *contents = NULL, *log_file = NULL;
	int retval = 1, result;
	regex_t regexp;
	va_list ap;

	assert(entry);
	assert(desc);

	r_entry = generate_line_regexp(entry, uuid_count);
	if (!r_entry) {
		failed_ok("generate_line_regexp()"); /* gncov */
		goto cleanup; /* gncov */
	}
	r_file = allocstr("<\\?xml version=\"1\\.0\" encoding=\"UTF-8\"\\?>\n"
	                  "<!DOCTYPE suuids SYSTEM \"dtd/suuids\\.dtd\">\n"
	                  "<suuids>\n"
	                  "%s"
	                  "</suuids>\n", uuid_count ? r_entry : "");
	if (!r_file) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}

	pattern = allocstr(r_file, uuid_count);
	if (!pattern) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	result = regcomp(&regexp, pattern, REG_EXTENDED);
	if (result) {
		char errbuf[1024];

		regerror(result, &regexp, errbuf, sizeof(errbuf)); /* gncov */
		OK_ERROR_L(linenum, "%s():%d: regcomp() failed: %s", /* gncov */
		           __func__, __LINE__, errbuf);
		regfree(&regexp); /* gncov */
		goto cleanup; /* gncov */
	}
	if (entry->host) {
		log_file = allocstr("%s/uuids/%s%s",
		                    TMPDIR, entry->host, LOGFILE_EXTENSION);
	} else {
		log_file = mystrdup(logfile);
	}
	if (!log_file) {
		failed_ok("allocstr() or mystrdup()"); /* gncov */
		goto cleanup; /* gncov */
	}
	contents = read_from_file(log_file);
	if (!contents) {
		failed_ok("read_from_file()"); /* gncov */
		diag("test %d: log_file = \"%s\"", /* gncov */
		     testnum, log_file);
	} else {
		result = regexec(&regexp, contents, 0, NULL, 0);
		va_start(ap, desc);
		if (ok_va(!!result, linenum, desc, ap)) {
			diag("Expected sess values:"); /* gncov */
			diag_sess_array(entry->sess); /* gncov */
			diag(""); /* gncov */
		}
		va_end(ap);
		if (result) {
			diag("Contents of \"%s\":\n\n%s", /* gncov */
			     log_file, contents);
			diag("regexp = \"%s\"", pattern); /* gncov */
		}
	}
	regfree(&regexp);

	retval = 0;

cleanup:
	free(contents);
	free(log_file);
	free(pattern);
	free(r_file);
	free(r_entry);

	return retval;
}

/*
 * init_tempdir_func() - Populate the temporary work directory defined in 
 * TMPDIR with an rc file and a `uuids` directory. Returns 0 if ok, or 1 if 
 * anything failed.
 */

static int init_tempdir_func(const int linenum)
{
	struct Rc rc;
	int result;

	init_rc(&rc);
	rc.hostname = HNAME;
	OK_SUCCESS_L(result = create_rcfile(rcfile, &rc), linenum,
	             "Create rcfile");
	if (result) {
		diag("%s(): Cannot create rcfile \"%s\", skipping" /* gncov */
		     " tests: %s",
		     __func__, rcfile, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return 1; /* gncov */
	}

	OK_SUCCESS_L(result = mkdir(TMPDIR "/uuids", 0777), linenum,
	             "mkdir uuids");
	if (result) {
		diag("Cannot create uuids/ directory, skipping" /* gncov */
		     " tests: %s", strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return 1; /* gncov */
	}

	return 0;
}

/*
 * cleanup_tempdir() - Delete the files and the `uuids` directory in the 
 * temporary work directory defined in TMPDIR. Expects `linenum` to be 
 * `__LINE__` or the `linenum` value from a parent function. Returns nothing.
 */

static void cleanup_tempdir(const int linenum)
{
	delete_logfile_func(linenum);
	if (file_exists(TMPDIR "/uuids"))
		OK_SUCCESS_L(rmdir(TMPDIR "/uuids"), linenum,
		             "Delete " TMPDIR "/uuids");
	if (file_exists(rcfile))
		OK_SUCCESS_L(remove(rcfile), linenum,
		             "Delete rc file");
	if (file_exists(stderr_file))
		OK_SUCCESS_L(remove(stderr_file), linenum,
		             "Delete %s", stderr_file);
	if (file_exists(stdout_file))
		OK_SUCCESS_L(remove(stdout_file), linenum,
		             "Delete %s", stdout_file);
}

/******************************************************************************
                 Function tests, no temporary directory needed
******************************************************************************/

                             /*** selftest.c ***/

/*
 * test_ok_macros() - Tests the OK_*() macros. Returns nothing.
 */

static void test_ok_macros(void)
{
	diag("Test the OK_*() macros");

	OK_EQUAL(3.14, 3.14, "OK_EQUAL(%.2f, %.2f, ...)", 3.14, 3.14);
	/* OK_ERROR("OK_ERROR(...), can't be tested"); */
	OK_FAILURE(1, "OK_FAILURE(1, ...)");
	OK_FALSE(5 == 9, "OK_FALSE(5 == 9, ...)");
	OK_NOTEQUAL(19716, 1916, "OK_NOTEQUAL(%u, %u, ...)", 19716, 1916);
	OK_NOTNULL(strstr("abcdef", "cde"), "OK_NOTNULL(strstr(\"abcdef\", \"cde\"), ...)");
	OK_NULL(strstr("abcdef", "notfound"), "OK_NULL(strstr(\"abcdef\", \"notfound\"), ...)");
	OK_STRCMP("str", "str", "OK_STRCMP(\"%s\", \"%s\", ...)", "str", "str");
	OK_STRNCMP("abcde", "abcyz", 3, "OK_STRNCMP(\"abcde\", \"abcyz\", 3, ...)");
	OK_SUCCESS(0, "OK_SUCCESS(0, ...)");
	OK_TRUE(9 > -4, "OK_TRUE(%d > %d, ...)", 9, -4);
}

/*
 * init_testvars() - Initialize file-static variables and environment variables 
 * to use the temporary directory defined in TMPDIR. Returns 1 if anything 
 * fails, or 0 if OK.
 */

static int init_testvars(void)
{
	char *cwd = NULL, *p = NULL;
	int retval = 1;
	struct Rc rc;

	diag("Initialize variables and the environment");
	cwd = getpath();
	if (!cwd) {
		failed_ok("getpath()"); /* gncov */
		return 1; /* gncov */
	}

	p = allocstr("%s/%s", cwd, TMPDIR);
	if (!p) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	if (set_env("HOME", p))
		goto cleanup; /* gncov */
	free(p);
	p = NULL;

	if (unset_env(ENV_EDITOR))
		goto cleanup; /* gncov */
	if (unset_env(ENV_HOSTNAME))
		goto cleanup; /* gncov */
	if (unset_env(ENV_LOGDIR))
		goto cleanup; /* gncov */
	if (unset_env(ENV_SESS))
		goto cleanup; /* gncov */

	init_rc(&rc);
	rc.hostname = HNAME;
	logfile = allocstr("%s/%s/uuids/%s%s",
	                   cwd, TMPDIR, get_hostname(&rc), LOGFILE_EXTENSION);
	if (!logfile) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	retval = 0;

cleanup:
	free(p);
	free(cwd);
	return retval;
}

/*
 * test_diag_big() - Tests diag_output() with a string larger than BUFSIZ. 
 * Returns nothing.
 */

static void test_diag_big(void)
{
	size_t size;
	char *p, *outp;

	size = BUFSIZ * 2;
	p = malloc(size + 1);
	if (!p) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}

	memset(p, 'a', size);
	p[3] = 'b';
	p[4] = 'c';
	p[size] = '\0';

	outp = diag_output("%s", p);
	OK_NOTNULL(outp, "diag_big: diag_output() returns ok");
	OK_EQUAL(strlen(outp), size + 2, "diag_big: String length is correct");
	OK_STRNCMP(outp, "# aaabcaaa", 10, "diag_big: Beginning is ok");
	free(outp);
	free(p);
}

/*
 * test_diag() - Tests the diag_output() function. diag() can't be tested 
 * directly because it would pollute the the test output. Returns nothing.
 */

static void test_diag(void) {
	char *p, *s;
	const char *desc;

	diag("Test diag()");

	OK_EQUAL(diag(NULL), 1, "diag(NULL)");
	OK_NULL(diag_output(NULL), "diag_output() receives NULL");

	p = diag_output("Text with\nnewline");
	OK_NOTNULL(p, "diag_output() with newline didn't return NULL");
	s = "# Text with\n# newline";
	desc = "diag_output() with newline, output is ok";
	if (p)
		OK_STRCMP(p, s, desc);
	else
		OK_ERROR(desc); /* gncov */
	print_gotexp(p, s);
	free(p);

	p = diag_output("\n\n\n\n\n\n\n\n\n\n");
	OK_NOTNULL(p, "diag_output() with only newlines didn't return NULL");
	s = "# \n# \n# \n# \n# \n# \n# \n# \n# \n# \n# ";
	desc = "diag_output() with only newlines";
	if (p)
		OK_STRCMP(p, s, desc);
	else
		OK_ERROR(desc); /* gncov */
	print_gotexp(p, s);
	free(p);

	p = diag_output("%d = %s, %d = %s, %d = %s",
	                1, "one", 2, "two", 3, "three");
	OK_NOTNULL(p, "diag_output() with %%d and %%s didn't return NULL");
	s = "# 1 = one, 2 = two, 3 = three";
	desc = "diag_output() with %%d and %%s";
	if (p)
		OK_STRCMP(p, s, desc);
	else
		OK_ERROR(desc); /* gncov */
	print_gotexp(p, s);
	free(p);

	test_diag_big();
}

/*
 * chk_go() - Used by test_gotexp_output(). Verifies that `gotexp_output(got, 
 * exp)` returns the correct output with the correct `exp_got` and `exp_exp` 
 * values, or an empty string if the strings are identical. Returns nothing.
 */

static void chk_go(const int linenum, const char *got, const char *exp,
                   const char *exp_got, const char *exp_exp)
{
	char *s, *exp_str;

	assert(exp_got);
	assert(exp_exp);

	s = gotexp_output(got, exp);
	if (!s) {
		failed_ok("gotexp_output()"); /* gncov */
		return; /* gncov */
	}
	if (exp_got == exp_exp
	    || (exp_got && exp_exp && !strcmp(exp_got, exp_exp))) {
		exp_str = mystrdup("");
	} else {
		exp_str = allocstr("         got: '%s'\n"
		                   "    expected: '%s'", exp_got, exp_exp);
	}
	if (!exp_str) {
		failed_ok("mystrdup() or allocstr()"); /* gncov */
		free(s); /* gncov */
		return; /* gncov */
	}
	OK_STRCMP_L(s, exp_str, linenum, "gotexp_output(\"%s\", \"%s\")",
	            no_null(got), no_null(exp));
	if (strcmp(s, exp_str))
		diag("Got:\n%s\nExpected:\n%s", s, exp_str); /* gncov */
	free(exp_str);
	free(s);
}

/*
 * test_gotexp_output() - Tests the gotexp_output() function. print_gotexp() 
 * can't be tested directly because it would pollute stderr. Returns nothing.
 */

static void test_gotexp_output(void)
{
	diag("Test gotexp_output()");

#define chk_go(got, exp, exp_got, exp_exp)  chk_go(__LINE__, (got), (exp), \
                                                   (exp_got), (exp_exp))
	chk_go("", "", "", "");
	chk_go("a", "a", "", "");
	chk_go("a", "b", "a", "b");
	chk_go("got this", "expected this", "got this", "expected this");
	chk_go("with\nnewline", "also with\nnewline",
	       "with\nnewline", "also with\nnewline");

	chk_go("a", NULL, "a", "(null)");
	chk_go(NULL, "b", "(null)", "b");
	chk_go(NULL, NULL, "", "");
#undef chk_go
}

/*
 * test_valgrind_lines() - Test the behavior of valgrind_lines(). Returns 
 * nothing.
 */

static void test_valgrind_lines(void)
{
	int i;
	const char
	*has[] = {
		"\n==123==",
		"\n==154363456657465745674567456523==maybe",
		"\n==\n==123==maybe",
		"\n==\n==123==maybe==456==",
		"indeed\n==1==",
		NULL
	},
	*hasnot[] = {
		"",
		"==123==",
		"\n=",
		"\n=123== \n234==",
		"\n=123==",
		"\n== 234==",
		"\n==",
		"\n==12.3==",
		"\n==123",
		"\n==123=",
		"\n==jj==",
		"abc",
		"abc\n==",
		NULL
	};

	diag("Test valgrind_lines()");

	i = 0;
	while (has[i]) {
		OK_TRUE(valgrind_lines(has[i]),
		        "valgrind_lines(): Has valgrind marker, string %d", i);
		i++;
	}

	i = 0;
	while (hasnot[i]) {
		OK_FALSE(valgrind_lines(hasnot[i]),
		         "valgrind_lines(): No valgrind marker, string %d", i);
		i++;
	}
}

/*
 * chk_cu() - Used by test_count_uuids(). Verifies that count_uuids() returns 
 * `count` number of UUIDs in `s` with the separator `sep`. `desc` is a short 
 * test description. Returns nothing.
 */

static void chk_cu(const int linenum, const char *s, const char *sep,
                   unsigned long count, const char *desc)
{
	assert(s);
	assert(sep);
	assert(desc);

	OK_EQUAL_L(count_uuids(s, sep), count, linenum, "%s", desc);
}

/*
 * test_count_uuids() - Tests the count_uuids() function. Returns nothing.
 */

static void test_count_uuids(void)
{
#define U  "da99fa2c-69bd-11f0-8ac5-83850402c3ce"
	diag("Test count_uuids()");

#define chk_cu(s, sep, count, desc)  chk_cu(__LINE__, (s), (sep), (count), (desc))
	chk_cu(U "\n", "\n", 1, "1 UUID with \\n");
	chk_cu(U "\n" U "\n", "\n", 2, "2 UUIDs with \\n");
	chk_cu(U, "\n", 0, "1 UUID, missing \\n");
	chk_cu(U "  ", " ", 1, "1 UUID, extra space");
	chk_cu(U " ", "  ", 0, "1 UUID, missing 1 space from separator");
	chk_cu("a" U " ", " ", 0, "1 UUID, 'a' in front");
	chk_cu(U "a", "b", 0, "1 UUID, separator doesn't match");
	chk_cu(U "a" U "a" U "b" U "a" U "a", "a", 2,
	       "5 UUIDs, separator 3 doesn't match");
	chk_cu(U, "", 1, "1 UUID, empty separator");
	chk_cu(U U U U U U U U U U, "", 10, "10 UUIDs, empty separator");
	chk_cu("da99fa2c-69bd-11f0-8ac5-83850402c3c", "a", 0,
	       "1 UUID which is 1 char too short, separator is 'a'");
#undef chk_cu
#undef U
}

                               /*** suuid.c ***/

/*
 * test_std_strerror() - Tests the std_strerror() function. Returns nothing.
 */

static void test_std_strerror(void)
{
	diag("Test std_strerror()");
	OK_STRCMP(std_strerror(EACCES), "Permission denied",
	          "std_strerror(EACCES) is as expected");
}

                                /*** io.c ***/

/*
 * test_read_from_file() - Tests the read_from_file() function. Returns 
 * nothing.
 */

static void test_read_from_file(void)
{
	char *p = NULL;
	int orig_errno;

	diag("Test read_from_file()");

	p = read_from_file(TMPDIR "/non-existing");
	orig_errno = errno;
	errno = 0;
	OK_NULL(p, "read_from_file(): Non-existing file, NULL is returned");
	OK_EQUAL(orig_errno, ENOENT, "read_from_file(): errno is ENOENT");
	if (orig_errno != ENOENT) {
		diag("errno was %d (%s)", /* gncov */
		     orig_errno, strerror(orig_errno));
	}
}

                              /*** logfile.c ***/

/*
 * chk_csx() - Used by test_create_sess_xml(). Verifies that the values in 
 * `sess` results in the string `exp`. `desc` is a short test description. 
 * Returns nothing.
 */

static void chk_csx(const int linenum, const struct Sess *sess,
                    const char *exp, const char *desc)
{
	struct Entry entry;
	char *result;
	int i;

	assert(sess);
	assert(exp);
	assert(desc);

	init_xml_entry(&entry);
	for (i = 0; i < MAX_SESS; i++) {
		entry.sess[i].uuid = sess[i].uuid;
		entry.sess[i].desc = sess[i].desc;
	}
	OK_NOTNULL_L(result = create_sess_xml(&entry), linenum,
	             "Generate sess array, %s", desc);
	if (!result) {
		failed_ok("create_sess_xml()"); /* gncov */
		return; /* gncov */
	}
	OK_STRCMP_L(result, exp, linenum, "The sess array is correct");
	print_gotexp(result, exp);
	free(result);
}

/*
 * test_create_sess_xml() - Tests the create_sess_xml() function. Returns 
 * nothing.
 */

static void test_create_sess_xml(void)
{
	struct Sess sess[MAX_SESS];

	diag("Test create_sess_xml()");

#define chk_csx(sess, exp, desc)  chk_csx(__LINE__, (sess), (exp), (desc))
	init_sess_array(sess);
	chk_csx(sess, "", "No sess elements");

	sess[0].uuid = "5175c9c8-5f82-11f0-a282-83850402c3ce";
	chk_csx(sess, "<sess>5175c9c8-5f82-11f0-a282-83850402c3ce</sess> ",
	        "1 sess element, uuid only");

	sess[1].uuid = "cd2c846c-5f82-11f0-903a-83850402c3ce";
	chk_csx(sess, "<sess>5175c9c8-5f82-11f0-a282-83850402c3ce</sess> "
	              "<sess>cd2c846c-5f82-11f0-903a-83850402c3ce</sess> ",
	        "2 sess elements, uuid only");

	sess[0].desc = "desc_1";
	chk_csx(sess,
	        "<sess desc=\"desc_1\">5175c9c8-5f82-11f0-a282-83850402c3ce</sess> "
	        "<sess>cd2c846c-5f82-11f0-903a-83850402c3ce</sess> ",
	        "2 sess elements, 1 desc");

	sess[1].desc = "desc_2";
	chk_csx(sess,
	        "<sess desc=\"desc_1\">5175c9c8-5f82-11f0-a282-83850402c3ce</sess> "
	        "<sess desc=\"desc_2\">cd2c846c-5f82-11f0-903a-83850402c3ce</sess> ",
	        "2 sess elements, 2 descs");
#undef chk_csx
}

                              /*** rcfile.c ***/

/*
 * chk_hk() - Used by test_has_key(). Verifies that `has_key(line, keyword)` 
 * returns `exp`. Returns nothing.
 */

static void chk_hk(const int linenum, const char *line, const char *keyword,
                   const char *exp)
{
	const char *result;

	assert(line);
	assert(keyword);

	result = has_key(line, keyword);
	if (!result || !exp) {
		OK_EQUAL_L(result, exp, linenum,
		           "has_key(\"%s\", \"%s\") (NULL check)",
		           line, keyword);
		print_gotexp(result, exp);
		return;
	}
	OK_STRCMP_L(result, exp, linenum,
	            "has_key(\"%s\", \"%s\")", line, keyword);
	print_gotexp(result, exp);
}

/*
 * test_has_key() - Tests the has_key() function. Returns nothing.
 */

static void test_has_key(void)
{
	diag("Test has_key()");

#define chk_hk(line, keyword, exp)  chk_hk(__LINE__, (line), (keyword), (exp))
	chk_hk("hostname = abc", "hostname", "abc");
	chk_hk("hostname=abc", "hostname", "abc");
	chk_hk("hostname= abc", "hostname", "abc");
	chk_hk("hostname =abc", "hostname", "abc");
	chk_hk("hostname     =    abc", "hostname", "abc");
	chk_hk("hostname=    abc", "hostname", "abc");
	chk_hk("hostname =", "hostname", "");
	chk_hk("hostname=", "hostname", "");
	chk_hk("# hostname = abc", "hostname", NULL);
	chk_hk("hostname: abc", "hostname", NULL);
	chk_hk("hostname:abc", "hostname", NULL);
	chk_hk("macaddr = abc", "hostname", NULL);
	chk_hk("unknown = abc", "unknown", "abc");
	chk_hk("abc==def", "abc", "=def");
	chk_hk("abc = = def ", "abc", "= def ");
#undef chk_hk
}

                              /*** sessvar.c ***/

/*
 * chk_gsi() - Used by test_get_sess_info(). Verifies that the value `env` in 
 * the environment variable defined by ENV_SESS results in the XML in `exp`. 
 * Returns nothing.
 */

static void chk_gsi(const int linenum, const char *env, const char *exp,
                    const char *desc)
{
	struct Entry entry;
	char *chk_env, *result = NULL;

	assert(env);
	assert(exp);
	assert(desc);
	assert(*desc);

	diag(desc);

	if (OK_SUCCESS_L(setenv(ENV_SESS, env, 1), linenum,
	                 "Set the %s environment variable", ENV_SESS)) {
		diag("Cannot set the %s envvar: %s", /* gncov */
		     ENV_SESS, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return; /* gncov */
	}
	chk_env = getenv(ENV_SESS);
	OK_NOTNULL_L(chk_env, linenum, "getenv(\"%s\") is not NULL", ENV_SESS);
	if (!chk_env)
		return; /* gncov */
	OK_STRCMP_L(chk_env, env, linenum, "%s is correct", ENV_SESS);

	init_xml_entry(&entry);
	if (get_sess_info(&entry)) {
		failed_ok("get_sess_info()"); /* gncov */
		return; /* gncov */
	}

	OK_NOTNULL_L(result = create_sess_xml(&entry), linenum,
	                                      "Extract info from %s",
	                                      ENV_SESS);
	if (!result) {
		failed_ok("create_sess_xml()"); /* gncov */
		goto cleanup; /* gncov */
	}
	trim_str_end(result);
	OK_STRCMP_L(result, exp, linenum, "Generated XML is correct");
	print_gotexp(result, exp);

cleanup:
	free(result);
	free_sess(&entry);
}

/*
 * test_get_sess_info() - Tests the get_sess_info() function. Returns nothing.
 */

static void test_get_sess_info(void)
{
	diag("Test get_sess_info()");

#define chk_gsi(env, exp, desc)  chk_gsi(__LINE__, (env), (exp), (desc))
	chk_gsi("", "", "Empty string");
	chk_gsi("27538da4-fc68-11dd-996d-000475e441b9",
	        "<sess>27538da4-fc68-11dd-996d-000475e441b9</sess>",
	        "1 single UUID");
	chk_gsi("ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,",
	        "<sess desc=\"ssh-agent\">da700fd8-43eb-11e2-889a-0016d364066c</sess>",
	        "\"ssh-agent/\" prefix and comma at the end");
	chk_gsi("ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,"
	        "dingle©/4c66b03a-43f4-11e2-b70d-0016d364066c,",
	        "<sess desc=\"ssh-agent\">da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess desc=\"dingle©\">4c66b03a-43f4-11e2-b70d-0016d364066c</sess>",
	        "\"ssh-agent\" and \"dingle©\"");
	chk_gsi("abc", "", "Variable doesn't contain any UUIDs");
	chk_gsi("abcdef;b/4c66b03a-43f4-11e2-b70d-0016d364066c",
	        "<sess desc=\"b\">4c66b03a-43f4-11e2-b70d-0016d364066c</sess>",
	        "Everything before the semicolon is gone");
	chk_gsi("ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c",
	        "<sess desc=\"ssh-agent\">da700fd8-43eb-11e2-889a-0016d364066c</sess>",
	        "With \"ssh-agent\", missing comma");
	chk_gsi("/da700fd8-43eb-11e2-889a-0016d364066c",
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess>",
	        "Missing name and comma, but has slash");
	chk_gsi("ee5db39a-43f7-11e2-a975-0016d364066c,"
	        "/da700fd8-43eb-11e2-889a-0016d364066c",
	        "<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess> "
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess>",
	        "Contains 2 UUIDs; latter missing name and comma, but has slash");
	chk_gsi("ee5db39a-43f7-11e2-a975-0016d364066c"
	        "da700fd8-43eb-11e2-889a-0016d364066c",
	        "<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess> "
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess>",
	        "2 UUIDs smashed together");
	chk_gsi("da700fd8-43eb-11e2-889a-0016d364066c"
	        "abc"
	        "ee5db39a-43f7-11e2-a975-0016d364066c",
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess desc=\"abc\">ee5db39a-43f7-11e2-a975-0016d364066c</sess>",
	        "2 UUIDs, only separated by \"abc\"");
	chk_gsi("da700fd8-43eb-11e2-889a-0016d364066c"
	        "abc/ee5db39a-43f7-11e2-a975-0016d364066c",
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess desc=\"abc\">ee5db39a-43f7-11e2-a975-0016d364066c</sess>",
	        "2 UUIDs, separated by \"abc/\"");
	chk_gsi("da700fd8-43eb-11e2-889a-0016d364066c"
	        "ee5db39a-43f7-11e2-a975-0016d364066c"
	        "abc",
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess>",
	        "2 UUIDs together, 'abc' at EOS");
	chk_gsi("da700fd8-43eb-11e2-889a-0016d364066c"
	        ",,ee5db39a-43f7-11e2-a975-0016d364066c",
	        "<sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess>",
	        "2 UUIDs separated by 2 commas");
	chk_gsi(",,,,,"
	        "abc/da700fd8-43eb-11e2-889a-0016d364066c"
	        ",,,,,,,,,,"
	        "def/ee5db39a-43f7-11e2-a975-0016d364066c"
	        ",,%..¤¤¤%¤,,,",
	        "<sess desc=\"abc\">da700fd8-43eb-11e2-889a-0016d364066c</sess> "
	        "<sess desc=\"def\">ee5db39a-43f7-11e2-a975-0016d364066c</sess>",
	        "Lots of commas+punctuation and 2 UUIDs with descs");
	chk_gsi("5f650dac-4404-11e2-8e0e-0016d364066c"
	        "5f660e28-4404-11e2-808e-0016d364066c"
	        "5f66ef14-4404-11e2-8b45-0016d364066c"
	        "5f67e266-4404-11e2-a6f8-0016d364066c",
	        "<sess>5f650dac-4404-11e2-8e0e-0016d364066c</sess> "
	        "<sess>5f660e28-4404-11e2-808e-0016d364066c</sess> "
	        "<sess>5f66ef14-4404-11e2-8b45-0016d364066c</sess> "
	        "<sess>5f67e266-4404-11e2-a6f8-0016d364066c</sess>",
	        "4 UUIDs, no separators");
	chk_gsi("5f650dac-4404-11e2-8e0e-0016d364066c"
	        "abc"
	        "5f660e28-4404-11e2-808e-0016d364066c"
	        "5f66ef14-4404-11e2-8b45-0016d364066c,"
	        "nmap/5f67e266-4404-11e2-a6f8-0016d364066c",
	        "<sess>5f650dac-4404-11e2-8e0e-0016d364066c</sess> "
	        "<sess desc=\"abc\">5f660e28-4404-11e2-808e-0016d364066c</sess> "
	        "<sess>5f66ef14-4404-11e2-8b45-0016d364066c</sess> "
	        "<sess desc=\"nmap\">5f67e266-4404-11e2-a6f8-0016d364066c</sess>",
	        "4 UUIDs, \"abc\" separates the first 2, last one has desc");
	chk_gsi("ssh-agent/fea9315a-43d6-11e2-8294-0016d364066c,"
	        "logging/febfd0f4-43d6-11e2-9117-0016d364066c,"
	        "screen/0e144c10-43d7-11e2-9833-0016d364066c,"
	        "ti/152d8f16-4409-11e2-be17-0016d364066c,",
	        "<sess desc=\"ssh-agent\">fea9315a-43d6-11e2-8294-0016d364066c</sess> "
	        "<sess desc=\"logging\">febfd0f4-43d6-11e2-9117-0016d364066c</sess> "
	        "<sess desc=\"screen\">0e144c10-43d7-11e2-9833-0016d364066c</sess> "
	        "<sess desc=\"ti\">152d8f16-4409-11e2-be17-0016d364066c</sess>",
	        "4 UUIDs, all with desc");
#undef chk_gsi
}

                              /*** strings.c ***/

/*
 * test_mystrdup() - Tests the mystrdup() function. Returns nothing.
 */

static void test_mystrdup(void)
{
	const char *txt = "Test string";
	char *s;

	diag("Test mystrdup()");
	OK_NULL(mystrdup(NULL), "mystrdup(NULL) == NULL");

	s = mystrdup(txt);
	if (!s) {
		failed_ok("mystrdup()"); /* gncov */
		return; /* gncov */
	}
	OK_STRCMP(s, txt, "mystrdup(): Strings are identical");
	free(s);
}

/*
 * test_allocstr() - Tests the allocstr() function. Returns nothing.
 */

static void test_allocstr(void)
{
	const size_t bufsize = BUFSIZ * 2 + 1;
	char *p, *p2, *p3;
	size_t alen;

	diag("Test allocstr()");
	p = malloc(bufsize);
	if (!p) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	memset(p, 'a', bufsize - 1);
	p[bufsize - 1] = '\0';
	p2 = allocstr("%s", p);
	if (!p2) {
		failed_ok("allocstr() with BUFSIZ * 2"); /* gncov */
		goto cleanup; /* gncov */
	}
	alen = strlen(p2);
	OK_EQUAL(alen, BUFSIZ * 2, "allocstr(): strlen is correct");
	p3 = p2;
	while (*p3) {
		if (*p3 != 'a') {
			p3 = NULL; /* gncov */
			break; /* gncov */
		}
		p3++;
	}
	OK_NOTNULL(p3, "allocstr(): Content of string is correct");

cleanup:
	free(p2);
	free(p);
}

/*
 * chk_cs() - Used by test_count_substr(). Verifies that the number of 
 * non-overlapping substrings `substr` inside string `s` is `count`. `desc` is 
 * the test description. Returns nothing.
 */

static void chk_cs(const int linenum, const char *s, const char *substr,
                   const size_t count, const char *desc)
{
	size_t result;

	result = count_substr(s, substr);
	OK_EQUAL_L(result, count, linenum, "count_substr(): %s", desc);
	if (result != count) {
		char *s_result = allocstr("%zu", result), /* gncov */
		     *s_count = allocstr("%zu", count); /* gncov */
		if (s_result && s_count) /* gncov */
			print_gotexp(s_result, s_count); /* gncov */
		else
			failed_ok("allocstr()"); /* gncov */
		free(s_count); /* gncov */
		free(s_result); /* gncov */
	}
}

/*
 * test_count_substr() - Tests the count_substr() function. Returns nothing.
 */

static void test_count_substr(void)
{
	char *s;
	size_t bsize = 10000;

	diag("Test count_substr()");

#define chk_cs(s, substr, count, desc)  chk_cs(__LINE__, (s), (substr), \
                                               (count), (desc))
	chk_cs("", "", 0, "s and substr are empty");
	chk_cs("", "a", 0, "s is empty");
	chk_cs("aaa", "", 0, "substr is empty");

	chk_cs("", NULL, 0, "substr is NULL");
	chk_cs(NULL, "abcdef", 0, "s is NULL");
	chk_cs(NULL, NULL, 0, "s and substr is NULL");

	chk_cs("Abc", "abc", 0, "Case sensitivity");
	chk_cs("a", "aa", 0, "substr is longer than s");
	chk_cs("aaa", "a", 3, "3 \"a\" in \"aaa\"");
	chk_cs("aaa", "aa", 1, "Non-overlapping \"aa\" in \"aaa\"");
	chk_cs("aaabaaa", "aaa", 2, "Non-overlapping \"aaa\" split by \"b\"");
	chk_cs("abababab", "ab", 4, "4 \"ab\" in s");
	chk_cs("abc", "b", 1, "Single character substring");
	chk_cs("abc", "d", 0, "Substring not found");
	chk_cs("abcdeabc", "abc", 2, "Substring at start and end");
	chk_cs("abcdef" "abcdef" "abcdef", "abc", 3, "3 \"abc\" in s");
	chk_cs("abcdef", "abcdef", 1, "s and substr are identical");
	chk_cs("zzzGHJ\nabc\nGHJ\nabcGHJ", "GHJ", 3, "s with newlines");
	chk_cs("Ḡṹṛḡḷḗ", "ḡ", 1, "UTF-8, U+1Exx area");

	s = malloc(bsize + 1);
	if (!s) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	memset(s, '!', bsize);
	s[bsize] = '\0';
	chk_cs(s, "!!!!!!!!!!", bsize / 10, "Large buffer");
	free(s);
#undef chk_cs
}

/*
 * chk_sr() - Used by test_str_replace(). Verifies that all non-overlapping 
 * occurrences of substring `s1` are replaced with the string `s2` in the 
 * string `s`, resulting in the string `exp`. Returns nothing.
 */

static void chk_sr(const int linenum, const char *s, const char *s1,
                   const char *s2, const char *exp, const char *desc)
{
	char *result;

	assert(desc);

	result = str_replace(s, s1, s2);
	if (!result || !exp) {
		OK_EQUAL_L(result, exp, linenum, "str_replace(): %s", desc);
	} else {
		OK_STRCMP_L(result, exp, linenum, "str_replace(): %s", desc);
		print_gotexp(result, exp);
	}
	free(result);
}

/*
 * test_str_replace() - Tests the str_replace() function. Returns nothing.
 */

static void test_str_replace(void)
{
	char *s;
	size_t bsize = 10000;

	diag("Test str_replace()");

#define chk_sr(s, s1, s2, exp, desc)  chk_sr(__LINE__, (s), (s1), (s2), (exp), (desc))
	chk_sr("", "", "", "", "s, s1, and s2 are empty");
	chk_sr("abc", "", "b", "abc", "s1 is empty");
	chk_sr("", "a", "b", "", "s is empty");
	chk_sr("", "a", "", "", "s and s2 is empty");

	chk_sr(NULL, "a", "b", NULL, "s is NULL");
	chk_sr("abc", NULL, "b", NULL, "s1 is NULL");
	chk_sr("abc", "a", NULL, NULL, "s2 is NULL");
	chk_sr(NULL, NULL, NULL, NULL, "s, s1, and s2 is NULL");

	chk_sr("test", "test", "test", "test", "s, s1, and s2 are identical");
	chk_sr("abc", "b", "DEF", "aDEFc", "abc, replace b with DEF");
	chk_sr("abcabcabc", "b", "DEF", "aDEFcaDEFcaDEFc",
	       "abcabcabc, replace all b with DEF");
	chk_sr("abcdefgh", "defg", "", "abch", "Replace defg with nothing");
	chk_sr("abcde", "bcd", "X", "aXe", "Replace bcd with X");
	chk_sr("abc", "d", "X", "abc", "d not in abc");
	chk_sr("ababab", "aba", "X", "Xbab", "Replace aba in ababab");
	chk_sr("abc", "b", "", "ac", "Replace b with nothing");
	chk_sr("abc", "", "X", "abc", "Replace empty with X");
	chk_sr("Ḡṹṛḡḷḗ", "ḡ", "X", "ḠṹṛXḷḗ", "Replace UTF-8 character with X");

	s = malloc(bsize + 1);
	if (!s) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	memset(s, '!', bsize);
	s[bsize] = '\0';
	chk_sr(s, "!!!!!!!!!!", "", "", "Replace all text in large buffer");
	free(s);

	s = malloc(bsize + 1);
	if (!s) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	memset(s, '!', bsize);
	s[1234] = 'y';
	s[bsize - 1] = 'z';
	s[bsize] = '\0';
	chk_sr(s, "!!!!!!!!!!", "", "!!!!y!!!!z",
	       "Large buffer with y and z");
	free(s);
#undef chk_sr
}

/*
 * test_string_to_lower() - Tests the string_to_lower() function. Returns 
 * nothing.
 */

static void test_string_to_lower(void)
{
	char s1[] = "ABCÅÆØ";

	diag("Test string_to_lower()");
	OK_NULL(string_to_lower(NULL), "string_to_lower(NULL)");
	OK_STRCMP(string_to_lower(s1), "abcÅÆØ",
	          "string_to_lower(\"ABCÅÆØ\")");
}

                               /*** uuid.c ***/

/*
 * chk_vu() - Used by test_valid_uuid(). Checks that `valid_uuid(uuid, 
 * check_len)` returns `exp_valid`. Returns nothing.
 */

static void chk_vu(const int linenum, const char *uuid, const bool check_len,
                   const bool exp_valid)
{
	bool res;

	assert(uuid);
	res = valid_uuid(uuid, check_len);
	OK_EQUAL_L(res, exp_valid, linenum,
	           "valid_uuid(\"%s\", %s) should return %s",
	           uuid, check_len ? "true" : "false",
	           exp_valid ? "true" : "false");
}

/*
 * test_valid_uuid() - Tests the valid_uuid() function. Returns nothing.
 */

static void test_valid_uuid(void)
{
	diag("Test valid_uuid()");
#define chk_vu(uuid, check_len, exp_valid)  chk_vu(__LINE__, (uuid), \
                                                   (check_len), (exp_valid));
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0", true, true);
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0123", false, true);
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0123", true, false);
	chk_vu("c9ffa9cb-708d-454b-b1f2-f18f609cb825", true, false);
#undef chk_vu
}

/*
 * chk_ivd() - Used by test_is_valid_date(). Checks that `is_valid_date(date, 
 * check_len)` returns the value in `exp`. Returns nothing.
 */

static void chk_ivd(const int linenum, const char *date, const bool check_len,
                    const int exp)
{
	int res;

	assert(date);
	res = is_valid_date(date, check_len);
	OK_EQUAL_L(res, exp, linenum, "is_valid_date(\"%s\", %s), expecting %d",
	                              date, check_len ? "true" : "false", exp);
}

/*
 * test_is_valid_date() - Tests the is_valid_date() function. Returns nothing.
 */

static void test_is_valid_date(void)
{
	diag("Test is_valid_date()");

#define chk_ivd(date, check_len, exp)  chk_ivd(__LINE__, (date), (check_len), \
                                               (exp))
	chk_ivd("2017-12-23T02:33:57Z", true, 0);
	chk_ivd("2017-12-23T02:33:57Z", false, 0);
	chk_ivd("2017-12-23T02:33:57.1234567Z", true, 1);
	chk_ivd("2017-12-23T02:33:57.1234567Z", false, 1);
	chk_ivd("2017-12-23T02:33:57.1234567Zabcd", false, 1);
#undef chk_ivd
}

/*
 * chk_ud() - Used by test_uuid_date(). The function first checks that 
 * uuid_date() returns the correct value, i.e. that `uuid` is a valid v1 UUID. 
 * This value is specified in `exp_ret`: 0 if uuid_date() is expected to return 
 * NULL, otherwise 1. It then checks that the generated timestamp is as 
 * expected.
 *
 * Returns nothing.
 */

static void chk_ud(const int linenum, const char *uuid, const int exp_ret,
                   const char *exp_date)
{
	int ret;
	char buf[DATE_LENGTH + 1];

	assert(uuid);
	assert(exp_date);

	ret = !!uuid_date(buf, uuid);
	OK_EQUAL_L(ret, exp_ret, linenum,
	           "uuid_date(): \"%s\" is%s a valid v1 UUID",
	           uuid, exp_ret ? "" : " not");
	if (!ret)
		return;
	OK_STRCMP_L(buf, exp_date, linenum, "Date string is correct", uuid);
	print_gotexp(buf, exp_date);
}

/*
 * test_uuid_date() - Tests the uuid_date() function. Returns nothing.
 */

static void test_uuid_date(void)
{
	diag("Test uuid_date()");

#define chk_ud(uuid, exp_ret, exp_date)  chk_ud(__LINE__, (uuid), (exp_ret), \
                                                (exp_date))
	chk_ud("00000000-0000-11e7-87d5-f74d993421b0", 1,
	       "2017-03-03T10:56:05.8089472Z");
	chk_ud("acdaf974-e78e-11e7-87d5-f74d993421b0", 1,
	       "2017-12-23T03:09:22.9493620Z");
	chk_ud("notvalid", 0, "");
	chk_ud("", 0, "");
	chk_ud("c9ffa9cb-708d-454b-b1f2-f18f609cb825", 0, "");
	chk_ud("acdaf974-e78e-11e7-87d5-g74d993421b0", 0, "");
#undef chk_ud
}

/******************************************************************************
                   Function tests, use a temporary directory
******************************************************************************/

                              /*** environ.c ***/

/*
 * test_get_log_prefix() - Tests the get_log_prefix() function. Returns 
 * nothing.
 */

static void test_get_log_prefix(void)
{
	struct Rc rc;
	struct Options o = opt_struct();
	char *hostname, *result = NULL, *exp = NULL, *dir = NULL;
	const char *desc;

	diag("Test get_log_prefix()");

	init_rc(&rc);
	rc.hostname = HNAME;
	hostname = get_hostname(&rc);

	desc = "get_log_prefix() with default values";

	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		return; /* gncov */
	}
	result = get_log_prefix(&rc, &o, LOGFILE_EXTENSION);
	restore_output_files();

	if (!result) {
		failed_ok("get_log_prefix()"); /* gncov */
		goto cleanup; /* gncov */
	}

	dir = getpath();
	if (!dir) {
		failed_ok("getpath()"); /* gncov */
		goto cleanup; /* gncov */
	}
	exp = allocstr("%s/%s/uuids/%s%s",
	               dir, TMPDIR, hostname, LOGFILE_EXTENSION);
	if (!exp) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_STRCMP(result, exp, "%s (retval)", desc);
	print_gotexp(result, exp);
	free(exp);
	free(result);
	verify_output_files(desc, "", "");

	desc = "get_log_prefix() with slash in rc.hostname";

	rc.hostname = "with/slash";
	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		goto cleanup; /* gncov */
	}
	result = get_log_prefix(&rc, &o, LOGFILE_EXTENSION);
	restore_output_files();
	OK_NULL(result, "%s (retval)", desc);

	exp = str_replace(EXECSTR ": Got invalid hostname: \"with/slash\"\n",
	                  EXECSTR, execname);
	if (!exp) {
		failed_ok("str_replace()"); /* gncov */
		goto cleanup; /* gncov */
	}
	verify_output_files(desc, "", exp);

cleanup:
	free(exp);
	free(result);
	free(dir);
}

                              /*** genuuid.c ***/

/*
 * test_fill_entry_struct() - Tests the fill_entry_struct() function. Returns 
 * nothing.
 */

static void test_fill_entry_struct(void)
{
	size_t bufsize = 1 + (UUID_LENGTH + 1) * (MAX_SESS + 1) + 1,
	       t, buf_len;
	char *buf, *p, *bufchk, *desc = NULL, *exp_stderr = NULL;
	int res;
	struct Entry entry;
	struct Rc rc;
	struct Options opt = opt_struct();

	diag("Test fill_entry_struct()");

	buf = malloc(bufsize);
	if (!buf) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}

	p = buf;
	*p++ = ',';
	for (t = 0; t < MAX_SESS + 1; t++) {
		generate_uuid(p);
		p += UUID_LENGTH;
		*p++ = ',';
	}
	*p = '\0';
	OK_SUCCESS(res = setenv(ENV_SESS, buf, 1),
	           "Init %s variable with MAX_SESS + 1 UUIDs", ENV_SESS);
	if (res) {
		diag("Cannot set %s variable: %s", /* gncov */
		     ENV_SESS, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	bufchk = getenv(ENV_SESS);
	if (!bufchk) {
		failed_ok("getenv()"); /* gncov */
		goto cleanup; /* gncov */
	}
	buf_len = strlen(bufchk);
	OK_EQUAL(buf_len, bufsize - 1,
	         "Length of the %s variable is %zu bytes",
	         ENV_SESS, bufsize - 1);

	init_xml_entry(&entry);
	init_rc(&rc);
	rc.hostname = HNAME;

	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		goto cleanup; /* gncov */
	}
	res = fill_entry_struct(&entry, &rc, &opt);
	restore_output_files();

	desc = allocstr("%s contains more than %u UUIDs", ENV_SESS, MAX_SESS);
	if (!desc) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_EQUAL(res, 1, "%s (retval)", desc);
	exp_stderr = allocstr("%s: Maximum number of sess entries (%u)"
	                      " exceeded\n", execname, MAX_SESS);
	verify_output_files(desc, "", exp_stderr);
	unset_env(ENV_SESS);

cleanup:
	free(exp_stderr);
	free(desc);
	free(buf);
	free(entry.cwd);
	free_sess(&entry);
	cleanup_tempdir(__LINE__);
}

/*
 * test_create_and_log_uuids() - Tests the create_and_log_uuids() function. 
 * Returns nothing.
 */

static void test_create_and_log_uuids(void)
{
	struct Options o;
	const char *desc;
	char *s = NULL, *s2 = NULL;
	struct uuid_result result;
	struct Entry entry;

	diag("Test create_and_log_uuids()");

	if (init_tempdir())
		return; /* gncov */

	diag("o.uuid contains a valid UUID");

	desc = "create_and_log_uuids() with predefined UUID";
	o = opt_struct();
	o.uuid = "a06f9b42-69d4-11f0-9d7b-83850402c3ce";

	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		return; /* gncov */
	}
	result = create_and_log_uuids(&o);
	restore_output_files();

	OK_TRUE(result.success, "%s (result.success)", desc);
	OK_EQUAL(result.count, 1, "%s (result.count)", desc);
	OK_STRCMP(result.lastuuid, o.uuid, "%s (result.lastuuid)", desc);

	s2 = allocstr("%s\n", o.uuid);
	if (!s2) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	verify_output_files(desc, s2, "");
	free(s2);

	s = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	    "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">\n"
	    "<suuids>\n"
	    "<suuid t=\"2025-07-26T03:57:19.4137410Z\""
	    " u=\"a06f9b42-69d4-11f0-9d7b-83850402c3ce\"> <host>";
	s2 = read_from_file(logfile);
	if (!s2) {
		failed_ok("read_from_file(logfile)"); /* gncov */
		s = NULL; /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NOTNULL(strstr(s2, s), "%s (log file)", desc);
	free(s2);
	s = s2 = NULL;
	delete_logfile();

	diag("o.uuid contains an invalid UUID");

	desc = "create_and_log_uuids() with predefined invalid UUID";
	o.uuid = "a06f9b42-69d4-11f0-9d7b-83850402c3cg";

	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		goto cleanup; /* gncov */
	}
	result = create_and_log_uuids(&o);
	restore_output_files();

	OK_FALSE(result.success, "%s (result.success)", desc);
	OK_EQUAL(result.count, 0, "%s (result.count)", desc);
	OK_STRCMP(result.lastuuid, "", "%s (result.lastuuid)", desc);
	s2 = str_replace("process_uuid(): UUID"
	                 " \"a06f9b42-69d4-11f0-9d7b-83850402c3cg\" is not"
	                 " valid.\n"
	                 EXECSTR ": Generated only 0 of 1 UUIDs\n",
	                 EXECSTR, execname);
	if (!s2) {
		failed_ok("str_replace()"); /* gncov */
		goto cleanup; /* gncov */
	}
	verify_output_files(desc, "", s2);
	free(s2);

	init_xml_entry(&entry);
	verify_logfile(&entry, 0, "%s (log file)", desc);
	delete_logfile();

	diag("o.uuid contains a v4 UUID");

	desc = "create_and_log_uuids() with predefined v4 UUID";
	o.uuid = "17dc339e-9e43-4032-bf8d-449db7b2547b";

	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		goto cleanup; /* gncov */
	}
	result = create_and_log_uuids(&o);
	restore_output_files();

	OK_FALSE(result.success, "%s (result.success)", desc);
	OK_EQUAL(result.count, 0, "%s (result.count)", desc);
	OK_STRCMP(result.lastuuid, "", "%s (result.lastuuid)", desc);
	s2 = str_replace("process_uuid(): UUID"
	                 " \"17dc339e-9e43-4032-bf8d-449db7b2547b\" is not"
	                 " valid.\n"
	                 EXECSTR ": Generated only 0 of 1 UUIDs\n",
	                 EXECSTR, execname);
	if (!s2) {
		failed_ok("str_replace()"); /* gncov */
		goto cleanup; /* gncov */
	}
	verify_output_files(desc, "", s2);
	verify_logfile(&entry, 0, "%s (log file)", desc);

cleanup:
	free(s);
	free(s2);
	cleanup_tempdir(__LINE__);
}

                                /*** io.c ***/

/*
 * test_create_file() - Tests the create_file() function. Returns nothing.
 */

static void test_create_file(void)
{
	const char *desc, *file, *res;
	struct stat sb;

	diag("Test create_file()");

	OK_TRUE(file_exists(TMPDIR), "%s exists", TMPDIR);
	OK_NULL(create_file(TMPDIR, NULL),
	        "create_file(%s), but it's already a directory", TMPDIR);
	OK_EQUAL(errno, EISDIR, "errno is EISDIR");
	if (errno != EISDIR)  {
		diag("test %d: errno = %d (%s)", /* gncov */
		     testnum, errno, strerror(errno)); /* gncov */
	}
	errno = 0;

	desc = "create_file() with NULL creates empty file";
	file = TMPDIR "/emptyfile";
	OK_NOTNULL(res = create_file(file, NULL), "%s (exec)", desc);
	if (!res) {
		diag("test %d: errno = %d (%s)", /* gncov */
		     testnum, errno, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
	}
	OK_STRCMP(res, "", "%s (retval)", desc);
	if (stat(file, &sb)) {
		failed_ok("stat()"); /* gncov */
		return; /* gncov */
	}
	OK_EQUAL(sb.st_size, 0, "%s is empty", file);
	OK_SUCCESS(remove(file), "Delete %s", file);
}

                              /*** rcfile.c ***/

/*
 * chk_rr_memb() - Used by chk_rr() and is created to avoid unnecessary 
 * duplication and make it easy to add new rc file keywords. Verifies that 
 * `got` (the keyword value delivered by read_rcfile()) is equal to the 
 * expected value `exp`. `name` is the keyword, and `desc` is the short test 
 * description specified in test_read_rcfile(). Returns nothing.
 */

static void chk_rr_memb(const int linenum, const char *got, const char *exp,
                        const char *name, const char *desc)
{
	int res;

	assert(name);
	assert(desc);

	if (!exp)
		return;

	res = got == exp || (got && exp && !strcmp(got, exp));
	OK_TRUE_L(res, linenum, "%s (%s)", desc, name);
	print_gotexp(got, exp);
}

/*
 * chk_rr() - Used by test_read_rcfile(). Verifies that read_rcfile() parses 
 * the contents of an rc file correctly. The contents of the rc file is stored 
 * in `contents`, and the result must be identical to the values in `exp`. 
 * `desc` is a short description of the test. Returns nothing.
 */

static void chk_rr(const int linenum, const char *contents, struct Rc *exp,
                   const char *desc)
{
	struct Rc got;

	assert(contents);
	assert(exp);
	assert(desc);

	diag("%s", desc);
	init_rc(&got);
	if (OK_NOTNULL_L(create_file(rcfile, contents), linenum,
	                 "Create rc file")) {
		failed_ok("create_file()"); /* gncov */
		return; /* gncov */
	}
	if (OK_SUCCESS_L(read_rcfile(rcfile, &got), linenum, "Read rc file")) {
		failed_ok("read_rcfile()"); /* gncov */
		goto remove_file; /* gncov */
	}

	chk_rr_memb(linenum, got.hostname, exp->hostname, "hostname", desc);
	chk_rr_memb(linenum, got.macaddr, exp->macaddr, "macaddr", desc);

	free_rc(&got);

remove_file:
	if (OK_SUCCESS_L(remove(rcfile), linenum, "Delete rc file"))
		failed_ok("remove()"); /* gncov */
}

/*
 * chk_rr_im() - "Check read_rcfile() with invalid MAC." Used by 
 * test_read_rcfile(). Creates an rc file with an invalid MAC address, stored 
 * in `mac` and verifies that the return value, `got.hostname`, stdout, and 
 * stderr is correct. `desc` is a short description of the test. Returns 
 * nothing.
 */

static void chk_rr_im(const int linenum, const char *mac, enum rr_msgs errval,
                      const char *desc)
{
	struct Rc got;
	char *contents, *exp_stderr = NULL;
	int res;
	const char *err_msgs[] = {
		[RR_ILLEGAL_CHARS] = "MAC address contains illegal characters,"
		                     " can only contain hex digits",
		[RR_MULTICAST ] = "MAC address doesn't follow RFC 4122,"
		                  " multicast bit not set",
		[RR_WRONG_LENGTH] = "Wrong MAC address length, must be exactly"
		                    " 12 hex digits"
	};
	const char *s;

	assert(mac);
	assert(desc);

	diag("%s", desc);
	init_rc(&got);
	contents = allocstr("macaddr = %s\n", mac);
	if (!contents) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}
	if (OK_NOTNULL_L(create_file(rcfile, contents), linenum,
	                 "Create rc file")) {
		failed_ok("create_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	s = "Read rc file";
	if (init_output_files()) {
		restore_output_files(); /* gncov */
		failed_ok("init_output_files()"); /* gncov */
		goto cleanup; /* gncov */
	}
	res = read_rcfile(rcfile, &got);
	restore_output_files();
	OK_EQUAL_L(res, 1, linenum, "%s (retval)", s);
	OK_NULL_L(got.macaddr, linenum, "%s (macaddr)", s);
	exp_stderr = allocstr("%s: %s\n", execname, err_msgs[errval]);
	if (!exp_stderr) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	verify_output_files_func(linenum, desc, "", exp_stderr);
	print_gotexp(got.macaddr, NULL);

cleanup:
	free(exp_stderr);
	free_rc(&got);
	free(contents);
	cleanup_tempdir(linenum);
}

/*
 * test_read_rcfile() - Tests the read_rcfile() function. Returns nothing.
 */

static void test_read_rcfile(void)
{
	diag("Test read_rcfile()");

#define MAC  "a1a1e513f90b"
#define sr  &(struct Rc)

#define chk_rr(contents, exp, desc)  chk_rr(__LINE__, (contents), (exp), (desc))
	chk_rr("hostname = abc\n", sr{ .hostname = "abc" }, "hostname = abc");
	chk_rr("hostname=abc\n", sr{ .hostname = "abc" }, "hostname=abc");
	chk_rr("macaddr=" MAC, sr{ .macaddr = MAC }, "hostname=" MAC
	       ", no \\n");
	chk_rr("hostname = abc\nmacaddr=" MAC "\n",
	       (sr{ .hostname = "abc", .macaddr = MAC }),
	       "hostname and macaddr, all is good");
	chk_rr("macaddr=A1A1E513F90B\n", (sr{ .macaddr = MAC }),
	       "MAC address is upper case");
	chk_rr("   macaddr     =       " MAC "      \n", (sr{ .macaddr = MAC }),
	       "macaddr with surrounding spaces");
	chk_rr("macaddr =\n", (sr{ .macaddr = NULL }),
	       "macaddr with no value, that's OK");
	chk_rr("macaddr =   \n", (sr{ .macaddr = NULL }),
	       "macaddr with no value, but with trailing spaces");
	chk_rr("macaddr: " MAC "\n", (sr{ .macaddr = NULL }),
	       "macaddr with colon instead of equal sign");
#undef chk_rr

	diag("Invalid MAC address in the rc file");

#define chk_rr_im(mac, errval, desc)  chk_rr_im(__LINE__, (mac), (errval), \
                                                (desc))
	chk_rr_im("10460a166a4d", RR_MULTICAST,
	          "rc file with invalid macaddr, doesn't follow the RFC");
	chk_rr_im("1b460a166a4", RR_WRONG_LENGTH,
	          "rc file with invalid macaddr, one digit too short");
	chk_rr_im(MAC "a", RR_WRONG_LENGTH,
	          "rc file with invalid macaddr, one digit too long");
	chk_rr_im(MAC "y", RR_WRONG_LENGTH,
	          "rc file with invalid macaddr, extra invalid character");
	chk_rr_im("iiiiiiiiiiii", RR_ILLEGAL_CHARS,
	          "rc file with invalid macaddr, correct length, invalid hex"
	          " digits");
	chk_rr_im("invalid", RR_ILLEGAL_CHARS,
	          "rc file with invalid macaddr, not a hex number at all");
	chk_rr_im("'" MAC "'", RR_ILLEGAL_CHARS,
	          "rc file with valid macaddr, but it's inside ''");
	chk_rr_im("\"" MAC "\"", RR_ILLEGAL_CHARS,
	          "rc file with valid macaddr, but it's inside \"\"");
#undef chk_rr_im

#undef MAC
#undef sr
}

/******************************************************************************
            Test the executable file, no temporary directory needed
******************************************************************************/

/*
 * test_valgrind_option() - Tests the --valgrind command line option. Returns 
 * nothing.
 */

static void test_valgrind_option(const struct Options *o)
{
	struct streams ss;

	assert(o);
	diag("Test --valgrind");

	if (o->valgrind) {
		struct Options mod_opt = *o; /* gncov */

		mod_opt.valgrind = false; /* gncov */
		streams_init(&ss); /* gncov */
		streams_exec(&mod_opt, &ss, chp{ "valgrind", /* gncov */
		                                 "--version", NULL });
		if (!strstr(ss.out.buf, "valgrind-")) { /* gncov */
			OK_ERROR("Valgrind is not installed," /* gncov */
			         " disabling Valgrind checks");
			set_opt_valgrind(false); /* gncov */
		} else {
			OK_SUCCESS(0, "Valgrind is installed"); /* gncov */
		}
		streams_free(&ss); /* gncov */
	}

	sc((chp{ execname, "--valgrind", "-h", NULL }),
	   "Show this",
	   "",
	   EXIT_SUCCESS,
	   "--valgrind -h");
}

/*
 * print_version_info() - Display output from the --version command. Returns 0 
 * if ok, or 1 if streams_exec() failed.
 */

static int print_version_info(const struct Options *o)
{
	struct streams ss;
	int res;

	assert(o);
	streams_init(&ss);
	res = streams_exec(o, &ss, chp{ execname, "--version", NULL });
	if (res) {
		failed_ok("streams_exec()"); /* gncov */
		if (ss.err.buf) /* gncov */
			diag(ss.err.buf); /* gncov */
		return 1; /* gncov */
	}
	diag("========== BEGIN version info ==========\n"
	     "%s"
	     "=========== END version info ===========", no_null(ss.out.buf));
	streams_free(&ss);

	return 0;
}

/*
 * test_standard_options() - Tests the various generic options available in 
 * most programs. Returns nothing.
 */

static void test_standard_options(void)
{
	char *s;

	diag("Test standard options");

	diag("Test -h/--help");
	sc((chp{ execname, "-h", NULL }),
	   "  Show this help",
	   "",
	   EXIT_SUCCESS,
	   "-h");
	sc((chp{ execname, "--help", NULL }),
	   "  Show this help",
	   "",
	   EXIT_SUCCESS,
	   "--help");

	diag("Test -v/--verbose");
	sc((chp{ execname, "-h", "--verbose", NULL }),
	   "  Show this help",
	   "",
	   EXIT_SUCCESS,
	   "-hv: Help text is displayed");
	sc((chp{ execname, "-hv", NULL }),
	   EXEC_VERSION,
	   "",
	   EXIT_SUCCESS,
	   "-hv: Version number is printed along with the help text");
	sc((chp{ execname, "-vvv", "--verbose", "--help", NULL }),
	   "  Show this help",
	   EXECSTR ": main(): Using verbose level 4\n",
	   EXIT_SUCCESS,
	   "-vvv --verbose: Using correct verbose level");
	sc((chp{ execname, "-vvvvq", "--verbose", "--verbose", "--help", NULL }),
	   "  Show this help",
	   EXECSTR ": main(): Using verbose level 5\n",
	   EXIT_SUCCESS,
	   "--verbose: One -q reduces the verbosity level");

	diag("Test --version");
	s = allocstr("%s %s (%s)\n", execname, EXEC_VERSION, EXEC_DATE);
	if (s) {
		sc((chp{ execname, "--version", NULL }),
		   s,
		   "",
		   EXIT_SUCCESS,
		   "--version");
		free(s);
	} else {
		failed_ok("allocstr()"); /* gncov */
	}
	tc((chp{ execname, "--version", "-q", NULL }),
	   EXEC_VERSION "\n",
	   "",
	   EXIT_SUCCESS,
	   "--version with -q shows only the version number");

	diag("Test --license");
	sc((chp{ execname, "--license", NULL }),
	   "GNU General Public License",
	   "",
	   EXIT_SUCCESS,
	   "--license: It's GPL");
	sc((chp{ execname, "--license", NULL }),
	   "either version 2 of the License",
	   "",
	   EXIT_SUCCESS,
	   "--license: It's version 2 of the GPL");

	diag("Unknown option");
	sc((chp{ execname, "--gurgle", NULL }),
	   "",
	   OPTION_ERROR_STR,
	   EXIT_FAILURE,
	   "Unknown option: \"Option error\" message is printed");
}

/*
 * chk_ihv() - Used by test_invalid_hostname_var(). Sets the environment 
 * variable defined by ENV_HOSTNAME to the value in `val` and executes the 
 * program. `desc` is a short test description. Returns nothing.
 */

static void chk_ihv(const int linenum, const char *val, const char *desc)
{
	char *exp_stderr;
	int res;

	exp_stderr = allocstr("%s: Got invalid hostname: \"%s\"\n",
	                      execname, val);
	if (!exp_stderr) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}
	res = OK_SUCCESS_L(setenv(ENV_HOSTNAME, val, 1), linenum,
	                   "Set the %s variable", ENV_HOSTNAME);
	if (res) {
		failed_ok("setenv()"); /* gncov */
		return; /* gncov */
	}
	tc_func(linenum, (chp{ execname, NULL }),
	        "",
	        exp_stderr,
	        EXIT_FAILURE,
	        "%s %s", ENV_HOSTNAME, desc);
	free(exp_stderr);
}

/*
 * test_invalid_hostname_var() - Tests invalid values in the environment 
 * variable defined in ENV_HOSTNAME. Returns nothing.
 */

static void test_invalid_hostname_var(void)
{
	char *p;

	diag("Invalid value in %s", ENV_HOSTNAME);

#define chk_ihv(val, desc)  chk_ihv(__LINE__, (val), (desc))

	chk_ihv("", "is empty");
	chk_ihv("a;b", "contains semicolon");
	chk_ihv("a..b", "contains \"..\"");
	chk_ihv("a/b", "contains slash");
	chk_ihv("x\xf8z", "contains latin1 char");

	p = malloc(MAX_HOSTNAME_LENGTH + 2);
	if (!p) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	memset(p, 'a', MAX_HOSTNAME_LENGTH + 1);
	p[MAX_HOSTNAME_LENGTH + 1] = '\0';
	chk_ihv(p, "is too long");
	free(p);

#undef chk_ihv

	unset_env(ENV_HOSTNAME);
}

/******************************************************************************
              Test the executable file with a temporary directory
******************************************************************************/

                             /*** No options ***/

/*
 * test_without_options() - Test the executable without options. Returns 
 * nothing.
 */

static void test_without_options(void)
{
	char *p = NULL, *orig_home = NULL;
	int result;
	struct Entry entry;

	diag("Test without options");

	diag("uuids directory doesn't exist");

	if (init_tempdir())
		return; /* gncov */
	OK_SUCCESS(rmdir(TMPDIR "/uuids"), "rmdir uuids");

	p = allocstr("%s: %s: Could not create log file: No such file or"
	             " directory\n", EXECSTR, logfile);
	if (!p) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}
	tc((chp{ execname, NULL }),
	   "",
	   p,
	   EXIT_FAILURE,
	   "No options, uuids directory doesn't exist");
	free(p);
	p = NULL;

	diag("uuids directory exists");

	OK_SUCCESS(result = mkdir(TMPDIR "/uuids", 0777),
	           "mkdir %s/uuids", TMPDIR);
	if (result) {
		diag("Cannot create uuids/ directory, skipping" /* gncov */
		     " tests: %s", strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}

	init_xml_entry(&entry);
	uc((chp{ execname, NULL }), 1, 0, "No options, uuids directory exists");
	verify_logfile(&entry, 1, "First entry created");
	uc((chp{ execname, NULL }), 1, 0, "Create second entry");
	verify_logfile(&entry, 2, "Entries are added, not replaced");
	delete_logfile();

	diag("A directory exists where the log file should be");

	if (OK_SUCCESS(mkdir(logfile, 0755), "mkdir logfile")) {
		diag("%s(): Cannot create directory in place of" /* gncov */
		      " log file, skipping tests: %s",
		      __func__, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	p = allocstr("%s: %s: Could not open file for read+write: Is a"
	             " directory\n", EXECSTR, logfile);
	if (!p) {
		failed_ok("allocstr()"); /* gncov */
		OK_SUCCESS(rmdir(logfile), /* gncov */
		           "%s():%d: rmdir logfile", __func__, __LINE__);
		goto cleanup; /* gncov */
	}
	tc((chp{ execname, NULL }),
	   "",
	   p,
	   EXIT_FAILURE,
	   "No options, log file is blocked by a directory");
	if (OK_SUCCESS(rmdir(logfile), "rmdir logfile")) {
		diag("%s(): Cannot create directory in place of" /* gncov */
		      " log file, skipping tests: %s",
		      __func__, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	free(p);
	p = NULL;

	diag("No HOME environment variable");

	orig_home = mystrdup(getenv("HOME"));
	if (!orig_home) {
		failed_ok("mystrdup()"); /* gncov */
		goto cleanup; /* gncov */
	}
	if (unset_env("HOME"))
		goto cleanup; /* gncov */
	tc((chp{ execname, NULL }),
	   "",
	   EXECSTR ": HOME environment variable not defined, cannot determine"
	   " name of rcfile\n"
	   EXECSTR ": $SUUID_LOGDIR and $HOME environment variables are not"
	   " defined, cannot create logdir path\n",
	   EXIT_FAILURE,
	   "No options and no HOME");
	OK_FALSE(file_exists(logfile), "Log file doesn't exist");
	if (OK_SUCCESS(setenv("HOME", orig_home, 1),
	               "Restore HOME environment variable")) {
		diag("%s(): Cannot restore HOME environment" /* gncov */
		     " variable: %s", __func__, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}

	diag("%s is defined", ENV_LOGDIR);

	if (set_env(ENV_LOGDIR, TMPDIR "/uuids"))
		goto cleanup; /* gncov */
	uc((chp{ execname, NULL }), 1, 0, "No options, %s is defined",
	                                  ENV_LOGDIR);
	verify_logfile(&entry, 1, "Log file after use with %s", ENV_LOGDIR);
	if (unset_env(ENV_LOGDIR))
		goto cleanup; /* gncov */

cleanup:
	free(p);
	free(orig_home);
	cleanup_tempdir(__LINE__);
}

/*
 * test_sess_elements() - Tests that the `<sess>` elements are generated 
 * properly in the XML file. Returns nothing.
 */

static void test_sess_elements(void)
{
	struct Entry entry;

	diag("Test %s environment variable", ENV_SESS);

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	diag("3 UUIDs with descriptions");

	entry.sess[0].desc = "top";
	entry.sess[0].uuid = "22d2a6c4-5cba-11f0-98e7-83850402c3ce";
	entry.sess[1].desc = "middle";
	entry.sess[1].uuid = "31353646-5cba-11f0-a4c5-83850402c3ce";
	entry.sess[2].desc = "bottom";
	entry.sess[2].uuid = "3db1691c-5cba-11f0-8f93-83850402c3ce";

	if (setenv(ENV_SESS, ",top/22d2a6c4-5cba-11f0-98e7-83850402c3ce"
	                     ",middle/31353646-5cba-11f0-a4c5-83850402c3ce"
	                     ",bottom/3db1691c-5cba-11f0-8f93-83850402c3ce,",
	           1))
	        failed_ok("setenv()"); /* gncov */

	uc((chp{ execname, NULL }), 1, 0, "%s with no options", ENV_SESS);
	verify_logfile(&entry, 1, "Log file after %s with no options",
	                          ENV_SESS);
	delete_logfile();

	diag("2 UUIDs separated by \"abc\"");

	if (setenv(ENV_SESS, "22d2a6c4-5cba-11f0-98e7-83850402c3ce"
	                     "abc"
	                     "31353646-5cba-11f0-a4c5-83850402c3ce", 1))
		failed_ok("setenv()"); /* gncov */

	uc((chp{ execname, NULL }), 1, 0,
	   "%s with 2 UUIDs, only separated by 'abc'", ENV_SESS);
	entry.sess[0].desc = NULL;
	entry.sess[1].desc = "abc";
	entry.sess[2].desc = NULL;
	entry.sess[2].uuid = NULL;
	verify_logfile(&entry, 1, "Log file after %s with 2 UUIDs separated"
	                          " with abc", ENV_SESS);

	diag("%s contains no UUIDs", ENV_SESS);

	init_xml_entry(&entry);
	if (setenv(ENV_SESS, "no_uuids", 1))
		failed_ok("setenv()"); /* gncov */
	delete_logfile();

	uc((chp{ execname, NULL }), 1, 0, "%s contains no UUIDs", ENV_SESS);
	verify_logfile(&entry, 1, "Log file after %s with no UUIDs", ENV_SESS);

	diag("Clean up after %s()", __func__);
	unset_env(ENV_SESS);
	cleanup_tempdir(__LINE__);
}

/*
 * test_truncated_logfile() - Contains tests for situations where the log file 
 * is truncated. Returns nothing.
 */

static void test_truncated_logfile(void)
{
	struct Entry entry;
	char *exp_stderr, *contents;
	struct stat sb;

	diag("Log file is truncated");
	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	uc((chp{ execname, NULL }), 1, 0, "Create log file before truncate");
	verify_logfile(&entry, 1, "Log file is ok before it's truncated");

	if (stat(logfile, &sb)) {
		failed_ok("stat()"); /* gncov */
		return; /* gncov */
	}
	OK_SUCCESS(truncate(logfile,
	                    sb.st_size - (off_t)strlen("\n</suuids>")),
	                    "Truncate log file");
	exp_stderr = allocstr("%s: %s: Unknown end line, adding to end of"
	                      " file\n", EXECSTR, logfile);
	if (!exp_stderr) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}
	tc((chp{ execname, NULL }),
	   NULL,
	   exp_stderr,
	   EXIT_SUCCESS,
	   "Add to truncated log file");
	verify_logfile(&entry, 2, "Log file is ok after truncation");
	delete_logfile();

	OK_NOTNULL(create_file(logfile, NULL), "Create empty log file");
	uc((chp{ execname, NULL }), 1, 0, "Add to empty log file");
	verify_logfile(&entry, 1, "Log file header was generated");
	delete_logfile();

	diag("Initial log file has only 1 byte");

	OK_NOTNULL(create_file(logfile, "a"), "Create file with 1 byte");
	tc((chp{ execname, NULL }),
	   NULL,
	   exp_stderr,
	   EXIT_SUCCESS,
	   "Add to log file containing 1 char");

	free(exp_stderr);

	contents = read_from_file(logfile);
	if (!contents) {
		failed_ok("read_from_file(logfile)"); /* gncov */
		return; /* gncov */
	}
	OK_STRNCMP(contents, "a<suuid t=\"", 11,
	           "Start of log file is as expected");
	free(contents);
	cleanup_tempdir(__LINE__);
}

                            /*** -c/--comment ***/

/*
 * chk_comment() - Used by test_comment_option(). Verifies that the comment 
 * `cmt` is stored in the log file as something that matches the regexp 
 * `regexp`. `desc` is a short test description. Returns nothing.
 */

static void chk_comment(const int linenum, char *cmt, char *regexp,
                        const char *desc)
{
	struct streams ss;
	struct Options opt = opt_struct();
	char *s;
	struct Entry entry;

	assert(cmt);
	assert(regexp);
	assert(desc);

	uc_func(linenum, (chp{ execname, "-c", cmt, NULL }), 1, 0,
	        "%s, -c option", desc);

	streams_init(&ss);
	s = allocstr("%s from stdin", desc);
	if (!s) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	binbuf_allocstr(&ss.in, "%s", cmt);
	streams_exec(&opt, &ss, chp{ execname, "--comment", "-", NULL });
	if (!ss.out.buf || !ss.err.buf) {
		failed_ok("ss.out.buf or ss.err.buf is NULL," /* gncov */
		          " streams_exec()");
		goto cleanup; /* gncov */
	}
	uc_check(linenum, "out", ss.out.buf, 1, "%s from stdin", desc);
	OK_STRCMP(ss.err.buf, "", "%s (stderr)", s);
	print_gotexp(ss.err.buf, "");
	OK_EQUAL(ss.ret, EXIT_SUCCESS, "%s (retval)", s);
	init_xml_entry(&entry);
	entry.txt = regexp;
	verify_logfile(&entry, 2, "%s (log file)", desc);
	delete_logfile_func(linenum);

cleanup:
	streams_free(&ss);
	free(s);
}

/*
 * chk_inv_comment() - Used by test_comment_option(). `cmt` is a comment with 
 * invalid characters (for example text that's not in UTF-8), and `desc` is a 
 * short test description. Returns nothing.
 */

static void chk_inv_comment(const int linenum, char *cmt, const char *desc)
{
	struct streams ss;
	struct Options opt = opt_struct();
	char *s;

	assert(cmt);
	assert(desc);

	s = str_replace(EXECSTR ": Comment contains illegal characters or is"
	                " not valid UTF-8\n", EXECSTR, execname);
	if (!s) {
		failed_ok("str_replace()"); /* gncov */
		return; /* gncov */
	}

	tc_func(linenum, (chp{ execname, "-c", cmt, NULL }),
	        "",
	        s,
	        EXIT_FAILURE,
	        "%s, -c option", desc);

	streams_init(&ss);
	binbuf_allocstr(&ss.in, "%s", cmt);
	streams_exec(&opt, &ss, chp{ execname, "--comment", "-", NULL });
	if (!ss.out.buf || !ss.err.buf) {
		failed_ok("ss.out.buf or ss.err.buf is NULL," /* gncov */
		          " streams_exec()");
		goto cleanup; /* gncov */
	}
	OK_STRCMP_L(ss.out.buf, "", linenum, "%s from stdin (stdout)", desc);
	print_gotexp(ss.out.buf, "");
	OK_STRCMP_L(ss.err.buf, s, linenum, "%s from stdin (stderr)", desc);
	print_gotexp(ss.err.buf, s);
	OK_EQUAL_L(ss.ret, EXIT_FAILURE, linenum, "%s from stdin (retval)", desc);

cleanup:
	streams_free(&ss);
	free(s);
}

/*
 * test_comment_option() - Tests the -c/--comment option. Returns nothing.
 */

static void test_comment_option(void)
{
	struct Entry entry;

	diag("Test -c/--comment");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	uc((chp{ execname, "-c", "Great test", NULL }), 1, 0,
	   "-c \"Great test\"");
	entry.txt = "Great test";
	verify_logfile(&entry, 1, "-c created 1 entry");

	entry.txt = "(Great test|Another one)";
	uc((chp{ execname, "--comment", "Another one", NULL }), 1, 0,
	   "--comment");
	verify_logfile(&entry, 2, "--comment created 1 entry");
	delete_logfile();

#define chk_comment(cmt, regexp, desc)  chk_comment(__LINE__, (cmt), \
                                                    (regexp), (desc))
	chk_comment("", "", "Empty comment");
	chk_comment(" Initial space", "Initial space",
	            "Comment with initial space");
	chk_comment("\nInitial newline", "Initial newline",
	            "Comment with initial newline");
	chk_comment("Trailing space ", "Trailing space",
	            "Comment with trailing space");
	chk_comment("Trailing newline\n", "Trailing newline",
	            "Comment with trailing newline");
	chk_comment("\xe0\xad\xb2", "\xe0\xad\xb2",
	            "Comment with 3-byte UTF-8 sequence");
	chk_comment("\xf0\x9d\x85\x9d", "\xf0\x9d\x85\x9d",
	            "Comment with 4-byte UTF-8 sequence");
	chk_comment("Special chars: < & > \\ \n \t end",
	            "Special chars: &lt; &amp; &gt; \\\\\\\\ \\\\n \\\\t end",
	            "Special chars: < & > \\ \\n \\t");
#undef chk_comment

	diag("Invalid characters in comment");

#define chk_inv_comment(cmt, desc)  chk_inv_comment(__LINE__, (cmt), (desc))
	chk_inv_comment("F\xf8kka \xf8pp", "Comment with latin1 chars");
	chk_inv_comment("Ctrl-d: \x04", "Reject Ctrl-d in comment");
	chk_inv_comment("\x7f", "Reject U+007F (DELETE) in comment");
	chk_inv_comment("\xc1\xbf", "Overlong UTF-8 char in comment, 2 bytes");
	chk_inv_comment("\xe0\x80\xaf",
	                "Overlong UTF-8 char in comment, 3 bytes");
	chk_inv_comment("\xf0\x80\x80\xaf",
	                "Overlong UTF-8 char in comment, 4 bytes");
	chk_inv_comment("\xed\xa0\x80",
	                "UTF-8 contains UTF-16 surrogate char U+D800");
	chk_inv_comment("\xef\xbf\xbe", "UTF-8 contains U+FFFE");
	chk_inv_comment("\xef\xbf\xbf", "UTF-8 contains U+FFFF");
	chk_inv_comment("\xf4\x90\x80\x80",
	                "UTF-8 contains U+110000, is above U+10FFFF");
	OK_FALSE(file_exists(logfile),
	         "The log file doesn't exist after the invalid comments");
#undef chk_inv_comment

	cleanup_tempdir(__LINE__);
}

/*
 * read_long_text_from_stdin() - Reads a long string (120000 bytes, "00001 
 * 00002 [...] 19999 20000") from stdin. Returns nothing.
 */

static void read_long_text_from_stdin(void)
{
	struct Entry entry;
	char *buf, *p;
	const char *desc = "Read monster string from stdin";
	unsigned long l;
	struct streams ss;
	struct Options o = opt_struct();

	diag("Read long text (120000 bytes) from stdin");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	buf = malloc(120001);
	if (!buf) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	p = buf;
	for (l = 1; l <= 20000; l++) {
		snprintf(p, 7, "%05lu ", l);
		p += 6;
	}
	streams_init(&ss);
	binbuf_allocstr(&ss.in, "%s", buf);
	OK_EQUAL(streams_exec(&o, &ss, chp{ execname, "-c", "-", NULL }),
	         EXIT_SUCCESS, "%s (retval)", desc);
	free(buf);
	OK_EQUAL(count_uuids(ss.out.buf, "\n"), 1, "%s (stdout)", desc);
	OK_STRCMP(ss.err.buf, "", "%s (stderr)", desc);

	buf = read_from_file(logfile);
	if (!buf) {
		failed_ok("read_from_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NOTNULL(strstr(buf, "<txt>00001 00002 00003 00004 00005 "),
	           "Start of string is in the log file");
	OK_NOTNULL(strstr(buf, "10001 10002 10003 10004 10005 "),
	           "Middle of string is in the log file");
	OK_NOTNULL(strstr(buf, "19995 19996 19997 19998 19999 20000</txt>"),
	           "End of string is in the log file");

cleanup:
	free(buf);
	streams_free(&ss);
	cleanup_tempdir(__LINE__);
}

/*
 * external_editor_with_vars() - Runs the external editor with the 2 
 * environment variables it checks for. `ENV_EDITOR` is set to the string in 
 * `suuid_editor`, and "EDITOR" is set to the string in `editor`. `fake` is the 
 * value of the `fake_editor` script in test_external_editor(). Returns 
 * nothing.
 */

static void external_editor_with_vars(const int linenum,
                                      const char *suuid_editor,
                                      const char *editor, const char *fake)
{
	char *vars, *desc = NULL, *exp_stderr = NULL;
	struct Entry entry;

	assert(suuid_editor);
	assert(editor);
	assert(fake);

#define e_var(v)  (strlen(v) ? (!strcmp((v), fake) ? "fake" : "non-existing") \
                             : "empty")
	vars = allocstr("%s=%s, EDITOR=%s",
	                ENV_EDITOR, e_var(suuid_editor), e_var(editor));
	if (!vars) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}

	desc = allocstr("External editor, %s", vars);
	if (!desc) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}
	diag("%s", desc);

	if (suuid_editor && set_env_func(linenum, ENV_EDITOR, suuid_editor))
		goto cleanup; /* gncov */
	if (editor && set_env_func(linenum, "EDITOR", editor))
		goto cleanup; /* gncov */

	uc_func(linenum, (chp{ execname, "-c", "--", NULL }), 1, 0, "%s",
	        desc);
	init_xml_entry(&entry);
	entry.txt = "Text from editor";
	verify_logfile_func(linenum, &entry, 1, "%s (Log file)", desc);

cleanup:
	if (suuid_editor)
		unset_env_func(linenum, ENV_EDITOR);
	if (editor)
		unset_env_func(linenum, "EDITOR");
	delete_logfile_func(linenum);
	free(exp_stderr);
	free(desc);
	free(vars);
#undef e_var
}

/*
 * external_editor_missing_vars() - Emulate a situation when the external 
 * editor can't be executed because both the `ENV_EDITOR` or "EDITOR" variables 
 * are undefined or empty. Returns nothing.
 */

static void external_editor_missing_vars(const int linenum,
                                         const char *suuid_editor,
                                         const char *editor)
{
	char *vars, *desc = NULL, *exp_stderr = NULL;

	vars = allocstr("%s=\"%s\", EDITOR=\"%s\"",
	                ENV_EDITOR, no_null(suuid_editor), no_null(editor));
	if (!vars) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}

	diag("External editor missing variables, %s", vars);
	desc = allocstr("External editor fails, %s", vars);
	if (!desc) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}

	if (suuid_editor && set_env_func(linenum, ENV_EDITOR, suuid_editor))
		goto cleanup; /* gncov */
	if (editor && set_env_func(linenum, "EDITOR", editor))
		goto cleanup; /* gncov */

	exp_stderr = allocstr("%s: Environment variables %s and EDITOR aren't"
	                      " defined, cannot start editor\n",
	                      EXECSTR, ENV_EDITOR);
	if (!exp_stderr) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}

	tc_func(linenum, (chp{ execname, "-c", "--", NULL }),
	        "",
	        exp_stderr,
	        EXIT_FAILURE,
	        "%s", desc);

cleanup:
	if (suuid_editor)
		unset_env_func(linenum, ENV_EDITOR);
	if (editor)
		unset_env_func(linenum, "EDITOR");
	free(exp_stderr);
	free(desc);
	free(vars);
}

/*
 * test_external_editor() - Tests the use of an external editor when the 
 * argument to -c/--comment is "--". Returns nothing.
 */

static void test_external_editor(void)
{
	const char *p, *fake_editor = TMPDIR "/fake-editor",
	           *nonexisting = TMPDIR "/non-existing";
	int i;

	diag("Test external editor");

	if (init_tempdir())
		return; /* gncov */

	OK_NOTNULL(p = create_file(fake_editor,
	                           "#!/bin/sh\n"
	                           "\n"
	                           "echo Text from editor >\"$1\"\n"),
	           "Create %s", fake_editor);
	if (!p) {
		diag("%s: Cannot create file: %s", /* gncov */
		     fake_editor, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	OK_SUCCESS(i = chmod(fake_editor, 0755),
	           "Make %s executable", fake_editor);
	if (i) {
		diag("%s: Cannot make file executable: %s", /* gncov */
		     fake_editor, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}

	if (unset_env(ENV_EDITOR))
		goto cleanup; /* gncov */
	if (unset_env("EDITOR"))
		goto cleanup; /* gncov */

#define external_editor_with_vars(suuid_editor, editor, fake)  \
        external_editor_with_vars(__LINE__, (suuid_editor), (editor), (fake))

	external_editor_with_vars("", fake_editor, fake_editor);
	external_editor_with_vars(fake_editor, "", fake_editor);
	external_editor_with_vars(fake_editor, fake_editor, fake_editor);
	external_editor_with_vars(fake_editor, nonexisting, fake_editor);
#undef external_editor_with_vars

#define external_editor_missing_vars(suuid_editor, editor)  \
        external_editor_missing_vars(__LINE__, (suuid_editor), (editor))

	external_editor_missing_vars("", "");
	external_editor_missing_vars("", NULL);
	external_editor_missing_vars(NULL, "");
	external_editor_missing_vars(NULL, NULL);
#undef external_editor_missing_vars

cleanup:
	if (file_exists(fake_editor)) {
		i = remove(fake_editor);
		OK_SUCCESS(i, "Delete fake editor script");
		if (i) {
			diag("test %d: errno = %d (%s)", /* gncov */
			     testnum, errno, strerror(errno)); /* gncov */
			errno = 0; /* gncov */
		}
	}
	unset_env(ENV_EDITOR);
	unset_env("EDITOR");
	cleanup_tempdir(__LINE__);
}

/*
 * test_unreadable_editor_file() - Simulate a situation when the external 
 * editor saves a file with wrong permissions, making it unreadable. Returns 
 * nothing.
 */

static void test_unreadable_editor_file(void)
{
	const char *p, *fake_editor = TMPDIR "/fake-editor", *desc;
	char *errdup = NULL, *tempfile = NULL, *colon;
	int i;
	struct streams ss;
	struct Options opt = opt_struct();
	size_t err_offs;

	desc = "The editor saves an unreadable file";

	if (is_root()) {
		diag("%s # SKIP Running as root", desc); /* gncov */
		return; /* gncov */
	}

	diag("%s", desc);

	if (init_tempdir())
		return; /* gncov */

	streams_init(&ss);
	OK_NOTNULL(p = create_file(fake_editor,
	                           "#!/bin/sh\n"
	                           "\n"
	                           "echo Text from editor >\"$1\"\n"
	                           "chmod 000 \"$1\""),
	           "Create %s, makes the saved file unreadable", fake_editor);
	if (!p) {
		failed_ok("create_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_SUCCESS(i = chmod(fake_editor, 0755),
	           "%s(): Make %s executable", __func__, fake_editor);
	if (i) {
		diag("%s(): %s: Cannot make file executable: %s", /* gncov */
		     __func__, fake_editor, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	if (set_env(ENV_EDITOR, fake_editor))
		goto cleanup; /* gncov */

	desc = "\"--comment --\" with unreadable file";
	streams_exec(&opt, &ss, chp{ execname, "--comment", "--", NULL });
	OK_STRCMP(ss.out.buf, "", "%s (stdout)", desc);
	print_gotexp(ss.out.buf, "");
	OK_NOTNULL(strstr(ss.err.buf,
	           ": Cannot read comment from file: Permission denied\n"),
	           "%s (stderr)", desc);
	OK_EQUAL(ss.ret, EXIT_FAILURE, "%s (retval)", desc);
	OK_FALSE(file_exists(logfile),
	         "Log file doesn't exist after unreadable file");
	errdup = mystrdup(ss.err.buf);
	if (!errdup) {
		failed_ok("mystrdup()"); /* gncov */
		goto cleanup; /* gncov */
	}
	err_offs = strlen(execname) + strlen(": ");
	if (strlen(errdup) <= err_offs) {
		OK_ERROR("%s(): stderr is too short to contain" /* gncov */
		         " tempfile name", __func__);
		goto cleanup; /* gncov */
	}
	tempfile = errdup + err_offs;
	colon = strstr(tempfile, ": ");
	if (!colon) {
		OK_ERROR("Didn't find colon after tempfile name" /* gncov */
		         " in stderr");
		goto cleanup; /* gncov */
	}
	*colon = '\0';
	OK_TRUE(file_exists(tempfile), "%s(): Tempfile exists", __func__);

cleanup:
	streams_free(&ss);
	if (file_exists(fake_editor)) {
		i = remove(fake_editor);
		OK_SUCCESS(i, "%s(): Delete fake editor script", __func__);
		if (i) {
			diag("test %d: errno = %d (%s)", /* gncov */
			     testnum, errno, strerror(errno)); /* gncov */
			errno = 0; /* gncov */
		}
	}
	if (tempfile && file_exists(tempfile)) {
		OK_SUCCESS(i = chmod(tempfile, 0555),
		           "Make tempfile writable");
		if (i)
			failed_ok("chmod()"); /* gncov */
		if (OK_SUCCESS(remove(tempfile),
		               "%s(): Delete tempfile", __func__)) {
			diag("%s: Cannot remove file: %s", /* gncov */
			     tempfile, strerror(errno)); /* gncov */
		}
	}
	free(errdup);
	unset_env(ENV_EDITOR);
	cleanup_tempdir(__LINE__);
}

/*
 * test_nonexisting_editor() - Tests the behavior when the external editor 
 * doesn't exist. Returns nothing.
 */

static void test_nonexisting_editor(void)
{
	struct streams ss;
	struct Options o = opt_struct();
	const char *desc = "External editor not found",
	           *searchstr = "File contents is stored in temporary file ";
	char *tempfile, *p;

	diag("Non-existing editor");
	if (init_tempdir())
		return; /* gncov */

	streams_init(&ss);
	if (unset_env(ENV_EDITOR))
		goto cleanup; /* gncov */
	if (set_env("EDITOR", TMPDIR "/doesnt-exist"))
		goto cleanup; /* gncov */

	streams_exec(&o, &ss, chp{ execname, "-c", "--", NULL });
	OK_STRCMP(ss.out.buf, "", "%s (stdout)", desc);
	print_gotexp(ss.out.buf, "");
	OK_NOTNULL(tempfile = strstr(ss.err.buf, searchstr),
	           "%s (stderr)", desc);
	OK_EQUAL(ss.ret, EXIT_FAILURE, "%s (retval)", desc);
	if (!tempfile)
		goto cleanup; /* gncov */
	tempfile += strlen(searchstr);
	p = strstr(tempfile, "\n");
	if (!p) {
		OK_ERROR("%s(): Newline not found in tempfile" /* gncov */
		         " string", __func__);
		goto cleanup; /* gncov */
	}
	*p = '\0';
	OK_TRUE(file_exists(tempfile), "Tempfile from editor exists");
	OK_SUCCESS(remove(tempfile), "Delete tempfile from editor");

cleanup:
	streams_free(&ss);
	cleanup_tempdir(__LINE__);
}

                             /*** -n/--count ***/

/*
 * test_count_option() - Tests the -n/--count option. Returns nothing.
 */

static void test_count_option(void)
{
	struct Entry entry;

	diag("Test -n/--count");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	uc((chp{ execname, "-n", "10", NULL }), 10, 0, "-n 10");
	verify_logfile(&entry, 10, "-n created 10 entries");

	uc((chp{ execname, "--count", "20", NULL }), 20, 0, "--count 20");
	verify_logfile(&entry, 30, "--count created 20 entries");

	tc((chp{ execname, "-n", "y", NULL }),
	   "",
	   EXECSTR ": Error in -n/--count argument\n"
	   OPTION_ERROR_STR,
	   EXIT_FAILURE,
	   "-n with invalid argument");

	tc((chp{ execname, "--count", "", NULL }),
	   "",
	   EXECSTR ": Error in -n/--count argument\n"
	   OPTION_ERROR_STR,
	   EXIT_FAILURE,
	   "--count with empty argument");

	cleanup_tempdir(__LINE__);
}

                             /*** -l/--logdir ***/

/*
 * test_logdir_option() - Tests the -l/--logdir option. Returns nothing.
 */

static void test_logdir_option(void)
{
	struct Entry entry;
	int result;
	char *bck_home = NULL;
	struct Rc rc;

	diag("Test -l/--logdir");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	OK_SUCCESS(result = rename(TMPDIR "/uuids", TMPDIR "/uuids2"),
	           "rename uuids/ to uuids2/");
	if (result) {
		OK_ERROR("Cannot rename uuids/ to uuids2/" /* gncov */
		         " directory, skipping tests: %s",
		         strerror(errno));
		errno = 0; /* gncov */
		return; /* gncov */
	}

	entry.txt = "With -l/--logdir";
	uc((chp{ execname, "-l", TMPDIR "/uuids2", "-c", "With -l/--logdir",
	         NULL }),
	   1, 0, "\"-l " TMPDIR "/uuids2\"");

	uc((chp{ execname, "--logdir", TMPDIR "/uuids2", "-c",
	         "With -l/--logdir", NULL }), 1, 0,
	        "\"--logdir " TMPDIR "/uuids2\"");

	OK_SUCCESS(result = rename(TMPDIR "/uuids2", TMPDIR "/uuids"),
	           "Rename uuids2/ back to uuids/");
	if (result) {
		OK_ERROR("Cannot rename uuids2/ to uuids/: %s", /* gncov */
		         strerror(errno));
		errno = 0; /* gncov */
		return; /* gncov */
	}

	verify_logfile(&entry, 2, "Log file, \"-l " TMPDIR "/uuids2\"");
	delete_logfile();

	tc((chp{ execname, "--logdir", "", NULL }),
	   "",
	   EXECSTR ": -l/--logdir argument cannot be empty\n"
	   OPTION_ERROR_STR,
	   EXIT_FAILURE,
	   "Empty argument to --logdir");

	diag("No HOME envvar, but -l is used");
	OK_NOTNULL(getenv("HOME"), "getenv(\"HOME\") doesn't return NULL");
	bck_home = mystrdup(getenv("HOME"));
	if (!bck_home) {
		failed_ok("mystrdup()"); /* gncov */
		goto cleanup; /* gncov */
	}
	unset_env("HOME");

	tc((chp{ execname, "-l", TMPDIR "/uuids", NULL }),
	   NULL,
	   EXECSTR ": HOME environment variable not defined, cannot determine"
	   " name of rcfile\n",
	   EXIT_SUCCESS,
	   "-l is used with undefined HOME");

	if (OK_SUCCESS(setenv("HOME", bck_home, 1), "Restore HOME envvar")) {
		failed_ok("setenv()"); /* gncov */
		goto cleanup; /* gncov */
	}
	init_rc(&rc);
	entry.host = get_hostname(&rc);
	entry.txt = NULL;
	if (!entry.host)
		failed_ok("get_hostname()"); /* gncov */
	else
		verify_logfile(&entry, 1, "Log file after -l without HOME");

cleanup:
	free(bck_home);
	cleanup_tempdir(__LINE__);
}

                           /*** -m/--random-mac ***/

/*
 * chk_unique_macs() - Check that all `num` newline-separated UUID strings in 
 * the string `s` have unique or identical MAC addresses.
 *
 * `mode`:
 * - CHECK_UNIQUE: Check that all MAC addresses are unique.
 * - CHECK_EQUAL: Check that all MAC addresses are identical.
 *
 * Returns:
 * - 0 if the condition is met
 * - 1 if the condition is not met
 * - -1 on error (`s` is NULL)
 */

int chk_unique_macs(const char *s, unsigned int num, enum cum_mode mode)
{
	char macs[num][13];
	unsigned int count = 0;
	const char *line_start = s;

	assert(s);

	if (!s)
		return -1; /* gncov */

	if (!num)
		return 0; /* gncov */

	while (*line_start && count < num) {
		const char *line_end = strchr(line_start, '\n');
		size_t line_len = line_end ? (size_t)(line_end - line_start)
		                           : strlen(line_start);

		/* Extract last 12 characters as MAC string */
		const char *mac_start = line_start + line_len - 12;

		/* Copy MAC substring */
		memcpy(macs[count], mac_start, 12);
		macs[count][12] = '\0';

		if (mode == CHECK_UNIQUE) {
			unsigned int i;

			/* Check for duplicates among previous MACs */
			for (i = 0; i < count; i++) {
				if (strcmp(macs[count], macs[i]) == 0) {
					/* Duplicate found */
					return 1; /* gncov */
				}
			}
		} else {
			/* Check if current MAC differs from first MAC */
			if (count > 0 /* gncov */
			    && strcmp(macs[count], macs[0]) != 0) { /* gncov */
				/* Difference found */
				return 1; /* gncov */
			}
		}

		count++;
		if (!line_end)
			break; /* gncov */
		line_start = line_end + 1;
	}

	return 0;
}

/*
 * test_unique_macs() - Verifies that -m/--random-mac creates unique MAC 
 * addresses. Returns nothing.
 */

static void test_unique_macs(void)
{
	struct streams ss;
	struct Options opt = opt_struct();
	unsigned int num = 100;
	char *num_s;

	num_s = allocstr("%u", num);
	if (!num_s) {
		failed_ok("allocstr()"); /* gncov */
		return; /* gncov */
	}

	streams_init(&ss);
	streams_exec(&opt, &ss, chp{ execname, "-m", "-n", num_s, NULL });

	if (opt.verbose >= 1) {
		diag("ss.out.buf = \"%s\"", no_null(ss.out.buf)); /* gncov */
		diag("ss.err.buf = \"%s\"", no_null(ss.err.buf)); /* gncov */
		diag("ss.ret = \"%d\"", ss.ret); /* gncov */
	}

	OK_SUCCESS(chk_unique_macs(ss.out.buf, num, CHECK_UNIQUE),
	           "-m -n %u, all MAC addresses are unique", num);

	streams_free(&ss);
	free(num_s);
}

/*
 * test_random_mac_option() - Tests the -m/--random-mac option. Returns 
 * nothing.
 */

static void test_random_mac_option(void)
{
	struct Entry entry;
	int result;
	struct Rc rc;
	char *s = NULL;

	diag("Test -m/--random-mac");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	init_rc(&rc);
	rc.hostname = HNAME;
	rc.macaddr = "897feb323faa";

	OK_SUCCESS(result = create_rcfile(rcfile, &rc),
	           "%s():%d: Create rcfile", __func__, __LINE__);
	if (result) {
		diag("%s(): Cannot create rcfile \"%s\", skipping" /* gncov */
		     " tests: %s",
		     __func__, rcfile, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return; /* gncov */
	}

	uc((chp{ execname, NULL }), 1, 0, "Use macaddr from the rc file");
	s = read_from_file(logfile);
	if (!s) {
		failed_ok("read_from_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NOTNULL(strstr(s, rc.macaddr), "rc.macaddr exists in the log file");
	free(s);
	s = NULL;
	delete_logfile();

	uc((chp{ execname, "-m", NULL }), 1, 0, "Use \"-m\"");
	s = read_from_file(logfile);
	if (!s) {
		failed_ok("read_from_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NULL(strstr(s, rc.macaddr),
	        "rc.macaddr doesn't exist in the log file after -m");
	free(s);
	s = NULL;
	delete_logfile();

	uc((chp{ execname, "--random-mac", NULL }), 1, 0,
	   "Use \"--random-mac\"");
	s = read_from_file(logfile);
	if (!s) {
		failed_ok("read_from_file()"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NULL(strstr(s, rc.macaddr),
	        "rc.macaddr doesn't exist in the log file after --random-mac");
	free(s);
	s = NULL;

	test_unique_macs();

cleanup:
	free(s);
	cleanup_tempdir(__LINE__);
}

                                /*** --raw ***/

/*
 * test_raw_option() - Tests the --raw option. Returns nothing.
 */

static void test_raw_option(void)
{
	struct Entry entry;

	diag("Test --raw");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	entry.txt = "Raw data";
	uc((chp{ execname, "--raw", "-c", "Raw data", NULL }), 1, 0,
	   "--raw -c \"Raw data\"");
	verify_logfile(&entry, 1, "--raw with plain text has no surrounding"
	                          " spaces");
	delete_logfile();

	entry.txt = " <dingle><dangle>bær</dangle></dingle> ";
	uc((chp{ execname, "--raw", "-c",
	         "<dingle><dangle>bær</dangle></dingle>", NULL }),
	   1, 0, "--raw -c \"<dingle><dangle>bær</dangle></dingle>\"");
	verify_logfile(&entry, 1, "XML with --raw gets surrounding spaces");
	delete_logfile();

	entry.txt = "abc<raw>Raw data</raw>def";
	uc((chp{ execname, "--raw", "-c", "abc<raw>Raw data</raw>def", NULL }),
	   1, 0, "--raw -c \"abc<raw>Raw data</raw>def\"");
	verify_logfile(&entry, 1, "--raw: XML with surrounding plain text gets"
	                          " no surrounding spaces");
	delete_logfile();

	cleanup_tempdir(__LINE__);
}

                              /*** --rcfile ***/

/*
 * test_rcfile_option() - Tests the --rcfile option. Returns nothing.
 */

static void test_rcfile_option(void)
{
	struct Entry entry;
	struct Rc rc;
	char *rc2 = TMPDIR "/rc2", *s = NULL;

	diag("Test --rcfile");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	init_rc(&rc);
	rc.hostname = HNAME;
	rc.macaddr = "ed604660c349";

	if (OK_SUCCESS(create_rcfile(rc2, &rc),
	               "Create rc file \"%s\"", rc2)) {
	        diag("%s():%d: %s: Cannot create rc file: %s", /* gncov */
	             __func__, __LINE__, rc2, strerror(errno)); /* gncov */
	        errno = 0; /* gncov */
	        goto cleanup; /* gncov */
	}

	uc((chp{ execname, "--rcfile", rc2, NULL }), 1, 0, "--rcfile %s", rc2);
	s = read_from_file(logfile);
	if (!s) {
		failed_ok("read_from_file(logfile)"); /* gncov */
		goto cleanup; /* gncov */
	}
	OK_NOTNULL(strstr(s, rc.macaddr),
	           "The correct MAC address is in the log file");

	if (set_env(ENV_HOSTNAME, HNAME))
		goto cleanup; /* gncov */
	uc((chp{ execname, "--rcfile", TMPDIR "/nosuchrc", NULL }), 1, 0,
	   "--rcfile %s/nosuchrc", TMPDIR);
	if (unset_env(ENV_HOSTNAME))
		goto cleanup; /* gncov */
	verify_logfile(&entry, 2,
	               "Log file after missing rc file has 2 entries");

	rc.macaddr = "";
	if (OK_SUCCESS(create_rcfile(rc2, &rc),
	               "Create rc file \"%s\" with empty MAC address", rc2)) {
		diag("%s():%d: %s: Cannot create rc file: %s", /* gncov */
		     __func__, __LINE__, rc2, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	uc((chp{ execname, "--rcfile", rc2, NULL }), 1, 0,
	   "--rcfile %s with empty MAC address", rc2);
	verify_logfile(&entry, 3,
	               "Log file after empty MAC address in the rc file");

	if (OK_NOTNULL(create_file(rc2, "hostname = " HNAME "\nmacaddr =\n"),
	               "Create rc file \"%s\" with empty MAC address and no"
	               " terminating space character after \"=\"", rc2)) {
		diag("%s():%d: %s: Cannot create rc file: %s", /* gncov */
		     __func__, __LINE__, rc2, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		goto cleanup; /* gncov */
	}
	uc((chp{ execname, "--rcfile", rc2, NULL }), 1, 0,
	   "--rcfile %s with empty MAC address and no terminating space", rc2);
	verify_logfile(&entry, 4, "Log file after empty MAC address and no"
	                          " space in the rc file");
	delete_logfile();

	if (set_env(ENV_HOSTNAME, HNAME))
		goto cleanup; /* gncov */
	uc((chp{ execname, "--rcfile", "", NULL }), 1, 0,
	   "Empty argument to --rcfile");
	verify_logfile(&entry, 1, "Log file after empty argument to --rcfile");

cleanup:
	unset_env(ENV_HOSTNAME);
	free(s);
	if (file_exists(rc2))
		OK_SUCCESS(remove(rc2), "Delete rc file %s", rc2);
	cleanup_tempdir(__LINE__);
}

                              /*** -t/--tag ***/

/*
 * test_tag_option() - Tests the -t/--tag option. Returns nothing.
 */

static void test_tag_option(void)
{
	struct Entry entry;

	diag("Test -t/--tag");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	entry.tag[0] = "snaddertag";

	uc((chp{ execname, "-t", entry.tag[0], NULL }), 1, 0, "1 single tag");
	verify_logfile(&entry, 1, "Log file after 1 single tag");

	uc((chp{ execname, "--tag", "   snaddertag   ", NULL }), 1, 0,
	   "1 single tag with surrounding spaces");
	verify_logfile(&entry, 2,
	               "Log file after 1 single tag with surrounding spaces");

	delete_logfile();

	memcpy(entry.tag, chp{ "first", "second", "third", NULL },
	       sizeof(char *) * 4);

	uc((chp{ execname, "-t", "first,second,third", NULL }), 1, 0,
	   "\"-t first,second,third\"");
	verify_logfile(&entry, 1, "Log file after \"-t first,second,third\"");

	uc((chp{ execname, "--tag", "first,second,third", NULL }), 1, 0,
	   "\"--tag first,second,third\"");
	verify_logfile(&entry, 2,
	               "Log file after \"--tag first,second,third\"");

	uc((chp{ execname, "-t", "first", "-t", "second", "--tag", "third", NULL }),
	   1, 0, "\"-t first -t second --tag third\"");
	verify_logfile(&entry, 3, "Log file after \"-t first -t second --tag"
	                          " third\"");

	uc((chp{ execname, "-t", "first", "--tag", "", "-t", "second", "--tag",
	         "third", NULL }),
	   1, 0, "Empty tag between first and second");
	verify_logfile(&entry, 4, "Log file after empty tag");

	uc((chp{ execname, "--tag", ",,first,,,second,,,third,,,", NULL }), 1, 0,
	   "Empty comma-separated tags");
	verify_logfile(&entry, 5, "Log file after empty comma-separated tags");

	uc((chp{ execname, "-t", "first", "-t", "second", "-t", "third",
	         "-t", "first", "-t", "second", "-t", "third", NULL }),
	   1, 0, "Duplicated tags");
	verify_logfile(&entry, 6, "Log file after duplicated tags");

	uc((chp{ execname, "--tag", "first,second,third,first,second", NULL }),
	   1, 0, "Duplicated comma-separated tags");
	verify_logfile(&entry, 7,
	               "Log file after duplicated comma-separated tags");

	delete_logfile();
	init_xml_entry(&entry);
	entry.tag[0] = "Ḡṹṛḡḷḗ";

	uc((chp{ execname, "--tag", "Ḡṹṛḡḷḗ", NULL }), 1, 0, "UTF-8 tag");
	verify_logfile(&entry, 1, "Log file after UTF-8 tag");

	tc((chp{ execname, "-t", "schn\xfc" "ffelhund", NULL }),
	   "",
	   EXECSTR ": Tags have to be in UTF-8\n",
	   EXIT_FAILURE,
	   "Invalid UTF-8 in tag");
	verify_logfile(&entry, 1, "Log file after invalid UTF-8 in tag");

	delete_logfile();
	init_xml_entry(&entry);
	entry.tag[0] = "tag with spaces";

	uc((chp{ execname, "--tag", "tag with spaces", NULL }), 1, 0,
	   "Tag with spaces");
	verify_logfile(&entry, 1, "Log file, tag with spaces");

	uc((chp{ execname, "--tag", "   tag with spaces   ", NULL }), 1, 0,
	   "Tag with spaces and surrounding space");
	verify_logfile(&entry, 2,
	               "Log file, tag with spaces and surrounding space");

	delete_logfile();
	init_xml_entry(&entry);
	entry.tag[0] = "tag\\\\twith\\\\ttabs";

	uc((chp{ execname, "--tag", "tag\twith\ttabs", NULL }), 1, 0,
	   "Tag with tabs");
	verify_logfile(&entry, 1, "Log file, tag with tabs");

	delete_logfile();
	init_xml_entry(&entry);
	entry.tag[0] = "tag\\\\nwith\\\\nnewlines";

	uc((chp{ execname, "--tag", "tag\nwith\nnewlines", NULL }), 1, 0,
	   "Tag with newlines");
	verify_logfile(&entry, 1, "Log file, tag with newlines");

	delete_logfile();
	init_xml_entry(&entry);
	entry.tag[0] = "&lt;&amp;&gt;";

	uc((chp{ execname, "--tag", "<&>", NULL }), 1, 0,
	   "Tag is \"<&>\"");
	verify_logfile(&entry, 1, "Log file, tag is \"<&>\"");

	cleanup_tempdir(__LINE__);
}

/*
 * test_too_many_tags() - Tests the -t/--tag option with too many tags. Returns 
 * nothing.
 */

static void test_too_many_tags(void)
{
	struct Entry entry;
	unsigned int i;
	size_t tag_count = MAX_TAGS + 1,
	       arrsize = 2 * tag_count + 2;
	char *arr[arrsize], *s = NULL;

	diag("Too many -t/--tag options");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	for (i = 0; i < arrsize; i++)
		arr[i] = NULL;

	arr[0] = mystrdup(execname);
	for (i = 0; i < tag_count; i++) {
		arr[i * 2 + 1] = mystrdup("-t");
		if (!arr[i * 2 + 1]) {
			failed_ok("mystrdup()"); /* gncov */
			goto cleanup; /* gncov */
		}

		arr[i * 2 + 2] = allocstr("%u", i);
		if (!arr[i * 2 + 2]) {
			failed_ok("allocstr()"); /* gncov */
			goto cleanup; /* gncov */
		}
	}

	s = allocstr("%s: Maximum number of tags (%u) exceeded\n%s",
	             EXECSTR, MAX_TAGS, OPTION_ERROR_STR);
	if (!s) {
		failed_ok("allocstr()"); /* gncov */
		goto cleanup; /* gncov */
	}

	tc(arr, "", s, EXIT_FAILURE, "Too many tags");
	OK_FALSE(file_exists(logfile),
	         "Log file doesn't exist after too many tags");

cleanup:
	free(s);
	for (i = 0; i < arrsize; i++)
		free(arr[i]);
	cleanup_tempdir(__LINE__);
}

/*
 * test_too_many_comma_tags() - Tests what happens when too many 
 * comma-separated tags (more than MAX_TAGS) are used with --tag. Returns 
 * nothing.
 */

static void test_too_many_comma_tags(void)
{
	struct Entry entry;
	size_t digits = 1, n = MAX_TAGS + 1, bufsize;
	char *buf, *p, *exp_stderr = NULL;
	unsigned int i;

	diag("Too many comma-separated tags");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	while (n >= 10) {
		digits++;
		n /= 10;
	}
	bufsize = MAX_TAGS * (strlen("t") + digits + strlen(",")) + 1;
	if (bufsize < MAX_TAGS) {
		OK_ERROR("Buffer size overflow for MAX_TAGS=%u", /* gncov */
		         MAX_TAGS);
		return; /* gncov */
	}

	buf = malloc(bufsize);
	if (!buf) {
		failed_ok("malloc()"); /* gncov */
		return; /* gncov */
	}
	*buf = '\0';

	p = buf;
	for (i = 1; i <= MAX_TAGS + 1; i++) {
		p = allocstr("t%u,", i);
		if (!p) {
			failed_ok("allocstr()"); /* gncov */
			goto cleanup; /* gncov */
		}
		strcat(buf, p);
		free(p);
	}

	exp_stderr = allocstr("%s: Maximum number of tags (%u) exceeded\n",
	                      execname, MAX_TAGS);
	tc((chp{ execname, "--tag", buf, NULL }),
	   "",
	   exp_stderr,
	   EXIT_FAILURE,
	   "Too many comma-separated tags");

cleanup:
	free(exp_stderr);
	free(buf);
	cleanup_tempdir(__LINE__);
}

                            /*** -w/--whereto ***/

/*
 * test_whereto_option() - Tests the -w/--whereto option. Returns nothing.
 */

static void test_whereto_option(void)
{
	struct Entry entry;

	diag("Test -w/--whereto");

	if (init_tempdir())
		return; /* gncov */
	init_xml_entry(&entry);

	uc((chp{ execname, "-w", "o", NULL }), 1, 0, "\"-w o\" prints to stdout");
	uc((chp{ execname, "-w", "e", NULL }), 0, 1, "\"-w e\" prints to stderr");
	uc((chp{ execname, "-w", "a", NULL }), 1, 1, "\"-w a\" prints to stdout and stderr");
	uc((chp{ execname, "-w", "oe", NULL }), 1, 1, "\"-w oe\" prints to stdout and stderr");
	uc((chp{ execname, "-w", "n", NULL }), 0, 0, "\"-w n\" prints nothing");
	uc((chp{ execname, "-w", "y", NULL }), 0, 0, "\"-w y\" prints nothing");
	uc((chp{ execname, "-w", "", NULL }), 0, 0, "\"-w\" with empty argument prints nothing");

	verify_logfile(&entry, 7, "Log file contains the correct number of"
	                          " entries after -w/--whereto");
	cleanup_tempdir(__LINE__);
}

/******************************************************************************
                        Top-level --selftest functions
******************************************************************************/

/*
 * functests_with_tempdir() - Tests functions that need a temporary directory 
 * to store the output from stderr and stdout. Returns nothing.
 */

static void functests_with_tempdir(void)
{
	int result;

	diag("Test functions that need a temporary directory for stdout and"
	     " stderr");
	result = mkdir(TMPDIR, 0755);
	OK_SUCCESS(result, "mkdir " TMPDIR " for function tests");
	if (result) {
		diag("Cannot create directory \"%s\", skipping" /* gncov */
		     " tests: %s", TMPDIR, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return; /* gncov */
	}

	/* environ.c */
	test_get_log_prefix();

	/* genuuid.c */
	test_fill_entry_struct();
	test_create_and_log_uuids();

	/* io.c */
	test_create_file();

	/* rcfile.c */
	test_read_rcfile();

	result = rmdir(TMPDIR);
	OK_SUCCESS(result, "rmdir " TMPDIR " after function tests");
	if (result) {
		diag("test %d: %s", testnum, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return; /* gncov */
	}
}

/*
 * tests_with_tempdir() - Executes tests on the executable with a temporary 
 * directory. The temporary directory uses a standard name, defined in TMPDIR. 
 * If the directory already exists or it's unable to create it, it aborts. 
 * Returns nothing.
 */

static void tests_with_tempdir(void)
{
	int result;

	result = mkdir(TMPDIR, 0777);
	OK_SUCCESS(result, "%s(): Create temporary directory %s",
	                   __func__, TMPDIR);
	if (result) {
		diag("Cannot create directory \"%s\", skipping" /* gncov */
		     " tests: %s", TMPDIR, strerror(errno)); /* gncov */
		errno = 0; /* gncov */
		return; /* gncov */
	}

	test_without_options();
	test_sess_elements();
	test_truncated_logfile();
	test_comment_option();
	read_long_text_from_stdin();
	test_external_editor();
	test_unreadable_editor_file();
	test_nonexisting_editor();
	test_count_option();
	test_logdir_option();
	test_random_mac_option();
	test_raw_option();
	test_rcfile_option();
	test_tag_option();
	test_too_many_tags();
	test_too_many_comma_tags();
	test_whereto_option();

	OK_SUCCESS(rmdir(TMPDIR), "Delete temporary directory %s", TMPDIR);
}

/*
 * test_functions() - Tests various functions directly. Returns nothing.
 */

static void test_functions(const struct Options *o)
{
	assert(o);

	if (!o->testfunc)
		return; /* gncov */

	diag("Test selftest routines");

	/* selftest.c */
	test_diag();
	test_gotexp_output();
	test_valgrind_lines();
	test_count_uuids();

	diag("Test various routines");

	/* suuid.c */
	test_std_strerror();

	/* io.c */
	test_read_from_file();

	/* logfile.c */
	test_create_sess_xml();

	/* rcfile.c */
	test_has_key();

	/* sessvar.c */
	test_get_sess_info();

	/* strings.c */
	test_mystrdup();
	test_allocstr();
	test_count_substr();
	test_str_replace();
	test_string_to_lower();

	/* uuid.c */
	test_valid_uuid();
	test_is_valid_date();
	test_uuid_date();

	functests_with_tempdir();
}

/*
 * test_executable() - Run various tests with the executable and verify that 
 * stdout, stderr and the return value are as expected. Returns nothing.
 */

static void test_executable(const struct Options *o)
{
	assert(o);
	if (!o->testexec)
		return; /* gncov */

	diag("Test the executable");
	test_valgrind_option(o);
	print_version_info(o);
	test_standard_options();
	test_invalid_hostname_var();
	tests_with_tempdir();
	print_version_info(o);
}

/*
 * opt_selftest() - Run internal testing to check that it works on the current 
 * system. Executed if --selftest is used. Returns `EXIT_FAILURE` if any tests 
 * fail; otherwise, it returns `EXIT_SUCCESS`.
 */

int opt_selftest(char *main_execname, const struct Options *o)
{
	assert(main_execname);
	assert(o);

	execname = main_execname;
	diag("Running tests for %s %s (%s)",
	     execname, EXEC_VERSION, EXEC_DATE);

	test_ok_macros();
	if (init_testvars())
		return EXIT_FAILURE; /* gncov */
	test_functions(o);
	test_executable(o);

	printf("1..%d\n", testnum);
	if (failcount) {
		diag("Looks like you failed %d test%s of %d.", /* gncov */
		     failcount, (failcount == 1) ? "" : "s", /* gncov */
		     testnum);
	}

	free(logfile); /* Allocated in init_testvars() */
	return failcount ? EXIT_FAILURE : EXIT_SUCCESS;
}

#undef DIAG_DEBL
#undef DIAG_VARS
#undef EXECSTR
#undef HNAME
#undef OK_EQUAL
#undef OK_EQUAL_L
#undef OK_ERROR
#undef OK_ERROR_L
#undef OK_FAILURE
#undef OK_FAILURE_L
#undef OK_FALSE
#undef OK_FALSE_L
#undef OK_NOTEQUAL
#undef OK_NOTEQUAL_L
#undef OK_NOTNULL
#undef OK_NOTNULL_L
#undef OK_NULL
#undef OK_NULL_L
#undef OK_STRCMP
#undef OK_STRCMP_L
#undef OK_STRNCMP
#undef OK_STRNCMP_L
#undef OK_SUCCESS
#undef OK_SUCCESS_L
#undef OK_TRUE
#undef OK_TRUE_L
#undef OPTION_ERROR_STR
#undef R_TIMESTAMP
#undef R_UUID
#undef TMPDIR
#undef chp
#undef delete_logfile
#undef failed_ok
#undef init_tempdir
#undef sc
#undef set_env
#undef tc
#undef uc
#undef unset_env
#undef verify_logfile
#undef verify_output_files

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
