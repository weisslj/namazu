#
# -*- Perl -*-
# $Id: pdf.pl,v 1.1 1999-08-28 00:46:51 satoru Exp $
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

package pdf;
use strict;
require 'util.pl';
#require 'filter.pl';

my $TMPFILE = util::tmpnam('NMZ.pdf');
my $TMPFILE2 = util::tmpnam('NMZ.pdf2');

sub mediatype() {
    return ('application/pdf');
}

sub status() {
    my $pdfconvpath = util::checkcmd('pdftotext');
    return 'yes' if (defined $pdfconvpath);
    return 'no';
}

sub recursive() {
    return 0;
}

sub filter ($$$$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields, $size)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    print "Proccessing pdf file ... (use '@PDFTOTEXT_PATH@')\n"
      if ($conf::VerboseOpt);

    my $fh = util::efopen("> $TMPFILE");
    print $fh $$cont;
    undef $fh;

    system("@PDFTOTEXT_PATH@ -eucjp $TMPFILE $TMPFILE2");
    $fh = util::efopen("< $TMPFILE2");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($TMPFILE);
    unlink($TMPFILE2);

    filter::line_adjust_filter($cont) unless $conf::NoLineAdOpt;
    filter::line_adjust_filter($weighted_str) unless $conf::NoLineAdOpt;
    filter::white_space_adjust_filter($cont);
    $fields->{title} = filter::filename_to_title($cfile, $weighted_str) 
      unless $fields->{title};
    filter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

1;
