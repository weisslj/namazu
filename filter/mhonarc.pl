#
# -*- Perl -*-
# $Id: mhonarc.pl,v 1.16 1999-10-11 04:25:04 satoru Exp $
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

package mhonarc;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';
require 'mailnews.pl';

sub mediatype() {
    return ('text/html; x-type=mhonarc');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub codeconv() {
    return 1;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing MHonArc file ...\n");

    unless ($cfile =~ /($conf::MHONARC_MESSAGE_FILE)$/o) 
    {
	return "is MHonArc's index file! skipped."; # error
    } 
    

    mhonarc_filter($contref, $weighted_str, $fields);
    html::html_filter($contref, $weighted_str, $fields, $headings);

    mailnews::uuencode_filter($contref);
    mailnews::mailnews_filter($contref, $weighted_str, $fields);
    mailnews::mailnews_citation_filter($contref, $weighted_str);

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
			   $fields, $headings);
    return undef;
}

# Assume a normal message files by MHonArc v2.1.0
sub mhonarc_filter ($$) {
    my ($contref, $weighted_str) = @_;

    # It's useful to handle MHonArc message files.
    $$contref =~ s/<!--X-MsgBody-End-->.*//s;
    $$contref =~ s/<!--X-TopPNI-->.*<!--X-TopPNI-End-->//s;
    $$contref =~ s/<!--X-Subject-Header-Begin-->.*<!--X-Subject-Header-End-->//s;

    # Separate headers and a body message.
    $$contref =~ s/<!--X-Head-Body-Sep-Begin-->/\n/;

    # For pluggin spaces before headers
    $$contref =~ s/^<LI>//gim;

    # Make header's name not to be indexed words.
    $$contref =~ s!</?EM>!!gi;
    $$contref =~ s/^\s+//;
}

1;
