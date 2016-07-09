/*
 * sess.h
 * File ID: 4c971b52-459f-11e6-bee7-d32e100dee6b
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

#ifndef _sess_H
#define _sess_H

/*
 * Defines
 */

#define VERSION       "0.0.0"
#define RELEASE_DATE  "2016-00-00"

#define FALSE  0
#define TRUE   1

#define EXIT_OK     0
#define EXIT_ERROR  1

#define PERL_COMPAT  1 /* Compile a version with some changes to make it 
                        * compatible with the Perl version
                        */

#define stddebug  stderr

/*
 * Standard header files
 */

#include <errno.h>
#include <getopt.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
 * Macros
 */

#define DEBL  msg(2, "%s, line %u in %s()", __FILE__, __LINE__, __func__)
#define in_range(a,b,c)  ((a) >= (b) && (a) <= (c) ? TRUE : FALSE)

/*
 * Typedefs
 */

typedef unsigned char bool;
struct Options {
	bool help;
	bool license;
	int verbose;
	bool version;
};

/*
 * Function prototypes
 */

#if 1 /* Set to 0 to test without prototypes */

/* sess.c */
extern int msg(int, const char *, ...);
extern int myerror(const char *, ...);
extern void print_license(void);
extern void print_version(void);
extern void usage(int);

#endif

/*
 * Global variables
 */

extern char *progname;
extern struct Options opt;

#endif /* ifndef _sess_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
