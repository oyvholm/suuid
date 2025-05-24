/*
 * uuid.c
 * File ID: 06472a8e-3744-11e6-8115-02010e0a6634
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

#include "suuid.h"

/*
 * check_hex() - Check that len bytes at the location pointed to by p are all 
 * legal lowercase hex chars. Return a pointer to the first invalid character 
 * or NULL if everything is ok.
 */

const char *check_hex(const char *hex, const size_t len)
{
	const char *p;

	assert(hex);

	for (p = hex; p < hex + len; p++) {
		if (!strchr("0123456789abcdef", *p))
			return p;
	}

	return NULL;
}

/*
 * write_hex() - Read `len` bytes of binary data from `src` and write it as 
 * lowercase hexadecimal to `dest`. `dest` must be at least double the size of 
 * `src` plus a terminating null byte. Returns `dest`.
 */

char *write_hex(char *dest, const unsigned char *src, size_t len)
{
	size_t i;

	assert(dest);
	assert(src);

	for (i = 0; i < len; i++)
		sprintf(dest + 2 * i, "%02x", src[i]);

	return dest;
}

/*
 * valid_uuid() - Check that the UUID pointed to by u is a valid UUID. If 
 * check_len is true, also check that the string length is exactly the same as 
 * a standard UUID, UUID_LENGTH chars.
 * Return true if valid, false if not.
 */

bool valid_uuid(const char *u, const bool check_len)
{
	assert(check_len == false || check_len == true);

	if (!u || strlen(u) < UUID_LENGTH)
		return false;
	if (check_len && strlen(u) != UUID_LENGTH)
		return false;

	/*
	 * Check that it only contains lowercase hex and dashes at the right 
	 * places.
	 */
	if (check_hex(u, 8) || u[8] != '-' || check_hex(u + 9, 4)
	    || u[13] != '-' || check_hex(u + 14, 4) || u[18] != '-'
	    || check_hex(u + 19, 4) || u[23] != '-' || check_hex(u + 24, 12))
		return false;

	/*
	 * At the moment only v1 UUIDs are allowed.
	 */
	if (u[14] != '1')
		return false;

	return true;
}

/*
 * scan_for_uuid() - Return a pointer to the first UUID in the string s, or 
 * NULL if no UUID was found.
 */

const char *scan_for_uuid(const char *s)
{
	const char *p = s;

	assert(s);

	while (strlen(p) >= UUID_LENGTH) {
		if (valid_uuid(p, false))
			return p;
		p++;
	}

	return NULL;
}

/*
 * create_uuid_time() - Store `tv` as 60-bit integer in `utime`. Returns 
 * nothing.
 */

void create_uuid_time(utime_t *utime, const struct timeval *tv)
{
	assert(utime);
	assert(tv);

	*utime = ((utime_t)tv->tv_sec * 10000000ULL)
	         + ((utime_t)tv->tv_usec * 10ULL)
	         + EPOCH_DIFF * 10000000ULL;
}

/*
 * fill_uuid_time() - Insert values from `tv` into `ut`. Returns nothing.
 */

void fill_uuid_time(struct uuid_time *ut, const struct timeval *tv)
{
	utime_t utime;

	assert(ut);
	assert(tv);

	create_uuid_time(&utime, tv);
	ut->low = (utime & 0xFFFFFFFFUL);
	ut->mid = ((utime >> 32) & 0xFFFF);
	ut->hi = ((utime >> 48) & 0xFFF);
	ut->hi |= (1 << 12);
}

/*
 * get_current_time() - Store the current system time in `tv`. If the computer 
 * is fast enough to create identical timestamps, repeat until the timestamp 
 * changes. Returns pointer to `tv` if ok or NULL if error.
 */

struct timeval *get_current_time(struct timeval *tv)
{
	struct timeval tvbuf;
	utime_t utime;
	unsigned long count = 0UL;
	const unsigned long maxcount = 1000000UL;
	static utime_t prevtime = 0ULL;

	assert(tv);

	while (1) {
		if (gettimeofday(&tvbuf, NULL)) {
			myerror("%s(): gettimeofday() failed", /* gncov */
			        __func__);
			return NULL; /* gncov */
		}
		create_uuid_time(&utime, &tvbuf);
		if (utime != prevtime)
			break;
		if (++count > maxcount) { /* gncov */
			myerror("%s(): Got the same timestamp" /* gncov */
			        " after %lu tries. System clock broken?",
			        __func__, maxcount);
			return NULL; /* gncov */
		}
	}
	prevtime = utime;
	memcpy(tv, &tvbuf, sizeof(tvbuf));

	return tv;
}

/*
 * get_clockseq() - Set u->clseq_hi and u->clseq_lo to next value or init them 
 * to random values if not initialised. Returns nothing.
 */

