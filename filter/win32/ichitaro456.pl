#
# -*- Perl -*-
# $Id: ichitaro456.pl,v 1.1 2000-03-01 03:20:21 satoru Exp $
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
use strict;
require 'util.pl';
require 'gfilter.pl';

my $ichitaro456 = undef;

sub mediatype() {
    return ('application/ichitaro4', 'application/ichitaro5', 'application/ichitaro6');
}

sub status() {
    $ichitaro456 = util::checkcmd('jstxt');
    return 'yes' if (defined $ichitaro456);
    return 'no';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv () {
    return 1;
}

sub add_magic ($) {
    my ($magic) = @_;
    $magic->addFileExts('(?i)\\.jsw', 'application/ichitaro4');
    $magic->addFileExts('(?i)\\.jaw', 'application/ichitaro5');
    $magic->addFileExts('(?i)\\.jbw', 'application/ichitaro6');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.jstxt');

    return "Unable to execute ichitaro-converter" unless (-x $ichitaro456);

    util::vprint("Processing ichitaro file ... (using  '$ichitaro456')\n");

    system("$ichitaro456 -k -s -p $$orig_cfile > $tmpfile");
    my $fh = util::efopen("< $tmpfile");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);

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
