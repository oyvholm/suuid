/*
 * suuid.h
 * File ID: 289a8d22-2b93-11e6-879f-02010e0a6634
 *
 * (C)opyleft 2016- Øyvind A. Holm <sunny@sunbase.org>
 *
 * This program is free software; you can redistribute it and/or modify 
 * it under the terms of the GNU General Public License as published by 
 * the Free Software Foundation; either version 2 of the License, or (at 
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but 
 * WITHOUT ANY WARRANTY; without even the implied warranty of 
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU 
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License 
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _suuid_H
#define _suuid_H

/*
 * Defines
 */

#define FALSE  0
#define TRUE   1

#define EXIT_OK     0
#define EXIT_ERROR  1

#define FAKE_HOST 1

#define stddebug  stderr

#ifdef C_ASSERT
#ifdef NDEBUG
#undef NDEBUG
#endif /* ifdef NDEBUG        */
#else /* ifdef C_ASSERT      */
#define NDEBUG  1
#endif /* ifdef C_ASSERT else */

/*
 * Macros
 */

#define in_range(a,b,c)  ((a) >= (b) && (a) <= (c) ? TRUE : FALSE)
#define myerror(a)       { fprintf(stderr, "%s: ", progname); perror(a); }

/*
 * Standard header files
 */

#include <err.h>
#include <errno.h>
#include <getopt.h>
#include <limits.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

/*
 * Typedefs
 */

typedef unsigned char bool;
struct Entry {
	char *date;
	char *uuid;
	char *txt;
	char *host;
	char *cwd;
	char *user;
};

/*
 * Function prototypes
 */

extern int msg(int, const char *, ...);
extern void print_license(void);
extern void print_version(void);
extern void usage(int);
extern int add_to_logfile(char *, struct Entry *);
extern void create_logfile(char *);
extern char *generate_uuid(void);
extern char *get_hostname(void);
extern char *getpath(void);
extern char *uuid_date(char *);
extern char *xml_entry(struct Entry);

/*
 * Global variables
 */

extern char *progname;
extern struct Options opt;

#endif /* ifndef _suuid_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w fenc=UTF-8 : */
