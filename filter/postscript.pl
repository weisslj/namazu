#
# -*- Perl -*-
# $Id: postscript.pl,v 1.4 2001-01-04 01:57:58 baba Exp $
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

    my $tmpfile = util::tmpnam('NMZ.postscript');
    my $tmpfile2 = util::tmpnam('NMZ.postscript2');

    util::vprint("Processing postscript file ... (using  '$postscriptpath')\n");

    my $fh = util::efopen("> $tmpfile");
    print $fh $$cont;
    undef $fh;

    my $landscape = 0;
    $fh = util::efopen("$tmpfile");
    while (<$fh>) {
	last if (/^%%EndComments$/);
	$landscape = 1 if (/^%%Orientation: Landscape$/i);
    }
    undef $fh;

    if (util::islang("ja")) {
	if ($landscape) {
	    system("$postscriptpath -q -l $tmpfile > $tmpfile2");
	} else {
	    system("$postscriptpath -q $tmpfile > $tmpfile2");
	}
    } else {
	system("$postscriptpath -q $tmpfile > $tmpfile2");
    }
    unless ($? == 0) {
	unlink($tmpfile);
	unlink($tmpfile2);
	return "Unable to convert postscript file ($postscriptpath error occured)";
    }

    $fh = util::efopen("$tmpfile2");
    my $size = util::filesize($fh);
    if ($size == 0) {
	undef $fh;
	unlink($tmpfile);
	unlink($tmpfile2);
	return "Unable to convert postscript file ($postscriptpath error occured)";
    }
    if ($size > $conf::FILE_SIZE_MAX) {
	undef $fh;
	unlink($tmpfile);
	unlink($tmpfile2);
	return 'too_large_postscript_file';
    }
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);
    unlink($tmpfile2);
    return undef;
}

1;


