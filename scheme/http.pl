#
# -*- Perl -*-
# $Id: http.pl,v 1.1 2003-01-15 10:12:18 knok Exp $
#
#

package Namazu::Scheme::http;

use strict;
require 'util.pl';

sub scheme() {
    return ('http');
}

sub status() {
    return 'no' if (! util::checklib('Net/HTTP.pm'));
    return 'no' if (! util::checklib('URI.pm'));
    return 'yes';
}

sub recursive() {
    return 0;
}

sub init() {
    eval 'use Net::HTTP;';
    eval 'use URI;';
    eval 'use HTTP::Date;';
}

sub fetch($$) {
    my ($url, $contref) = @_;
    my $err = undef;

    my $u = URI->new($url);
    my $h = Net::HTTP->new(Host => $u->host());
    $h->write_request(GET => $u->path());
    my $code;
    ($code, undef, undef) = $h->read_response_headers();
    if ($code eq '200') {
	$h->read_entity_body($$contref, $conf::FILE_SIZE_MAX);
    } else {
	$err = $code;
    }

    return $err;
}

sub mtime($) {
    my $url = shift @_;
    my $u = URI->new($url);
    my $h = Net::HTTP->new(Host => $u->host());
    $h->write_request(HEAD => $u->path());
    my ($code, $mess, %head) = $h->read_response_headers();
    return time unless defined $head{'Last-Modified'};
    return HTTP::Date::str2time($head{'Last-Modified'});
}

1;
