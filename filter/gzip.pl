#
# -*- Perl -*-
# $Id: gzip.pl,v 1.7 1999-08-31 04:51:20 knok Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi ,
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

package gzip;
use strict;
require 'util.pl';

sub mediatype() {
    return ('application/x-gzip');
}

sub status() {
    return 'yes' if (util::checklib('Compress/Zlib.pm'));
    my $gzippath = util::checkcmd('gzip');
    return 'yes' unless (defined $gzippath);
    return 'no';
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

    if (util::checklib('Compress/Zlib.pm')) {
	filter_xs($cont);
    } else {
	filter_file($cont);
    }
}

sub filter_file ($) {
    my ($contref) = @_;

    my $tmpfile = util::tmpnam('NMZ.gzip');
    my $gzippath = util::checkcmd('gzip');
    my $fh = util::efopen("|$gzippath -cd > $tmpfile");

    util::vprint("Processing gzip file ... (using  '$gzippath')\n");

    print $fh $$contref;
    undef $fh;
    $fh = util::efopen("$tmpfile");
    $$contref = util::readfile($fh);
    $fh->close();
    unlink($tmpfile);
    return;
}

sub filter_xs ($) {
    my ($contref) = @_;

    util::vprint("Processing gzip file ... (using  Compress::Zlib)\n");

    eval 'use Compress::Zlib;';

    my $offset = 0;
    $offset += 3;
    my $flags = unpack('C', substr($$contref, $offset, 1));
    $offset += 1;
    $offset += 6;
    $$contref = substr($$contref, $offset);
    $$contref = substr($$contref, 2) if ($flags & 0x04);
    $$contref =~ s/^[^\0]*\0// if ($flags & 0x08);
    $$contref =~ s/^[^\0]*\0// if ($flags & 0x10);
    $$contref = substr($$contref, 2) if ($flags & 0x02);

    my $zl = inflateInit(-WindowBits => - MAX_WBITS());
    my ($inf, $stat) = $zl->inflate($$contref);
    if ($stat == Z_OK() or $stat == Z_STREAM_END()) {
	$$contref = $inf;
    } else {
	warn 'Bad compressed data.';
	$$contref = '';
    }

    return;
}

1;
