#
# -*- Perl -*-
# $Id: mailnews.pl,v 1.12 1999-08-31 02:29:10 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu All rights reserved.
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

package mailnews;
use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('message/rfc822', 'message/news');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing mail/news file ...\n");

    uuencode_filter($cont);
    mailnews_filter($cont, $weighted_str, $fields);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{title} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

# Mail/News �ѤΥե��륿
# ���Ȥʤ��Τϸ���@��ޥϤ���ˤ��������ޤ���
sub mailnews_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    my $boundary = "";
    my $line     = "";
    my $partial  = 0;

    $$contref =~ s/^\s+//;
    # 1 ���ܤ��إå��äݤ��ʤ��ե�����ϡ��إå��������ʤ�
    return unless $$contref =~ /(^\S+:|^from )/i;

    my @tmp = split(/\n/, $$contref);
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
	    my $title = $line;
	    html::encode_entity(\$title);
	    $fields->{title} = $title;
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
    $$contref = join("\n", @tmp);
    if ($boundary) {
	# MIME �� Multipart  �򤽤�ʤ�˽�������
	$boundary =~ s/(\W)/\\$1/g;
	$$contref =~ s/This is multipart message.\n//i;


	# MIME multipart processing,
	# modified by Furukawa-san's patch on [1998/08/27]
 	$$contref =~ s/--$boundary(--)?\n?/\xff/g;
 	my (@parts) = split(/\xff/, $$contref);
 	$$contref = '';
 	for $_ (@parts){
 	    if (s/^(.*?\n\n)//s){
 		my ($head) = $1;
 		$$contref .= $_ if $head =~ m!^content-type:.*text/plain!mi;
 	    }
 	}
    }
}

# Mail/News �ΰ��ѥޡ��������դ���
# �ޤ���Ƭ��̾�������ιԤ䡢������ʬ����������Ͻ񤭤ޤ����ʤɤιԤ�
# ����˴ޤޤ�ʤ��褦�ˤ��� (��ޤ������餵��Υ����ǥ�����ĺ���ޤ���)
sub mailnews_citation_filter ($$) {
    my ($contref, $weighted_str) = @_;

    my $omake = "";
    $$contref =~ s/^\s+//;
    my @tmp = split(/\n/, $$contref);
    $$contref = "";

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
    for my $line (@tmp) {
	# ��Ƭ�� HTML �������褿���ϰ��ѽ������ʤ�
	if ($line !~ /^[^>]*</ &&
	    $line =~ s/^((\S{1,10}>)|(\s*[\>\|\:\#]+\s*))+//) {
	    $omake .= $line . "\n";
	    $$contref .= "\n";  # ���Ԥ򤤤�褦
	    next;
	}
	$$contref .= $line. "\n";
    }
	
    # �����Ǥ϶��Ԥ���ڤ�ˤ���������פǽ������Ƥ���
    # �֢�������Ϣ����ε����ˤ����Ƣ��������񤭤ޤ����פ��त���Υ
    @tmp = split(/\n\n+/, $$contref);
    $$contref = "";
    my $i = 0;
    for my $line (@tmp) {
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
	$$contref .= $line. "\n\n";
        $i++;
    }
    $$weighted_str .= "\x7f1\x7f$omake\x7f/1\x7f\n";
}

# uuencode ���ɤ����Ф��롼����ϸ���@��ޥϤ��󤬤�������ޤ�����[1997-09-28]
# �ŤͽŤʹ��դǤ������� BinHex ���ɲä��Ƥ�餤�ޤ��� [1997-11-13]
# �䤬�����ä����Ȥˤ��Х��������Ƥ��������ޤ��� [1998-02-05] Thanks!
sub uuencode_filter ($) {
    my ($content) = @_;
    my @tmp = split(/\n/, $$content);
    $$content = "";
    
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
        $$content .= $line;
    }
}


1;
