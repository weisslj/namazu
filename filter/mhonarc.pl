#
# -*- Perl -*-
# $Id: mhonarc.pl,v 1.9 1999-08-30 07:25:42 satoru Exp $
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

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing MHonArc file ...\n");

    mhonarc_filter($contref, $weighted_str, $fields);
    html::html_filter($contref, $weighted_str, $fields, $headings);

    gfilter::uuencode_filter($contref);
    mailnews::mailnews_filter($contref, $weighted_str, $fields);
    mailnews::mailnews_citation_filter($contref, $weighted_str);

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    $fields->{title} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    gfilter::show_filter_debug_info($contref, $weighted_str,
			   $fields, $headings);
}

# MHonArc �ѤΥե��륿
# MHonArc v2.1.0 ��ɸ��ǽ��Ϥ��� HTML �����ꤷ�Ƥ��ޤ�
sub mhonarc_filter ($$) {
    my ($contref, $weighted_str) = @_;

    # MHonArc ��Ȥ��Ȥ��Ϥ���ʴ����˽������������
    $$contref =~ s/<!--X-MsgBody-End-->.*//s;
    $$contref =~ s/<!--X-TopPNI-->.*<!--X-TopPNI-End-->//s;
    $$contref =~ s/<!--X-Subject-Header-Begin-->.*<!--X-Subject-Header-End-->//s;
    $$contref =~ s/<!--X-Head-Body-Sep-Begin-->/\n/;  # �إå�����ʸ����ڤ�
    $$contref =~ s/^<LI>//gim;   # �إå������˶���򤢤������ʤ�����
    $$contref =~ s!</?EM>!!gi;  # �إå���̾���򥤥�ǥå����ˤ��줿���ʤ�
    $$contref =~ s/^\s+//;
}

1;
