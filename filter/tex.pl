#
# -*- Perl -*-
# $Id: tex.pl,v 1.1 1999-12-30 13:16:34 satoru Exp $
# Copyright (C) 1999 Satoru Takabayashi ,
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

package tex;
use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/x-tex');
}

sub status() {
    my $texconvpath = util::checkcmd('detex');
    return 'yes' if (defined $texconvpath);
    return 'no';
}

sub recursive() {
    return 0;
}

sub codeconv() {
    return 1;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile = util::tmpnam('NMZ.tex');
    my $texconvpath = util::checkcmd('detex');
    return "Unable to execute tex-converter" unless (-x $texconvpath);
    util::vprint("Processing tex file ... (using  '$texconvpath')\n");

    {
	my $fh = util::efopen("| $texconvpath > $tmpfile");
	print $fh $$cont;
    }

    {
	my $fh = util::efopen("< $tmpfile");
	$$cont = util::readfile($fh);
    }

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
