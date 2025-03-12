#!/usr/bin/env perl

#==============================================================================
# tests/suuid.t
# File ID: 7a006334-f988-11dd-8845-000475e441b9
#
# Test suite for suuid(1).
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

our $CMDB = "suuid";
our $CMD = "../$CMDB";
$ENV{'SESS_UUID'} = "";

our %Opt = (

	'all' => 0,
	'help' => 0,
	'quiet' => 0,
	'todo' => 0,
	'valgrind' => 0,
	'verbose' => 0,
	'version' => 0,

);

our $progname = $0;
$progname =~ s/^.*\/(.*?)$/$1/;
our $VERSION = '0.0.0'; # Not used here, $CMD decides

my %descriptions = ();

Getopt::Long::Configure('bundling');
GetOptions(

	'all|a' => \$Opt{'all'},
	'help|h' => \$Opt{'help'},
	'quiet|q+' => \$Opt{'quiet'},
	'todo|t' => \$Opt{'todo'},
	'valgrind' => \$Opt{'valgrind'},
	'verbose|v+' => \$Opt{'verbose'},
	'version' => \$Opt{'version'},

) || die("$progname: Option error. Use -h for help.\n");

$Opt{'verbose'} -= $Opt{'quiet'};
$Opt{'help'} && usage(0);
if ($Opt{'version'}) {
	print_version();
	exit(0);
}

my $cdata = '[^<]+';
my $Lh = "[0-9a-fA-F]";
my $Templ = "$Lh\{8}-$Lh\{4}-$Lh\{4}-$Lh\{4}-$Lh\{12}";
my $v1_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\{12}";
my $v1rand_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\[13579bdf]$Lh\{10}";
my $date_templ = '20[0-9][0-9]-[0-1][0-9]-[0-3][0-9]T'
                 . '[0-2][0-9]:[0-5][0-9]:[0-6][0-9]\.\d+Z';
my $xml_header = join("",
                      '<\?xml version="1\.0" encoding="UTF-8"\?>\n',
                      '<!DOCTYPE suuids SYSTEM "dtd\/suuids\.dtd">\n',
                      '<suuids>\n');

my $Outdir = "tmp-suuid-t-$$-" . substr(rand, 2, 8);

my $exec_version = `$CMD --version`;
my $FAKE_HOST = ($exec_version =~ /has FAKE_HOST/s) ? 1 : 0;

# Definitions from suuid.h
my $MAX_TAGS = 1000;

if ($Opt{'valgrind'}) {
	$CMD = "valgrind -q --leak-check=full --show-leak-kinds=all --"
	       . " ../$CMDB";
}

exit(main());

sub main {
	my $Retval = 0;

	diag('========== BEGIN version info ==========');
	diag(`$CMD --version`);
	diag('=========== END version info ===========');

	if ($Opt{'todo'} && !$Opt{'all'}) {
		goto todo_section;
	}

	$ENV{'HOME'} = "/dontexist/$Outdir";
	delete $ENV{'SESS_UUID'};
	delete $ENV{'SUUID_EDITOR'};
	delete $ENV{'SUUID_HOSTNAME'};
	delete $ENV{'SUUID_LOGDIR'};

	test_standard_options();
	test_suuid_selftest();
	test_test_functions();
	test_suuid_executable();

	diag('========== BEGIN version info ==========');
	diag(`$CMD --version`);
	diag('=========== END version info ===========');

	todo_section:
	;

	if ($Opt{'all'} || $Opt{'todo'}) {
		diag('Running TODO tests...');
		TODO: {
			local $TODO = '';
			# Insert TODO tests here.
		}
	}

	diag('Testing finished.');

	return $Retval;
}

sub test_standard_options {
	diag('Testing -v (--verbose) option...');
	likecmd("$CMD -hv",
	        '/^\n\S+ \d+\.\d+\.\d+/s',
	        '/^$/',
	        0,
	        'Option -v with -h returns version number and help screen');

	diag('--license option');
	likecmd("$CMD --license",
	        '/GNU General Public License'
	        . '.*'
	        . 'either version 2 of the License/s',
	        '/^$/',
	        0,
	        'Option --license displays the program license');

	diag('Unknown option');
	likecmd("$CMD --gurgle",
	        '/^$/',
	        "/\\.\\.\\/$CMDB: Option error\\n"
	        . "\\.\\.\\/$CMDB: Type \"\\.\\.\\/$CMDB --help\" for help"
	        . " screen\\. Returning with value 1\\.\\n/s",
	        1,
	        'Unknown option specified');
	return;
}

sub test_suuid_selftest {
	diag("--selftest");
	likecmd("$CMD --selftest",
	        '/.*/',
	        '/.*/',
	        0,
	        '--selftest');
}

sub test_s_suuid_sess {
	my ($l_desc, $l_slash, $l_uuid, $l_comma) = @_;
	my $fail = 0;
	my $str = "$l_desc$l_slash$l_uuid$l_comma";
	my $humstr = sprintf("s_suuid_sess() %s desc, %s slash, %s uuid,"
	                     . " %s comma",
	                     length($l_desc) ? "with" : "without",
	                     length($l_slash) ? "with" : "without",
	                     length($l_uuid) ? "with" : "without",
	                     length($l_comma) ? "with" : "without");
	length($l_slash) || ($fail = 1);
	length($l_comma) || ($fail = 1);
	length($l_uuid)  || ($fail = 1);
	if ($fail) {
		if (length($str)) {
			is(s_suuid_sess($str), undef, $humstr);
		}
	} else {
		like(s_suuid_sess($str),
		     '/^<sess( desc="deschere")?>'
		     . 'ff529c20-4522-11e2-8c4a-0016d364066c'
		     . '<\/sess> $/',
		     $humstr)
	}
}

