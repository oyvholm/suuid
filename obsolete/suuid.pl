#!/usr/bin/env perl

#=======================================================================
# suuid
# File ID: 04c64336-f744-11dd-bddd-000475e441b9
#
# Generate and store UUID with optional comment in a log file.
#
# Character set: UTF-8
# ©opyleft 2008– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of 
# file for legal stuff.
#=======================================================================

use strict;
use warnings;
use Fcntl ':flock';
use Getopt::Long;

local $| = 1;

my $suuid_env = "SUUID_LOGDIR";
my $env_dir = defined($ENV{$suuid_env})
    ? $ENV{$suuid_env}
    : "$ENV{'HOME'}/uuids";

our %Std = (

    'dbname' => "suuids",
    'editor' => "vi",
    'logdir' => $env_dir,
    'rcfile' => "$ENV{'HOME'}/.suuidrc",
    'whereto' => "o",

);

our %Opt = (

    'comment' => "",
    'count' => 1,
    'dbname' => $Std{'dbname'},
    'help' => 0,
    'logdir' => $Std{'logdir'},
    'random-mac' => 0,
    'raw' => 0,
    'rcfile' => $Std{'rcfile'},
    'quiet' => 0,
    'tag' => "",
    'verbose' => 0,
    'version' => 0,
    'whereto' => $Std{'whereto'},

);

our $progname = $0;
$progname =~ s/^.*\/(.*?)$/$1/;
our $VERSION = '0.1.1';

Getopt::Long::Configure('bundling');
GetOptions(

    'comment|c=s' => \$Opt{'comment'},
    'count|n=i' => \$Opt{'count'},
    'dbname|d=s' => \$Opt{'dbname'},
    'help|h' => \$Opt{'help'},
    'logdir|l=s' => \$Opt{'logdir'},
    'quiet|q+' => \$Opt{'quiet'},
    'random-mac|m' => \$Opt{'random-mac'},
    'raw' => \$Opt{'raw'},
    'rcfile=s' => \$Opt{'rcfile'},
    'tag|t=s' => \$Opt{'tag'},
    'verbose|v+' => \$Opt{'verbose'},
    'version' => \$Opt{'version'},
    'whereto|w=s' => \$Opt{'whereto'},

) || die("$progname: Option error. Use -h for help.\n");

$Opt{'verbose'} -= $Opt{'quiet'};
msg(2, "\@INC = '" . join("', '", @INC) . "'");
$Opt{'help'} && usage(0);
if ($Opt{'version'}) {
    print_version();
    exit(0);
}

my $Lh = "[0-9a-fA-F]";
my $Templ = "$Lh\{8}-$Lh\{4}-$Lh\{4}-$Lh\{4}-$Lh\{12}";
my $v1_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\{12}";
my $v1rand_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\[37bf]$Lh\{10}";

my %rc = read_rcfile();
my $Cmd = defined($rc{'uuidcmd'})
    ? $rc{'uuidcmd'}
    : "uuidgen -t";

exit(main());

