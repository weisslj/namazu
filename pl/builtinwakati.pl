#
# -*- Perl -*-
# $Id: builtinwakati.pl,v 1.3 2006-08-12 05:45:02 opengl2772 Exp $
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

# Unicode: CJK Unified Ideographs(U+4E00 - U+9FAF)
my $KANJI = "(?:[\xE4-\xE9][\x80-\xBF][\x80-\xBF])";
my $CHOON    = "(?:(?:\xE3\x80\x9C)|(?:\xE3\x83\xBC)|(?:\xE2\x80\x95))";
# Unicode: Hiragana(U+3041 - U+3093)
my $HIRAGANA = "(?:(?:\xE3\x81[\x81-\xBF])|(?:\xE3\x82[\x80-\x93])|$CHOON)";
# Unicode: Katakana(U+30A1 - U+30F6)
my $KATAKANA = "(?:(?:\xE3\x82[\xA1-\xBF])|(?:\xE3\x83[\x80-\xB6])|$CHOON)";
# Unicode: (U+FF10 - U+FF19, U+FF21 - U+FF3A, U+FF41 - U+FF5A)
my $ALNUM = "(?:(?:\xEF\xBC[\x90-\x99])|(?:\xEF\xBC[\xA1-\xBA])|(?:\xEF\xBD[\x81-\x9A]))";
#my $CHAR  = "(?:[\x21-\x7e]|[\xC0-\xDF][\x80-\xBF]|[\xE0-\xEF][\x80-\xBF]{2}|[\xF0-\xF8][\x80-\xBF]{3}|[\xF8-\xFB][\x80-\xBF]{4}|[\xFC-\xFD][\x80-\xBF]{5})";
my $CHAR  = "(?:[\x21-\x7e]|[\xC0-\xFD][\x80-\xBF]+)";

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
        return ($string);
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
    return ($string) if (($#parts == 0) && $rest_string);
    return (join(' ', @parts)); # return with value
}

sub wakati ($) {
    my ($contref) = @_;
    my $l_wakati_cont='';
    my $s_wakati_cont='';
    my $ss_wakati_cont='';
    while (1) {
        my $tmp = '';
        if ($$contref =~ /\G($KANJI+(?:$HIRAGANA)*|$HIRAGANA+)(\s*)/ogc) {
            my $l_cont = wakatize($1);
            $l_wakati_cont .= $l_cont;
            $l_wakati_cont .= $2 ? $2 : " ";
            foreach my $tmp (split(/\s+/, $l_cont)) {
                my $s_cont = wakatize($tmp);
                $s_wakati_cont .= $s_cont;
                $s_wakati_cont .= $2 ? $2 : " ";
                foreach my $tmp (split(/\s+/, $s_cont)) {
                    $ss_wakati_cont .= wakatize($tmp);
                    $ss_wakati_cont .= $2 ? $2 : " ";
                }
            }
        } elsif ($$contref =~ 
                 /\G
                 ([\x21-\x7e]+|$KATAKANA+|$ALNUM+|\S+)
                 (\s*)
                 /ogcx) 
        {
            $l_wakati_cont .= $1;
            $l_wakati_cont .= $2 ? $2 : " ";
            $s_wakati_cont .= $1;
            $s_wakati_cont .= $2 ? $2 : " ";
            $ss_wakati_cont .= $1;
            $ss_wakati_cont .= $2 ? $2 : " ";
        } elsif ($$contref =~ /\G(\s+)/ogc) {
            $l_wakati_cont .= " ";
            $s_wakati_cont .= " ";
            $ss_wakati_cont .= " ";
        } else {
            last;
        }
    }
    $s_wakati_cont =~ s!\x7F\d+\x7f.*\x7F/\d+\x7F!!g;
    $ss_wakati_cont =~ s!\x7F\d+\x7f.*\x7F/\d+\x7F!!g;

    my @uniq;
    my %seen = ();
    my $tmptext = '';
    foreach my $tmp (split(/\s+/, $s_wakati_cont . $ss_wakati_cont)) {
        push(@uniq, $tmp) unless $seen{$tmp};
        $seen{$tmp} = 1;
    }
    return($l_wakati_cont . "\n.\n" . $s_wakati_cont . "\n.\n"
           . $ss_wakati_cont . "\n.\n" . join(" ", @uniq));
}

1;
