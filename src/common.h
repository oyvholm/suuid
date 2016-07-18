/*
 * common.h
 * File ID: 0ee270ea-46f8-11e6-bad2-2ba27a9a54c1
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

#ifndef _COMMON_H
#define _COMMON_H

/*
 * Standard header files
 */

#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <pwd.h>
#include <signal.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/file.h>
#include <sys/time.h>
#include <unistd.h>

#define FALSE  0
#define TRUE   1

#define EXIT_OK     0
#define EXIT_ERROR  1

#define T_RESET  "\x1b[m\x0f"
#define T_RED    "\x1b[31m"
#define T_GREEN  "\x1b[32m"

#define DATE_LENGTH  28 /* Length of ISO date format with nanoseconds */
#define UUID_LENGTH  36 /* Length of a standard UUID */

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
 * Function prototypes
 */

#if 1 /* Set to 0 to test without prototypes */

/* environ.c */
extern char *get_editor(void);
extern bool valid_hostname(const char *s);
extern char *get_hostname(const struct Rc *rc);
extern char *getpath(void);
extern char *get_username(void);
extern char *get_tty(void);

/* genuuid.c */
extern bool init_randomness(void);
extern void init_opt(struct Options *dest);
extern char *process_comment_option(const char *cmt);
extern int fill_entry_struct(struct Entry *entry, const struct Rc *rc,
                             const struct Options *opt);
extern char *process_uuid(FILE *logfp, const struct Rc *rc,
                          const struct Options *opt, struct Entry *entry);
extern void sighandler(const int sig);
extern struct uuid_result create_and_log_uuids(const struct Options *opt);

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
extern char *get_xml_tags(const struct Entry *entry);
extern char *create_sess_xml(const struct Entry *entry);
extern char *xml_entry(const struct Entry *entry, const bool raw);
extern char *get_logdir(const struct Options *opt);
extern char *get_logfile_name(const struct Rc *rc, const struct Options *opt);
extern FILE *lock_file(FILE *fp, const char *fname);
extern FILE *write_xml_header(FILE *fp);
extern FILE *seek_to_eof(FILE *fp, const char *fname);
extern FILE *unknown_end_line(FILE *fp, const char *fname);
extern FILE *check_last_log_line(FILE *fp, const char *fname);
extern FILE *seek_to_entry_pos(FILE *fp, const char *fname);
extern FILE *open_logfile(const char *fname);
extern int add_to_logfile(FILE *fp, const struct Entry *entry, const bool raw);
extern int close_logfile(FILE *fp);

/* rcfile.c */
extern char *get_rcfilename(const struct Options *opt);
extern char *has_key(const char *line, const char *keyword);
extern int parse_rc_line(const char *line, struct Rc *rc);
extern int read_rcfile(const char *rcfile, struct Rc *rc);

/* sessvar.c */
extern bool is_legal_desc_char(const unsigned char c);
extern bool is_valid_desc_string(const char *s);
extern char *get_desc_from_command(const char *cmd);
extern int fill_sess(struct Entry *dest, const char *uuid,
                     const char *desc, const size_t desclen);
extern int get_sess_info(struct Entry *entry);
extern char *concat_cmd_string(const int argc, char * const argv[]);
extern char *clean_up_sessvar(char *dest);
extern const char *add_to_sessvar(const char *desc, const char *uuid);
extern int run_session(const struct Options *orig_opt,
                       const int argc, char * const argv[]);

/* string.c */
extern char *check_hex(const char *hex, const size_t len);
extern char *trim_str_front(char *dest);
extern char *trim_str_end(char *dest);
extern char *utf8_check(const char *text);

/* tag.c */
extern void rewind_tag(void);
extern bool tag_exists(const struct Entry *entry, const char *tag);
extern char *get_next_tag(const struct Entry *entry);
extern char *store_tag(struct Entry *entry, const char *arg);

/* uuid.c */
extern bool valid_uuid(const char *u, const bool check_len);
extern char *scramble_mac_address(char *uuid);
extern char *generate_uuid(const struct Rc *rc, const bool random_mac);
extern char *uuid_date(char *dest, const char *uuid);
extern bool is_valid_date(const char *s, const bool check_len);
extern char *uuid_date_from_uuid(char *dest, const char *uuid);
extern char *scan_for_uuid(const char *s);

#endif

#endif /* ifndef _COMMON_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
