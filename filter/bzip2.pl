#
# -*- Perl -*-
# $Id: bzip2.pl,v 1.9 2000-01-06 10:01:48 satoru Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu All rights reserved.
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

package bzip2;
use strict;
require 'util.pl';

sub mediatype() {
    return ('application/x-bzip2');
}

sub status() {
    my $bzip2path = util::checkcmd('bzip2');
    return 'no' unless (defined $bzip2path);
    return 'yes';
}

sub recursive() {
    return 1;
}

sub codeconv() {
    return 0;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;

    my $tmpfile = util::tmpnam('NMZ.bzip2');
    my $bzip2path = util::checkcmd('bzip2');
    return "Unable to execute bzip2" unless (-x $bzip2path);

    util::vprint("Processing bzip2 file ... (using  '$bzip2path')\n");

    my $fh = util::efopen("|$bzip2path -cd > $tmpfile");
    print $fh $$cont;
    undef $fh;
    $fh = util::efopen("$tmpfile");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);
    return undef;
}

1;
