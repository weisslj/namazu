# 
# -*- Perl -*-
# $Id: oletaro.pl,v 1.10 2004-10-16 14:54:13 opengl2772 Exp $
# 
# Copyright (C) 2000-2004 Namazu Project All rights reserved.
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

package oletaro;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'olemsword.pl';

use Win32::OLE::Const;

sub mediatype() {
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 11.0 Object Library");
    if ($const) {
        open (STDERR,">&SAVEERR");
        return (
            'application/ichitaro7', 'application/x-js-taro'
        );
    }
    $const = Win32::OLE::Const->Load("Microsoft Word 10.0 Object Library");
    if ($const) {
        open (STDERR,">&SAVEERR");
        return (
            'application/ichitaro5',
            'application/ichitaro6', 'application/ichitaro7',
            'application/x-js-taro'
        );
    }
    $const = Win32::OLE::Const->Load("Microsoft Word 9.0 Object Library");
    if ($const) {
        open (STDERR,">&SAVEERR");
        return (
            'application/ichitaro4', 'application/ichitaro5',
            'application/ichitaro6', 'application/ichitaro7',
            'application/x-js-taro'
        );
    }
    $const = Win32::OLE::Const->Load("Microsoft Word 8.0 Object Library");
    if ($const) {
        open (STDERR,">&SAVEERR");
        return (
            'application/ichitaro4', 'application/ichitaro5',
            'application/ichitaro6', 'application/ichitaro7',
            'application/x-js-taro'
        );
    }
    open (STDERR,">&SAVEERR");

    return (
        'application/ichitaro7', 'application/x-js-taro'
    );
}

sub status() {
    # The check of a dependence filter.
    return 'no' if (olemsword::status() ne 'yes');

    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 11.0 Object Library");
    $const = Win32::OLE::Const->Load("Microsoft Word 10.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 9.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 8.0 Object Library") unless $const;
    open (STDERR,">&SAVEERR");
    return 'yes' if (defined $const);
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
    $magic->addFileExts('\\.jsw$', 'application/ichitaro4');
    $magic->addFileExts('\\.jaw$|\\.jtw$', 'application/ichitaro5');
    $magic->addFileExts('\\.jbw$|\\.juw$', 'application/ichitaro6');
    $magic->addFileExts('\\.jfw$|\\.jvw$', 'application/ichitaro7');
    $magic->addFileExts('\\.jtd$|\\.jtt$', 'application/x-js-taro');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields) = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing ichitaro file ...\n");

    my $err = ReadMSWord::ReadDocument($cfile, $cont, $fields, $weighted_str);
    return $err if (defined $err);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
	unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str, $fields, $headings);

    return undef;
}

1;
