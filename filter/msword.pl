#
# -*- Perl -*-
# $Id: msword.pl,v 1.4 1999-08-28 11:32:25 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu All rights reserved.
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

package msword;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $TMPFILE  = util::tmpnam('NMZ.word');
my $TMPFILE2 = util::tmpnam('NMZ.word2');

sub mediatype() {
    return ('application/msword');
}

sub status() {
    my $wordconvpath = util::checkcmd('mswordview');
    my $utfconvpath = util::checkcmd('lv');
    return 'yes' if (defined $wordconvpath && defined $utfconvpath);
    return 'no';
}

sub recursive() {
    return 0;
}

sub filter ($$$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields, $size)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $wordconvpath = util::checkcmd('mswordview');
    my $utfconvpath = util::checkcmd('lv');

    util::vprint("Processing ms-word file ... (using  '$wordconvpath', '$utfconvpath')\n");

    my $fh = util::efopen("> $TMPFILE");
    print $fh $$cont;
    undef $fh;

    system("$wordconvpath -o - $TMPFILE | $utfconvpath -Iu8 -Oej > $TMPFILE2");
    $fh = util::efopen("< $TMPFILE2");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($TMPFILE);
    unlink($TMPFILE2);

    html::html_filter($cont, $weighted_str, $fields, $headings);

    gfilter::line_adjust_filter($cont) unless $var::Opt{NoLineAd};
    gfilter::line_adjust_filter($weighted_str) unless $var::Opt{NoLineAd};
    gfilter::white_space_adjust_filter($cont);
    $fields->{title} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

1;
