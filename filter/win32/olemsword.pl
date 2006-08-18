#
# -*- Perl -*-
# $Id: olemsword.pl,v 1.22 2006-08-18 17:35:19 opengl2772 Exp $
# Copyright (C) 1999 Jun Kurabe,
#		1999-2000 Ken-ichi Hirose,
#               2004-2006 Namazu Project All rights reserved.
#     This is free software with ABSOLUTELY NO WARRANTY.
#
#  This program is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either versions 2, or (at your option)
#  any later version.
#
#  This program is distributed in the hope that it will be useful
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
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
#		   Get Text From Grouped Shape Items
# V2.10 1999/11/09 Change Name
#		   Merge three program ReadMSWord.pl, ReadExcel.pl, ReadPPT.pl
# V2.11 1999/11/15  separate file.
#		    modify some functions.
# V2.12 1999/11/27 Use Office::OLE::Const to define constant value
# V2.13 2000/04/27 Optimize for Namazu filter ...
# V2.14 2000/10/28 contribute patch by Yoshinori.TAKESAKO-san.
# V2.15 2000/01/26 Use existing instance if Word is already running.
#

package olemsword;
use strict;
no strict 'refs';  # for symbolic reference: $fields;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/msword');
}

sub status() {
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 11.0 Object Library");
    $const = Win32::OLE::Const->Load("Microsoft Word 10.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 9.0 Object Library") unless $const;
    $const = Win32::OLE::Const->Load("Microsoft Word 8.0 Object Library") unless $const;
    open (STDERR,">&SAVEERR");
    if (defined $const){
	if (!util::islang("ja")) {
	    return 'yes';
	} else {
	    if ($conf::NKF ne 'no') {
		return 'yes';
	    } else {
		return 'no';
	    }
	}
    }else {
	return 'no';
    }
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
    my ($magic) = @_;

    $magic->addFileExts('\\.doc$', 'application/msword');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields) = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing ms-word file ... (using  'Win32::OLE->new Word.Application')\n");

    my $err = ReadMSWord::ReadDocument($cfile, $cont, $fields, $weighted_str);
    return $err if (defined $err);

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

    die "No Objects or No Function" unless ($enum_objs and $func);

    my $e = Win32::OLE::Enum->new($enum_objs);
    while((my $obj = $e->Next)) {
	return 0 if (!&$func($obj, $cont));
    }
    return 1;
}

sub getProperties ($$$) {
    my ($cfile, $fields, $weighted_str) = @_;

    # See VBA online help using Microsoft Word in detail.
    # Topic item: 'DocumentProperty Object'.

    my $title = $cfile->BuiltInDocumentProperties('Title')->{Value};
    $title = $cfile->BuiltInDocumentProperties('Subject')->{Value}
	unless (defined $title);
    if (defined $title) {
	codeconv::to_inner_encoding(\$title, 'shiftjis');
        $fields->{'title'} = $title;

        my $weight = $conf::Weight{'html'}->{'title'};
        $$weighted_str .= "\x7f$weight\x7f$fields->{'title'}\x7f/$weight\x7f\n";
    }

    my $author = $cfile->BuiltInDocumentProperties('Last Author')->{Value};
    $author = $cfile->BuiltInDocumentProperties('Author')->{Value}
	unless (defined $author);
    if (defined $author) {
	codeconv::to_inner_encoding(\$author, 'shiftjis');
        $fields->{'author'} = $author;
    }

    # my $date = $cfile->BuiltInDocumentProperties('Last Save Time')->{Value};
    # $date = $cfile->BuiltInDocumentProperties('Creation Date')->{Value}
    #    unless (defined $date);
    # if (defined $date) {
    #     $date = codeconv::shiftjis_to_eucjp($date);
    #     codeconv::normalize_eucjp(\$date);
    #     $fields->{'date'} = $date;
    # }

    my $keyword = $cfile->BuiltInDocumentProperties('keywords')->{Value};
    if (defined $keyword) {
	codeconv::to_inner_encoding(\$keyword, 'shiftjis');

        my $weight = $conf::Weight{'metakey'};
        $$weighted_str .= "\x7f$weight\x7f$keyword\x7f/$weight\x7f\n";
    }

    return undef;
}

