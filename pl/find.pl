#
# -*- Perl -*-
# $Id: find.pl,v 1.6 2000-01-27 13:13:38 satoru Exp $
# Copyright (C) 2000 Namazu Project All rights reserved..
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
#  NOTE: Original of this code is "find.pl" in the perl distribution.

package find;
no strict;
use Cwd;

sub find {
    my ($rarray, @target) = @_;
    local ($cwd);
    local ($dev, $dir, $dont_use_nlink, $fixtopdir, $ino, $mode,
	$name, $nlink, $prune, $subcount, $tmp, $topdev, $topdir, $topino,
	$topmode, $topnlink);

    $dont_use_nlink = 1; ## S.Takabayashi added.
    $cwd = cwd();

    for $topdir (@target) {
	(($topdev,$topino,$topmode,$topnlink) = stat($topdir))
	  || (warn(_("Can't stat ")."$topdir: $!\n"), next);

	if (-d $topdir) { 
	    if (chdir($topdir)) {
		($dir,$_) = ($topdir,'.');
		$name = $topdir;
		mknmz::wanted($name,$rarray);
		($fixtopdir = $topdir) =~ s!/$!! ;
		finddir($fixtopdir,$topnlink,$rarray);
	    }
	    else {
		warn _("Can't cd to ") ."$topdir: $!\n";
	    }
	} else {
	    unless (($dir,$_) = $topdir =~ m#^(.*/)(.*)$#) {
		($dir,$_) = ('.', $topdir);
	    }
	    $name = $topdir;
	    chdir $dir && mknmz::wanted($name,$rarray);
	}
	chdir $cwd;
    }
}


# Sort file names with consideration for numbers.
# Original of this code was contributed by <furukawa@tcp-ip.or.jp>. 
sub fncmp {
    my ($x, $y) = ($a, $b);

    $x =~ s/(\d+)/sprintf("%08d", $1)/ge;
    $y =~ s/(\d+)/sprintf("%08d", $1)/ge;
    $x cmp $y;
}

sub finddir {
    my ($dir,$nlink,$rarray) = @_;
    local ($dev,$ino,$mode,$subcount);
    local ($name);
    local ($tmp);   ## S.Takabayashi added.

    ## Check .htaccess
    ## Added by G.Kunito <kunito@hal.t.u-tokyo.ac.jp>
    if( $var::Opt{'HtaccessExclude'} && 
       ( -f ".htaccess" ) && html::parse_htaccess() ){
	printf ("%s "._("is exclude because of .htaccess\n"), cwd());
	return(0);
    }
    ##


    # Get the list of files in the current directory.

    opendir(DIR,'.') || (warn _("Can't open ")."$dir: $!\n", return);
    my @filenames = grep(!/^\.\.?$/, readdir(DIR));
    closedir(DIR);

    # sort filenames with fncmp()
    @filenames = sort fncmp @filenames;

    if ($nlink == 2 && !$dont_use_nlink) {  # This dir has no subdirectories.
	for (@filenames) {
	    $name = "$dir/$_";
	    $nlink = 0;
	    mknmz::wanted($name,$rarray);
	}
    }
    else {                    # This dir has subdirectories.
	$subcount = $nlink - 2;
	for (@filenames) {
	    $nlink = $prune = 0;
	    $name = "$dir/$_";
	    mknmz::wanted($name,$rarray);
	    if ($subcount > 0 || $dont_use_nlink) {    # Seen all the subdirs?

		# Get link count and check for directoriness.
		## Modified by S.Takabayashi lstat() -> stat()
		($dev,$ino,$mode,$nlink) = stat($_) unless $nlink;
		
		if (-d $_) { 
		    # It really is a directory, so do it recursively.
		    ## and Symbolic Linked dir, also do it.
		    $tmp = '..';
		    if (-l $_) {
			($dev,$ino,$mode,$nlink) = lstat($_);
			if ($SymLinks{"$dev $ino"}) {
			    print _("Looped symbolic link detected ")."[$name],"._(" skipped.\n");
			    last;
			}
			$SymLinks{"$dev $ino"} = 1;

			$tmp = cwd();
		    }

		    if (!$prune && chdir $_) {
			finddir($name,$nlink,$rarray);
			chdir $tmp;
		    }
		    --$subcount;
		}
	    }
	}
    }
}

# public function.
sub findfiles ($\@) {
    my ($rarray) = @_;
    find($rarray, cwd());
}

1;
