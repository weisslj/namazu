#
# -*- Perl -*-
# $Id: man.pl,v 1.3 1999-08-28 05:55:58 satoru Exp $
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

package man;
use strict;
require 'util.pl';
#require 'filter.pl';

my $TMPFILE = util::tmpnam('NMZ.man');

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

sub filter ($$$$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields, $size)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $roffpath = util::checkcmd('jgroff');
    $roffpath = util::checkcmd('groff') unless (defined $roffpath);
    $roffpath = util::checkcmd('nroff') unless (defined $roffpath);

    my $roffargs;
    $roffargs = '-Tnippon' if ($roffpath =~ /jgroff$/);
    $roffargs = '-Tascii' if ($roffpath =~ /groff$/);
    $roffargs = '' if ($roffpath =~ /nroff$/);

    vprint("Processing man file ... (using  '$roffpath')\n");

    my $fh = util::efopen("|$roffpath -man $roffargs > $TMPFILE");
    print $fh $$cont;
    undef $fh;
    $fh = util::efopen("$TMPFILE");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($TMPFILE);

    man_filter($cont, $weighted_str, $fields);

    filter::line_adjust_filter($cont) unless $var::Opt{NoLineAd};
    filter::line_adjust_filter($weighted_str) unless $var::Opt{NoLineAd};
    filter::white_space_adjust_filter($cont);
    $fields->{title} = filter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    filter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

# man 用のフィルタ
# よくわからないからいいかげんに
sub man_filter ($$$) {
    my ($contents, $weighted_str, $fields) = @_;
    my $name = "";

    # processing like col -b (2byte character acceptable)
    $$contents =~ s/_\x08//g;
    $$contents =~ s/\x08{1,2}([\x20-\x7e]|[\xa1-\xfe]{2})//g;

    $$contents =~ s/^\s+//gs;

    $$contents =~ /^(.*?)\s*\S*$/m;
    my $title = "$1";
    html::encode_entity(\$title);
    $fields->{title} = $title;
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";

    if ($$contents =~ /^(?:NAME|名前|名称)\s*\n(.*?)\n\n/ms) {
	$name = "$1::\n";
	$weight = $conf::Weight{'html'}->{'h1'};
	$$weighted_str .= "\x7f$weight\x7f$1\x7f/$weight\x7f\n";
    }

    if ($$contents =~ 
	s/(.+^(?:DESCRIPTION 解説|DESCRIPTIONS?|SHELL GRAMMAR|INTRODUCTION|【概要】|解説|説明|機能説明|基本機能説明)\s*\n)//ims) 
    {
	$$contents = $name . $$contents;
	$$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    }
}

1;