sub test_test_functions {
	diag("Testing s_top()...");
	like("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
	     . "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">\n"
	     . "<suuids>\n"
	     . "</suuids>\n",
	     s_top(''),
	     "s_top('') returns empty file");

	diag("Testing s_suuid_tag()...");
	is(s_suuid_tag(''), '', "s_suuid_tag('') returns ''");
	is(s_suuid_tag('test'), '<tag>test</tag> ', "s_suuid_tag('test')");
	is(s_suuid_tag('test,lixom'),
	   '<tag>test</tag> <tag>lixom</tag> ',
	   "s_suuid_tag('test,lixom')");
	is(s_suuid_tag('test,lixom,på en måte'),
	   '<tag>test</tag> <tag>lixom</tag> <tag>på en måte</tag> ',
	   "s_suuid_tag('test,lixom,på en måte')");
	is(s_suuid_tag('test,lixom, space '),
	   '<tag>test</tag> <tag>lixom</tag> <tag> space </tag> ',
	   "s_suuid_tag('test,lixom, space ')");

	diag("Testing s_suuid_sess()...");
	is(s_suuid_sess(''), '', "s_suuid_sess('') returns ''");

	for my $l_desc ('deschere', '') {
		for my $l_slash ('/', '') {
			for my $l_uuid ('ff529c20-4522-11e2-8c4a-0016d364066c',
			                '') {
				for my $l_comma (',', '') {
					test_s_suuid_sess($l_desc, $l_slash,
					                  $l_uuid, $l_comma);
				}
			}
		}
	}

	is(s_suuid_sess('ff529c20-4522-11e2-8c4a-0016d364066c'),
	   undef,
	   "s_suuid_sess() without comma and slash returns undef");
	is(s_suuid_sess('ff529c20-4522-11e2-8c4a-0016d364066c,'),
	   undef,
	   "s_suuid_sess() with comma but missing slash returns undef");
	is(s_suuid_sess('xterm/ff529c20-4522-11e2-8c4a-0016d364066c'),
	   undef,
	   "s_suuid_sess() with desc, but missing comma returns undef");
	is(s_suuid_sess('/ff529c20-4522-11e2-8c4a-0016d364066c,'),
	   '<sess>ff529c20-4522-11e2-8c4a-0016d364066c</sess> ',
	   "s_suuid_sess() without desc, but with slash and comma");
	is(s_suuid_sess('xterm/ff529c20-4522-11e2-8c4a-0016d364066c,'),
	   '<sess desc="xterm">ff529c20-4522-11e2-8c4a-0016d364066c</sess> ',
	   "s_suuid_sess() with desc and comma");
	is(s_suuid_sess('xfce/bbd272a0-44e0-11e2-bcdb-0016d364066c,'
	                . 'xterm/c1986406-44e0-11e2-af23-0016d364066c,'
	                . 'screen/e7f897b0-44e0-11e2-b5a0-0016d364066c,'),
	   '<sess desc="xfce">bbd272a0-44e0-11e2-bcdb-0016d364066c</sess>'
	   . ' <sess desc="xterm">c1986406-44e0-11e2-af23-0016d364066c</sess>'
	   . ' <sess desc="screen">e7f897b0-44e0-11e2-b5a0-0016d364066c'
	   . '</sess> ',
	   's_suuid_sess() receives string with three with desc');
	is(s_suuid_sess('/ee5db39a-43f7-11e2-a975-0016d364066c,'
	                . '/da700fd8-43eb-11e2-889a-0016d364066c,'),
	   '<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess>'
	   . ' <sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> ',
	   "s_suuid_sess() receives two without desc");

	return;
}

