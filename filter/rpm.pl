#
# -*- Perl -*-
# $Id: rpm.pl,v 1.9 2004-01-16 08:47:54 opengl2772 Exp $
# Copyright (C) 2000 Namazu Project All rights reserved ,
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

package rpm;
use strict;
require 'util.pl';
require 'gfilter.pl';

my $rpmpath = undef;
my @rpmopts = undef;
my $envpath = undef;

sub mediatype() {
    return ('application/x-rpm');
}

sub status() {
    $rpmpath = util::checkcmd('rpm');
    @rpmopts = ("-qpi");
    $envpath = util::checkcmd('env');
    return 'no' unless (defined $rpmpath && defined $envpath);
    return 'yes';
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

    $magic->addFileExts('\\.rpm$', 'application/x-rpm');
    return;
}

sub filter ($$$$$) {
    my ($orig_cfile, $cont, $weighted_str, $headings, $fields)
      = @_;
    my $cfile = defined $orig_cfile ? $$orig_cfile : '';
    my $size = 0;

    util::vprint("Processing rpm file ... (using  '$rpmpath')\n");

    my $tmpfile = util::tmpnam('NMZ.rpm');
    {   
        my $fh = util::efopen("> $tmpfile");
        print $fh $$cont;
    }

    {
        my @env = ($envpath, "LC_MESSAGE=$util::LANG", "LC_TIME=C");
	my @cmd = (@env, $rpmpath, @rpmopts, $tmpfile);
	my ($status, $fh_out, $fh_err) = util::systemcmd(@cmd);
	my $size = util::filesize($fh_out);
	if ($size == 0) {
	    return "Unable to convert file ($rpmpath error occurred).";
	}
	if ($size > $conf::TEXT_SIZE_MAX) {
	    return 'Too large rpm file.';
	}
	$$cont = util::readfile($fh_out);
    }
    unlink $tmpfile;

    rpm_filter($cont, $weighted_str, $fields, $headings);

    gfilter::line_adjust_filter($cont);
    gfilter::line_adjust_filter($weighted_str);
    gfilter::white_space_adjust_filter($cont);
    $fields->{'title'} = gfilter::filename_to_title($cfile, $weighted_str)
	unless $fields->{'title'};
    gfilter::show_filter_debug_info($cont, $weighted_str,
				    $fields, $headings);

    return undef;
}

sub rpm_filter ($$$$) {
    my ($contref, $weighted_str, $fields, $headings) = @_;

    rpm::get_title($contref, $weighted_str, $fields);
    rpm::get_author($contref, $fields);
    rpm::get_date($contref, $fields);
    rpm::get_size($contref, $fields);
    rpm::get_summary($contref, $fields);

    return;
}


# Below is the sample result of 'rpm -qi rpm'.
#
# Name        : rpm                          Relocations: (not relocateable)
# Version     : 3.0.5                             Vendor: (none)
# Release     : 3k                            Build Date: Fri Jun 30 10:12:19 2000
# Install date: Thu Sep 14 21:32:32 2000      Build Host: omoi.kondara.org
# Group       : System Environment/Base       Source RPM: rpm-3.0.5-3k.nosrc.rpm
# Size        : 3111493                          License: GPL
# URL         : http://www.rpm.org/
# Summary     : The Red Hat package management system.
# Description :
# The RPM Package Manager (RPM) is a powerful command line driven
# package management system capable of installing, uninstalling,
# verifying, querying, and updating software packages.  Each software
# package consists of an archive of files along with information about
# the package like its version, a description, etc.


sub get_title ($$$) {
    my ($contref, $weighted_str, $fields) = @_;

    if ($$contref =~ /Summary     : (.*)/) {
	my $tmp = $1;
	$fields->{'title'} = $tmp;
	my $weight = $conf::Weight{'html'}->{'title'};
	$$weighted_str .= "\x7f$weight\x7f$tmp\x7f/$weight\x7f\n";
    }
}

sub get_author ($$) {
    my ($contref, $fields) = @_;

    if ($$contref =~ /Vendor: (.*)/) {
	my $tmp = $1;
	$fields->{'author'} = $tmp;
    }
}

sub get_date ($$) {
    my ($contref, $fields) = @_;

    if ($$contref =~ /Build Date: (.*)/) {
	my $time = $1;
        my $err = time::ctime_to_rfc822time(\$time);
        $fields->{'date'} = $time unless ($err);
    }
}

sub get_size ($$) {
    my ($contref, $fields) = @_;

    if ($$contref =~ /Size\s+: (.*)$/) {
	my $tmp = $1;
	$fields->{'size'} = $tmp;
    }
}

sub get_summary ($$) {
    my ($contref, $fields) = @_;

    $$contref =~ s/^.*Description ://is;
    if ($$contref =~ /Description : (.*)/) {
	my $tmp = $1;
	$fields->{'summary'} = $tmp;
    }
}

1;
