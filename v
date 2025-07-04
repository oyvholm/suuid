#!/usr/bin/env perl

#==============================================================================
# v
# File ID: 797f5e70-fa63-11dd-9838-000475e441b9
#
# Kaller opp Vim og logger sessionen med suuid.
#
# Character set: UTF-8
# ©opyleft 2009– Øyvind A. Holm <sunny@sunbase.org>
# License: GNU General Public License version 2 or later, see end of file for 
# legal stuff.
#==============================================================================

use strict;
use warnings;
use Getopt::Long;

local $| = 1;

our %Opt = (

    'comment' => "",
    'gvim' => 0,
    'help' => 0,
    'last' => 0,
    'quiet' => 0,
    'tag' => "c_v",
    'verbose' => 0,
    'version' => 0,

);

our $progname = $0;
$progname =~ s/^.*\/(.*?)$/$1/;
our $VERSION = '0.2.0';
my $cmdline_str = join(" ", @ARGV);

Getopt::Long::Configure('bundling');
GetOptions(

    'comment|c=s' => \$Opt{'comment'},
    'gui|g' => \$Opt{'gui'},
    'help|h' => \$Opt{'help'},
    'last|l' => \$Opt{'last'},
    'quiet|q+' => \$Opt{'quiet'},
    'tag|t=s' => \$Opt{'tag'},
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
    my $Lh = "[0-9a-fA-F]";
    my $v1_templ = "$Lh\{8}-$Lh\{4}-1$Lh\{3}-$Lh\{4}-$Lh\{12}";
    my $comm_str = "";

    if (length($Opt{'comment'})) {
        $comm_str = sprintf(" <comment>%s</comment>", $Opt{'comment'});
    }

    my @Fancy = ("<cmdline>" . suuid_xml($cmdline_str) . "</cmdline>");
    my @Files = ();
    my @Other = ();
    my @stat_array = ();
    my %smsum = ();
    my %gitsum = ();
    my ($old_mdate, $new_mdate) = ("", "");

    for my $curr_arg (@ARGV) {
        if (-r $curr_arg) {
            my $chk_swap = $curr_arg;

            if ($chk_swap =~ /\//) {
                $chk_swap =~ s/^(.*\/)(.+?)$/$1.$2.swp/;
            } else {
                $chk_swap = ".$curr_arg.swp";
            }
            msg(2, "\$chk_swap = '$chk_swap'");

            if (-e $chk_swap) {
                die("$progname: $curr_arg: Swap file $chk_swap exists, "
                    . "seems as the file is being edited already\n");
            }

            unless (@stat_array = stat($curr_arg)) {
                die("$progname: $curr_arg: Cannot stat file: $!\n");
            }

            $old_mdate = sec_to_string($stat_array[9]);
            my $head_str = $Opt{'last'} ? "tail" : "head";

            chomp(my $file_id = `finduuid "$curr_arg" | $head_str -1`);
            chomp($smsum{"o.$curr_arg"} = `smsum <"$curr_arg"`);
            length($smsum{"o.$curr_arg"})
            || die("$progname: Error calculating smsum\n");
            chomp($gitsum{"o.$curr_arg"} = `git hash-object "$curr_arg"`);
            length($gitsum{"o.$curr_arg"})
            || die("$progname: Error calculating gitsum\n");

            push(@Files, $curr_arg);
            push(@Fancy,
                "<file> "
                . "<name>$curr_arg</name> "
                . (length($file_id) ? "<fileid>$file_id</fileid> " : "")
                . "<smsum>" . $smsum{"o.$curr_arg"} . "</smsum> "
                . "<gitsum>" . $gitsum{"o.$curr_arg"} . "</gitsum> "
                . "<mdate>$old_mdate</mdate> "
                . "</file>"
            );
        } elsif ($curr_arg =~ /^-\S/) {
            push(@Fancy, sprintf("<option>%s</option>", suuid_xml($curr_arg)));
        } else {
            push(@Fancy, sprintf("<other>%s</other>", suuid_xml($curr_arg)));
            push(@Other, $curr_arg);
        }
    }

    my $cmd_str = suuid_xml(join(" ", @Fancy), 1);
    my $exec_str = "suuid --raw -t $Opt{'tag'}_begin -w eo -c '<$Opt{'tag'} "
                   . "w=\"begin\"> $cmd_str$comm_str </$Opt{'tag'}>'";
    chomp(my $uuid=`$exec_str`);

    if (!defined($uuid) || $uuid !~ /^$v1_templ$/) {
        die("$progname: suuid error\n");
    }

    defined($ENV{'SESS_UUID'}) || ($ENV{'SESS_UUID'} = "");

    my @vim_str = $Opt{'gui'} ? ("gvim", "-f") : ("vim");

    $ENV{'SESS_UUID'} .= "v/$uuid,";
    system(@vim_str, @ARGV);
    $ENV{'SESS_UUID'} =~ s/v\/$uuid,//;

    my ($change_str, $gone_str, $other_str) = ("", "", "");
    for my $Curr (@Files) {
        if (!-r $Curr) {
            $gone_str .= sprintf(" <file> <name>%s</name> </file>",
                                 suuid_xml($Curr));
            next;
        }
        chomp($smsum{"n.$Curr"} = `smsum <"$Curr"`);
        chomp($gitsum{"n.$Curr"} = `git hash-object "$Curr"`);

        if ($smsum{"o.$Curr"} ne $smsum{"n.$Curr"}) {
            chomp(my $file_id = `finduuid "$Curr" | head -1`);

            unless (@stat_array = stat($Curr)) {
                die("$progname: $Curr: Cannot stat file: $!\n");
            }

            $new_mdate = sec_to_string($stat_array[9]);

            $change_str .= sprintf(
                " <file>"
                . " <name>%s</name>"
                . "%s" # File ID, the first UUID in the file
                . " <old>%s</old>"
                . " <new>%s</new>"
                . " <oldgitsum>%s</oldgitsum>"
                . " <newgitsum>%s</newgitsum>"
                . " <oldmdate>%s</oldmdate>"
                . " <newmdate>%s</newmdate>"
                . " </file>",
                suuid_xml($Curr),
                length($file_id) ? " <fileid>$file_id</fileid>" : "",
                $smsum{"o.$Curr"}, $smsum{"n.$Curr"},
                $gitsum{"o.$Curr"}, $gitsum{"n.$Curr"},
                $old_mdate, $new_mdate);
        }
    }

    if (length($change_str)) {
        $change_str = " <changed>$change_str </changed>";
    }
    if (length($gone_str)) {
        $gone_str = " <gone>$gone_str </gone>";
    }

    for my $Curr (@Other) {
        if (-r $Curr) {
            chomp(my $file_id = `finduuid "$Curr" | head -1`);

            unless (@stat_array = stat($Curr)) {
                die("$progname: $Curr: Cannot stat file: $!\n");
            }

            my $mtime = sec_to_string($stat_array[9]);

            chomp(my $smsum = `smsum <"$Curr"`);
            chomp(my $gitsum = `git hash-object "$Curr"`);

            $other_str .= sprintf(" <file>"
                                  . " <name>%s</name>"
                                  . "%s" # File ID, the first UUID in the file
                                  . " <smsum>%s</smsum>"
                                  . " <gitsum>%s</gitsum>"
                                  . " <mtime>%s</mtime>"
                                  . " </file>",
                                  suuid_xml($Curr),
                                  length($file_id)
                                      ? " <fileid>$file_id</fileid>"
                                      : "",
                                  $smsum,
                                  $gitsum,
                                  $mtime);
        }
    }

    if (length($other_str)) {
        $other_str = " <created>$other_str </created>";
    }

    system("suuid --raw -t $Opt{'tag'}_end -c '<$Opt{'tag'} w=\"end\"> "
           . "<finished>$uuid</finished>$change_str$gone_str$other_str "
           . "</$Opt{'tag'}>'");

    return $Retval;
}

