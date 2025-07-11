#!/usr/bin/env perl

#==============================================================================
# wi.t
# File ID: 66726d84-2fdd-11e5-9bf9-000df06acc56
#
# Test suite for wi(1).
#
# Character set: UTF-8
# ©opyleft 2015– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of file for 
# legal stuff.
#==============================================================================

use strict;
use warnings;

BEGIN {
    # push(@INC, "$ENV{'HOME'}/bin/STDlibdirDTS");
    use Test::More qw{no_plan};
    # use_ok() goes here
}

use Getopt::Long;

local $| = 1;

our $CMD = '../wi --test-simul';
our $CMD_ONLY = '../wi';

our %Opt = (

    'all' => 0,
    'help' => 0,
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
    'todo|t' => \$Opt{'todo'},
    'verbose|v+' => \$Opt{'verbose'},
    'version' => \$Opt{'version'},

) || die("$progname: Option error. Use -h for help.\n");

$Opt{'help'} && usage(0);
if ($Opt{'version'}) {
    print_version();
    exit(0);
}

exit(main());

sub main {
    my $Retval = 0;
    $ENV{'PATH'} = "..:$ENV{'PATH'}";

    diag(sprintf('========== Executing %s v%s ==========',
                 $progname, $VERSION));

    if ($Opt{'todo'} && !$Opt{'all'}) {
        goto todo_section;
    }

    diag('Testing -h (--help) option...');
    likecmd("$CMD -h",
            '/  Show this help\./',
            '/^$/',
            0,
            'Option -h prints help screen');

    diag('Testing --version option...');
    likecmd("$CMD --version",
            '/^\S+ \d+\.\d+\.\d+(\+git)?\n/',
            '/^$/',
            0,
            'Option --version returns version number');

    diag('Read from stdin...');
    testcmd("$CMD </dev/null",
            '',
            "wi: No search strings found\n",
            1,
            'Read from /dev/null with no arguments');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%ec3a1814-2feb-11e5-a5f7-bd3e22fb7899%'"
            . " OR s::varchar LIKE '%53ed5c0a-cbf4-4878-91ae-9dc97431793d%')"
            . " TO STDOUT;\n",
            <<END,
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
        0,
        'Search for UUIDs found in stdin');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD -a",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%ec3a1814-2feb-11e5-a5f7-bd3e22fb7899%'"
            . " AND s::varchar LIKE '%53ed5c0a-cbf4-4878-91ae-9dc97431793d%')"
            . " TO STDOUT;\n",
            <<END,
f = '-a'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
        0,
        'Search for UUIDs found in stdin, use -a (AND)');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD -a -i",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar"
            . " ILIKE '%ec3a1814-2feb-11e5-a5f7-bd3e22fb7899%'"
            . " AND s::varchar ILIKE '%53ed5c0a-cbf4-4878-91ae-9dc97431793d%')"
            . " TO STDOUT;\n",
            <<END,
f = '-a'
f = '-i'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
            0,
            'Search for UUIDs found in stdin,'
            . ' use -a (AND) and -i (ignore case)');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD -i -I",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%ec3a1814-2feb-11e5-a5f7-bd3e22fb7899%'"
            . " OR s::varchar LIKE '%53ed5c0a-cbf4-4878-91ae-9dc97431793d%')"
            . " TO STDOUT;\n",
            <<END,
