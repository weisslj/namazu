#
# -*- Perl -*-
# $Id: excel.pl,v 1.6 2000-03-16 13:17:10 satoru Exp $
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
    my ($magic) = @_;

    $magic->addFileExts('\\.xls$', 'application/excel');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.excel');
    my $tmpfile2 = util::tmpnam('NMZ.excel2');


    util::vprint("Processing ms-excel file ... (using  '$xlconvpath')\n");

    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }

#

    # -m: No encoding for multibyte. It's necessary to
    # handle a Japanese Excel 5.0 or 95 document correctly.
    system("$xlconvpath -m $tmpfile > $tmpfile2");

    {
	my $fh = util::efopen("< $tmpfile2");
	$$cont = util::readfile($fh);
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
	my $encoding = "u8"; # UTF-8
	# Pattern for xlHtml version 0.2.6.
	if ($$cont =~ m!^<FONT SIZE=-1><I>Last Updated&nbsp;using Excel 5.0 or 95</I></FONT><br>$!m) 
	{
	    $encoding = "s"; # Shift_JIS
	}
	{
	    my $fh = util::efopen("> $tmpfile");
	    print $fh $$cont;
	}
	system("$utfconvpath -I$encoding -Oej $tmpfile > $tmpfile2");
	{
	    my $fh = util::efopen("< $tmpfile2");
	    $$cont = util::readfile($fh);
	}
    } 

    # Extract the author and exclude xlHtml's footer at once.
    $$cont =~ s!^<FONT SIZE=-1><I>Spreadsheet's Author:&nbsp;(.*?)</I></FONT><br>.*!!ms;  # '
    $fields->{'author'} = $1;

    unlink($tmpfile);
    unlink($tmpfile2);

    html::html_filter($cont, $weighted_str, $fields, $headings);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);

    # Title shoud be overwritten with the file name.
    # Because xlHtml generate poor <TITLE>/foo/bar/NMZ.excel.tmp</TITLE>.
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str);
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}

1;
