#
# -*- Perl -*-
# $Id: postscript.pl,v 1.9 2002-09-23 08:52:32 baba Exp $
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

my $psconvpath = undef;
my @psconvopts = undef;
my $envpath = undef;

sub mediatype() {
    return ('application/postscript');
}

sub status() {
    if (util::islang("ja")) {
	$psconvpath = util::checkcmd('ps2text');
    } else {
	$psconvpath = util::checkcmd('ps2ascii');
    }
    $envpath = util::checkcmd('env');
    return 'no' unless (defined $psconvpath && defined $envpath);
    return 'yes';
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing postscript file ... (using  '$psconvpath')\n");

    my $tmpfile = util::tmpnam('NMZ.postscript');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$cont;
    }
    {
	my $landscape = 0;
	my $fh = util::efopen("< $tmpfile");
	while (<$fh>) {
	    last if (/^%%EndComments$/);
	    $landscape = 1 if (/^%%Orientation: Landscape$/i);
	}
	undef $fh;
	if (util::islang("ja") && $landscape) {
	    @psconvopts = ("-l");
	} else {
	    @psconvopts = ();
	}
    }
    {
	my @env = ($envpath, "LANG=$util::LANG");
	my @cmd = (@env, $psconvpath, @psconvopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert postscript file ($psconvpath error occurred)";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large postscript file';
	}
	$$cont = util::readfile($fh_out);
    }
    unlink $tmpfile;

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


