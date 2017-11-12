#!/usr/bin/env perl

#=======================================================================
# tests/suuid.t
# File ID: 7a006334-f988-11dd-8845-000475e441b9
#
# Test suite for suuid(1).
#
# Character set: UTF-8
# ©opyleft 2008– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of 
# file for legal stuff.
#=======================================================================

use strict;
use warnings;

BEGIN {
    use Test::More qw{no_plan};
    # use_ok() goes here
}

use bigint;
use Getopt::Long;

local $| = 1;

our $CMD_BASENAME = "suuid";
our $CMD = "../$CMD_BASENAME";
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
our $VERSION = '0.4.0';

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
my $v1rand_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\[37bf]$Lh\{10}";
my $date_templ = '20[0-9][0-9]-[0-1][0-9]-[0-3][0-9]T[0-2][0-9]:[0-5][0-9]:[0-6][0-9]\.\d+Z';
my $xml_header = join("",
    '<\?xml version="1\.0" encoding="UTF-8"\?>\n',
    '<!DOCTYPE suuids SYSTEM "dtd\/suuids\.dtd">\n',
    '<suuids>\n',
);

my $Outdir = "tmp-suuid-t-$$-" . substr(rand, 2, 8);

if ($Opt{'valgrind'}) {
    $CMD = "valgrind -q --leak-check=full --show-leak-kinds=all -- " .
           "../$CMD_BASENAME";
}

exit(main());

sub main {
    # {{{
    my $Retval = 0;

    diag(sprintf('========== Executing %s v%s ==========',
                 $progname, $VERSION));
    diag(`$CMD --version`);
    diag('');

    if ($Opt{'todo'} && !$Opt{'all'}) {
        goto todo_section;
    }

=pod

    testcmd("$CMD command", # {{{
        <<'END',
[expected stdout]
END
        '',
        0,
        'description',
    );

    # }}}

=cut

    diag('Testing -h (--help) option...');
    likecmd("$CMD -h", # {{{
        '/  Show this help/i',
        '/^$/',
        0,
        'Option -h prints help screen',
    );

    # }}}
    diag('Testing -v (--verbose) option...');
    likecmd("$CMD -hv", # {{{
        '/^\n\S+ \d+\.\d+\.\S+ \(\d\d\d\d-\d\d-\d\d\)\n/s',
        '/^$/',
        0,
        'Option -v with -h returns version number and help screen',
    );

    # }}}
    diag('Testing --version option...');
    likecmd("$CMD --version", # {{{
        '/^\S+ \d+\.\d+\.\S+ \(\d\d\d\d-\d\d-\d\d\)\n/',
        '/^$/',
        0,
        'Option --version returns version number',
    );

    # }}}
    likecmd("$CMD --version --quiet", # {{{
            '/^\d+\.\d+\.\d+\S+$/',
            '/^$/',
            0,
            '--version with --quiet shows only the version number');

    # }}}
    diag('--license option');
    likecmd("$CMD --license", # {{{
            '/GNU General Public License' .
            '.*' .
            'either version 2 of the License/s',
            '/^$/',
            0,
            'Option --license displays the program license');

    # }}}
    diag('Unknown option');
    likecmd("$CMD --gurgle", # {{{
            '/^$/',
            "/\\.\\.\\/$CMD_BASENAME: Option error\\n" .
            "\\nType \"\\.\\.\\/$CMD_BASENAME --help\" for help screen\\. " .
            "Returning with value 1\\.\\n/s",
            1,
            'Unknown option specified');

    # }}}

    $ENV{'HOME'} = "/dontexist/$Outdir";
    delete $ENV{'SESS_UUID'};
    delete $ENV{'SUUID_EDITOR'};
    delete $ENV{'SUUID_HOSTNAME'};
    delete $ENV{'SUUID_LOGDIR'};

    test_test_functions();
    test_suuid_executable();

    todo_section:
    ;

    if ($Opt{'all'} || $Opt{'todo'}) {
        diag('Running TODO tests...'); # {{{

        TODO: {

            local $TODO = '';
            # Insert TODO tests here.

        }
        # TODO tests }}}
    }

    diag('Testing finished.');
    return $Retval;
    # }}}
} # main()

