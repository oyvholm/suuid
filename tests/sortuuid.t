#!/usr/bin/env perl

#==============================================================================
# sortuuid.t
# File ID: f83a2d54-fa1c-11dd-b34c-000475e441b9
#
# Test suite for sortuuid(1).
#
# Character set: UTF-8
# ©opyleft 2009– Øyvind A. Holm <sunny@sunbase.org>
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

our $CMD = '../sortuuid';

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

    diag('Testing -v (--verbose) option...');
    likecmd("$CMD -hv",
            '/^\n\S+ \d+\.\d+\.\d+(\+git)?\n/s',
            '/^$/',
            0,
            'Option -v with -h returns version number and help screen');

    diag('Testing --version option...');
    likecmd("$CMD --version",
            '/^\S+ \d+\.\d+\.\d+(\+git)?\n/',
            '/^$/',
            0,
            'Option --version returns version number');

    testcmd("$CMD", # Sort list of UUIDs
            <<'END',
922052ac-750e-11dd-9335-000475e441b9 2008-08-28T14:35:33.2566700Z
af06e4fe-c421-11de-8250-0ff74f20b9f9 2009-10-29T00:26:21.7123070Z
f212ae26-4ea6-11df-8aca-90e6ba3022ac 2010-04-23T07:07:57.7252390Z
174def3e-afc8-11df-8d8f-952aaa41f63e 2010-08-24T21:39:36.3707710Z
66653d04-444f-11e0-89e8-00023faf1383 2011-03-01T22:01:03.2130820Z
5b1d7e70-6123-11e0-8cac-e7080c6f55a8 2011-04-07T14:28:50.1524080Z
52db258a-e0a5-11e0-bfda-933003f10efd 2011-09-16T20:49:08.5434250Z
c13b1172-1e64-11e2-bc9b-fefdb24f8e10 2012-10-25T05:28:06.1972850Z
2ce3b0ce-4551-11e2-a915-0016d364066c 2012-12-13T18:16:12.2323150Z
0a42158e-ad38-11e2-8523-0016d364066c 2013-04-24T23:38:17.6413070Z
61468cba-afa5-11e2-8ed0-0016d364066c 2013-04-28T01:46:01.1014330Z
e66385be-2a9e-11e3-ba32-f0def1836eb5 2013-10-01T13:39:30.9728190Z
994780bc-2527-11e4-8634-c80aa9e67bbd 2014-08-16T09:27:53.3509820Z
4fb5ebec-3755-11e4-a6a2-fefdb24f8e10 2014-09-08T12:40:27.7027820Z
fa606a5c-e67a-11e4-af17-000df06acc56 2015-04-19T10:00:58.8978780Z
3ed8edca-1237-11e5-a4b0-000df06acc56 2015-06-14T01:46:59.1573450Z
888d36ee-2b12-11e5-b816-000df06acc56 2015-07-15T16:57:10.5029870Z
END
        '',
        0,
        'Sort list of UUIDs',
        <<END,
c13b1172-1e64-11e2-bc9b-fefdb24f8e10 2012-10-25T05:28:06.1972850Z
5b1d7e70-6123-11e0-8cac-e7080c6f55a8 2011-04-07T14:28:50.1524080Z
174def3e-afc8-11df-8d8f-952aaa41f63e 2010-08-24T21:39:36.3707710Z
888d36ee-2b12-11e5-b816-000df06acc56 2015-07-15T16:57:10.5029870Z
0a42158e-ad38-11e2-8523-0016d364066c 2013-04-24T23:38:17.6413070Z
994780bc-2527-11e4-8634-c80aa9e67bbd 2014-08-16T09:27:53.3509820Z
52db258a-e0a5-11e0-bfda-933003f10efd 2011-09-16T20:49:08.5434250Z
e66385be-2a9e-11e3-ba32-f0def1836eb5 2013-10-01T13:39:30.9728190Z
fa606a5c-e67a-11e4-af17-000df06acc56 2015-04-19T10:00:58.8978780Z
66653d04-444f-11e0-89e8-00023faf1383 2011-03-01T22:01:03.2130820Z
61468cba-afa5-11e2-8ed0-0016d364066c 2013-04-28T01:46:01.1014330Z
3ed8edca-1237-11e5-a4b0-000df06acc56 2015-06-14T01:46:59.1573450Z
f212ae26-4ea6-11df-8aca-90e6ba3022ac 2010-04-23T07:07:57.7252390Z
af06e4fe-c421-11de-8250-0ff74f20b9f9 2009-10-29T00:26:21.7123070Z
4fb5ebec-3755-11e4-a6a2-fefdb24f8e10 2014-09-08T12:40:27.7027820Z
2ce3b0ce-4551-11e2-a915-0016d364066c 2012-12-13T18:16:12.2323150Z
922052ac-750e-11dd-9335-000475e441b9 2008-08-28T14:35:33.2566700Z
END
    );

    testcmd("$CMD", # UUID is not in first column
            <<'END',
08-28T14:35:33.2566700Z 922052ac-750e-11dd-9335-000475e441b9
kjbshd  jsd2009-10-29T00:26:21.7123070Z af06e4fe-c421-11de-8250-0ff74f20b9f9
f212ae26-4ea6-11df-8aca-90e6ba3022ac
2010-08-24T dfs 21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
-03-01T22:01:03.2130820Z 66653d04-444f-11e0-89e8-00023faf1383
2011-09-1dfsv 6T20:49:08.5434250Z 52db258a-e0a5-11e0-bfda-933003f10efd
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
æløå-OC2012-12-13T18:16:12.2323150Z 2ce3b0ce-4551-11e2-a915-0016d364066c
2013-04-24T23:38:17.6413070Z 0a42158e-ad38-11e2-8523-0016d364066c
🕱🤘 61468cba-afa5-11e2-8ed0-0016d364066c
e66385be-2a9e-11e3-ba32-f0def1836eb5
4fb5ebec-3755-11e4-a6a2-fefdb24f8e10
fa606a5c-e67a-11e4-af17-000df06acc56
sd as vf vfsdv ;3ed8edca-1237-11e5-a4b0-000df06acc56
888d36ee-2b12-11e5-b816-000df06acc56
END
            '',
            0,
            'UUID is not in first column',
            <<END,
æløå-OC2012-12-13T18:16:12.2323150Z 2ce3b0ce-4551-11e2-a915-0016d364066c
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
2011-0→524080Z 5b1d7e70-6123-11e0-8cac-e780c6f55a8
08-28T14:35:33.2566700Z 922052ac-750e-11dd-9335-000475e441b9
2011-09-1dfsv 6T20:49:08.5434250Z 52db258a-e0a5-11e0-bfda-933003f10efd
2010-08-24T dfs 21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
øøøøøøø 99478bc-2527-11e4-8634-c80aa9e67bbd
888d36ee-2b12-11e5-b816-000df06acc56
f212ae26-4ea6-11df-8aca-90e6ba3022ac
2013-04-24T23:38:17.6413070Z 0a42158e-ad38-11e2-8523-0016d364066c
sd as vf vfsdv ;3ed8edca-1237-11e5-a4b0-000df06acc56
🕱🤘 61468cba-afa5-11e2-8ed0-0016d364066c
kjbshd  jsd2009-10-29T00:26:21.7123070Z af06e4fe-c421-11de-8250-0ff74f20b9f9
-03-01T22:01:03.2130820Z 66653d04-444f-11e0-89e8-00023faf1383
4fb5ebec-3755-11e4-a6a2-fefdb24f8e10
e66385be-2a9e-11e3-ba32-f0def1836eb5
fa606a5c-e67a-11e4-af17-000df06acc56
END
    );

    testcmd("$CMD", # Lines without UUIDs are removed from output by default
            <<'END',
2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
END
            '',
            0,
            'Lines without UUIDs are removed from output by default',
            <<END,
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
.
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8
fsdvsfd
D
erf

2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
139783193
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
END
    );

    testcmd("$CMD", # UUIDs v4 are skipped
            <<'END',
2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
END
            '',
            0,
            'UUIDs v4 are skipped',
            <<END,
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8
afc4d374-f7a8-4b99-be36-e322af41d122
450afbd6-a5c5-4dca-90b6-f83ebb3088fc
2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
89d95926-16e8-468c-91d1-725fb304bda9
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
END
    );

    # -a includes lines without UUID, listed unsorted at the end
    testcmd("$CMD -a",
            <<'END',
2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8 🌌
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
.
fsdvsfd
🐮
D
erf

🚁 139783193
END
            '',
            0,
            '-a includes lines without UUID, listed unsorted at the end',
            <<END,
2012-10-25T05:28:06.1972850Z c13b1172-1e64-11e2-bc9b-fefdb24f8e10
.
2011-04-07T14:28:50.1524080Z 5b1d7e70-6123-11e0-8cac-e7080c6f55a8 🌌
fsdvsfd
🐮
D
erf

2010-08-24T21:39:36.3707710Z 174def3e-afc8-11df-8d8f-952aaa41f63e
🚁 139783193
2015-07-15T16:57:10.5029870Z 888d36ee-2b12-11e5-b816-000df06acc56
END
    );

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
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc, $Input) = @_;
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
    my $TMP_STDERR = 'sortuuid-stderr.tmp';
    my $retval = 1;

    if (defined($Exp_stderr)) {
        $stderr_cmd = " 2>$TMP_STDERR";
    }
    if (defined($Input)) {
        my $tmp_input = 'sortuuid-stdin.tmp';
        open(my $tmpinpfp, ">$tmp_input") or
            return "$tmp_input: Cannot create file: $!";
        print($tmpinpfp $Input);
        close($tmpinpfp);
        is(`$Cmd$stderr_cmd <$tmp_input`, "$Exp_stdout", "$Txt (stdout)");
        unlink($tmp_input);
    } else {
        is(`$Cmd$stderr_cmd`, "$Exp_stdout", "$Txt (stdout)");
    }
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
    my $TMP_STDERR = 'sortuuid-stderr.tmp';
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

Contains tests for the sortuuid(1) program.

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
