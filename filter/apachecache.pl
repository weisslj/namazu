package apachecache;

require 'util.pl';
use FileHandle;
use strict;

sub mediatype()    { 'application/x-apache-cache'; }
sub recursive()    { 1; }
sub pre_codeconv() { 0; }
sub post_codeconv(){ 0; }
sub status()       { 'yes'; }

sub add_magic ($) {
    my ($magic) = @_;
    $magic->addMagicEntry( "119\tstring\tX-URL:\tapplication/x-apache-cache" );
}

# This function is stolen from CGI_Lite.pm.
sub url_decode ($) {
    my $string = shift;
    $string =~ s/%([\da-fA-F]{2})/chr (hex ($1))/eg;
    $string;
}

sub getapacheurl {
    my ($str) = @_;
    $str =~ /\A.{126}([^\n]*)\n/s;
    &url_decode($1);
}

sub replacecode {
    my $fh = new FileHandle( $_, "r" );
    if( $fh ){
	my $buf;
	read( $fh, $buf, 126 + 1024 );
	close $fh;
	$_ = &getapacheurl( $buf );
    } else {
	util::vprint( "Can not open cache file: $_\n" );
    }
}

sub filter ($$$$$) {
    my( $orig_cfile, $contref, $weighted_str, $headings, $fields ) = @_;
    util::vprint( "Cache File: ".$$orig_cfile,
		  "Cache URL : ".&getapacheurl($$contref) );

    # Remove header.
    $$contref =~ s/\A.*?\r\n\r\n//s;
    $$contref =~ s/\A.*?\r\n\r\n//s;
    return undef;
}

1;