sub test_test_functions {
    # {{{
    diag("Testing s_top()..."); # {{{
    like(
        (
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n" .
            "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">\n" .
            "<suuids>\n" .
            "</suuids>\n"
        ),
        s_top(''),
        "s_top('') returns empty file"
    );

    # }}}
    diag("Testing s_suuid_tag()..."); # {{{
    is(s_suuid_tag(''), '', "s_suuid_tag('') returns ''");
    is(s_suuid_tag('test'), '<tag>test</tag> ', "s_suuid_tag('test')");
    is(s_suuid_tag('test,lixom'), '<tag>test</tag> <tag>lixom</tag> ', "s_suuid_tag('test,lixom')");
    is(s_suuid_tag('test,lixom,på en måte'), '<tag>test</tag> <tag>lixom</tag> <tag>på en måte</tag> ', "s_suuid_tag('test,lixom,på en måte')");
    is(s_suuid_tag('test,lixom, space '), '<tag>test</tag> <tag>lixom</tag> <tag> space </tag> ', "s_suuid_tag('test,lixom, space ')");

    # }}}
    diag("Testing s_suuid_sess()..."); # {{{
    is(s_suuid_sess(''), '', "s_suuid_sess('') returns ''");

    for my $l_desc ('deschere', '') {
        for my $l_slash ('/', '') {
            for my $l_uuid ('ff529c20-4522-11e2-8c4a-0016d364066c', '') {
                for my $l_comma (',', '') {
                    my $fail = 0;
                    my $str = "$l_desc$l_slash$l_uuid$l_comma";
                    my $humstr = sprintf(
                        "s_suuid_sess() %s desc, %s slash, %s uuid, %s comma",
                        length($l_desc)  ? "with" : "without",
                        length($l_slash) ? "with" : "without",
                        length($l_uuid)  ? "with" : "without",
                        length($l_comma) ? "with" : "without",
                    );
                    length($l_slash) || ($fail = 1);
                    length($l_comma) || ($fail = 1);
                    length($l_uuid)  || ($fail = 1);
                    if ($fail) {
                        if (length($str)) {
                            is(s_suuid_sess($str), undef, $humstr);
                        }
                    } else {
                        like(s_suuid_sess($str), '/^<sess( desc="deschere")?>ff529c20-4522-11e2-8c4a-0016d364066c<\/sess> $/', $humstr)
                    }
                }
            }
        }
    }

    is(
        s_suuid_sess('ff529c20-4522-11e2-8c4a-0016d364066c'),
        undef,
        "s_suuid_sess() without comma and slash returns undef",
    );
    is(
        s_suuid_sess('ff529c20-4522-11e2-8c4a-0016d364066c,'),
        undef,
        "s_suuid_sess() with comma but missing slash returns undef",
    );
    is(
        s_suuid_sess('xterm/ff529c20-4522-11e2-8c4a-0016d364066c'),
        undef,
        "s_suuid_sess() with desc, but missing comma returns undef",
    );
    is(
        s_suuid_sess('/ff529c20-4522-11e2-8c4a-0016d364066c,'),
        '<sess>ff529c20-4522-11e2-8c4a-0016d364066c</sess> ',
        "s_suuid_sess() without desc, but with slash and comma",
    );
    is(
        s_suuid_sess('xterm/ff529c20-4522-11e2-8c4a-0016d364066c,'),
        '<sess desc="xterm">ff529c20-4522-11e2-8c4a-0016d364066c</sess> ',
        "s_suuid_sess() with desc and comma",
    );
    is(
        s_suuid_sess(
            'xfce/bbd272a0-44e0-11e2-bcdb-0016d364066c,' .
            'xterm/c1986406-44e0-11e2-af23-0016d364066c,' .
            'screen/e7f897b0-44e0-11e2-b5a0-0016d364066c,'
        ),
        (
            '<sess desc="xfce">bbd272a0-44e0-11e2-bcdb-0016d364066c</sess> ' .
            '<sess desc="xterm">c1986406-44e0-11e2-af23-0016d364066c</sess> ' .
            '<sess desc="screen">e7f897b0-44e0-11e2-b5a0-0016d364066c</sess> ',
        ),
        's_suuid_sess() receives string with three with desc',
    );
    is(
        s_suuid_sess('/ee5db39a-43f7-11e2-a975-0016d364066c,/da700fd8-43eb-11e2-889a-0016d364066c,'),
        '<sess>ee5db39a-43f7-11e2-a975-0016d364066c</sess> <sess>da700fd8-43eb-11e2-889a-0016d364066c</sess> ',
        "s_suuid_sess() receives two without desc",
    );

    # }}}
    return;
    # }}}
} # test_test_functions()

