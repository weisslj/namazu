#
# -*- Perl -*-
# $Id: taro7_10.pl,v 1.2 2003-03-13 09:56:55 knok Exp $
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
use File::Copy;
require 'util.pl';
require 'gfilter.pl';
#require 'unicode.pl';

sub mediatype() {
    return ('application/x-js-taro');
}

sub status() {
    my $unicodepath = util::checklib('unicode.pl');
    return 'yes' if defined $unicodepath;
    return 'no'; 
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv () {
    return 0;
}

sub add_magic ($) {
    my ($magic) = @_;
    $magic->addFileExts('(?i)\\.jtd', 'application/x-js-taro');
    $magic->addFileExts('(?i)\\.jfw', 'application/x-js-taro');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
    my $tmpfile  = util::tmpnam('NMZ.taro');
    my $tmpfile2 = util::tmpnam('NMZ.taro2');
    eval 'require "unicode.pl";';
    open(OUT, "> $tmpfile2");
    binmode(OUT);
    my @data = unpack("C*", $$cont);
    my $i = 0;
    while ( $i <= $#data) {
        if (pack("C", $data[$i]) eq "T") {
            my $matchdata = "";
            my $j = $i;
            while ( $j <= ($i + 7)) {
                $matchdata .= pack("C", $data[$j]);
                $j++;
            }
            if ( $matchdata eq "TextV.01"){
                my $textsizep = pack("C4", $data[$i+8], $data[$i+9],
                                        $data[$i+10], $data[$i+11]);
                my $textsize = unpack("N", $textsizep);
                my $k = 1;
                while ( $k <= ($textsize * 2)) {
                    print OUT pack("C", $data[$i + 11 + $k]);
                    $k++;
                }
                print OUT  pack("n", 10);
                $i = $j + $k;
            }
        }elsif (pack("C", $data[$i]) eq "\x04") {
            my $matchdata = "";
            my $j = $i;
            while ( $j <= ($i + 9)) {
                $matchdata .= pack("C", $data[$j]);
                $j++;
            }
            if ( $matchdata eq pack("H20", "040000315c4f10620580")){
                my $textsize =  $data[$i+80] - 2;
                my $k = 1;
                my $authorname;
                while ( $k <= ($textsize)) {
                    $authorname .= pack("C", $data[$i + 95 + $k]);
                    $k++;
                }
                my @unicodeList = unpack("v*", $authorname);
                $authorname = &unicode::u2e(@unicodeList);
                $authorname =~ s/\x00//g;
                $fields->{'author'} = $authorname;
            }
        }
        $i++;
    }
    close(OUT);

     my $buf; 
     my ($dev, $ino, $mode, $nlink, $uid, $gid,
     $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks)
        = stat("$tmpfile2");
    open(IN, "$tmpfile2");
    binmode(IN);
    read(IN, $buf, $size);
    close(IN);
    my @unicodeList = unpack("n*", $buf);
    my $eucString   = &unicode::u2e(@unicodeList);

    open(OUT, "> $tmpfile");
    binmode(OUT);
    $i =0;
    while ( $i < length($eucString)) {
        my $code1 = unpack("C",substr($eucString, $i, 1));
        my $code2 = unpack("C",substr($eucString, $i+1, 1));
        my $code = "";
        if (($code1 == hex("00")) and ($code2 >= hex("20"))
          and ($code2 <= hex("7f"))) {
            $code = pack("C", $code2);
            $i++;
        }
        if (($code1 == hex("00")) and ($code2 == hex("0a"))) {
            $code = pack("C", $code2);
            $i++;
        }
        if (($code1 >= hex("a1")) and ($code1 <= hex("a8"))
          and ($code2 > hex("a0")) and ($code2 < hex("ff"))) {
            $code = pack("CC", $code1, $code2);
            $i++;
        }
        if (($code1 >= hex("b0")) and ($code1 <= hex("f4"))
          and ($code2 > hex("a0")) and ($code2 < hex("ff"))) {
            $code = pack("CC", $code1, $code2);
            $i++;
        }
        $i++;
        print OUT $code;
    }
    close(OUT);

    {
        my $fh = util::efopen("< $tmpfile");
        $$cont = util::readfile($fh);
    }

    unlink($tmpfile);
    unlink($tmpfile2);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str,
               $fields, $headings);
    return undef;
}

1;

