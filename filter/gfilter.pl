#
# -*- Perl -*-
# $Id: gfilter.pl,v 1.6 1999-08-31 15:01:23 satoru Exp $
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