sub test_suuid_executable {
    # {{{
    chomp(my $osname = `uname`);

    if (-e $Outdir) {
        die("$progname: $Outdir: WTF?? Directory element already exists.");
    }
    unless (mkdir($Outdir)) {
        die("$progname: $Outdir: Cannot mkdir(): $!\n");
    }

    diag("No options (except --logfile)...");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "No options (except -l) sends UUID to stdout",
    );

    # }}}
    my $Outfile = glob("$Outdir/*.xml");
    defined($Outfile) || ($Outfile = '');
    like($Outfile, "/^$Outdir\\/\\S+\.xml\$/", "Filename of logfile OK");
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(),
        ),
        "Log contents OK after exec with no options",
    );

    # }}}
    testcmd("$CMD -l $Outdir >/dev/null", # {{{
        '',
        '',
        0,
        "Redirect stdout to /dev/null",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
             s_suuid() .
             s_suuid(),
        ),
        "Entries are added, not replacing",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    read_long_text_from_stdin($CMD, $Outdir, $Outfile);
    testcmd("$CMD --rcfile rcfile-inv-uuidcmd -l $Outdir", # {{{
        '',
        "../suuid: UUID generation failed\n",
        1,
        "uuidcmd does not generate valid UUID",
    );

    # }}}
    my $host_outfile = glob("$Outdir/*.xml");
    defined($host_outfile) || ($host_outfile = '');
    like(file_data($host_outfile), # {{{
        s_top(''),
        "suuid file is empty",
    );

    # }}}
    diag("Read the SUUID_LOGDIR environment variable...");
    likecmd("SUUID_LOGDIR=$Outdir $CMD", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Read SUUID_LOGDIR environment variable",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(),
        ),
        "The SUUID_LOGDIR environment variable was read",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    diag("Read the SUUID_HOSTNAME environment variable...");
    likecmd("SUUID_HOSTNAME=urk13579kru $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Read SUUID_HOSTNAME environment variable",
    );

    # }}}
    like(file_data("$Outdir/urk13579kru.xml"), # {{{
        s_top(
            s_suuid(
                'host' => 'urk13579kru',
            ),
        ),
        "The SUUID_HOSTNAME environment variable was read",
    );

    # }}}
    ok(unlink("$Outdir/urk13579kru.xml"), "Delete [Outdir]/urk13579kru.xml");
    diag("Testing -m (--random-mac) option...");
    likecmd("$CMD -m -l $Outdir", # {{{
        "/^$v1rand_templ\\n\$/s",
        '/^$/s',
        0,
        "--random-mac option works",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(),
        ),
        "Log contents OK after --random-mac",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    diag("Testing --raw option...");
    likecmd("$CMD --raw -c '<dingle><dangle>bær</dangle></dingle>' -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "--raw option works",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('txt' => ' <dingle><dangle>bær<\/dangle><\/dingle> '),
        ),
        "Log contents after --raw is OK",
    );
    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    diag("Testing --rcfile option...");
    likecmd("$CMD --rcfile rcfile1 -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "--rcfile option works",
    );

    # }}}
    like(file_data("$Outdir/altrc1.xml"), # {{{
        s_top(
            s_suuid('host' => 'altrc1'),
        ),
        "hostname from rcfile1 is stored in the file",
    );

    # }}}
    ok(unlink("$Outdir/altrc1.xml"), "Delete [Outdir]/altrc1.xml");
    ok(!-e 'nosuchrc', "'nosuchrc' doesn't exist");
    likecmd("$CMD --rcfile nosuchrc -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "--rcfile with non-existing file",
    );

    # }}}
    ok(unlink($host_outfile), "Delete [host_outfile]");
    my $mac = "1b460a166a4d";
    create_file("rc-macaddr", "macaddr=$mac\n");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with valid macaddr",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
        "MAC address from the rc file is in the log file",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    create_file("rc-macaddr", "macaddr=" . uc($mac) . "\n");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with valid macaddr, upper case",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
        "MAC address from the rc file is in the log file",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    create_file("rc-macaddr", "   macaddr     =       $mac      \n");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with valid macaddr and lots of spaces",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
        "MAC address from the rc file is in the log file",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    create_file("rc-macaddr", "macaddr=$mac");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with valid macaddr and no \\n at EOF",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(s_suuid('suuid_u' => "........-....-....-....-$mac")),
        "MAC address from the rc file is in the log file",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    create_file("rc-macaddr", "macaddr =\n");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with macaddr keyword but no value, that's ok",
    );

    # }}}
    like(file_data($Outfile), s_top(s_suuid()), "One entry created");
    ok(unlink($Outfile), "Delete [Outfile]");
    create_file("rc-macaddr", "macaddr =   \n");
    likecmd("$CMD --rcfile rc-macaddr -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "rc file with macaddr keyword and no value but spaces, that's ok",
    );

    # }}}
    like(file_data($Outfile), s_top(s_suuid()), "One entry created");
    ok(unlink($Outfile), "Delete [Outfile]");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "10460a166a4d", 'macaddr-rfc-fail',
                              "rc file with invalid macaddr, " .
                              "doesn't follow the RFC");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "1b460a166a4", 'macaddr-wrong-length',
                              "rc file with invalid macaddr, " .
                              "one digit too short");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "${mac}a", 'macaddr-wrong-length',
                              "rc file with invalid macaddr, " .
                              "one digit too long");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "${mac}y", 'macaddr-wrong-length',
                              "rc file with invalid macaddr, " .
                              "extra invalid character");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "iiiiiiiiiiii", 'macaddr-invalid-digit',
                              "rc file with invalid macaddr, " .
                              "correct length, invalid hex digits");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "invalid", 'macaddr-invalid-digit',
                              "rc file with invalid macaddr, " .
                              "not a hex number at all");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "'$mac'", 'macaddr-invalid-digit',
                              "rc file with valid macaddr, " .
                              "but it's inside ''");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "\"$mac\"", 'macaddr-invalid-digit',
                              "rc file with valid macaddr, " .
                              "but it's inside \"\"");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              "= \"$mac\"", 'macaddr-invalid-digit',
                              "rc file with valid macaddr, " .
                              "but extra equal sign");
    invalid_macaddr_in_rcfile($Outdir, $Outfile,
                              " = \"$mac\"", 'macaddr-invalid-digit',
                              "rc file with valid macaddr, " .
                              "but extra equal sign with space");
    ok(unlink("rc-macaddr"), "Delete rc-macaddr");
    diag("Testing -t (--tag) option...");
    likecmd("$CMD -t snaddertag -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "-t (--tag) option",
    );

    # }}}
    testcmd("$CMD -t schn\xfcffelhund -l $Outdir", # {{{
        "",
        "../suuid: Tags have to be in UTF-8\n",
        1,
        "Refuse non-UTF-8 tags",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'snaddertag'),
        ),
        "Log contents OK after tag",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -t abc,def,ghi -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Three tags in one argument separated with commas",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'abc,def,ghi'),
        ),
        "Log contents OK after comma-separated tags",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -t abc,abc,abc -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Three comma-separated tags, duplicates",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'abc'),
        ),
        "Duplicate comma-separated tags was removed",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -t abc --tag abc -t abc -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Three -t with duplicated tags",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'abc'),
        ),
        "Duplicate tags was removed",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -t abc -t '' -t def -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "-t with empty tag",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'abc,def'),
        ),
        "Empty tag was not stored",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD --tag '  abc  ' -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Tag with surrounding spaces",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'abc'),
        ),
        "Remove surrounding spaces from tag",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD --tag '  with space  ' -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Tag with surrounding spaces and space in the middle",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'with space'),
        ),
        "Remove surrounding spaces from tag, whitespace in tag kept",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD --tag '&<>' -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Tag with &, < and >",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => '&amp;&lt;&gt;'),
        ),
        "&<> in tag was converted",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    test_suuid_comment($Outdir, $Outfile);
    diag("Testing -n (--count) option...");
    likecmd("$CMD -n 5 -c \"Great test\" -t testeri -l $Outdir", # {{{
        "/^($v1_templ\n){5}\$/s",
        '/^$/',
        0,
        "-n (--count) option with comment and tag",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'tag' => 'testeri',
                'txt' => 'Great test',
            ) x 5,
        ),
        "Log contents OK after count, comment and tag",
    );

    # }}}
    if ($Opt{'all'}) {
        # Disable the testing of non-unique MAC addresses by default. It has 
        # never worked reliably and varies from computer to computer. It's 
        # random whether uuid(1) or uuidgen(1) gets it right. As an example, on 
        # this machine none of them works.
        diag("Check for randomness in the MAC address field...");
        cmp_ok(unique_macs($Outfile), '==', 1, 'MAC adresses does not change');
    }
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -m -n 5 -l $Outdir", # {{{
        "/^($v1rand_templ\n){5}\$/s",
        '/^$/',
        0,
        "-n (--count) option with -m (--random-mac)",
    );

    # }}}
    cmp_ok(unique_macs($Outfile), '==', 5, 'MAC adresses are random');
    diag("Testing -w (--whereto) option...");
    likecmd("$CMD -w o -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "Output goes to stdout",
    );

    # }}}
    likecmd("$CMD --whereto e -l $Outdir", # {{{
        '/^$/s',
        "/^$v1_templ\\n\$/s",
        0,
        "Output goes to stderr",
    );

    # }}}
    likecmd("$CMD -w eo -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        "/^$v1_templ\\n\$/s",
        0,
        "Output goes to stdout and stderr",
    );

    # }}}
    likecmd("$CMD -w a -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        "/^$v1_templ\\n\$/s",
        0,
        "Option -wa sends output to stdout and stderr",
    );

    # }}}
    testcmd("$CMD -w n -l $Outdir", # {{{
        '',
        '',
        0,
        "Output goes nowhere",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    test_suuid_environment($Outdir, $Outfile);
    diag("Test behaviour when unable to write to the log file...");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "Create logfile with one entry",
    );

    # }}}
    my @stat_array = stat($Outfile);
    ok(chmod(0444, $Outfile), "Make [Outfile] read-only");
    likecmd("$CMD -l $Outdir", # {{{
        '/^$/s',
        '/^\.\.\/suuid: .*?\.xml: Could not open file for read\+write: Permission denied\n' .
        '$/s',
        1,
        "Unable to write to the log file",
    );
    if (defined($stat_array[2])) {
        ok(chmod($stat_array[2], $Outfile), "Make [Outfile] writable again");
    }

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    diag("Test what happens when the end of the log file is messed up");
    ok(create_file($Outfile, ""), "Create empty log file");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^$/s',
        0,
        "Write to empty log file",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(s_suuid()),
        "The empty file was initialised",
    );

    # }}}
    ok(create_file($Outfile, "Destroyed file\n"), "Create destroyed log file");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^\.\.\/suuid: tmp-suuid-t-\d+-\d+\/.*?\.xml: Unknown end line, ' .
            'adding to end of file\n$/s',
        0,
        "Write to log file with destroyed EOF",
    );

    # }}}
    like(file_data($Outfile), # {{{
        '/^Destroyed file\n' . s_suuid() . '<\/suuids>\n$/s',
        "New entry was added to end of file",
    );

    # }}}
    ok(create_file($Outfile, "a"), "Create log file with one char");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^\.\.\/suuid: tmp-suuid-t-\d+-\d+\/.*?\.xml: Unknown end line, ' .
            'adding to end of file\n$/s',
        0,
        "Write to log file containing one char",
    );

    # }}}
    like(file_data($Outfile), # {{{
        '/^a' . s_suuid() . '<\/suuids>\n$/s',
        "New entry was added to EOF after that one character",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    if ($osname eq "OpenBSD") {
        diag("NOTICE: SIGPIPE test hangs on OpenBSD, skipping test");
    } else {
        test_suuid_signal($Outfile);
    }
    ok(rmdir($Outdir), "rmdir [Outdir]");
    return;
    # }}}
} # test_suuid_executable()

