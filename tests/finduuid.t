#!/usr/bin/env perl

#==============================================================================
# finduuid.t
# File ID: 008facd0-f988-11dd-bf3b-000475e441b9
#
# Test suite for finduuid(1).
#
# Character set: UTF-8
# ©opyleft 2008– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of file for 
# legal stuff.
#==============================================================================

use strict;
use warnings;

BEGIN {
	use Test::More qw{no_plan};
	# use_ok() goes here
}

use Getopt::Long;

local $| = 1;

our $CMD_BASENAME = "finduuid";

our %Opt = (

	'all' => 0,
	'help' => 0,
	'quiet' => 0,
	'todo' => 0,
	'verbose' => 0,
	'version' => 0,

);

our $progname = $0;
$progname =~ s/^.*\/(.*?)$/$1/;
our $VERSION = '0.1.0';

my %descriptions = ();

Getopt::Long::Configure('bundling');
GetOptions(

	'all|a' => \$Opt{'all'},
	'help|h' => \$Opt{'help'},
	'quiet|q+' => \$Opt{'quiet'},
	'todo|t' => \$Opt{'todo'},
	'verbose|v+' => \$Opt{'verbose'},
	'version' => \$Opt{'version'},

) || die("$progname: Option error. Use -h for help.\n");

$Opt{'verbose'} -= $Opt{'quiet'};
$Opt{'help'} && usage(0);
if ($Opt{'version'}) {
	print_version();
	exit(0);
}

exit(main());

sub main {
	my $Retval = 0;
	my $CMD = "../$CMD_BASENAME";

	diag(sprintf('========== Executing %s v%s ==========',
	             $progname, $VERSION));

	if ($Opt{'todo'} && !$Opt{'all'}) {
		ok(1, "No todo tests here");
		done_testing(1);
		return 0;
	}

	test_standard_options($CMD);
	test_executable($CMD);
	diag('Testing finished.');
	done_testing(101);

	return $Retval;
}

sub test_standard_options {
	my $CMD = shift;

	diag('-h/--help option');
	likecmd("$CMD -h",
	        '/  Show this help/i',
	        '/^$/',
	        0,
	        'Option -h prints help screen');
	likecmd("$CMD --help",
	        '/  Show this help/i',
	        '/^$/',
	        0,
	        'Option --help prints help screen');

	diag('-v/--verbose option');
	likecmd("$CMD -hv",
	        '/^\n\S+ \d+\.\d+\.\d+/s',
	        '/^$/',
	        0,
	        'Option -v with -h returns version number and help screen');
	likecmd("$CMD --help --verbose",
	        '/^\n\S+ \d+\.\d+\.\d+/s',
	        '/^$/',
	        0,
	        'Option --verbose with --help returns version number ' .
	        'and help screen');

	diag('--version option');
	likecmd("$CMD --version",
	        '/^\S+ \d+\.\d+\.\d+/',
	        '/^$/',
	        0,
	        'Option --version returns version number');
	return;
}

sub test_executable {
	my $CMD = shift;

	test_without_options($CMD);
	test_date_option($CMD);
	test_filenames_option($CMD);
	test_first_option($CMD);
	test_line_option($CMD);
	test_remove_option($CMD);
	test_unique_option($CMD);
	test_multiple_options($CMD);

	return;
}

sub test_without_options {
	my $CMD = shift;

	diag("Without options");
	testcmd("$CMD </dev/null", # {{{
	        "",
	        "",
	        1,
	        "Read empty input",
	);

	# }}}
	testcmd("$CMD finduuid-files/std-random", # {{{
	        <<END,
2bd76352-88d5-11dd-8848-000475e441b9
END
	        "",
	        0,
	        "Find UUID inside random data",
	);

	# }}}
	testcmd("$CMD <finduuid-files/std-random", # {{{
	        <<END,
2bd76352-88d5-11dd-8848-000475e441b9
END
	        "",
	        0,
	        "Read random data from stdin",
	);

	# }}}
	testcmd("$CMD finduuid-files/compact", # {{{
	        <<END,
daa9b45c-88d5-11dd-be73-000475e441b9
c2680b68-144e-4f4e-9c1c-3fbb758a94d2
db3b0506-88d5-11dd-8e5b-000475e441b9
8b592e20-245f-4860-8ebf-0cbd5e2cf072
dbbee448-88d5-11dd-bf1c-000475e441b9
07370456-ea42-4808-bc74-e24602e52172
dc6d9380-88d5-11dd-beb6-000475e441b9
07ac5c92-f413-4fb3-b0c5-fa9d25cac4ff
dd293036-88d5-11dd-84ca-000475e441b9
6396c79f-859a-404b-b285-b71288973b3b
END
	        "",
	        0,
	        "Search file with many UUIDs stacked toghether",
	);

	# }}}

	return;
}

