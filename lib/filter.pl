#
# -*- Perl -*-
# $Id: filter.pl,v 1.2 1999-05-04 04:42:37 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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

package filter;
use strict;
require "html.pl";

# Filters
sub document_filter ($$$$$$\%) {
    my ($orig_cfile, $title, $contents, $weighted_str, $headings, $fields)
	= @_;

    my $cfile = $orig_cfile;
    my $mhonarc_opt = 0;

    $cfile =~ s/\.(gz|Z)$//;  # zipped file
    analize_rcs_stamp();
    $mhonarc_opt = 1 if 
	(!$conf::NoMHonArcOpt && $$contents =~/^$conf::MHONARC_HEADER/);
    if (html::is_html($cfile)) {
	mhonarc_filter($contents, $weighted_str) 
	    if $mhonarc_opt;
	html_filter($contents, $weighted_str, $title, $fields, $headings);
    } elsif ($cfile =~ /rfc\d+\.txt/i ) {
	rfc_filter($contents, $weighted_str, $title);
    } elsif ($conf::ManOpt) {
	man_filter($contents, $weighted_str, $title);
    }
    uuencode_filter($contents) if $conf::UuencodeOpt;
    if ($mhonarc_opt  || $conf::MailNewsOpt) {
	mailnews_filter($contents, $weighted_str, $title, $fields);
	mailnews_citation_filter($contents, $weighted_str);
    }
    line_adjust_filter($contents) unless $conf::NoLineAdOpt;
    line_adjust_filter($weighted_str) unless $conf::NoLineAdOpt;
    white_space_adjust_filter($contents);
    filename_to_title($cfile, $title, $weighted_str) unless $$title;
    show_filter_debug_info($contents, $weighted_str,
			   $title, $fields, $headings);
}

# Show debug information for filters
sub show_filter_debug_info ($$$$) {
    my ($contents, $weighted_str, $title, $fields, $headings) = @_;
    util::dprint("-- title --\n$$title\n");
    util::dprint("-- content --\n$$contents\n");
    util::dprint("-- weighted_str: --\n$$weighted_str\n");
    util::dprint("-- headings --\n$$headings\n");
}

# Adjust white spaces
sub white_space_adjust_filter ($) {
    my ($text) = @_;
    $$text =~ s/^ +//gm;
    $$text =~ s/ +$//gm;
    $$text =~ s/ +/ /g;
    $$text =~ s/\n+/\n/g;
}

# �ե�����̾���饿���ȥ����� (ñ�ʤ�ƥ����ȥե�����ξ��)
sub filename_to_title ($\$\$) {
    my ($cfile, $title, $weighted_str) = @_;

    # for MSWin32's filename using Shift_JIS [1998-09-24]
    if (($namazu::SYSTEM eq "MSWin32") || ($namazu::SYSTEM eq "os2")) {
	$cfile = codeconv::shiftjis_to_eucjp($cfile);
    }
    
    $cfile =~ m!^.*/([^/]*)$!;
    my $filename = $1;
    # �ե�����̾�򸵤˥�����ɤ���Ф��Ƥߤ� v1.1.1
    # modified [1998-09-18] 
    my $tmp = $filename;
    $tmp =~ s|/\\_\.-| |g;

    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n";

    $$title = $filename . $conf::TEXT_TITLE;
}

# HTML �ѤΥե��륿
sub html_filter ($$$$$) {
    my ($contents, $weighted_str, $title, $fields, $headings) = @_;

    html::escape_lt_gt($contents);
    html::get_title($contents, $weighted_str, $title);
    html::get_author($contents, $fields);
    html::get_meta_info($contents, $weighted_str);
    html::get_img_alt($contents);
    html::get_table_summary($contents);
    html::get_title_attr($contents);
    html::normalize_html_element($contents);
    html::erase_above_body($contents);
    html::weight_element($contents, $weighted_str, $headings);
    html::remove_html_elements($contents);
    # ���줾����λ��Ȥ�����
    html::decode_entity($contents);
    html::decode_entity($weighted_str);
    html::decode_entity($headings);
}