sub test_suuid_comment {
    # {{{
    my ($Outdir, $Outfile) = @_;
    diag("Testing -c (--comment) option...");
    likecmd("$CMD -c \"Great test\" -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "-c (--comment) option",
    );

    # }}}
    testcmd("$CMD -c \"F\xf8kka \xf8pp\" -l $Outdir", # {{{
        "",
        "../suuid: Comment contains illegal characters or is not valid UTF-8\n",
        1,
        "Refuse non-UTF-8 text to --comment option",
    );

    # }}}
    testcmd("$CMD -c \"Ctrl-d: \x04\" -l $Outdir", # {{{
        "",
        "../suuid: Comment contains illegal characters or is not valid UTF-8\n",
        1,
        "Reject Ctrl-d in comment",
    );

    # }}}
    likecmd("echo \"Great test\" | $CMD -c - -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Read comment from stdin",
    );

    # }}}
    testcmd("echo \"F\xf8kka \xf8pp\" | $CMD -c - -l $Outdir", # {{{
        "",
        "../suuid: Comment contains illegal characters or is not valid UTF-8\n",
        1,
        "Reject non-UTF-8 comment from stdin",
    );

    # }}}
    testcmd("echo \"Ctrl-d: \x04\" | $CMD -c - -l $Outdir", # {{{
        "",
        "../suuid: Comment contains illegal characters or is not valid UTF-8\n",
        1,
        "Reject Ctrl-d in comment from stdin",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('txt' => 'Great test') .
            s_suuid('txt' => 'Great test', 'tty' => ''),
        ),
        "Log contents OK after comment",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("$CMD -c - -l $Outdir </dev/null", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Read empty comment from stdin",
    );

    # }}}
    likecmd("$CMD --comment '' -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Enter empty comment with --comment",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tty' => '') .
            s_suuid(),
        ),
        "Log contents OK after empty comments",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("echo " . "a" x 82 . " | $CMD -t aa -c - -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Read line with 82 bytes from stdin and use tag",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid('tag' => 'aa', 'tty' => '', 'txt' => 'a' x 82),
        ),
        "Log contents OK after 82 character line",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    return;
    # }}}
} # test_suuid_comment()

