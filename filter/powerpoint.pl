#
# -*- Perl -*-
# $Id: powerpoint.pl,v 1.11 2004-01-09 23:01:43 usu Exp $
# Copyright (C) 2000 Ken-ichi Hirose, 
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

package powerpoint;
use strict;
use File::Basename;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $pptconvpath = undef;
my @pptconvopts = undef;
my $utfconvpath = undef;

sub mediatype() {
    return ('application/powerpoint');
}

sub status() {
    $pptconvpath = util::checkcmd('ppthtml') || util::checkcmd('pptHtml');
#    return 'no' unless defined $pptconvpath
    if (defined $pptconvpath) {
	@pptconvopts = ();
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
    } else {
	$pptconvpath = util::checkcmd('doccat');
	@pptconvopts = ("-o", "e");
	return 'yes' if defined $pptconvpath;
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
    my ($magic) = @_;

    $magic->addFileExts('\\.pp[st]$', 'application/powerpoint');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;
    my $err = undef;

    if (basename($pptconvpath) =~ /ppthtml/i) {
	$err = filter_ppt($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else {
	$err = filter_doccat($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }
    return $err;
}

sub filter_ppt ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing powerpoint file ... (using  '$pptconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.powerpoint');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }

    {
	# handle a Japanese PowerPoint document correctly.
	my @cmd = ($pptconvpath, @pptconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	$$cont = util::readfile($fh_out);
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
	my $encoding = "u8"; # UTF-8
	# Pattern for pptHtml
	if ($$cont =~ m!^<FONT SIZE=-1><I>Last Updated&nbsp;using Excel 5.0 or 95</I></FONT><br>$!m) 
	{
	    $encoding = "s"; # Shift_JIS
	}
	{
	    my $fh = util::efopen("> $tmpfile");
	    print $fh $$cont;
	}
	{
	    my @cmd = ($utfconvpath, "-I$encoding", "-Oej", $tmpfile);
	    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	    my $size = util::filesize($fh_out);
	    if ($size == 0) {
		return "Unable to convert file ($utfconvpath error occurred).";
	    }
	    if ($size > $conf::TEXT_SIZE_MAX) {
		return 'Too large powerpoint file';
	    }
	    $$cont = util::readfile($fh_out);
            codeconv::normalize_eucjp($cont);
	}
    } 
    unlink $tmpfile;

    # Extract the author and exclude pptHtml's footer at once.
    $$cont =~ s!^<FONT SIZE=-1><I>Spreadsheet's Author:&nbsp;(.*?)</I></FONT><br>.*!!ms;  # '
    $fields->{'author'} = $1 if defined $1;

    # Title shoud be removed.
    $$cont =~ s!<TITLE>.+</TITLE>!!;

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

    util::vprint("Processing powerpoint file ... (using  '$pptconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.powerpoint');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($pptconvpath, @pptconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($pptconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large powerpoint file.';
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
