#
# -*- Perl -*-
# $Id: mailnews.pl,v 1.5 1999-08-28 00:07:39 satoru Exp $
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
#require 'filter.pl';

sub mediatype() {
    return ('message/rfc822', 'message/news');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub filter ($$$$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields, $size)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    print "Proccessing mail/news file ...\n"
      if ($conf::VerboseOpt);

    filter::uuencode_filter($cont);
    mailnews_filter($cont, $weighted_str, $fields);

    filter::line_adjust_filter($cont) unless $conf::NoLineAdOpt;
    filter::line_adjust_filter($weighted_str) unless $conf::NoLineAdOpt;
    filter::white_space_adjust_filter($cont);
    $fields->{title} = filter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    filter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

# Mail/News �ѤΥե��륿
# ���Ȥʤ��Τϸ���@��ޥϤ���ˤ��������ޤ���
sub mailnews_filter ($$$) {
    my ($contents, $weighted_str, $fields) = @_;

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

1;