sub test_suuid_environment {
    # {{{
    my ($Outdir, $Outfile) = @_;
    my $bck_home;

    diag("Test logging of \$SESS_UUID environment variable...");
    likecmd("SESS_UUID=27538da4-fc68-11dd-996d-000475e441b9 $CMD -t yess -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "Use SESS_UUID environment variable",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'tag' => 'yess',
                'sess' => '/27538da4-fc68-11dd-996d-000475e441b9,',
            ),
        ),
        "\$SESS_UUID envariable is logged",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c, $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with 'ssh-agent/'-prefix and comma at the end",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,',
            ),
        ),
        "<sess> contains desc attribute",
    );

    # }}}
    likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,dingle©/4c66b03a-43f4-11e2-b70d-0016d364066c, $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with 'ssh-agent' and 'dingle©'",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,',
            ) .
            s_suuid(
                'sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,' .
                          'dingle©/4c66b03a-43f4-11e2-b70d-0016d364066c,',
            ),
        ),
        "<sess> contains both desc attributes, one with ©",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with 'ssh-agent', missing comma",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => 'ssh-agent/da700fd8-43eb-11e2-889a-0016d364066c,',
            ),
        ),
        "<sess> is correct without comma",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=/da700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID missing name and comma, but has slash",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,',
            ),
        ),
        "<sess> is OK without name and comma",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=ee5db39a-43f7-11e2-a975-0016d364066c,/da700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs, latter missing name and comma, but has slash",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/ee5db39a-43f7-11e2-a975-0016d364066c,/da700fd8-43eb-11e2-889a-0016d364066c,',
            ),
        ),
        "Second <sess> is correct without comma",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=ee5db39a-43f7-11e2-a975-0016d364066cda700fd8-43eb-11e2-889a-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs smashed together",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/ee5db39a-43f7-11e2-a975-0016d364066c,/da700fd8-43eb-11e2-889a-0016d364066c,',
            ),
        ),
        "Still separates them into two UUIDs",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066cabcee5db39a-43f7-11e2-a975-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs, only separated by 'abc'",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,abc/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "Separated the two UUIDs, keeps 'abc'",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=da700fd8-43eb-11e2-889a-0016d364066cabc/ee5db39a-43f7-11e2-a975-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs, separated by 'abc/'",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/da700fd8-43eb-11e2-889a-0016d364066c,abc/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "The two UUIDs are separated, 'abc/' is kept",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=" . "da700fd8-43eb-11e2-889a-0016d364066c" . "ee5db39a-43f7-11e2-a975-0016d364066c" . "abc" . " $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs together, 'abc' at EOS",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' =>
                    '/da700fd8-43eb-11e2-889a-0016d364066c,' .
                    '/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "The two UUIDs are found, 'abc' at EOS is discarded",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=" . "da700fd8-43eb-11e2-889a-0016d364066c" . ",,ee5db39a-43f7-11e2-a975-0016d364066c" . " $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs separated by two commas",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' =>
                    '/da700fd8-43eb-11e2-889a-0016d364066c,' .
                    '/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "The two UUIDs are found, ignoring useless commas",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID='" . ",,,,," . "abc/da700fd8-43eb-11e2-889a-0016d364066c" . ",,,,,,,,,," . "def/ee5db39a-43f7-11e2-a975-0016d364066c" . ",,%..¤¤¤%¤,,,'" . " $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID, lots of commas+punctuation and two UUIDS with descs",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' =>
                    'abc/da700fd8-43eb-11e2-889a-0016d364066c,' .
                    'def/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "The two UUIDs with desc are found, ignore garbage",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=" . "da700fd8-43eb-11e2-889a-0016d364066c" . "def/ee5db39a-43f7-11e2-a975-0016d364066c" . " $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID with two UUIDs separated by 'def/'",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' =>
                    '/da700fd8-43eb-11e2-889a-0016d364066c,' .
                    'def/ee5db39a-43f7-11e2-a975-0016d364066c,',
            ),
        ),
        "The two UUIDs are found, 'def/' is kept with second UUID",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=5f650dac-4404-11e2-8e0e-0016d364066c5f660e28-4404-11e2-808e-0016d364066c5f66ef14-4404-11e2-8b45-0016d364066c5f67e266-4404-11e2-a6f8-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID contains four UUIDs, no separators",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/5f650dac-4404-11e2-8e0e-0016d364066c,' .
                          '/5f660e28-4404-11e2-808e-0016d364066c,' .
                          '/5f66ef14-4404-11e2-8b45-0016d364066c,' .
                          '/5f67e266-4404-11e2-a6f8-0016d364066c,',
            ),
        ),
        "All four UUIDs are separated",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=5f650dac-4404-11e2-8e0e-0016d364066cabc5f660e28-4404-11e2-808e-0016d364066c5f66ef14-4404-11e2-8b45-0016d364066c,nmap/5f67e266-4404-11e2-a6f8-0016d364066c $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID contains four UUIDs, 'abc' separates the first two, last one has desc",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => '/5f650dac-4404-11e2-8e0e-0016d364066c,' .
                          'abc/5f660e28-4404-11e2-808e-0016d364066c,' .
                          '/5f66ef14-4404-11e2-8b45-0016d364066c,' .
                          'nmap/5f67e266-4404-11e2-a6f8-0016d364066c,',
            ),
        ),
        "All four UUIDs separated, 'abc' and 'nmap' kept",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    likecmd("SESS_UUID=ssh-agent/fea9315a-43d6-11e2-8294-0016d364066c,logging/febfd0f4-43d6-11e2-9117-0016d364066c,screen/0e144c10-43d7-11e2-9833-0016d364066c,ti/152d8f16-4409-11e2-be17-0016d364066c, $CMD -l $Outdir", # {{{
        "/^$v1_templ\n\$/s",
        '/^$/',
        0,
        "SESS_UUID is OK and contains four UUIDs, all with desc",
    );

    # }}}
    like(file_data($Outfile), # {{{
        s_top(
            s_suuid(
                'sess' => 'ssh-agent/fea9315a-43d6-11e2-8294-0016d364066c,' .
                          'logging/febfd0f4-43d6-11e2-9117-0016d364066c,' .
                          'screen/0e144c10-43d7-11e2-9833-0016d364066c,' .
                          'ti/152d8f16-4409-11e2-be17-0016d364066c,',
            ),
        ),
        "The four UUIDs are separated, all four descs kept",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    diag("Check behaviour without various environment variables");
    $bck_home = $ENV{'HOME'};
    delete $ENV{'HOME'};
    is($ENV{'HOME'},           undef, "HOME is undefined");
    is($ENV{'SESS_UUID'},      undef, "SESS_UUID is undefined");
    is($ENV{'SUUID_EDITOR'},   undef, "SUUID_EDITOR is undefined");
    is($ENV{'SUUID_HOSTNAME'}, undef, "SUUID_HOSTNAME is undefined");
    is($ENV{'SUUID_LOGDIR'},   undef, "SUUID_LOGDIR is undefined");
    likecmd("$CMD -l $Outdir", # {{{
        "/^$v1_templ\\n\$/s",
        '/^\.\.\/suuid: HOME environment variable not defined, cannot ' .
            'determine name of rcfile\n$/s',
        0,
        "No HOME defined, but -l is set to [Outdir]",
    );

    # }}}
    testcmd("$CMD", # {{{
        '',
        "../suuid: HOME environment variable not defined, cannot determine " .
            "name of rcfile\n" .
            "../suuid: \$SUUID_LOGDIR and \$HOME environment variables are " .
            "not defined, cannot create logdir path\n",
        1,
        "Now it doesn't even have -l/--logdir",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    $ENV{'HOME'} = $bck_home;
    return;
    # }}}
} # test_suuid_environment()

sub test_suuid_signal {
    # {{{
    my $Outfile = shift;
    my $sigpipe_stderr = "$CMD_BASENAME-stderr.tmp";

    diag("Receive SIGPIPE signal");
    likecmd("$CMD -l $Outdir -n 1000 -wo 2>\"$sigpipe_stderr\" | true",
        '/^$/s',
        '/^\.\.\/suuid: Termination signal \(Broken pipe\) received, ' .
            'aborting\n' .
            '\.\.\/suuid: Generated only \d+ of 1000 UUIDs\n$/s',
        0,
        "Receive SIGPIPE",
    );
    like(file_data($Outfile),
        '/ <\/suuid>\n<\/suuids>\n$/s',
        "Logfile is not corrupted after SIGPIPE",
    );
    ok(unlink($Outfile), "Delete [Outfile]");
    # }}}
} # test_suuid_signal()

# src(): Generate various types of output, avoid repeating it over and over 
# again.
#
# The first argument is a text label defining the type of message, and 
# variables are delivered via %Var.
#
# Always returns text output, no errors returned. The only error is an unknown 
# $label, in which case it aborts.
sub src {
        # {{{
        my ($label, %Var) = @_;

        if ($label eq "empty") {
            return "";
        }
        if ($label eq "macaddr-invalid-digit") {
                return "$Var{cmd}: MAC address contains illegal characters, " .
                       "can only contain hex digits\n";
        }
        if ($label eq "macaddr-rfc-fail") {
                return "$Var{cmd}: MAC address doesn't follow RFC 4122, the " .
                       "second hex digit must be one of \"37bf\"\n";
        }
        if ($label eq "macaddr-wrong-length") {
                return "$Var{cmd}: Wrong MAC address length, must be " .
                       "exactly 12 hex digits\n";
        }

        BAIL_OUT("src(): $label: Unknown label");
        # }}}
}

# invalid_macaddr_in_rcfile(): Test various variants of invalid MAC address in 
# the rc file.
sub invalid_macaddr_in_rcfile {
    # {{{
    my ($Outdir, $Outfile, $mac, $errmsg, $desc) = @_;

    create_file("rc-macaddr", "macaddr =$mac\n");
    testcmd("../$CMD_BASENAME --rcfile rc-macaddr -l $Outdir",
        "",
        src($errmsg, ('cmd' => "../$CMD_BASENAME")),
        1,
        $desc,
    );
    ok(!-f $Outfile, "Log file wasn't created");
    # }}}
}

sub read_long_text_from_stdin {
    # {{{
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
        "Read long text from stdin, 108894 chars",
    );
    ok(unlink($tmpfile), "Delete $tmpfile");
    like(file_data($Outfile), # {{{
        s_top(
             s_suuid(
                'txt' => '1\\\\n2\\\\n3\\\\n[\d\\\\n]+19998\\\\n19999\\\\n20000',
                'tty' => '',
             ),
        ),
        "Monster entry was added to the log file",
    );

    # }}}
    ok(unlink($Outfile), "Delete [Outfile]");
    # }}}
}

