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

int test_errno(void)
{
	errno = EACCES;
	myerror("errno is EACCES");
	return EXIT_SUCCESS;
}

/*
 * selftest() - Run internal testing to check that it works on the current 
 * system. Executed if --selftest is used.
 * Returns number of failed tests.
 */

int selftest(void)
{
	int total = 1; /* Update this when adding or deleting tests */
	int fail = 0;

	printf("1..%u\n", total);
	if (!(fail += test_errno()))
		printf("ok 1 - myerror() when errno is EACCES\n");
	fprintf(stderr, "# %u/%u tests failed\n", fail, total);

	return fail;
}

/* vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 : */
