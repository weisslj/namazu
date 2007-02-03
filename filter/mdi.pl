#
# -*- Perl -*-
# $Id: mdi.pl,v 1.1 2007-02-03 04:42:49 usu Exp $
# Copyright (C) 2007 Yukio USUDA,
#               2007 Namazu Project All rights reserved.
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

package mdi;
use strict;
use English;
require 'util.pl';
require 'gfilter.pl';


my $utfconvpath = undef;

sub mediatype() {
    return ('image/vnd.ms-modi');
}

sub status() {
    if (util::islang("ja")) {
        if ($conf::NKF ne 'no') {
            return 'yes';
        }
        return 'no'; 
    }
    return 'yes'; 
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

    $magic->addMagicEntry("0\tbelong\t0x45502a00\timage/vnd.ms-modi");
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
        = @_;
    filter_msdocumentimaging($contref, $weighted_str, $headings, $fields);
    return undef;
}

sub filter_msdocumentimaging ($$$$$) {
    my ($contref, $weighted_str, $headings, $fields) = @_;

    if ($$contref =~ m/\x01\x00(.*?)\x0d\x00/s){
        $$contref = $1;
        $$contref =~ s/.*\x01\x00(.*)/$1/g;
        my $length = substr($$contref, 0, 4);
        $$contref = substr($$contref, 4);
        $$contref =~ s/\x20\x0a/\x0a/g;
        $$contref =~ s/\x20//g;
    }else {
        $$contref = '';
    }

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
         codeconv::normalize_jp($contref);
    }
    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
                                    $fields, $headings);
}

1;