f = '-i'
f = '-I'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
        0,
        'Search for UUIDs found in stdin, -i is overridden by extra -I');

    diag('Search for command line arguments...');
    testcmd("$CMD abc",
            "COPY (SELECT s FROM uuids WHERE s::varchar LIKE '%abc%')"
            . " TO STDOUT;\n",
            "f = 'abc'\n",
            0,
            'Single argument specified');

    testcmd("$CMD abc def",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' OR s::varchar LIKE '%def%')"
            . " TO STDOUT;\n",
            <<END,
f = 'abc'
f = 'def'
END
            0,
            'Two arguments specified, use OR as default');

    testcmd("$CMD abc def 'with space'",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' OR s::varchar LIKE '%def%'"
            . " OR s::varchar LIKE '%with space%') TO STDOUT;\n",
            <<END,
f = 'abc'
f = 'def'
f = 'with space'
END
            0,
            'Three args, the last one contains space');

    testcmd("$CMD abc def ' with space '",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' OR s::varchar LIKE '%def%'"
            . " OR s::varchar LIKE '% with space %') TO STDOUT;\n",
            <<END,
f = 'abc'
f = 'def'
f = ' with space '
END
            0,
            'Three args, one with surrounding space');

    diag("Test -a (AND) and -o (OR)...");
    testcmd("$CMD abc -o def",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' OR s::varchar LIKE '%def%')"
            . " TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-o'
f = 'def'
END
            0,
            'Use -o, but that\'s default anyway');

    testcmd("$CMD -a abc def",
            "COPY (SELECT s FROM uuids WHERE s::varchar LIKE '%abc%'"
            . " AND s::varchar LIKE '%def%') TO STDOUT;\n",
            <<END,
f = '-a'
f = 'abc'
f = 'def'
END
            0,
            '-a specified first, use AND from now on');

    testcmd("$CMD abc -a def",
            "COPY (SELECT s FROM uuids WHERE s::varchar LIKE '%abc%'"
            . " AND s::varchar LIKE '%def%') TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-a'
f = 'def'
END
        0,
        'Use AND between args');

    testcmd("$CMD abc -a def -o ghi",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' AND s::varchar LIKE '%def%'"
            . " OR s::varchar LIKE '%ghi%') TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-a'
f = 'def'
f = '-o'
f = 'ghi'
END
            0,
            '-a and -o between args');

    testcmd("$CMD abc -o -a def -a -o ghi",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar LIKE '%abc%' AND s::varchar LIKE '%def%'"
            . " OR s::varchar LIKE '%ghi%') TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-o'
f = '-a'
f = 'def'
f = '-a'
f = '-o'
f = 'ghi'
END
            0,
            '-a followed by -o and vice versa');

    testcmd("$CMD abc def -a ghi jkl mno -o pqr",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' OR"
            . " s::varchar LIKE '%def%' AND"
            . " s::varchar LIKE '%ghi%' AND"
            . " s::varchar LIKE '%jkl%' AND"
            . " s::varchar LIKE '%mno%' OR"
            . " s::varchar LIKE '%pqr%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'abc'
f = 'def'
f = '-a'
f = 'ghi'
f = 'jkl'
f = 'mno'
f = '-o'
f = 'pqr'
END
            0,
            'Several args with no -a or -o between');

    diag("Test -i (ignore case)...");
    testcmd("$CMD abc -i def",
            "COPY (SELECT s FROM uuids WHERE s::varchar LIKE '%abc%'"
            . " OR s::varchar ILIKE '%def%') TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-i'
f = 'def'
END
            0,
            '-i sets the following arg to case insensitive');

    testcmd("$CMD abc -i def g h i",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' OR"
            . " s::varchar ILIKE '%def%' OR"
            . " s::varchar ILIKE '%g%' OR"
            . " s::varchar ILIKE '%h%' OR"
            . " s::varchar ILIKE '%i%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-i'
f = 'def'
f = 'g'
f = 'h'
f = 'i'
END
        0,
        '-i works with all following args');

    testcmd("$CMD abc -a def -i ghi -o jkl",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' AND"
            . " s::varchar LIKE '%def%' AND"
            . " s::varchar ILIKE '%ghi%' OR"
            . " s::varchar ILIKE '%jkl%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-a'
f = 'def'
f = '-i'
f = 'ghi'
f = '-o'
f = 'jkl'
END
            0,
            'Use -i with -a and -o');

    diag("Test -I (Be case sensitive about following patterns)...");
    testcmd("$CMD -I abc",
            "COPY (SELECT s FROM uuids WHERE s::varchar LIKE '%abc%')"
            . " TO STDOUT;\n",
            <<END,
f = '-I'
f = 'abc'
END
            0,
            '-I as first option makes no difference,'
            . ' case sensitive search is the default');

    testcmd("$CMD -i abc -I def",
            "COPY (SELECT s FROM uuids"
            . " WHERE s::varchar ILIKE '%abc%' OR s::varchar LIKE '%def%')"
            . " TO STDOUT;\n",
            <<END,
f = '-i'
f = 'abc'
f = '-I'
f = 'def'
END
            0,
            'First arg all-case, second arg case sensitive');

    testcmd("$CMD abc -I def -a ghi -i JkL mno -o -i pqr -I stu",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' OR"
            . " s::varchar LIKE '%def%' AND"
            . " s::varchar LIKE '%ghi%' AND"
            . " s::varchar ILIKE '%JkL%' AND"
            . " s::varchar ILIKE '%mno%' OR"
            . " s::varchar ILIKE '%pqr%' OR"
            . " s::varchar LIKE '%stu%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'abc'
f = '-I'
f = 'def'
f = '-a'
f = 'ghi'
f = '-i'
f = 'JkL'
f = 'mno'
f = '-o'
f = '-i'
f = 'pqr'
f = '-I'
f = 'stu'
END
            0,
            'Several args with -I, -i -a and -o');

    testcmd("$CMD ABC -a -i def -I -i -I ghi -o JKL",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%ABC%' AND"
            . " s::varchar ILIKE '%def%' AND"
            . " s::varchar LIKE '%ghi%' OR"
            . " s::varchar LIKE '%JKL%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'ABC'
f = '-a'
f = '-i'
f = 'def'
f = '-I'
f = '-i'
f = '-I'
f = 'ghi'
f = '-o'
f = 'JKL'
END
            0,
            'More -I, -i, -a and -o combinations, some succeeding -i and -I');

    diag('Test --sql option...');
    testcmd("$CMD --sql abc -i def g h i",
            '',
            "wi: Cannot mix --sql and --test-simul\n",
            1,
            'Can\'t mix --sql and --test-simul');

    testcmd("$CMD_ONLY --sql abc -i def g h i",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' OR"
            . " s::varchar ILIKE '%def%' OR"
            . " s::varchar ILIKE '%g%' OR"
            . " s::varchar ILIKE '%h%' OR"
            . " s::varchar ILIKE '%i%'"
            . ") TO STDOUT;\n",
            '',
            0,
            '--sql sends generated SQL to stdout');

    diag('Test -u...');
    testcmd("$CMD -u 0886454e-3021-11e5-a047-fefdb24f8e10",
            "COPY (SELECT s FROM uuids"
            . " WHERE u = '0886454e-3021-11e5-a047-fefdb24f8e10')"
            . " TO STDOUT;\n",
            <<END,
f = '-u'
f = '0886454e-3021-11e5-a047-fefdb24f8e10'
END
            0,
            'Go into UUID mode with -u');

    testcmd("$CMD -u 0886454e-3021-11e5-a047-fefdb24f8e10"
            . " abc -i def -a -I ghi",
            "COPY (SELECT s FROM uuids WHERE"
            . " u = '0886454e-3021-11e5-a047-fefdb24f8e10' OR"
            . " s::varchar LIKE '%abc%' OR"
            . " s::varchar ILIKE '%def%' AND"
            . " s::varchar LIKE '%ghi%'"
            . ") TO STDOUT;\n",
            <<END,
f = '-u'
f = '0886454e-3021-11e5-a047-fefdb24f8e10'
f = 'abc'
f = '-i'
f = 'def'
f = '-a'
f = '-I'
f = 'ghi'
END
            0,
            '-u with additional args which are not uuids');

    diag('-u and stdin...');
    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD -u",
            "COPY (SELECT s FROM uuids"
            . " WHERE u = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'"
            . " OR u = '53ed5c0a-cbf4-4878-91ae-9dc97431793d') TO STDOUT;\n",
            <<END,