# MHonArc �ѤΥե��륿
# MHonArc v2.1.0 ��ɸ��ǽ��Ϥ��� HTML �����ꤷ�Ƥ��ޤ�
sub mhonarc_filter ($$) {
    my ($contents, $weighted_str) = @_;

    # MHonArc ��Ȥ��Ȥ��Ϥ���ʴ����˽������������
    $$contents =~ s/<!--X-MsgBody-End-->.*//s;
    $$contents =~ s/<!--X-TopPNI-->.*<!--X-TopPNI-End-->//s;
    $$contents =~ s/<!--X-Subject-Header-Begin-->.*<!--X-Subject-Header-End-->//s;
    $$contents =~ s/<!--X-Head-Body-Sep-Begin-->/\n/;  # �إå�����ʸ����ڤ�
    $$contents =~ s/^<LI>//gim;   # �إå������˶���򤢤������ʤ�����
    $$contents =~ s!</?EM>!!gi;  # �إå���̾���򥤥�ǥå����ˤ��줿���ʤ�
    $$contents =~ s/^\s+//;
}

# Mail/News �ѤΥե��륿
# ���Ȥʤ��Τϸ���@��ޥϤ���ˤ��������ޤ���
sub mailnews_filter ($$$\%) {
    my ($contents, $weighted_str, $title, $fields) = @_;

    my $boundary = "";
    my $line     = "";
    my $partial  = 0;

    $$contents =~ s/^\s+//;
    # 1 ���ܤ��إå��äݤ��ʤ��ե�����ϡ��إå��������ʤ�
    return unless $$contents =~ /(^\S+:|^from )/i;

    my @tmp = split(/\n/, $$contents);
  HEADER_PROCESSING:
    while (@tmp) {
	$line = shift(@tmp);
	last if ($line =~ /^$/);  # if an empty line, header is over
	# connect the two lines if next line has leading spaces
	while (defined($tmp[0]) && $tmp[0] =~ /^\s+/) {
	    # if connection is Japanese character, remove spaces
	    # from Furukawa-san's idea [1998-09-22]
	    my $nextline = shift @tmp;
	    $line =~ s/([\xa1-\xfe])\s+$/$1/;
	    $nextline =~ s/^\s+([\xa1-\xfe])/$1/;
	    $line .= $nextline;
	}
	# keep field info
	if ($line =~ /^(\S+):\s(.*)/i) {
	    my $name = $1;
	    my $value = $2;
	    $fields->{lc($name)} = $value;
	    if ($name =~ /^($conf::REMAIN_HEADER)$/io) {
		# keep some fields specified REMAIN_HEADER for search keyword
		my $weight = $conf::Weight{'headers'};
		$$weighted_str .= 
		    "\x7f$weight\x7f$value\x7f/$weight\x7f\n";
	    }
 	}
	if ($line =~ s/^subject:\s*//i){
	    $$title = $line;
	    html::encode_entity($title);
	    # ML ��ͭ�� [hogehoge-ML:000] ���ɤ����Ф���
	    # �Τ��տޤ��������ݤʤΤǡ�
	    # �����塢�ǽ�� [...] ���ɤ����Ф���
	    $line =~ s/^\[.*?\]\s*//;

	    # 'Re:' ���ɤ����Ф���
	    $line =~ s/\bre:\s*//gi;

	    my $weight = $conf::Weight{'html'}->{'title'};
	    $$weighted_str .= "\x7f$weight\x7f$line\x7f/$weight\x7f\n";
	} elsif ($line =~ s/^content-type:\s*//i) {
	    if ($line =~ /multipart.*boundary="(.*)"/i){
		$boundary = $1;
		util::dprint("((boundary: $boundary))\n");
  	    } elsif ($line =~ m!message/partial;\s*(.*)!i) {
		# The Message/Partial subtype routine [1998-10-12]
		# contributed by Hiroshi Kato <tumibito@mm.rd.nttdata.co.jp>
  		$partial = $1;
  		util::dprint("((partial: $partial))\n");
	    }
	} 
    }
    if ($partial) {
	# MHonARC makes several empty lines between header and body,
	# so remove them.
	while(@tmp) {
	    last if (! $line =~ /^\s*$/);
	    $line = shift @tmp;
	}
	undef $partial;
	goto HEADER_PROCESSING;
    }
    $$contents = join("\n", @tmp);
    if ($boundary) {
	# MIME �� Multipart  �򤽤�ʤ�˽�������
	$boundary =~ s/(\W)/\\$1/g;
	$$contents =~ s/This is multipart message.\n//i;


	# MIME multipart processing,
	# modified by Furukawa-san's patch on [1998/08/27]
 	$$contents =~ s/--$boundary(--)?\n?/\xff/g;
 	my (@parts) = split(/\xff/, $$contents);
 	$$contents = '';
 	foreach $_ (@parts){
 	    if (s/^(.*?\n\n)//s){
 		my ($head) = $1;
 		$$contents .= $_ if $head =~ m!^content-type:.*text/plain!mi;
 	    }
 	}
    }
}

