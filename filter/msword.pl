#
# -*- Perl -*-
# $Id: msword.pl,v 1.24 2000-04-06 19:05:34 kenzo- Exp $
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

package msword;
use strict;
use File::Copy;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $wordconvpath  = undef;
my $utfconvpath   = undef;
my $wvversionpath = undef;

sub mediatype() {
    return ('application/msword');
}

sub status() {
    $wordconvpath = util::checkcmd('wvHtml');
#    return 'no' unless defined $wordconvpath;
    if (defined $wordconvpath) {
    if (!util::islang("ja")) {
	return 'yes';
    } else {
	$utfconvpath   = util::checkcmd('lv');
        $wvversionpath = util::checkcmd('wvVersion');
	if ((defined $utfconvpath) && (defined $wvversionpath)) {
	    return 'yes';
	} else {
	    return 'no';
	}
    }
    } else {
        $wordconvpath = util::checkcmd('doccat');
        return 'yes' if defined $wordconvpath;
	return 'no';
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
    my $err = undef;
 
    if (util::checkcmd('wvHtml')) {
    $err = filter_wv($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else { 
    $err = filter_doccat($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }
    return $err;
}   

sub filter_wv ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.word');
    my $tmpfile2 = util::tmpnam('NMZ.word2');


    if (util::islang("ja")) {
    }

    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    { 
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }

    if (!util::islang("ja")) {
	system("$wordconvpath $tmpfile > $tmpfile2");
    } else {
	my $version = "unknown";
	my $supported = undef;
	my $fh_cmd = util::efopen("$wvversionpath $tmpfile |");
	while (<$fh_cmd>) {
	    if (/^Version: (word\d+),/i) {
		$version = $1;
		#
		# Only word8 format is supported for Japanese.
		#
		if ($version =~ /^word8$/) {
		    $supported = 1;
		}
	    }
	}
	return _("Unsupported format: ") .  $version unless $supported;
	system("$wordconvpath $tmpfile | $utfconvpath -Iu8 -Oej > $tmpfile2");
    }

    {
	my $fh = util::efopen("< $tmpfile2");
	$$cont = util::readfile($fh);

	# Exclude wvHtml's footer becaues it has no good index terms.
	$$cont =~ s/<!--Section Ends-->.*$//s;
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

sub filter_doccat ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
 
    my $tmpfile  = util::tmpnam('NMZ.word');
    my $tmpfile2 = util::tmpnam('NMZ.word2');   
    copy("$cfile", "$tmpfile2");

    system("$wordconvpath -o e $tmpfile2 > $tmpfile");

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
