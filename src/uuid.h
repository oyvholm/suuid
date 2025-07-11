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
#define EPOCH_DIFF 12219292800ULL
#define MACADDR_LENGTH  6 /* Length of MAC address */
#define UUID_LENGTH  36 /* Length of a standard UUID */

typedef unsigned long long utime_t;

struct uuid_time {
	unsigned long low;
	unsigned short mid;
	unsigned short hi;
};
struct uuid {
	struct uuid_time time;
	unsigned char clseq_hi;
	unsigned char clseq_lo;
	unsigned char node[MACADDR_LENGTH];
};

/* uuid.c */
bool valid_uuid(const char *u, const bool check_len);
const char *scan_for_uuid(const char *s);
bool valid_macaddr(const char *macaddr);
void scramble_mac_address(char *dest);
char *generate_uuid(char *uuid);
bool is_valid_date(const char *src, const bool check_len);
char *uuid_date(char *dest, const char *uuid);
#ifdef VERIFY_UUID
char *uuid_date_from_uuid(char *dest, const char *uuid);
#endif

#endif /* ifndef _UUID_H */

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
