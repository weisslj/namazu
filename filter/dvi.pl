#
# -*- Perl -*-
# $Id: dvi.pl,v 1.2 2001-01-04 01:57:58 baba Exp $
# Copyright (C) 2000 Namazu Project All rights reserved ,
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

package dvi;
use strict;
require 'util.pl';

my $dvipath = undef;
my $nkfpath = undef;

sub mediatype() {
    return ('application/x-dvi');
}

sub status() {
    $dvipath = util::checkcmd('dvi2tty');
    if (util::islang("ja")) {
	$nkfpath = util::checkcmd('nkf');
    }
    return 'no' unless (defined $dvipath);
    return 'yes';
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile = util::tmpnam('NMZ.dvi');
    my $tmpfile2 = util::tmpnam('NMZ.dvi2');

    # note that dvi2tty need suffix .dvi
    my $fh = util::efopen("> $tmpfile.dvi");
    print $fh $$cont;
    undef $fh;

    util::vprint("Processing dvi file ... (using  '$dvipath')\n");
    if (util::islang("ja")) {
	# -J option: dvi2tty-5.1 for Debian
#	system("$dvipath -J -q $tmpfile -o $tmpfile2");
	# ugly: nkf is needed because of prevent 'mojibake' :-(
	system("$dvipath -J -q $tmpfile | $nkfpath -e > $tmpfile2");
    } else {
	system("$dvipath -q $tmpfile -o $tmpfile2");
    }
    unless (-e $tmpfile2) {
	unlink("$tmpfile.dvi");
	unlink($tmpfile2);
	return 'Unable to convert dvi file';
    }

    $fh = util::efopen("$tmpfile2");
    my $size = util::filesize($fh);
    if ($size > $conf::FILE_SIZE_MAX) {
	unlink("$tmpfile.dvi");
	unlink($tmpfile2);
	return 'too_large_dvi_file';
    }
    $$cont = util::readfile($fh);
    undef $fh;
    unlink("$tmpfile.dvi");
    unlink($tmpfile2);
    return undef;
}

1;
