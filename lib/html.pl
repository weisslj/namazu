#
# -*- Perl -*-
# $Id: html.pl,v 1.5 1999-06-12 14:29:27 satoru Exp $
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

# ñ�Ȥ� < > ����λ��Ȥ��Ѵ������ݸ��
# ���ν����� Perl ������ɽ���ִ��λ��ͤˤ�ꡢ��Ť˹Ԥ��ޤ�
sub escape_lt_gt ($) {
    my ($content) = @_;

    $$content =~ s/\s<\s/ &lt; /g;
    $$content =~ s/\s>\s/ &gt; /g;
    $$content =~ s/\s<\s/ &lt; /g;
    $$content =~ s/\s>\s/ &gt; /g;
}

sub get_author ($$) {
    my ($content, $fields) = @_;

    # <LINK REV=MADE HREF="mailto:ccsatoru@vega.aichi-u.ac.jp">

    if ($$content =~ m!<LINK\s[^>]*?HREF=([\"\'])mailto:(.*?)\1>!i) { #"
	    $fields->{'author'} = $2;
    } elsif ($$content =~ m!.*<ADDRESS[^>]*>([^<]*?)</ADDRESS>!i) {
	my $tmp = $1;
#	$tmp =~ s/\s//g;
	if ($tmp =~ /\b([\w\.\-]+\@[\w\.\-]+(?:\.[\w\.\-]+)+)\b/) {
	    $fields->{'author'} = $1;
	}
    }
    
}

# TITLE ����Ф� <TITLE LANG="ja_JP"> �ʤɤˤ��θ���Ƥ��ޤ�
# </TITLE> ����İʾ夢�äƤ������ v1.03
sub get_title ($$$) {
    my ($content, $weighted_str) = @_;
    my $title = "";
    if ($$content =~ s!<TITLE[^>]*>([^<]+)</TITLE>!!i) {
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

# <META NAME="keywords|description" CONTENT="foo bar"> ���б��������
sub get_meta_info ($$) {
    my ($content, $weighted_str) = @_;
    
    my $weight = $conf::Weight{'metakey'};
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$content =~ /<META\s+NAME\s*=\s*([\'\"]?) #"
	    KEYWORDS\1\s+[^>]*CONTENT\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$content =~ /<META\s+NAME\s*=\s*([\'\"]?)DESCRIPTION #"
	    \1\s+[^>]*CONTENT\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"
}

# <IMG ... ALT="foo"> �� foo �μ��Ф�
# HTML �ΰ����ϸ�̩�ǤϤʤ��Ǥ�
sub get_img_alt ($) {
    my ($content) = @_;

    $$content =~ s/<IMG[^>]*\s+ALT\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# pick up foo of <TABLE ... SUMMARY="foo">
sub get_table_summary ($) {
    my ($content) = @_;

    $$content =~ s/<TABLE[^>]*\s+SUMMARY\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# pick up foo of <XXX ... TITLE="foo">
sub get_title_attr ($) {
    my ($content) = @_;

    $$content =~ s/<[A-Z]+[^>]*\s+TITLE\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# <A HREF...> �ʤɤ� <A> ������ (������Ȥ򤹤٤ƺ��)
sub normalize_html_element ($) {
    my ($content) = @_;

    $$content =~ s/<([!\w]+)\s+[^>]*>/<$1>/g;
}

# <BODY> ����򤹤٤ƺ�� (<STYLE> �� <SCRIPT> �ؤ��н�)
# �ʤ��� MSWin32 �� Perl ���� (.|\n)* �Ȥ����
sub erase_above_body ($) {
    my ($content) = @_;

    $$content =~ s/^.*<BODY>//is;
}


# %conf::Weight{'html'} �����ꤵ��Ƥ�����ͤ˱����� \x7fXX\x7f, \x7f/XX\x7f �Ȥ����Ͷ��Υ���
# ���ꡢ����Ƕ���Ǥ��� (\x7f ��ͽ�᤹�٤ƶ�����Ѵ����Ƥ���)
# ñ��Υ�����Ȥκݤˤ��βͶ��Υ��������Ѥ��Ʒ׻�����
# <A> ���ǽ�˽��������褦�� sort keys ���Ƥ��ޤ�(��ľ)��
# �Ȥ����Τ� <A> ��¾�Υ�������¦����뤳�Ȥ�¿������Ǥ�
# ��̩������ҽ����ϹԤäƤ��ޤ���
# ����� <H[1-6]> �ˤĤ��Ƥ���������Τ���˲����������򤷤Ƥ���
sub weight_element ($$$ ) {
    my ($content, $weighted_str, $headings) = @_;

    for my $element (sort keys(%{$conf::Weight{'html'}})) {
	my $tmp = "";
	$$content =~ s!<($element)>(.*?)</$element>!weight_element_sub($1, $2, \$tmp)!gies;
	$$headings .= $tmp if $element =~ /^H[1-6]$/i && ! $conf::NoHeadAbstOpt 
	    && $tmp;
	my $weight = $element =~ /^H[1-6]$/i && ! $conf::NoHeadAbstOpt ? 
	    $conf::Weight{'html'}->{$element} : $conf::Weight{'html'}->{$element} - 1;
	$$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n" if $tmp;
    }
}

sub weight_element_sub ($$$) {
    my ($element, $text, $tmp) = @_;

    my $space = element_space($element);
    $text =~ s/<[^>]*>//g;
    $$tmp .= "$text " if (length($text)) < $conf::INVALID_LENG;
    $element =~ /^H[1-6]$/i && ! $conf::NoHeadAbstOpt  ? " " : "$space$text$space";
}


# determine whether a given element should be delete or be substituted with space
sub element_space ($) {
    $_[0] =~ /^($conf::NON_SEPARATION_ELEMENTS)$/io ? "" : " ";
}

# remove all HTML elements. it's not perfect but almost works.
sub remove_html_elements ($) {
    my ($content) = @_;

    # remove all comments
    $$content =~ s/<!--.*?-->//gs;

    # remove all elements
    $$content =~ s!</?([A-Z]\w*)(?:\s+[A-Z]\w*(?:\s*=\s*(?:(["']).*?\2|[\w-.]+))?)*\s*>!element_space($1)!gsixe;

}

# numberd entity ��������Ԥ� /  ̵�����ͤϤϤ���
sub decode_numbered_entity ($) {
    my ($num) = @_;
    return ""
	if $num >= 0 && $num <= 8 ||  $num >= 11 && $num <= 31 || $num >=127;
    sprintf ("%c",$num);
}


# ���λ��Ȥ����� ISO-8859-1 �α�Ⱦʬ��̵�뤷�ޤ�
# HTML 2.x �ǳ�ĥ���줿 numbered entity �ˤ�̤�б��Ǥ�
# �ɤ�������ܸ� EUC �Ǥ�̵���ʤΤǤ�
# &quot &lt &gt; �Τ褦�˶����³���ƺǸ�� ; ��Ĥ��뵭�Ҥ������ v1.03
sub decode_entity ($) {
    my ($text) = @_;

    return unless defined($$text);

    $$text =~ s/&#(\d{2,3})[;\s]/decode_numbered_entity($1)/ge;
    $$text =~ s/&quot[;\s]/\"/g; #"
    $$text =~ s/&amp[;\s]/&/g;
    $$text =~ s/&lt[;\s]/</g;
    $$text =~ s/&gt[;\s]/>/g;
    $$text =~ s/&nbsp/ /g; ## ���̰��� v1.1.2.1
}


# '<' �� '>' '&' ����λ��Ȥ��Ѵ�
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

# Robots.txt ���ɤ߹���
# Disallow �԰ʳ��Ϥ��ä���̵��
#
# This code was contributed by 
#  - [Gorochan ^o^ <kunito@hal.t.u-tokyo.ac.jp>]
#
sub parse_robots_txt () {
    if (not -f "$conf::ROBOTS_TXT") {
	warn "$conf::ROBOTS_TXT does not exists";
	$conf::ROBOTS_EXCLUDE_URIS="\t";
	return 0;
    }

    my $fh_robottxt = util::fopen_or_die($conf::ROBOTS_TXT);
    while(<$fh_robottxt>){
	/^\s*Disallow:\s*(\S+)/i && do {
	    my $uri = $1;
	    $uri =~ s/\%/%25/g;  # ������ޤޤ�� % �� %25 ���ѹ� v1.1.1.2
	    $uri =~ s/([^a-zA-Z0-9\-\_\.\/\:\%])/
		sprintf("%%%02X",ord($1))/ge;
	    if (($namazu::SYSTEM eq "MSWin32") || ($namazu::SYSTEM eq "os2")) {
		# restore '|' for drive letter rule of Win32, OS/2
		$uri =~ s!^/([A-Z])%7C!/$1|!i;
	    }
	    $conf::ROBOTS_EXCLUDE_URIS .= "^$conf::HTDOCUMENT_ROOT_URI_PREFIX$uri|";
	}
    }
    chop($conf::ROBOTS_EXCLUDE_URIS);
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
	$err = $!,  $CWD = cwd() , die "$CWD/.htaccess : $err\n";
    while(<$fh>) {
	s/^\#.*$//;
	/^\s*$/ && next;
	/^\s*deny\s+|require\s+(valid-user|user|group)([^\w]+|$)/i && do {
	    $r=1;
	    last;
	};
    }
    return($r);
}


1;

