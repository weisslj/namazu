#
# -*- Perl -*-
# $Id: olepowerpoint.pl,v 1.14 2004-04-10 16:32:13 opengl2772 Exp $
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
# V2.20 2001/01/24 get a title from filename which has no slide title
#

package olepowerpoint;
use strict;
no strict 'refs';  # for symbolic reference: $fields;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/powerpoint');
}

sub status() {
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $const;
    $const = Win32::OLE::Const->Load("Microsoft PowerPoint 11.0 Object Library");
    $const = Win32::OLE::Const->Load("Microsoft PowerPoint 10.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft PowerPoint 9.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft PowerPoint 8.0 Object Library") unless $const;
    open (STDERR,">&SAVEERR");
    return 'yes' if (defined $const);
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

    $magic->addFileExts('\\.pp[st]$', 'application/powerpoint');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields) = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing powerpoint file ... (using  'Win32::OLE->new PowerPoint.Application')\n");

    $cfile =~ s/\//\\/g;
    $$cont = "";
    ReadPPT::ReadPPT($cfile, $cont, $fields, $weighted_str);
    $cfile = defined $orig_cfile ? $$orig_cfile : '';

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
	unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str, $fields, $headings);

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
    while((my $obj = $e->Next)) {
        return 0 if (!&$func($obj, $cont));
    }
    return 1;
}

sub getProperties ($$$) {
    my ($cfile, $fields, $weighted_str) = @_;

    # See VBA online help using Microsoft PowerPoint in detail.
    # Topic item: 'DocumentProperty Object'.

    my $title = $cfile->BuiltInDocumentProperties('Title')->{Value};
    $title = $cfile->BuiltInDocumentProperties('Subject')->{Value}
	unless (defined $title);
    undef $title if $title eq # which has no slide title
	"\xbd\xd7\xb2\xc4\xde\x20\xc0\xb2\xc4\xd9\x82\xc8\x82\xb5";
    $fields->{'title'} = codeconv::shiftjis_to_eucjp($title)
	if (defined $title);

    my $weight = $conf::Weight{'html'}->{'title'};
    $$weighted_str .= "\x7f$weight\x7f$fields->{'title'}\x7f/$weight\x7f\n";

    my $author = $cfile->BuiltInDocumentProperties('Author')->{Value};
    $author = $cfile->BuiltInDocumentProperties('Last Author')->{Value}
	unless (defined $author);
    $fields->{'author'} = codeconv::shiftjis_to_eucjp($author)
	if (defined $author);

#    my $date = $cfile->BuiltInDocumentProperties('Last Save Time')->{Value};
#    $date = $cfile->BuiltInDocumentProperties('Creation Date')->{Value}
#	unless (defined $date);
#    $fields->{'date'} = codeconv::shiftjis_to_eucjp($date) if (defined $date);

    my $keyword = $cfile->BuiltInDocumentProperties('keywords')->{Value};
    $keyword = codeconv::shiftjis_to_eucjp($keyword);
    $weight = $conf::Weight{'metakey'};
    $$weighted_str .= "\x7f$weight\x7f$keyword\x7f/$weight\x7f\n";

    return undef;
}

package ReadPPT;

my $office_consts = undef;

sub ReadPPT ($$$$) {
    my ($cfile, $cont, $fields, $weighted_str) = @_;

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

    # Redirect stderr to null device, to ignore Error and Exception message.
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    # Load Office 97/98/2000/XP/2003 Constant
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 11.0 Object Library");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 10.0 Object Library") unless $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 9.0 Object Library") unless $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library") unless $office_consts;
    # 'Visible = false' causes exception but noharm, so we ignore... X-(
    $ppt->{Visible} = 0;
    # Restore stderr device usually.
    open (STDERR,">&SAVEERR");

    my $prs = $ppt->{Presentations}->Open({
	'FileName' => $cfile,
	'ReadOnly' => 1,
	'WithWindow' => 0,
	});
    die "Cannot open File $cfile" unless (defined $prs);

    olepowerpoint::getProperties($prs, $fields, $weighted_str);
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
	    if ($shape->{HasTextFrame} && $shape->TextFrame->TextRange) {
		my $p = $shape->TextFrame->TextRange->{Text};
		$$cont .= "$p\n" if (defined $p);
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

#my $$cont = "";
#ReadPPT::ReadPPT("$ARGV[0]", $cont, "", "");
#(my $base = $ARGV[0]) =~ s/\.ppt$//;
#open(F, "> $base.txt") || die;
#print F $$cont;
#close(F);

1;
