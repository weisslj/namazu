#
# -*- Perl -*-
# $Id: applefile.pl,v 1.6 2006-08-12 07:18:44 opengl2772 Exp $
# Copyright (C) 2004 Tadamasa Teranishi All rights reserved ,
# Copyright (C) 2004 Namazu Project All rights reserved ,
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

# rfc1740
#     <http://www.cis.ohio-state.edu/htbin/rfc/rfc1740.html>

package applefile;
use strict;
require 'util.pl';
require 'document.pl';

sub mediatype() {
    return ('application/applefile');
}

sub status() {
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

#   AppleSingle encoded Macintosh file
    $magic->addMagicEntry("0\tbelong\t0x00051600\tapplication/applefile");

#   AppleDouble encoded Macintosh file
    $magic->addMagicEntry("0\tbelong\t0x00051607\tapplication/applefile");

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    util::vprint("Processing applefile ...\n");

    my $magic = unpack('N', substr($$contref, 0, 4));

    # Not support AppleDouble.
    return 'not_support_appldouble file' if ($magic == 0x00051607);

    my $entries = unpack('C', substr($$contref, 24, 1)) * 256 + unpack('C', substr($$contref, 25, 1));

    my $type = '    ';
    my $creator = '    ';
    my $datafork = undef;

    my $i;
    for($i = 0; $i < $entries; $i++) {
        my $id = unpack('N', substr($$contref, 26 + $i * 12, 4));
        my $offset = unpack('N', substr($$contref, 26 + $i * 12 + 4, 4));
        my $length = unpack('N', substr($$contref, 26 + $i * 12 + 8, 4));

        if ($id == 9) {
            $type = substr($$contref, $offset, 4);
            $creator = substr($$contref, $offset + 4, 4);
        } elsif ($id == 1) {
            if ($length > $conf::FILE_SIZE_MAX) {
                return 'too_large_applefile_file';
            }
            $datafork = substr($$contref, $offset, $length);
        }
    }

    $$contref = $datafork;

    my $mmtype = undef;
    if (($type =~ /^XLS/) && ($creator =~ /XCEL/)) { 
        $mmtype = 'application/excel';
    } elsif (($type =~ /W.BN/) && ($creator =~ /MSWD/)) {
        $mmtype = 'application/msword';
    } elsif (($type =~ /(SLD8|PPSS)/) && ($creator =~ /^PPT/)) { 
        $mmtype = 'application/powerpoint';
    } elsif (($type =~ /(SLD3)/) && ($creator =~ /^PPT/)) { 
        # ppthtml doesn't sopport this file.
        $mmtype = 'application/powerpoint4';
    } elsif ($type =~ /^PDF/) {
        $mmtype = 'application/pdf';
    } elsif ($type =~ /^RTF/) {
        $mmtype = 'application/rtf';
    } elsif ($type =~ /TEXT/) {
        $mmtype = undef; 
    }
    
    my $dummy_shelterfilename = "";
    my $err = undef;
    my $mtype;
    {
	my $uri;
	my $Document = undef;
	$Document = mknmz::document->new();
	$Document->init_doc($uri, $$orig_cfile, \$datafork, $mmtype);
	$$contref = ${$Document->get_filtered_contentref()};
	$mtype = $Document->get_mimetype();
	$$weighted_str = $Document->get_weighted_str();
	$$headings = $Document->get_headings();
	%$fields = $Document->get_fields();
    }
    if ($mtype =~ /; x-system=unsupported$/) {
        $$contref = "";
        $err = "filter/macbinary.pl gets error from other filter";
        util::dprint($err);
        $err = $mtype;
    } elsif ($mtype =~ /; x-error=(.*)$/) {
        $$contref = "";
        $err = "filter/macbinary.pl gets error from other filter";
        util::dprint($err);
        $err = $1;
    } else {
        gfilter::show_filter_debug_info($contref, $weighted_str,
                        $fields, $headings);
    }

    return $err;
}

1;
