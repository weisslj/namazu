#
# -*- Perl -*-
# $Id: tar.pl,v 1.5 2004-10-02 12:57:29 usu Exp $
#  tar filter for namazu
#  Copyright (C) 2004 Tadamasa Teranishi,
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

package tar;
use strict;
require 'util.pl';
eval 'use Archive::Tar';
$Archive::Tar::WARN = 0;

my $tarpath = undef;

sub mediatype() {
    return ('application/x-tar', 'application/x-gtar');
}

sub status() {
    return 'yes' if (util::checklib('Archive/Tar.pm'));

    $tarpath = util::checkcmd('gtar');
    return 'yes' if (defined $tarpath);
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
#    my ($magic) = @_;
#    $magic->addFileExts('\\.tar', 'application/x-tar');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $err = undef;

    if (defined $tarpath) {
        $err = filter_gtar($orig_cfile, $cont, $weighted_str, $headings, $fields);
    } else {
        $err = filter_archive_tar($orig_cfile, $cont, $weighted_str, $headings, $fields);
    }

    return $err;
}

sub filter_gtar ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    my $uniqnumber = 0;
    my $tmpfile;
    do {
       $tmpfile = util::tmpnam('NMZ.tar' . substr("000$uniqnumber", -4));
       $uniqnumber++;
    } while (-f $tmpfile);

    {
	my $fh = util::efopen("> $tmpfile");
	print $fh $$contref;
        util::fclose($fh);
    }

    util::vprint("Processing tar file ... (using  '$tarpath')\n");

    $$contref = "";
    my %files;
    my $tmpfile2 = util::tmpnam('NMZ.tar.list');
    my $status = system("$tarpath tvf $tmpfile > $tmpfile2");
    if ($status == 0) {
        my $filelist = util::readfile("$tmpfile2");
        while ($filelist =~ s/^\S+\s+	# permission
		(?:\S+\s+)?		# (uid, giD)
		(\d+)\s+		# filesize
		\S+\s+			# year-month-day
		\S+\s+			# hour:min:sec
		(.+)$//mx)		# filename
        {
            my $name = $2;
            $files{$name} = $1;
            my $fname = "./" . $name;
            codeconv::toeuc(\$fname);
            $fname = gfilter::filename_to_title($fname, $weighted_str);
            $$contref .= $fname . " ";

            codeconv::toeuc(\$name);
            util::vprint("tar: $name");
        }
    } else {
        unlink($tmpfile2);
        unlink($tmpfile);
        return "Unable to convert file ($tarpath error occurred).";
    }
    unlink($tmpfile2);

    foreach my $fname (keys %files){
        my $size = $files{$fname};
        if ($size == 0) {
            util::dprint("$fname: filesize is 0");
        } elsif ($size > $conf::FILE_SIZE_MAX) {
            util::dprint("$fname: Too large tared file");
	} elsif ($fname =~ m!^($conf::DENY_FILE)$!i ) {
	    codeconv::toeuc(\$fname);
	    util::vprint(sprintf(_("Denied:	%s"), $fname));
	} elsif ($fname !~ m!^($conf::ALLOW_FILE)$!i) {
	    codeconv::toeuc(\$fname);
	    util::vprint(sprintf(_("Not allowed:	%s"), $fname));
        } else {
            my $tmpfile3 = util::tmpnam('NMZ.tar.file');
            my $status = system("$tarpath xfO $tmpfile \"$fname\" > $tmpfile3");
            if ($status == 0) {
                my $con = util::readfile($tmpfile3);
                unlink($tmpfile3);

                my $taredname = "tared_content";
                if ($fname =~ /.*(\..*)/){
                    $taredname = $taredname . $1;
                }
                my $err = tar::nesting_filter($taredname, \$con, $weighted_str);
                if (defined $err) {
                    util::dprint("filter/tar.pl gets error message \"$err\"");
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

sub filter_archive_tar ($$$$$) {
    my ($orig_cfile, $contref, $weighted_str, $headings, $fields)
      = @_;

    my $tmpfile;
    my $uniqnumber = int(rand(10000));
    do {
       $tmpfile = util::tmpnam('NMZ.tar' . substr("000$uniqnumber", -4));
       $uniqnumber++;
    } while (-f $tmpfile);

    {
        my $fh = util::efopen("> $tmpfile");
        print $fh $$contref;
        util::fclose($fh);
    }

    util::vprint("Processing tar file ... (using  'Archive::Tar')\n");

    my $tar = Archive::Tar->new;
    my $num = $tar->read($tmpfile, 0);

    my $err = $tar->error();
    if ($err) {
        unlink($tmpfile);
        return "Unable to convert file (Archive::Tar error occurred).";
    }

    $$contref = "";

    my @properties = ('name', 'size');
    my @list_files = $tar->list_files(\@properties);

    my %files;
    for (my $i = 0; $i <= $#list_files; $i++) {
        my $name = $list_files[$i]{'name'};
        my $size = $list_files[$i]{'size'};

        $files{$name} = $size;

        my $fname = "./" . $name;
        codeconv::toeuc(\$fname);
        $fname = gfilter::filename_to_title($fname, $weighted_str);
        $$contref .= $fname . " ";

        codeconv::toeuc(\$name);
        util::vprint("tar: $name");
    }

    foreach my $fname (keys %files){
        my $size = $files{$fname};
        if ($size == 0) {
            util::dprint("$fname: filesize is 0");
        } elsif ($size > $conf::FILE_SIZE_MAX) {
            util::dprint("$fname: Too large tared file");
	} elsif ($fname =~ m!^($conf::DENY_FILE)$!i ) {
	    codeconv::toeuc(\$fname);
	    util::vprint(sprintf(_("Denied:	%s"), $fname));
	} elsif ($fname !~ m!^($conf::ALLOW_FILE)$!i) {
	    codeconv::toeuc(\$fname);
	    util::vprint(sprintf(_("Not allowed:	%s"), $fname));
        } else {
            my $con = $tar->get_content($fname);

            my $taredname = "tared_content";
            if ($fname =~ /.*(\..*)/){
                $taredname = $taredname . $1;
            }
            my $err = tar::nesting_filter($taredname, \$con, $weighted_str);
            if (defined $err) {
                util::dprint("filter/tar.pl gets error message \"$err\"");
            }
            $$contref .= $con . " ";
        }
    }
    $tar->clear();

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
