#
# -*- Perl -*-
# $Id: html.pl,v 1.12 1999-08-30 09:19:27 satoru Exp $
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

package html;
use strict;
require 'gfilter.pl';

sub mediatype() {
    return ('text/html');
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

    util::vprint("Processing html file ...\n");

    html_filter($cont, $weighted_str, $fields, $headings);
    
    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{title} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{title};
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
}

# HTML 用のフィルタ
sub html_filter ($$$$) {
    my ($contref, $weighted_str, $fields, $headings) = @_;

    html::escape_lt_gt($contref);
    $fields->{title} = html::get_title($contref, $weighted_str);
    html::get_author($contref, $fields);
    html::get_meta_info($contref, $weighted_str);
    html::get_img_alt($contref);
    html::get_table_summary($contref);
    html::get_title_attr($contref);
    html::normalize_html_element($contref);
    html::erase_above_body($contref);
    html::weight_element($contref, $weighted_str, $headings);
    html::remove_html_elements($contref);
    # それぞれ実体参照の復元
    html::decode_entity($contref);
    html::decode_entity($weighted_str);
    html::decode_entity($headings);
}

# 単独の < > を実体参照に変換し、保護する
# この処理は Perl の正規表現置換の仕様により、二重に行います
sub escape_lt_gt ($) {
    my ($contref) = @_;

    $$contref =~ s/\s<\s/ &lt; /g;
    $$contref =~ s/\s>\s/ &gt; /g;
    $$contref =~ s/\s<\s/ &lt; /g;
    $$contref =~ s/\s>\s/ &gt; /g;
}

