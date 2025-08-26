/*
 * suuid.h
 * File ID: 289a8d22-2b93-11e6-879f-02010e0a6634
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

#ifndef _SUUID_H
#define _SUUID_H

#include "version.h"

#define T_GREEN  "\x1b[32m"
#define T_RED    "\x1b[31m"
#define T_RESET  "\x1b[m\x0f"

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <regex.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

#include "binbuf.h"
#include "uuid.h"

#if 1
#  define DEBL  msg(2, "DEBL: %s, line %u in %s()", \
                       __FILE__, __LINE__, __func__)
#else
#  define DEBL  ;
#endif

#ifdef CHECK_ERRNO
#define check_errno  do { \
	if (errno) { \
		myerror("%s():%s:%d: errno = %d", \
		        __func__, __FILE__, __LINE__, errno); \
	} \
} while (0)
#else
#define check_errno  do { } while (0)
#endif

#define failed(a)  myerror("%s():%d: %s failed", __func__, __LINE__, (a))
#define no_null(a)  ((a) ? (a) : "(null)")

#define ENV_EDITOR  "SUUID_EDITOR" /* Name of editor to use with "-c --" */
#define ENV_HOSTNAME  "SUUID_HOSTNAME" /* Optional environment variable */
#define ENV_LOGDIR  "SUUID_LOGDIR" /* Optional environment variable with path 
                                    * to log directory
                                    */
#define ENV_SESS  "SESS_UUID" /* Name of environment variable where the session 
                               * information is stored
                               */
#define LOGDIR_NAME  "uuids"
#define LOGFILE_EXTENSION  ".xml"
#define MAX_HOSTNAME_LENGTH  100
#define MAX_SESS  1000 /* Maximum number of sess elements per entry */
#define MAX_TAGS  1000 /* Maximum number of tags */
#define STD_RCFILE  ".suuidrc"

#define LEGAL_UTF8_CHARS  "\x80\x81\x82\x83\x84\x85\x86\x87" \
                          "\x88\x89\x8a\x8b\x8c\x8d\x8e\x8f" \
                          "\x90\x91\x92\x93\x94\x95\x96\x97" \
                          "\x98\x99\x9a\x9b\x9c\x9d\x9e\x9f" \
                          "\xa0\xa1\xa2\xa3\xa4\xa5\xa6\xa7" \
                          "\xa8\xa9\xaa\xab\xac\xad\xae\xaf" \
                          "\xb0\xb1\xb2\xb3\xb4\xb5\xb6\xb7" \
                          "\xb8\xb9\xba\xbb\xbc\xbd\xbe\xbf" \
                                  "\xc2\xc3\xc4\xc5\xc6\xc7" \
                          "\xc8\xc9\xca\xcb\xcc\xcd\xce\xcf" \
                          "\xd0\xd1\xd2\xd3\xd4\xd5\xd6\xd7" \
                          "\xd8\xd9\xda\xdb\xdc\xdd\xde\xdf" \
                          "\xe0\xe1\xe2\xe3\xe4\xe5\xe6\xe7" \
                          "\xe8\xe9\xea\xeb\xec\xed\xee\xef" \
                          "\xf0\xf1\xf2\xf3\xf4\xf5\xf6\xf7" \
                          "\xf8"
#define DESC_LEGAL  "-."                         \
                    "0123456789"                 \
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ" \
                    "_"                          \
                    "abcdefghijklmnopqrstuvwxyz" \
                    LEGAL_UTF8_CHARS /* Legal chars in sess descriptions */

struct Rc {
	char *hostname;
	char *macaddr;
};

struct Sess {
	char *uuid;
	char *desc;
};

struct Entry {
	char date[DATE_LENGTH + 1];
	char uuid[UUID_LENGTH + 1];
	char *tag[MAX_TAGS];
	char *txt;
	char *host;
	char *cwd;
	char *user;
	char *tty;
	struct Sess sess[MAX_SESS];
};

struct Logs {
	FILE *logfp;
};

struct Options {
	/* sort -d -k2 */
	char *comment;
	bool help;
	bool license;
	char *logdir;
	unsigned long count;
	bool random_mac;
	bool raw;
	char *rcfile;
	bool selftest;
	char *tag[MAX_TAGS];
	bool testexec;
	bool testfunc;
	char *uuid;
	bool valgrind;
	int verbose;
	bool version;
	char *whereto;
};

struct streams {
	struct binbuf in;
	struct binbuf out;
	struct binbuf err;
	int ret;
};

struct uuid_result {
	unsigned long count;
	char lastuuid[UUID_LENGTH + 1];
	bool success;
};

/*
 * Public function prototypes
 */

/* suuid.c */
struct Options opt_struct(void);
int msg(const int verbose, const char *format, ...);
const char *std_strerror(const int errnum);
int myerror(const char *format, ...);
void init_opt(struct Options *dest);
void set_opt_valgrind(bool b);

/* environ.c */
char *get_editor(void);
bool valid_hostname(const char *s);
char *get_hostname(const struct Rc *rc);
char *get_logdir(const struct Options *opt);
char *get_log_prefix(const struct Rc *rc, const struct Options *opt,
                     const char *ext);
char *getpath(void);
char *get_username(void);
char *get_tty(void);

/* genuuid.c */
int fill_entry_struct(struct Entry *entry, const struct Rc *rc,
                      const struct Options *opts);
struct uuid_result create_and_log_uuids(const struct Options *opt);

/* io.c */
bool file_exists(const char *s);
void streams_init(struct streams *dest);
void streams_free(struct streams *dest);
char *read_from_fp(FILE *fp, struct binbuf *dest);
const char *create_file(const char *file, const char *txt);
char *read_from_file(const char *fname);
char *read_from_editor(const char *editor);
int streams_exec(const struct Options *o, struct streams *dest, char *cmd[]);

/* logfile.c */
bool valid_xml_chars(const char *s);
void init_sess_array(struct Sess *sess);
void init_xml_entry(struct Entry *e);
char *create_sess_xml(const struct Entry *entry);
FILE *open_logfile(const char *fname);
int add_to_logfile(FILE *fp, const struct Entry *entry, const bool raw);
int close_logfile(FILE *fp);

/* rcfile.c */
void init_rc(struct Rc *rc);
void free_rc(struct Rc *rc);
int create_rcfile(const char *file, struct Rc *rc);
char *get_rcfilename(const struct Options *opt);
char *has_key(const char *line, const char *keyword);
int read_rcfile(const char *rcfile, struct Rc *rc);

/* selftest.c */
int opt_selftest(char *execname, const struct Options *o);

/* sessvar.c */
int get_sess_info(struct Entry *entry);
void free_sess(struct Entry *entry);
int run_session(const struct Options *orig_opt,
                const int argc, char * const argv[]);

/* strings.c */
char *mystrdup(const char *s);
char *allocstr_va(const char *format, va_list ap);
char *allocstr(const char *format, ...);
size_t count_substr(const char *s, const char *substr);
char *str_replace(const char *s, const char *s1, const char *s2);
#if defined(UNUSED)
char *squeeze_chars(char *s, const char *chars);
#endif
char *string_to_lower(char *str);
char *trim_str_front(char *dest);
char *trim_str_end(char *dest);
const char *utf8_check(const char *text);

/* tag.c */
void rewind_tag(void);
char *get_next_tag(const struct Entry *entry);
int store_tag(struct Entry *entry, const char *arg);
void free_tags(struct Entry *entry);

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
