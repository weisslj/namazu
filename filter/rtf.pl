#
# -*- Perl -*-
# $Id: rtf.pl,v 1.1 2003-03-20 14:40:54 opengl2772 Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000 Satoru Takabayashi Namazu Project All rights reserved.
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
use File::Copy;
require 'util.pl';
require 'gfilter.pl';

my $rtfconvpath  = undef;
my $doccatversionpath = undef;
my $rtfconvname = undef;

sub mediatype() {
    return ('application/rtf');
}

sub status() {
    $rtfconvpath = util::checkcmd('doccat');

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

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $err = undef;

    if (not defined $rtfconvname) {
	$rtfconvname = basename($rtfconvpath);
    }
    $err = filter_doccat($orig_cfile, $cont, $weighted_str, $headings, $fields);
    return $err;
}   

sub filter_doccat ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
 
    my $tmpfile  = util::tmpnam('NMZ.rtf');
    my $tmpfile2 = util::tmpnam('NMZ.rtf2');   
    copy("$cfile", "$tmpfile2");

    system("$rtfconvpath -o e $tmpfile2 > $tmpfile");

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
