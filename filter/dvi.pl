#
# -*- Perl -*-
# $Id: dvi.pl,v 1.3 2002-09-23 08:52:32 baba Exp $
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

my $dviconvpath = undef;
my @dviconvopts = undef;
my $envpath = undef;
my @env = undef;

sub mediatype() {
    return ('application/x-dvi');
}

sub status() {
    $dviconvpath = util::checkcmd('dvi2tty');
    $envpath = util::checkcmd('env');
    @env = ($envpath, "DVI2TTY=");
    return 'no' unless (defined $dviconvpath && defined $envpath);
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

    util::vprint("Processing dvi file ... (using '$dviconvpath')\n");

    if (util::islang("ja")) {
	# -J option: dvi2tty-5.1 for Debian
	@dviconvopts = ("-q", "-J");
    } else {
	@dviconvopts = ("-q");
    }

    my $tmpfile = util::tmpnam('NMZ.dvi');
    {
	# note that dvi2tty need suffix .dvi
	my $fh = util::efopen("> $tmpfile.dvi");
	print $fh $$cont;
    }
    {
	my @cmd = (@env, $dviconvpath, @dviconvopts, "$tmpfile.dvi");
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($dviconvpath error occurred)";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large dvi file';
	} 
	$$cont = util::readfile($fh_out);
    }
    unlink("$tmpfile.dvi");

    # post_codeconv() does not work... (?_?)
    if (util::islang("ja")) {
	codeconv::toeuc($cont);
    }

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
