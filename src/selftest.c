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

#define chp  (char *[])

static int testnum = 0;

/*
 * ok() - Print a log line to stdout. If `i` is 0, an "ok" line is printed, 
 * otherwise a "not ok" line is printed. `desc` is the test description and can 
 * use printf sequences.
 *
 * If `desc` is NULL, it returns 1. Otherwise, it returns `i`.
 */

static int ok(const int i, const char *desc, ...)
{
	va_list ap;

	if (!desc)
		return 1;
	va_start(ap, desc);
	printf("%sok %d - ", (i ? "not " : ""), ++testnum);
	vprintf(desc, ap);
	puts("");
	va_end(ap);
	fflush(stdout);

	return i;
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
	int needed;
	size_t buffer_size = BUFSIZ, converted_size;
	va_list ap_copy;

	if (!format)
		return NULL; /* gncov */

	buffer = malloc(buffer_size);
	if (!buffer)
		return NULL; /* gncov */

	va_copy(ap_copy, ap);
	needed = vsnprintf(buffer, buffer_size, format, ap);

	if ((size_t)needed >= buffer_size) {
		free(buffer);
		buffer_size = (size_t)needed + 1;
		buffer = malloc(buffer_size);
		if (!buffer)
			return NULL; /* gncov */
		vsnprintf(buffer, buffer_size, format, ap_copy);
	}
	va_end(ap_copy);

	/* Prepare for worst case, every char is a newline. */
	converted_size = strlen(buffer) * 3 + 1;
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
 * Returns number of failed tests.
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
 * diag() - Prints a diagnostic message prefixed with "# " to stdout. `printf` 
 * sequences can be used. All `\n` characters are converted to "\n# ".
 *
 * A terminating `\n` is automatically added to the string. Returns 0 if 
 * successful, or 1 if `format` is NULL.
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
		return ok(1, "%s(): diag_output_va() failed", /* gncov */
		             __func__);
	}
	fprintf(stderr, "%s\n", converted_buffer);
	fflush(stderr);
	free(converted_buffer);

	return 0;
}

/*
 * gotexp_output() - Generate the output used by print_gotexp(). The output is 
 * returned as an allocated string that must be free()'ed after use. Returns 
 * NULL if `got` or `exp` is NULL or allocstr() fails. Otherwise, it returns a 
 * pointer to the string with the output.
 */

static char *gotexp_output(const char *got, const char *exp)
{
	char *s;

	if (!got || !exp)
		return NULL;

	s = allocstr("         got: '%s'\n"
	             "    expected: '%s'",
	             got, exp);
	if (!s)
		ok(1, "%s(): allocstr() failed", __func__); /* gncov */

	return s;
}

/*
 * print_gotexp() - Print the value of the actual and exepected data. Used when 
 * a test fails. Returns 1 if `got` or `exp` is NULL, otherwise 0.
 */

static int print_gotexp(const char *got, const char *exp)
{
	char *s;

	if (!got || !exp)
		return 1;
	if (!strcmp(got, exp))
		return 0;

	s = gotexp_output(got, exp); /* gncov */
	diag(s); /* gncov */
	free(s); /* gncov */

	return 0; /* gncov */
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

	if (identical || !strlen(exp))
		return !!strcmp(got, exp);

	return !strstr(got, exp);
}

/*
 * test_command() - Run the executable with arguments in `cmd` and verify 
 * stdout, stderr and the return value against `exp_stdout`, `exp_stderr` and 
 * `exp_retval`. Returns the number of failed tests, or 1 if `cmd` is NULL.
 */

static int test_command(const char identical, char *cmd[],
                        const char *exp_stdout, const char *exp_stderr,
                        const int exp_retval, const char *desc)
{
	int r = 0;
	struct streams ss;

	assert(cmd);
	if (!cmd)
		return 1; /* gncov */

	if (opt.verbose >= 4) {
		int i = -1; /* gncov */
		fprintf(stderr, "# %s(", __func__); /* gncov */
		while (cmd[++i]) /* gncov */
			fprintf(stderr, "%s\"%s\"", /* gncov */
			                i ? ", " : "", cmd[i]); /* gncov */
		fprintf(stderr, ")\n"); /* gncov */
	}

	streams_init(&ss);
	streams_exec(&ss, cmd);
	if (exp_stdout) {
		r += ok(tc_cmp(identical, ss.out.buf, exp_stdout),
		        "%s (stdout)", desc);
		if (tc_cmp(identical, ss.out.buf, exp_stdout))
			print_gotexp(ss.out.buf, exp_stdout); /* gncov */
	}
	if (exp_stderr) {
		r += ok(tc_cmp(identical, ss.err.buf, exp_stderr),
		        "%s (stderr)", desc);
		if (tc_cmp(identical, ss.err.buf, exp_stderr))
			print_gotexp(ss.err.buf, exp_stderr); /* gncov */
	}
	r += ok(!(ss.ret == exp_retval), "%s (retval)", desc);
	if (ss.ret != exp_retval) {
		char *g = allocstr("%d", ss.ret), /* gncov */
		     *e = allocstr("%d", exp_retval); /* gncov */
		if (!g || !e) /* gncov */
			r += ok(1, "%s(): allocstr() failed", /* gncov */
			           __func__); /* gncov */
		else
			print_gotexp(g, e); /* gncov */
		free(e); /* gncov */
		free(g); /* gncov */
	}
	streams_free(&ss);

	return r;
}

