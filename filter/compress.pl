#
# -*- Perl -*-
# $Id: compress.pl,v 1.19 2004-02-22 10:59:00 opengl2772 Exp $
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

package compress;
use strict;
require 'util.pl';

my $zcatpath = undef;
my @zcatopts = undef;

sub mediatype() {
    return ('application/x-compress');
}

sub status() {
    $zcatpath = util::checkcmd('zcat');
    @zcatopts = ("-cd");
    return 'no' unless (defined $zcatpath);
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
	= @_;

    util::vprint("Processing compress file ... (using  '$zcatpath')\n");

    my $tmpfile = util::tmpnam('NMZ.compr');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my @cmd = ($zcatpath, @zcatopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
            unlink $tmpfile;
	    return "Unable to convert file ($zcatpath error occurred)";
	}
	if ($size > $conf::FILE_SIZE_MAX) {
            unlink $tmpfile;
	    return 'Too large compressed file';
	}
	$$cont = util::readfile($fh_out);
    }
    unlink $tmpfile;

    return undef;
}

1;
