#
# -*- Perl -*-
# $Id: ichitaro456.pl,v 1.13 2000-04-05 18:15:37 kenzo- Exp $
# Copyright (C) 1999 Ken-ichi Hirose,
#               2000 Namazu Project All rights reserved.
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
#  You need "jstxt.exe" command.
#

package ichitaro456;
use strict;
use File::Copy;
require 'util.pl';
require 'gfilter.pl';

my $ichitaro456 = undef;
my $doscmd = undef;

sub mediatype() {
    return ('application/ichitaro4', 'application/ichitaro5', 'application/ichitaro6');
}

sub status() {
    $ichitaro456 = util::checkcmd('jstxt.exe');
    if (defined $ichitaro456) {
    if ($mknmz::SYSTEM eq "MSWin32") {
        return 'yes';
    } else {
    $doscmd = util::checkcmd('doscmd');
    if (defined $doscmd) {
        $ichitaro456 = "$doscmd $ichitaro456";
        return 'yes';
    }
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
    return 1;
}

sub add_magic ($) {
    my ($magic) = @_;
    $magic->addFileExts('(?i)\\.jsw', 'application/ichitaro4');
    $magic->addFileExts('(?i)\\.jaw', 'application/ichitaro5');
    $magic->addFileExts('(?i)\\.jbw', 'application/ichitaro6');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;

    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.jstxt');
    my $tmpfile2 = $cfile;
    if ($cfile =~ /[\x81-\x9f\xe0-\xef][\x40-\x7e\x80-\xfc]|[\x20\xa1-\xdf]/) {
        my $tmpfile3 = util::tmpnam('NMZJSTXT');
        copy("$cfile", "$tmpfile3");
        $tmpfile2 = $tmpfile3;
        my $tmpext = $cfile;
        $tmpext =~ s/^.*(j[sab]w$)/$1/i;
        $tmpfile2 =~ s/tmp$/$tmpext/;
        move("$tmpfile3", "$tmpfile2");
    }
    $tmpfile2 = "C:$tmpfile2"
     if ($mknmz::SYSTEM ne "MSWin32" && $tmpfile2 =~ m!^/!);

    util::vprint("Processing ichitaro file ... (using  '$ichitaro456')\n");

    system("$ichitaro456 -k -s -p $tmpfile2 > $tmpfile");
    my $fh = util::efopen("< $tmpfile");
    $$cont = util::readfile($fh);
    undef $fh;
    unlink($tmpfile);
    $tmpfile2 =~ s/^C://;
    unlink("$tmpfile2");

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}

1;