sub test_suuid_executable {
	chomp(my $osname = `uname`);

	if (-e $Outdir) {
		die("$progname: $Outdir: WTF?? Directory element"
		    . " already exists.");
	}
	unless (mkdir($Outdir)) {
		die("$progname: $Outdir: Cannot mkdir(): $!\n");
	}

	diag("No options (except --logfile)...");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "No options (except -l) sends UUID to stdout");
	my $Outfile = glob("$Outdir/*.xml");
	defined($Outfile) || ($Outfile = '');
	like($Outfile, "/^$Outdir\\/\\S+\.xml\$/", "Filename of logfile OK");
	like(file_data($Outfile),
	     s_top(s_suuid()),
	     "Log contents OK after exec with no options");
	testcmd("$CMD -l $Outdir >/dev/null",
	        '',
	        '',
	        0,
	        "Redirect stdout to /dev/null");
	like(file_data($Outfile),
	     s_top(s_suuid() . s_suuid()),
	     "Entries are added, not replacing");
	ok(unlink($Outfile), "Delete [Outfile]");
	read_long_text_from_stdin($CMD, $Outdir, $Outfile);

	diag("Read the SUUID_LOGDIR environment variable...");
	likecmd("SUUID_LOGDIR=$Outdir $CMD -vvv",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read SUUID_LOGDIR environment variable, use -vvv");
	like(file_data($Outfile),
	     s_top(s_suuid()),
	     "The SUUID_LOGDIR environment variable was read");
	ok(unlink($Outfile), "Delete [Outfile]");

	diag("Read the SUUID_HOSTNAME environment variable...");
	likecmd("SUUID_HOSTNAME=urk13579kru $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read SUUID_HOSTNAME environment variable");
	my $filename = $FAKE_HOST ? "fake" : "urk13579kru";
	like(file_data("$Outdir/$filename.xml"),
	     s_top(s_suuid('host' => $filename)),
	     "The SUUID_HOSTNAME environment variable was read");
	ok(unlink("$Outdir/$filename.xml"), "Delete [Outdir]/$filename.xml");

	diag("Testing -m (--random-mac) option...");
	likecmd("$CMD -m -l $Outdir",
	        "/^$v1rand_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "--random-mac option works");
	like(file_data($Outfile),
	     s_top(s_suuid()),
	     "Log contents OK after --random-mac");
	ok(unlink($Outfile), "Delete [Outfile]");

	diag("Testing --raw option...");
	likecmd("$CMD --raw -c '<dingle><dangle>bær</dangle></dingle>'"
	        . " -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "--raw option works");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => ' <dingle><dangle>bær<\/dangle>'
	                            . '<\/dingle> ')),
	     "Log contents after --raw is OK");
	ok(unlink($Outfile), "Delete [Outfile]");

	diag("Testing --rcfile option...");
	likecmd("$CMD --rcfile rcfile1 -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "--rcfile option works");
	$filename = $FAKE_HOST ? "fake" : "altrc1";
	like(file_data("$Outdir/$filename.xml"),
	     s_top(s_suuid('host' => $filename)),
	     "hostname from rcfile1 is stored in the file");
	ok(unlink("$Outdir/$filename.xml"), "Delete [Outdir]/$filename.xml");

	ok(!-e 'nosuchrc', "'nosuchrc' doesn't exist");
	likecmd("$CMD --rcfile nosuchrc -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "--rcfile with non-existing file");
	ok(unlink($Outfile), "Delete [Outfile], line " . __LINE__);

	my $mac = "1b460a166a4d";
	create_file("rc-macaddr", "macaddr=$mac\n");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with valid macaddr");
	like(file_data($Outfile),
	     s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
	     "MAC address from the rc file is in the log file");
	ok(unlink($Outfile), "Delete [Outfile]");

	create_file("rc-macaddr", "macaddr=" . uc($mac) . "\n");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with valid macaddr, upper case");
	like(file_data($Outfile),
	     s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
	     "MAC address from the rc file is in the log file");
	ok(unlink($Outfile), "Delete [Outfile]");

	create_file("rc-macaddr", "   macaddr     =       $mac      \n");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with valid macaddr and lots of spaces");
	like(file_data($Outfile),
	     s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
	     "MAC address from the rc file is in the log file");
	ok(unlink($Outfile), "Delete [Outfile]");

	create_file("rc-macaddr", "macaddr=$mac");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with valid macaddr and no \\n at EOF");
	like(file_data($Outfile),
	     s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
	     "MAC address from the rc file is in the log file");
	ok(unlink($Outfile), "Delete [Outfile]");

	create_file("rc-macaddr", "macaddr =\n");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with macaddr keyword but no value, that's ok");
	like(file_data($Outfile), s_top(s_suuid()), "One entry created");
	ok(unlink($Outfile), "Delete [Outfile]");

	create_file("rc-macaddr", "macaddr =   \n");
	likecmd("$CMD --rcfile rc-macaddr -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "rc file with macaddr keyword and no value but spaces,"
	        . " that's ok");
	like(file_data($Outfile), s_top(s_suuid()), "One entry created");
	ok(unlink($Outfile), "Delete [Outfile]");

	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "10460a166a4d", 'macaddr-rfc-fail',
	                          "rc file with invalid macaddr,"
	                          . " doesn't follow the RFC");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "1b460a166a4", 'macaddr-wrong-length',
	                          "rc file with invalid macaddr,"
	                          . " one digit too short");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "${mac}a", 'macaddr-wrong-length',
	                          "rc file with invalid macaddr,"
	                          . " one digit too long");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "${mac}y", 'macaddr-wrong-length',
	                          "rc file with invalid macaddr,"
	                          . " extra invalid character");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "iiiiiiiiiiii", 'macaddr-invalid-digit',
	                          "rc file with invalid macaddr,"
	                          . " correct length, invalid hex digits");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "invalid", 'macaddr-invalid-digit',
	                          "rc file with invalid macaddr,"
	                          . " not a hex number at all");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "'$mac'", 'macaddr-invalid-digit',
	                          "rc file with valid macaddr,"
	                          . " but it's inside ''");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "\"$mac\"", 'macaddr-invalid-digit',
	                          "rc file with valid macaddr,"
	                          . " but it's inside \"\"");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          "= \"$mac\"", 'macaddr-invalid-digit',
	                          "rc file with valid macaddr,"
	                          . " but extra equal sign");
	invalid_macaddr_in_rcfile($Outdir, $Outfile,
	                          " = \"$mac\"", 'macaddr-invalid-digit',
	                          "rc file with valid macaddr,"
	                          . " but extra equal sign with space");
	ok(unlink("rc-macaddr"), "Delete rc-macaddr");

	diag("Testing -t (--tag) option...");
	likecmd("$CMD -t snaddertag -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "-t (--tag) option");
	testcmd("$CMD -t schn\xfcffelhund -l $Outdir",
	        "",
	        "../suuid: Tags have to be in UTF-8\n",
	        1,
	        "Refuse non-UTF-8 tags");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'snaddertag')),
	     "Log contents OK after tag");

	my $manytags = "";
	for (my $i = 1; $i <= $MAX_TAGS + 1; $i++) {
		$manytags .= " -tt$i";
	}
	likecmd("$CMD $manytags -l $Outdir",
	        '/^$/',
	        "/: Maximum number of tags \\($MAX_TAGS\\) exceeded/",
	        1,
	        "Number of tags is greater than MAX_TAGS");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -t abc,def,ghi -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Three tags in one argument separated with commas");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'abc,def,ghi')),
	     "Log contents OK after comma-separated tags");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -t abc,abc,abc -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Three comma-separated tags, duplicates");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'abc')),
	     "Duplicate comma-separated tags was removed");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -t abc --tag abc -t abc -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Three -t with duplicated tags");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'abc')),
	     "Duplicate tags was removed");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -t abc -t '' -t def -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "-t with empty tag");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'abc,def')),
	     "Empty tag was not stored");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD --tag '  abc  ' -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Tag with surrounding spaces");
	like(file_data($Outfile),
	        s_top(s_suuid('tag' => 'abc')),
	        "Remove surrounding spaces from tag");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD --tag '  with space  ' -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Tag with surrounding spaces and space in the middle");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'with space')),
	     "Remove surrounding spaces from tag, whitespace in tag kept");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD --tag '&<>' -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Tag with &, < and >");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => '&amp;&lt;&gt;')),
	     "&<> in tag was converted");
	ok(unlink($Outfile), "Delete [Outfile]");

	test_suuid_comment($Outdir, $Outfile);
	test_suuid_editor($Outdir, $Outfile);

	diag("Testing -n (--count) option...");
	testcmd("$CMD --count j",
	        "",
	        "../$CMDB: Error in -n/--count argument\n"
	        . "../$CMDB: Option error\n"
	        . "../$CMDB: Type \"../$CMDB --help\" for help screen."
	        . " Returning with value 1.\n",
	        1,
	        "Invalid value in --count argument");
	likecmd("$CMD -n 5 -c \"Great test\" -t testeri -l $Outdir",
	        "/^($v1_templ\n){5}\$/s",
	        '/^$/',
	        0,
	        "-n (--count) option with comment and tag");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'testeri', 'txt' => 'Great test') x 5),
	     "Log contents OK after count, comment and tag");
	if ($Opt{'all'}) {
		# Disable the testing of non-unique MAC addresses by default. 
		# It has never worked reliably and varies from computer to 
		# computer. It's random whether uuid(1) or uuidgen(1) gets it 
		# right. As an example, on this machine none of them works.
		diag("Check for randomness in the MAC address field...");
		cmp_ok(unique_macs($Outfile), '==', 1,
		       'MAC adresses does not change');
	}
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -m -n 5 -l $Outdir",
	        "/^($v1rand_templ\n){5}\$/s",
	        '/^$/',
	        0,
	        "-n (--count) option with -m (--random-mac)");
	cmp_ok(unique_macs($Outfile), '==', 5, 'MAC adresses are random');

	diag("Testing -w (--whereto) option...");
	likecmd("$CMD -w o -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "Output goes to stdout");
	likecmd("$CMD --whereto e -l $Outdir",
	        '/^$/s',
	        "/^$v1_templ\\n\$/s",
	        0,
	        "Output goes to stderr");
	likecmd("$CMD -w eo -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        "/^$v1_templ\\n\$/s",
	        0,
	        "Output goes to stdout and stderr");
	likecmd("$CMD -w a -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        "/^$v1_templ\\n\$/s",
	        0,
	        "Option -wa sends output to stdout and stderr");
	testcmd("$CMD -w n -l $Outdir",
	        '',
	        '',
	        0,
	        "Output goes nowhere");
	ok(unlink($Outfile), "Delete [Outfile]");

	test_suuid_environment($Outdir, $Outfile);

	diag("Test behaviour when unable to write to the log file...");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "Create logfile with one entry");

	my @stat_array = stat($Outfile);
	ok(unlink($Outfile), "Delete [Outfile]");
	ok(mkdir($Outfile), "mkdir [Outfile]");
	likecmd("$CMD -l $Outdir",
	        '/^$/s',
	        '/^\.\.\/suuid: .*?\.xml: Could not open file'
	        . ' for read\+write:'
	        . ' Is a directory\n'
	        . '$/s',
	        1,
	        "Unable to write to the log file");
	ok(rmdir($Outfile), "rmdir [Outfile]");

	diag("Test what happens when the end of the log file is messed up");
	ok(create_file($Outfile, ""), "Create empty log file");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^$/s',
	        0,
	        "Write to empty log file");
	like(file_data($Outfile),
	     s_top(s_suuid()),
	     "The empty file was initialised");

	ok(create_file($Outfile, "Destroyed file\n"),
	   "Create destroyed log file");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^\.\.\/suuid: tmp-suuid-t-\d+-\d+\/.*?\.xml:'
	        . ' Unknown end line, adding to end of file\n$/s',
	        0,
	        "Write to log file with destroyed EOF");
	like(file_data($Outfile),
	     '/^Destroyed file\n' . s_suuid() . '<\/suuids>\n$/s',
	     "New entry was added to end of file");

	ok(create_file($Outfile, "a"), "Create log file with one char");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^\.\.\/suuid: tmp-suuid-t-\d+-\d+\/.*?\.xml:'
	        . ' Unknown end line, adding to end of file\n$/s',
	        0,
	        "Write to log file containing one char");
	like(file_data($Outfile),
	     '/^a' . s_suuid() . '<\/suuids>\n$/s',
	     "New entry was added to EOF after that one character");
	ok(unlink($Outfile), "Delete [Outfile]");

	if ($osname eq "OpenBSD") {
		diag("NOTICE: SIGPIPE test hangs on OpenBSD, skipping test");
	} else {
		test_suuid_signal($Outfile);
	}
	ok(rmdir($Outdir), "rmdir [Outdir]");

	return;
}

