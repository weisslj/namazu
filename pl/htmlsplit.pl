#
# -*- Perl -*-
# $Id: htmlsplit.pl,v 1.1 2000-03-02 02:39:33 satoru Exp $
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
use FileHandle;
my $Header = << 'EOS';
<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01//EN"
        "http://www.w3.org/TR/html4/strict.dtd">
<html>
<head>
<title>${subject}</title>
</head>
<body>
<h1>${subject}</h1>
<hr>
EOS

    my $Footer = << 'EOS';
<hr>
<address>
    ${author}
</address>
</body>
</html>
EOS

my $Title;
my $Author;
my $Name;
my $Anchored;
my @Names;
my $Base;

sub split ($$) {
    my ($fname, $base) = @_;

    my $fh = util::efopen($fname);
    my $cont   = join '', <$fh>;
    $Title  = get_title(\$cont);
    $Author = get_author(\$cont);

    $Base = $base;
    $Name     = "";
    $Anchored = "";
    @Names = ();

    my $id = 0;
    $cont =~ s/(<a\s[^>]*href=(["']))#(.+?)(\2[^>]*>)/$1$3.html$4/g; #'
$cont =~ s{\G(.+?)<a\s[^>]*name=(["'])(.+?)\2[^>]*>(.*?)</a>} #'
          {write_partial_file($1, $3, $4, $id++)}sgex;
    write_partial_file($cont, "", "", $id);

    return @Names;
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

sub write_partial_file($$$) {
    my ($cont, $name, $anchored, $id) = @_;
    my $fname = util::tmpnam("$Base.$id");

    my $fh = new FileHandle;
    $fh->open(">$fname") || die "$fname: $!";

    my $header = $Header;
    my $title = $Anchored ? "$Title: $Anchored" : $Title;
    $header =~ s/\$\{subject\}/$title/g;
    print $fh $header;
    print $fh $cont;

    my $footer = $Footer;
    $footer =~ s/\$\{author\}/$Author/g;
    print $fh $footer;

    push @Names, $Name;

    $Name = $name;
    $Anchored = $anchored;

    return "";
}

1;
