#
# -*- Perl -*-
# $Id: msword.pl,v 1.44 2004-02-22 10:59:00 opengl2772 Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000-2004 Namazu Project All rights reserved.
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
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $wordconvpath  = undef;
my @wordconvopts  = undef;
my $wvversionpath = undef;
my $utfconvpath   = undef;
my $wvsummarypath = undef;

sub mediatype() {
    return ('application/msword');
}

sub status() {
    $wvsummarypath = util::checkcmd('wvSummary');

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
    my $docversion = "unknown";

    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.word');
    { 
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
        $fh->close();
    }

    # get summary info in all OLE documents.
    getSummaryInfo($tmpfile, $cont, $weighted_str, $headings, $fields);
    my $title = $fields->{'title'};

    # Check version of word document (greater than word7,8 or else).
    if (util::islang("ja")) {
	my $supported = 0;
	my @cmd = ($wvversionpath, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $result = util::readfile($fh_out);
	if ($result =~ /^Version: (word\d+)(?:,| )/i) {
	    $docversion = $1;
	    # Only word7,8 format is supported for Japanese.
	    $supported = 1 if ($docversion =~ /^word[78]$/);
	}
	unless ($supported) {
            unlink $tmpfile;
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
            if (util::islang("ja")) {
                unlink $tmpfile;
                unlink $tmpfile2;
                return _("Unsupported format: ") .  $docversion
                    if ($docversion =~ /^word7$/);
            }

	    $ofile = $tmpfile2;
	    @wordconvopts = ();
	}
    }

    util::systemcmd($wordconvpath, @wordconvopts, $tmpfile, $ofile);
    unless (-e $tmpfile2) {
        unlink $tmpfile;
	return "Unable to convert file ($wordconvpath error occurred).";
    } else {
	my $fh = util::efopen("< $tmpfile2");
	$$cont = util::readfile($fh);
        $fh->close();
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        if ($docversion =~ /^word7$/) {
            # Title shoud be removed.
            $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is;
        }

        # div name shoud be removed.
        $$cont =~ s!(<div(?:\s[A-Z]+\w*(?:=(?:".*?"|'.*?'|[^\s>]*))?)*)\s+name=(?:".*?"|'.*?'|[^\s>]*)(\s[A-Z]+\w*(?:=(?:".*?"|'.*?'|[^\s>]*))?\s*>)!$1$2!igs;

        {
            my $fh = util::efopen("> $tmpfile2");
            print $fh $$cont;
            $fh->close();
        }

	my @cmd = ($utfconvpath, "-Iu8", "-Oej", $tmpfile2);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            unlink $tmpfile;
            unlink $tmpfile2;
	    return "Unable to convert file ($utfconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
            unlink $tmpfile;
            unlink $tmpfile2;
	    return 'Too large word file';
	}
	$$cont = util::readfile($fh_out);
        codeconv::normalize_eucjp($cont);
    }

    unlink $tmpfile;
    unlink $tmpfile2;

    # Title shoud be removed.
    $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is
        if ((defined $title) or
        (util::islang("ja") && $docversion =~ /^word7$/));

    # Exclude wvHtml's footer because it has no good index terms.
    $$cont =~ s/(.*)<!--Section Ends-->.*$/$1/s;

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
    
    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.word');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
        $fh->close();
    }
    {
	my @cmd = ($wordconvpath, @wordconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            unlink $tmpfile;
	    return "Unable to convert file ($wordconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
            unlink $tmpfile;
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

sub getSummaryInfo ($$$$$) {
    my ($cfile, $cont, $weighted_str, $headings, $fields)
        = @_;

    return undef unless (defined $wvsummarypath);

    my @cmd = ($wvsummarypath, $cfile);
    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
    my $summary = util::readfile($fh_out);
    my $orgsummary = $summary;

    my $size = util::filesize($fh_out);
    $fh_out->close();
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
        $fh->close();
    }

    my @cmd = ($utfconvpath, "-Iu8", "-Oej", $tmpfile);
    my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
    $$cont = util::readfile($fh_out);
    codeconv::normalize_eucjp($cont);

    unlink $tmpfile;

    return undef;
}

1;