sub test_suuid_comment {
	my ($Outdir, $Outfile) = @_;

	diag("Testing -c (--comment) option...");
	likecmd("$CMD -c \"Great test\" -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "-c (--comment) option");

	test_invalid_comment($CMD, 0, "F\xf8kka \xf8pp", $Outdir,
	                     "Refuse non-UTF-8 text to --comment option");
	test_invalid_comment($CMD, 0, "Ctrl-d: \x04", $Outdir,
	                     "Reject Ctrl-d in comment");
	test_invalid_comment($CMD, 0, "\x7F", $Outdir,
	                     "Reject U+007F (DELETE) in comment");
	test_invalid_comment($CMD, 0, "\xC1\xBF", $Outdir,
	                     "Overlong UTF-8 char in comment, 2 bytes");
	test_invalid_comment($CMD, 0, "\xE0\x80\xAF", $Outdir,
	                     "Overlong UTF-8 char in comment, 3 bytes");
	test_invalid_comment($CMD, 0, "\xF0\x80\x80\xAF", $Outdir,
	                     "Overlong UTF-8 char in comment, 4 bytes");
	test_invalid_comment($CMD, 0, "\xED\xA0\x80", $Outdir,
	                     "UTF-8 contains UTF-16 surrogate char U+D800");
	test_invalid_comment($CMD, 0, "\xEF\xBF\xBE", $Outdir,
	                     "UTF-8 contains U+FFFE");
	test_invalid_comment($CMD, 0, "\xEF\xBF\xBF", $Outdir,
	                     "UTF-8 contains U+FFFF");
	test_invalid_comment($CMD, 0, "\xF4\x90\x80\x80", $Outdir,
	                     "UTF-8 contains U+110000, is above U+10FFFF");
	likecmd("echo \"Great test\" | $CMD -c - -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read comment from stdin");

	test_invalid_comment($CMD, 1, "F\xf8kka \xf8pp", $Outdir,
	                     "Reject non-UTF-8 comment from stdin");
	test_invalid_comment($CMD, 1, "Ctrl-d: \x04", $Outdir,
	                     "Reject Ctrl-d in comment from stdin");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => 'Great test')
	           . s_suuid('txt' => 'Great test', 'tty' => '')),
	     "Log contents OK after comment");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("echo \"\xE0\xAD\xB2\" | $CMD -c - -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read comment from stdin with 3 byte UTF-8 sequence");

	like(file_data($Outfile),
	     s_top(s_suuid('txt' => "\xE0\xAD\xB2", 'tty' => '')),
	     "Log contents OK after 3-byte UTF-8 sequence");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("echo \"\xF0\x9D\x85\x9D\" | $CMD -c - -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read comment from stdin with 4 byte UTF-8 sequence");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => "\xF0\x9D\x85\x9D", 'tty' => '')),
	     "Log contents OK after 4-byte UTF-8 sequence");
	ok(unlink($Outfile), "Delete [Outfile]");

	my $tmpfile = ".$progname-dat.tmp";
	create_file($tmpfile, "A\tB\\C");
	likecmd("$CMD -c - -l $Outdir <$tmpfile",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read tab and backslash from stdin");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => 'A\\\\tB\\\\\\\\C', 'tty' => '')),
	     "Log ok after tab and backslash");
	ok(unlink($tmpfile), "Delete $tmpfile");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("$CMD -c - -l $Outdir </dev/null",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read empty comment from stdin");
	likecmd("$CMD --comment '' -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Enter empty comment with --comment");
	like(file_data($Outfile),
	     s_top(s_suuid('tty' => '') . s_suuid()),
	     "Log contents OK after empty comments");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("echo " . "a" x 82 . " | $CMD -t aa -c - -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read line with 82 bytes from stdin and use tag");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'aa', 'tty' => '', 'txt' => 'a' x 82)),
	     "Log contents OK after 82 character line");
	ok(unlink($Outfile), "Delete [Outfile]");

	return;
}

