#!/usr/local/bin/perl -w
#
# $Id: towin.pl,v 1.3 2005-12-21 09:28:20 opengl2772 Exp $
# Copyright (C) 2004 Tadamasa Teranishi
#               2004 Namazu Project All rights reserved.
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
#

use strict;
use File::Copy;

my $SYSTEM = $^O;

my @TESTS = (
    'mknmz-1.pl', 'mknmz-2.pl', 'mknmz-4.pl', 
    'gcnmz-1.pl',
    'mknmz-5.pl', 'mknmz-6.pl', 'mknmz-7.pl', 'mknmz-8.pl',
    'mknmz-10.pl', 'mknmz-12.pl', 'mknmz-14.pl', 'mknmz-15.pl',
    'mknmz-16.pl', 'mknmz-17.pl', 'mknmz-18.pl',
    'idxdiff-1.pl', 'idxdiff-3.pl',
    'namazu-1.pl', 'namazu-2.pl', 'namazu-3.pl', 'namazu-4.pl',
    'namazu-7.pl', 'namazu-8.pl', 'namazu-9.pl', 
    'namazu-10.pl', 'namazu-11.pl', 'namazu-12.pl',
    'namazu-cgi-1.pl', 'namazu-cgi-2.pl', 
    'namazu-cgi-3.pl', 'namazu-cgi-4.pl', 'namazu-cgi-5.pl',
    'namazu-cgi-7.pl', 'namazu-cgi-8.pl',
    'namazu-cgi-9.pl', 'namazu-cgi-10.pl',
    'chasen-1.pl', 'chasen-2.pl', 'chasen-3.pl',
    'mecab-1.pl', 'mecab-2.pl', 'mecab-3.pl',
    'kakasi-1.pl', 'kakasi-2.pl', 'kakasi-3.pl',
    'pltests.pl',
);

if (!($SYSTEM eq "MSWin32" || $SYSTEM eq "os2")) {
    warn 'This program operates only by Windows.';
    exit 1;
}

my $comspec = "cmd";
$comspec = $ENV{'COMSPEC'} if (defined $ENV{'COMSPEC'});

my $file = "alltests.pl";
my @cmd = ( "$comspec", "/c", "pl2bat", "-s", "/.pl.in/", "$file.in" );
print join(" ", @cmd) . "\n";
my $status = system { $cmd[0] } @cmd;
warn "Error : $!" if ($status != 0);

foreach $file (@TESTS) {
    print "copy $file.in $file\n";
    copy("$file.in", "$file");
}

exit 0;
