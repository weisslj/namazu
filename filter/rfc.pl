#
# -*- Perl -*-
# $Id: rfc.pl,v 1.1 1999-08-28 00:46:51 satoru Exp $
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

package rfc;
use strict;
require 'util.pl';
#require 'filter.pl';

sub mediatype() {
    return ('text/plain; x-type=rfc');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub filter ($$$$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields, $size)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    print "Proccessing rfc file ...\n"
      if ($conf::VerboseOpt);

    rfc_filter($cont, $weighted_str, $fields);

    filter::line_adjust_filter($cont) unless $conf::NoLineAdOpt;
    filter::line_adjust_filter($weighted_str) unless $conf::NoLineAdOpt;
    filter::white_space_adjust_filter($cont);
    $fields->{title} = filter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    filter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

# RFC 用のフィルタ
# わりと書式はまちまちみたいだからそれなりに

sub rfc_filter ($$$) {
    my ($contents, $weighted_str, $fields) = @_;

    $$contents =~ s/^\s+//s;
    $$contents =~ s/((.+\n)+)\s+(.*)//;
    my $title = $fields->{title};
    $title = $3 if defined $3;
    html::encode_entity($title);
    $fields->{title} = $title;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n" if defined $1;
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";
    # summary または Introductionがあればそれを先頭に持ってくる
#    $$contents =~ s/\A(.+?^(\d+\.\s*)?(Abstract|Introduction)\n\n)//ims;
    $$contents =~ s/([\s\S]+^(\d+\.\s*)?(Abstract|Introduction)\n\n)//im;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n" if defined $1;
}

1;
