#
# -*- Perl -*-
# $Id: doccat.pl,v 1.7 2004-10-16 14:54:12 opengl2772 Exp $
# Copyright (C) 2001 SATOH Fumiyasu,
#               2001,2004 Namazu Project. All rights reserved.
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
require 'util.pl';
require 'gfilter.pl';

my $doccatpath  = undef;

sub mediatype() {
    if (status() eq 'no') {
        return ();
    }

    my $info = "";
    my @cmd = ("$doccatpath", "-h");
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => \$info,
            "stderr" => "/dev/null",
            "mode_stdout" => "wt",
            "mode_stderr" => "wt",
        },
    );
    my @type = ();

    # Standard supported media types
    if ($info =~ /License flag: 0/i) {
	push(@type, qw(
	    application/msword
	    application/excel
	    application/powerpoint
	    application/ichitaro5
	    application/ichitaro6
	    application/ichitaro7
	    application/x-js-taro
	    application/x-lotus-wordpro
	    application/oasys
	));
        if ($info =~ /TF Library *: *Version *: *(\d+\.\d+)/i) {
            my $ver = $1;
            if ($ver >= 1.42) {
	        push(@type, 'application/rtf');
            } else {
	        util::dprint("filter/doccat.pl: No RTF support (TF library version < 1.42)");
	    }
        } else {
	    util::dprint("filter/doccat.pl: No RTF support (TF library version < 1.42)");
	}
    } else {
	util::dprint("filter/doccat.pl: No valid license for DocCat");
    }

    # Optional supported media types
    if ($info =~ /PDF Lic.* flag: 0/i) {
	push(@type, 'application/pdf');
    } else {
	util::dprint("filter/doccat.pl: No valid license for DocCat PDF option");
    }

    return @type;
}

sub status() {
    $doccatpath = util::checkcmd('doccat');
    if (defined $doccatpath) {
        my @cmd = ("$doccatpath", "-V");
        my $fh_cmd = IO::File->new_tmpfile();
        my $status = util::syscmd(
            command => \@cmd,
            option => {
                "stdout" => $fh_cmd,
                "stderr" => "/dev/null",
            },
        );
        while (<$fh_cmd>) {
            if (/DocCat *: *Version *: *(\d*)\.\d*/i) {
                my $major = $1;
                if ($major >= 3) {
                    util::fclose($fh_cmd);
                    return 'yes';
                }
            }
        }
        util::fclose($fh_cmd);
    }
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
    $magic->addFileExts('\\.pp[st]$', 'application/powerpoint');

    # Justsystem Ichitaro 5
    $magic->addFileExts('\\.jaw$', 'application/ichitaro5');
    # Justsystem Ichitaro 6
    $magic->addFileExts('\\.jbw$', 'application/ichitaro6');
    # Justsystem Ichitaro 7
    $magic->addFileExts('\\.jfw$', 'application/ichitaro7');
    # Justsystem Ichitaro 8 - 13, 2004
    $magic->addFileExts('\\.jt[dt]$', 'application/x-js-taro');

    # Lotus WordPro
    $magic->addFileExts('\\.lwp$', 'application/x-lotus-wordpro');

    # Fujitsu OASYS for Windows 6, 7, 8, 2002
    $magic->addFileExts('\\.oa[23]$', 'application/oasys');

    # Adobe PDF
    # File::MMagic detects PDF document as `application/pdf'

    # RTF
    $magic->addFileExts('\\.rtf$', 'application/rtf');

    $magic->addMagicEntry("0\tstring\t{\\rtf\t");
    $magic->addMagicEntry(">6\tstring\t\\ansi\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\mac\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\pc\tapplication/rtf");
    $magic->addMagicEntry(">6\tstring\t\\pca\tapplication/rtf");

    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    my $tmpfile = util::tmpnam('NMZ.doccat');
    {
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
        util::fclose($fh);
    }

    my $tmpfile2  = util::tmpnam('NMZ.doccat2');
    my @cmd = ("$doccatpath", "-p", "-o", "e", "$tmpfile");
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $tmpfile2,
            "stderr" => "/dev/null",
            "mode_stdout" => "wt",
            "mode_stderr" => "wt",
        },
    );

    {
        my $size = util::filesize($tmpfile2);
        if ($size == 0) {
            unlink($tmpfile);
            unlink($tmpfile2);
            return "Unable to convert file ($doccatpath error occurred).";
        }
        if ($size > $conf::TEXT_SIZE_MAX) {
            unlink($tmpfile);
            unlink($tmpfile2);
            return 'Too large doccat file.';
        }
    }

    {
        $$cont = util::readfile($tmpfile2, "t");
    }

    unlink($tmpfile);
    unlink($tmpfile2);

    doccat_filter($orig_cfile, $cont, $weighted_str, $headings, $fields);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
        unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str,
               $fields, $headings);
    return undef;
}

sub doccat_filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';

    # Check if no properties part
    if ($$cont =~ s/^\n//s) {
	# Do nothing in this sub routine
	return undef;
    }
    # Check if broken DocCat :-(
    if ($$cont !~ /^([\w\-]+): /s) {
	# Workaround for broken DocCat that does not output null
	# line before part of document body if document has no
	# any properties.
	return undef;
    }

    # Get part of document properties from content
    if ($$cont !~ s/^(.+?)\n\n//s) {
	# No properties part. This is broken DocCat!? :-(
	return undef;
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
	$fields->{'title'} = $subject;
    }

    if (defined $property{'author'} || defined $property{'operator'}) {
	# Remove duplicated name
	my %count = ();
	my @author = grep {
	    !$count{$_}++
	} @{$property{'author'}}, @{$property{'operator'}};

	$fields->{'author'} = join(', ', @author);
    }

    if (defined $property{'keywords'}) {
	my $keywords = join(' ', @{$property{'keywords'}});
        my $weight = $conf::Weight{'metakey'};
        $$weighted_str .= "\x7f$weight\x7f$keywords\x7f/$weight\x7f\n";

#        if ($var::Opt{'meta'}) {
#            my @keys = split '\|', $conf::META_TAGS;
#            for my $key (@keys) {
#                if ($key =~ m/^keywords$/i) {
#                    $fields->{'keywords'} = $keywords;
#                }
#                util::dprint("meta: keywords: $fields->{'keywords'}\n")
#                    if defined $fields->{'keywords'};
#                }
#            }
#        }
    }

    if (defined $property{'software'}) {
	my $software = ${$property{'software'}}[0];
	$fields->{'software'} = $software;
    }

    if (defined $property{'company'}) {
	my $company = ${$property{'company'}}[0];
	$fields->{'company'} = $company;
    }

    return undef;
}

1;
