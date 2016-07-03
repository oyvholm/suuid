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

#define VERSION       "0.0.0"
#define RELEASE_DATE  "2016-00-00"

#define FALSE  0
#define TRUE   1

#define EXIT_OK     0
#define EXIT_ERROR  1

#define T_RESET  "\x1b[m\x0f"
#define T_RED    "\x1b[31m"
#define T_GREEN  "\x1b[32m"

#define DATE_LENGTH  28 /* Length of ISO date format with nanoseconds */
#define UUID_LENGTH  36 /* Length of a standard UUID */

#define FAKE_HOST  1 /* Use "fake" as hostname to avoid conflicts with files 
                      * created by the Perl version
                      */
#define PERL_COMPAT  1 /* Compile a version with some changes to make it 
                        * compatible with the Perl version
                        */

#define stddebug  stderr

#define ENV_SESS  "SESS_UUID" /* Name of environment variable where the session 
                               * information is stored
                               */
#define ENV_HOSTNAME  "SUUID_HOSTNAME" /* Optional environment variable */
#define ENV_LOGDIR  "SUUID_LOGDIR" /* Optional environment variable with path 
                                    * to log directory
                                    */
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

#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Macros
 */

#define DEBL  msg(2, "%s, line %u in %s()", __FILE__, __LINE__, __FUNCTION__)
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

/* environ.c */
extern char *get_hostname(void);
extern char *getpath(void);
extern char *get_username(void);
extern char *get_tty(void);

/* io.h */
extern char *read_from_fp(FILE *);

/* logfile.c */
extern void init_xml_entry(struct Entry *);
extern char *allocate_entry(char *, char *);
extern char *suuid_xml(char *);
extern char *alloc_attr(char *, char *);
extern char *get_xml_tags(void);
extern char *create_sess_xml(struct Entry *);
extern char *xml_entry(struct Entry *);
extern char *get_logdir();
extern int add_to_logfile(char *, struct Entry *);
extern char *create_logfile(char *);
extern char *set_up_logfile(struct Options *, char *);
extern bool valid_xml_chars(char *);

/* rcfile.c */
extern int read_rcfile(char *, struct Rc *);

/* sessvar.c */
extern bool is_legal_desc_char(unsigned char);
extern bool is_valid_desc_string(char *);
extern int fill_sess(struct Entry *, char *, char *, size_t);
extern int get_sess_info(struct Entry *);

/* string.c */
extern char *trim_str_front(char *);
extern char *trim_str_end(char *);

/* suuid.c */
extern int msg(int, const char *, ...);
extern int myerror(const char *, ...);
extern void print_license(void);
extern void print_version(void);
extern void usage(int);
extern int choose_opt_action(struct Options *, int, struct option *);
extern int parse_options(struct Options *, int, char *[]);
extern int fill_entry_struct(struct Entry *, struct Options *, struct Rc *);
extern char *process_uuid(char *, struct Entry *);

/* tag.c */
void rewind_tag(void);
bool tag_exists(char *);
char *get_next_tag(void);
extern char *store_tag(char *);

/* utf8.c */
extern char *utf8_check(char *);

/* uuid.c */
extern char *generate_uuid(void);
extern char *uuid_date(char *, char *);
extern char *check_hex(char *, size_t);
extern char *scan_for_uuid(char *);
extern bool valid_uuid(char *, bool);

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