package ReadMSWord;

my $word = undef;
my $office_consts = undef;

# Word application destructor
END {
    if (defined $word) {
	util::vprint("Word->Quit\n");
	$word->Quit(0);
	undef $word;
    }
}

sub GetExt($) {
    my ($filename) = @_;

    my $ext = $filename;
    $ext =~ s!.*/!!g;
    if ($ext !~ s/^.*(\.[^\.]*)$/$1/) {
        $ext = "";
    }

    return $ext;
}

sub ReadDocument ($$$$) {
    my ($cfile, $cont, $fields, $weighted_str) = @_;

    my $ext = GetExt($cfile);
    my $tmpfile = util::tmpnam('NMZ.olemsword') . $ext;
    {
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
        util::fclose($fh);
    }

    my $err = ReadMSWord::ReadMSWord($tmpfile, $cont, $fields, $weighted_str);
    unlink $tmpfile;

    codeconv::to_inner_encoding(\$title, undef);

    return $err;
}

sub ReadMSWord ($$$$) {
    my ($cfile, $cont, $fields, $weighted_str) = @_;

    $cfile =~ s/\//\\/g;
    $$cont = "";

    # Copy From Win32::OLE Example Program
    # use existing instance if Word is already running
    # eval {$word = Win32::OLE->GetActiveObject('Word.Application')};
    # die "MSWord not installed" if $@;
    unless (defined $word) {
	$word = Win32::OLE->new('Word.Application', 'Quit')
	    or return "Oops, cannot start Word." . Win32::OLE->LastError();
    }
    # End of Copy From Win32::OLE Example Program

    # Redirect stderr to null device, to ignore Error and Exception message.
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    # Load Office 97/98/2000/XP/2003 Constant
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 11.0 Object Library") unless $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 10.0 Object Library") unless $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 9.0 Object Library") unless $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library") unless $office_consts;

    $Win32::OLE::Warn = 0;            # False
    # for debug
    # $word->{Visible} = 1;             # True
    $word->{Visible} = 0;             # False
    $word->{DisplayAlerts} = 0;       # wdAlertsNone
    $word->{AutomationSecurity} = 3;  # msoAutomationSecurityForceDisable

    # Restore stderr device usually.
    open (STDERR,">&SAVEERR");

    # In order to skip password-protected file, send a dummy password.
    my $doc = $word->{Documents}->Open({
	'FileName'         => $cfile,
	'PasswordDocument' => 'dummy password',
	'PasswordTemplate' => 'dummy password',
	'ReadOnly'         => 1,  # True
	});
    return "Cannot open File $cfile" unless (defined $doc);

    olemsword::getProperties($doc, $fields, $weighted_str);
    getParagraphs($doc, $cont);
    getFrames($doc, $cont);
    getShapes($doc, $cont);
    getHeadersFooters($doc, $cont);

    $doc->Close({
        'SaveChanges' => 0  # False
    });
    undef $doc;

    return undef;
}

sub getParagraphs ($$) {
    my ($doc, $cont) = @_;

    my $enum_func = sub {
	my $obj = shift;
	my $p = $obj->Range->{Text};
#	chop $p;
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
	if ($obj->TextFrame->{HasText}) {
	    my $p = $obj->TextFrame->TextRange->{Text};
#	    chop $p;
	    $$cont .= "$p\n" if (defined $p);
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
        return 1 if (!defined($obj->Range));
	my $p = $obj->Range->{Text};
#	chop $p;
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
#	    chop $p;
	    $$cont .= "$p\n" if (defined $p);
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

#my $$cont = "";
#ReadMSWord::ReadMSWord("$ARGV[0]", $cont, "", "");
#(my $base = $ARGV[0]) =~ s/\.doc$//;
#open(F, "> $base.txt") || die;
#print F $$cont;
#close(F);

1;
