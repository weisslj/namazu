#
# -*- Perl -*-
# $Id: msword.pl,v 1.52 2004-10-17 17:42:41 opengl2772 Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi,
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

package msword;
use strict;
use File::Basename;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';
eval 'require NKF;';

my $perlver = $];
my $wordconvpath  = undef;
my @wordconvopts  = undef;
my $wvversionpath = undef;
my $utfconvpath   = undef;
my $convname = undef;
my $wvsummarypath = undef;

sub mediatype() {
    return ('application/msword');
}

sub status() {
    # The check of a dependence filter.
    return 'no' if (html::status() ne 'yes');

    $wvsummarypath = util::checkcmd('wvSummary');
    $wvversionpath = util::checkcmd('wvVersion');

    $wordconvpath = util::checkcmd('wvWare');
    if (defined $wordconvpath) {
        my @cmd = ($wordconvpath, "--version");
        my $fh_out = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_out,
                "stderr" => "/dev/null",
            },
        );
        my $version = util::readfile($fh_out, "t");
        util::fclose($fh_out);

        if ($version =~ s/wvWare (\d\.\d)\.(\d).*/$1$2/) {
            if ($version >= 0.75) {
                if (!util::islang("ja")) {
                    return 'yes';
                } else {
                    my $nkfversion = 0.00;
                    eval '$nkfversion = $NKF::VERSION;';
                    $utfconvpath   = util::checkcmd('lv');
                    if (defined $wvversionpath
                    && (defined $utfconvpath || $perlver >= 5.008
                        || $nkfversion >= 2.04)) {
                        return 'yes';
                    }
                }
            }
        }
    }

    $wordconvpath = util::checkcmd('wvHtml');
    if (defined $wordconvpath) {
        if (!util::islang("ja")) {
	    return 'yes';
        } else {
            my $nkfversion = 0.00;
            eval '$nkfversion = $NKF::VERSION;';
            $utfconvpath   = util::checkcmd('lv');
            if (defined $wvversionpath
            && (defined $utfconvpath || $perlver >= 5.008
                || $nkfversion >= 2.04)) {
                return 'yes';
            }
        }
    }

    $wordconvpath = util::checkcmd('doccat');
    @wordconvopts = ("-o", "e");
    return 'yes' if defined $wordconvpath;
    return 'no';
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

    $convname = basename($wordconvpath) unless (defined $convname);

    if ($convname =~ /(?:wvWare|wvHtml)/i) {
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
    my $err = undef;

    util::vprint("Processing ms-word file ... (using  '$wordconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.word');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
        util::fclose($fh);
    }

    # Check version of word document (greater than word7,8 or else).
    if (util::islang("ja")) {
        my $docversion = getDocumentVersion($tmpfile);
	if ($docversion !~ /^word[78]$/) {
            unlink $tmpfile;
	    return _("Unsupported format: ") . $docversion;
	}
    }

    # get summary info in all OLE documents.
    getSummaryInfo($tmpfile, $cont, $weighted_str, $headings, $fields);
    my $title = $fields->{'title'};


    if (basename($wordconvpath) =~ /wvWare/i) {
        $err = filter_wvWare($tmpfile, $cont, $weighted_str, $headings, $fields);
    } else {
        $err = filter_wvHtml($tmpfile, $cont, $weighted_str, $headings, $fields);
    }
    unlink $tmpfile;
    return $err if (defined $err);


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

sub filter_wvWare ($$$$$) {
    my ($cfile, $cont, $weighted_str, $headings, $fields)
	= @_;

    my $tmpfile2 = util::tmpnam('NMZ.word2');
    my ($ofile, $tpath) = ("", "");
    ($ofile, $tpath) = fileparse($tmpfile2);
    my $name = basename($ofile);

    if (util::islang("ja")) {
        @wordconvopts = (
            "--nographics", "-d", "$tpath", "-b", "$name", 
            "--charset=EUC-JP"
        );
    } else {
        @wordconvopts = (
            "--nographics", "-d", "$tpath", "-b", "$name"
        );
    }

    my @cmd = ($wordconvpath, @wordconvopts, $cfile);
    my $fh_out = IO::File->new_tmpfile();
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $fh_out,
            "stderr" => "/dev/null",
        },
    );
    my $size = util::filesize($fh_out);
    if ($size == 0) {
        util::fclose($fh_out);
        unlink($tmpfile2);
        return "Unable to convert file ($wordconvpath error occurred).";
    }
    if ($size > $conf::TEXT_SIZE_MAX) {
        util::fclose($fh_out);
        unlink($tmpfile2);
        return 'Too large word file.';
    }
    $$cont = util::readfile($fh_out, "t");
    util::fclose($fh_out);
    unlink($tmpfile2);

    if (util::islang("ja")) {
        # Title shoud be removed.
        $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is;

        # div name shoud be removed.
        $$cont =~ s!(<div(?:\s[A-Z]+\w*(?:=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*))?)*)\s+name=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*)(\s[A-Z]+\w*(?:=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*))?\s*>)!$1$2!igs;

        codeconv::normalize_eucjp($cont);
    }

    return undef;
}

