#
# -*- Perl -*-
# $Id: wordpro.pl,v 1.5 2001-04-13 04:50:53 knok Exp $
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

package wordpro;
use strict;
use File::Copy;
require 'util.pl';
require 'gfilter.pl';

my $wordproconvpath  = undef;

sub mediatype() {
    return ('application/x-lotus-wordpro');
}

sub status() {
    $wordproconvpath = util::checkcmd('doccat');
    return 'yes' if defined $wordproconvpath;
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

    $magic->addFileExts('\\.lwp', 'application/x-lotus-wordpro');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.wordpro');
    my $tmpfile2 = util::tmpnam('NMZ.wordpro2');
    copy("$cfile", "$tmpfile2");

    system("$wordproconvpath -o e $tmpfile2 > $tmpfile");

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
