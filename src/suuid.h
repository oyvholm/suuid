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

#define FAKE_HOST  1 /* Use "fake" as hostname to avoid conflicts with files 
                      * created by the Perl version
                      */

#define stddebug  stderr

#define ENV_LOGDIR  "SUUID_LOGDIR" /* Optional environment variable with path 
                                    * to log directory
                                    */

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

#define in_range(a,b,c)  ((a) >= (b) && (a) <= (c) ? TRUE : FALSE)

/*
 * Typedefs
 */

typedef unsigned char bool;
struct Rc {
	char *hostname;
	char *uuidcmd;
};
struct Entry {
	char *date;
	char *uuid;
	char *tag;
	char *txt;
	char *host;
	char *cwd;
	char *user;
	char *tty;
	char *sess;
};
struct Options {
	char *comment;
	unsigned int count;
	int help;
	int license;
	char *logdir;
	char *rcfile;
	int verbose;
	int version;
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
extern char *xml_entry(struct Entry *);
extern char *get_logdir();
extern int add_to_logfile(char *, struct Entry *);
extern char *create_logfile(char *);
extern char *set_up_logfile(struct Options *, char *);
extern int valid_xml_chars(char *);

/* rcfile.c */
extern int read_rcfile(char *, struct Rc *);

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

/* utf8.c */
extern unsigned char *utf8_check(unsigned char *);

/* uuid.c */
extern char *generate_uuid(void);
extern char *uuid_date(char *);
extern char *check_hex(char *, size_t);
extern int valid_uuid(char *);

/*
 * Global variables
 */

extern char *progname;
extern struct Options opt;
extern struct Rc rc;

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
