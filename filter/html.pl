#
# -*- Perl -*-
# $Id: html.pl,v 1.25 2000-02-16 07:35:30 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000 Namazu Project All rights reserved.
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

sub pre_codeconv() {
    return 1;
}

sub post_codeconv () {
    return 0;
}

sub add_magic ($) {
    return;
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
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}

sub html_filter ($$$$) {
    my ($contref, $weighted_str, $fields, $headings) = @_;

    html::escape_lt_gt($contref);
    $fields->{'title'} = html::get_title($contref, $weighted_str);
    html::get_author($contref, $fields);
    html::get_meta_tags($contref, $weighted_str, $fields);
    html::get_img_alt($contref);
    html::get_table_summary($contref);
    html::get_title_attr($contref);
    html::normalize_html_element($contref);
    html::erase_above_body($contref);
    html::weight_element($contref, $weighted_str, $headings);
    html::remove_html_elements($contref);
    # restore entities of each content.
    html::decode_entity($contref);
    html::decode_entity($weighted_str);
    html::decode_entity($headings);
}

# Convert independent < > s into entity references for escaping.
# Substitute twice for safe.
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


# Get title from <title>..</title>
# It's okay to exits two or more <title>...</TITLE>. 
# First one will be retrieved.
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

# get foo bar from <META NAME="keywords|description" CONTENT="foo bar"> 
sub get_meta_tags ($$$) {
    my ($contref, $weighted_str, $fields) = @_;
    
    # <meta name="keywords" content="foo bar baz">

    my $weight = $conf::Weight{'metakey'};
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$contref =~ /<meta\s+name\s*=\s*([\'\"]?) #"
	    keywords\1\s+[^>]*content\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"

    # <meta name="description" content="foo bar baz">
    $$weighted_str .= "\x7f$weight\x7f$3\x7f/$weight\x7f\n" 
	if $$contref =~ /<meta\s+name\s*=\s*([\'\"]?)description #"
	    \1\s+[^>]*content\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/ix; #"

    if ($var::Opt{'meta'}) {
	my @keys = split '\|', $conf::META_TAGS;
	for my $key (@keys) {
	    while ($$contref =~ /<meta\s+name\s*=\s*([\'\"]?)$key #"
	       \1\s+[^>]*content\s*=\s*([\'\"]?)([^>]*?)\2[^>]*>/gix) 
	    {
		$fields->{$key} .= $3 . " ";
	    }
	    util::dprint("meta: $key: $fields->{$key}\n") 
		if defined $fields->{$key};
	}
    }
}

# Get foo from <IMG ... ALT="foo">
# It's not to handle HTML strictly.
sub get_img_alt ($) {
    my ($contref) = @_;

    $$contref =~ s/<IMG[^>]*\s+ALT\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# Get foo from <TABLE ... SUMMARY="foo">
sub get_table_summary ($) {
    my ($contref) = @_;

    $$contref =~ s/<TABLE[^>]*\s+SUMMARY\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# Get foo from <XXX ... TITLE="foo">
sub get_title_attr ($) {
    my ($contref) = @_;

    $$contref =~ s/<[A-Z]+[^>]*\s+TITLE\s*=\s*[\"\']?([^\"\']*)[\"\']?[^>]*>/ $1 /gi; #"
}

# Normalize elements like: <A HREF...> -> <A>
sub normalize_html_element ($) {
    my ($contref) = @_;

    $$contref =~ s/<([!\w]+)\s+[^>]*>/<$1>/g;
}

# Remove contents above <body>.
sub erase_above_body ($) {
    my ($contref) = @_;

    $$contref =~ s/^.*<body>//is;
}


# Weight a score of a keyword in a given text using %conf::Weight hash.
# This process make the text be surround by temporary tags 
# \x7fXX\x7f and \x7f/XX\x7f. XX represents score.
# Sort keys of %conf::Weight for processing <a> first.
# Because <a> has a tendency to be inside of other tags.
# Thus, it does'not processing for nexted tags strictly.
# Moreover, it does special processing for <h[1-6]> for summarization.
sub weight_element ($$$ ) {
    my ($contref, $weighted_str, $headings) = @_;

    for my $element (sort keys(%{$conf::Weight{'html'}})) {
	my $tmp = "";
	$$contref =~ s!<($element)>(.*?)</$element>!weight_element_sub($1, $2, \$tmp)!gies;
	$$headings .= $tmp if $element =~ /^H[1-6]$/i && ! $var::Opt{'NoHeadAbst'} 
	    && $tmp;
	my $weight = $element =~ /^H[1-6]$/i && ! $var::Opt{'NoHeadAbst'} ? 
	    $conf::Weight{'html'}->{$element} : $conf::Weight{'html'}->{$element} - 1;
	$$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n" if $tmp;
    }
}

sub weight_element_sub ($$$) {
    my ($element, $text, $tmp) = @_;

    my $space = element_space($element);
    $text =~ s/<[^>]*>//g;
    $$tmp .= "$text " if (length($text)) < $conf::INVALID_LENG;
    $element =~ /^H[1-6]$/i && ! $var::Opt{'NoHeadAbst'}  ? " " : "$space$text$space";
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

# Decode a numberd entity. Exclude an invalid number.
sub decode_numbered_entity ($) {
    my ($num) = @_;
    return ""
	if $num >= 0 && $num <= 8 ||  $num >= 11 && $num <= 31 || $num >=127;
    sprintf ("%c",$num);
}


# Decode an entity. Ignore characters of right half of ISO-8859-1.
# Because it can't be handled in EUC encoding.
# This function provides sequential entities like: &quot &lt &gt;
sub decode_entity ($) {
    my ($text) = @_;

    return unless defined($$text);

    $$text =~ s/&#(\d{2,3})[;\s]/decode_numbered_entity($1)/ge;
    $$text =~ s/&quot[;\s]/\"/g; #"
    $$text =~ s/&amp[;\s]/&/g;
    $$text =~ s/&lt[;\s]/</g;
    $$text =~ s/&gt[;\s]/>/g;
    $$text =~ s/&nbsp/ /g; ## special handling v1.1.2.1
}


# encode entities: only '<', '>', and '&'
sub encode_entity ($) {
    my ($tmp) = @_;

    $$tmp =~ s/&/&amp;/g;    # &amp; should be processed first
    $$tmp =~ s/</&lt;/g;
    $$tmp =~ s/>/&gt;/g;
    $$tmp;
}

# Handle robots.txt
# Ignore all lines except Disallow:.
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

    my $fh_robottxt = util::efopen($conf::ROBOTS_TXT);
    while(defined(my $line = <$fh_robottxt>)) {
	$line =~ /^Disallow:\s*(\S+)/i && do {
	    my $uri = $1;
	    $uri =~ s/\%/%25/g;  # Replace original % with %25  v1.1.1.2
	    $uri =~ s/([^a-zA-Z0-9\-\_\.\/\:\%])/
		sprintf("%%%02X",ord($1))/ge;
	    if (($mknmz::SYSTEM eq "MSWin32") || ($mknmz::SYSTEM eq "os2")) {
		# restore '|' for drive letter rule of Win32, OS/2
		$uri =~ s!^/([A-Z])%7C!/$1|!i;
	    }
	    $conf::ROBOTS_EXCLUDE_URIS .= 
		"^$conf::HTDOCUMENT_ROOT_URI_PREFIX$uri|";
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
	$err = $!,  $CWD = cwd() , util::cdie("$CWD/.htaccess : $err\n");
    while(defined(my $line = <$fh>)) {
	$line =~ s/^\#.*$//;
	$line =~ /^\s*$/ && next;
	$line =~ /^\s*deny\s+|require\s+(valid-user|usr|group)([^\w]+|$)/i && 
        do {
	    $r=1;
	    last;
	};
    }
    return $r;
}


1;

