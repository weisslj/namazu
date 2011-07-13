#
# -*- Perl -*-
# $Id: man.pl,v 1.40 2011-07-13 15:22:03 usu Exp $
# Copyright (C) 1997-2000 Satoru Takabayashi ,
#               1999 NOKUBI Takatsugu ,
#               2004 Namazu Project All rights reserved.
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
#  This file must be encoded in utf-8 encoding
#

package man;
use strict;
require 'util.pl';
require 'gfilter.pl';


sub mediatype() {
    return ('text/x-roff');
}

sub status() {
    return 'yes';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
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

    codeconv::to_inner_encoding($cont, undef);
    #codeconv::toeuc($cont, undef);
    roff_filter($cont, $weighted_str, $fields);
    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    gfilter::show_filter_debug_info($cont, $weighted_str,
				    $fields, $headings);
    return undef;
}

sub roff_filter ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    my $title = "";
    my $comment = "";
    my @lines;
    @lines = split(/\n/, $$contref);
    $$contref = "";
    while (@lines){
	my $str = shift @lines;
	if ($str =~ /^\.TH ([^\s]*)(.*)$/) {
	    $title = $1;
	    $$contref .= $1 . " " . $2 ."\n";
	}elsif ($str =~ /^\.S[HSMB] (.*)$/){
	    if ($1 =~ /NAME|Ì¾Á°|Ì¾¾Î/){
		do {
		    $str = shift @lines;
		} while ($str =~ /^\./);
		chomp $str;
		my $weight = $conf::Weight{'html'}->{'h1'};
		$$weighted_str .= "\x7f$weight\x7f$str\x7f/$weight\x7f\n";
		$$contref .=  $str . "::\n";
	    }
	}elsif ($str =~ /^\.[RBI]+ (.*)$/){
	    $$contref .=  $1;
	}elsif ($str =~ /^\.\\" (.*)$/){
	    $comment .=  $1 . "\n";
	}elsif ($str =~ /^\.ig/){
	    $str ="";
	    do {
		$str = shift @lines;
		$comment .=  $str;
	    } while ($str !~ /^\./);
	}elsif ($str =~ /^\..?/){
	}else {
	    $$contref .=  $str;
	}
    }
    $fields->{'title'} = $title;
    # $$contref .= $comment;
}

1;