void get_clockseq(struct uuid *u)
{
	static unsigned short seq;
	static bool done_init = false;
	unsigned short val;

	assert(u);

	if (!done_init) {
		seq = (unsigned short)random() & 0xFFFF;
		done_init = true;
	}
	val = ++seq;

	u->clseq_lo = (unsigned char)val & 0xFF;
	u->clseq_hi = (unsigned char)((val & 0x3F00) >> 8);
	u->clseq_hi |= 0x80;
}

/*
 * valid_macaddr() - Check that macaddr is a valid MAC address.
 * Return true if OK, false if something is wrong.
 */

bool valid_macaddr(const char *macaddr)
{
	unsigned int first;

	assert(macaddr);

	if (check_hex(macaddr, 12)) {
		myerror("MAC address contains illegal characters, can only"
		        " contain hex digits");
		return false;
	}
	if (strlen(macaddr) != 12) {
		myerror("Wrong MAC address length, must be exactly 12 hex"
		        " digits");
		return false;
	}
	if (sscanf(macaddr, "%2x", &first) != 1) {
		myerror("%s(): sscanf() failed when scanning \"%s\"",
		        __func__, macaddr);
		return false;
	}
	if (!(first & 0x01)) {
		myerror("MAC address doesn't follow RFC 4122, multicast bit"
		        " not set");
		return false;
	}

	return true;
}

/*
 * generate_macaddr() - Generate random node address in `dest`, MACADDR_LENGTH 
 * bytes and set the multicast bit. Returns nothing.
 */

void generate_macaddr(unsigned char *dest)
{
	int i;

	assert(dest);

	for (i = 0; i < MACADDR_LENGTH; i++)
		dest[i] = (unsigned char)random() & 0xFF;
	dest[0] |= 0x01;
}

/*
 * scramble_mac_address() - Overwrite MAC address in `dest` with random 
 * hexadecimal digits as specified by RFC 4122. Returns nothing.
 */

void scramble_mac_address(char *dest)
{
	unsigned char buf[MACADDR_LENGTH];

	assert(dest);
	generate_macaddr(buf);
	write_hex(dest, buf, MACADDR_LENGTH);
}

/*
 * finish_uuid() - Write finished uuid to `dest`, use values in `u`. Returns 
 * pointer to `dest`.
 */

char *finish_uuid(char *dest, const struct uuid *u)
{
	assert(dest);
	assert(u);

	sprintf(dest, "%08lx-%04x-1%03x-%02x%02x-",
	              u->time.low & 0xFFFFFFFFUL,
	              (unsigned int)u->time.mid & 0xFFFF,
	              (unsigned int)u->time.hi  & 0xFFF,
	              (unsigned int)u->clseq_hi & 0xFF,
	              (unsigned int)u->clseq_lo & 0xFF);
	write_hex(dest + 24, u->node, MACADDR_LENGTH);

	return dest;
}

/*
 * generate_uuid() - Write new unique uuid v1 to `uuid`, a buffer containing at 
 * least UUID_LENGTH + 1 bytes. Returns pointer to `uuid` or NULL if error.
 */

char *generate_uuid(char *uuid)
{
	struct uuid u;
	struct timeval currtime;

	assert(uuid);

	if (!get_current_time(&currtime))
		return NULL; /* gncov */
	fill_uuid_time(&u.time, &currtime);
	get_clockseq(&u);
	generate_macaddr(u.node);
	finish_uuid(uuid, &u);
	if (!valid_uuid(uuid, true))
		return NULL; /* gncov */

	return uuid;
}

/*
 * is_valid_date() - Check that the date pointed to by s is valid. If check_len 
 * is true, also check that the string length is correct.
 * Return 1 if ok, 0 if invalid.
 */

bool is_valid_date(const char *src, const bool check_len)
{
	const unsigned char *s = (const unsigned char *)src;

	assert(s);
	assert(check_len == false || check_len == true);

	if (check_len && strlen((const char *)s) != DATE_LENGTH)
		return false;

	if (s[0] != '2' || s[1] != '0' /* Yay for Y2.1K */
	    || !isdigit(s[2]) || !isdigit(s[3]) /* Two last digits in year */
	    || s[4] != '-'
	    || !strchr("01", s[5]) || !isdigit(s[6]) /* Month */
	    || s[7] != '-'
	    || !strchr("0123", s[8]) || !isdigit(s[9]) /* Day */
	    || s[10] != 'T'
	    || !strchr("012", s[11]) || !isdigit(s[12]) /* Hour */
	    || s[13] != ':'
	    || !strchr("012345", s[14]) || !isdigit(s[15]) /* Minute */
	    || s[16] != ':'
	    || !strchr("0123456", s[17]) || !isdigit(s[18]) /* Second */
	    || s[19] != '.'
	    || !isdigit(s[20]) || !isdigit(s[21]) || !isdigit(s[22])
	    || !isdigit(s[23]) || !isdigit(s[24]) || !isdigit(s[25])
	    || !isdigit(s[26]) || /* Nanoseconds */
	    s[27] != 'Z')
		return false;
	else
		return true;
}

