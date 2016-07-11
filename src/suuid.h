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

#define VERSION       "0.2.1"
#define RELEASE_DATE  "2016-07-10"



#define FAKE_HOST  1 /* Use "fake" as hostname to avoid conflicts with files 
                      * created by the Perl version
                      */
#define TEST_FUNC  0 /* Send non-option arguments to a function for testing. 
                      * Doesn't break anything, non-option arguments are 
                      * ignored by the program.
                      */

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

#include "common.h"
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
	int verbose;
	bool version;
	char *whereto;
};
struct uuid_result {
	unsigned int count;
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
extern int fill_sess(struct Entry *dest, const char *uuid,
                     const char *desc, const size_t desclen);
extern int get_sess_info(struct Entry *entry);

/* string.c */
extern char *check_hex(const char *hex, const size_t len);
extern char *trim_str_front(char *dest);
extern char *trim_str_end(char *dest);
extern char *utf8_check(const char *text);

/* suuid.c */
extern int verbose_level(const int action, ...);
extern int msg(const int verbose, const char *format, ...);
extern int myerror(const char *format, ...);
extern void print_license(void);
extern void print_version(void);
extern void usage(const int retval);
extern int choose_opt_action(struct Options *dest,
                             const int c, const struct option *opts);
extern int parse_options(struct Options *dest,
                         const int argc, char * const argv[]);

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

/*
 * Global variables
 */

extern char *progname;
extern struct Options opt;
extern struct Rc rc;
extern struct Entry entry;

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
