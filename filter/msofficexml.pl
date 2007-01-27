#
# -*- Perl -*-
# $Id: msofficexml.pl,v 1.12 2007-01-27 02:33:11 usu Exp $
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

package msofficexml;
use strict;
use English;
require 'util.pl';
require 'gfilter.pl';
require 'ooo.pl';
require 'zip.pl';


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
    msofficexml::zip_read($contref, $metafile, \$xml);

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
    my $tmpfile;
    my $uniqnumber = int(rand(10000));
    do {
        $tmpfile = util::tmpnam('NMZ.zip' . substr("000$uniqnumber", -4));
        $uniqnumber++;
    } while (-f $tmpfile);
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
            "mode_stdout" => "wb",
            "mode_stderr" => "wt",
        },
    );
    unlink $tmpfile;
}

sub get_embedded_list ($$$) {
    my ($zipref, $embeddingsref, $apptype) = @_;
    my $tmpfile  = util::tmpnam('NMZ.zip');
    { 
        my $fh = util::efopen("> $tmpfile");
        print $fh $$zipref;
        util::fclose($fh);
    }
    my @unzipopts_getlist = ("-Z", "-1");
    my @cmd = ($unzippath, @unzipopts_getlist, $tmpfile);
    my $file_list;
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => \$file_list,
            "stderr" => "/dev/null",
            "mode_stdout" => "wt",
            "mode_stderr" => "wt",
        },
    );
    if ($status == 0) {
	while ($file_list =~ m!\n
			($apptype/embeddings/.+)!gx){	# embeddings filename
            my $filename = $1;
            push(@$embeddingsref, $filename);
	}
    }
    unlink $tmpfile;
}

sub filter_contentfile ($$$$$) {
    my ($contref, $weighted_str, $headings, $fields, $ext) = @_;
    my @contentfile;
    my @embeddedfiles;
    my $xml = "";
    if ($ext =~ /docx/i){
        $contentfile[0] = 'word/document.xml';
        get_embedded_list($contref, \@embeddedfiles, 'word');
    }elsif ($ext =~ /xlsx/i){
        $contentfile[0] = 'xl/sharedStrings.xml';
        get_embedded_list($contref, \@embeddedfiles, 'xl');
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
        get_embedded_list($contref, \@embeddedfiles, 'ppt');
    }
    $xml = "";
    foreach my $filename (@contentfile){
        my $xmlcont = '';
        msofficexml::zip_read($contref, $filename, \$xmlcont);
        $xml .= $xmlcont
    }
    if ($ext =~ /xlsx/i){
        my $xmlcont = '';
        my $filename = 'xl/workbook.xml';
        msofficexml::zip_read($contref, $filename, \$xmlcont);
        $xml .= msofficexml::get_sheetname(\$xmlcont);
    }

    msofficexml::remove_txbodytag(\$xml);
    ooo::remove_all_tag(\$xml);
    ooo::decode_entity(\$xml);

    # Code conversion for Japanese document.
    if (util::islang("ja")) {
        codeconv::normalize_jp(\$xml);
    }

    my $embeddedcont = '';
    if (@embeddedfiles) {
        foreach my $fname (@embeddedfiles){
            my $cont = '';
            msofficexml::zip_read($contref, $fname, \$cont);
            my $unzippedname = "unzipped_content";
            if ($fname =~ /.*(\..*)/){
                $unzippedname = $unzippedname . $1;
            }
            my $err = zip::nesting_filter($unzippedname, \$cont, $weighted_str);
            if (defined $err) {
                util::dprint("filter/zip.pl gets error message \"$err\"");
            }
            $embeddedcont .= " " . $cont;
        }
    }
    $$contref = $xml . $embeddedcont;

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

sub get_sheetname ($) {
    my ($contref) = @_;
    my @sheetnames;
    push(@sheetnames ,$$contref =~ m!<sheet name="([^"]*)"!g);
    return  join(" ",@sheetnames);
}

sub remove_txbodytag($){
    my ($contref) = @_;
    my $txbodies = '';
    while ($$contref =~ m!<p:txBody>(.*?)</p:txBody>!sg){
       my $txbody = $1;
       $txbody =~ s/<a:p>/\n/gs;
       $txbody =~ s/<[^>]*>//gs;
       $txbodies .= " " . $txbody; 
    }
    $$contref =~ s!<p:txBody>(.*?)</p:txBody>!!sg;
    $$contref .= $txbodies;
}

1;
