#
# -*- Perl -*-
# $Id: rtf.pl,v 1.7 2004-01-16 13:07:10 opengl2772 Exp $
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

package rtf;
use strict;
use File::Basename;
require 'util.pl';
require 'gfilter.pl';

my $rtfconvpath  = undef;
my @rtfconvopts  = undef;

sub mediatype() {
    return ('application/rtf');
}

sub status() {
    #
    # http://www.45.free.net/~vitus/ice/misc/rtf2html.tar.gz
    #
    $rtfconvpath = util::checkcmd('rtf2html');
    if (defined $rtfconvpath) {
	@rtfconvopts = ();
        return 'yes';
    }
    else {
        $rtfconvpath = util::checkcmd('doccat');
        @rtfconvopts = ("-o", "e");
        if (defined $rtfconvpath) {
            my $fh_cmd = util::efopen("$rtfconvpath -V |");
            while (<$fh_cmd>) {
                if (/TF Library *: *Version *: *(\d*)\.(\d*)/i) {
                    my $major = $1;
                    my $minor = $2 . "000";
                    if ($major >= 1 && substr($minor, 0, 2) >= 42) {
                        return 'yes';
                    }
                }
            }
        }
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

    $magic->addFileExts('\\.rtf$', 'application/rtf');

    $magic->addMagicEntry("0\tstring\t{\\rtf\t");
    $magic->addMagicEntry(">6\tstring\t\\ansi\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\mac\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\pc\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\pca\tapplication/rtf");

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;
    my $err = undef;

    if (basename($rtfconvpath) =~ /rtf2html/i) {
        $err = filter_rtf2html($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else {
        $err = filter_doccat($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }
 
    return $err;
}   

sub filter_rtf2html ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing rtf file ... (using '$rtfconvpath')\n");

    # rtf2html : Fixed the problem in [namazu-devel-ja#03530].
    $$cont =~ s/\x5c$//gm;

    my $tmpfile = util::tmpnam('NMZ.rtf');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($rtfconvpath, @rtfconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($rtfconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large rtf file.';
	}
        $$cont = util::readfile($fh_out);
    }
    unlink $tmpfile;

    codeconv::toeuc($cont);

    # Title shoud be removed.
    $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is;

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
    
    util::vprint("Processing rtf file ... (using  '$rtfconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.rtf');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($rtfconvpath, @rtfconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($rtfconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large rtf file.';
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
