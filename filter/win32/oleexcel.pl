#
# -*- Perl -*-
# $Id: oleexcel.pl,v 1.7 2001-01-12 05:15:31 baba Exp $
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
# V2.11 1999/11/15 separate file.
#		   modify some functions.
# V2.12 1999/11/27 Use Office::OLE::Const to define constant value
# V2.13 2000/05/16 Optimize for Namazu filter ...
# V2.14 2000/10/28 contribute patch by Yoshinori.TAKESAKO-san.
#

package oleexcel;
#use strict;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/excel');
}

sub status() {
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    my $excel = Win32::OLE->new('Excel.Application','Quit');
    open (STDERR,">&SAVEERR");
    return 'yes' if (defined $excel);
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

    $magic->addFileExts('\\.xls$', 'application/excel');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields) = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing excel file ... (using  'Win32::OLE->new Excel.Application')\n");

    $cfile =~ s/\//\\/g;
    $$cont = "";
    ReadExcel::ReadExcel($cfile, $cont, $fields);
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

    die "No Objects or No Function" unless ($enum_objs and $func);

    my $e = Win32::OLE::Enum->new($enum_objs);
    while(($obj = $e->Next)) {
	return 0 if (!&$func($obj, $cont));
    }
    return 1;
}

sub getProperties ($$) {
    my ($cfile, $fields) = @_;

    # See VBA online help using Microsoft Excel in detail.
    # Topic item: 'DocumentProperty Object'.

    my $title = $cfile->BuiltInDocumentProperties('Title')->{Value};
    $title = $cfile->BuiltInDocumentProperties('Subject')->{Value}
	unless (defined $title);
    $fields->{'title'} = $title if (defined $title);

    my $author = $cfile->BuiltInDocumentProperties('Last Author')->{Value};
    $author = $cfile->BuiltInDocumentProperties('Author')->{Value}
	unless (defined $author);
    $fields->{'author'} = $author if (defined $author);

#    my $date = $cfile->BuiltInDocumentProperties('Last Save Time')->{Value};
#    $date = $cfile->BuiltInDocumentProperties('Creation Date')->{Value}
#	unless (defined $date);
#    $fields->{'date'} = $date if (defined $date);

    return undef;
}

package ReadExcel;

sub ReadExcel ($$$) {
    my ($cfile, $cont, $fields) = @_;

    # Copy from Win32::OLE Example Program
    # use existing instance if Excel is already running
    my $excel;
    eval {$excel = Win32::OLE->GetActiveObject('Excel.Application')};
    die "Excel not installed" if $@;
    unless (defined $excel) {
	$excel = Win32::OLE->new('Excel.Application', sub {$_[0]->Quit;})
	    or die "Oops, cannot start Excel";
    }
    # End of Copy from Win32::OLE Example Program

    # Redirect stderr to null device, to ignore Error and Exception message.
    open (SAVEERR,">&STDERR");
    open (STDERR,">nul");
    # Load Office 98 Constant
    local $office_consts;
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 9.0 Object Library");
    $office_consts = Win32::OLE::Const->Load("Microsoft Office 8.0 Object Library") unless $office_consts;
    # for debug
    # $excel->{Visible} = 1;
    # Restore stderr device usually.
    open (STDERR,">&SAVEERR");

    my $wb = $excel->Workbooks->Open({
	'FileName' => $cfile,
	'ReadOnly' => 1
	});
    die "Cannot open File $cfile" unless (defined $wb);
    # print $cfile,"\n";
    # print $wb,"\n";

    oleexcel::getProperties($wb, $fields);
    getSheets($wb, $cont);

    $wb->close(0);
    undef $wb;
    undef $excel;

    return undef;
}

sub getSheets ($$) {
    my ($wb, $cont) = @_;

    my $enum_a_sheet = sub {
	my $obj = shift;
	getCells($obj, $cont);
	getShapes($obj, $cont);
	return 1;
    };

    oleexcel::enum($wb->Worksheets, $enum_a_sheet, $cont);
    return undef;
}

sub getCells ($$) {
    # parameter
    # sheet: Sheet Object
    # Return Value
    #	Text which is contained in cell on a sheet
    my ($sheet, $cont) = @_;

    # In order to avoid too much index time, restrict 100x100 cells.
    my $tmpur = $sheet->{UsedRange};
    my $tmprc = ($tmpur->Rows->Count	>100 ? 100 : $tmpur->Rows->Count);
    my $tmpcc = ($tmpur->Columns->Count >100 ? 100 : $tmpur->Columns->Count);
    my $ur = $sheet->Range(
			   $sheet->Cells($tmpur->Rows->Row, $tmpur->Columns->Column),
			   $sheet->Cells($tmpur->Rows($tmprc)->Row, $tmpur->Columns($tmpcc)->Column)
			   );

    my $enum_a_cell = sub {
	my $cell = shift;
	my $p = $cell->{Value};
	$$cont .= "$p\n" if (defined $p);
	return 1;
    };

    oleexcel::enum($ur->Cells, $enum_a_cell, $cont);
    return undef;
}

sub getShapes ($$) {
    my ($sheet, $cont) = @_;
    my $depth = 0;

    sub enum_a_shape {
	my ($obj, $cont) = @_;
	$getShapes::depth++;
	# print "passed YY $getShapes::depth\n";
	# print "type = ", $obj->{Type}, "\n";
	if ( $obj->{Type} == $office_consts->{msoGroup} ) { #msoGroup = 6
	    # print "passed XX\n";
	    oleexcel::enum($obj->GroupItems, \&enum_a_shape, $cont);
	    } elsif ($obj->{Type} == $office_consts->{msoTextBox} ||
		     $obj->{Type} == $office_consts->{msoAutoShape} ) {
		if ($obj->{TextFrame} && $obj->TextFrame->{Characters}) {
		    my $p = $obj->TextFrame->Characters->{Text} ;
		    $$cont .= "$p\n" if (defined $p);
		}
	    }
	$getShapes::depth--;
	return 1;
    };

    oleexcel::enum($sheet->Shapes,\&enum_a_shape, $cont);
    return undef;
}

#main
#use Cwd;
#$ARGV[0] = cwd().'\\'.$ARGV[0] unless ($ARGV[0] =~ m/^[a-zA-Z]\:[\\\/]/ || $ARGV[0] =~ m/^\/\//);
#$ARGV[0] =~ s|/|\\|g;

#print ReadExcel::ReadExcel("$ARGV[0]");

1;
