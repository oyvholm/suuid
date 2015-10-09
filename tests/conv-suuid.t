#!/usr/bin/perl

#=======================================================================
# conv-suuid.t
# File ID: fd1a6b18-1261-11e5-9f1a-000df06acc56
#
# Test suite for conv-suuid(1).
#
# Character set: UTF-8
# ©opyleft 2015– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of 
# file for legal stuff.
#=======================================================================

use strict;
use warnings;

BEGIN {
    # push(@INC, "$ENV{'HOME'}/bin/STDlibdirDTS");
    use Test::More qw{no_plan};
    # use_ok() goes here
}

use Getopt::Long;

local $| = 1;

our $CMD = '../conv-suuid';

our %Opt = (

    'all' => 0,
    'help' => 0,
    'todo' => 0,
    'verbose' => 0,
    'version' => 0,

);

our $progname = $0;
$progname =~ s/^.*\/(.*?)$/$1/;
our $VERSION = '0.00';

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

my $tmpdb = "tmp-$$-" . substr(rand, 2, 8);

exit(main(%Opt));

sub main {
    # {{{
    my %Opt = @_;
    my $Retval = 0;

    diag(sprintf('========== Executing %s v%s ==========',
        $progname,
        $VERSION));

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
        '/  Show this help\./',
        '/^$/',
        0,
        'Option -h prints help screen',
    );

    # }}}
    diag('Testing -v (--verbose) option...');
    likecmd("$CMD -hv", # {{{
        '/^\n\S+ v\d\.\d\d\n/s',
        '/^$/',
        0,
        'Option --version with -h returns version number and help screen',
    );

    # }}}
    diag('Testing --version option...');
    likecmd("$CMD --version", # {{{
        '/^\S+ v\d\.\d\d\n/',
        '/^$/',
        0,
        'Option --version returns version number',
    );

    # }}}

    ok(chdir("conv-suuid-files"), "chdir conv-suuid-files");
    testcmd("../$CMD test.xml", # {{{
        xml_output('test', '*', ''),
        '',
        0,
        'Read test.xml',
    );

    # }}}
    testcmd("../$CMD -o xml test.xml", # {{{
        xml_output('test', '*', ''),
        '',
        0,
        'Output XML format',
    );

    # }}}
    testcmd("../$CMD --output-format postgres --verbose -vv test.xml", # {{{
        gen_output('test', 'postgres', 'copy-to-uuids-from-stdin'),
        <<END,
conv-suuid: Left in suuid: ''
conv-suuid: tag: 'std'
conv-suuid: Left in suuid: ''
conv-suuid: tag: 'ti|another'
conv-suuid: Left in suuid: ''
conv-suuid: tag: ''
conv-suuid: Left in suuid: ''
conv-suuid: tag: ''
END
        0,
        'Output Postgres format',
    );

    # }}}
    testcmd("../$CMD --pg-table --output-format postgres test.xml", # {{{
        gen_output('test', 'postgres', 'copy-to-uuids-from-stdin pg-table'),
        '',
        0,
        'Output Postgres tables and format',
    );

    # }}}
    diag('Test --pg-index option...');
    testcmd("../$CMD --pg-index -o postgres test.xml", # {{{
        gen_output('test', 'postgres', 'copy-to-uuids-from-stdin pg-index'),
        '',
        0,
        'Output create index',
    );

    # }}}
    testcmd("../$CMD --pg-table --pg-index --output-format postgres test.xml", # {{{
        gen_output('test', 'postgres', 'copy-to-uuids-from-stdin pg-index pg-table'),
        '',
        0,
        'Output Postgres tables and index',
    );

    # }}}
    diag('Testing Postgres database...');
    testcmd("createdb $tmpdb", '', '', 0, "Create test database");
    likecmd("../$CMD --pg-table -o postgres test.xml | psql -X -d $tmpdb", # {{{
        '/^' .
            'CREATE TABLE\n' .
            '(COPY 4\n)?' .
            '$/',
        '/^$/',
        0,
        'Import test data into database',
    );

    # }}}
    testcmd("psql -d $tmpdb -c \"COPY (SELECT * FROM uuids) TO STDOUT;\"", # {{{
        gen_output('test', 'postgres', ''),
        '',
        0,
        "Check contents of database",
    );

    # }}}
    likecmd("../$CMD --pg-table -o postgres test2.xml | psql -X -d $tmpdb", # {{{
        '/^' .
            '(COPY 3\n)?' .
            '$/',
        '/^ERROR:  relation "uuids" already exists\n$/',
        0,
        'Import more data into db, table already exists',
    );

    # }}}
    testcmd("psql -d $tmpdb -c \"COPY (SELECT * FROM uuids) TO STDOUT;\"", # {{{
        gen_output('test', 'postgres', '') .
        <<END,
2015-07-08 13:18:42.52731\tdab29b0c-2573-11e5-ae4f-000df06acc56\t{'c_stpl'}\tbellmann\t/home/sunny\tsunny\t/dev/pts/15\t{'xterm/01829b90-2571-11e5-82ee-000df06acc56','logging/105891b0-2571-11e5-bfe5-000df06acc56','screen/12e3d0ac-2571-11e5-810c-000df06acc56'}\tstpl /tmp/stpl.tmp\t<suuid t="2015-07-08T13:18:42.5273100Z" u="dab29b0c-2573-11e5-ae4f-000df06acc56"> <tag>c_stpl</tag> <txt>stpl /tmp/stpl.tmp</txt> <host>bellmann</host> <cwd>/home/sunny</cwd> <user>sunny</user> <tty>/dev/pts/15</tty> <sess desc="xterm">01829b90-2571-11e5-82ee-000df06acc56</sess> <sess desc="logging">105891b0-2571-11e5-bfe5-000df06acc56</sess> <sess desc="screen">12e3d0ac-2571-11e5-810c-000df06acc56</sess> </suuid>
2015-07-08 13:25:54.51638\tdc2ee818-2574-11e5-b355-000df06acc56\t{'c_v_begin'}\tbellmann\t/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev\tsunny\t/dev/pts/5\t{'xterm/01829b90-2571-11e5-82ee-000df06acc56','logging/105891b0-2571-11e5-bfe5-000df06acc56','screen/12e3d0ac-2571-11e5-810c-000df06acc56'}\t <c_v w="begin"> <cmdline>README.md</cmdline> <file> <name>README.md</name> <fileid>a8487d1c-1c4f-11e5-b5a1-398b4cddfd2b</fileid> <smsum>6b30f51d1b3906db3301c7a3d865f6cc1d6ab801-42ad87640ec84b0d3bf971e9bca17576-11342</smsum> <gitsum>f335a485d4214aab544688cb636e399ea1648526</gitsum> <mdate>2015-06-29T21:49:06Z</mdate> </file> </c_v> \t<suuid t="2015-07-08T13:25:54.5163800Z" u="dc2ee818-2574-11e5-b355-000df06acc56"> <tag>c_v_begin</tag> <txt> <c_v w="begin"> <cmdline>README.md</cmdline> <file> <name>README.md</name> <fileid>a8487d1c-1c4f-11e5-b5a1-398b4cddfd2b</fileid> <smsum>6b30f51d1b3906db3301c7a3d865f6cc1d6ab801-42ad87640ec84b0d3bf971e9bca17576-11342</smsum> <gitsum>f335a485d4214aab544688cb636e399ea1648526</gitsum> <mdate>2015-06-29T21:49:06Z</mdate> </file> </c_v> </txt> <host>bellmann</host> <cwd>/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev</cwd> <user>sunny</user> <tty>/dev/pts/5</tty> <sess desc="xterm">01829b90-2571-11e5-82ee-000df06acc56</sess> <sess desc="logging">105891b0-2571-11e5-bfe5-000df06acc56</sess> <sess desc="screen">12e3d0ac-2571-11e5-810c-000df06acc56</sess> </suuid>
2015-07-08 13:56:23.309865\t1e3aa59a-2579-11e5-b682-000df06acc56\t{'c_v_end'}\tbellmann\t/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev\tsunny\t/dev/pts/5\t{'xterm/01829b90-2571-11e5-82ee-000df06acc56','logging/105891b0-2571-11e5-bfe5-000df06acc56','screen/12e3d0ac-2571-11e5-810c-000df06acc56'}\t <c_v w="end"> <finished>dc2ee818-2574-11e5-b355-000df06acc56</finished> <changed> <file> <name>README.md</name> <fileid>a8487d1c-1c4f-11e5-b5a1-398b4cddfd2b</fileid> <old>6b30f51d1b3906db3301c7a3d865f6cc1d6ab801-42ad87640ec84b0d3bf971e9bca17576-11342</old> <new>3fa0a88448bb13c7176b7081edb022b7cad3832f-d2fcd61a0bcd95f5aeaba5210050e960-12414</new> <oldgitsum>f335a485d4214aab544688cb636e399ea1648526</oldgitsum>  <newgitsum>c2f0209bef68b6f73cbb55f87639d3fb57d2592c</newgitsum> <oldmdate>2015-06-29T21:49:06Z</oldmdate> <newmdate>2015-07-08T13:56:21Z</newmdate> </file> </changed> </c_v> \t<suuid t="2015-07-08T13:56:23.3098650Z" u="1e3aa59a-2579-11e5-b682-000df06acc56"> <tag>c_v_end</tag> <txt> <c_v w="end"> <finished>dc2ee818-2574-11e5-b355-000df06acc56</finished> <changed> <file> <name>README.md</name> <fileid>a8487d1c-1c4f-11e5-b5a1-398b4cddfd2b</fileid> <old>6b30f51d1b3906db3301c7a3d865f6cc1d6ab801-42ad87640ec84b0d3bf971e9bca17576-11342</old> <new>3fa0a88448bb13c7176b7081edb022b7cad3832f-d2fcd61a0bcd95f5aeaba5210050e960-12414</new> <oldgitsum>f335a485d4214aab544688cb636e399ea1648526</oldgitsum>  <newgitsum>c2f0209bef68b6f73cbb55f87639d3fb57d2592c</newgitsum> <oldmdate>2015-06-29T21:49:06Z</oldmdate> <newmdate>2015-07-08T13:56:21Z</newmdate> </file> </changed> </c_v> </txt> <host>bellmann</host> <cwd>/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev</cwd> <user>sunny</user> <tty>/dev/pts/5</tty> <sess desc="xterm">01829b90-2571-11e5-82ee-000df06acc56</sess> <sess desc="logging">105891b0-2571-11e5-bfe5-000df06acc56</sess> <sess desc="screen">12e3d0ac-2571-11e5-810c-000df06acc56</sess> </suuid>
END
        '',
        0,
        "Database has six entries",
    );

    # }}}
    likecmd("dropdb $tmpdb", '/^$/', '/^$/', 0, "Delete test database");

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
    # }}}
} # main()

