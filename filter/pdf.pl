#
# -*- Perl -*-
# $Id: pdf.pl,v 1.19 2000-03-15 06:53:50 satoru Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi ,
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

package pdf;
use strict;
require 'util.pl';
require 'gfilter.pl';

my $pdfconvpath = undef;

sub mediatype() {
    return ('application/pdf');
}

sub status() {
    $pdfconvpath = util::checkcmd('pdftotext');
    return 'yes' if (defined $pdfconvpath);
    return 'no';
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile = util::tmpnam('NMZ.pdf');
    my $tmpfile2 = util::tmpnam('NMZ.pdf2');
    util::vprint("Processing pdf file ... (using  '$pdfconvpath')\n");

    my $fh = util::efopen("> $tmpfile");
    print $fh $$cont;
    undef $fh;

    system("$pdfconvpath -q -eucjp $tmpfile $tmpfile2");
    return 'Unable to convert pdf file (maybe copying protection)'
      unless (-e $tmpfile2);

    $fh = util::efopen("< $tmpfile2");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink $tmpfile;
    unlink $tmpfile2;

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