sub test_date_option {
	my $CMD = shift;

	diag("-d/--date option");
	testcmd("$CMD -d finduuid-files/text2", # {{{
	        "08CCB59A-88E1-11DD-A80C-000475E441B9" .
	          "(2008-09-22T19:59:58.7635610Z)\n",
	        "",
	        0,
	        "Option -d lists timestamp after UUID",
	);

	# }}}
	testcmd("$CMD --date -l finduuid-files/text2", # {{{
	        "here 08CCB59A-88E1-11DD-A80C-000475E441B9" .
	          "(2008-09-22T19:59:58.7635610Z)blabla\n",
	        "",
	        0,
	        "Option --date works with -l",
	);

	# }}}

	return;
}

sub test_filenames_option {
	my $CMD = shift;

	diag("-f/--filenames option");
	testcmd("$CMD -f finduuid-files/std-random " .
	        "finduuid-files/textfile", # {{{
	        <<END,
finduuid-files/std-random:2bd76352-88d5-11dd-8848-000475e441b9
finduuid-files/textfile:9829c1a8-88d5-11dd-9a24-000475e441b9
finduuid-files/textfile:fd5d1200-88da-11dd-b7cf-000475e441b9
finduuid-files/textfile:9829C1A8-88D5-11DD-9A24-000475E441B9
finduuid-files/textfile:9829C1A8-88D5-11DD-9A24-000475E441B9
finduuid-files/textfile:4e4e8d08-9b38-11df-9954-3793b0cfdf88
finduuid-files/textfile:9829C1A8-88D5-11DD-9A24-000475E441B9
finduuid-files/textfile:fd5d1200-88da-11dd-b7cf-000475e441b9
finduuid-files/textfile:ced8e04e-9b57-11df-9b37-d97f703ed9b7
finduuid-files/textfile:0625e3ca-9b6d-11df-bc5b-f1285fef4db2
finduuid-files/textfile:0629cc60-9b6d-11df-867d-bde64fd0a5c7
finduuid-files/textfile:062c7a0a-9b6d-11df-94d2-638823d95bf3
finduuid-files/textfile:062edbf6-9b6d-11df-9e48-0d35319cdba1
finduuid-files/textfile:0625e3ca-9b6d-11df-bc5b-f1285fef4db2
finduuid-files/textfile:0629cc60-9b6d-11df-867d-bde64fd0a5c7
finduuid-files/textfile:062c7a0a-9b6d-11df-94d2-638823d95bf3
finduuid-files/textfile:062edbf6-9b6d-11df-9e48-0d35319cdba1
END
	        "",
	        0,
	        "Option -f lists file name",
	);

	# }}}
	testcmd("$CMD --filenames <finduuid-files/std-random", # {{{
	        <<END,
-:2bd76352-88d5-11dd-8848-000475e441b9
END
	        "",
	        0,
	        "--filenames shows \"-\" as file name when reading from stdin",
	);

	# }}}

	return;
}

sub test_first_option {
	my $CMD = shift;

	diag("-1/--first option");
	testcmd("$CMD -1 finduuid-files/textfile", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Read from file, stop after the first UUID with -1",
	);

	# }}}
	testcmd("$CMD --first finduuid-files/textfile", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Read from file, stop after the first UUID with --first",
	);

	# }}}
	testcmd("$CMD -1 <finduuid-files/textfile", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Read from stdin, stop after the first UUID with -1",
	);

	# }}}
	testcmd("$CMD --first <finduuid-files/textfile", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Read from stdin, stop after the first UUID with --first",
	);

	# }}}
	testcmd("$CMD finduuid-files/textfile -1 finduuid-files/text2", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Several files, still returns only one UUID",
	);

	# }}}
	testcmd("$CMD finduuid-files/textfile finduuid-files/doesntexist -1", # {{{
	        "9829c1a8-88d5-11dd-9a24-000475e441b9\n",
	        "",
	        0,
	        "Ignore non-existing second file, one UUID is already found",
	);

	# }}}
	testcmd("$CMD -l1 finduuid-files/textfile", # {{{
	        <<END,
4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9
END
	        "",
	        0,
	        "-l (--line) and -1 (--first), print only one line",
	);

	# }}}
	testcmd("$CMD -l1f finduuid-files/textfile finduuid-files/text2", # {{{
	        <<END,
finduuid-files/textfile:4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9
END
	        "",
	        0,
	        "-l1f with two files, return only the first line",
	);

	# }}}

	return;
}

