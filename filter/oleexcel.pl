#
# -*- Perl -*-
# $Id: oleexcel.pl,v 1.1 1999-11-14 22:59:48 kenzo- Exp $
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
#                  Get Text From Grouped Shape Items
# V2.10 1999/11/09 Change Name 
#                  Merge three program ReadMSWord.pl, ReadExcel.pl, ReadPPT.pl
# V2.11 1999/11/15  separate file.
#					modify some functions.
#

package oleexcel;
#use strict;
use Win32::OLE;
use Win32::OLE::Enum;
require 'util.pl';
require 'gfilter.pl';

sub mediatype() {
    return ('application/excel');
}

sub status() {
    my $excel = Win32::OLE->GetActiveObject('Excel.Application');
    return 'yes' if (defined $excel);
    return 'no';
}

sub recursive() {
    return 0;
}

sub codeconv() {
    return 0;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    util::vprint("Processing excel file ...\n");

    mailnews_filter($cont, $weighted_str, $fields);
    mailnews_citation_filter($cont, $weighted_str);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    gfilter::show_filter_debug_info($cont, $weighted_str,
			   $fields, $headings);
    return undef;
}


# Original of this code was contributed by <jun-krb@mars.dti.ne.jp>. 

sub enum($$) {
    my $enum_objs = shift;
    my $func = shift;

    die "No Objects or No Function" unless ($enum_objs or $enum_func );

    my $e = Win32::OLE::Enum->new($enum_objs);
    while(($obj = $e->Next)) {
	return 0 if (!&$func($obj));
    }
    return 1;
}

sub getProperties($) {
    my $doc = shift;

    # get Title
    $title = $doc->BuiltInDocumentProperties(1)->{Value};
    $subject = $doc->BuiltInDocumentProperties(2)->{Value};
    $author = $doc->BuiltInDocumentProperties(3)->{Value};
    $lastAuthor = $doc->BuiltInDocumentProperties(7)->{Value};
    $createDate = $doc->BuiltInDocumentProperties(11)->{Value};
    $editDate = $doc->BuiltInDocumentProperties(13)->{Value};

    my $allText = '';
    $allText .= 'Subject: ' . $title . ' ' . $subjext ;
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

package ReadExcel;

sub ReadExcel($) {
    my $fileName = shift;

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
    # for debug
    $excel->{Visible} = 1;
    my $wb = $excel->Workbooks->Open($fileName);
    print $fileName,"\n";
    print $wb,"\n";
    die "Cannot open File $fileName" unless ( defined $wb );

    my $allText = '';
    $allText = '';
    $allText .= oleexcel::getProperties($wb);
    $allText .= getSheets($wb);
    $wb->close(0);
    undef $wb;
    undef $excel;

    return $allText;
}

sub getSheets($) {
    my $wb = shift;
    my $allText = '';
    my $enum_a_sheet = sub {
		my $obj = shift;
		$allText .= getCells($obj);
		$allText .= getShapes($obj);
		return 1;
    };

    oleexcel::enum($wb->Worksheets, $enum_a_sheet);

    return $allText;
}

sub getCells($) {
    # parameter 
    # sheet: Sheet Object
    # Return Value
    #   Text which is contained in cell on a sheet
    my $sheet = shift;
    my $allText = '';
    my $ur = $sheet->{UsedRange};
    my $enum_a_cell = sub {
		my $cell = shift;
		$allText .= $cell->{Value};
		$allText .= "\n";
		return 1;
    };

    oleexcel::enum($ur->Cells, $enum_a_cell);
    return $allText;
}

sub getShapes($) {
    my $sheet = shift;
	my $allText = '';

    sub enum_a_shape($) {
	my $obj = shift;
	if ( $obj->{Type} == 6 ) { #msoShapeGroup = 6
	    oleexcel::enum($obj->GroupItems,\&enum_a_shape);
	} elsif ($obj->{Type} == 17 ) { # msoShapeTextBox = 17
	    my $p = $obj->TextFrame->Characters->{Text};
	    $allText .= $p;
	    $allText .= "\n";
	}
	return 1;
    };

    oleexcel::enum($sheet->Shapes,\&enum_a_shape);
    return $allText;
}

1
