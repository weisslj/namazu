#
# -*- CPerl -*-
# $Id: gfilter.pl,v 1.3 1999-08-30 03:13:17 satoru Exp $
# Copyright (C) 1999 Satoru Takabayashi ,
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

package gfilter;

# Show debug information for filters
sub show_filter_debug_info ($$$$) {
    my ($contref, $weighted_str, $fields, $headings) = @_;
    util::dprint("-- title --\n$fields->{title}\n");
    util::dprint("-- content --\n$$contref\n");
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
sub filename_to_title ($$) {
    my ($cfile, $weighted_str) = @_;

    # for MSWin32's filename using Shift_JIS [1998-09-24]
    if (($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")) {
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

    my $title = $filename . $conf::TEXT_TITLE;
    return $title
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

# 行頭・行末の空白、タブ、行頭の > | # を削除 (':' もつけ加えた by 高林)
# 行末が日本語で終わる場合は改行コードを削除
# この部分のコードは古川@ヤマハさんがくださりました。[1997-09-15]
# 英文ハイフォネーションの解除は私が付け足しました
# 40文字未満の行について行末の日本語連結処理を行わないようにした v1.1.1
sub line_adjust_filter ($) {
    my ($text) = @_;
    return undef unless defined($$text);

    my @tmp = split(/\n/, $$text);
    for my $line (@tmp) {
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

