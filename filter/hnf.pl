#
# -*- Perl -*-
# $Id: hnf.pl,v 1.1 2000-01-13 10:56:23 kenji Exp $
#
# HNF Filter for Namazu 2.0
# version 0.9.5
# 2000/1/1  Kenji Suzuki <kenji@h14m.org>
#
# Copyright (C) 1999,2000  Kenji Suzuki, HyperNikkiSystem Project
# All rights reserved.
#
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

package hnf;
use strict;
require 'util.pl';
require 'gfilter.pl';
require 'html.pl';


sub mediatype() {
    return ('text/hnf');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub codeconv() {
    return 1;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing hnf file ...\n");

    my $mark = "# ";
    my $end  = "--";
    $mark = "��" if util::islang("ja");
    $end  = "��" if util::islang("ja");

    get_uri($cfile, $fields);
    hnf_filter($contref, $weighted_str, $fields, $headings, $cfile, 
	$mark, $end);
    html::html_filter($contref, $weighted_str, $fields, $headings);
    $fields->{'summary'} = 
	make_summary($contref, $headings, $cfile, $mark, $end);

    gfilter::line_adjust_filter($contref);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($contref);
    gfilter::show_filter_debug_info($contref, $weighted_str,
			   $fields, $headings);
    return undef;
}

sub hnf_filter ($$$$$$$) {
    my ($contref, $weighted_str, $fields, $headings, $cfile, $mark, $end) = @_;

    $$contref =~ s/</&lt;/g;
    $$contref =~ s/>/&gt;/g;

    # has OK?
    if ($$contref =~ /^OK$/m) {
        # has correct User Variable?
        my @tmp = split ("OK\n", $$contref);
        my $header = $tmp[0];

        # illeagel hnf header means has no OK
        if ($header =~ /\nCAT |\nNEW |\nLNEW /) {
            $$contref = "\ncommand_NG\n";	# NG is a pseudo-command
        }
        else {
          $tmp[0] = "";
          $$contref = join("OK\n", @tmp);
          $$contref =~ s/OK\n//;
          $$contref .= "\ncommand_OK\n" . $header;
        }
    }
    # has no OK
    else {
        $$contref = "\ncommand_NG\n";	# NG is a pseudo-command
    }

    # Title & Date string (YYYYMMDD)
    my $title = $cfile;
    $title =~ s/(.*)\/d(\d{8,})\.hnf/$2/;
    my $date = $title;
    $title =~ s/(\d{4,})(\d\d)(\d\d)/$1\/$2\/$3/;
    $$contref = "<title>$title</title>\n" . $$contref;

    # ~
    $$contref =~ s/~\n/\n/g;

    # command
    $$contref =~ s/^GRP (.*)/command_GRP GRP $1/gm;
    $$contref =~ s/^CAT (.*)/command_CAT CAT $1/gm;
    $$contref =~ s/^NEW (.*)/command_NEW <h1>$mark$1<\/h1>/gm;
    $$contref =~ 
	s/^LNEW (.*?) (.*)/command_LNEW <h1>$mark<a href=\"$1\">$2<\/a><\/h1>/gm;
    $$contref =~ s/command_GRP (.*)\n/command_GRP $1 /gm;
    $$contref =~ s/command_CAT (.*)\n/command_CAT $1 /gm;

    # hiding GRP section 
    $$contref =~ s/^command_GRP (.*)<h1>(.*)<\/h1>/command_GRP $1 $2/gm
	if $hnf::grp_hide;

    $$contref =~ s/^SUB (.*)/
	command_SUB <strong>$1<\/strong>/gm;
    $$contref =~ s/^LSUB (.*?) (.*)/
	command_LSUB <strong><a href=\"$1\">$2<\/a><\/strong>/gm;
 
    $$contref =~ s/^LINK (.*?) (.*)/
	command_LINK <a href=\"$1\">$2<\/a>/gm;
    $$contref =~ s/^URL (.*?) (.*)/
	command_URL <a href=\"$1\">$2<\/a>/gm;
    $$contref =~ s/^RLINK (.*?) (.*?) (.*)/
	command_RLINK <a href=\"$1 $2\">$3<\/a>/gm;

    $$contref =~ s/^FONT (.*?) (.*?) (.*)/
	command_FONT $1 $2 $3/gm;

    $$contref =~ s/^PRE$/
	command_PRE/gm;
    $$contref =~ s/^P$/
	command_P/gm;
    $$contref =~ s/^CITE\s(.*)/
	command_CITE $1/gm;

    $$contref =~ s/\nFN\n/
	command_FN\n/g;

    $$contref =~ s/^UL$/
	command_UL/gm;
    $$contref =~ s/^OL$/
	command_OL/gm;
    $$contref =~ s/^DL$/
	command_DL/gm;

    $$contref =~ s/^\/([A-Z]+)$//gm;

    $$contref =~ s/^LI (.*)$/$1/gm;
    $$contref =~ s/^DT (.*)$/$1/gm;
    $$contref =~ s/^DD (.*)$/$1/gm;

    $$contref =~ s/^STRIKE (.*)/
	command_STRIKE <strike>$1<\/strike>/gm;
    $$contref =~ s/^LSTRIKE (.*?) (.*)/
        command_LSTRIKE <strike><a href=\"$1\">$2<\/a><\/strike>/gm;

    $$contref =~ s/^STRONG (.*)/
	command_STRONG <strong>$1<\/strong>/gm;

    $$contref =~ s/^IMG (.*?) (.*?) (.*)/
	command_IMG $1 $2 $3/gm;
    $$contref =~ s/^LIMG (.*?) (.*?) (.*?) (.*)/
	command_LIMG $1 $2 $3 $4/gm;

    $$contref =~ s/^MARK (.*)/
	command_MARK $1/gm;

    if ($$contref =~ /^ALIAS (.*)/m) {
        read_alias_file() unless $hnf::alias{$1};
    }
    $$contref =~ s/^ALIAS (.*)/
	command_ALIAS $hnf::alias{$1}/gm;

    $$contref .= "<h1>$end</h1>";
    $$contref .= $date;
}

sub get_uri ($$) {
    my ($cfile, $fields) = @_;

    my ($uri);
    if ($cfile =~ /^(.*)\/d(\d\d\d\d*)([0-1]\d)([0-3])(\d)\.hnf$/) {
      my $year = $2;
      my $month = $3;
      my $day = $4 . $5;
      my $hiday = $4;
      my $abc;
      if ($day < 11) {
        $abc = "a";
      }
      elsif ($day < 21) {
        $abc = "b";
      }
      else {
        $abc = "c";
      }
      if ($hnf::hns_version >= 2) {
        $uri = "?$year$month$abc#$year$month$day"; # for hns-2.00 or later
      }
      else {
        $uri = "?$year$month$hiday#$year$month$day"; # for hns-1.x
      }
      $uri = $hnf::diary_uri . $uri . "0";
      $uri =~ s/%7E/~/i;
    }
    $fields->{'uri'} = $uri;
    $fields->{'author'} = $hnf::author;
}

sub make_summary ($$$$$) {
    my ($contref, $headings, $cfile, $mark, $end) = @_;

    # pick up $conf::SUMMARY_LENGTH bytes string
    my $tmp = "";
    if ($$headings ne "") {
        $$headings =~ s/^\s+//;
        $$headings =~ s/\s+/ /g;
        $tmp = $$headings;
	$tmp = "" if $tmp eq "$end ";	# for no OK hnf
    } else {
        $tmp = "";
    }

    my $offset = 0;
    my $tmplen = 0;
    my $tmp2 = $$contref;
    $tmp2 =~ s/\ncommand_OK\n.*//s;	# remove below of command_OK
    $tmp2 =~ s/\ncommand_NG\n.*//s;	# remove below of command_NG
    # hiding GRP section
    $tmp2 =~ s/\ncommand_GRP (.*?)\ncommand_/\ncommand_/gs if $hnf::grp_hide;
    $tmp2 =~ s/command_CAT CAT (.*)//gm;
    $tmp2 =~ s/command_[A-Z]+//g;
    $tmp2 =~ s/^! (.*)$//gm;
    $tmp2 =~ s/^!# (.*)$//gm;

    while (($tmplen = $conf::SUMMARY_LENGTH + 1 - length($tmp)) > 0
           && $offset < length($tmp2))
    {
        $tmp .= substr $tmp2, $offset, $tmplen;
        $offset += $tmplen;
        $tmp =~ s/(([\xa1-\xfe]).)/$2 eq "\xa8" ? '': $1/ge;
        $tmp =~ s/([-=*\#])\1{2,}/$1$1/g;
    }

    my $summary = substr $tmp, 0, $conf::SUMMARY_LENGTH;
    my $kanji = $summary =~ tr/\xa1-\xfe/\xa1-\xfe/;
    $summary .= substr($tmp, $conf::SUMMARY_LENGTH, 1) if $kanji %2;
    $summary =~ s/^\s+//;
    $summary =~ s/\s+/ /g;   # normalize white spaces

    return $summary;
}

sub read_alias_file () {
    if (-f $hnf::alias_file) {
        my $def = util::readfile($hnf::alias_file);
        my @aliases = split("\n", $def);
        foreach (@aliases) {
            if ($_ =~ /(\S+) (.*)/) {
                $hnf::alias{$1} = $2;
                util::vprint("alias: $1 $2\n");
            }
        }
    }
}

1;