sub main {
    # {{{
    my $Retval = 0;
    my $Host = host_id();

    if (!-d "$Opt{'logdir'}/.") {
        die("$progname: $Opt{'logdir'}: Log directory not found\n");
    }
    my $Logfile = "$Opt{'logdir'}/$Host.xml";

    my $Comment = length($Opt{'comment'}) ? $Opt{'comment'} : "";
    my $cmd_editor = $ENV{'SUUID_EDITOR'} || $ENV{'EDITOR'} || $Std{'editor'};
    msg(2, "cmd_editor = '$cmd_editor'");

    if ($Comment eq "-") {
        $Comment = "";
        while (<STDIN>) {
            $Comment .= $_;
        }
        chomp($Comment);
    } elsif ($Comment eq "--") {
        my $tmpfile = "/tmp/" . `$Cmd`; # FIXME
        system("touch $tmpfile");
        system("$cmd_editor $tmpfile");
        $Comment = `cat $tmpfile`;
        unlink($tmpfile);
    }
    if (!valid_xml_chars($Comment)) {
        warn("$progname: Comment contains illegal characters or is not valid UTF-8\n");
        exit(1);
    }
    my $Tag = length($Opt{'tag'}) ? $Opt{'tag'} : "";
    if (!valid_xml_chars($Tag)) {
        warn("$progname: Tags have to be in UTF-8\n");
        exit(1);
    }
    my $Hostname = host_id();
    chomp($Hostname);
    my $curr_dir = `/bin/pwd`; # FIXME
    chomp($curr_dir);

    $Comment =~ s/^\s+//;
    $Comment =~ s/\s+$//;
    my $safe_comment = suuid_xml($Comment, $Opt{'raw'});
    my $safe_tag = suuid_xml($Tag);
    my ($safe_host, $safe_currdir) = ("", "");
    length($Hostname) && ($safe_host = suuid_xml($Hostname));
    length($curr_dir) && ($safe_currdir = suuid_xml($curr_dir));
    my $Username = getpwuid($<);
    my $safe_username = suuid_xml($Username);
    chomp(my $tty = `tty`);
    my $safe_tty = suuid_xml($tty);

    my $sess_uuid = "";
    if (defined($ENV{'SESS_UUID'})) {
        if ($ENV{'SESS_UUID'} =~ /([[:xdigit:]]{8}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{4}-[[:xdigit:]]{12})/) {
            $sess_uuid = $ENV{'SESS_UUID'};
        }
    }

    # If the log file doesn't exist, create an empty one.

    if (!-e $Logfile) {
        open(LogFP, ">", $Logfile) || die("$progname: $Logfile: Cannot create file: $!\n");
        flock(LogFP, LOCK_EX) || die("$progname: $Logfile: Cannot flock(): $!\n");
        print(LogFP join("\n",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>",
            "<!DOCTYPE suuids SYSTEM \"dtd/suuids.dtd\">",
            "<suuids>",
            "</suuids>"
        ) . "\n");
        close(LogFP);
    }

    # Open the log file for read+write and use exclusive lock.

    if (open(LogFP, "+>>", $Logfile)) {
        # {{{
        if (flock(LogFP, LOCK_EX)) {

            # Seek to start of the end tag, read it to be sure it's OK.

            unless (seek(LogFP, -10, 2)) {
                warn("$progname: Cannot seek to start of end tag, adding to file. Error: $!\n");
            }
            my $filepos = tell(LogFP);
            my $check_end_tag = <LogFP>;
            msg(2, "check_end_tag = '$check_end_tag'");
            defined($check_end_tag) || ($check_end_tag = "");

            # If the end tag was read correctly, truncate log file 
            # there, otherwise just add new entries to the end.

            if ($check_end_tag eq "</suuids>\n") {
                msg(2, "check_end_tag is OK");
                truncate(LogFP, $filepos) || die("$progname: $Logfile: Cannot truncate() to size $filepos: $!\n");
            } else {
                warn("$progname: $Logfile: Unknown end line, adding to end of file\n");
            }

            # Generate $Count UUIDs and write them to the log file.

            for (my $Count = 0; $Count < $Opt{'count'}; $Count++) {
                seek(LogFP, 0, 2) || die("$progname: Cannot seek to end of file: $!\n");
                my $filepos = tell(LogFP);

                # Generate the UUID

                chomp(my $uuid = `$Cmd`);
                if ($uuid !~ /^$v1_templ$/) {
                    warn("$progname: '$uuid': Generated UUID is not in the expected format\n");
                    $Retval = 1;
                    last;
                }

                # If -m/--random-mac is specified, replace the last 
                # field with random data specified by the standard.

                if ($Opt{'random-mac'}) {
                    $uuid =~ s/^(........................)(............)$/sprintf("%s%s", $1, random_mac())/e;
                }

                # Generate XML based on what we have and push it to an 
                # array.

                my @Xml = ();
                if (length($safe_tag)) {
                    my $tmp_tag = $safe_tag;
                    $tmp_tag =~ s/([^,]+)/push(@Xml, sprintf("<tag>%s<\/tag>", suuid_xml($1)))/gse;
                }
                my $txt_space = ($safe_comment =~ /^</) ? " " : "";
                length($safe_comment) && push(@Xml, "<txt>$txt_space$safe_comment$txt_space</txt>");
                length($safe_host) && push(@Xml, "<host>$safe_host</host>");
                length($safe_currdir) && push(@Xml, "<cwd>$safe_currdir</cwd>");
                length($safe_username) && push(@Xml, "<user>$safe_username</user>");
                length($safe_tty) && push(@Xml, "<tty>$safe_tty</tty>");
                length($sess_uuid) && push(@Xml, parse_sess_uuid($sess_uuid));
                my $uuid_time = uuid_time2($uuid);
                length($uuid_time) && ($uuid_time = " t=\"$uuid_time\"");
                my $out_str = join(" ", "<suuid$uuid_time u=\"$uuid\">", @Xml, "</suuid>") . "\n";

                # Print the entry to the log file.

                print(LogFP $out_str);
                if (seek(LogFP, $filepos, 0)) {
                    my $check_str = <LogFP>;
                    if ($check_str eq $out_str) {
                        $Opt{'whereto'} =~ /[ao]/ && print("$uuid\n");
                        $Opt{'whereto'} =~ /[ae]/ && print(STDERR "$uuid\n");
                    } else {
                        msg(2, "check_str = '$check_str', out_str = '$out_str'");
                        die("$progname: $Logfile: Unable to write to file: $!\n");
                    }
                } else {
                    die("$progname: $Logfile: Cannot seek to position $filepos: $!\n");
                }
                # FIXME: The psql thing below is experimental at the 
                # moment. It works, but there's no guarantee that it 
                # goes into a database, for example if it doesn't exist. 
                # The XML files are still the place where the important 
                # data is stored.
                if ($rc{'use-postgres'}) {
                    if (open(PsqlFP, "| conv-suuid -o postgres | psql -X -q -d $Opt{'dbname'}")) {
                        print(PsqlFP $out_str);
                        close(PsqlFP);
                    }
                }
            }

            # Done writing the entries, add end tag and close the file.

            seek(LogFP, 0, 2) || die("$progname: Cannot seek to end of file to write end tag: $!\n");
            print(LogFP "</suuids>\n");
            close(LogFP);
        } else {
            die("$progname: $Logfile: Cannot lock file: $!\n");
        }
        # }}}
    } else {
        die("$progname: $Logfile: Cannot open file for append: $!\n");
    }
    return($Retval);
    # }}}
} # main()

