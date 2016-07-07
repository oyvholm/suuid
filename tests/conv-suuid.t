#!/usr/bin/env perl

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
    use Test::More qw{no_plan};
    # use_ok() goes here
}

use Getopt::Long;

local $| = 1;

our $CMD_BASENAME = "conv-suuid";
our $CMD = "../$CMD_BASENAME";

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
our $VERSION = '0.1.2';

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

my $tmpdb = "tmp-$$-" . substr(rand, 2, 8);

exit(main());

sub main {
    # {{{
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
    diag('Testing Postgres database...');
    testcmd("createdb $tmpdb", '', '', 0, "Create test database");
    likecmd("../$CMD --pg-table -o postgres test.xml | psql -X -d $tmpdb", # {{{
        '/^' .
            'CREATE TABLE\n' .
            'SELECT 0\n' .
            'SELECT 0\n' .
            'CREATE FUNCTION\n' .
            'CREATE TRIGGER\n' .
            'CREATE INDEX\n' .
            'CREATE INDEX\n' .
            'CREATE INDEX\n' .
            '(COPY 4\n)?' .
            '$/',
        '/^$/',
        0,
        'Import test data into database',
    );

    # }}}
    testcmd("psql -X -d $tmpdb -c \"COPY (SELECT * FROM uuids) TO STDOUT;\"", # {{{
        gen_output('test', 'postgres', ''),
        '',
        0,
        "Check contents of database",
    );

    # }}}
    likecmd("../$CMD --pg-table -o postgres test2.xml | psql -X -d $tmpdb", # {{{
        '/^' .
            'CREATE FUNCTION\n' .
            '(COPY 3\n)?' .
        '$/',
        '/^' .
            'ERROR:  relation "uuids" already exists\n' .
            'ERROR:  relation "new" already exists\n' .
            'ERROR:  relation "new_rej" already exists\n' .
            'ERROR:  trigger "trg_insert_new" for relation "new" already exists\n' .
            'ERROR:  relation "idx_uuids_u" already exists\n' .
            'ERROR:  relation "idx_uuids_t" already exists\n' .
            'ERROR:  relation "idx_new_u" already exists\n' .
            '$/',
        0,
        'Import more data into db, table already exists',
    );

    # }}}
    testcmd("psql -X -d $tmpdb -c \"COPY (SELECT * FROM uuids) TO STDOUT;\"", # {{{
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
    testcmd("dropdb $tmpdb", '', '', 0, "Delete test database");

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
    return($Retval);
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
CREATE TABLE new AS
    SELECT * FROM uuids LIMIT 0;
CREATE TABLE new_rej AS
    SELECT * FROM uuids LIMIT 0;

CREATE OR REPLACE FUNCTION loop_new() RETURNS trigger AS \$\$
DECLARE
    curr_id uuid;
BEGIN
    LOOP
        curr_id = (SELECT u FROM new LIMIT 1);
        IF curr_id IS NOT NULL THEN
            IF (SELECT u FROM uuids WHERE u = curr_id LIMIT 1) IS NOT NULL THEN
                INSERT INTO new_rej SELECT * FROM new WHERE u = curr_id;
            ELSE
                INSERT INTO uuids SELECT * FROM new WHERE u = curr_id;
            END IF;
            DELETE FROM new WHERE u = curr_id;
        ELSE
            EXIT;
        END IF;
    END LOOP;
    RETURN NULL;
END;
\$\$ LANGUAGE plpgsql;

CREATE TRIGGER trg_insert_new AFTER INSERT
    ON new
    EXECUTE PROCEDURE loop_new();

CREATE INDEX idx_uuids_u ON uuids (u);
CREATE INDEX idx_uuids_t ON uuids (t);
CREATE INDEX idx_new_u ON new (u);

END
    }
    if ($flags =~ /copy-to-uuids-from-stdin/) {
        $retval .= "COPY new (t, u, tag, host, cwd, username, tty, sess, txt, s) FROM stdin;\n";
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
        }
    }
    return($retval);
    # }}}
} # gen_output()

sub testcmd {
    # {{{
    my ($Cmd, $Exp_stdout, $Exp_stderr, $Exp_retval, $Desc) = @_;
    defined($descriptions{$Desc}) &&
        BAIL_OUT("testcmd(): '$Desc' description is used twice");
    $descriptions{$Desc} = 1;
    my $stderr_cmd = '';
    my $cmd_outp_str = $Opt{'verbose'} >= 1 ? "\"$Cmd\" - " : '';
    my $Txt = join('',
        $cmd_outp_str,
        defined($Desc)
            ? $Desc
            : ''
    );
    $Txt =~ s/$tmpdb/[tmpdb]/g;
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
    return($retval);
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
    my $Txt = join('',
        $cmd_outp_str,
        defined($Desc)
            ? $Desc
            : ''
    );
    $Txt =~ s/$tmpdb/[tmpdb]/g;
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
    return($retval);
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

Usage: $progname [options] [file [files [...]]]

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