sub test_suuid_editor {
	my ($Outdir, $Outfile) = @_;

	diag("Create comment with external editor");

	$ENV{'SUUID_EDITOR'} = "./fake-editor";
	$ENV{'EDITOR'} = "./not-exist";
	execute_editor($Outdir, $Outfile,
	               "use SUUID_EDITOR, EDITOR is defined, but ignored");
	delete $ENV{'SUUID_EDITOR'};

	$ENV{'EDITOR'} = "./fake-editor";
	execute_editor($Outdir, $Outfile,
	               "SUUID_EDITOR is undefined, use EDITOR");

	$ENV{'EDITOR'} = "./not-exist";
	ok(!-e $ENV{'EDITOR'}, "$ENV{'EDITOR'} doesn't exist");
	my @tmpglob = glob(".tmp-suuid.*");
	scalar(@tmpglob) && unlink(@tmpglob);
	is(glob(".tmp-suuid.*"), undef, "No .tmp-suuid.* files exist");
	likecmd("$CMD --comment -- -l $Outdir",
	        '/^$/s',
	        "/\\.\\.\\/$CMDB: read_from_editor\\(\\): Cannot execute"
	        . " \"\\.\\/not-exist \\.tmp-suuid\\./",
	        1,
	        "\"--comment --\", SUUID_EDITOR is undefined,"
	        . " EDITOR has non-existing editor");
	@tmpglob = glob(".tmp-suuid.*");
	is(scalar(@tmpglob), 1, "One .tmp-suuid.* tempfile is created");
	ok(unlink($tmpglob[0]), "Delete tempfile");

	$ENV{'SUUID_EDITOR'} = "";
	$ENV{'EDITOR'} = "./fake-editor";
	execute_editor($Outdir, $Outfile, "SUUID_EDITOR is empty, use EDITOR");

	$ENV{'EDITOR'} = "";
	editor_fail($Outdir, $Outfile,
	            "both SUUID_EDITOR and EDITOR are empty");

	delete $ENV{'SUUID_EDITOR'};
	delete $ENV{'EDITOR'};
	editor_fail($Outdir, $Outfile,
	            "neither SUUID_EDITOR nor EDITOR are defined");
}

sub execute_editor {
	my ($Outdir, $Outfile, $msg) = @_;

	likecmd("$CMD --comment -- -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "\"--comment --\", $msg");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => 'Text from editor')),
	     "Log contents OK after $msg");
	ok(unlink($Outfile), "Delete [Outfile]");
}

sub editor_fail {
	my ($Outdir, $Outfile, $msg) = @_;

	testcmd("$CMD -c -- -l $Outdir",
	        "",
	        "../$CMDB: Environment variables SUUID_EDITOR and EDITOR"
	        . " aren't defined, cannot start editor\n",
	        1,
	        "\"-c --\", $msg");
	ok(!-e $Outfile, "[Outfile] doesn't exist");
}

