#
# -*- Perl -*-
# $Id: zip.pl,v 1.6 2004-05-04 06:08:47 usu Exp $
#  zip filter for namazu
#  Copyright (C) 2004 MATSUMURA Namihiko <po-jp@counterghost.net>
#                2004 Yukio USUDA <usu@namazu.org>
#                2004 Namazu Project All rights reserved.
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

package zip;
use strict;
require 'util.pl';

my $unzippath;

sub mediatype() {
    return ('application/x-zip');
}

sub status() {
    $unzippath = util::checkcmd('unzip');
    return 'yes' if (defined $unzippath);
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
    $magic->addFileExts('\\.zip', 'application/x-zip');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    my $depth = 0;
    my $tmpfile;
    do {
	$tmpfile = util::tmpnam('NMZ.zip' . substr("000$depth", -1, 4));
	$depth++;
    } while ( -f $tmpfile);

    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$contref;
        util::fclose($fh);
    }

    util::vprint("Processing zip file ... (using  '$unzippath')\n");

    $$contref = "";

    my $status = system("$unzippath -P passwd -qq -t $tmpfile");
    if ($status != 0) {
        unlink($tmpfile);
        return 'Unable to convert zip file (maybe copying protection)';
    }

    my $tmpfile2 = util::tmpnam('NMZ.zip_comment');
    $status = system("$unzippath -z -qq $tmpfile > $tmpfile2");
    if ($status == 0) {
	my $summary = util::readfile("$tmpfile2");
        codeconv::toeuc(\$summary);
	$$contref .= $summary . " ";
    }
    unlink($tmpfile2);

    my %files;
    my $filenames;
    $status = system("$unzippath -Z $tmpfile > $tmpfile2");
    if ($status == 0) {
	my $filelist = util::readfile("$tmpfile2");
	while ($filelist =~/\n\S+\s+	# permission
			\S+\s+		# version
			(\S+)\s+	# filesystem
			(\d+)\s+	# filesize
			\S+\s+		#
			\S+\s+		#
			\S+\s+		# day-month-year
			\S+\s+		# hour:min
			(.+)/gx){	# filename
	    my $filename = $3;
	    $files{$filename} = $2;
	    my $filesystem = $1;
	    # The unzip output japanese filename incorrectly when filesystem
	    # attribute is 'fat' or 'hpfs'.
	    if ($filesystem =~ /unx|nft/) {
		$filename = './' . $filename;
		codeconv::toeuc(\$filename);
		$filename = gfilter::filename_to_title($filename, $weighted_str);
		$filenames .= $filename . " ";
	    }
	}
    }
    $$contref .= $filenames . " ";

    my $fname;
    foreach $fname (keys %files){
	my $size = $files{$fname};
	if ($size == 0) {
	    util::dprint("$fname: filesize is 0");
	} elsif ($size > $conf::FILE_SIZE_MAX) {
	    util::dprint("$fname: Too large ziped file");
	} else {
	    my $con = "";
	    my $fh = util::efopen("$unzippath -p $tmpfile \"$fname\"|");
	    while (defined(my $line = <$fh>)){
		$con .= $line;
	    }
	    my $unzippedname = "unzipped_content";
	    if ($fname =~ /.*(\..*)/){
		$unzippedname = $unzippedname . $1;
	    }
	    my $err = zip::nesting_filter($unzippedname, \$con, $weighted_str);
	    if (defined $err) {
		util::dprint("filter/zip.pl gets error message \"$err\"");
	    }
	    $$contref .= $con . " ";
	    util::fclose($fh);
	}
    };
    unlink($tmpfile);
    return undef;
}

sub nesting_filter ($$$){
    my ($filename, $contref, $weighted_str) = @_;
    my $err = undef;
    my $dummy_shelterfname = "";
    my $headings = "";
    my %fields;
    my $mmtype = undef;

    my ($kanji, $mtype) = mknmz::apply_filter(\$filename, $contref, 
			$weighted_str, \$headings, \%fields, 
			$dummy_shelterfname, $mmtype);

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
