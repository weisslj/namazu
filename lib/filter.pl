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

# ファイル名からタイトルを取得 (単なるテキストファイルの場合)
sub filename_to_title ($\$\$) {
    my ($cfile, $title, $weighted_str) = @_;

    # for MSWin32's filename using Shift_JIS [1998-09-24]
    if (($namazu::SYSTEM eq "MSWin32") || ($namazu::SYSTEM eq "os2")) {
	$cfile = codeconv::shiftjis_to_eucjp($cfile);
    }
    
    $cfile =~ m!^.*/([^/]*)$!;
    my $filename = $1;
    # ファイル名を元にキーワードを割り出してみる v1.1.1
    # modified [1998-09-18] 
    my $tmp = $filename;
    $tmp =~ s|/\\_\.-| |g;

    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n";

    $$title = $filename . $conf::TEXT_TITLE;
}

# HTML 用のフィルタ
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
    # それぞれ実体参照の復元
    html::decode_entity($contents);
    html::decode_entity($weighted_str);
    html::decode_entity($headings);
}

# MHonArc 用のフィルタ
# MHonArc v2.1.0 が標準で出力する HTML を想定しています
sub mhonarc_filter ($$) {
    my ($contents, $weighted_str) = @_;

    # MHonArc を使うときはこんな感じに処理すると便利
    $$contents =~ s/<!--X-MsgBody-End-->.*//s;
    $$contents =~ s/<!--X-TopPNI-->.*<!--X-TopPNI-End-->//s;
    $$contents =~ s/<!--X-Subject-Header-Begin-->.*<!--X-Subject-Header-End-->//s;
    $$contents =~ s/<!--X-Head-Body-Sep-Begin-->/\n/;  # ヘッダと本文を区切る
    $$contents =~ s/^<LI>//gim;   # ヘッダの前に空白をあけたくないから
    $$contents =~ s!</?EM>!!gi;  # ヘッダの名前をインデックスにいれたくない
    $$contents =~ s/^\s+//;
}

# Mail/News 用のフィルタ
# 元となるものは古川@ヤマハさんにいただきました
sub mailnews_filter ($$$\%) {
    my ($contents, $weighted_str, $title, $fields) = @_;

    my $boundary = "";
    my $line     = "";
    my $partial  = 0;

    $$contents =~ s/^\s+//;
    # 1 行目がヘッダっぽくないファイルは、ヘッダ処理しない
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
    $$contents = join("\n", @tmp);
    if ($boundary) {
	# MIME の Multipart  をそれなりに処理する
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

# Mail/News の引用マークを片付ける
# また冒頭の名乗るだけの行や、引用部分、◯◯さんは書きましたなどの行は
# 要約に含まれないようにする (やまだあきらさんのアイディアを頂きました)
sub mailnews_citation_filter ($$) {
    my ($contents, $weighted_str) = @_;

    my $omake = "";
    $$contents =~ s/^\s+//;
    my @tmp = split(/\n/, $$contents);
    $$contents = "";

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
    foreach my $line (@tmp) {
	# 行頭に HTML タグが来た場合は引用処理しない
	if ($line !~ /^[^>]*</ &&
	    $line =~ s/^((\S{1,10}>)|(\s*[\>\|\:\#]+\s*))+//) {
	    $omake .= $line . "\n";
	    $$contents .= "\n";  # 改行をいれよう
	    next;
	}
	$$contents .= $line. "\n";
    }
	
    # ここでは空行を区切りにした「段落」で処理している
    # 「◯◯さんは△△の記事において□□時頃書きました」の類いを隔離
    @tmp = split(/\n\n+/, $$contents);
    $$contents = "";
    my $i = 0;
    foreach my $line (@tmp) {
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
	$$contents .= $line. "\n\n";
        $i++;
    }
    $$weighted_str .= "\x7f1\x7f$omake\x7f/1\x7f\n";
}


# RFC 用のフィルタ
# わりと書式はまちまちみたいだからそれなりに
sub rfc_filter ($$$) {
    my ($contents, $weighted_str, $title) = @_;

    $$contents =~ s/^\s+//s;
    $$contents =~ s/((.+\n)+)\s+(.*)//;
    $$title = $3;
    html::encode_entity($title);
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$$title\x7f/$weight\x7f\n";
    # summary または Introductionがあればそれを先頭に持ってくる
#    $$contents =~ s/\A(.+?^(\d+\.\s*)?(Abstract|Introduction)\n\n)//ims;
    $$contents =~ s/([\s\S]+^(\d+\.\s*)?(Abstract|Introduction)\n\n)//im;
    $$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
}

# man 用のフィルタ
# よくわからないからいいかげんに
sub man_filter ($$$) {
    my ($contents, $weighted_str, $title) = @_;
    my $name = "";

    $$contents =~ s/^\s+//gs;

    $$contents =~ /^(.*?)\s*\S*$/m;
    $$title = "$1";
    html::encode_entity($title);
    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$$title\x7f/$weight\x7f\n";

    if ($$contents =~ /^(?:NAME|名前|名称)\s*\n(.*?)\n\n/ms) {
	$name = "$1::\n";
	$weight = $conf::Weight{'html'}->{'h1'};
	$$weighted_str .= "\x7f$weight\x7f$1\x7f/$weight\x7f\n";
    }

    if ($$contents =~ 
	s/(.+^(?:DESCRIPTION 解説|DESCRIPTIONS?|SHELL GRAMMAR|INTRODUCTION|【概要】|解説|説明|機能説明|基本機能説明)\s*\n)//ims) 
    {
	$$contents = $name . $$contents;
	$$weighted_str .= "\x7f1\x7f$1\x7f/1\x7f\n";
    }
}

# uuencode の読み飛ばしルーチンは古川@ヤマハさんがくださりました。[1997-09-28]
# 重ね重ね感謝です。後日 BinHex も追加してもらいました [1997-11-13]
# 私がいじったことによるバグを修正してくださいました [1998-02-05] Thanks!
sub uuencode_filter ($) {
    my ($contents) = @_;
    my @tmp = split(/\n/, $$contents);
    $$contents = "";
    
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
        $$contents .= $line;
    }
}

# 行頭・行末の空白、タブ、行頭の > | # を削除 (':' もつけ加えた by 高林)
# 行末が日本語で終わる場合は改行コードを削除
# この部分のコードは古川@ヤマハさんがくださりました。[1997-09-15]
# 英文ハイフォネーションの解除は私が付け足しました
# 40文字未満の行について行末の日本語連結処理を行わないようにした v1.1.1
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
	$line =~ s/(。|、)$/$1\n/;
	$line =~ s/([a-z])-\n/$1/;  # for hyphenation.
    }
    $$text = join('', @tmp);
}

# not implimented yet.
sub analize_rcs_stamp()
{
}


1;
