#
# -*- Perl -*-
# $Id: util.pl,v 1.8 2000-01-07 01:29:52 knok Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
#     This is free software with ABSOLUTELY NO WARRANTY.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either versions 2, or (at your option)
#  any later version.
# 
#  This program is distributed in the hope that it will be useful
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program; if not, write to the Free Software
#  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
#  02111-1307, USA
#
#  This file must be encoded in EUC-JP encoding
#

package util;
use strict;
use IO::File;

# Replacement for cp command.  Efficiency is nearly equal to cp command. since v1.1.1
sub cp ($$) {
    my ($from, $to) = @_;

    my $fh_from = efopen($from);
    my $fh_to = efopen(">$to");

    dprint("cp: $from -> $to\n");
    my $buf = "";
    while(read ($fh_from, $buf, 16384)) {
        print $fh_to $buf;
    }
}

#  rename() with consideration for OS/2
sub Rename($$) {
    my ($from, $to) = @_;

    return unless -e $from;
    unlink $to if (-f $from) && (-f $to); # some systems require this
    if (0 == rename($from, $to)) {
	cdie("rename($from, $to): $!\n");
    }
    dprint(_("Renamed: ")."$from, $to\n");
}

sub efopen ($) {
    my ($fname) = @_;

    my $fh = fopen($fname) || cdie("$fname: $!\n");

    return $fh;
}

sub fopen ($) {
    my ($fname) = @_;
    my $fh = new IO::File;

    if ($fh->open($fname)) {
	binmode($fh);
    } else {
	$fh = undef;
    }

    return $fh;
}

sub dprint (@) {
    if ($var::Opt{'debug'}) {
	for my $str (@_) {
	    map {print STDERR '// ', $_, "\n"} split "\n", $str;
	}
    }
} 

sub vprint (@) {
    if ($var::Opt{'verbose'} || $var::Opt{'debug'}) {
	for my $str (@_) {
	    map {print STDERR '@@ ', $_, "\n"} split "\n", $str;
	}
    }
} 


sub commas ($) {
    my ($num) = @_;

    $num = "0" if ($num eq "");
#    1 while $num =~ s/(.*\d)(\d\d\d)/$1,$2/;
    # from Mastering Regular Expressions
    $num =~ s<\G((?:^-)?\d{1,3})(?=(?:\d\d\d)+(?!\d))><$1,>g;
    $num;
}

# RFC 822 format without timezone
sub rfc822time ($)
{
    my ($time) = @_;

    my @week_names = ("Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat");
    my @month_names = ("Jan", "Feb", "Mar", "Apr", "May", "Jun",
		       "Jul", "Aug", "Sep", "Oct", "Nov", "Dec");
    my ($sec, $min, $hour, $mday, $mon, $year, $wday, $yday, $isdst) 
	= localtime($time);

    return sprintf("%s, %.2d %s %d %.2d:%.2d:%.2d", 
		   $week_names[$wday],
		   $mday, $month_names[$mon], $year + 1900,
		   $hour, $min, $sec);
}

sub readfile ($) {
    my ($arg) = @_;

    my $fh;
    if (ref $arg) {
	if ($arg =~ /^(IO::File|FileHandle)/) {
	    $fh = $arg;
	} else {
	    warn "$arg: "._("not an IO::File/FileHandle object!\n");
	    return '';
	}
    } else {
	$fh = efopen($arg);
    }

    my $cont = "";
    my $size = -s $fh;
#    if ($size > $conf::FILE_SIZE_LIMIT) {
#	warn "$arg: too large!\n";
#	return '';
#    }
    read $fh, $cont, $size;

    return $cont;
}

sub filesize($) {
    my ($arg) = @_;
    my $fh;
    if (ref $arg) {
	if ($arg =~ /^(IO::File|FileHandle)/) {
	    $fh = $arg;
	} else {
	    warn "$arg: "._("not an IO::File/FileHandle object!\n");
	    return '';
	}
    } else {
	$fh = efopen($arg);
    }
    my $size = -s $fh;
    return $size;
}

# checklib ... check existence of library file 
sub checklib ($) {
    my $libfile = shift;
    for my $path (@INC) {
	my $cpath = "$path/$libfile";
	return 1 if -e $cpath;
    }
    return 0;
}

# checkcmd ... check command path
sub checkcmd ($) {
    my $cmd = shift;
    for my $dir (split(/:|;/, $ENV{'PATH'})) {
	return "$dir/$cmd" if (-x "$dir/$cmd");
	return "$dir/$cmd.com" if (-x "$dir/$cmd.com" &&
		(($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")));
	return "$dir/$cmd.exe" if (-x "$dir/$cmd.exe" &&
		(($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")));
	return "$dir/$cmd.bat" if (-x "$dir/$cmd.bat" &&
										  ($mknmz::SYSTEM eq "MSWin32"));
	return "$dir/$cmd.cmd" if (-x "$dir/$cmd.cmd" &&
										  ($mknmz::SYSTEM eq "os2"));
    }
    return undef;
}

# tmpnam ... make temporary file name
sub tmpnam ($) {
    my ($base) = @_;
    cdie("util::tmpnam: Set \$var::OUTPUT_DIR first!\n") 
	if $var::OUTPUT_DIR eq "";
    dprint("tmpnam: $var::OUTPUT_DIR/$base.$$.tmp\n");
    return "$var::OUTPUT_DIR/$base.$$.tmp";
}

# cdie ... clean files before die
sub cdie (@) {
    my (@msgs)  = @_;

    remove_tmpfiles();
    print STDERR "mknmz: ", @msgs;
    print STDERR "\n" unless $msgs[$#msgs] =~ /\n$/;
    exit 2;
}

# remove_tmpfiles ... remove temporary files which mknmz would make
sub remove_tmpfiles () {
    return unless defined $var::OUTPUT_DIR;
    return if $var::Opt{'debug'};

    my @list = glob "$var::OUTPUT_DIR/NMZ.*.$$.tmp";
    push @list, $var::NMZ{'err'}   if -z $var::NMZ{'err'}; # if size == 0
    push @list, $var::NMZ{'lock'}  if -f $var::NMZ{'lock'};
    push @list, $var::NMZ{'lock2'} if -f $var::NMZ{'lock2'};
    dprint(_("Remove tmporary files:"), @list);
    unlink @list;
}

sub islang($) {
    my ($lang) = @_;

    if ($mknmz::LANG =~ /^$lang/) {  # prefix matching
	return 1;
    } else {
	return 0;
    }
}

sub assert($$) {
    my ($bool, $msg) = @_;

    if (!$bool) {
	die _("ASSERTION ERROR!: ")."$msg";
    }
}
1;