sub uuid_time2 {
    # {{{
    my $uuid = shift;
    my $Retval = "";
    my $Lh = "[0-9a-fA-F]"; # FIXME: Should be global
    my $v1_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\{12}"; # FIXME: Should be global
    ($uuid =~ /^$v1_templ$/) || return("");
    my $hex_string = uuid_hex_date($uuid);
    my $val = bighex($hex_string);
    my $nano = sprintf("%07u", $val % 10_000_000);
    my $t = ($val / 10_000_000) - 12_219_292_800;
    my @TA = gmtime($t);
    $Retval = sprintf("%04u-%02u-%02uT%02u:%02u:%02u.%sZ",
        $TA[5]+1900, $TA[4]+1, $TA[3],
        $TA[2], $TA[1], $TA[0], $nano);
    return($Retval);
    # }}}
} # uuid_time2()

sub uuid_hex_date {
    # {{{
    my $uuid = shift;
    my $time_low = lc(substr($uuid, 0, 8));
    my $time_mid = lc(substr($uuid, 9, 4));
    my $time_hi = lc(substr($uuid, 15, 3));
    # CO: Notes {{{
    # 2639d59e-fa20-11dd-8aa6-000475e441b9
    # 012345678901234567890123456789012345
    # 000000000011111111112222222222333333
    #
    # 2639d59e 0-3 time_low (0-7)
    # -
    # fa20 4-5 time_mid (9-12)
    # -
    # 11dd 6-7 time_hi_and_version (15-17)
    # -
    # 8a  8 clock_seq_hi_and_reserved (19-20)
    # a6  9 clock_seq_low (21-22)
    # -
    # 000475e441b9 10-15 node (24-35)
    # }}}
    my $Retval = "$time_hi$time_mid$time_low";
    # msg(2, "uuid_hex_date('$uuid') returns '$Retval'");
    return($Retval);
    # }}}
} # uuid_hex_date()