f = '-u'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
            0,
            'A single -u returns only the original suuid entry'
            . ' for all UUIDs from stdin');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793daaa | $CMD -a -u",
            "COPY (SELECT s FROM uuids WHERE"
            . " u = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'"
            . " AND u = '53ed5c0a-cbf4-4878-91ae-9dc97431793d') TO STDOUT;\n",
            <<END,
f = '-a'
f = '-u'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
            0,
            '-a and -u, read from stdin');

    testcmd("echo jepp"
            . "ec3a1814-2feb-11e5-a5f7-bd3e22fb7899"
            . "23452345"
            . "53ed5c0a-cbf4-4878-91ae-9dc97431793d"
            . "aaa | $CMD -a abc -i def -u",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%abc%' AND"
            . " s::varchar ILIKE '%def%' AND"
            . " u = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899' AND"
            . " u = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'"
            . ") TO STDOUT;\n",
            <<END,
f = '-a'
f = 'abc'
f = '-i'
f = 'def'
f = '-u'
f = 'ec3a1814-2feb-11e5-a5f7-bd3e22fb7899'
f = '53ed5c0a-cbf4-4878-91ae-9dc97431793d'
END
        0,
        '-a, -i and -u, read from stdin');

    testcmd("$CMD c0de4e76-302a-11e5-907c-fefdb24f8e10 abc -u"
            . " c2421e3c-302a-11e5-b410-fefdb24f8e10 -U a"
            . " c3c56df4-302a-11e5-ac4e-fefdb24f8e10",
            "COPY (SELECT s FROM uuids WHERE"
            . " s::varchar LIKE '%c0de4e76-302a-11e5-907c-fefdb24f8e10%' OR"
            . " s::varchar LIKE '%abc%' OR"
            . " u = 'c2421e3c-302a-11e5-b410-fefdb24f8e10' OR"
            . " s::varchar LIKE '%a%' OR"
            . " s::varchar LIKE '%c3c56df4-302a-11e5-ac4e-fefdb24f8e10%'"
            . ") TO STDOUT;\n",
            <<END,
f = 'c0de4e76-302a-11e5-907c-fefdb24f8e10'
f = 'abc'
f = '-u'
f = 'c2421e3c-302a-11e5-b410-fefdb24f8e10'
f = '-U'
f = 'a'
f = 'c3c56df4-302a-11e5-ac4e-fefdb24f8e10'
END
            0,
            '-u and -U with uuids and non-uuid args too');

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
}

