#
# -*- Perl -*-
# $Id: ooo.pl,v 1.16 2005-06-05 09:52:33 opengl2772 Exp $
# Copyright (C) 2003 Yukio USUDA 
#               2003-2005 Namazu Project All rights reserved ,
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

package ooo;
use strict;
use English;
require 'util.pl';
require 'gfilter.pl';


my $utfconvpath = undef;
my $unzippath = undef;
my @unzipopts;

sub mediatype() {
    # http://framework.openoffice.org/documentation/mimetypes/mimetypes.html
    return ('application/vnd.sun.xml.writer',
            'application/vnd.sun.xml.calc',
            'application/vnd.sun.xml.impress',
            'application/vnd.sun.xml.draw',
            'application/vnd.oasis.opendocument.text',
            'application/vnd.oasis.opendocument.spreadsheet',
            'application/vnd.oasis.opendocument.presentation',
            'application/vnd.oasis.opendocument.graphics');
}

sub status() {
    $unzippath = util::checkcmd('unzip');
    if (defined $unzippath){
        @unzipopts = ("-p");
        if (util::islang("ja")) {
           if ($English::PERL_VERSION >= 5.008) {
               $utfconvpath = "none";
               return 'yes';
           }
           $utfconvpath = util::checkcmd('lv');
           if ($utfconvpath){ 
               return 'yes';
           }else{
               $utfconvpath = util::checklib('unicode.pl');
               if ($utfconvpath){ 
                   return 'yes';
               }
           }
           return 'no'; 
        } else {
           return 'yes'; 
        }
    return 'no';
    }
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
    $magic->addFileExts('\\.sxw', 'application/vnd.sun.xml.writer');
    $magic->addFileExts('\\.sxc', 'application/vnd.sun.xml.calc');
    $magic->addFileExts('\\.sxi', 'application/vnd.sun.xml.impress');
    $magic->addFileExts('\\.sxd', 'application/vnd.sun.xml.draw');

    $magic->addFileExts('\\.odt', 'application/vnd.oasis.opendocument.text');
    $magic->addFileExts('\\.ods', 'application/vnd.oasis.opendocument.spreadsheet');
    $magic->addFileExts('\\.odp', 'application/vnd.oasis.opendocument.presentation');
    $magic->addFileExts('\\.odg', 'application/vnd.oasis.opendocument.graphics');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
        = @_;
    filter_metafile($contref, $weighted_str, $fields);
    filter_contentfile($contref, $weighted_str, $headings, $fields);
    return undef;
}

sub filter_metafile ($$$) {
    my ($contref, $weighted_str, $fields) = @_;
    my $metafile = 'meta.xml';
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
    my $keywords = ooo::get_keywords(\$xml);
    ooo::decode_entity(\$authorname);
    ooo::decode_entity(\$title);
    ooo::decode_entity(\$keywords);

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        ooo::utoe(\$authorname);
        ooo::utoe(\$title);
        ooo::utoe(\$keywords);
        codeconv::normalize_eucjp(\$authorname);
        codeconv::normalize_eucjp(\$title);
        codeconv::normalize_eucjp(\$keywords);
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

sub filter_contentfile ($$$$$) {
    my ($contref, $weighted_str, $headings, $fields) = @_;
    my $contentfile = "content.xml";
    my $xml = "";
    my $tmpfile  = util::tmpnam('NMZ.zip');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$contref;
        util::fclose($fh);
    }
    my @cmd = ($unzippath, @unzipopts, $tmpfile, $contentfile);
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

    ooo::remove_all_tag(\$xml);
    ooo::decode_entity(\$xml);

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        ooo::utoe(\$xml);
        codeconv::normalize_eucjp(\$xml);
    }
    $$contref = $xml;
    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
                                    $fields, $headings);
}

sub get_author ($){
  my ($contref) = @_;
  if ($$contref =~ m!<dc:creator>(.*)</dc:creator>!) {
      return $1;
  }else {
      return "";
  }
}

sub get_title ($){
  my ($contref) = @_;
  if ($$contref =~ m!<dc:title>(.*)</dc:title>!){
      return $1;
  }else {
      return "";
  }
}

sub get_keywords ($) {
  my ($contref) = @_;
  my @keywordstmp;
  push(@keywordstmp ,$$contref =~ m!<meta:keyword>(.*)</meta:keyword>!g);
  return  join(" ",@keywordstmp);
}

sub remove_all_tag ($) {
  my ($contref) = @_;
      $$contref =~ s/<[^>]*>/\n/gs;
      $$contref =~ s/\n+/\n/gs;
      $$contref =~ s/^\n+//;
}

# convert utf-8 to euc
# require Perl5.8 or unicode.pl
sub utoe ($) {
    my ($tmp) = @_;
    if ($utfconvpath =~ /lv/){
        my $tmpfile  = util::tmpnam('NMZ.ooo');
        {
            my $fh = util::efopen("> $tmpfile");
            print $fh $$tmp;
            util::fclose($fh);
        }
        my $cmd = ($utfconvpath . " -Iu8 " . "-Oej " . $tmpfile . " |");
        $$tmp = "";
        my $fh = util::efopen($cmd);
        while (defined(my $line = <$fh>)){
            $$tmp .= $line;
        }
        util::fclose($fh);
        unlink $tmpfile;
    }elsif ($English::PERL_VERSION >= 5.008){
        eval 'use Encode qw/from_to Unicode JP/;';
        Encode::from_to($$tmp, "utf-8" ,"euc-jp");
    }else{
        eval require 'unicode.pl';
        my @unicodeList = unicode::UTF8toUTF16($$tmp);
        $$tmp = unicode::u2e(@unicodeList);
        $$tmp =~ s/\00//g;
    }
}

# Decode a numberd entity. Exclude an invalid number.
sub decode_numbered_entity ($) {
    my ($num) = @_;
    return ""
        if $num >= 0 && $num <= 8 ||  $num >= 11 && $num <= 31 || $num >=127;
    sprintf ("%c",$num);
}

# Decode an entity. Ignore characters of right half of ISO-8859-1.
# Because it can't be handled in EUC encoding.
# This function provides sequential entities like: &quot &lt &gt;
sub decode_entity ($) {
    my ($text) = @_;

    return unless defined($$text);

    $$text =~ s/&#(\d{2,3})[;\s]/decode_numbered_entity($1)/ge;
    $$text =~ s/&#x([\da-f]+)[;\s]/decode_numbered_entity(hex($1))/gei;
    $$text =~ s/&quot[;\s]/\"/g; #"
    $$text =~ s/&apos[;\s]/\'/g; #"
    $$text =~ s/&amp[;\s]/&/g;
    $$text =~ s/&lt[;\s]/</g;
    $$text =~ s/&gt[;\s]/>/g;
    $$text =~ s/&nbsp[;\s]/ /g;
}

1;
