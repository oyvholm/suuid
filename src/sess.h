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

#ifndef _SESS_H
#define _SESS_H

/*
 * Defines
 */

#define VERSION       "0.0.0"
#define RELEASE_DATE  "2016-00-00"

#ifndef FAKE_HOST
#  define FAKE_HOST  1
#endif
#ifndef PERL_COMPAT
#  define PERL_COMPAT  1
#endif

#include "common.h"

/*
 * Function prototypes
 */

#if 1 /* Set to 0 to test without prototypes */

/* sess.c */
extern int verbose_level(const int action, ...);
extern int msg(const int verbose, const char *format, ...);
extern int myerror(const char *format, ...);
extern int print_license(void);
extern int print_version(void);
extern int usage(const int retval);
extern int choose_opt_action(struct Options *dest,
                             const int c, const struct option *opts);
extern int parse_options(struct Options *dest,
                         const int argc, char * const argv[]);

#endif

/*
 * Global variables
 */

extern char *progname;

#endif /* ifndef _SESS_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
