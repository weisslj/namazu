#
# -*- Perl -*-
# $Id: lha.pl,v 1.19 2008-04-29 03:21:34 usu Exp $
#  lha filter for namazu
#  Copyright (C) 2004-2007 Tadamasa Teranishi,
#                2004 MATSUMURA Namihiko <po-jp@counterghost.net>,
#                2004-2007 Namazu Project All rights reserved.
#
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

package lha;
use strict;
use English;
require 'util.pl';
require 'document.pl';

my $haslhapm = undef;
my $lhapath = undef;

sub mediatype() {
    return ('application/x-lha');
}

sub status() {
    if (util::checklib('Archive/Lha.pm')){
        $haslhapm = 1;
    }

    return 'no' if ($English::OSNAME =~ /^(?:MSWin32|os2)$/i);
    # Only LHa for UNIX.
    $lhapath = util::checkcmd('lha');

    return 'yes' if (defined $lhapath);
    return 'yes' if (defined $haslhapm);
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

    $magic->addMagicEntry("2\tstring\t-lh0-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh1-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh2-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh3-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh4-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh5-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh6-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lh7-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lhd-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lzs-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lz4-\tapplication/x-lha");
    $magic->addMagicEntry("2\tstring\t-lz5-\tapplication/x-lha");

    # FIXME: very ad hoc.
#    $magic->addFileExts('\\.lzh$', 'application/x-lha');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
        = @_;
    my $err = undef;

    if (defined $haslhapm) {
        $err = filter_arc_lha($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else {
        $err = filter_lha_unix($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }
    return $err;
}


sub filter_arc_lha ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    eval 'use Carp;';
    eval 'use Archive::Lha::Stream;';
    eval 'use Archive::Lha::Header;';
    eval 'use Archive::Lha::Decode;';

    my $processedcont;
    my $stream = Archive::Lha::Stream->new( string => $$contref );
    while(defined(my $level = $stream->search_header)) {
        my $header = Archive::Lha::Header->new(
          level => $level,
          stream => $stream
        );
        $stream->seek($header->data_top);
        my $fname = $header->pathname('guess' => 'utf-8');
        $processedcont .= $fname . " ";
        my $decodedcont;
        my $decoder = Archive::Lha::Decode->new(
            header => $header,
            read   => sub { $stream->read(@_) },
            write  => sub { ($decodedcont) = @_ }, 
        );
        my $crc16 = $decoder->decode;
        return "crc mismatch" if $crc16 != $header->crc16;
        my $lhaedname = "lhaed_content";
        if ($fname =~ /.*(\..*)/){
            $lhaedname = $lhaedname . $1;
        }
        my $err = lha::nesting_filter($lhaedname, \$decodedcont, $weighted_str);
        if (defined $err) {
            util::dprint("filter/lha.pl gets error message \"$err\"");
        }
        $processedcont .= $decodedcont . " ";
    }
    $$contref = $processedcont;
    return undef;
}

sub filter_lha_unix ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    my $tmpfile;
    my $uniqnumber = int(rand(10000));
    do {
       $tmpfile = util::tmpnam('NMZ.lha' . substr("000$uniqnumber", -4));
       $uniqnumber++;
    } while (-f $tmpfile);

    {
        my $fh = util::efopen("> $tmpfile");
        print $fh $$contref;
        util::fclose($fh);
    }

    util::vprint("Processing lha file ... (using  '$lhapath')\n");

    my %files;
    my $tmpfile2 = util::tmpnam('NMZ.lha.list');
    my @cmd = ("$lhapath", "lq2g", "$tmpfile");
    my $status = util::syscmd(
        command => \@cmd,
        option => {
            "stdout" => $tmpfile2,
            "stderr" => "/dev/null",
        },
    );
    if ($status == 0) {
        my $filelist = util::readfile("$tmpfile2");
        codeconv::normalize_document(\$filelist);
        while ($filelist =~ s/^\S+\s+	# permission
		(?:\S+\s+)?		# (uid, giD)
		(\d+)\s+		# filesize
		\S+\s+			# rate
		\S+\s+			# month
		\S+\s+			# day
		\S+\s+			# year
		(.+)$//mx)		# filename
        {
            my $name = $2;
            $files{$name} = $1;
            my $fname = "./" . $name;
	    codeconv::to_inner_encoding(\$fname, 'unknown');
            $fname = gfilter::filename_to_title($fname, $weighted_str);
            $$contref .= $fname . " ";

	    codeconv::to_inner_encoding(\$name, 'unknown');
            util::vprint("lha: $name");
        }
    } else {
        unlink($tmpfile2);
        unlink($tmpfile);
        return "Unable to convert file ($lhapath error occurred).";
    }
    unlink($tmpfile2);

    $$contref = "";

    foreach my $fname (keys %files){
        my $size = $files{$fname};
        if ($size == 0) {
            util::dprint("$fname: filesize is 0");
        } elsif ($size > $conf::FILE_SIZE_MAX) {
            util::dprint("$fname: Too large lhaed file");
	} elsif ($fname =~ m!^($conf::DENY_FILE)$!i ) {
	    codeconv::to_inner_encoding(\$fname, 'unknown');
	    util::vprint(sprintf(_("Denied:	%s"), $fname));
	} elsif ($fname !~ m!^($conf::ALLOW_FILE)$!i) {
	    codeconv::to_inner_encoding(\$fname, 'unknown');
	    util::vprint(sprintf(_("Not allowed:	%s"), $fname));
        } else {
            my $tmpfile3 = util::tmpnam('NMZ.lha.file');
            my @cmd = ("$lhapath", "-pq2", "$tmpfile", "$fname");
            my $status = util::syscmd(
                command => \@cmd,
                option => {
                    "stdout" => $tmpfile3,
                    "stderr" => "/dev/null",
                    "mode_stdout" => "wb",
                    "mode_stderr" => "wt",
                },
            );
            if ($status == 0) {
                my $con = util::readfile($tmpfile3);
                unlink($tmpfile3);

                my $lhaedname = "lhaed_content";
                if ($fname =~ /.*(\..*)/){
                    $lhaedname = $lhaedname . $1;
                }
                my $err = lha::nesting_filter($lhaedname, \$con, $weighted_str);
                if (defined $err) {
                    util::dprint("filter/lha.pl gets error message \"$err\"");
                }
                $$contref .= $con . " ";
            } else {
                unlink($tmpfile3);
            }
        }
    }
    unlink($tmpfile);
    return undef;
}

sub nesting_filter ($$$){
    my ($filename, $contref, $weighted_str) = @_;
    my $err = undef;
    my $headings = "";
    my %fields;
    my $mmtype = undef;

    my $mtype;
    {
	my $uri;
	my $Document = undef;
	$Document = mknmz::document->new();
	$Document->init_doc($uri, $filename, $contref, $mmtype);
	$$contref = ${$Document->get_filtered_contentref()};
	$mtype = $Document->get_mimetype();
	$$weighted_str = $Document->get_weighted_str();
	$headings = $Document->get_headings();
	%fields = $Document->get_fields();
    }
    if ($mtype =~ /; x-system=unsupported$/){
        $$contref = "";
        $err = $mtype;
    }elsif ($mtype =~ /; x-error=(.*)$/){
        $$contref = "";
        $err = $1;
    }else{
        gfilter::show_filter_debug_info($contref, $weighted_str,
					\%fields, \$headings);
        for my $field (keys %fields) {
            $$contref .= " ". $fields{$field};
        }
    }
    return $err;
}

1;
