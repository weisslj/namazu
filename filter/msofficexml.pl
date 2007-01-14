#
# -*- Perl -*-
# $Id: msofficexml.pl,v 1.6 2007-01-14 09:03:20 opengl2772 Exp $
# Copyright (C) 2007 Yukio USUDA 
#               2007 Namazu Project All rights reserved ,
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

package msofficexml;
use strict;
use English;
require 'util.pl';
require 'gfilter.pl';
require 'ooo.pl';


my $utfconvpath = undef;
my $unzippath = undef;
my @unzipopts;

sub mediatype() {
    return (
        'application/vnd.openxmlformats-officedocument.wordprocessingml',
        'application/vnd.openxmlformats-officedocument.spreadsheetml',
        'application/vnd.openxmlformats-officedocument.presentationml',
    );
}

sub status() {
    $unzippath = util::checkcmd('unzip');
    if (defined $unzippath){
        @unzipopts = ("-p");
        if (util::islang("ja")) {
           if ($conf::NKF ne 'no') {
               return 'yes';
           }
           return 'no'; 
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
    $magic->addFileExts('\\.docx$', 'application/vnd.openxmlformats-officedocument.wordprocessingml');
    $magic->addFileExts('\\.xlsx$', 'application/vnd.openxmlformats-officedocument.spreadsheetml');
    $magic->addFileExts('\\.pp[st]x$', 'application/vnd.openxmlformats-officedocument.presentationml');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
        = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
    my $ext = '';
    if ($cfile =~ m/\.([A-Za-z]{4})$/){
        $ext = $1;
    }
    filter_metafile($contref, $weighted_str, $fields);
    filter_contentfile($contref, $weighted_str, $headings, $fields, $ext);
    return undef;
}

sub filter_metafile ($$$) {
    my ($contref, $weighted_str, $fields) = @_;
    my $metafile = 'docProps/core.xml';
    my $xml = "";
    my $tmpfile  = util::tmpnam('NMZ.zip');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$contref;
        util::fclose($fh);
    }
    my @cmd = ($unzippath, @unzipopts, $tmpfile, $metafile);
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => \$xml,
            "stderr" => "/dev/null",
            "mode_stdout" => "wt",
            "mode_stderr" => "wt",
        },
    );
    unlink $tmpfile;

    my $authorname = ooo::get_author(\$xml);
    my $title = ooo::get_title(\$xml);
    my $keywords = msofficexml::get_keywords(\$xml);
    ooo::decode_entity(\$authorname);
    ooo::decode_entity(\$title);
    ooo::decode_entity(\$keywords);

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        codeconv::normalize_jp(\$authorname);
        codeconv::normalize_jp(\$title);
        codeconv::normalize_jp(\$keywords);
    }
    if (!($authorname eq "")){
        $fields->{'author'} = $authorname;
    }
    if (!($title eq "")){
        $fields->{'title'} = $title;
        my $weight = $conf::Weight{'html'}->{'title'};
        $$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";
    }
    my @weight_str = split(' ',$keywords);
    for my $tmp (@weight_str) {
        my $weight = $conf::Weight{'metakey'};
        $$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n" if $tmp;
    }
}

sub zip_read ($$$) {
    my ($zipref, $fname, $unzipcontref) = @_;
    my $tmpfile  = util::tmpnam('NMZ.zip');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$zipref;
        util::fclose($fh);
    }
    my @cmd = ($unzippath, @unzipopts, $tmpfile, $fname);
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $unzipcontref,
            "stderr" => "/dev/null",
            "mode_stdout" => "wt",
            "mode_stderr" => "wt",
        },
    );
    unlink $tmpfile;
}

sub filter_contentfile ($$$$$) {
    my ($contref, $weighted_str, $headings, $fields, $ext) = @_;
    my @contentfile;
    my $xml = "";
    if ($ext =~ /docx/i){
        $contentfile[0] = 'word/document.xml';
    }elsif ($ext =~ /xlsx/i){
        $contentfile[0] = 'xl/sharedStrings.xml';
    }elsif ($ext =~ /pp(t|s)x/i){
        my $filename = 'docProps/app.xml';
        msofficexml::zip_read($contref, $filename, \$xml);
        my $slides = 1;
        if ($xml =~ m!<Slides>(\d)</Slides>!){
            $slides = $1;
        }
        for (my $i = 1; $i <= $slides; $i++){
            $contentfile[$i-1] = 'ppt/slides/slide' . $i . '.xml';
        }
    }
    $xml = "";
    foreach my $filename (@contentfile){
        my $xmlcont = '';
        msofficexml::zip_read($contref, $filename, \$xmlcont);
        $xml .= $xmlcont
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
}

sub get_keywords ($) {
  my ($contref) = @_;
  my @keywordstmp;
  push(@keywordstmp ,$$contref =~ m!<cp:keywords>(.*)</cp:keywords>!g);
  return  join(" ",@keywordstmp);
}

1;
