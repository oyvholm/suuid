/*
 * uuid.h
 * File ID: d289949e-3825-11e9-97fb-4f45262dc9b5
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

#ifndef _UUID_H
#define _UUID_H

#define DATE_LENGTH  28 /* Length of ISO date format with nanoseconds */
#define EPOCH_DIFF 12219292800
#define MACADDR_LENGTH  6 /* Length of MAC address */
#define UUID_LENGTH  36 /* Length of a standard UUID */

typedef unsigned long long utime_t;

/* uuid.c */
extern bool valid_uuid(const char *u, const bool check_len);
extern char *scan_for_uuid(const char *s);
extern bool valid_macaddr(const char *macaddr);
extern char *scramble_mac_address(char *uuid);
extern char *generate_uuid(char *uuid);
extern bool is_valid_date(const char *s, const bool check_len);
extern char *uuid_date(char *dest, const char *uuid);
#ifdef VERIFY_UUID
extern char *uuid_date_from_uuid(char *dest, const char *uuid);
#endif

#endif /* ifndef _UUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