sub s_top {
    # {{{
    my $xml = shift;
    my @Ret = ();

    push(@Ret,
        '/^',
        $xml_header,
        $xml,
        '<\/suuids>\n',
        '$/s',
    );
    return(join('', @Ret));
    # }}}
} # s_top()

sub s_suuid {
    # {{{
    my %d = @_;
    my @Ret = ();

    defined($d{suuid_t}) || ($d{suuid_t} = $date_templ);
    defined($d{suuid_u}) || ($d{suuid_u} = $v1_templ);
    defined($d{tag}) || ($d{tag} = '');
    defined($d{txt}) || ($d{txt} = '');
    defined($d{host}) || ($d{host} = $cdata);
    defined($d{cwd}) || ($d{cwd} = $cdata);
    defined($d{user}) || ($d{user} = $cdata);
    defined($d{tty}) || ($d{tty} = $cdata);
    defined($d{sess}) || ($d{sess} = '');

    push(@Ret,
        "<suuid t=\"$d{suuid_t}\" u=\"$d{suuid_u}\">",
        ' ',
        length($d{tag})
            ? s_suuid_tag($d{tag})
            : '',
        length($d{txt})
            ? "<txt>$d{txt}</txt> "
            : '',
        "<host>$d{host}<\\/host>",
        ' ',
        "<cwd>$d{cwd}<\\/cwd>",
        ' ',
        "<user>$d{user}<\\/user>",
        ' ',
        length($d{tty}) ? "<tty>$d{tty}<\\/tty> " : "",
        length($d{sess})
            ? s_suuid_sess($d{sess})
            : '',
        "<\\/suuid>\\n",
    );
    return(join('', @Ret));
    # }}}
} # s_suuid()

