#!/usr/bin/env perl

#=======================================================================
# sess.t
# File ID: b9f9e134-fb83-11dd-b202-000475e441b9
#
# Test suite for sess(1).
#
# Character set: UTF-8
# ©opyleft 2009– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of 
# file for legal stuff.
#=======================================================================

use strict;
use warnings;

BEGIN {
    use Test::More qw{no_plan};
    # use_ok() goes here
}

use Getopt::Long;

local $| = 1;

our $CMD_BASENAME = "sess";
our $CMD = "../$CMD_BASENAME";

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
our $VERSION = '0.1.0';

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

exit(main());

sub main {
    # {{{
    my $Retval = 0;
    my $logdir = "tmp-sess-t-logdir";
    my $CMD;
    my $valgrind_str;

    $ENV{'SESS_UUID'} = '';
    $ENV{'PATH'} = "../src:$ENV{'PATH'}";

    if ($Opt{'valgrind'}) {
        $valgrind_str = " valgrind -q " .
                        "--leak-check=full --show-leak-kinds=all --";
    } else {
        $valgrind_str = "";
    }
    $CMD = "SUUID_LOGDIR=$logdir$valgrind_str ../$CMD_BASENAME";

    diag(sprintf('========== Executing %s v%s ==========',
                 $progname, $VERSION));

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
        '/^\n\S+ \d+\.\d+\.\d+/s',
        '/^$/',
        0,
        'Option -v with -h returns version number and help screen',
    );

    # }}}
    diag('Testing --version option...');
    likecmd("$CMD --version", # {{{
        '/^\S+ \d+\.\d+\.\d+/',
        '/^$/',
        0,
        'Option --version returns version number',
    );

    # }}}
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

    system("rm -rf $logdir");
    ok(!-d $logdir, "$logdir doesn't exist");
    ok(mkdir($logdir), "mkdir $logdir") or die("$progname: $logdir: Cannot create directory, skipping tests");
    testcmd("$CMD", # {{{
        '',
        "sess: No command specified. Use -h for help.\n",
        1,
        'Invoked with no arguments',
    );

    # }}}
    likecmd("$CMD echo yess mainn", # {{{
        "/^yess mainn\\n\$/",
        "/^sess.begin:echo/$v1_templ\\n" .
            "sess.end:echo/$v1_templ -- 00:00:\\d\\d.\\d+, exit code '0'\\.\\n\$/",
        0,
        'Execute "echo yess mainn"',
    );

    # }}}
    my $logfile = glob("$logdir/*");
    like(file_data($logfile), # {{{
        '/^' . $xml_header .
        join(' ',
              "<suuid t=\"$date_templ\" u=\"$v1_templ\">",
                "<txt>",
                  "<sess_begin>",
                    "<cmd>echo yess mainn<\/cmd>",
                  "<\/sess_begin>",
                "<\/txt>",
                "<host>$cdata<\/host>",
                "<cwd>$cdata<\/cwd>",
                "<user>$cdata<\/user>",
                "<tty>$cdata<\/tty>",
              "<\/suuid>",
        ) . '\n' . join(' ',
              "<suuid t=\"$date_templ\" u=\"$v1_templ\">",
                "<txt>",
                  "<sess_end>",
                    "<finished>$v1_templ<\/finished>",
                    "<cmd>echo yess mainn<\/cmd>",
                    "<duration>",
                      "<totsecs>\\d.\\d+<\/totsecs>",
                      "<seconds>\\d.\\d+<\/seconds>",
                    "<\/duration>",
                    "<exit>0<\/exit>",
                  "<\/sess_end>",
                "<\/txt>",
                "<host>$cdata<\/host>",
                "<cwd>$cdata<\/cwd>",
                "<user>$cdata<\/user>",
                "<tty>$cdata<\/tty>",
              "<\/suuid>",
        ) . '\n<\/suuids>\n$/s',
        "$logfile looks OK"
    );

    # }}}

    ok(unlink($logfile), "rm $logfile");
    ok(rmdir($logdir), "rmdir $logdir");
    ok(!-d $logdir, "$logdir is gone");

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

sub testcmd {
    # {{{
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
    my $Txt;

    open(my $fp, '<', $File) or return undef;
    local $/ = undef;
    $Txt = <$fp>;
    close($fp);
    return $Txt;
    # }}}
} # file_data()

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
