#
# -*- Perl -*-
# $Id: util.pl,v 1.2 1999-05-04 04:42:38 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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

    my $fh_from = fopen_or_die($from);
    my $fh_to = fopen_or_die(">$to");

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
	die "rename($from, $to): $!\n";
    }
    dprint("// Renamed: $from, $to\n");
}

sub fopen_or_die ($) {
    my ($fname) = @_;

    my $fh = fopen($fname) || die "$fname: $!\n";

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
    print STDERR @_ if $conf::DebugOpt;
} 

sub include($) {
    my ($filename) = @_;

    my $fh_include = util::fopen_or_die($filename);
    my $code = join('',<$fh_include>);
    return $code;
}

sub commas ($) {
    my ($num) = @_;

    $num = "0" if ($num eq "");
#    1 while $num =~ s/(.*\d)(\d\d\d)/$1,$2/;
    # from Mastering Regular Expressions
    $num =~ s<\G((?:^-)?\d{1,3})(?=(?:\d\d\d)+(?!\d))><$1,>g;
    $num;
}

# Check the size of int
sub get_int_size () {
    my ($tmp);
    $tmp = 0;
    $tmp = pack("i", $tmp);
    $conf::INTSIZE = length($tmp);
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

sub read_file ($) {
    my ($file) = @_;
    my $fh_file;

    if ($conf::LANGUAGE eq "ja" && ! $conf::USE_NKF_MODULE) {
	$fh_file = util::fopen_or_die("$conf::NKF -e $file|");
    } else {
	$fh_file = util::fopen_or_die($file);
    }
    my $buf = join("", <$fh_file>);
    $buf = NKF::nkf("-e", $buf) if $conf::USE_NKF_MODULE;
    return $buf;
}

1;
