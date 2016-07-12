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
 * Function prototypes
 */

#if 1 /* Set to 0 to test without prototypes */

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

#endif

/*
 * Global variables
 */

extern char *progname;

#endif /* ifndef _SUUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
