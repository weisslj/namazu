#
# -*- Perl -*-
# $Id: taro56.pl,v 1.2 2003-03-13 09:56:55 knok Exp $
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

package taro56;
use strict;
require 'util.pl';
require 'gfilter.pl';


sub mediatype() {
    return ('application/ichitaro5', 'application/ichitaro6');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv () {
    return 1;
}

sub add_magic ($) {
    my ($magic) = @_;
    $magic->addFileExts('(?i)\\.jaw', 'application/ichitaro5');
    $magic->addFileExts('(?i)\\.jbw', 'application/ichitaro6');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
    my $tmpfile  = util::tmpnam('NMZ.jstxt');


    my ($dev, $ino, $mode, $nlink, $uid, $gid,
         $rdev, $size, $atime, $mtime, $ctime, $blksize, $blocks)
        = stat($cfile);
    open(IN,$cfile) || die "Can't open $cfile: $!\n"
	if $cfile;
    binmode(IN);
    my $buf;
    sysread(IN, $buf, $size);
    close(IN);

    open(OUT, "> $tmpfile");
    binmode(OUT);

    my @data = unpack("C*", $buf);
    my $textsizep = pack("C4", $data[0x800], $data[0x801],
                               $data[0x802], $data[0x803]);
    my $textsize = unpack("V", $textsizep);

    my $i =0;
    my $code="";
    while ( $i < $textsize) {
        my $code1 = $data[0x803 +$i];
        my $code2 = $data[0x804 +$i];
        if (($code1 == hex("fe")) and ($code2 == hex("41"))) {
            $code = pack("C", hex("0a"));
            $i++;
        }
        if (($code1 >= hex("20")) and ($code1 <= hex("7f"))) {
            $code = pack("C", $code1);
        }
        if (($code1 >= hex("81")) and ($code1 <= hex("84"))
          and ($code2 >= hex("40")) and ($code2 < hex("fe"))) {
            $code = pack("CC", $code1, $code2);
            $i++;
        }
        if (($code1 >= hex("88")) and ($code1 <= hex("9f"))
          and ($code2 >= hex("40")) and ($code2 < hex("fe"))) {
            $code = pack("CC", $code1, $code2);
            $i++;
        }
        if (($code1 >= hex("e0")) and ($code1 <= hex("ea"))
          and ($code2 >= hex("40")) and ($code2 < hex("fe"))) {
            $code = pack("CC", $code1, $code2);
            $i++;
        }
        $i++;
        print OUT $code;
        $code ="";
    }
    close(OUT);

    my $fh = util::efopen("< $tmpfile");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);

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
