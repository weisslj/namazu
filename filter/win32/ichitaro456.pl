#
# -*- Perl -*-
# $Id: ichitaro456.pl,v 1.8 2004-05-10 06:02:49 opengl2772 Exp $
# Copyright (C) 1999 Ken-ichi Hirose,
#               2000-2004 Namazu Project All rights reserved.
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
use Cwd;
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

    my $tmpfile = util::tmpnam('NMZjstxt');
    copy("$cfile", "$tmpfile");
    my $tmpfile2 = $tmpfile;
    my $tmpext = $cfile;
    $tmpext =~ s/^.*(j[sab]w$)/$1/i;
    $tmpfile2 =~ s/tmp$/$tmpext/;
    move("$tmpfile", "$tmpfile2");

    util::vprint("Processing ichitaro file ... (using  '$ichitaro456')\n");

    my $cwd = getcwd();
    chdir("$var::OUTPUT_DIR");
    my $fh_cmd = util::efopen("$ichitaro456 -k -s -p NMZjstxt.$tmpext |");
    chdir("$cwd");
 
    $$cont = "";
    while (<$fh_cmd>) {$$cont .= $_;}
    util::fclose($fh_cmd);
    undef $fh_cmd;

    unlink($tmpfile2);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
	unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str, $fields, $headings);

    return undef;
}

1;
