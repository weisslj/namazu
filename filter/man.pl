#
# -*- Perl -*-
# $Id: man.pl,v 1.20 2000-02-13 04:44:37 satoru Exp $
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

package man;
use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('text/x-roff');
}

sub status() {
    my $roffpath = util::checkcmd('jgroff');
    return 'yes' if (defined $roffpath);
    $roffpath = util::checkcmd('groff');
    return 'yes' if (defined $roffpath);
    $roffpath = util::checkcmd('nroff');
    return 'yes' if (defined $roffpath);
    return 'no';
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

    my $tmpfile = util::tmpnam('NMZ.man');
    my $roffpath = util::checkcmd('jgroff');
    $roffpath = util::checkcmd('groff') unless (defined $roffpath);
    $roffpath = util::checkcmd('nroff') unless (defined $roffpath);
    return "Unable to execute nroff/groff/jgroff" unless (-x $roffpath);

    my $roffargs;
    $roffargs = '-Tnippon' if ($roffpath =~ /\bjgroff$/);
    $roffargs = '-Tascii' if ($roffpath =~ /\bgroff$/);
    $roffargs = '' if ($roffpath =~ /nroff$/);

    {
      util::vprint("Processing man file ... (using '$roffpath -man $roffargs')\n");
	my $fh = util::efopen("|$roffpath -man $roffargs > $tmpfile");

	# Make groff output one paragraph per one line.
	# Thanks to Tatsuo SEKINE <tsekine@isoternet.org> for his suggestion.
	print $fh ".ll 100i\n";

	print $fh $$cont;
    }
    {
	my $fh = util::efopen("$tmpfile");
	$$cont = util::readfile($fh);
	unlink($tmpfile);
    }

    codeconv::toeuc($cont);

    man_filter($cont, $weighted_str, $fields);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}

# This is not perfect but works not bad.
sub man_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;
    my $name = "";

    # processing like col -b (2byte character acceptable)
    $$contref =~ s/_\x08//g;
    $$contref =~ s/\x08{1,2}([\x20-\x7e]|[\xa1-\xfe]{2})//g;

    $$contref =~ s/^\s+//gs;

    $$contref =~ /^(.*?)\s*\S*$/m;
    my $title = "$1";
    $fields->{'title'} = $title;
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";

    if ($$contref =~ /^(?:NAME|名前|名称)\s*\n(.*?)\n\n/ms) {
	$name = "$1::\n";
	$weight = $conf::Weight{'html'}->{'h1'};
	$$weighted_str .= "\x7f$weight\x7f$1\x7f/$weight\x7f\n";
    }

    if ($$contref =~ 
	s/\A(.+^(?:DESCRIPTION 解説|DESCRIPTIONS?|SHELL GRAMMAR|INTRODUCTION|【概要】|解説|説明|機能説明|基本機能説明)\s*\n)//ims) 
    {
	$$contref = $name . $$contref;
	$$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    }
}

1;