sub s_suuid_tag {
    # {{{
    my $txt = shift;
    $txt =~ s/,+$//;
    $txt .= ',';
    my @arr = split(/,/, $txt);
    my $retval = '';
    for (@arr) {
        $retval .= "<tag>$_</tag> ";
    }
    return($retval);
    # }}}
} # s_suuid_tag()

sub s_suuid_sess {
    # {{{
    my $txt = shift;
    my @arr = ();
    $txt =~ s{
        ([^/]+?)?
        /
        ($v1_templ)
        ,
    } {
        my ($desc, $uuid) = ($1, $2);
        defined($desc) || ($desc = '');
        push(@arr,
            join('',
                '<sess',
                length($desc)
                    ? " desc=\"$desc\""
                    : '',
                '>',
                $uuid,
                '</sess>',
            ),
        );
        '';
    }egx;
    length($txt) && return(undef);
    $txt =~ s/,+$//;
    $txt .= ',';
    my $retval = '';
    for (@arr) {
        $retval .= "$_ ";
    }
    return($retval);
    # }}}
} # s_suuid_sess()

sub unique_macs {
    # {{{
    my $file = shift;
    my %mac = ();
    if (open(my $fp, '<', $file)) {
        while (<$fp>) {
            /^<suuid t="[^"]+" u="$Lh{8}-$Lh{4}-1$Lh{3}-$Lh{4}-($Lh{12})".*/ && ($mac{$1} = 1);
        }
        close($fp);
    } else {
        diag("$file: Cannot open file for read: $!\n");
        return('');
    }
    return(scalar(keys %mac));
    # }}}
} # unique_macs()

