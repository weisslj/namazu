#
# -*- Perl -*-
# $Id: excel.pl,v 1.22 2004-03-22 06:22:09 opengl2772 Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu, 
#               2000-2004 Namazu Project All rights reserved.
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
use File::Basename;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $xlconvpath  = undef;
my @xlconvopts  = undef;
my $utfconvpath = undef;
my $wvsummarypath = undef;

sub mediatype() {
    return ('application/excel');
}

sub status() {
    $wvsummarypath = util::checkcmd('wvSummary');

    $xlconvpath = util::checkcmd('xlhtml') || util::checkcmd('xlHtml');
#    return 'no' unless defined $xlconvpath;
    if (defined $xlconvpath) {
	@xlconvopts = ("-m");
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
        $xlconvpath = util::checkcmd('doccat');
	@xlconvopts = ("-o", "e");
        return 'yes' if defined $xlconvpath;
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

    $magic->addFileExts('\\.xls$', 'application/excel');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;
    my $err = undef;

    if (basename($xlconvpath) =~ /xlhtml/i) {
	$err = filter_xl($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else {
	$err = filter_doccat($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }
    return $err;
}

sub filter_xl ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing ms-excel file ... (using  '$xlconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.excel');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
        util::fclose($fh);
    }

    # get summary info in all OLE documents.
    getSummaryInfo($tmpfile, $cont, $weighted_str, $headings, $fields);
    my $title = $fields->{'title'};

    # -m: No encoding for multibyte. It's necessary to
    # handle a Japanese Excel 5.0 or 95 document correctly.
    {
	my @cmd = ($xlconvpath, @xlconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            util::fclose($fh_out);
            util::fclose($fh_err);
            unlink $tmpfile;
	    return "Unable to convert file ($xlconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
            util::fclose($fh_out);
            util::fclose($fh_err);
            unlink $tmpfile;
	    return 'Too large excel file';
	}
	$$cont = util::readfile($fh_out);
        util::fclose($fh_out);
        util::fclose($fh_err);
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
	my $encoding = "u8"; # UTF-8
	# Pattern for xlHtml version 0.2.6.
	if ($$cont =~ m!^<FONT SIZE="?-1"?><I>Last Updated(&nbsp;using| with) Excel 5.0 or 95</I></FONT><br>$!m) 
	{
	    $encoding = "s"; # Shift_JIS
	}
	{
	    my $fh = util::efopen("> $tmpfile");
	    print $fh $$cont;
            util::fclose($fh);
	}
	{
	    my @cmd = ($utfconvpath, "-I$encoding", "-Oej", $tmpfile);
	    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	    my $size = util::filesize($fh_out);
	    if ($size == 0) {
                util::fclose($fh_out);
                util::fclose($fh_err);
                unlink $tmpfile;
		return "Unable to convert file ($xlconvpath error occurred)";
	    }
	    if ($size > $conf::TEXT_SIZE_MAX) {
                util::fclose($fh_out);
                util::fclose($fh_err);
                unlink $tmpfile;
		return 'Too large excel file';
	    }
	    $$cont = util::readfile($fh_out);
            util::fclose($fh_out);
            util::fclose($fh_err);
            codeconv::normalize_eucjp($cont);
	}
    } 

    unlink $tmpfile;

    # Extract the author and exclude xlHtml's footer at once.
    $$cont =~ s!^<FONT SIZE="?-1"?><I>Spreadsheet's Author:&nbsp;(.*?)</I></FONT><br>.*!!ms;  # '
    if (defined $1) {
        $fields->{'author'} = $1 unless (defined $fields->{'author'});
    }

    # Title shoud be removed.
    # Because xlHtml generate poor <TITLE>/foo/bar/NMZ.excel.tmp</TITLE>.
    $$cont =~ s!<TITLE.*?>.*?</TITLE>!!si;

    html::html_filter($cont, $weighted_str, $fields, $headings);
    $fields->{'title'} = $title if (defined $title);

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

    util::vprint("Processing ms-excel file ... (using  '$xlconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.excel');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
        util::fclose($fh);
    }
    {
	my @cmd = ($xlconvpath, @xlconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            util::fclose($fh_out);
            util::fclose($fh_err);
            unlink $tmpfile;
	    return "Unable to convert file ($xlconvpath error occurred)";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
            util::fclose($fh_out);
            util::fclose($fh_err);
            unlink $tmpfile;
	    return 'Too large excel file.';
	}
        $$cont = util::readfile($fh_out);
        util::fclose($fh_out);
        util::fclose($fh_err);
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

sub getSummaryInfo ($$$$$) {
    my ($cfile, $cont, $weighted_str, $headings, $fields)
        = @_;

    return undef unless (defined $wvsummarypath);

    my @cmd = ($wvsummarypath, $cfile);
    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
    my $summary = util::readfile($fh_out);
    my $orgsummary = $summary;

    my $size = util::filesize($fh_out);
    util::fclose($fh_out);
    util::fclose($fh_err);
    if ($size == 0) {
        return undef;
    }
    if ($size > $conf::TEXT_SIZE_MAX) {
        return 'Too large word file';
    }

    # Codepage
    #   932 : 0x000003a4 : Shift_JIS
    # 10001 : 0x00002711 : x-mac-japanese
    # 65001 : 0xfffffde9 : UTF-8


    my $codepage = "000003a4"; # Shift_JIS
    my $title = undef;
    my $subject = undef;
    my $lastauthor = undef;
    my $author = undef;
    my $keywords = undef;

    if ($summary =~ /^Codepage is 0x([0-9a-f]*)/m) {
        $codepage = sprintf("%8.8x", hex($1));
    }

    if ($codepage eq "fffffde9") { 
        utf8_to_eucjp(\$summary);
    } else {
        codeconv::toeuc(\$summary);
    }

    if ($summary =~ /^The title is (.*)$/m) {
        my $orgtitle;

        $title = $1;

        # PowerPoint Only
#        $orgsummary =~ /^The title is (.*)$/m;
#        $orgtitle = $1;
#        undef $title if $orgtitle eq # which has no slide title
#            "\xbd\xd7\xb2\xc4\xde\x20\xc0\xb2\xc4\xd9\x82\xc8\x82\xb5";
    }
    if ($summary =~ /^The subject is (.*)$/m) {
        $subject = $1;
    }
    if ($summary =~ /^The last author was (.*)$/m) {
        $lastauthor = $1;
    }
    if ($summary =~ /^The author is (.*)$/m) {
        $author = $1;
    }
    if ($summary =~ /^The keywords are (.*)$/m) {
        $keywords = $1;
    }

    my $weight = $conf::Weight{'html'}->{'title'};
    if (defined $title) {
        $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";
    }
    if (defined $subject) {
        $$weighted_str .= "\x7f$weight\x7f$subject\x7f/$weight\x7f\n";
    }

    $fields->{'title'} = $title;
    $fields->{'title'} = $subject unless (defined $title);

    $fields->{'author'} = $lastauthor;
    $fields->{'author'} = $author unless (defined $lastauthor);

    if (defined $keywords) {
        $weight = $conf::Weight{'metakey'};
        $$weighted_str .= "\x7f$weight\x7f$keywords\x7f/$weight\x7f\n";
    }

    return undef;
}

sub utf8_to_eucjp($) {
    my ($cont) = @_;

    return undef unless (util::islang("ja"));
    return undef unless (defined $utfconvpath);

    my $tmpfile  = util::tmpnam('NMZ.tmp.utf8');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
        util::fclose($fh);
    }

    my @cmd = ($utfconvpath, "-Iu8", "-Oej", $tmpfile);
    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
    $$cont = util::readfile($fh_out);
    util::fclose($fh_out);
    util::fclose($fh_err);
    codeconv::normalize_eucjp($cont);

    unlink $tmpfile;

    return undef;
}

1;
