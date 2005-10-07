#
# -*- Perl *-*
# $Id: html.pl,v 1.2 2005-10-07 18:35:19 opengl2772 Exp $
#

package Namazu::Archive::html;

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

use strict;

sub mediatype() {
    return ('text/html');
}

sub archive() {
    return ('yes');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 'no';
}

sub init() {
}

sub option() {
    return ('7|html-split' => \$var::Opt{'htmlsplit'});
}

sub splitfile ($$) {
    my ($fname, $base) = @_;
    my $mtime = (stat($fname))[9];
    my $fh = util::efopen($fname);
    my $cont   = join '', <$fh>;
    util::fclose($fh);
    Namazu::Archive::html::split(\$cont, $mtime, $base);
}

sub split ($$$) {
    my ($contref, $mtime, $base) = @_;

    codeconv::normalize_document($contref);

    my %info = (
		'title'    => get_title($contref),
		'author'   => get_author($contref),
		'anchored' => "",
		'name'     => "",
		'base'     => $base,
		'names'    => [],
		);

    # <http://www.w3.org/TR/html4/intro/sgmltut.html#h-3.2.2>
    # 
    # In certain cases, authors may specify the value of an attribute
    # without any quotation marks. The attribute value may only contain
    # letters (a-z and A-Z), digits (0-9), hyphens (ASCII decimal 45), and
    # periods (ASCII decimal 46). We recommend using quotation marks even
    # when it is possible to eliminate them.

    my $id = 0;
#    $cont =~ s/(<a\s[^>]*href=(["']))#(.+?)(\2[^>]*>)/$1$3.html$4/gi; #'
    $$contref =~ s#(<a[^>]*\s+)name=(["'])\2([^>]*>(.*?)</a>)#$1$4#sgi; #'
    $$contref =~ s {
                \G(.+?)                                      # 1
	        (<h([1-6])>)?\s*                             # 2, 3
                <a[^>]*\s+name=([a-zA-Z0-9-\.]+|             # 4,
                (["']).+?\5)[^>]*>(.*?)</a>                  # 5,6
                \s*(</h\3>)?                                 # 7
             } {
                write_partial_file($1, $4, $6, $id++, $mtime, \%info)
             }sgexi;
    write_partial_file($$contref, "", "", $id, $mtime, \%info);

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

sub write_partial_file($$$$$$) {
    my ($cont, $name, $anchored, $id, $mtime, $info_ref) = @_;

    $name =~ s/^([\"\'])(.*)\1$/$2/;  # Remove quotation marks.

    my $author        = $info_ref->{'author'};
    my $base          = $info_ref->{'base'};
    my $orig_title    = $info_ref->{'title'};
    my $prev_name     = $info_ref->{'name'};
    my $prev_anchored = $info_ref->{'anchored'};

    $prev_name        =~ s#\n\r##sg;
    $prev_name        =~ s#\n##sg;

    html::remove_html_elements(\$prev_anchored);
    $prev_anchored =~ s/^\s+//;
    $prev_anchored =~ s/\s+$//;
    my $title = $orig_title;

    # FIXME: I don't know why this processing causes "Use of
    # uninitialized value" warning if use $prev_anchored or
    # $prev_name directly. perl's bug?
    if ($prev_anchored ne "") {
	$title .= ": $prev_anchored";
    } elsif ($prev_name ne "") {
	$title .= ": $prev_name";
    }

    my $fname = util::tmpnam("$base.$id");
    my $fh = util::efopen(">$fname");
    my $header = $Header;
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
    util::fclose($fh);
    utime($mtime, $mtime, $fname);

    return "";
}

1;