sub xml_output {
    # {{{
    my ($file, $line, $flags) = @_;
    my $retval = '';
    my @xml_test = (
        join(' ',
            # Entry #1 from test.xml {{{
            "<suuid t=\"2015-06-14T02:34:41.5608070Z\" u=\"e8f90906-123d-11e5-81a8-000df06acc56\">",
                "<tag>std</tag>",
                "<txt>std -l python suuids-to-postgres.py</txt>",
                "<host>bellmann</host>",
                "<cwd>/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/postgres</cwd>",
                "<user>sunny</user>",
                "<tty>/dev/pts/4</tty>",
                "<sess desc=\"xterm\">f923e8fc-11e6-11e5-913a-000df06acc56</sess>",
                "<sess desc=\"logging\">09733f50-11e7-11e5-a1ac-000df06acc56</sess>",
                "<sess>0bb564f0-11e7-11e5-bc0c-000df06acc56</sess>",
            "</suuid>",
            # }}}
        ),
        join(' ',
            # Entry #2 from test.xml {{{
            "<suuid t=\"2015-06-14T02:51:50.4477750Z\" u=\"4e3cba36-1240-11e5-ab4e-000df06acc56\">",
                "<tag>ti</tag>",
                "<tag>another</tag>",
                "<txt>Yo mainn.</txt>",
                "<host>bellmann</host>",
                "<cwd>/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/postgres</cwd>",
                "<user>sunny</user>",
                "<tty>/dev/pts/13</tty>",
                "<sess desc=\"xterm\">f923e8fc-11e6-11e5-913a-000df06acc56</sess>",
                "<sess desc=\"logging\">09733f50-11e7-11e5-a1ac-000df06acc56</sess>",
                "<sess>0bb564f0-11e7-11e5-bc0c-000df06acc56</sess>",
            "</suuid>",
            # }}}
        ),
        join(' ',
            # Entry #3 from test.xml {{{
            "<suuid t=\"2015-06-21T10:49:19.2036620Z\" u=\"2b1e350c-1803-11e5-9c66-000df06acc56\">",
                "<txt>Weird</txt>",
                "<host>bellmann</host>",
                "<cwd>/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/tests</cwd>",
                "<user>sunny</user>",
                "<tty>/dev/pts/15</tty>",
                "<sess desc=\"xterm\">edcbd7d8-16ca-11e5-9739-000df06acc56</sess>",
                "<sess desc=\"logging\">03a706ae-16cb-11e5-becb-000df06acc56</sess>",
                "<sess desc=\"screen\">088f9e56-16cb-11e5-a56c-000df06acc56</sess>",
            "</suuid>",
            # }}}
        ),
        "<suuid t=\"2015-07-14T02:07:50.9817960Z\" u=\"2162ae68-29cd-11e5-aa3e-000df06acc56\"> </suuid>",
    );
    if ($file eq 'test') {
        if ($line eq '*') {
            $retval = join("\n", @xml_test) . "\n";
        } else {
            $retval = $xml_test[$line - 1]; # I, as a stupid human, count from 1
        }
    }

    return($retval);
    # }}}
} # xml_output()