sub suuid_xml {
    # {{{
    my ($Str, $skip_conv) = @_;
    defined($skip_conv) || ($skip_conv = 0);
    if (!$skip_conv) {
        $Str =~ s/&/&amp;/gs;
        $Str =~ s/</&lt;/gs;
        $Str =~ s/>/&gt;/gs;
        $Str =~ s/\\/\\\\/gs;
        $Str =~ s/\n/\\n/gs;
        $Str =~ s/\r/\\r/gs;
        $Str =~ s/\t/\\t/gs;
    }
    return($Str);
    # }}}
} # suuid_xml()

sub bighex {
    # {{{
    my $Hex = scalar reverse shift;
    my $Retval = 0;
    my $Digit = 1;
    $Hex =~ s{
        ([0-9A-Fa-f])
    } {
        $Retval += hex($1) * $Digit;
        $Digit *= 16;
        "";
    }gsex;
    length($Hex) && ($Retval = NaN());
    return($Retval);
    # }}}
} # bighex()

sub parse_sess_uuid {
    # Parse a $SESS_UUID variable and return an array with <sess> elements {{{
    my $sess_uuid = shift;
    # FIXME: Copied from ./sess
    my $legal_descchars = join('',
        '\x2d-\x2e', # -.
        '\x30-\x39', # 0-9
        '\x41-\x5a', # A-Z
        '\x5f',      # _
        '\x61-\x7a', # a-z
        '\x80-\xbf', # Valid UTF-8 bytes
        '\xc2-\xef', # Valid UTF-8 bytes
    );
    my @Xml = ();
    my $tmp_sess = $sess_uuid;
    if ($tmp_sess =~ /$Templ/) {
        my $descname = '';
        $tmp_sess =~ s{
            ([$legal_descchars]*?/?)
            ($Templ)
        } {
            my ($desc, $str) = ($1, $2);
            defined($desc) || ($desc = '');
            $desc =~ s/\/$//;
            if ($desc =~ /$Templ/) {
                push(@Xml, parse_sess_uuid($desc));
                $desc = '';
            }
            push(@Xml,
                sprintf("<sess%s>%s</sess>",
                    length($desc)
                        ? sprintf(' desc="%s"', suuid_xml($desc))
                        : '',
                    suuid_xml("$str")
                )
            );
            '';
        }egsx;
    }
    return(@Xml);
    # }}}
} # parse_sess_uuid()

sub valid_xml_chars {
    # Check that the string is valid UTF-8 {{{
    my $text = shift;
    # UTF-8 regexp copied from linux-2.6.git/scripts/checkpatch.pl in 
    # commit ddb503b42960792f3be580f98959add669241a04. Originally from 
    # http://www.w3.org/International/questions/qa-forms-utf-8.en.php .

    my $UTF8 = qr {
        [\x09\x0A\x0D\x20-\x7E]              # ASCII
        | [\xC2-\xDF][\x80-\xBF]             # non-overlong 2-byte
        |  \xE0[\xA0-\xBF][\x80-\xBF]        # excluding overlongs
        | [\xE1-\xEC\xEE\xEF][\x80-\xBF]{2}  # straight 3-byte
        |  \xED[\x80-\x9F][\x80-\xBF]        # excluding surrogates
        |  \xF0[\x90-\xBF][\x80-\xBF]{2}     # planes 1-3
        | [\xF1-\xF3][\x80-\xBF]{3}          # planes 4-15
        |  \xF4[\x80-\x8F][\x80-\xBF]{2}     # plane 16
    }x;

    my $retval;
    $text =~ s/$UTF8//gs;
    $retval = length($text) ? 0 : 1;
    return($retval);
    # }}}
} # valid_xml_chars()

sub read_rcfile {
    # Read userdefined data in $Opt{'rcfile'} {{{
    my %Retval = ();
    if (-e $Opt{'rcfile'}) {
        if (open(RcFP, "<", $Opt{'rcfile'})) {
            while (my $Line = <RcFP>) {
                $Line =~ s/^\s*(.*?)\s*$/$1/;
                if ($Line =~ /^hostname\s*=\s*(\S+)/) {
                    $Retval{'hostname'} = $1;
                } elsif ($Line =~ /^use-postgres\s*=\s*(.+)\s*$/) {
                    $Retval{'use-postgres'} = $1;
                } elsif ($Line =~ /^uuidcmd\s*=\s*(.+)\s*$/) {
                    $Retval{'uuidcmd'} = $1;
                }
            }
            close(RcFP);
        } else {
            warn("$progname: $Opt{'rcfile'}: Cannot open file for read: $!\n");
        }
    }
    return(%Retval);
    # }}}
} # read_rcfile()

