#
# -*- Perl -*-
# $Id: xps.pl,v 1.9 2008-07-28 13:05:26 opengl2772 Exp $
# Copyright (C) 2007 Yukio USUDA, 
#               2007-2008 Namazu Project All rights reserved.
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

package xps;
use strict;
use English;
require 'util.pl';
require 'gfilter.pl';
require 'ooo.pl';
require 'msofficexml.pl';


sub mediatype() {
    return (
        'application/vnd.ms-xpsdocument',
    );
}

sub status() {
    if (ext::issupport('EXT_ZIP') eq 'yes') {
        # The check of a dependence filter.
        return 'no' if (msofficexml::status() ne 'yes');

        if (util::islang("ja")) {
            if ($conf::NKF ne 'no') {
                return 'yes';
            }
        } else {
            return 'yes';
        }
    }

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
    my ($magic) = @_;

    # FIXME: very ad hoc.
    $magic->addFileExts('\\.xps$', 'application/vnd.ms-xpsdocument');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
        = @_;
    my $err = undef;
    $err = msofficexml::filter_metafile($contref, $weighted_str, $fields);
#    return $err if (defined $err);
    $err = filter_contentfile($contref, $weighted_str, $headings, $fields);
    return $err;
}

sub get_pages_list ($$) {
    my ($zipref, $pagesref) = @_;

    my $err = undef;
    $err = $extzip::zip_membersMatching->($zipref, 'Documents/1/Pages/\d+\.fpage', $pagesref);
    return $err;
}

sub filter_contentfile ($$$$$) {
    my ($contref, $weighted_str, $headings, $fields) = @_;
    my @pagefiles;

    my $xml = "";
    my $err = get_pages_list($contref, \@pagefiles);
    return $err if (defined $err);

    foreach my $filename (@pagefiles) {
        my $xmlcont = '';
        my $err = $extzip::zip_read->($contref, $filename, \$xmlcont);
        return $err if (defined $err);

        if ($xmlcont =~ m!^\377\376\<\000F\000i\000x\000e\000d!) {
            codeconv::to_inner_encoding(\$xmlcont, 'UTF-16LE');
        } else {
            codeconv::to_inner_encoding(\$xmlcont, 'unknown');
        }
        codeconv::normalize_nl(\$xmlcont);

        xps::get_document(\$xmlcont);
        $xml .= ' ' . $xmlcont;
    }

    ooo::remove_all_tag(\$xml);
    ooo::decode_entity(\$xml);

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        codeconv::normalize_jp(\$xml);
    }

    $$contref = $xml;

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
                                    $fields, $headings);

    return undef;
}

sub get_keywords ($) {
    my ($contref) = @_;
    my @keywordstmp;
    push(@keywordstmp, $$contref =~ m!<cp:keywords>(.*)</cp:keywords>!g);
    return join(" ", @keywordstmp);
}

sub get_document ($) {
    my ($contref) = @_;
    my @documents;
    push(@documents, $$contref =~ m!UnicodeString="([^"]*)"!g);
    $$contref = join(" ", @documents);
}

1;
