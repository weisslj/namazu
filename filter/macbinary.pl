#
# -*- Perl -*-
# $Id: macbinary.pl,v 1.1 2003-08-01 07:31:53 opengl2772 Exp $
# Copyright (C) 2003 Namazu Project All rights reserved ,
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

package macbinary;
use strict;
require 'util.pl';

sub mediatype() {
    return ('application/macbinary');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 1;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv () {
    return 0;
}

sub add_magic ($) {
    my ($magic) = @_;

    $magic->addMagicEntry("0\tubyte\t0\t");
    $magic->addMagicEntry(">74\tubyte\t0\t");

#   MacBinaryIII
    $magic->addMagicEntry(">>122\tubyte\t130\t");
    $magic->addMagicEntry(">>>102\tstring\tmBIN\tapplication/macbinary");

#   MacBinaryII
    $magic->addMagicEntry(">>122\tubyte\t129\t");
    $magic->addMagicEntry(">>>123\tubyte\t129\t");
    $magic->addMagicEntry(">>>>102\tbelong\t0\tapplication/macbinary");

#   MacBinaryI
    $magic->addMagicEntry(">>122\tubyte\t0\t");
    $magic->addMagicEntry(">>>123\tubyte\t0\t");
    $magic->addMagicEntry(">>>>102\tbelong\t0\tapplication/macbinary");

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;

    util::vprint("Processing macbinary file ...\n");

    # 083 Data Fork Length
    my $size = unpack("N", substr($$cont, 83, 4));

    if ($size > $conf::FILE_SIZE_MAX) {
        return 'too_large_macbinary_file';
    }

    # 128-byte Header Size
    my $datafork = substr($$cont, 128, $size);
    $$cont = $datafork;

    return undef;
}

1;
