#
# -*- Perl -*-
# $Id: excel.pl,v 1.2 2000-02-27 14:22:29 satoru Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu, 
#               2000 Namazu Project All rights reserved.
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

package excel;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $xlconvpath  = undef;
my $utfconvpath = undef;

sub mediatype() {
    return ('application/excel');
}

sub status() {
    $xlconvpath = util::checkcmd('xlHtml');
    return 'no' unless defined $xlconvpath;
    if (!util::islang("ja")) {
	return 'yes';
    } else {
	$utfconvpath = util::checkcmd('lv');
	if (defined $utfconvpath) {
	    return 'yes';
	} else {
	    return 'no';
	}
    } 
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.excel');
    my $tmpfile2 = util::tmpnam('NMZ.excel2');

    return "Unable to execute excel-converter" unless (-x $xlconvpath);
    return "Unable to execute utf-converter" unless (-x $utfconvpath);

    util::vprint("Processing ms-excel file ... (using  '$xlconvpath', '$utfconvpath')\n");

    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }

    if (util::islang("ja")) {
	system("$xlconvpath $tmpfile | $utfconvpath -Iu8 -Oej > $tmpfile2");
    } else {
	system("$xlconvpath $tmpfile > $tmpfile2");
    }

    {
	my $fh = util::efopen("< $tmpfile2");
	$$cont = util::readfile($fh);
    }

    unlink($tmpfile);
    unlink($tmpfile2);

    html::html_filter($cont, $weighted_str, $fields, $headings);

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