sub test_line_option {
	my $CMD = shift;

	diag("-l/--line option");
	testcmd("$CMD -l finduuid-files/textfile", # {{{
	        join("\n",
	             "4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "6 fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "8 once more 9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "9 yet another one 9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "10 unique + dup 4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "11 dup + unique fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "12 four uniques 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "13 four dups 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "",
	       ),
	        "",
	        0,
	        "-l, print whole line with UUID",
	);

	# }}}
	testcmd("$CMD --line <finduuid-files/textfile", # {{{
	        join("\n",
	             "4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "6 fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "8 once more 9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "9 yet another one 9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "10 unique + dup 4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "11 dup + unique fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "12 four uniques 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "13 four dups 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "",
	        ),
	        "",
	        0,
	        "--line, read from stdin and print whole line with UUID",
	);

	# }}}
	testcmd("$CMD -lf finduuid-files/textfile finduuid-files/text2", # {{{
	        join("\n",
	             "finduuid-files/textfile:4 dfv dsf " .
	               "9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "finduuid-files/textfile:6 " .
	               "fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "finduuid-files/textfile:8 once more " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "finduuid-files/textfile:9 yet another one " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "finduuid-files/textfile:10 unique + dup " .
	               "4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "finduuid-files/textfile:11 dup + unique " .
	               "fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "finduuid-files/textfile:12 four uniques " .
	               "0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "finduuid-files/textfile:13 four dups " .
	               "0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "finduuid-files/text2:here " .
	               "08CCB59A-88E1-11DD-A80C-000475E441B9blabla",
	             "",
	        ),
	        "",
	        0,
	        "Print filename and whole line with UUID",
	);

	# }}}
	testcmd("$CMD finduuid-files/compact --line", # {{{
	        join("",
	             "daa9b45c-88d5-11dd-be73-000475e441b9",
	             "c2680b68-144e-4f4e-9c1c-3fbb758a94d2",
	             "db3b0506-88d5-11dd-8e5b-000475e441b9",
	             "8b592e20-245f-4860-8ebf-0cbd5e2cf072",
	             "dbbee448-88d5-11dd-bf1c-000475e441b9",
	             "07370456-ea42-4808-bc74-e24602e52172",
	             "dc6d9380-88d5-11dd-beb6-000475e441b9",
	             "07ac5c92-f413-4fb3-b0c5-fa9d25cac4ff",
	             "dd293036-88d5-11dd-84ca-000475e441b9",
	             "6396c79f-859a-404b-b285-b71288973b3b",
	             "\n",
	        ),
	        "",
	        0,
	        "--line, print whole line containg many UUIDs",
	);

	# }}}

	return;
}

sub test_remove_option {
	my $CMD = shift;

	diag("--remove option");
	testcmd("$CMD --remove finduuid-files/textfile", # {{{
	        join("\n",
	             "1 dsfv sdfJada",
	             "2 kldfjnvdsv",
	             "3 dsfv dsfv dsf",
	             "4 dfv dsf ",
	             "5 dsfv dsf qw weqd",
	             "6 ",
	             "7 i ksjn klasjdnc",
	             "8 once more ",
	             "9 yet another one ",
	             "10 unique + dup  ",
	             "11 dup + unique  ",
	             "12 four uniques    ",
	             "13 four dups    ",
	             "",
	        ),
	        "",
	        0,
	        "Strip UUIDs from input",
	);

	# }}}
	testcmd("$CMD finduuid-files/compact --remove", # {{{
	        "",
	        "",
	        0,
	        "Nothing left from compact when using --remove",
	);

	# }}}
	testcmd("echo Nothing here | $CMD --remove", # {{{
	        "Nothing here\n",
	        "",
	        1,
	        "No UUIDs in the input, exit with 1",
	);

	# }}}

	return;
}