sub testcmd {
    # {{{
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    defined($descriptions{$Desc}) &&
        BAIL_OUT("testcmd(): '$Desc' description is used twice");
    $descriptions{$Desc} = 1;
    my $stderr_cmd = '';
    my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
    my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
    $Txt =~ s/$Outdir/[Outdir]/g;
    my $TMP_STDERR = "$CMD_BASENAME-stderr.tmp";
    my $retval = 1;

    if (defined($Exp_stderr)) {
        $stderr_cmd = " 2>$TMP_STDERR";
    }
    $retval &= is(`$Cmd$stderr_cmd`, $Exp_stdout, "$Txt (stdout)");
    my $ret_val = $?;
    if (defined($Exp_stderr)) {
        $retval &= is(file_data($TMP_STDERR), $Exp_stderr, "$Txt (stderr)");
        unlink($TMP_STDERR);
    } else {
        diag("Warning: stderr not defined for '$Txt'");
    }
    $retval &= is($ret_val >> 8, $Exp_retval, "$Txt (retval)");

    return $retval;
    # }}}
} # testcmd()

sub likecmd {
    # {{{
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    defined($descriptions{$Desc}) &&
        BAIL_OUT("likecmd(): '$Desc' description is used twice");
    $descriptions{$Desc} = 1;
    my $stderr_cmd = '';
    my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
    my $Txt = join('', $cmd_outp_str, defined($Desc) ? $Desc : '');
    $Txt =~ s/$Outdir/[Outdir]/g;
    my $TMP_STDERR = "$CMD_BASENAME-stderr.tmp";
    my $retval = 1;

    if (defined($Exp_stderr)) {
        $stderr_cmd = " 2>$TMP_STDERR";
    }
    $retval &= like(`$Cmd$stderr_cmd`, $Exp_stdout, "$Txt (stdout)");
    my $ret_val = $?;
    if (defined($Exp_stderr)) {
        $retval &= like(file_data($TMP_STDERR), $Exp_stderr, "$Txt (stderr)");
        unlink($TMP_STDERR);
    } else {
        diag("Warning: stderr not defined for '$Txt'");
    }
    $retval &= is($ret_val >> 8, $Exp_retval, "$Txt (retval)");

    return $retval;
    # }}}
} # likecmd()

sub file_data {
    # Return file content as a string {{{
    my $File = shift;
    defined($File) || return('');
    my $Txt;

    open(my $fp, '<', $File) or return undef;
    local $/ = undef;
    $Txt = <$fp>;
    close($fp);
    return $Txt;
    # }}}
} # file_data()

sub create_file {
    # Create new file and fill it with data {{{
    my ($file, $text) = @_;
    open(my $fp, ">$file") or return 0;
    print($fp $text);
    close($fp);
    return (file_data($file) eq $text) ? 1 : 0;
    # }}}
} # create_file()

sub print_version {
    # Print program version {{{
    print("$progname $VERSION\n");
    return;
    # }}}
} # print_version()

sub usage {
    # Send the help message to stdout {{{
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
  --valgrind
    Run all tests under Valgrind to check for memory leaks and other 
    problems.
  -v, --verbose
    Increase level of verbosity. Can be repeated.
  --version
    Print version information.

END
    exit($Retval);
    # }}}
} # usage()

sub msg {
    # Print a status message to stderr based on verbosity level {{{
    my ($verbose_level, $Txt) = @_;

    $verbose_level > $Opt{'verbose'} && return;
    print(STDERR "$progname: $Txt\n");
    return;
    # }}}
} # msg()

__END__

# This program is free software; you can redistribute it and/or modify 
# it under the terms of the GNU General Public License as published by 
# the Free Software Foundation; either version 2 of the License, or (at 
# your option) any later version.
#
# This program is distributed in the hope that it will be useful, but 
# WITHOUT ANY WARRANTY; without even the implied warranty of 
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License 
# along with this program.
# If not, see L<http://www.gnu.org/licenses/>.

# vim: set fenc=UTF-8 ft=perl fdm=marker ts=4 sw=4 sts=4 et fo+=w :
