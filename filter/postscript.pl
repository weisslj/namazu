#
# -*- Perl -*-
# $Id: postscript.pl,v 1.2 2000-12-28 10:50:48 baba Exp $
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

package postscript;
use strict;
require 'util.pl';

my $postscriptpath = undef;

sub mediatype() {
    return ('application/postscript');
}

sub status() {
    if (util::islang("ja")) {
	$postscriptpath = util::checkcmd('ps2text');
    } else {
	$postscriptpath = util::checkcmd('ps2ascii');
    }
    return 'no' unless (defined $postscriptpath);
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

    my $tmpfile = util::tmpnam('NMZ.postscript');

    util::vprint("Processing postscript file ... (using  '$postscriptpath')\n");

    my $landscape = 0;
    my $fh = util::efopen("$cfile");
    while (<$fh>) {
	last if (/^%%EndComments$/);
	$landscape = 1 if (/^%%Orientation: Landscape$/i);
    }
    undef $fh;

    if (util::islang("ja")) {
	if ($landscape) {
	    $fh = util::efopen("| $postscriptpath -l > $tmpfile");
	} else {
	    $fh = util::efopen("| $postscriptpath > $tmpfile");
	}
    } else {
	$fh = util::efopen("| $postscriptpath > $tmpfile");
    }
    print $fh $$cont;
    undef $fh;
    $fh = util::efopen("$tmpfile");
    my $size = util::filesize($fh);
    if ($size > $conf::FILE_SIZE_MAX) {
	return 'too_large_postscript_file';
    }
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);
    return undef;
}

1;