sub test_invalid_comment {
	my ($CMD, $from_stdin, $txt, $Outdir, $desc) = @_;
	my $c_str = $from_stdin ? "echo \"$txt\" | $CMD -c - -l $Outdir"
	                        : "$CMD -c \"$txt\" -l $Outdir";

	testcmd($c_str,
	        "",
	        "../suuid: Comment contains illegal characters"
	        . " or is not valid UTF-8\n",
	        1,
	        $desc);
}

sub test_suuid_environment {
	my ($Outdir, $Outfile) = @_;
	my $bck_home;

	diag("Test logging of \$SESS_UUID environment variable...");
	likecmd("SESS_UUID=27538da4-fc68-11dd-996d-000475e441b9 $CMD -t yess"
	        . " -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Use SESS_UUID environment variable");
	like(file_data($Outfile),
	     s_top(s_suuid('tag' => 'yess',
	                   'sess' => '/27538da4-fc68-11dd-996d-'
	                             . '000475e441b9,')),
	     "\$SESS_UUID envariable is logged");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,"
	        . " $CMD"
	        . " -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with 'ssh-agent/'-prefix and comma at the end");
	like(file_data($Outfile),
	        s_top(s_suuid('sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-'
	                                . '0016d364066c,')),
	        "<sess> contains desc attribute");

	likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,"
	        . "dingle©/4c66b03a-43f4-11e2-b70d-0016d364066c,"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with 'ssh-agent' and 'dingle©'");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,')
	           . s_suuid('sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-'
	                               . '0016d364066c,'
	                               . 'dingle©/4c66b03a-43f4-11e2-b70d-'
	                               . '0016d364066c,')),
	     "<sess> contains both desc attributes, one with ©");
	ok(unlink($Outfile), "Delete [Outfile]");

	$ENV{'SESS_UUID'}="abc";
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID doesn't have UUID");
	ok(unlink($Outfile), "Delete [Outfile]");

	$ENV{'SESS_UUID'}="abcdef;b/4c66b03a-43f4-11e2-b70d-0016d364066c";
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID contains semicolon");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => 'b/4c66b03a-43f4-11e2-b70d-'
	                             . '0016d364066c,')),
	     "Everything before the semicolon is gone");
	delete $ENV{'SESS_UUID'};
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with 'ssh-agent', missing comma");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,')),
	     "<sess> is correct without comma");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=/da700fd8-43eb-11e2-889a-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID missing name and comma, but has slash");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,')),
	     "<sess> is OK without name and comma");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=ee5db39a-43f7-11e2-a975-0016d364066c,"
	        . "/da700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs, latter missing name and comma,"
	        . " but has slash");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/ee5db39a-43f7-11e2-a975-0016d364066c,'
	                             . '/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,')),
	     "Second <sess> is correct without comma");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=ee5db39a-43f7-11e2-a975-0016d364066c"
	        . "da700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs smashed together");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/ee5db39a-43f7-11e2-a975-0016d364066c,'
	                             . '/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,')),
	     "Still separates them into two UUIDs");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066c"
	        . "abc"
	        . "ee5db39a-43f7-11e2-a975-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs, only separated by 'abc'");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,'
	                             . 'abc/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "Separated the two UUIDs, keeps 'abc'");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066c"
	        . "abc/ee5db39a-43f7-11e2-a975-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs, separated by 'abc/'");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,'
	                             . 'abc/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "The two UUIDs are separated, 'abc/' is kept");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066c"
	        . "ee5db39a-43f7-11e2-a975-0016d364066c"
	        . "abc"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs together, 'abc' at EOS");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,'
	                             . '/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "The two UUIDs are found, 'abc' at EOS is discarded");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066c"
	        . ",,ee5db39a-43f7-11e2-a975-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs separated by two commas");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,'
	                             . '/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "The two UUIDs are found, ignoring useless commas");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID='"
	        . ",,,,,"
	        . "abc/da700fd8-43eb-11e2-889a-0016d364066c"
	        . ",,,,,,,,,,"
	        . "def/ee5db39a-43f7-11e2-a975-0016d364066c"
	        . ",,%..¤¤¤%¤,,,'"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID, lots of commas+punctuation"
	        . " and two UUIDS with descs");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => 'abc/da700fd8-43eb-11e2-889a-'
	                             . '0016d364066c,'
	                             . 'def/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "The two UUIDs with desc are found, ignore garbage");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID="
	        . "da700fd8-43eb-11e2-889a-0016d364066c"
	        . "def/ee5db39a-43f7-11e2-a975-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID with two UUIDs separated by 'def/'");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,'
	                             . 'def/ee5db39a-43f7-11e2-a975-'
	                             . '0016d364066c,')),
	     "The two UUIDs are found, 'def/' is kept with second UUID");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=5f650dac-4404-11e2-8e0e-0016d364066c"
	        . "5f660e28-4404-11e2-808e-0016d364066c"
	        . "5f66ef14-4404-11e2-8b45-0016d364066c"
	        . "5f67e266-4404-11e2-a6f8-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID contains four UUIDs, no separators");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/5f650dac-4404-11e2-8e0e-0016d364066c,'
	                             . '/5f660e28-4404-11e2-808e-0016d364066c,'
	                             . '/5f66ef14-4404-11e2-8b45-0016d364066c,'
	                             . '/5f67e266-4404-11e2-a6f8-'
	                             . '0016d364066c,')),
	     "All four UUIDs are separated");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=5f650dac-4404-11e2-8e0e-0016d364066c"
	        . "abc"
	        . "5f660e28-4404-11e2-808e-0016d364066c"
	        . "5f66ef14-4404-11e2-8b45-0016d364066c,"
	        . "nmap/5f67e266-4404-11e2-a6f8-0016d364066c"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID contains four UUIDs,"
	        . " 'abc' separates the first two, last one has desc");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => '/5f650dac-4404-11e2-8e0e-0016d364066c,'
	                             . 'abc/5f660e28-4404-11e2-808e-'
	                             . '0016d364066c,'
	                             . '/5f66ef14-4404-11e2-8b45-0016d364066c,'
	                             . 'nmap/5f67e266-4404-11e2-a6f8-'
	                             . '0016d364066c,')),
	     "All four UUIDs separated, 'abc' and 'nmap' kept");
	ok(unlink($Outfile), "Delete [Outfile]");

	likecmd("SESS_UUID=ssh-agent/fea9315a-43d6-11e2-8294-0016d364066c,"
	        . "logging/febfd0f4-43d6-11e2-9117-0016d364066c,"
	        . "screen/0e144c10-43d7-11e2-9833-0016d364066c,"
	        . "ti/152d8f16-4409-11e2-be17-0016d364066c,"
	        . " $CMD -l $Outdir",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "SESS_UUID is OK and contains four UUIDs, all with desc");
	like(file_data($Outfile),
	     s_top(s_suuid('sess' => 'ssh-agent/fea9315a-43d6-11e2-8294-'
	                             . '0016d364066c,'
	                             . 'logging/febfd0f4-43d6-11e2-9117-'
	                             . '0016d364066c,'
	                             . 'screen/0e144c10-43d7-11e2-9833-'
	                             . '0016d364066c,'
	                             . 'ti/152d8f16-4409-11e2-be17-'
	                             . '0016d364066c,')),
	     "The four UUIDs are separated, all four descs kept");
	ok(unlink($Outfile), "Delete [Outfile]");

	diag("Check behaviour without various environment variables");
	$bck_home = $ENV{'HOME'};
	delete $ENV{'HOME'};
	is($ENV{'HOME'},           undef, "HOME is undefined");
	is($ENV{'SESS_UUID'},      undef, "SESS_UUID is undefined");
	is($ENV{'SUUID_EDITOR'},   undef, "SUUID_EDITOR is undefined");
	is($ENV{'SUUID_HOSTNAME'}, undef, "SUUID_HOSTNAME is undefined");
	is($ENV{'SUUID_LOGDIR'},   undef, "SUUID_LOGDIR is undefined");
	likecmd("$CMD -l $Outdir",
	        "/^$v1_templ\\n\$/s",
	        '/^\.\.\/suuid: HOME environment variable not defined, cannot'
	        . ' determine name of rcfile\n$/s',
	        0,
	        "No HOME defined, but -l is set to [Outdir]");
	testcmd("$CMD",
	        '',
	        "../suuid: HOME environment variable not defined,"
	        . " cannot determine name of rcfile\n"
	        . "../suuid: \$SUUID_LOGDIR and \$HOME environment variables"
	        . " are not defined, cannot create logdir path\n",
	        1,
	        "Now it doesn't even have -l/--logdir");
	ok(unlink($Outfile), "Delete [Outfile]");

	$ENV{'HOME'} = $bck_home;
	test_invalid_hostname("", "SUUID_HOSTNAME is empty", $Outfile);
	test_invalid_hostname("a" x 101, "SUUID_HOSTNAME is too long,"
	                                 . " 101 bytes", $Outfile);
	test_invalid_hostname(";", "SUUID_HOSTNAME contains semicolon",
	                      $Outfile);
	test_invalid_hostname("a..b", "SUUID_HOSTNAME contains \"..\"",
	                      $Outfile);
	test_invalid_hostname("\xf8", "SUUID_HOSTNAME contains latin1 char",
	                      $Outfile);
	return;
}

