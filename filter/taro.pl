#
# -*- Perl -*-
# $Id: taro.pl,v 1.9 2003-08-03 04:00:09 opengl2772 Exp $
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

package taro;
use strict;
require 'util.pl';
require 'gfilter.pl';

my $taroconvpath = undef;
my @taroconvopts = undef;

sub mediatype() {
    # File::MMagic detects Ichitaro 6 document as `application/ichitaro6'
    return qw(
	application/x-js-taro
	application/ichitaro6
    );
}

sub status() {
    $taroconvpath = util::checkcmd('doccat');
    @taroconvopts = ("-o", "e");
    return 'yes' if defined $taroconvpath;
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

    # Ichitaro 6, 7
    $magic->addFileExts('\\.j[bf]w$', 'application/x-js-taro');
    # Ichitaro 8, 9, 10
    $magic->addFileExts('\\.jt[dt]$', 'application/x-js-taro');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing taro file ... (using '$taroconvpath')\n");

    my $tmpfile  = util::tmpnam('NMZ.taro');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($taroconvpath, @taroconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($taroconvpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large taro file.';
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