sub gen_output {
    # Generate output similar to what's in the test files {{{
    my ($file, $format, $flags) = @_;
    my $fl_copy_to_uuids = 0;
    my $retval = '';
    if ($flags =~ /pg-table/) {
        $retval .= <<END;
CREATE TABLE uuids (
    t timestamp,
    u uuid,
    tag varchar[],
    host varchar,
    cwd varchar,
    username varchar,
    tty varchar,
    sess varchar[],
    txt varchar,
    s xml
);
END
    }
    if ($flags =~ /copy-to-uuids-from-stdin/) {
        $retval .= "COPY uuids (t, u, tag, host, cwd, username, tty, sess, txt, s) FROM stdin;\n";
        $fl_copy_to_uuids = 1;
    }
    if ($file eq 'test') {
        if ($format eq 'postgres') {
            $retval .= join("\n",
                join("\t",
                    # Postgres import {{{
                    $fl_copy_to_uuids ? "2015-06-14T02:34:41.5608070Z" : "2015-06-14 02:34:41.560807",
                    "e8f90906-123d-11e5-81a8-000df06acc56",
                    "{'std'}",
                    "bellmann",
                    "/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/postgres",
                    "sunny",
                    "/dev/pts/4",
                    "{'xterm/f923e8fc-11e6-11e5-913a-000df06acc56','logging/09733f50-11e7-11e5-a1ac-000df06acc56','0bb564f0-11e7-11e5-bc0c-000df06acc56'}",
                    "std -l python suuids-to-postgres.py",
                    xml_output('test', 1, ''),
                    # }}}
                ),
                join("\t",
                    # Postgres import {{{
                    $fl_copy_to_uuids ? "2015-06-14T02:51:50.4477750Z" : "2015-06-14 02:51:50.447775",
                    "4e3cba36-1240-11e5-ab4e-000df06acc56",
                    "{'ti','another'}",
                    "bellmann",
                    "/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/postgres",
                    "sunny",
                    "/dev/pts/13",
                    "{'xterm/f923e8fc-11e6-11e5-913a-000df06acc56','logging/09733f50-11e7-11e5-a1ac-000df06acc56','0bb564f0-11e7-11e5-bc0c-000df06acc56'}",
                    "Yo mainn.",
                    xml_output('test', 2, ''),
                    # }}}
                ),
                join("\t",
                    # Postgres import {{{
                    $fl_copy_to_uuids ? "2015-06-21T10:49:19.2036620Z" : "2015-06-21 10:49:19.203662",
                    "2b1e350c-1803-11e5-9c66-000df06acc56",
                    "\\N",
                    "bellmann",
                    "/home/sunny/src/git/.er_ikke_i_bellmann/utils.dev/Git/suuid/tests",
                    "sunny",
                    "/dev/pts/15",
                    "{'xterm/edcbd7d8-16ca-11e5-9739-000df06acc56','logging/03a706ae-16cb-11e5-becb-000df06acc56','screen/088f9e56-16cb-11e5-a56c-000df06acc56'}",
                    "Weird",
                    xml_output('test', 3, ''),
                    # }}}
                ),
                join("\t",
                    # Postgres import {{{
                    $fl_copy_to_uuids ? "2015-07-14T02:07:50.9817960Z" : "2015-07-14 02:07:50.981796",
                    "2162ae68-29cd-11e5-aa3e-000df06acc56",
                    "\\N",
                    "\\N",
                    "\\N",
                    "\\N",
                    "\\N",
                    "\\N",
                    "\\N",
                    xml_output('test', 4, ''),
                    # }}}
                ),
                '',
            );
            if ($fl_copy_to_uuids) {
                $retval .= "\\.\n";
            }
            if ($flags =~ /pg-index/) {
                $retval .= <<END;
CREATE INDEX idx_uuids_u ON uuids (u);
CREATE INDEX idx_uuids_t ON uuids (t);
END
            }
        }
    }
    return($retval);
    # }}}
} # gen_output()