sub testcmd {
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    defined($descriptions{$Desc})
    && BAIL_OUT("testcmd(): '$Desc' description is used twice");
    $descriptions{$Desc} = 1;
    my $stderr_cmd = '';
    my $Txt = join('',
        "\"$Cmd\"",
        defined($Desc)
            ? " - $Desc"
            : ''
    );
    my $TMP_STDERR = 'wi-stderr.tmp';
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
}

sub likecmd {
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    defined($descriptions{$Desc})
    && BAIL_OUT("likecmd(): '$Desc' description is used twice");
    $descriptions{$Desc} = 1;
    my $stderr_cmd = '';
    my $Txt = join('',
        "\"$Cmd\"",
        defined($Desc)
            ? " - $Desc"
            : ''
    );
    my $TMP_STDERR = 'wi-stderr.tmp';
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
}

sub file_data {
    # Return file content as a string
    my $File = shift;
    my $Txt;
    if (open(my $fp, '<', $File)) {
        local $/ = undef;
        $Txt = <$fp>;
        close($fp);
        return $Txt;
    } else {
        return;
    }
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

Usage: $progname [options] [file [files [...]]]

Contains tests for the wi(1) program.

Options:

  -a, --all
    Run all tests, also TODOs.
  -h, --help
    Show this help.
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

    if ($Opt{'verbose'} >= $verbose_level) {
        print(STDERR "$progname: $Txt\n");
    }
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

# vim: set fenc=UTF-8 ft=perl fdm=marker ts=4 sw=4 sts=4 et fo+=w :
