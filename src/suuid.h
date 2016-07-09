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

/*
 * Defines
 */

#define VERSION       "0.1.8"
#define RELEASE_DATE  "2016-07-08"

#define FALSE  0
#define TRUE   1

#define EXIT_OK     0
#define EXIT_ERROR  1

#define T_RESET  "\x1b[m\x0f"
#define T_RED    "\x1b[31m"
#define T_GREEN  "\x1b[32m"

#define DATE_LENGTH  28 /* Length of ISO date format with nanoseconds */
#define UUID_LENGTH  36 /* Length of a standard UUID */

#define FAKE_HOST  0 /* Use "fake" as hostname to avoid conflicts with files 
                      * created by the Perl version
                      */
#define PERL_COMPAT  0 /* Compile a version with some changes to make it 
                        * compatible with the Perl version
                        */
#define TEST_FUNC  0 /* Send non-option arguments to a function for testing. 
                      * Doesn't break anything, non-option arguments are 
                      * ignored by the program.
                      */

#define stddebug  stderr

#define ENV_EDITOR  "SUUID_EDITOR" /* Name of editor to use with "-c --" */
#define ENV_SESS  "SESS_UUID" /* Name of environment variable where the session 
                               * information is stored
                               */
#define ENV_HOSTNAME  "SUUID_HOSTNAME" /* Optional environment variable */
#define ENV_LOGDIR  "SUUID_LOGDIR" /* Optional environment variable with path 
                                    * to log directory
                                    */
#define STD_EDITOR  "vi"
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
 * Standard header files
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>

/*
 * Macros
 */

#define DEBL  msg(2, "%s, line %u in %s()", __FILE__, __LINE__, __func__)
#define in_range(a,b,c)  ((a) >= (b) && (a) <= (c) ? TRUE : FALSE)

/*
 * Typedefs
 */

typedef unsigned char bool;
struct Rc {
	char *hostname;
	char *uuidcmd;
};
struct Sess {
	char *uuid;
	char *desc;
};
struct Entry {
	char *date;
	char *uuid;
	char *tag[MAX_TAGS];
	char *txt;
	char *host;
	char *cwd;
	char *user;
	char *tty;
	struct Sess sess[MAX_SESS];
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
	int verbose;
	bool version;
	char *whereto;
};

/*
 * Function prototypes
 */

#if 1 /* Set to 0 to test without prototypes */

/* environ.c */
extern char *get_editor(void);
extern bool valid_hostname(const char *s);
extern char *get_hostname(void);
extern char *getpath(void);
extern char *get_username(void);
extern char *get_tty(void);

/* io.h */
extern char *read_from_fp(FILE *fp);
extern char *read_from_file(const char *fname);
extern char *read_from_editor(const char *editor);

/* logfile.c */
extern bool valid_xml_chars(const char *s);
extern char *suuid_xml(const char *text);
extern void init_xml_entry(struct Entry *e);
extern char *allocate_elem(const char *elem, const char *src);
extern char *alloc_attr(const char *attr, const char *data);
extern char *get_xml_tags(void);
extern char *create_sess_xml(const struct Entry *entry);
extern char *xml_entry(const struct Entry *entry);
extern char *get_logdir(void);
extern char *get_logfile_name(void);
extern FILE *lock_file(FILE *fp, const char *fname);
extern FILE *write_xml_header(FILE *fp);
extern FILE *seek_to_eof(FILE *fp, const char *fname);
extern FILE *unknown_end_line(FILE *fp, const char *fname);
extern FILE *check_last_log_line(FILE *fp, const char *fname);
extern FILE *seek_to_entry_pos(FILE *fp, const char *fname);
extern FILE *open_logfile(const char *fname);
extern int add_to_logfile(FILE *fp, const struct Entry *entry);
extern int close_logfile(FILE *fp);

/* rcfile.c */
extern char *get_rcfilename(void);
extern char *has_key(const char *line, const char *keyword);
extern void parse_rc_line(const char *line, struct Rc *rc);
extern int read_rcfile(const char *rcfile, struct Rc *rc);

/* sessvar.c */
extern bool is_legal_desc_char(const unsigned char c);
extern bool is_valid_desc_string(const char *s);
extern int fill_sess(struct Entry *dest, const char *uuid,
                     const char *desc, const size_t desclen);
extern int get_sess_info(struct Entry *);

/* string.c */
extern char *check_hex(const char *hex, const size_t len);
extern char *trim_str_front(char *);
extern char *trim_str_end(char *);
extern char *utf8_check(const char *text);

/* suuid.c */
extern int msg(int, const char *, ...);
extern int myerror(const char *, ...);
extern void print_license(void);
extern void print_version(void);
extern void usage(int);
extern int choose_opt_action(struct Options *, int, struct option *);
extern int parse_options(struct Options *, int, char *[]);
extern char *process_comment_option(char *);
extern int fill_entry_struct(struct Entry *, struct Options *);
extern char *process_uuid(FILE *, struct Entry *);
extern bool init_randomness(void);

/* tag.c */
extern void rewind_tag(void);
extern bool tag_exists(char *);
extern char *get_next_tag(void);
extern char *store_tag(char *);

/* uuid.c */
extern bool valid_uuid(const char *u, const bool check_len);
extern char *scramble_mac_address(char *);
extern char *generate_uuid(void);
extern char *uuid_date(char *, char *);
extern bool is_valid_date(char *, bool);
extern char *uuid_date_from_uuid(char *, char *);
extern char *scan_for_uuid(char *);

#endif

/*
 * Global variables
 */

extern char *progname;
extern struct Options opt;
extern struct Rc rc;
extern struct Entry entry;
#if PERL_COMPAT
extern bool perlexit13;
#endif

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