/*
 * uuid_date() - Receive an UUID v1 and write the UUID date to dest, 29 bytes 
 * (ISO 8601 date plus terminating null byte). Return pointer to dest if ok, or 
 * NULL if it's not a valid v1 UUID.
 */

char *uuid_date(char *dest, const char *uuid)
{
	char hexbuf[16];
	utime_t val;
	utime_t nano; /* Same type as `val` due to modulus */
	time_t timeval;
	struct tm *tm;
	char *p;
#ifdef VERIFY_UUID
	char chkbuf[DATE_LENGTH + 1];
	char *chkres;
#endif

	assert(dest);
	assert(uuid);

	if (!valid_uuid(uuid, false))
		return NULL;
	if (uuid[14] != '1')
		return NULL; /* Not a v1 UUID, has no timestamp */

	memset(hexbuf, 0, 16);
	strncat(hexbuf, uuid + 15, 3);
	strncat(hexbuf, uuid + 9, 4);
	strncat(hexbuf, uuid, 8);

	val = strtoull(hexbuf, NULL, 16);
	nano = val % 10000000ULL;
	timeval = (time_t)((val / 10000000ULL) - EPOCH_DIFF);
	tm = gmtime(&timeval);

	memset(dest, 0, DATE_LENGTH + 1);
	strftime(dest, DATE_LENGTH, "%Y-%m-%dT%H:%M:%S", tm);
	p = dest + strlen(dest);
	if (p - dest != 19) {
		myerror("%td: Invalid date length, should be 19", /* gncov */
		        (ptrdiff_t)(p - dest)); /* gncov */
		return NULL; /* gncov */
	}
	snprintf(p, DATE_LENGTH - 19 + 1, ".%07lluZ", nano);
	if (!is_valid_date(dest, 1)) {
		myerror("%s(): %s: Generated invalid timestamp", /* gncov */
		        __func__, dest);
		return NULL; /* gncov */
	}

#ifdef VERIFY_UUID
	chkres = uuid_date_from_uuid(chkbuf, uuid);
	if (chkres) {
		if (strcmp(dest, chkbuf)) {
			myerror("%s(): UUID DATE DIFFERENCE:", __func__);
			myerror("%s(): dest   = \"%s\"", __func__, dest);
			myerror("%s(): chkbuf = \"%s\"", __func__, chkbuf);
			return NULL;
		}
	} else {
		myerror("%s(): uuid_date_from_uuid() failed, cannot verify"
		        " UUID timestamp", __func__);
		return NULL;
	}
#endif

	return dest;
}

#ifdef VERIFY_UUID
/*
 * uuid_date_from_uuid() - Same functionality as uuid_date(), but use the 
 * uuid(1) program to calculate the date. Use until uuid_date() works.
 */

char *uuid_date_from_uuid(char *dest, const char *uuid)
{
	FILE *fp;
	char cmd[50];
	char *ap, *p;

	assert(dest);
	assert(uuid);

	if (uuid[14] != '1')
		return NULL; /* Not a v1 UUID, has no timestamp */
	if (!valid_uuid(uuid, false))
		return NULL;

	memset(cmd, 0, 50);
	snprintf(cmd, 49, "uuid -d %s", uuid);

	fp = popen(cmd, "r");
	if (!fp) {
		myerror("%s(): Could not exec \"%s\"", __func__, cmd);
		return NULL;
	}
	ap = read_from_fp(fp, NULL);
	pclose(fp);

	p = strstr(ap, "content: time:");
	if (!p) {
		myerror("%s(): Search string not found in uuid(1) output",
		        __func__);
		return NULL;
	}
	p = strstr(p, "20"); /* This is how you create Y2.1K problems, kids */
	if (!p) {
		myerror("%s(): Didn't find year in uuid(1) output", __func__);
		return NULL;
	}

	p[10] = 'T';
	p[26] = p[27];
	p[27] = 'Z';
	p[28] = '\0';

	strncpy(dest, p, DATE_LENGTH + 1);
	free(ap);
	if (!is_valid_date(dest, true)) {
		fprintf(stderr, "%s(): Generated date is"
		                " invalid: \"%s\"\n", __func__, dest);
		return NULL;
	}

	return dest;
}
#endif

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
