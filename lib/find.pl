#
# -*- Perl -*-
# $Id: find.pl,v 1.4 1999-08-28 02:43:15 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
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
#  find.pl をほんの少し修正して Symbolic link なディレクトリもたどるようにした

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
	  || (warn("Can't stat $topdir: $!\n"), next);

	if (-d _) { 
	    if (chdir($topdir)) {
		($dir,$_) = ($topdir,'.');
		$name = $topdir;
		namazu::wanted($name,$rarray);
		($fixtopdir = $topdir) =~ s!/$!! ;
		finddir($fixtopdir,$topnlink,$rarray);
	    }
	    else {
		warn "Can't cd to $topdir: $!\n";
	    }
	} else {
	    unless (($dir,$_) = $topdir =~ m#^(.*/)(.*)$#) {
		($dir,$_) = ('.', $topdir);
	    }
	    $name = $topdir;
	    chdir $dir && namazu::wanted($name,$rarray);
	}
	chdir $cwd;
    }
}


# ファイル名を数字を考慮してソートする
# このコードはは古川@ヤマハさんに頂きました
sub fncmp {
    my ($x, $y) = ($a, $b);
    # ファイル名のソートを数値も考慮して行なう
    # 普通にやると、1, 10, 2, 3, ... の順になってしまう。
    # ちゃんとやる方法もあるが、面倒なので、
    # ケタ数を適当に制限して安易に実装。
    # ファイル名に 8 ケタより長い数字が無ければ大丈夫。

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
    if( $var::Opt{HtaccessExclude} && 
       ( -f ".htaccess" ) && html::parse_htaccess() ){
	printf ("%s is exclude because of .htaccess\n", cwd());
	return(0);
    }
    ##


    # Get the list of files in the current directory.

    opendir(DIR,'.') || (warn "Can't open $dir: $!\n", return);
    my @filenames = grep(!/^\.\.?$/, readdir(DIR));
    closedir(DIR);

    # ファイル名を数字を考慮してソートする
    # 新しい順/古い順による疑似ソートを実現するためです
    @filenames = sort fncmp @filenames;

    if ($nlink == 2 && !$dont_use_nlink) {  # This dir has no subdirectories.
	for (@filenames) {
	    $name = "$dir/$_";
	    $nlink = 0;
	    namazu::wanted($name,$rarray);
	}
    }
    else {                    # This dir has subdirectories.
	$subcount = $nlink - 2;
	for (@filenames) {
	    $nlink = $prune = 0;
	    $name = "$dir/$_";
	    namazu::wanted($name,$rarray);
	    if ($subcount > 0 || $dont_use_nlink) {    # Seen all the subdirs?

		# Get link count and check for directoriness.
		## Modified by S.Takabayashi lstat() -> stat()
		($dev,$ino,$mode,$nlink) = stat($_) unless $nlink;
		
		if (-d _) { 
		    # It really is a directory, so do it recursively.
		    ## and Symbolic Linked dir, also do it.
		    $tmp = '..';
		    if (-l $_) {
			($dev,$ino,$mode,$nlink) = lstat($_);
			if ($SymLinks{"$dev $ino"}) {
			    print "Looped symbolic link detected [$name], skipped.\n";
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

# 対象ディレクトリから処理の対象となるファイルを抽出
sub findfiles ($\@) {
    my ($rarray) = @_;
    find($rarray, cwd());
}

1;
