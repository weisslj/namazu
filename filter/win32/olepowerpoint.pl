#
# -*- Perl -*-
# $Id: olepowerpoint.pl,v 1.4 2001-01-04 01:58:01 baba Exp $
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
#                   modify some functions.
# V2.12 1999/11/27 Use Office::OLE::Const to define constant value
# V2.13 2000/05/16 Optimize for Namazu filter ...
# V2.14 2000/10/28 contribute patch by Yoshinori.TAKESAKO-san.
#

package olepowerpoint;
#use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/powerpoint');
}

sub status() {
    open (SAVEERR,">&STDERR");    
    open (STDERR,">nul");    
    my $powerpoint = Win32::OLE->new('PowerPoint.Application','Quit');
    open (STDERR,">&SAVEERR");
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
    my ($magic) = @_;

    $magic->addFileExts('\\.ppt$', 'application/powerpoint');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing powerpoint file ... (using  'Win32::OLE->new PowerPoint.Application')\n");

    $cfile =~ s/\//\\/g;
    $$cont = "";
    ReadPPT::ReadPPT($cfile, $cont, $fields);
    $cfile = defined $orig_cfile ? $$orig_cfile : '';

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

sub enum ($$$) {
    my ($enum_objs, $func, $cont) = @_;

    die "No Objects or No Function" unless ($enum_objs and $func );

    my $e = Win32::OLE::Enum->new($enum_objs);
    while(($obj = $e->Next)) {
        return 0 if (!&$func($obj, $cont));
    }
    return 1;
}

sub getProperties {
    my ($cfile, $fields) = @_;

    # get Title
    $fields->{'title'} = "$cfile->BuiltInDocumentProperties(1)->{Value}"
      unless $cfile->BuiltInDocumentProperties(1)->{Value};            # title
#    $fields->{'title'} = $cfile->BuiltInDocumentProperties(2)->{Value}; # subject
#    $fields->{'author'} = $cfile->BuiltInDocumentProperties(3)->{Value}; # author
    $fields->{'author'} = "$cfile->BuiltInDocumentProperties(7)->{Value}"
      unless $cfile->BuiltInDocumentProperties(7)->{Value};            # lastauthor
#    $fields->{'date'} = $cfile->BuiltInDocumentProperties(11)->{Value}; # createdate
    $fields->{'date'} = "$cfile->BuiltInDocumentProperties(12)->{Value}"
      unless $cfile->BuiltInDocumentProperties(12)->{Value};            # editdate
#    $fields->{'date'} = $cfile->BuiltInDocumentProperties(13)->{Value}; # editdate?

    return undef;
}

package ReadPPT;

sub ReadPPT ($$$) {
    my ($cfile, $cont, $fields) = @_;

    # Copy From Win32::OLE Example Program
    # use existing instance if PowerPoint is already running
    my $ppt; 
    eval {$ppt = Win32::OLE->GetActiveObject('PowerPoint.Application')};
    die "PowerPoint not installed" if $@;
    unless (defined $ppt) {
    $ppt = Win32::OLE->new('PowerPoint.Application', sub {$_[0]->Quit(0);})
     or die "Oops, cannot start PowerPoint";
    }
    #End of Copy From Win32::OLE Example Program
    # Must set Visible = true 
    $ppt->{Visible} = 1;

    # Load Office 98 Constant
    local $office_consts;
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 9.0 Object Library");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library") unless $office_consts;
    open (STDERR,">&SAVEERR");

    my $prs = $ppt->{Presentations}->Open($cfile);
    die "Cannot open File $cfile" unless ( defined $prs );

    olepowerpoint::getProperties($prs, $fields);
    getSlides($prs, $cont);

    $prs->close();
    undef $prs;
    undef $ppt;

    return undef;
}

sub getSlides ($$) {
    my ($prs, $cont) = @_;

    my $enum_a_slide = sub {
    my $slide = shift;

    my $enum_a_headerfooter = sub {
        my $obj = shift;
        $$cont .= $obj->Header->{Text} if ( $obj->{Header} && $obj->Header->{Text} ) ;
        $$cont .= $obj->Footer->{Text} if ( $obj->{Footer} && $obj->Footer->{Text} ) ;
        return 1;
    };

    sub enum_a_shape ($$) {
    my ($shape, $cont) = @_;
        # Get text whaen TextFrame in Shapes and Text in TextFrame 
        if ($shape->{HasTextFrame} && $shape->TextFrame->TextRange) { # 
        my $p = $shape->TextFrame->TextRange->{Text};
            $$cont .= "$p\n" if ( defined $p );
        } elsif ( $shape->{Type} == $office_consts->{msoGroup} ) { 
            olepowerpoint::enum($shape->GroupItems, \&enum_a_shape, $cont);
        } 
        return 1;
    };

        olepowerpoint::enum($slide->Shapes, \&enum_a_shape, $cont);
#        &$enum_a_headerfooter($slide->HeadersFooters);
        return 1;
    };

    olepowerpoint::enum($prs->Slides, $enum_a_slide, $cont);
    return undef;
}

#main
#use Cwd;
#$ARGV[0] = cwd().'\\'.$ARGV[0] unless ($ARGV[0] =~ m/^[a-zA-Z]\:[\\\/]/ || $ARGV[0] =~ m/^\/\//);
#$ARGV[0] =~ s|/|\\|g;

#print ReadPPT::ReadPPT("$ARGV[0]");

1;