sub get_author ($$) {
    my ($contref, $fields) = @_;

    # <LINK REV=MADE HREF="mailto:ccsatoru@vega.aichi-u.ac.jp">

    if ($$contref =~ m!<LINK\s[^>]*?HREF=([\"\'])mailto:(.*?)\1>!i) { #"
	    $fields->{'author'} = $2;
    } elsif ($$contref =~ m!.*<ADDRESS[^>]*>([^<]*?)</ADDRESS>!i) {
	my $tmp = $1;
#	$tmp =~ s/\s//g;
	if ($tmp =~ /\b([\w\.\-]+\@[\w\.\-]+(?:\.[\w\.\-]+)+)\b/) {
	    $fields->{'author'} = $1;
	}
    }
    
}

# TITLE を取り出す <TITLE LANG="ja_JP"> などにも考慮しています
# </TITLE> が二つ以上あっても大丈夫 v1.03
sub get_title ($$) {
    my ($contref, $weighted_str) = @_;
    my $title = '';
    
    if ($$contref =~ s!<TITLE[^>]*>([^<]+)</TITLE>!!i) {
	$title = $1;
	$title =~ s/\s+/ /g;
	$title =~ s/^\s+//;
	$title =~ s/\s+$//;
	my $weight = $conf::Weight{'html'}->{'title'};
	$$weighted_str .= "\x7f$weight\x7f$title\x7f/$weight\x7f\n";
    } else {
	$title = $conf::NO_TITLE;
    }

    return $title;
}

# <META NAME="keywords|description" CONTENT="foo bar"> に対応する処理
sub get_meta_info ($$) {
    my ($contref, $weighted_str) = @_;
    
    my $weight = $conf::Weight{'metakey'};
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$contref =~ /<META\s+NAME\s*=\s*([\'\"]?) #"
	    KEYWORDS\1\s+[^>]*CONTENT\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$contref =~ /<META\s+NAME\s*=\s*([\'\"]?)DESCRIPTION #"
	    \1\s+[^>]*CONTENT\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"
}

# <IMG ... ALT="foo"> の foo の取り出し
# HTML の扱いは厳密ではないです
sub get_img_alt ($) {
    my ($contref) = @_;

    $$contref =~ s/<IMG[^>]*\s+ALT\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# <TABLE ... SUMMARY="foo"> の foo の取り出し
sub get_table_summary ($) {
    my ($contref) = @_;

    $$contref =~ s/<TABLE[^>]*\s+SUMMARY\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# <XXX ... TITLE="foo"> の foo の取り出し
sub get_title_attr ($) {
    my ($contref) = @_;

    $$contref =~ s/<[A-Z]+[^>]*\s+TITLE\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# <A HREF...> などを <A> に統一 (エレメントをすべて削除)
sub normalize_html_element ($) {
    my ($contref) = @_;

    $$contref =~ s/<([!\w]+)\s+[^>]*>/<$1>/g;
}

# <BODY> より上をすべて削除 (<STYLE> や <SCRIPT> への対処)
# なぜか MSWin32 の Perl だと (.|\n)* とすると
sub erase_above_body ($) {
    my ($contref) = @_;

    $$contref =~ s/^.*<BODY>//is;
}


# %conf::Weight{'html'} に設定されている数値に応じて \x7fXX\x7f, \x7f/XX\x7f という架空のタグ
# を作り、これで挟んでおく (\x7f は予めすべて空白に変換してある)
# 単語のカウントの際にこの架空のタグを利用して計算する
# <A> が最初に処理されるように sort keys しています(安直)。
# というのは <A> は他のタグの内側に来ることが多いからです
# 厳密な入れ子処理は行っていません
# さらに <H[1-6]> については要約作成のために怪しい処理をしている
sub weight_element ($$$ ) {
    my ($contref, $weighted_str, $headings) = @_;

    for my $element (sort keys(%{$conf::Weight{'html'}})) {
	my $tmp = "";
	$$contref =~ s!<($element)>(.*?)</$element>!weight_element_sub($1, $2, \$tmp)!gies;
	$$headings .= $tmp if $element =~ /^H[1-6]$/i && ! $var::Opt{NoHeadAbst} 
	    && $tmp;
	my $weight = $element =~ /^H[1-6]$/i && ! $var::Opt{NoHeadAbst} ? 
	    $conf::Weight{'html'}->{$element} : $conf::Weight{'html'}->{$element} - 1;
	$$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n" if $tmp;
    }
}

sub weight_element_sub ($$$) {
    my ($element, $text, $tmp) = @_;

    my $space = element_space($element);
    $text =~ s/<[^>]*>//g;
    $$tmp .= "$text " if (length($text)) < $conf::INVALID_LENG;
    $element =~ /^H[1-6]$/i && ! $var::Opt{NoHeadAbst}  ? " " : "$space$text$space";
}


# determine whether a given element should be delete or be substituted with space
sub element_space ($) {
    $_[0] =~ /^($conf::NON_SEPARATION_ELEMENTS)$/io ? "" : " ";
}

# remove all HTML elements. it's not perfect but almost works.
sub remove_html_elements ($) {
    my ($contref) = @_;

    # remove all comments
    $$contref =~ s/<!--.*?-->//gs;

    # remove all elements
    $$contref =~ s!</?([A-Z]\w*)(?:\s+[A-Z]\w*(?:\s*=\s*(?:(["']).*?\2|[\w-.]+))?)*\s*>!element_space($1)!gsixe;

}

# numberd entity の復元を行う /  無効な値ははじく
sub decode_numbered_entity ($) {
    my ($num) = @_;
    return ""
	if $num >= 0 && $num <= 8 ||  $num >= 11 && $num <= 31 || $num >=127;
    sprintf ("%c",$num);
}


# 実体参照の復元 ISO-8859-1 の右半分は無視します
# HTML 2.x で拡張された numbered entity には未対応です
# どちらも日本語 EUC では無理なのです
# &quot &lt &gt; のように空白で続けて最後に ; をつける記述も大丈夫 v1.03
sub decode_entity ($) {
    my ($text) = @_;

    return unless defined($$text);

    $$text =~ s/&#(\d{2,3})[;\s]/decode_numbered_entity($1)/ge;
    $$text =~ s/&quot[;\s]/\"/g; #"
    $$text =~ s/&amp[;\s]/&/g;
    $$text =~ s/&lt[;\s]/</g;
    $$text =~ s/&gt[;\s]/>/g;
    $$text =~ s/&nbsp/ /g; ## 特別扱い v1.1.2.1
}


# '<' と '>' '&' を実体参照へ変換
sub encode_entity ($) {
    my ($tmp) = @_;

    $$tmp =~ s/&/&amp;/g;    # &amp; should be processed first
    $$tmp =~ s/</&lt;/g;
    $$tmp =~ s/>/&gt;/g;
    $$tmp;
}

# determine wheter the target file is html or not
sub is_html ($) {
    my ($cfile) = @_;

    return ($cfile =~ /\.($conf::HTML_SUFFIX)$/io || 
    $cfile =~ /($conf::DEFAULT_FILE)$/o || 
    $cfile =~ /\?/ || 
    $cfile =~ /($conf::CGI_DIR)/io);
}

# Robots.txt の読み込み
# Disallow 行以外はいっさい無視
#
# This code was contributed by 
#  - [Gorochan ^o^ <kunito@hal.t.u-tokyo.ac.jp>]
#
sub parse_robots_txt () {
    if (not -f "$conf::ROBOTS_TXT") {
	warn "$conf::ROBOTS_TXT does not exists";
	$conf::ROBOTS_EXCLUDE_URLS="\t";
	return 0;
    }

    my $fh_robottxt = util::efopen($conf::ROBOTS_TXT);
    while(<$fh_robottxt>){
	/^Disallow:\s*(\S+)/i && do {
	    my $url = $1;
	    $url =~ s/\%/%25/g;  # 元から含まれる % は %25 に変更 v1.1.1.2
	    $url =~ s/([^a-zA-Z0-9\-\_\.\/\:\%])/
		sprintf("%%%02X",ord($1))/ge;
	    if (($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")) {
		# restore '|' for drive letter rule of Win32, OS/2
		$url =~ s!^/([A-Z])%7C!/$1|!i;
	    }
	    $conf::ROBOTS_EXCLUDE_URLS .= "^$conf::HTDOCUMENT_ROOT_URL_PREFIX$url|";
	}
    }
    chop($conf::ROBOTS_EXCLUDE_URLS);
}



#
# Added by G.Kunito <kunito@hal.t.u-tokyo.ac.jp>
# slightly modified by <satoru@isoternet.org> [1999-01-30]
#
# check a ".htaccess" to make sure of the access restricted files
# return(1) if deny, require valid-user, user, group directives are set
# 
sub parse_htaccess () {
    my $inLimit = 0;
    my $err;
    my $r = 0;
    my $CWD;

    my $fh = util::fopen(".htaccess") or 
	$err = $!,  $CWD = cwd() , util::cdie("$CWD/.htaccess : $err\n");
    while(<$fh>) {
	s/^\#.*$//;
	/^\s*$/ && next;
	/^\s*deny\s+|require\s+(valid-user|usr|group)([^\w]+|$)/i && do {
	    $r=1;
	    last;
	};
    }
    return($r);
}


1;