sub test_invalid_hostname {
	my ($h, $desc, $Outfile) = @_;

	$ENV{SUUID_HOSTNAME} = $h;
	testcmd("$CMD",
	        '',
	        "../suuid: Got invalid hostname: \"$h\"\n",
	        1,
	        $desc);
	delete $ENV{SUUID_HOSTNAME};
}

sub test_suuid_signal {
	my $Outfile = shift;
	my $sigpipe_stderr = "$CMDB-stderr.tmp";

	diag("Receive SIGPIPE signal");
	likecmd("$CMD -l $Outdir -n 1000 -wo 2>\"$sigpipe_stderr\" | true",
	        '/^$/s',
	        '/^\.\.\/suuid: Termination signal \(Broken pipe\) received,'
	        . ' aborting\n'
	        . '\.\.\/suuid: Generated only \d+ of 1000 UUIDs\n$/s',
	        0,
	        "Receive SIGPIPE");
	like(file_data($Outfile),
	     '/ <\/suuid>\n<\/suuids>\n$/s',
	     "Logfile is not corrupted after SIGPIPE");
	ok(unlink($Outfile), "Delete [Outfile]");
}

# src(): Generate various types of output, avoid repeating it over and over 
# again.
#
# The first argument is a text label defining the type of message, and 
# variables are delivered via %Var.
#
# Always returns text output, no errors returned. The only error is an unknown 
# $label, in which case it aborts.
sub src {
	my ($label, %Var) = @_;

	if ($label eq "empty") {
		return "";
	}
	if ($label eq "macaddr-invalid-digit") {
		return "$Var{cmd}: MAC address contains illegal characters,"
		       . " can only contain hex digits\n";
	}
	if ($label eq "macaddr-rfc-fail") {
		return "$Var{cmd}: MAC address doesn't follow RFC 4122,"
		       . " multicast bit not set\n";
	}
	if ($label eq "macaddr-wrong-length") {
		return "$Var{cmd}: Wrong MAC address length,"
		       . " must be exactly 12 hex digits\n";
	}

	BAIL_OUT("src(): $label: Unknown label");
}