sub filter_wvHtml ($$$$$) {
    my ($cfile, $cont, $weighted_str, $headings, $fields)
	= @_;

    # Check version of wvWare (greater than 0.7 or else).
    my $tmpfile2 = util::tmpnam('NMZ.word2');
    my ($ofile, $tpath) = ("", "");
    {
	my @cmd = ($wordconvpath, "--version");
        my $fh_out = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_out,
                "stderr" => "/dev/null",
            },
        );
	my $result = util::readfile($fh_out, "t");
        util::fclose($fh_out);
	if ($result ne "" and $result !~ /usage/i and $result ge "0.7") {
            ($ofile, $tpath) = fileparse($tmpfile2);
            @wordconvopts = ("--targetdir=$tpath");
	} else {
            if (util::islang("ja")) {
                my $docversion = getDocumentVersion($cfile);
                return _("Unsupported format: ") . $docversion
                    if ($docversion =~ /^word7$/i);
            }

	    $ofile = $tmpfile2;
	    @wordconvopts = ();
	}
    }

    my @cmd = ($wordconvpath, @wordconvopts, $cfile, $ofile);
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => "/dev/null",
            "stderr" => "/dev/null",
        },
    );
    unless (-e $tmpfile2) {
	return "Unable to convert file ($wordconvpath error occurred).";
    }
    my $size = util::filesize($tmpfile2);
    if ($size == 0) {
        unlink $tmpfile2;
        return "Unable to convert file ($wordconvpath error occurred).";
    }
    if ($size > $conf::TEXT_SIZE_MAX) {
        unlink $tmpfile2;
        return 'Too large word file.';
    }

    {
	$$cont = util::readfile($tmpfile2, "t");
    }
    unlink $tmpfile2;

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        # Title shoud be removed.
        $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is;

        # div name shoud be removed.
        $$cont =~ s!(<div(?:\s[A-Z]+\w*(?:=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*))?)*)\s+name=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*)(\s[A-Z]+\w*(?:=(?:"(?:\\"|[^"])*"|'(?:\\'|[^'])*'|[^\s>]*))?\s*>)!$1$2!igs;

        utf8_to_eucjp($cont);
    }

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
        util::fclose($fh);
    }
    {
	my @cmd = ($wordconvpath, @wordconvopts, $tmpfile);
        my $fh_out = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_out,
                "stderr" => "/dev/null",
            },
        );
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            util::fclose($fh_out);
            unlink $tmpfile;
	    return "Unable to convert file ($wordconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
            util::fclose($fh_out);
            unlink $tmpfile;
	    return 'Too large word file.';
	}
        $$cont = util::readfile($fh_out, "t");
        util::fclose($fh_out);
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

sub getDocumentVersion ($) {
    my ($cfile) = @_;
    my $docversion = "unknown";

    # Check version of word document (greater than word7,8 or else).
    if (util::islang("ja")) {
        my @cmd = ($wvversionpath, $cfile);
        my $fh_out = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_out,
                "stderr" => "/dev/null",
            },
        );
        my $result = util::readfile($fh_out, "t");
        util::fclose($fh_out);
        if ($result =~ /^Version: (word\d+)(?:,| )/i) {
            $docversion = $1;
        }
    }
 
    return $docversion;
}

sub getSummaryInfo ($$$$$) {
    my ($cfile, $cont, $weighted_str, $headings, $fields)
        = @_;

    return undef unless (defined $wvsummarypath);

    my @cmd = ($wvsummarypath, $cfile);
    my $fh_out = IO::File->new_tmpfile();
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $fh_out,
            "stderr" => "/dev/null",
        },
    );
    my $summary = util::readfile($fh_out, "t");
    my $orgsummary = $summary;

    my $size = util::filesize($fh_out);
    util::fclose($fh_out);

    if ($size == 0) {
        return undef;
    }
    if ($size > $conf::TEXT_SIZE_MAX) {
        return 'Too large summary file.';
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

    if ($perlver >= 5.008){
        eval 'use Encode qw/from_to Unicode JP/;';
        Encode::from_to($$cont, "utf-8" ,"euc-jp");
        codeconv::normalize_eucjp($cont);
        return undef;
    }

    if ($var::USE_NKF_MODULE) {
        if ($NKF::VERSION >= 2.04) {
            $$cont = NKF::nkf("-WemXZ1", $$cont);
            return undef;
        }
    }
    return undef unless (defined $utfconvpath);

    my $tmpfile  = util::tmpnam('NMZ.tmp.utf8');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
        util::fclose($fh);
    }

    my @cmd = ($utfconvpath, "-Iu8", "-Oej", $tmpfile);
    my $fh_out = IO::File->new_tmpfile();
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $fh_out,
            "stderr" => "/dev/null",
        },
    );
    $$cont = util::readfile($fh_out, "t");
    util::fclose($fh_out);

    codeconv::normalize_eucjp($cont);

    unlink $tmpfile;

    return undef;
}

1;
