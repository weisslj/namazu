#
# -*- Perl -*-
# $Id: zip.pl,v 1.2 2004-04-29 15:22:41 opengl2772 Exp $
#  zip filter for namazu
#  Copyright (C) 2004 MATSUMURA Namihiko <po-jp@counterghost.net>
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
use File::Find;
use Cwd;
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

    my $tmpfile = util::tmpnam('NMZ.zip');
    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$contref;
        util::fclose($fh);
    }

    my $tmpdir = util::tmpnam('NMZ.zip_dir');
    my $cwd = cwd();
    $tmpdir = mknmz::absolute_path($cwd, $tmpdir);
    rm_r($tmpdir) if (-d $tmpdir);
    mkdir($tmpdir);

    util::vprint("Processing zip file ... (using  '$unzippath')\n");

    system("$unzippath -P passwd -qq -d $tmpdir $tmpfile");
    my $status = $?;

    $$contref = "";

    if ($status != 0) {
        unlink($tmpfile);
        rm_r($tmpdir);
        return 'Unable to convert zip file (maybe copying protection)';
    }

    my $tmpfile2 = util::tmpnam('NMZ.zip2');
    system("$unzippath -z -qq $tmpfile > $tmpfile2");
    $status = $?;
    if ($status == 0) {
	my $summary = util::readfile("$tmpfile2");
        codeconv::toeuc(\$summary);
	$$contref .= $summary . " ";
    }
    unlink($tmpfile2);

    my $sub = sub {
	my $tmpfile = "$File::Find::dir/$_";
	if (-f $tmpfile) {
	    my $fh = util::efopen("$tmpfile");
	    my $con = util::readfile($fh);
	    my $err = zip::nesting_filter($tmpfile, \$con, $weighted_str);
	    if (defined $err) {
		util::dprint("filter/zip.pl gets error message \"$err\"");
	    }
	    $$contref .= $con . " ";
	    util::fclose($fh);
	    unlink($tmpfile);
	}
    };
    find ($sub, $tmpdir);
    unlink($tmpfile);
    rm_r($tmpdir);
    return undef;
}

sub rm_r {
    my ($targetdir) = @_;
    my @dirs;
    my $sub = sub {
	my $tmpfile = "$File::Find::dir/$_";
	if (-f $tmpfile) {
	    unlink($tmpfile);
	} elsif (-d $tmpfile) {
	    unshift @dirs, $tmpfile
	}
    };
    find($sub, $targetdir);
    foreach (@dirs) {
	rmdir $_;
    }
    rmdir $targetdir;
}

sub nesting_filter ($$$){
    my ($filename, $contref, $weighted_str) = @_;
    my $err = undef;
    my $dummy_shelterfname = "";
    my $headings = "";
    my %fields;
    my $mmtype = undef;
    codeconv::toeuc(\$filename);
    $filename = gfilter::filename_to_title($filename, $weighted_str);

    my ($kanji, $mtype) = mknmz::apply_filter(\$filename, $contref, 
			$weighted_str, \$headings, \%fields, 
			$dummy_shelterfname, $mmtype);

    $$contref .= " ". $filename;

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