sub testcmd {
    # {{{
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    my $stderr_cmd = '';
    my $Txt = join('',
        "\"$Cmd\"",
        defined($Desc)
            ? " - $Desc"
            : ''
    );
    $Txt =~ s/$tmpdb/[tmpdb]/g;
    my $TMP_STDERR = 'conv-suuid-stderr.tmp';

    if (defined($Exp_stderr)) {
        $stderr_cmd = " 2>$TMP_STDERR";
    }
    is(`$Cmd$stderr_cmd`, "$Exp_stdout", "$Txt (stdout)");
    my $ret_val = $?;
    if (defined($Exp_stderr)) {
        is(file_data($TMP_STDERR), $Exp_stderr, "$Txt (stderr)");
        unlink($TMP_STDERR);
    } else {
        diag("Warning: stderr not defined for '$Txt'");
    }
    is($ret_val >> 8, $Exp_retval, "$Txt (retval)");
    return;
    # }}}
} # testcmd()

sub likecmd {
    # {{{
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    my $stderr_cmd = '';
    my $Txt = join('',
        "\"$Cmd\"",
        defined($Desc)
            ? " - $Desc"
            : ''
    );
    $Txt =~ s/$tmpdb/[tmpdb]/g;
    my $TMP_STDERR = 'conv-suuid-stderr.tmp';

    if (defined($Exp_stderr)) {
        $stderr_cmd = " 2>$TMP_STDERR";
    }
    like(`$Cmd$stderr_cmd`, "$Exp_stdout", "$Txt (stdout)");
    my $ret_val = $?;
    if (defined($Exp_stderr)) {
        like(file_data($TMP_STDERR), "$Exp_stderr", "$Txt (stderr)");
        unlink($TMP_STDERR);
    } else {
        diag("Warning: stderr not defined for '$Txt'");
    }
    is($ret_val >> 8, $Exp_retval, "$Txt (retval)");
    return;
    # }}}
} # likecmd()

