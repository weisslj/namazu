#
# -*- Perl -*-
# $Id: olemsword.pl,v 1.5 2001-01-04 07:21:48 baba Exp $
# Copyright (C) 1999 Jun Kurabe ,
#               1999-2000 Ken-ichi Hirose All rights reserved.
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
# V2.13 2000/04/27 Optimize for Namazu filter ...
# V2.14 2000/10/28 contribute patch by Yoshinori.TAKESAKO-san.
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

    util::vprint("Processing ms-word file ... (using  'Win32::OLE->new Word.Application')\n");

    $cfile =~ s/\//\\/g;
    $$cont = "";
    ReadMSWord::ReadMSWord($cfile, $cont, $fields);
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

sub getProperties ($$) {
    my ($cfile, $fields) = @_;

    # get Title
    $fields->{'title'} = $cfile->BuiltInDocumentProperties(1)->{Value}
      unless $cfile->BuiltInDocumentProperties(1)->{Value};            # title
#    $fields->{'title'} = $cfile->BuiltInDocumentProperties(2)->{Value}; # subject
#    $fields->{'author'} = $cfile->BuiltInDocumentProperties(3)->{Value}; # author
    $fields->{'author'} = $cfile->BuiltInDocumentProperties(7)->{Value}
      unless $cfile->BuiltInDocumentProperties(7)->{Value};            # lastauthor
#    $fields->{'date'} = $cfile->BuiltInDocumentProperties(11)->{Value}; # createdate
#    $fields->{'date'} = $cfile->BuiltInDocumentProperties(12)->{Value}
#      unless $cfile->BuiltInDocumentProperties(12)->{Value};            # editdate
#    $fields->{'date'} = $cfile->BuiltInDocumentProperties(13)->{Value}; # editdate?

    return undef;
}

package ReadMSWord;

sub ReadMSWord ($$$) {
    my ($cfile, $cont, $fields) = @_;

    # Copy From Win32::OLE Example Program
    # use existing instance if Word is already running
    my $word; 
    eval {$word = Win32::OLE->GetActiveObject('Word.Application')};
    die "MSWord not installed" if $@;
    unless (defined $word) {
        $word = Win32::OLE->new('Word.Application', sub {$_[0]->Quit(0);})
         or die "Oops, cannot start Word";
    }
    # End of Copy From Win32::OLE Example Program
    # for debug
    # $word->{Visible} = 1;

    # Load Office 98 Constant
    local $office_consts;
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 9.0 Object Library");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library") unless $office_consts;
    open (STDERR,">&SAVEERR");

    # In order to skip password-protected file, send a dummy password.
    my $doc = $word->{Documents}->open({
	'FileName' => $cfile,
	'PasswordDocument' => 'dummy password',
	'ReadOnly' => 1
	});
    die "Cannot open File $cfile" unless ( defined $doc ) ;

    olemsword::getProperties($doc, $fields);
    getParagraphs($doc, $cont);
    getFrames($doc, $cont);
    getShapes($doc, $cont);
    getHeadersFooters($doc, $cont);

    $doc->close(0);
    undef $doc;
    undef $word;

    return undef;
}

sub getParagraphs ($$) {
    my ($doc, $cont) = @_;

    my $enum_func = sub {
    my $obj = shift;
    my $p = $obj->Range->{Text};
#       chop $p;
        $$cont .= "$p\n" if ( defined $p );
        return 1;
    };

    olemsword::enum($doc->Paragraphs, $enum_func, $cont);
    return undef;
}

sub getShapes ($$) {
    my ($doc, $cont) = @_;

    sub enum_a_shape ($$) {
    my ($obj, $cont) = @_;
    if ($obj->TextFrame->{HasText}) { # 
        my $p = $obj->TextFrame->TextRange->{Text};
#       chop $p;
        $$cont .= "$p\n" if ( defined $p );
    } elsif ( $obj->{Type} == $office_consts->{msoGroup} ) { 
        # msoGroup = 6
        olemsword::enum($obj->GroupItems, \&enum_a_shape, $cont);
    } 
    return 1;
    };

    olemsword::enum($doc->Shapes, \&enum_a_shape, $cont);
    return undef;
}

sub getFrames ($$) {
    my ($doc, $cont) = @_;

    my $enum_func = sub {
    my $obj = shift;
    my $p = $obj->Range->{Text};
#       chop $p;
        $$cont .= "$p\n" if ( defined $p );
        return 1;
    };

    olemsword::enum($doc->Frames, $enum_func, $cont);
    return undef;
}

sub getHeadersFooters ($$) {
    my ($doc, $cont) = @_;

    my $enum_a_section = sub {
    my $obj = shift;
    my $enum_a_headerfooter = sub {
        my $obj = shift;
        my $p = $obj->Range->{Text};
#       chop $p;
        $$cont .= "$p\n" if ( defined $p );
        return 1;
    };

    olemsword::enum($obj->Headers, $enum_a_headerfooter, $cont);
    olemsword::enum($obj->Footers, $enum_a_headerfooter, $cont);
    return 1;
    };

    olemsword::enum($doc->Sections, $enum_a_section, $cont);
    return undef;
}

#main
#use Cwd;
#$ARGV[0] = cwd().'\\'.$ARGV[0] unless ($ARGV[0] =~ m/^[a-zA-Z]\:[\\\/]/ || $ARGV[0] =~ m/^\/\//);
#$ARGV[0] =~ s|/|\\|g;

#print ReadMSWord::ReadMSWord("$ARGV[0]");

1;