/*
 * sc() - Execute command `cmd` and verify that stdout, stderr and the return 
 * value corresponds to the expected values. The `exp_*` variables are 
 * substrings that must occur in the actual output. Returns the number of 
 * failed tests.
 */

static int sc(char *cmd[], const char *exp_stdout, const char *exp_stderr,
              const int exp_retval, const char *desc)
{
	return test_command(0, cmd, exp_stdout, exp_stderr, exp_retval, desc);
}

/*
 ******************
 * Function tests *
 ******************
 */

/*
 * selftest functions
 */

/*
 * test_diag_big() - Tests diag_output() with a string larger than BUFSIZ. 
 * Returns the number of failed tests.
 */

static int test_diag_big(void)
{
	int r = 0;
	size_t size;
	char *p, *outp;

	size = BUFSIZ * 2;
	p = malloc(size + 1);
	if (!p) {
		return ok(1, "%s(): malloc(%zu) failed", /* gncov */
		             __func__, size + 1);
	}

	memset(p, 'a', size);
	p[3] = 'b';
	p[4] = 'c';
	p[size] = '\0';

	outp = diag_output("%s", p);
	r += ok(!outp, "diag_big: diag_output() returns ok");
	r += ok(!(strlen(outp) == size + 2),
	        "diag_big: String length is correct");
	r += ok(!!strncmp(outp, "# aaabcaaa", 10),
	        "diag_big: Beginning is ok");
	free(outp);
	free(p);

	return r;
}

/*
 * test_diag() - Tests the diag_output() function. diag() can't be tested 
 * directly because it would pollute the the test output. Returns the number of 
 * failed tests.
 */

static int test_diag(void) {
	int r = 0;
	char *p, *s;

	diag("Test diag()");

	r += ok(!diag(NULL), "diag(NULL)");
	r += ok(!(diag_output(NULL) == NULL), "diag_output() receives NULL");

	p = diag_output("Text with\nnewline");
	r += ok(!p, "diag_output() with newline didn't return NULL");
	s = "# Text with\n# newline";
	r += ok(p ? !!strcmp(p, s) : 1,
	        "diag_output() with newline, output is ok");
	print_gotexp(p, s);
	free(p);

	p = diag_output("%d = %s, %d = %s, %d = %s",
	                1, "one", 2, "two", 3, "three");
	r += ok(!p, "diag_output() with %%d and %%s didn't return NULL");
	s = "# 1 = one, 2 = two, 3 = three";
	r += ok(p ? !!strcmp(p, s) : 1, "diag_output() with %%d and %%s");
	print_gotexp(p, s);
	free(p);

	r += test_diag_big();

	return r;
}

/*
 * test_gotexp_output() - Tests the gotexp_output() function. print_gotexp() 
 * can't be tested directly because it would pollute stderr. Returns the number 
 * of failed tests.
 */

static int test_gotexp_output(void)
{
	int r = 0;
	char *p, *s;

	diag("Test gotexp_output()");

	r += ok(!!gotexp_output(NULL, "a"), "gotexp_output(NULL, \"a\")");

	r += ok(!!strcmp((p = gotexp_output("got this", "expected this")),
	                 "         got: 'got this'\n"
	                 "    expected: 'expected this'"),
	        "gotexp_output(\"got this\", \"expected this\")");
	free(p);

	r += ok(!print_gotexp(NULL, "expected this"),
	        "print_gotexp(): Arg is NULL");

	s = "gotexp_output(\"a\", \"a\")";
	r += ok(!(p = gotexp_output("a", "a")), "%s doesn't return NULL", s);
	r += ok(!!strcmp(p, "         got: 'a'\n    expected: 'a'"),
	        "%s: Contents is ok", s);
	free(p);

	s = "gotexp_output() with newline";
	r += ok(!(p = gotexp_output("with\nnewline", "also with\nnewline")),
	        "%s: Doesn't return NULL", s);
	r += ok(!!strcmp(p, "         got: 'with\nnewline'\n"
	                    "    expected: 'also with\nnewline'"),
	        "%s: Contents is ok", s);
	free(p);

	return r;
}