sub test_unique_option {
	my $CMD = shift;

	diag("-u/--unique option");
	testcmd("$CMD --unique -l finduuid-files/textfile", # {{{
	        join("\n",
	             "4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "6 fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "10 unique + dup 4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "11 dup + unique fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "12 four uniques 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "",
	        ),
	        "",
	        0,
	        "Print whole line with only one UUID + --unique works",
	);

	# }}}
	testcmd("$CMD -u -l <finduuid-files/textfile", # {{{
	        join("\n",
	             "4 dfv dsf 9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "6 fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "10 unique + dup 4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "11 dup + unique fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "12 four uniques 0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "",
	        ),
	        "",
	        0,
	        "Read from stdin and print unique uuids",
	);

	# }}}
	my $tmpf = "tmp-finduuid-unique1";
	create_file($tmpf, <<END); # {{{
00000000-0000-11e6-ba2e-030000000000
00000000-0000-11e6-ba2e-030000000001
00000000-0000-11e6-ba2e-030000000002
00000000-0000-11e6-ba2e-030000000002 00000000-0000-11e6-ba2e-030000000003
00000000-0000-11e6-ba2e-030000000003 00000000-0000-11e6-ba2e-030000000002
00000000-0000-11e6-ba2e-030000000004
00000000-0000-11e6-ba2e-030000000005 00000000-0000-11e6-ba2e-030000000006
00000000-0000-11e6-ba2e-030000000005 00000000-0000-11e6-ba2e-030000000002
00000000-0000-11e6-ba2e-030000000005 00000000-0000-11e6-ba2e-030000000007
END
	testcmd("$CMD --unique -l <$tmpf", # {{{
	        <<END,
00000000-0000-11e6-ba2e-030000000000
00000000-0000-11e6-ba2e-030000000001
00000000-0000-11e6-ba2e-030000000002
00000000-0000-11e6-ba2e-030000000002 00000000-0000-11e6-ba2e-030000000003
00000000-0000-11e6-ba2e-030000000004
00000000-0000-11e6-ba2e-030000000005 00000000-0000-11e6-ba2e-030000000006
00000000-0000-11e6-ba2e-030000000005 00000000-0000-11e6-ba2e-030000000007
END
	        "",
	        0,
	        "Read from stdin with -u -l, lines with new UUIDs are shown",
	);

	# }}}
	ok(unlink($tmpf), "Delete $tmpf");

	return;
}

sub test_multiple_options {
	my $CMD = shift;

	testcmd("$CMD -u -lf finduuid-files/textfile " .
	        "finduuid-files/text2", # {{{
	        join("\n",
	             "finduuid-files/textfile:4 dfv dsf " .
	               "9829c1a8-88d5-11dd-9a24-000475e441b9",
	             "finduuid-files/textfile:6 " .
	               "fd5d1200-88da-11dd-b7cf-000475e441b9",
	             "finduuid-files/textfile:10 unique + dup " .
	               "4e4e8d08-9b38-11df-9954-3793b0cfdf88 " .
	               "9829C1A8-88D5-11DD-9A24-000475E441B9",
	             "finduuid-files/textfile:11 dup + unique " .
	               "fd5d1200-88da-11dd-b7cf-000475e441b9 " .
	               "ced8e04e-9b57-11df-9b37-d97f703ed9b7",
	             "finduuid-files/textfile:12 four uniques " .
	               "0625e3ca-9b6d-11df-bc5b-f1285fef4db2 " .
	               "0629cc60-9b6d-11df-867d-bde64fd0a5c7 " .
	               "062c7a0a-9b6d-11df-94d2-638823d95bf3 " .
	               "062edbf6-9b6d-11df-9e48-0d35319cdba1",
	             "finduuid-files/text2:here " .
	               "08CCB59A-88E1-11DD-A80C-000475E441B9blabla",
	             "",
	        ),
	        "",
	        0,
	        "Print filename and whole line with unique uuids",
	);

	# }}}
	testcmd("$CMD -u finduuid-files/textfile finduuid-files/text2", # {{{
	        <<END,
9829c1a8-88d5-11dd-9a24-000475e441b9
fd5d1200-88da-11dd-b7cf-000475e441b9
4e4e8d08-9b38-11df-9954-3793b0cfdf88
ced8e04e-9b57-11df-9b37-d97f703ed9b7
0625e3ca-9b6d-11df-bc5b-f1285fef4db2
0629cc60-9b6d-11df-867d-bde64fd0a5c7
062c7a0a-9b6d-11df-94d2-638823d95bf3
062edbf6-9b6d-11df-9e48-0d35319cdba1
08CCB59A-88E1-11DD-A80C-000475E441B9
END
	        "",
	        0,
	        "Several files, -u only",
	);

	# }}}

	return;
}

