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

char *check_hex(const char *hex, const size_t len)
{
	const char *p;

	assert(hex);

	for (p = hex; p < hex + len; p++)
		if (!strchr("0123456789abcdef", *p))
			return (char *)p;

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
		sprintf(dest + 2*i, "%02x", src[i]);

	return dest;
}

/*
 * valid_macaddr() - Check that macaddr is a valid MAC address.
 * Return true if OK, false if something is wrong.
 */

bool valid_macaddr(const char *macaddr)
{
	unsigned int first;

	if (check_hex(macaddr, 12)) {
		fprintf(stderr, "%s: MAC address contains illegal characters, "
		        "can only contain hex digits\n", progname);
		return false;
	}
	if (strlen(macaddr) != 12) {
		fprintf(stderr, "%s: Wrong MAC address length, "
		        "must be exactly 12 hex digits\n", progname);
		return false;
	}
	if (sscanf(macaddr, "%2x", &first) != 1) {
		fprintf(stderr, "%s: valid_macaddr(): sscanf() failed when "
		                "scanning \"%s\"\n", progname, macaddr);
		return false;
	}
	if (!(first & 0x01)) {
		fprintf(stderr, "%s: MAC address doesn't follow RFC 4122, "
		                "multicast bit not set\n",
		                progname);
		return false;
	}

	return true;
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
	if (check_hex(u, 8) || u[8] != '-' || check_hex(u + 9, 4) ||
	    u[13] != '-' || check_hex(u + 14, 4) || u[18] != '-' ||
	    check_hex(u + 19, 4) || u[23] != '-' || check_hex(u + 24, 12))
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

char *scan_for_uuid(const char *s)
{
	const char *p = s;

	assert(s);

	while (strlen(p) >= UUID_LENGTH) {
		if (valid_uuid(p, false))
			return (char *)p;
		p++;
	}

	return NULL;
}

/*
 * generate_macaddr() - Generate random node address in `dest`, MACADDR_LENGTH 
 * bytes and set the multicast bit. Returns pointer to dest or NULL if error.
 */

unsigned char *generate_macaddr(unsigned char *dest)
{
	int i;

	assert(dest);

	for (i = 0; i < MACADDR_LENGTH; i++)
		dest[i] = random() & 0xFF;
	dest[0] |= 0x01;
#ifdef VERIFY_UUID
	char chkbuf[MACADDR_LENGTH * 2 + 1];
	write_hex(chkbuf, dest, MACADDR_LENGTH);
	if (!valid_macaddr(chkbuf)) {
		fprintf(stderr, "%s: generate_macaddr() generated invalid "
		                "address: \"%s\"\n", progname, chkbuf);
		dest = NULL;
	}
#endif

	return dest;
}

/*
 * scramble_mac_address() - Overwrite the last 12 characters of the received 
 * pointer to an UUID with random bytes as specified by RFC 4122. Return 
 * pointer to the UUID if successful, or NULL if an invalid UUID was received.
 */

char *scramble_mac_address(char *uuid)
{
	unsigned char buf[MACADDR_LENGTH];
	char *macaddr;

	assert(uuid);
	macaddr = uuid + 24;
	if (!valid_uuid(uuid, false))
		return NULL;
	generate_macaddr(buf);
	write_hex(macaddr, buf, MACADDR_LENGTH);
#ifdef VERIFY_UUID
	if (!valid_macaddr(macaddr)) {
		fprintf(stderr, "%s: scramble_mac_address() generated invalid "
		                "address: \"%s\"\n", progname, macaddr);
		return NULL;
	}
#endif

	return uuid;
}

/*
 * generate_uuid() - Return a pointer to a string with a generated UUID v1, or 
 * NULL if error.
 */

char *generate_uuid(char *uuid, const struct Rc *rc)
{
	char *cmd = "uuid";
	FILE *fp;

	assert(rc);

	/* fixme: Generate it properly */
	fp = popen(cmd, "r");
	if (!fp) {
		fprintf(stderr, "%s: generate_uuid(): Could not exec \"%s\"\n",
		                progname, cmd);
		return NULL;
	}
	if (!fgets(uuid, UUID_LENGTH + 1, fp)) {
		/*
		 * Nevermind read errors, valid_uuid() checks if it's valid 
		 * later.
		 */
	}
	uuid[UUID_LENGTH] = '\0';
	pclose(fp);

	if (!valid_uuid(uuid, true))
		return NULL;

	return uuid;
}

/*
 * is_valid_date() - Check that the date pointed to by s is valid. If check_len 
 * is true, also check that the string length is correct.
 * Return 1 if ok, 0 if invalid.
 */

bool is_valid_date(const char *s, const bool check_len)
{
	assert(s);
	assert(check_len == false || check_len == true);

	if (check_len && strlen(s) != DATE_LENGTH)
		return false;

	if (s[0] != '2' || s[1] != '0' || /* Yay for Y2.1K */
	    !isdigit((int)s[2]) || !isdigit((int)s[3]) || /* Two last digits 
	                                                     in year */
	    s[4] != '-' ||
	    !strchr("01", s[5]) || !isdigit((int)s[6]) || /* Month */
	    s[7] != '-' ||
	    !strchr("0123", s[8]) || !isdigit((int)s[9]) || /* Day */
	    s[10] != 'T' ||
	    !strchr("012", s[11]) || !isdigit((int)s[12]) || /* Hour */
	    s[13] != ':' ||
	    !strchr("012345", s[14]) || !isdigit((int)s[15]) || /* Minute */
	    s[16] != ':' ||
	    !strchr("0123456", s[17]) || !isdigit((int)s[18]) || /* Second */
	    s[19] != '.' ||
	    !isdigit((int)s[20])     || !isdigit((int)s[21]) ||
	        !isdigit((int)s[22]) || !isdigit((int)s[23]) ||
	        !isdigit((int)s[24]) || !isdigit((int)s[25]) ||
	        !isdigit((int)s[26]) || /* Nanoseconds */
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
	unsigned long long val;
	unsigned int nano;
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
	nano = val % 10000000;
	timeval = (val / 10000000) - EPOCH_DIFF;
	tm = gmtime(&timeval);

	memset(dest, 0, DATE_LENGTH + 1);
	strftime(dest, DATE_LENGTH, "%Y-%m-%dT%H:%M:%S", tm);
	p = dest + strlen(dest);
	if (p - dest != 19) {
		fprintf(stderr, "%s: %lu: Invalid date length, should be 19\n",
		                progname, (unsigned long)(p - dest));
		return NULL;
	}
	snprintf(p, DATE_LENGTH - 19 + 1, ".%07uZ", nano);
	if (!is_valid_date(dest, 1)) {
		fprintf(stderr, "%s: uuid_date(): %s: Generated invalid "
		                "timestamp\n", progname, dest);
		return NULL;
	}

#ifdef VERIFY_UUID
	chkres = uuid_date_from_uuid(chkbuf, uuid);
	if (chkres) {
		if (strcmp(dest, chkbuf)) {
			fprintf(stderr, "%s: uuid_date(): "
			                "UUID DATE DIFFERENCE:\n", progname);
			fprintf(stderr, "%s: uuid_date(): dest   = \"%s\"\n",
			                progname, dest);
			fprintf(stderr, "%s: uuid_date(): chkbuf = \"%s\"\n",
			                progname, chkbuf);
			return NULL;
		}
	} else {
		fprintf(stderr, "%s: uuid_date(): uuid_date_from_uuid() "
		                "failed, cannot verify UUID timestamp\n",
		                progname);
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
		fprintf(stderr, "%s: uuid_date_from_uuid(): "
		                "Could not exec \"%s\"\n", progname, cmd);
		return NULL;
	}
	ap = read_from_fp(fp);
	pclose(fp);

	p = strstr(ap, "content: time:");
	if (!p) {
		fprintf(stderr, "%s: uuid_date_from_uuid(): Search string not "
		                 "found in uuid(1) output\n", progname);
		return NULL;
	}
	p = strstr(p, "20"); /* This is how you create Y2.1K problems, kids */
	if (!p) {
		fprintf(stderr, "%s: uuid_date_from_uuid(): Didn't find year "
		                "in uuid(1) output\n", progname);
		return NULL;
	}

	p[10] = 'T';
	p[26] = p[27];
	p[27] = 'Z';
	p[28] = '\0';

	strncpy(dest, p, DATE_LENGTH + 1);
	free(ap);
	if (!is_valid_date(dest, true)) {
		fprintf(stderr, "uuid_date_from_uuid(): Generated date is "
		                "invalid: \"%s\"\n", dest);
		return NULL;
	}

	return dest;
}
#endif

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