/*
 * test_allocstr() - Tests the allocstr() function. Returns the number of 
 * failed tests.
 */

static int test_allocstr(void)
{
	const size_t bufsize = BUFSIZ * 2 + 1;
	char *p, *p2, *p3;
	int r = 0;
	size_t alen;

	diag("Test allocstr()");
	p = malloc(bufsize);
	if (!p) {
		r += ok(1, "%s(): malloc() failed", __func__); /* gncov */
		return r; /* gncov */
	}
	memset(p, 'a', bufsize - 1);
	p[bufsize - 1] = '\0';
	p2 = allocstr("%s", p);
	if (!p2) {
		r += ok(1, "%s(): allocstr() failed" /* gncov */
		           " with BUFSIZ * 2",
		           __func__);
		goto free_p; /* gncov */
	}
	alen = strlen(p2);
	r += ok(!(alen == BUFSIZ * 2), "allocstr(): strlen is correct");
	p3 = p2;
	while (*p3) {
		if (*p3 != 'a') {
			p3 = NULL; /* gncov */
			break; /* gncov */
		}
		p3++;
	}
	r += ok(!(p3 != NULL), "allocstr(): Content of string is correct");
	free(p2);
free_p:
	free(p);

	return r;
}

/*
 * test_string_to_lower() - Tests the string_to_lower() function. Returns the 
 * number of failed tests.
 */

static int test_string_to_lower(void)
{
	int r = 0;
	char s1[] = "ABCÅÆØ";

	diag("Test string_to_lower()");
	r += ok(!(string_to_lower(NULL) == NULL), "string_to_lower(NULL)");
	r += ok(!!strcmp(string_to_lower(s1), "abcÅÆØ"),
	        "string_to_lower(\"ABCÅÆØ\")");

	return r;
}

/*
 * chk_ivd() - Used by test_is_valid_date(). Checks that `is_valid_date(date, 
 * check_len)` returns the value in `exp`. Returns the number of failed tests.
 */

static int chk_ivd(const char *date, const bool check_len, const int exp)
{
	int r = 0, res;

	res = is_valid_date(date, check_len);
	r += ok(!(res == exp),
	        "is_valid_date(\"%s\", %s), expecting %d",
	        date, check_len ? "true" : "false", res);

	return r;
}

/*
 * test_is_valid_date() - Tests the is_valid_date() function. Returns the 
 * number of failed tests.
 */

static int test_is_valid_date(void)
{
	int r = 0;

	diag("Test is_valid_date()");
	r += chk_ivd("2017-12-23T02:33:57Z", true, 0);
	r += chk_ivd("2017-12-23T02:33:57Z", false, 0);
	r += chk_ivd("2017-12-23T02:33:57.1234567Z", true, 1);
	r += chk_ivd("2017-12-23T02:33:57.1234567Z", false, 1);
	r += chk_ivd("2017-12-23T02:33:57.1234567Zabcd", false, 1);

	return r;
}

/*
 * chk_ud() - Used by test_uuid_date(). The function first checks that 
 * uuid_date() returns the correct value, i.e. that `uuid` is a valid v1 UUID. 
 * This value is specified in `exp_ret`: 0 if uuid_date() is expected to return 
 * NULL, otherwise 1. It then checks that the generated timestamp is as 
 * expected.
 *
 * Returns the number of failed tests.
 */

static int chk_ud(const char *uuid, const int exp_ret, const char *exp_date)
{
	int r = 0, ret;
	char buf[DATE_LENGTH + 1];

	assert(uuid);
	assert(exp_date);

	ret = !!uuid_date(buf, uuid);
	r += ok(!(ret == exp_ret),
	        "uuid_date(): \"%s\" is%s a valid v1 UUID",
	        uuid, exp_ret ? "" : " not");
	if (!ret)
		return r;
	r += ok(!!strcmp(buf, exp_date), "uuid_date(\"%s\")", uuid);
	print_gotexp(buf, exp_date);

	return r;
}

/*
 * test_uuid_date() - Tests the uuid_date() function. Returns the number of 
 * failed tests.
 */

