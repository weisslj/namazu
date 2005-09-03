#
# -*- Perl -*-
# $Id: builtinwakati.pl,v 1.1 2005-09-03 08:41:07 usu Exp $
# Copyright (C) 1998 Satoru Takabayashi All rights reserved.
# Copyright (C) 2005 Namazu Project All rights reserved.
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
#
# Built-in wakati routine 
#

package builtinwakati;
use strict;
require 'util.pl';

use Fcntl;
use SDBM_File;
my $CHAR  = "(?:[\x21-\x7e]|[\xa1-\xfe][\xa1-\xfe])";
my $NONKANJI = "(?:[\x21-\x7e]|[\xa1-\xaf][\xa1-\xfe])";
my $KIGOU = "(?:[\xa1\xa2\xa6-\xa8][\xa1-\xfe])";
my $ALNUM = "(?:\xa3[\xa1-\xfe])";
my $CHOON    = "(?:[\xa1][\xbc\xc1\xdd])";
my $HIRAGANA = "(?:(?:[\xa4][\xa1-\xf3])|$CHOON)";
my $KATAKANA = "(?:(?:[\xa5][\xa1-\xf6])|$CHOON)";
my $KANJI    = "(?:[\xb0-\xfe][\xa1-\xfe]|\xa1\xb9)";
my %dict;

sub init ($) {
    my ($sdbm_filename) = @_;
    tie %dict, "SDBM_File", $sdbm_filename, O_RDONLY, 0644 or
        util::cdie(_("invalid wakati module: Built-in Module")."$!\n");
    END {
        untie %dict;
    }; 
}
sub wakatize ($);
sub wakatize ($) {
    my ($string) = @_;
    my $rest_string = $string;
    my @parts = ();

    if (length($string) <= 4) { # too short to wakatize
        return ($string, '');
    }
    while (length($rest_string) > 0) {
        my $tmp = $rest_string;
        my $try = "";
        my $matched_part;

        # get the match
        while ($tmp =~ /\G($CHAR)/ogc) { 
            $try .= $1;
            last if ($try eq $string);
            if (defined($dict{$try})) {
                $matched_part = $try;
            } 
        }
        if (defined($matched_part)) {  # matched!
            $rest_string =~ s/^$matched_part//;
            push(@parts, $matched_part);
        } else {
            last;
        }
    }
    push(@parts, $rest_string) if $rest_string;
    return  ($string, '') if (($#parts == 0) && $rest_string);
    my $parts2;
    foreach my $part (@parts){
        my ($longpart, $flinders) = wakatize($part);
        $parts2 .= $longpart . " " . $flinders;
    }
    return (join(' ', @parts), $parts2); # return with value
}

sub wakati ($) {
    my ($contref) = @_;
    my $l_wakatized_content='';
    my $s_wakatized_content='';
    while (1) {
        my $tmp = '';
        if ($$contref =~ /\G($KANJI+(?:$HIRAGANA)*|$HIRAGANA+)(\s*)/ogc) {
            my ($l_wakati_cont, $s_wakati_cont) = wakatize($1);
            $l_wakatized_content .= $l_wakati_cont;
            $l_wakatized_content .= $2 ? $2 : " ";
            $s_wakatized_content .= $s_wakati_cont;
            $s_wakatized_content .= $2 ? $2 : " ";
        } elsif ($$contref =~ 
                 /\G
                 ([\x21-\x7e]+|$KATAKANA+|$ALNUM+|$KIGOU+|\S+)
                 (\s*)
                 /ogcx) 
        {
            $l_wakatized_content .= $1;
            $l_wakatized_content .= $2 ? $2 : " ";
        } elsif ($$contref =~ /\G(\s+)/ogc) {
            $l_wakatized_content .= $1;
        } else {
            last;
        }
    }
    my @uniq;
    my %seen = ();
    foreach my $tmp (split(' ', $s_wakatized_content)) {
        push(@uniq, $tmp) unless $seen{$tmp};
        $seen{$tmp} = 1;
    }
    return "$l_wakatized_content { @uniq }";
}


1;
