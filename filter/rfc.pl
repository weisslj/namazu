#
# -*- Perl -*-
# $Id: rfc.pl,v 1.13 2000-02-06 07:36:42 satoru Exp $
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

package rfc;
use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('text/plain; x-type=rfc');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0; # Need for tranrated document?
}

sub post_codeconv () {
    return 0;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing rfc file ...\n");

    rfc_filter($cont, $weighted_str, $fields);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}

# It's a halway filter because of fuzziness.
sub rfc_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    $$contref =~ s/^\s+//s;
    $$contref =~ s/((.+\n)+)\s+(.*)//;
    my $title = $fields->{'title'};
    $title = $3 if defined $3;
    html::encode_entity(\$title);
    $fields->{'title'} = $title;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n" if defined $1;
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";

    # Get Summary or Introduction and put it at the beginning 
    # for summarization.
#    $$contref =~ s/\A(.+?^(\d+\.\s*)?(Abstract|Introduction)\n\n)//ims;
    $$contref =~ s/([\s\S]+^(\d+\.\s*)?(Abstract|Introduction)\n\n)//im;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n" if defined $1;
}

1;
