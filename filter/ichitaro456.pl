#
# -*- Perl -*-
# $Id: ichitaro456.pl,v 1.1 1999-11-14 22:59:48 kenzo- Exp $
# Copyright (C) 1999 Ken-ichi Hirose All rights reserved.
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
#  You need "jstxt.exe" command.
#

package ichitaro456;
#use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/ichitaro4', 'application/ichitaro5', 'application/ichitaro6');
}

sub status() {
    my $ichitaro456 = util::checkcmd('jstxt');
    return 'yes' if (defined $ichitaro456);
    return 'no';
}

sub recursive() {
    return 0;
}

sub codeconv() {
    return 0;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.jstxt');
    my $tmpfile2 = util::tmpnam('NMZ.jstxt2');

    my $ichitaro456 = util::checkcmd('jstxt');
    return "Unable to execute ichitaro-converter" unless (-x $ichitaro456);

    util::vprint("Processing ichitaro file ... (using  '$ichitaro456')\n");

    my $fh = util::efopen("> $tmpfile");
    print $fh $$cont;
    undef $fh;

    system("$ichitaro456 -k $tmpfile > $tmpfile2");
    $fh = util::efopen("< $tmpfile2");
    $$cont = util::readfile($fh);
    undef $fh;
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