static int test_uuid_date(void)
{
	int r = 0;

	diag("Test uuid_date()");
	r += chk_ud("00000000-0000-11e7-87d5-f74d993421b0", 1,
	            "2017-03-03T10:56:05.8089472Z");
	r += chk_ud("acdaf974-e78e-11e7-87d5-f74d993421b0", 1,
	            "2017-12-23T03:09:22.9493620Z");
	r += chk_ud("notvalid", 0, "");
	r += chk_ud("", 0, "");
	r += chk_ud("c9ffa9cb-708d-454b-b1f2-f18f609cb825", 0, "");
	r += chk_ud("acdaf974-e78e-11e7-87d5-g74d993421b0", 0, "");

	return r;
}

/*
 * chk_vu() - Used by test_valid_uuid(). Checks that `valid_uuid(uuid, 
 * check_len)` returns `exp_valid`. Returns the number of failed tests.
 */

static int chk_vu(const char *uuid, const bool check_len, const bool exp_valid)
{
	int r = 0;
	bool res;

	res = valid_uuid(uuid, check_len);
	r += ok(!(res == exp_valid), "valid_uuid(\"%s\", %s) should return %s",
	        uuid, check_len ? "true" : "false",
	        exp_valid ? "true" : "false");

	return r;
}

/*
 * test_valid_uuid() - Tests the valid_uuid() function. Returns the number of 
 * failed tests.
 */

static int test_valid_uuid(void)
{
	int r = 0;

	diag("Test valid_uuid()");
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0", true, true);
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0123", false, true);
	chk_vu("acdaf974-e78e-11e7-87d5-f74d993421b0123", true, false);
	chk_vu("c9ffa9cb-708d-454b-b1f2-f18f609cb825", true, false);

	return r;
}

/*
 * test_standard_options() - Tests the various generic options available in 
 * most programs. Returns the number of failed tests.
 */

static int test_standard_options(char *execname)
{
	int r = 0;

	diag("Test standard options");

	diag("Test -h/--help");
	r += sc(chp{ execname, "-h", NULL },
	        "  Show this help",
	        "",
	        EXIT_SUCCESS,
	        "-h");
	r += sc(chp{ execname, "--help", NULL },
	        "  Show this help",
	        "",
	        EXIT_SUCCESS,
	        "--help");

	return r;
}

/*
 * test_functions() - Tests various functions directly. Returns the number of 
 * failed tests.
 */

static int test_functions(void)
{
	int r = 0;

	diag("Test selftest routines");
	r += ok(!ok(0, NULL), "ok(0, NULL)");
	r += test_diag();
	r += test_gotexp_output();

	diag("Test various routines");
	diag("Test myerror()");
	errno = EACCES;
	r += ok(!(myerror("errno is EACCES") > 37),
	        "myerror(): errno is EACCES");
	errno = 0;
	diag("Test std_strerror()");
	r += ok(!(std_strerror(0) != NULL), "std_strerror(0)");
	r += test_allocstr();
	r += test_string_to_lower();
	r += test_is_valid_date();
	r += test_uuid_date();
	r += test_valid_uuid();

	return r;
}

/*
 * print_version_info() - Display output from the --version command. Returns 0 
 * if ok, or 1 if streams_exec() failed.
 */

static int print_version_info(char *execname)
{
	struct streams ss;
	int res;

	streams_init(&ss);
	res = streams_exec(&ss, chp{ execname, "--version", NULL });
	ok(!!res, "streams_exec() with --version works");
	if (res) {
		diag("%s(): streams_exec() failed:\n%s",
		     __func__, ss.err.buf ? ss.err.buf : "(null)");
		return 1;
	}
	diag("========== BEGIN version info ==========\n"
	     "%s"
	     "=========== END version info ===========",
	     ss.out.buf ? ss.out.buf : "(null)");
	streams_free(&ss);

	return 0;
}

/*
 * test_executable() - Run various tests with the executable and verify that 
 * stdout, stderr and the return value are as expected. Returns the number of 
 * failed tests.
 */

static int test_executable(char *execname)
{
	int r = 0;

	diag("Test the executable");
	print_version_info(execname);

	r += test_standard_options(execname);

	return r;
}

/*
 * opt_selftest() - Run internal testing to check that it works on the current 
 * system. Executed if --selftest is used. Returns `EXIT_FAILURE` if any tests 
 * fail; otherwise, it returns `EXIT_SUCCESS`.
 */

int opt_selftest(char *execname)
{
	int r = 0;

	diag("Running tests for %s %s (%s)",
	     execname, EXEC_VERSION, EXEC_DATE);

	r += test_functions();
	r += test_executable(execname);

	printf("1..%d\n", testnum);
	if (r) {
		diag("Looks like you failed %d test%s of %d.", /* gncov */
		     r, (r == 1) ? "" : "s", testnum);
	}

	return r ? EXIT_FAILURE : EXIT_SUCCESS;
}

#undef chp

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