sub sec_to_string {
    # Convert seconds since 1970 to "yyyy-mm-ddThh:mm:ssZ"
    my ($Seconds) = shift;

    my @TA = gmtime($Seconds);
    my($DateString) = sprintf("%04u-%02u-%02uT%02u:%02u:%02uZ",
                              $TA[5]+1900, $TA[4]+1, $TA[3],
                              $TA[2], $TA[1], $TA[0]);

    return $DateString;
}

sub suuid_xml {
    my ($Str, $skip_xml) = @_;

    defined($skip_xml) || ($skip_xml = 0);
    if (!$skip_xml) {
        $Str =~ s/&/&amp;/gs;
        $Str =~ s/</&lt;/gs;
        $Str =~ s/>/&gt;/gs;
    }
    $Str =~ s/\\/\\\\/gs;
    $Str =~ s/\n/\\n/gs;
    $Str =~ s/\t/\\t/gs;

    return $Str;
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

Options:

  -c X, --comment X
    Session comment.
  -g, --gui
    Use graphical (gui) version of Vim, i.e. gvim.
  -h, --help
    Show this help.
  -l, --last
    Use last File ID found in the file instead of first.
  -q, --quiet
    Be more quiet. Can be repeated to increase silence.
  -t X, --tag X
    Use X as suuid tag.
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
