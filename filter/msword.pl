#
# -*- Perl -*-
# $Id: msword.pl,v 1.36 2003-07-21 11:39:36 usu Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000-2002 Namazu Project All rights reserved.
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
use File::Basename;
use File::Copy;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $wordconvpath  = undef;
my @wordconvopts  = undef;
my $wvversionpath = undef;
my $utfconvpath   = undef;

sub mediatype() {
    return ('application/msword');
}

sub status() {
    $wordconvpath = util::checkcmd('wvHtml');
    if (defined $wordconvpath) {
	if (!util::islang("ja")) {
	    return 'yes';
	} else {
	    $wvversionpath = util::checkcmd('wvVersion');
	    $utfconvpath   = util::checkcmd('lv');
	    if (defined $wvversionpath && defined $utfconvpath) {
		return 'yes';
	    } else {
		return 'no';
	    }
	}
    } else {
        $wordconvpath = util::checkcmd('doccat');
	@wordconvopts = ("-o", "e");
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

    if (basename($wordconvpath) =~ /wvhtml/i) {
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

    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.word');
    { 
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }

    # Check version of word document (greater than word8 or else).
    if (util::islang("ja")) {
	my $docversion = "unknown";
	my $supported = 0;
	my @cmd = ($wvversionpath, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $result = util::readfile($fh_out);
	if ($result =~ /^Version: (word\d+)(?:,| )/i) {
	    $docversion = $1;
	    # Only word8 format is supported for Japanese.
	    $supported = 1 if ($docversion =~ /^word8$/);
	}
	unless ($supported) {
	    return _("Unsupported format: ") .  $docversion;
	}
    }

    # Check version of wvWare (greater than 0.7 or else).
    my $tmpfile2 = util::tmpnam('NMZ.word2');
    my ($ofile, $tpath) = ("", "");
    {
	my @cmd = ($wordconvpath, "--version");
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $result = util::readfile($fh_out);
	if ($result ne "" and $result !~ /usage/i and $result ge "0.7") {
            ($ofile, $tpath) = fileparse($tmpfile2);
            @wordconvopts = ("--targetdir=$tpath");
	} else {
	    $ofile = $tmpfile2;
	    @wordconvopts = ();
	}
    }

    util::systemcmd($wordconvpath, @wordconvopts, $tmpfile, $ofile);
    unless (-e $tmpfile2) {
	return "Unable to convert file ($wordconvpath error occurred).";
    } else {
	my $fh = util::efopen("< $tmpfile2");
	$$cont = util::readfile($fh);
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
	my @cmd = ($utfconvpath, "-Iu8", "-Oej", $tmpfile2);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($utfconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large word file';
	}
	$$cont = util::readfile($fh_out);
        codeconv::normalize_eucjp($cont);
    }

    unlink $tmpfile;
    unlink $tmpfile2;

    # Exclude wvHtml's footer because it has no good index terms.
    $$cont =~ s/<!--Section Ends-->.*$//s;

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
    
    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.word');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($wordconvpath, @wordconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($wordconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large word file.';
	}
        $$cont = util::readfile($fh_out);
    }
    unlink $tmpfile;

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