# Mail/News �ΰ��ѥޡ��������դ���
# �ޤ���Ƭ��̾�������ιԤ䡢������ʬ����������Ͻ񤭤ޤ����ʤɤιԤ�
# ����˴ޤޤ�ʤ��褦�ˤ��� (��ޤ������餵��Υ����ǥ�����ĺ���ޤ���)
sub mailnews_citation_filter ($$) {
    my ($contents, $weighted_str) = @_;

    my $omake = "";
    $$contents =~ s/^\s+//;
    my @tmp = split(/\n/, $$contents);
    $$contents = "";

    # ��Ƭ��̾���Ф���ʬ����� (����Ϻǽ�� 1,2 �Ԥ�ˤ����ʤ��Ǥ��礦)
    for (my $i = 0; $i < 2 && defined($tmp[$i]); $i++) {
	if ($tmp[$i] =~ /(^\s*((([\xa1-\xfe][\xa1-\xfe]){1,8}|([\x21-\x7e]{1,16}))\s*(��|��|\.|��|,|��|\@|��|��)\s*){0,2}\s*(([\xa1-\xfe][\xa1-\xfe]){1,8}|([\x21-\x7e]{1,16}))\s*(�Ǥ�|�ȿ����ޤ�|�Ȥ⤦���ޤ�|�Ȥ����ޤ�)(.{0,2})?\s*$)/) {
	    # �ǥХå����󤫤鸡������ˤ� perl -n00e 'print if /^<<<</'
	    util::dprint("\n\n<<<<$tmp[$i]>>>>\n\n");
	    $omake .= $tmp[$i] . "\n";
	    $tmp[$i] = "";
        }
    }

    # ������ʬ���Υ
    foreach my $line (@tmp) {
	# ��Ƭ�� HTML �������褿���ϰ��ѽ������ʤ�
	if ($line !~ /^[^>]*</ &&
	    $line =~ s/^((\S{1,10}>)|(\s*[\>\|\:\#]+\s*))+//) {
	    $omake .= $line . "\n";
	    $$contents .= "\n";  # ���Ԥ򤤤�褦
	    next;
	}
	$$contents .= $line. "\n";
    }
	
    # �����Ǥ϶��Ԥ���ڤ�ˤ���������פǽ������Ƥ���
    # �֢�������Ϣ����ε����ˤ����Ƣ��������񤭤ޤ����פ��त���Υ
    @tmp = split(/\n\n+/, $$contents);
    $$contents = "";
    my $i = 0;
    foreach my $line (@tmp) {
	# �����˽�������Τ�̵�����Ȼפ��ޤ�������ʤ�Τ��ʤ�
        # ���μ�Υ�å������Ϥ��������ǽ�� 5 ����餤�˴ޤޤ�뤫��
	# �ޤ��� 5 �Ԥ��Ĺ������Ͻ������ʤ���
	# ����ˤ��Ƥ�ʤ�Ȥ��� hairy ����ɽ��������
	if ($i < 5 && ($line =~ tr/\n/\n/) <= 5 && $line =~ /(^\s*(Date:|Subject:|Message-ID:|From:|��̾|���п�|����))|(^.+(�ֻ��Ǥ�|reply\s*�Ǥ�|۩��|���勞|�񤭤ޤ���|�����ޤ���|�ä�|wrote|said|writes|says)(.{0,2})?\s*$)|(^.*In .*(article|message))|(<\S+\@([\w-.]\.)+\w+>)/im) {
	    util::dprint("\n\n<<<<$line>>>>\n\n");
	    $omake .= $line . "\n";
	    $line = "";
	    next;
	}
	$$contents .= $line. "\n\n";
        $i++;
    }
    $$weighted_str .= "\x7f1\x7f$omake\x7f/1\x7f\n";
}


# RFC �ѤΥե��륿
# ���Ƚ񼰤Ϥޤ��ޤ��ߤ��������餽��ʤ��
sub rfc_filter ($$$) {
    my ($contents, $weighted_str, $title) = @_;

    $$contents =~ s/^\s+//s;
    $$contents =~ s/((.+\n)+)\s+(.*)//;
    $$title = $3;
    html::encode_entity($title);
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$$title\x7f/$weight\x7f\n";
    # summary �ޤ��� Introduction������Ф������Ƭ�˻��äƤ���
#    $$contents =~ s/\A(.+?^(\d+\.\s*)?(Abstract|Introduction)\n\n)//ims;
    $$contents =~ s/([\s\S]+^(\d+\.\s*)?(Abstract|Introduction)\n\n)//im;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
}

# man �ѤΥե��륿
# �褯�狼��ʤ����餤���������
sub man_filter ($$$) {
    my ($contents, $weighted_str, $title) = @_;
    my $name = "";

    $$contents =~ s/^\s+//gs;

    $$contents =~ /^(.*?)\s*\S*$/m;
    $$title = "$1";
    html::encode_entity($title);
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$$title\x7f/$weight\x7f\n";

    if ($$contents =~ /^(?:NAME|̾��|̾��)\s*\n(.*?)\n\n/ms) {
	$name = "$1::\n";
	$weight = $conf::Weight{'html'}->{'h1'};
	$$weighted_str .= "\x7f$weight\x7f$1\x7f/$weight\x7f\n";
    }

    if ($$contents =~ 
	s/(.+^(?:DESCRIPTION ����|DESCRIPTIONS?|SHELL GRAMMAR|INTRODUCTION|�ڳ��ס�|����|����|��ǽ����|���ܵ�ǽ����)\s*\n)//ims) 
    {
	$$contents = $name . $$contents;
	$$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    }
}

# uuencode ���ɤ����Ф��롼����ϸ���@��ޥϤ��󤬤�������ޤ�����[1997-09-28]
# �ŤͽŤʹ��դǤ������� BinHex ���ɲä��Ƥ�餤�ޤ��� [1997-11-13]
# �䤬�����ä����Ȥˤ��Х��������Ƥ��������ޤ��� [1998-02-05] Thanks!
sub uuencode_filter ($) {
    my ($contents) = @_;
    my @tmp = split(/\n/, $$contents);
    $$contents = "";
    
    my $uuin = 0;
    while (@tmp) {
	my $line = shift @tmp;
	$line .= "\n";

	# BinHex ���ɤ����Ф�
	# ���ͤ��褯ʬ����ʤ��Τǡ��Ǹ�ޤ����Ф�
	last if $line =~ /^\(This file must be converted with BinHex/; #)

	# uuencode ���ɤ����Ф�
	# ����ʸ�� : SunOS 4.1.4 �� man 5 uuencode
	#            FreeBSD 2.2 �� uuencode.c
        # �����ޥå����Ƥ��ޤä����Υǥ��åȤ򾯤ʤ����뤿��
	# ���ΤΥե����ޥåȥ����å���Ԥʤ�
	#
	# News �ʤɤǥե������ʬ�䤷����Ƥ���Ƥ����Τξ�� begin ���ʤ�
	# ���Ȥ�����ΤǤ�����θ���ޤ� by S.Takabayashi [v1.0.5]
	# �����ޥå����뤳�ȤϤۤȤ�ɤʤ��Ȥϻפ��ޤ�����
	#
	# length �� 62 �� 63 ������ߤ����� [v1.0.5]
	# �⤷��������¾�ˤ�㤤������Τ���
	#
	# ���ͤ���¤�ɽ������ȡ�
	# int((ord($line) - ord(' ') + 2) / 3)
	#     != (length($line) - 2) / 4
	# �Ȥʤ뤬�������ѷ����ơ�
	# 4 * int(ord($line) / 3) != length($line) + $uunumb;

        # SunOS �� uuencode �ϡ�encode �˶����ȤäƤ��롣
        # �������������ǧ���ȡ����̤ιԤ� uuencode �Ԥȸ�ǧ����
        # ��ǽ�����⤯�ʤ롣
        # ����ƤȤ��ơ����Υ�������ǧ��롣
        #     begin �� end �δ�
        #     ���ιԤ� uuencode �Ԥ�Ƚ�Ǥ���ơ�ord �����ιԤ�Ʊ��
	
	# ��Ԥ� 0x20-0x60 ��ʸ���Τߤǹ����������Τ� uuencode 
	# �Ȥߤʤ� v1.1.2.3 (bug fix)

        $uuin = 1, next if $line =~ /^begin [0-7]{3,4} \S+$/;
        if ($line =~ /^end$/){
            $uuin = 0,next if $uuin;
        } else {
            # �����ǡ�ord ���ͤ� 32-95 ���ϰϤ�
	    my $uuord = ord($line);
	    $uuord = 32 if $uuord == 96;

            # uunumb = 38 �ιԤ� loop �γ��˽ФƤ���ȡ�
            # ���̤ιԤ� 63 ʸ���ιԤ����ä����ư��Ƥ��ޤ�
            my $uunumb = (length($line)==63)? 37: 38;

            if ((32 <= $uuord && $uuord < 96) &&
                length($line) <= 63 &&
                (4 * int($uuord / 3) == length($line) + $uunumb)){

                if ($uuin == 1 || $uuin == $uuord){
                    next if $line =~ /^[\x20-\x60]+$/;
                } else {
		    # begin����ϤޤäƤ��ʤ���Τϸ��������褦 [1998-05-22]
                    $uuin = $uuord, next if $line =~ /^M[\x21-\x60]+$/;
                }
            }
        }
        $uuin = 0;
        $$contents .= $line;
    }
}

# ��Ƭ�������ζ��򡢥��֡���Ƭ�� > | # ���� (':' ��Ĥ��ä��� by ����)
# ���������ܸ�ǽ������ϲ��ԥ����ɤ���
# ������ʬ�Υ����ɤϸ���@��ޥϤ��󤬤�������ޤ�����[1997-09-15]
# ��ʸ�ϥ��ե��͡������β���ϻ䤬�դ�­���ޤ���
# 40ʸ��̤���ιԤˤĤ��ƹ��������ܸ�Ϣ�������Ԥ�ʤ��褦�ˤ��� v1.1.1
sub line_adjust_filter ($) {
    my ($text) = @_;
    return undef unless defined($$text);

    my @tmp = split(/\n/, $$text);
    foreach my $line (@tmp) {
	$line .= "\n";
	$line =~ s/^[ \>\|\#\:]+//;
	$line =~ s/ +$//;
	$line =~ s/\n// if (($line =~ /[\xa1-\xfe]\n*$/) &&
			    (length($line) >=40));
	$line =~ s/(��|��)$/$1\n/;
	$line =~ s/([a-z])-\n/$1/;  # for hyphenation.
    }
    $$text = join('', @tmp);
}

# not implimented yet.
sub analize_rcs_stamp()
{
}


1;
