#
# -*- Perl -*-
# $Id: pipermail.pl,v 1.1 2004-07-20 08:29:56 opengl2772 Exp $
# Copyright (C) 2004 Namazu Project All rights reserved.
#
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

package pipermail;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';
require 'mailnews.pl';

#
# This pattern specifies pipermail's file names.
#
my $PIPERMAIL_MESSAGE_FILE = '\d{6}\.html';

sub mediatype() {
    return ('text/html; x-type=pipermail');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 1;
}

sub post_codeconv () {
    return 0;
}

sub add_magic ($) {
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing pipermail file ...\n");

    unless ($cfile =~ /($PIPERMAIL_MESSAGE_FILE)$/o) 
    {
	return "is Pipermail's index file! skipped."; # error
    } 
    
    pipermail_filter($contref, $weighted_str, $fields);
    html::html_filter($contref, $weighted_str, $fields, $headings);

    $$contref =~ s/^\s+//;
    mailnews::uuencode_filter($contref);
    mailnews::mailnews_filter($contref, $weighted_str, $fields);
    mailnews::mailnews_citation_filter($contref, $weighted_str);

#    undef $$fields{'from'};
    undef $$fields{'author'};

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
			   $fields, $headings);
    return undef;
}

# Assume a normal message files by pipermail (mailman edition)
sub pipermail_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    $$contref =~ s/<!--endarticle-->.*//s;
    if ($$contref =~ s/<H1>(.*)<!--beginarticle-->//s) {
        my $from = $1;
        if ($from =~ m/<b>([^<]*)<\/b>/is) {
            $$fields{'from'} = $1;
        }
    }
}

1;
