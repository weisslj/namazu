#
# -*- Perl -*-
# $Id: olemsword.pl,v 1.3 2000-03-22 21:53:14 kenzo- Exp $
# Copyright (C) 1999 Jun Kurabe ,
#               1999 Ken-ichi Hirose All rights reserved.
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
#  
#  Some Code copy from Win32::OLE Example program.
#  Licence for these code is 
#    You may distribute under the terms of either the GNU General Public
#    License or the Artistic License, as specified in the README(Win32::OLE) file.
#
# Created by Jun Kurabe
# E-Mail: jun-krb@mars.dti.ne.jp
# V1.00 1999/10/30
# V1.01 1999/11/03 Add getFrames by Jun Kurabe
# V1.02 1999/11/03 Change getProperties of check TextFrame statement
# V2.00 1999/11/06 Change Program Structure 
#                  Get Text From Grouped Shape Items
# V2.10 1999/11/09 Change Name 
#                  Merge three program ReadMSWord.pl, ReadExcel.pl, ReadPPT.pl
# V2.11 1999/11/15  separate file.
#					modify some functions.
# V2.12 1999/11/27 Use Office::OLE::Const to define constant value
#

package olemsword;
#use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/msword');
}

sub status() {
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $msword = Win32::OLE->new('Word.Application','Quit');
    open (STDERR,">&SAVEERR");
    return 'yes' if (defined $msword);
    return 'no';
}

sub recursive() {
    return 0;
}

sub pre_codeconv() {
    return 0;
}

sub post_codeconv () {
    return 1;
}

sub add_magic ($) {
    my ($magic) = @_;

    $magic->addFileExts('\\.doc$', 'application/msword');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing msword file ...\n");

	$cfile =~ s/\//\\/g;
    $$cont = ReadMSWord::ReadMSWord($cfile);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
      unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}


# Original of this code was contributed by <jun-krb@mars.dti.ne.jp>. 

use Win32::OLE;
use Win32::OLE::Enum;
use Win32::OLE::Const;

sub enum {
    my $enum_objs = shift;
    my $func = shift;

    die "No Objects or No Function" unless ($enum_objs and $func );

    my $e = Win32::OLE::Enum->new($enum_objs);
    while(($obj = $e->Next)) {
	return 0 if (!&$func($obj));
    }
    return 1;
}

sub getProperties {
    my $doc = shift;
    my $allText = '';

    # get Title
    my $title = $doc->BuiltInDocumentProperties(1)->{Value};
    my $subject = $doc->BuiltInDocumentProperties(2)->{Value};
    my $author = $doc->BuiltInDocumentProperties(3)->{Value};
    my $lastAuthor = $doc->BuiltInDocumentProperties(7)->{Value};
    my $createDate = $doc->BuiltInDocumentProperties(11)->{Value};
    my $editDate = $doc->BuiltInDocumentProperties(12)->{Value};

    $allText .= 'Subject: ' . $title . ' ' . $subject ;
    $allText .= "\n";
    $allText .= 'From: ' . $author . ',' . $lastAuthor;
    $allText .= "\n";
    $allText .= 'Date: ' . $createDate;
    $allText .= "\n";
    $allText .= 'Edit: ' . $editDate;
    $allText .= "\n";
    $allText .= "\n";

    return $allText;
}

package ReadMSWord;

sub ReadMSWord {
    my $fileName = shift;

    # Copy From Win32::OLE Example Program
    # use existing instance if Word is already running
    my $word; 
    eval {$word = Win32::OLE->GetActiveObject('Word.Application')};
    die "MSWord not installed" if $@;
    unless (defined $word) {
	$word = Win32::OLE->new('Word.Application', sub {$_[0]->Quit;})
	    or die "Oops, cannot start Word";
    }
    # End of Copy From Win32::OLE Example Program
    # for debug
    # $word->{Visible} = 1;

    # Load Office 98 Constant
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library");

    my $doc = $word->{Documents}->open($fileName);
    die "Cannnot open DOC" unless ( defined $doc ) ;

    $allText = '';
    $allText .= olemsword::getProperties($doc);
    $allText .= getParagraphs($doc);
    $allText .= getFrames($doc);
    $allText .= getShapes($doc);
    $allText .= getHeadersFooters($doc);
    $doc->close(0);
    undef $doc;
    undef $word;

    return $allText;
}

sub getParagraphs {
    my $doc = shift;
    my $allText = '';

    my $enum_func = sub {
	my $obj = shift;
	my $p = $obj->Range->{Text};
	chop $p;
	$allText .= $p;
	$allText .= "\n";
	return 1;
    };

    olemsword::enum($doc->Paragraphs,$enum_func);
    return $allText;
}

sub getShapes {
    my $doc = shift;
    my $allText = '';
    sub enum_a_shape {
	my $obj = shift;
	if ($obj->TextFrame->{HasText}) { # 
	    my $p = $obj->TextFrame->TextRange->{Text};
	    chop $p;
	    $getShapes::allText .= $p;
	    $getShapes::allText .= "\n";
	} elsif ( $obj->{Type} == $office_consts->{msoGroup} ) { #msoGroup = 6
	    olemsword::enum($obj->GroupItems,\&enum_a_shape);
	} 
	return 1;
    };

    olemsword::enum($doc->Shapes, \&enum_a_shape);
    return $allText;
}

sub getFrames {
    my $doc = shift;
    my $allText = '';
    my $enum_func = sub {
	my $obj = shift;
	my $p = $obj->Range->{Text};
	chop $p;
	$allText .= $p;
	$allText .= "\n";
	return 1;
    };

    olemsword::enum($doc->Frames, $enum_func);
    return $allText;
}

sub getHeadersFooters {
    my $doc = shift;
    my $allText = '';

    my $enum_a_section = sub {
	my $obj = shift;
	my $enum_a_headerfooter = sub {
	    my $obj = shift;
	    my $p = $obj->Range->{Text};
	    chop $p;
	    $allText .= $p;
	    $allText .= "\n";
	    return 1;
	};

	olemsword::enum($obj->Headers, $enum_a_headerfooter);
	olemsword::enum($obj->Footers, $enum_a_headerfooter);
	return 1;
    };

    olemsword::enum($doc->Sections,$enum_a_section);
    return $allText;
}

#main
#use Cwd;
#$ARGV[0] = cwd().'\\'.$ARGV[0] unless ($ARGV[0] =~ m/^[a-zA-Z]\:[\\\/]/ || $ARGV[0] =~ m/^\/\//);
#$ARGV[0] =~ s|/|\\|g;

#print ReadMSWord::ReadMSWord("$ARGV[0]");

1;
