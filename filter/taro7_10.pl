#
# -*- Perl -*-
# $Id: taro7_10.pl,v 1.7 2003-08-27 15:37:30 usu Exp $
# Copyright (C) 2003 Yukio USUDA
#               2003 Namazu Project All rights reserved.
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

package taro7_10;
use strict;
require 'util.pl';
require 'gfilter.pl';

my $perlver =$];
$perlver =~ s/\.//;
$perlver =~ m/^(\d\d\d\d)\d*$/;
$perlver = 0;
#$perlver = $1;

sub mediatype() {
    return ('application/x-js-taro');
}

sub status() {
    my $utfconvpath = undef;
    return 'yes' if ($perlver >= 5008);
    $utfconvpath = util::checklib('unicode.pl');
    return 'yes' if $utfconvpath;
    return 'no'; 
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv() {
    return 0;
}

sub add_magic($) {
    my ($magic) = @_;
    $magic->addFileExts('(?i)\\.jtd', 'application/x-js-taro');
    $magic->addFileExts('(?i)\\.jfw', 'application/x-js-taro');
    return;
}

sub filter($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $err = undef;
    $err = taro7_10_filter($orig_cfile, $contref, $weighted_str, $headings, $fields);
    return $err;
}

sub taro7_10_filter($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
    my $err = undef;
    my $tmp = "";
    my @data = unpack("C*", $$contref);
    my $i = 0;
    while ( $i <= $#data) {
        if (pack("C", $data[$i]) eq "T") {
            my $matchdata = "";
            my $j = $i;
            while ( $j <= ($i + 7)) {
                $matchdata .= pack("C", $data[$j]);
                $j++;
            }
            # This is magic word for search text data.
            if ( $matchdata eq "TextV.01"){
                my $textsizep = pack("C4", $data[$i+8], $data[$i+9],
                                        $data[$i+10], $data[$i+11]);
                my $textsize = unpack("N", $textsizep);
                if (($i + $textsize)> $#data){
                    $err = "File information (size of text data) have problem.";
                    return $err;
                }
                my $k = 1;
                while ( $k <= ($textsize * 2)) {
                    $tmp .= pack("C", $data[$i + 11 + $k]);
                    $k++;
                }
                $tmp .= pack("n", 10);
                $i = $j + $k;
            }
        }elsif (pack("C", $data[$i]) eq "\x04") {
            my $matchdata = "";
            my $j = $i;
            while ( $j <= ($i + 9)) {
                $matchdata .= pack("C", $data[$j]);
                $j++;
            }
            # This is magic word for search author's name.
            # This may be imprecise definition.
            if ( $matchdata eq pack("H20", "040000315c4f10620580")){
                my $textsize =  $data[$i+80] - 2;
                my $k = 1;
                my $authorname = "";
                while ( $k <= ($textsize)) {
                    my $hbyte = pack("C", $data[$i + 95 + $k]);
                    my $lbyte = pack("C", $data[$i + 95 + $k +1 ]);
                    $authorname .= $lbyte . $hbyte;
                    $k += 2;
                }
                taro7_10::u16toe(\$authorname);
                $authorname =~ s/\x00//g;
                codeconv::normalize_eucjp(\$authorname);
                $fields->{'author'} = $authorname;
            }
        }
        $i++;
    }

    taro7_10::remove_ctlcodearea(\$tmp);
    taro7_10::u16toe(\$tmp);
    $tmp =~ s/\x00//g;
    codeconv::normalize_eucjp(\$tmp);
    $$contref = $tmp;

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{'title'};
    gfilter::show_filter_debug_info($contref, $weighted_str,
               $fields, $headings);
    return undef;
}

sub remove_ctlcodearea($){
    my ($textref) = @_;
    my $ctl_in  = "\x00\x1c";
    my $ctl_out = "\x00\x1f";
    my $tmptext1 = $$textref;
    my $tmptext2="";
    my $pos1=0;
    my $pos2=0;
    my @incodes;
    while ($tmptext1 =~ /$ctl_in/sg){
        push(@incodes, pos($tmptext1)-2);
    }
    push(@incodes, length($tmptext1));
    my $i=1;
    while (@incodes){
        $pos2=shift(@incodes) ;
        my $tmptext3="";
        $tmptext3=substr($tmptext1, $pos1, $pos2-$pos1);
        $tmptext3=~s/$ctl_in.*$ctl_out//s;
        $tmptext3=~s/$ctl_in.*//s;
        $tmptext2 .= $tmptext3;
        $i++;
        $pos1 = $pos2;
    }
    $$textref = $tmptext2;
}

# convert utf-16 to euc
# require Perl5.8 or unicode.pl
sub u16toe ($) {
    my ($tmp) = @_;
    if ($perlver >= 5008){
        eval 'use Encode;';
        Encode::from_to($$tmp, "utf-16" ,"euc-jp");
    }else{
        eval require 'unicode.pl';
        my @unicodeList = unpack("n*", $$tmp);
        $$tmp = unicode::u2e(@unicodeList);
    }
}

1;
