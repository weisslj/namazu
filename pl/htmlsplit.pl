#
# -*- Perl -*-
# $Id: htmlsplit.pl,v 1.4 2000-03-04 11:11:37 satoru Exp $
#
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

package htmlsplit;
require "util.pl";

use strict;

my $Header = << 'EOS';
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<link rev=made href="mailto:${author}">
<title>${subject}</title>
</head>
<body>
<hr>
EOS

    my $Footer = << 'EOS';
<hr>
</body>
</html>
EOS

sub split ($$) {
    my ($fname, $base) = @_;

    my $fh = util::efopen($fname);
    my $cont   = join '', <$fh>;

    my %info = (
		'title'    => get_title(\$cont),
		'author'   => get_author(\$cont),
		'anchored' => "",
		'base'     => $base,
		'names'    => [],
		);

    my $id = 0;
#    $cont =~ s/(<a\s[^>]*href=(["']))#(.+?)(\2[^>]*>)/$1$3.html$4/gi; #'
    $cont =~ s {
                \G(.+?)                                      # 1
	        (<h([1-6])>)?\s*                             # 2, 3
                <a\s[^>]*name=(["'])(.+?)\4[^>]*>(.*?)</a>   # 4,5,6
                \s*(</h\3>)?                                 # 7
             } {
                write_partial_file($1, $5, $6, $id++, \%info)
             }sgexi;
    write_partial_file($cont, "", "", $id, \%info);

    return @{$info{'names'}};
}

sub get_title ($) {
    my ($contref) = @_;
    my $title = undef;
    
    if ($$contref =~ s!<TITLE[^>]*>([^<]+)</TITLE>!!i) {
	$title = $1;
	$title =~ s/\s+/ /g;
	$title =~ s/^\s+//;
	$title =~ s/\s+$//;
    } else {
	$title = "no title";
    }

    return $title;
}

sub get_author ($) {
    my ($contref) = @_;

    my $author = "unknown";

    # <LINK REV=MADE HREF="mailto:ccsatoru@vega.aichi-u.ac.jp">

    if ($$contref =~ m!<LINK\s[^>]*?HREF=([\"\'])mailto:(.*?)\1\s*>!i) { #"
	$author = $2;
    } elsif ($$contref =~ m!.*<ADDRESS[^>]*>([^<]*?)</ADDRESS>!i) {
	my $tmp = $1;
	if ($tmp =~ /\b([\w\.\-]+\@[\w\.\-]+(?:\.[\w\.\-]+)+)\b/) {
	    $author = $1;
	}
    }
    return $author;
}

sub write_partial_file($$$$$) {
    my ($cont, $name, $anchored, $id, $info_ref) = @_;

    my $author        = $info_ref->{'author'};
    my $base          = $info_ref->{'base'};
    my $orig_title    = $info_ref->{'title'};
    my $prev_name     = $info_ref->{'name'};
    my $prev_anchored = $info_ref->{'anchored'};

    my $fname = util::tmpnam("$base.$id");

    my $fh = util::efopen(">$fname");

    my $header = $Header;
    my $title = $prev_anchored ? "$orig_title: $prev_anchored" : $orig_title;
    $header =~ s/\$\{subject\}/$title/g;
    $header =~ s/\$\{author\}/$author/g;
    print $fh $header;
    print $fh $cont;

    my $footer = $Footer;
    print $fh $footer;

    push @{$info_ref->{'names'}}, $prev_name;
    $info_ref->{'anchored'} = $anchored;
    $info_ref->{'name'} = $name;

    # FIXME: Actually we don't need this. 
    #        But some perl versions need this.
    close($fh);

    return "";
}

1;