sub host_id {
    # Decide which hostname to use {{{
    my $Retval = "";
    if (defined($ENV{'SUUID_HOSTNAME'})) {
        $Retval = $ENV{'SUUID_HOSTNAME'};
    } elsif (defined($rc{'hostname'})) {
        $Retval = $rc{'hostname'};
    } else {
        chomp(my $Hostname = `hostname`);
        if (length($Hostname) && $Hostname ne "localhost") {
            $Retval = $Hostname;
        } else {
            warn("$progname: Warning: hostname not defined, using MAC value\n");
            chomp($Retval = `$Cmd`);
            $Retval =~ s/^.*-([[:xdigit:]]+)$/$1/;
        }
    }
    msg(2, "host_id() returns '$Retval'");
    return($Retval);
    # }}}
} # host_id()

sub random_mac {
    # Return a random MAC address field {{{
    my $Str = "";
    for (1..6) {
        $Str .= sprintf("%02x", int(rand(256)));
    }
    my @Arr = ("3", "7", "b", "f");
    my $mc_val = @Arr[int(rand(4))];
    $Str =~ s/^(.)(.)(..........)$/$1$mc_val$3/;
    return($Str);
    # }}}
} # random_mac()

sub sec_to_string {
    # Convert seconds since 1970 to "yyyy-mm-ddThh:mm:ssZ" {{{
    my ($Seconds) = shift;

    my @TA = gmtime($Seconds);
    my($DateString) = sprintf("%04u-%02u-%02uT%02u:%02u:%02uZ",
                              $TA[5]+1900, $TA[4]+1, $TA[3],
                              $TA[2], $TA[1], $TA[0]);
    return($DateString);
    # }}}
} # sec_to_string()

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

Generates one or more UUIDs and stores it to a log file with optional 
comment or tag/category.

Options:

  -c x, --comment x
    Store comment x in the log file. If "-" is specified as comment, the 
    program will read the comment from stdin. Two hyphens ("--") as a 
    comment opens the editor defined in the environment variable 
    \$SUUID_EDITOR to edit the message. If \$SUUID_EDITOR is not 
    defined, \$EDITOR is read, if not defined, "$Std{'editor'}" is 
    called. Sorry, emacs, but vi(m) rules the street and exists 
    everywhere.
  -d X, --dbname X
    Use Postgres database X. Default: '$Std{'dbname'}'.
  -h, --help
    Show this help.
  -l x, --logdir x
    Store log files in directory x.
    If the $suuid_env environment variable is defined, that value is 
    used. Otherwise the value "\$HOME/uuids" is used.
    Current default: $Std{'logdir'}
  -m, --random-mac
    Don’t use the hardware MAC address, generate a random address field.
  -n x, --count x
    Print and store x UUIDs.
  -q, --quiet
    Be more quiet. Can be repeated to increase silence.
  --raw
    Don’t convert <txt> element to XML. When using this option, it is 
    expected that the value of the -c/--comment option is valid XML, 
    otherwise it will create corrupted log files.
  --rcfile X
    Use file X instead of '$Std{'rcfile'}'.
  -t x, --tag x
    Use x as tag (category).
  -v, --verbose
    Increase level of verbosity. Can be repeated.
  --version
    Print version information.
  -w x, --whereto x
    x is a string which decides where the UUID will be written:
      The string contains 'e' - stderr
      The string contains 'o' - stdout
    All other characters will be ignored. Examples:
      e
        Send to stderr.
      eo
        Send to both stdout and stderr.
      a
        Synonym for eo.
      n
        Don’t output anything.
    Default: "$Std{'whereto'}"

If the \$SESS_UUID environment variable is defined by sess(1) or another 
program, the value is logged if it is an UUID.

A different hostname can be specified in the environment variable 
\$SUUID_HOSTNAME, or in the file $Std{'rcfile'} with the format 
"hostname = xxx".

To also store the entries in a Postgres database, add "use-postgres = 1" 
to $Std{'rcfile'} .

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