sub file_data {
    # Return file content as a string {{{
    my $File = shift;
    my $Txt;
    if (open(my $fp, '<', $File)) {
        local $/ = undef;
        $Txt = <$fp>;
        close($fp);
        return($Txt);
    } else {
        return;
    }
    # }}}
} # file_data()

sub print_version {
    # Print program version {{{
    print("$progname v$VERSION\n");
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

Usage: $progname [options] [file [files [...]]]

Contains tests for the conv-suuid(1) program.

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
    # }}}
} # usage()

sub msg {
    # Print a status message to stderr based on verbosity level {{{
    my ($verbose_level, $Txt) = @_;

    if ($Opt{'verbose'} >= $verbose_level) {
        print(STDERR "$progname: $Txt\n");
    }
    return;
    # }}}
} # msg()

__END__

# Plain Old Documentation (POD) {{{

=pod

=head1 NAME

run-tests.pl

=head1 SYNOPSIS

conv-suuid.t [options] [file [files [...]]]

=head1 DESCRIPTION

Contains tests for the conv-suuid(1) program.

=head1 OPTIONS

=over 4

=item B<-a>, B<--all>

Run all tests, also TODOs.

=item B<-h>, B<--help>

Print a brief help summary.

=item B<-t>, B<--todo>

Run only the TODO tests.

=item B<-v>, B<--verbose>

Increase level of verbosity. Can be repeated.

=item B<--version>

Print version information.

=back

=head1 AUTHOR

Made by Øyvind A. Holm S<E<lt>sunny@sunbase.orgE<gt>>.

=head1 COPYRIGHT

Copyleft © Øyvind A. Holm E<lt>sunny@sunbase.orgE<gt>
This is free software; see the file F<COPYING> for legalese stuff.

=head1 LICENCE

This program is free software; you can redistribute it and/or modify it 
under the terms of the GNU General Public License as published by the 
Free Software Foundation; either version 2 of the License, or (at your 
option) any later version.

This program is distributed in the hope that it will be useful, but 
WITHOUT ANY WARRANTY; without even the implied warranty of 
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along 
with this program.
If not, see L<http://www.gnu.org/licenses/>.

=head1 SEE ALSO

=cut

# }}}

# vim: set fenc=UTF-8 ft=perl fdm=marker ts=4 sw=4 sts=4 et fo+=w :
