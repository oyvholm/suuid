/*
 * selftest.c
 * File ID: ee49f58e-9f61-11e6-b9e0-e6436a218c69
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
 * selftest() - Run internal testing to check that it works on the current 
 * system. Executed if --selftest is used.
 */

int selftest(void)
{
	char buf1[] = "ABCÅÆØ";

	errno = EACCES;
	puts("# myerror(\"errno is EACCES\")");
	myerror("errno is EACCES");
	errno = 0;

	printf("string_to_lower(NULL) = %s\n", string_to_lower(NULL));
	printf("string_to_lower(\"%s\") = ", buf1);
	printf("%s\n", string_to_lower(buf1));

	printf("is_valid_date(\"2017-12-23T02:33:57Z\", 1) = %d\n",
	       is_valid_date("2017-12-23T02:33:57Z", 1));
	printf("is_valid_date(\"2017-12-23T02:33:57Z\", 0) = %d\n",
	       is_valid_date("2017-12-23T02:33:57Z", 0));
	printf("is_valid_date(\"2017-12-23T02:33:57.1234567Z\", 1) = %d\n",
	       is_valid_date("2017-12-23T02:33:57.1234567Z", 1));
	printf("is_valid_date(\"2017-12-23T02:33:57.1234567Z\", 0) = %d\n",
	       is_valid_date("2017-12-23T02:33:57.1234567Z", 0));
	printf("is_valid_date(\"2017-12-23T02:33:57.1234567Zabcd\", 0) = %d\n",
	       is_valid_date("2017-12-23T02:33:57.1234567Zabcd", 0));

	return EXIT_SUCCESS;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
