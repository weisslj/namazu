#
# -*- Perl -*-
# $Id: time.pl,v 1.2 2004-01-12 16:51:54 opengl2772 Exp $
# Copyright (C) 2004 Tadamasa Teranishi
#               2004 Namazu Project All rights reserved.
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

package time;
use strict;
use Time::Local;

#
# get timezone :
#      gettimezone() or timezone(-timelocal(gmtime(0)))
#
# 'JST' -> '+0900' :
#      timezone($zone{'JST'} * 3600)
#
# ctime -> rfc822time :
#      ctime_to_rfc822time("Sun Jan 11 16:38:39 JST 2004")
#

my %zone = (
    "UT"  =>   0, "GMT" =>   0,
    "EST" =>  -5, "EDT" =>  -4,
    "CST" =>  -6, "CDT" =>  -5,
    "MST" =>  -7, "MDT" =>  -6,
    "PST" =>  -8, "PDT" =>  -7,

    "JST" =>  +9,

    "A"   =>  +1, "B"   =>  +2, "C"   =>  +3, "D"   =>  +4,
    "E"   =>  +5, "F"   =>  +6, "G"   =>  +7, "H"   =>  +8,
    "I"   =>  +9,

    "K"   => +10, "L"   => +11, "M"   => +12,

    "N"   =>  -1, "O"   =>  -2, "P"   =>  -3, "Q"   =>  -4,
    "R"   =>  -5, "S"   =>  -6, "T"   =>  -7, "U"   =>  -8,
    "V"   =>  -9, "W"   => -10, "X"   => -11, "Y"   => -12,

    "Z"   =>   0,
);

sub ctime_to_rfc822time ($) {
    my ($conf) = @_;

    my $ctime = $$conf;

    if ($ctime =~ /^\s*(\w{3})\s+(\w{3})\s+(\d+)\s+(\d+):(\d+):(\d+)\s+(\d+)/) {
        my $week = $1;
        my $month = $2;
        my $day = $3;
        my $hour = $4;
        my $min = $5;
        my $sec = $6;
        my $year = $7;

        my $timezone = gettimezone();

        $$conf = sprintf("%s, %2.2d %s %d %2.2d:%2.2d:%2.2d %s\n",
            $week, $day, $month, $year, $hour, $min, $sec, $timezone);

        return undef;
    }

    return "Illegal format.";
}

sub timezone ($) {
    my ($time) = @_;

    my $hour = int($time / 3600);
    my $min = abs(int(($time - $hour * 3600) / 60));

    my $timezone = sprintf("%+2.2d%2.2d", $hour, $min);

    return $timezone;
}

sub gettimezone () {
    return timezone(-timelocal(gmtime(0)));
}

sub getwday {
    my ($year, $month, $day) = @_;

    if ($month == 1 or $month == 2) {
        $year--;
        $month += 12;
    }

    return ($year + int($year / 4) - int($year/100) + int($year / 400)
        + int((13 * $month + 8) / 5) + $day) % 7;
}

1;