sub testcmd {
	my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
	defined($descriptions{$Desc}) &&
		BAIL_OUT("testcmd(): '$Desc' description is used twice");
	$descriptions{$Desc} = 1;
	my $stderr_cmd = '';
	my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
	my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
	my $TMP_STDERR = "$CMD_BASENAME-stderr.tmp";
	my $retval = 1;

	if (defined($Exp_stderr)) {
		$stderr_cmd = " 2>$TMP_STDERR";
	}
	$retval &= is(`$Cmd$stderr_cmd`, $Exp_stdout, "$Txt (stdout)");
	my $ret_val = $?;
	if (defined($Exp_stderr)) {
		$retval &= is(file_data($TMP_STDERR),
		              $Exp_stderr, "$Txt (stderr)");
		unlink($TMP_STDERR);
	} else {
		diag("Warning: stderr not defined for '$Txt'");
	}
	$retval &= is($ret_val >> 8, $Exp_retval, "$Txt (retval)");

	return $retval;
}

sub likecmd {
	my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
	defined($descriptions{$Desc}) &&
		BAIL_OUT("likecmd(): '$Desc' description is used twice");
	$descriptions{$Desc} = 1;
	my $stderr_cmd = '';
	my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
	my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
	my $TMP_STDERR = "$CMD_BASENAME-stderr.tmp";
	my $retval = 1;

	if (defined($Exp_stderr)) {
		$stderr_cmd = " 2>$TMP_STDERR";
	}
	$retval &= like(`$Cmd$stderr_cmd`, $Exp_stdout, "$Txt (stdout)");
	my $ret_val = $?;
	if (defined($Exp_stderr)) {
		$retval &= like(file_data($TMP_STDERR),
		                $Exp_stderr, "$Txt (stderr)");
		unlink($TMP_STDERR);
	} else {
		diag("Warning: stderr not defined for '$Txt'");
	}
	$retval &= is($ret_val >> 8, $Exp_retval, "$Txt (retval)");

	return $retval;
}

sub file_data {
	# Return file content as a string
	my $File = shift;
	my $Txt;

	open(my $fp, '<', $File) or return undef;
	local $/ = undef;
	$Txt = <$fp>;
	close($fp);
	return $Txt;
}

sub create_file {
	# Create new file and fill it with data
	my ($file, $text) = @_;
	my $retval = 0;

	open(my $fp, ">$file") or return 0;
	print($fp $text);
	close($fp);
	$retval = is(file_data($file), $text,
	             "$file was successfully created");

	return $retval; # 0 if error, 1 if ok
}

sub print_version {
	# Print program version
	print("$progname $VERSION\n");
	return;
}

sub usage {
	# Send the help message to stdout
	my $Retval = shift;

	if ($Opt{'verbose'}) {
		print("\n");
		print_version();
	}
	print(<<"END");

Usage: $progname [options]

Contains tests for the $CMD_BASENAME(1) program.

Options:

  -a, --all
    Run all tests, also TODOs.
  -h, --help
    Show this help.
  -q, --quiet
    Be more quiet. Can be repeated to increase silence.
  -t, --todo
    Run only the TODO tests.
  -v, --verbose
    Increase level of verbosity. Can be repeated.
  --version
    Print version information.

END
	exit($Retval);
}

sub msg {
	# Print a status message to stderr based on verbosity level
	my ($verbose_level, $Txt) = @_;

	$verbose_level > $Opt{'verbose'} && return;
	print(STDERR "$progname: $Txt\n");
	return;
}

__END__

# This program is free software; you can redistribute it and/or modify it under 
# the terms of the GNU General Public License as published by the Free Software 
# Foundation; either version 2 of the License, or (at your option) any later 
# version.
#
# This program is distributed in the hope that it will be useful, but WITHOUT 
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS 
# FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License along with 
# this program.
# If not, see L<http://www.gnu.org/licenses/>.

# vim: set ts=8 sw=8 sts=8 noet fo+=w tw=79 fenc=UTF-8 :