# invalid_macaddr_in_rcfile(): Test various variants of invalid MAC address in 
# the rc file.
sub invalid_macaddr_in_rcfile {
	my ($Outdir, $Outfile, $mac, $errmsg, $desc) = @_;

	create_file("rc-macaddr", "macaddr =$mac\n");
	testcmd("../$CMDB --rcfile rc-macaddr -l $Outdir",
	        "",
	        src($errmsg, ('cmd' => "../$CMDB")),
	        1,
	        $desc);
	ok(!-f $Outfile, "Log file wasn't created");
}

sub read_long_text_from_stdin {
	my ($CMD, $Outdir, $Outfile) = @_;
	my ($fp, $i);
	my $tmpfile = "tmp-longtext";

	ok(open($fp, ">$tmpfile"), "Open $tmpfile for writing") or return;
	for $i (1..20000) {
		print($fp "$i\n");
	}
	ok(close($fp), "Close $tmpfile");
	likecmd("$CMD -l $Outdir -c - <$tmpfile",
	        "/^$v1_templ\n\$/s",
	        '/^$/',
	        0,
	        "Read long text from stdin, 108894 chars");
	ok(unlink($tmpfile), "Delete $tmpfile");
	like(file_data($Outfile),
	     s_top(s_suuid('txt' => '1\\\\n2\\\\n3\\\\n[\d\\\\n]+'
	                            . '19998\\\\n19999\\\\n20000',
	                   'tty' => '')),
	     "Monster entry was added to the log file");
	ok(unlink($Outfile), "Delete [Outfile]");
}

sub s_top {
	my $xml = shift;
	my @Ret = ();

	push(@Ret,
	     '/^',
	     $xml_header,
	     $xml,
	     '<\/suuids>\n',
	     '$/s');

	return join('', @Ret);
}

sub s_suuid {
	my %d = @_;
	my @Ret = ();

	defined($d{suuid_t}) || ($d{suuid_t} = $date_templ);
	defined($d{suuid_u}) || ($d{suuid_u} = $v1_templ);
	defined($d{tag})     || ($d{tag}     = '');
	defined($d{txt})     || ($d{txt}     = '');
	defined($d{host})    || ($d{host}    = $cdata);
	defined($d{cwd})     || ($d{cwd}     = $cdata);
	defined($d{user})    || ($d{user}    = $cdata);
	defined($d{tty})     || ($d{tty}     = $cdata);
	defined($d{sess})    || ($d{sess}    = '');

	push(@Ret,
	     "<suuid t=\"$d{suuid_t}\" u=\"$d{suuid_u}\">",
	     ' ',
	     length($d{tag}) ? s_suuid_tag($d{tag}) : '',
	     length($d{txt}) ? "<txt>$d{txt}</txt> " : '',
	     "<host>$d{host}<\\/host>",
	     ' ',
	     "<cwd>$d{cwd}<\\/cwd>",
	     ' ',
	     "<user>$d{user}<\\/user>",
	     ' ',
	     length($d{tty}) ? "<tty>$d{tty}<\\/tty> " : "",
	     length($d{sess}) ? s_suuid_sess($d{sess}) : '',
	     "<\\/suuid>\\n");

	return join('', @Ret);
}

sub s_suuid_tag {
	my $txt = shift;
	$txt =~ s/,+$//;
	$txt .= ',';
	my @arr = split(/,/, $txt);
	my $retval = '';
	for (@arr) {
		$retval .= "<tag>$_</tag> ";
	}

	return $retval;
}

sub s_suuid_sess {
	my $txt = shift;
	my @arr = ();
	$txt =~ s{
	            ([^/]+?)?
	            /
	            ($v1_templ)
	            ,
	        }{
	            my ($desc, $uuid) = ($1, $2);
	            defined($desc) || ($desc = '');
	            push(@arr,
	                 join('',
	                      '<sess',
	                      length($desc) ? " desc=\"$desc\"" : '',
	                      '>',
	                      $uuid,
	                      '</sess>'
	                 )
	            );
	            '';
	        }egx;
	length($txt) && return undef;
	$txt =~ s/,+$//;
	$txt .= ',';
	my $retval = '';
	for (@arr) {
		$retval .= "$_ ";
	}

	return $retval;
}

sub unique_macs {
	my $file = shift;
	my %mac = ();
	if (open(my $fp, '<', $file)) {
		while (<$fp>) {
			m{
			    ^<suuid
			    \s
			    t="[^"]+"
			    \s
			    u="$Lh{8}-$Lh{4}-1$Lh{3}-$Lh{4}-($Lh{12})"
			    .*
			}x && ($mac{$1} = 1);
		}
		close($fp);
	} else {
		diag("$file: Cannot open file for read: $!\n");
		return '';
	}

	return scalar(keys %mac);
}

sub testcmd {
	my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
	defined($descriptions{$Desc})
	&& BAIL_OUT("testcmd(): '$Desc' description is used twice");
	$descriptions{$Desc} = 1;
	my $stderr_cmd = '';
	my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
	my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
	$Txt =~ s/$Outdir/[Outdir]/g;
	my $TMP_STDERR = "$CMDB-stderr.tmp";
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
	defined($descriptions{$Desc})
	&& BAIL_OUT("likecmd(): '$Desc' description is used twice");
	$descriptions{$Desc} = 1;
	my $stderr_cmd = '';
	my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
	my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
	$Txt =~ s/$Outdir/[Outdir]/g;
	my $TMP_STDERR = "$CMDB-stderr.tmp";
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
	defined($File) || return '';
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

Contains tests for the $CMDB(1) program.

Options:

  -a, --all
    Run all tests, also TODOs.
  -h, --help
    Show this help.
  -q, --quiet
    Be more quiet. Can be repeated to increase silence.
  -t, --todo
    Run only the TODO tests.
  --valgrind
    Run all tests under Valgrind to check for memory leaks and other 
    problems.
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
