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

# Mail/News 用のフィルタ
# 元となるものは古川@ヤマハさんにいただきました
sub mailnews_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    my $boundary = "";
    my $line     = "";
    my $partial  = 0;

    $$contref =~ s/^\s+//;
    # 1 行目がヘッダっぽくないファイルは、ヘッダ処理しない
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
	    # ML 特有の [hogehoge-ML:000] を読み飛ばす。
	    # のが意図だが、面倒なので、
	    # 実装上、最初の [...] を読み飛ばす。
	    $line =~ s/^\[.*?\]\s*//;

	    # 'Re:' を読み飛ばす。
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
	# MIME の Multipart  をそれなりに処理する
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

# Mail/News の引用マークを片付ける
# また冒頭の名乗るだけの行や、引用部分、◯◯さんは書きましたなどの行は
# 要約に含まれないようにする (やまだあきらさんのアイディアを頂きました)
sub mailnews_citation_filter ($$) {
    my ($contref, $weighted_str) = @_;

    my $omake = "";
    $$contref =~ s/^\s+//;
    my @tmp = split(/\n/, $$contref);
    $$contref = "";

    # 冒頭の名乗り出る部分を処理 (これは最初の 1,2 行めにしかないでしょう)
    for (my $i = 0; $i < 2 && defined($tmp[$i]); $i++) {
	if ($tmp[$i] =~ /(^\s*((([\xa1-\xfe][\xa1-\xfe]){1,8}|([\x21-\x7e]{1,16}))\s*(。|．|\.|，|,|、|\@|＠|の)\s*){0,2}\s*(([\xa1-\xfe][\xa1-\xfe]){1,8}|([\x21-\x7e]{1,16}))\s*(です|と申します|ともうします|といいます)(.{0,2})?\s*$)/) {
	    # デバッグ情報から検索するには perl -n00e 'print if /^<<<</'
	    util::dprint("\n\n<<<<$tmp[$i]>>>>\n\n");
	    $omake .= $tmp[$i] . "\n";
	    $tmp[$i] = "";
        }
    }

    # 引用部分を隔離
    for my $line (@tmp) {
	# 行頭に HTML タグが来た場合は引用処理しない
	if ($line !~ /^[^>]*</ &&
	    $line =~ s/^((\S{1,10}>)|(\s*[\>\|\:\#]+\s*))+//) {
	    $omake .= $line . "\n";
	    $$contref .= "\n";  # 改行をいれよう
	    next;
	}
	$$contref .= $line. "\n";
    }
	
    # ここでは空行を区切りにした「段落」で処理している
    # 「◯◯さんは△△の記事において□□時頃書きました」の類いを隔離
    @tmp = split(/\n\n+/, $$contref);
    $$contref = "";
    my $i = 0;
    for my $line (@tmp) {
	# 完全に除外するのは無理だと思われます。こんなものかなあ
        # この手のメッセージはせいぜい最初の 5 段落くらいに含まれるかな
	# また、 5 行より長い段落は処理しない。
	# それにしてもなんという hairy 正規表現だろう…
	if ($i < 5 && ($line =~ tr/\n/\n/) <= 5 && $line =~ /(^\s*(Date:|Subject:|Message-ID:|From:|件名|差出人|日時))|(^.+(返事です|reply\s*です|曰く|いわく|書きました|言いました|話で|wrote|said|writes|says)(.{0,2})?\s*$)|(^.*In .*(article|message))|(<\S+\@([\w-.]\.)+\w+>)/im) {
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

# uuencode の読み飛ばしルーチンは古川@ヤマハさんがくださりました。[1997-09-28]
# 重ね重ね感謝です。後日 BinHex も追加してもらいました [1997-11-13]
# 私がいじったことによるバグを修正してくださいました [1998-02-05] Thanks!
sub uuencode_filter ($) {
    my ($content) = @_;
    my @tmp = split(/\n/, $$content);
    $$content = "";
    
    my $uuin = 0;
    while (@tmp) {
	my $line = shift @tmp;
	$line .= "\n";

	# BinHex の読み飛ばし
	# 仕様がよく分からないので、最後まで飛ばす
	last if $line =~ /^\(This file must be converted with BinHex/; #)

	# uuencode の読み飛ばし
	# 参考文献 : SunOS 4.1.4 の man 5 uuencode
	#            FreeBSD 2.2 の uuencode.c
        # 偶然マッチしてしまった場合のデメリットを少なくするため
	# 本体のフォーマットチェックを行なう
	#
	# News などでファイルを分割して投稿されているものの場合 begin がない
	# ことがあるのでそれを考慮します by S.Takabayashi [v1.0.5]
	# 偶然マッチすることはほとんどないとは思いますが…
	#
	# length は 62 と 63 があるみたい… [v1.0.5]
	# もしかしたら他にも違いがあるのかも
	#
	# 仕様を忠実に表現すると、
	# int((ord($line) - ord(' ') + 2) / 3)
	#     != (length($line) - 2) / 4
	# となるが、式を変形して…
	# 4 * int(ord($line) / 3) != length($line) + $uunumb;

        # SunOS の uuencode は、encode に空白も使っている。
        # しかし、空白も認めると、一般の行を uuencode 行と誤認する
        # 可能性が高くなる。
        # 折衷案として、次のケースで認める。
        #     begin と end の間
        #     前の行が uuencode 行と判断されて、ord が前の行と同じ
	
	# 一行が 0x20-0x60 の文字のみで構成される場合のみ uuencode 
	# とみなす v1.1.2.3 (bug fix)

        $uuin = 1, next if $line =~ /^begin [0-7]{3,4} \S+$/;
        if ($line =~ /^end$/){
            $uuin = 0,next if $uuin;
        } else {
            # ここで、ord の値は 32-95 の範囲に
	    my $uuord = ord($line);
	    $uuord = 32 if $uuord == 96;

            # uunumb = 38 の行が loop の外に出ていると、
            # 一般の行で 63 文字の行があったら誤動作してしまう
            my $uunumb = (length($line)==63)? 37: 38;

            if ((32 <= $uuord && $uuord < 96) &&
                length($line) <= 63 &&
                (4 * int($uuord / 3) == length($line) + $uunumb)){

                if ($uuin == 1 || $uuin == $uuord){
                    next if $line =~ /^[\x20-\x60]+$/;
                } else {
		    # beginから始まっていないものは厳しくしよう [1998-05-22]
                    $uuin = $uuord, next if $line =~ /^M[\x21-\x60]+$/;
                }
            }
        }
        $uuin = 0;
        $$content .= $line;
    }
}


1;
