#
# -*- Perl -*-
# $Id: doccat.pl,v 1.1 2004-05-10 09:17:55 fumiya Exp $
# Copyright (C) 2001 SATOH Fumiyasu,
#               2001 Namazu Project. All rights reserved.
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

# This is a filter for Windows Office documents that are converted
# to plain text by using Dehenken DocCat (the `doccat' command).
# For more details about DocCat, see http://www.dehenken.co.jp/
# (Japanese only).
#
# This filter supports DocCat 3.00 or later (Dehenken TF Library
# 1.41 or later). To check your DocCat version, run `doccat -h`.

package doccat;
use strict;
use File::Copy;
require 'util.pl';
require 'gfilter.pl';

my $doccatpath  = undef;

sub mediatype() {
    if (status() eq 'no') {
	return ();
    }

    my $info = `$doccatpath -h`;
    my @type = ();

    # Standard supported media types
    if ($info =~ /License flag: 0/i) {
	push(@type, qw(
	    application/msword
	    application/excel
	    application/powerpoint
	    application/ichitaro6
	    application/ichitaro
	    application/x-lotus-wordpro
	    application/oasys
	));
    }

    # Optional supported media types
    if ($info =~ /PDF Lic.* flag: 0/i) {
	push(@type, 'application/pdf');
    }

    return @type;
}

sub status() {
    $doccatpath = util::checkcmd('doccat');
    return 'yes' if defined $doccatpath;
    return 'no'; 
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

    # MS Word
    # File::MMagic detects MS Word document `application/msword'

    # MS Excel
    $magic->addFileExts('\\.xls$', 'application/excel');

    # MS PowerPoint
    $magic->addFileExts('\\.ppt$', 'application/powerpoint');

    # Justsystem Ichitaro 6, 7
    $magic->addFileExts('\\.j[bf]w$', 'application/ichitaro');
    # Justsystem Ichitaro 8, 9, 10, 11
    $magic->addFileExts('\\.jt[dt]$', 'application/ichitaro');
    # File::MMagic detects Ichitaro 6 document as `application/ichitaro6'

    # Lotus WordPro
    $magic->addFileExts('\\.lwp$', 'application/x-lotus-wordpro');

    # Fujitsu OASYS for Windows 6, 7
    $magic->addFileExts('\\.oa[23]$', 'application/oasys');

    # Adobe PDF
    # File::MMagic detects PDF document as `application/pdf'

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile  = util::tmpnam('NMZ.doccat');
    my $tmpfile2 = util::tmpnam('NMZ.doccat2');
    copy("$cfile", "$tmpfile2");

    system("$doccatpath -p -oe $tmpfile2 > $tmpfile");

    {
        my $fh = util::efopen("< $tmpfile");
        $$cont = util::readfile($fh);
    }

    unlink($tmpfile);
    unlink($tmpfile2);

    doccat_filter($cont, $cfile, $weighted_str, $fields);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    gfilter::show_filter_debug_info($cont, $weighted_str,
               $fields, $headings);
    return undef;
}

sub doccat_filter ($$$$) {
    my ($cont, $cfile, $weighted_str, $fields) = @_;

    # By most users, filename is used as document title instead of
    # `subject' property.
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str);

    # Check if no properties part
    if ($$cont =~ s/^\n//s) {
	# Do nothing in this sub routine
	return;
    }
    # Check if broken DocCat :-(
    if ($$cont !~ /^([\w\-]+): /s) {
	# Workaround for broken DocCat that does not output null
	# line before part of document body if document has no
	# any properties.
	return;
    }

    # Get part of document properties from content
    if ($$cont !~ s/^(.+?)\n\n//s) {
	# No properties part. This is broken DocCat!? :-(
	return;
    }
    my $property_chunk = $1;
    my %property = ();
    my $name = undef;
    foreach my $line (split(/\n/, $property_chunk)) {
	if ($line =~ /^([\w\-]+): (.*)$/) {
	    $name = lc($1);
	    push(@{$property{$name}}, $2);
	} elsif ($line =~ /^\s+(.*)$/) {
	    # Valid continuous line.
	    $property{$name}->[-1] .= " $1";
	} else {
	    # No heading space(s) in continuous line.
	    # This is workaround for broken DocCat :-(
	    $property{$name}->[-1] .= " $line";
	}
    }

    # Discard spaces-only properties, Trim heading/trailing spaces
    foreach my $name (keys(%property)) {
	@{$property{$name}} = map {
	    /^\s*(.*?)\s*$/
	} grep(/\S/, @{$property{$name}});
    }

    if (defined $property{'subject'}) {
	my $subject = ${$property{'subject'}}[0];
	$fields->{'title'} .= " ($subject)";
    }

    if (defined $property{'author'} || defined $property{'operator'}) {
	# Remove duplicated name
	my %count = ();
	my @author = grep {
	    !$count{$_}++
	} @{$property{'author'}}, @{$property{'operator'}};

	$fields->{'author'} = join(', ', @author);
    }

    # FIXME: Support other properties such as 'keywords'
}

1;
