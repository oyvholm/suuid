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

#define T_RESET  "\x1b[m\x0f"
#define T_RED    "\x1b[31m"
#define T_GREEN  "\x1b[32m"

#define stddebug  stderr

#ifdef USE_SQLITE
#  include "sqlite3.h"
#endif

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <time.h>
#include <unistd.h>

#include "uuid.h"

#define ENV_EDITOR  "SUUID_EDITOR" /* Name of editor to use with "-c --" */
#define ENV_SESS  "SESS_UUID" /* Name of environment variable where the session 
                               * information is stored
                               */
#define ENV_HOSTNAME  "SUUID_HOSTNAME" /* Optional environment variable */
#define ENV_LOGDIR  "SUUID_LOGDIR" /* Optional environment variable with path 
                                    * to log directory
                                    */
#define STD_RCFILE  ".suuidrc"
#define MAX_SESS  1000 /* Maximum number of sess elements per entry */
#define MAX_TAGS  1000 /* Maximum number of tags */
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

/*
 * FreeBSD doesn't have HOST_NAME_MAX, try to use something else.
 */

#ifndef HOST_NAME_MAX
#  ifdef _POSIX_HOST_NAME_MAX
#    define HOST_NAME_MAX  _POSIX_HOST_NAME_MAX
#  else
#    define HOST_NAME_MAX  255
#  endif
#endif

#define DEBL  msg(2, "%s, line %u in %s()", __FILE__, __LINE__, __func__)
#define in_range(a,b,c)  ((a) >= (b) && (a) <= (c) ? true : false)

struct Rc {
	char *hostname;
	char *macaddr;
	char *uuidcmd;
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
	char *comment;
	unsigned int count;
	bool help;
	bool license;
	char *logdir;
	bool random_mac;
	bool raw;
	char *rcfile;
	bool selftest;
	int simfail;
	char *tag[MAX_TAGS];
	char *uuid;
	int verbose;
	bool version;
	char *whereto;
};
struct uuid_result {
	unsigned int count;
	char lastuuid[UUID_LENGTH + 1];
	bool success;
};

/*
 * Public function prototypes
 */

/* database.c */

/* environ.c */
extern char *get_editor(void);
extern bool valid_hostname(const char *s);
extern char *get_hostname(const struct Rc *rc);
extern char *get_logdir(const struct Options *opt);
extern char *get_log_prefix(const struct Rc *rc,
                            const struct Options *opt, char *ext);
extern char *getpath(void);
extern char *get_username(void);
extern char *get_tty(void);

/* genuuid.c */
extern void init_opt(struct Options *dest);
extern struct uuid_result create_and_log_uuids(const struct Options *opt);

/* io.h */
extern char *read_from_fp(FILE *fp);
extern char *read_from_editor(const char *editor);

/* logfile.c */
extern bool valid_xml_chars(const char *s);
extern void init_xml_entry(struct Entry *e);
extern FILE *open_logfile(const char *fname);
extern int add_to_logfile(FILE *fp, const struct Entry *entry, const bool raw);
extern int close_logfile(FILE *fp);

/* rcfile.c */
extern char *get_rcfilename(const struct Options *opt);
extern int read_rcfile(const char *rcfile, struct Rc *rc);

/* selftest.c */
extern int selftest(void);

/* sessvar.c */
extern int get_sess_info(struct Entry *entry);
extern void free_sess(struct Entry *entry);
extern int run_session(const struct Options *orig_opt,
                       const int argc, char * const argv[]);

/* string.c */
extern void *mymalloc(const size_t size);
extern char *mystrdup(const char *s);
#if defined(UNUSED) || defined(TEST_FUNC)
extern char *squeeze_chars(char *s, const char *chars);
#endif
extern char *string_to_lower(char *str);
extern char *trim_str_front(char *dest);
extern char *trim_str_end(char *dest);
extern char *utf8_check(const char *text);

/* suuid.c */
extern int msg(const int verbose, const char *format, ...);
extern int myerror(const char *format, ...);

/* tag.c */
extern void rewind_tag(void);
extern char *get_next_tag(const struct Entry *entry);
extern int store_tag(struct Entry *entry, const char *arg);
extern void free_tags(struct Entry *entry);

/* uuid.c */
extern char *check_hex(const char *hex, const size_t len);
extern bool valid_macaddr(const char *macaddr);
extern bool valid_uuid(const char *u, const bool check_len);
extern char *scan_for_uuid(const char *s);
extern char *scramble_mac_address(char *uuid);
extern char *generate_uuid(char *uuid, const struct Rc *rc);
extern bool is_valid_date(const char *s, const bool check_len);
extern char *uuid_date(char *dest, const char *uuid);
#ifdef VERIFY_UUID
extern char *uuid_date_from_uuid(char *dest, const char *uuid);
#endif

/*
 * Global variables
 */

extern char *progname;
extern int simfail;

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
