#
# -*- Perl -*-
# $Id: olepowerpoint.pl,v 1.1 2000-03-06 10:16:55 rug Exp $
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

package olepowerpoint;
#use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/powerpoint');
}

sub status() {
    my $powerpoint = Win32::OLE->new('PowerPoint.Application','Quit');
    return 'yes' if (defined $powerpoint);
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
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing powerpoint file ...\n");

	$cfile =~ s/\//\\/g;
    $$cont = ReadPPT::ReadPPT($cfile);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
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

package ReadPPT;

sub ReadPPT {
    my $fileName = shift;

    # Copy From Win32::OLE Example Program
    # use existing instance if PowerPoint is already running
    my $ppt; 
    eval {$ppt = Win32::OLE->GetActiveObject('PowerPoint.Application')};
    die "PowerPoint not installed" if $@;
    unless (defined $ppt) {
	$ppt = Win32::OLE->new('PowerPoint.Application', sub {$_[0]->Quit;})
	    or die "Oops, cannot start PowerPoint";
    }
    #End of Copy From Win32::OLE Example Program
    # Must set Visible = true 
    $ppt->{Visible} = 1;

    # Load Office 98 Constant
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library");

    my $prs = $ppt->{Presentations}->Open($fileName);
    die "Cannot open File $fileName" unless ( defined $prs );

    $allText = '';
    $allText .= olepowerpoint::getProperties($prs);
    $allText .= getSlides($prs);
    $prs->close();
    undef $prs;
    undef $ppt;

    return $allText;
}

sub getSlides {
    my $prs = shift;
    my $allText = '';

    my $enum_a_slide = sub {
	my $slide = shift;

	my $enum_a_headerfooter = sub {
	    my $obj = shift;
	    $allText .= $obj->Header->{Text} if ( $obj->{Header} && $obj->Header->{Text} ) ;
	    $allText .= $obj->Footer->{Text} if ( $obj->{Footer} && $obj->Footer->{Text} ) ;
	    return 1;
	};

	sub enum_a_shape {
	    my $shape = shift;
	    # Get text whaen TextFrame in Shapes and Text in TextFrame 
	    if ($shape->{HasTextFrame} && $shape->TextFrame->TextRange) { # 
		my $p = $shape->TextFrame->TextRange->{Text};
		$getShapes::allText .= $p;
		$getShapes::allText .= "\n";
	    } elsif ( $shape->{Type} == $office_consts->{msoGroup} ) { 
		olepowerpoint::enum($shape->GroupItems,\&enum_a_shape);
	    } 
	    return 1;
	};

        olepowerpoint::enum($slide->Shapes, \&enum_a_shape);
        &$enum_a_headerfooter($slide->HeadersFooters);
        return 1;
    };

    olepowerpoint::enum($prs->Slides, $enum_a_slide);
    return $allText;
}

#main
#use Cwd;
#$ARGV[0] = cwd().'\\'.$ARGV[0] unless ($ARGV[0] =~ m/^[a-zA-Z]\:[\\\/]/ || $ARGV[0] =~ m/^\/\//);
#$ARGV[0] =~ s|/|\\|g;

#print ReadPPT::ReadPPT("$ARGV[0]");

1;
