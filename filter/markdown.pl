#
# -*- Perl -*-
# $Id: markdown.pl,v 1.1 2017-11-10 17:26:58 opengl2772 Exp $
# Copyright (C) 2017 Tadamasa Teranishi All rights reserved.
#               2017 Namazu Project All rights reserved.
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

package markdown;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';

my $mdconvpath  = undef;
my @mdconvopts  = undef;

sub mediatype() {
    return ('text/markdown');
}

sub status() {
    # The check of a dependence filter.
    return 'no' if (html::status() ne 'yes');

    $mdconvpath = util::checkcmd('pandoc');
    if (defined $mdconvpath) {
        @mdconvopts = ('-f', 'markdown', '-t', 'HTML');
        return 'yes';
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
    $magic->addFileExts('\\.md$', 'text/markdown');

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
        = @_;
    my $err = undef;

    $err = filter_md2html($orig_cfile, $cont, $weighted_str, $headings, $fields);

    return $err;
}

sub filter_md2html ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing markdown file ... (using  '$mdconvpath')\n");

    my $tmpfile = util::tmpnam('NMZ.md');
    util::writefile($tmpfile, $cont);
    {
        my @cmd = ($mdconvpath, @mdconvopts, $tmpfile);
        my $fh_out = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_out,
                "stderr" => "/dev/null",
            },
        );
        my $size = util::filesize($fh_out);
        if ($size == 0) {
            util::fclose($fh_out);
            unlink $tmpfile;
            return "Unable to convert file ($mdconvpath error occurred).";
        }
        if ($size > $conf::FILE_SIZE_MAX) {
            util::fclose($fh_out);
            unlink $tmpfile;
            return 'Too large markdown file.';
        }
        $$cont = util::readfile($fh_out);
        util::fclose($fh_out);
    }
    unlink $tmpfile;

    # codeconv::toeuc($cont);
    codeconv::codeconv_document($cont);

    # Title shoud be removed.
    $$cont =~ s!<TITLE.*?>.*?</TITLE>!!is;

    html::html_filter($cont, $weighted_str, $fields, $headings);

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
