# -*- CPerl -*-
# $Id: usage.pl,v 1.2 1999-08-28 01:29:38 satoru Exp $
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

package usage;

##
## Mini usage
##
$USAGE_MINI = <<EOFusage;
Usage: mknmz [options] [prefix] <target(s)>
Try `mknmz --help' for more options.\n"
EOFusage

##
## Japanese usage (long)
##
$USAGE_JA = <<EOFusage;
mknmz v$VERSION -  $BA4J88!:w%7%9%F%`(B Namazu $B$N%$%s%G%C%/%9:n@.%W%m%0%i%`(B
$COPYRIGHT

$B;H$$J}(B: mknmz [options] [prefix] <target(s)>

  $BBP>]%U%!%$%k(B:
    -a, --all                $B$9$Y$F$N%U%!%$%k$rBP>]$H$9$k(B
    -e, --robots-txt         $B%m%\%C%H$h$1$5$l$F$$$k%U%!%$%k$r=|30$9$k(B
    -A, --htaccess           .htaccess $B$G@)8B$5$l$?%U%!%$%k$r=|30$9$k(B
    -F, --target-list=file   $B%$%s%G%C%/%9BP>]$N%U%!%$%k$N%j%9%H$rFI$_9~$`(B
    -t, --allow-regex=regex  $BBP>]%U%!%$%k$N@55,I=8=$r;XDj$9$k(B

  $B7ABVAG2r@O(B:
    -c, --use-chasen        $BF|K\8l$NC18l$N$o$+$A=q$-$K(B ChaSen $B$rMQ$$$k(B
    -k, --use-kakasi        $BF|K\8l$NC18l$N$o$+$A=q$-$K(B KAKASI $B$rMQ$$$k(B
    -m, --use-chasen-morph  $BL>;l$N$_$rCj=P$9$k(B

  $B%U%#%k%?@)8f(B:
    -r, --man          man $B$N%U%!%$%k$r=hM}$9$k(B
    -u, --uuencode     uuencode $B$H(B BinHex $B$NItJ,$rL5;k$9$k(B
    -h, --rfc822       Mail/News $B$N%X%C%@ItJ,$r$=$l$J$j$K=hM}$9$k(B
    -M, --no-mhonarc   MHonArc $B$G:n@.$5$l$?(B HTML $B$N=hM}$r9T$o$J$$(B

  $BJ8;zNs=hM}(B:
    -E, --no-edge-symbol  $BC18l$NN>C<$N5-9f$O:o=|$9$k(B
    -G, --no-okurigana    $BAw$j2>L>$r:o=|$9$k(B
    -H, --no-hiragana     $BJ?2>L>$N$_$NC18l$OEPO?$7$J$$(B
    -K, --no-symbol       $B5-9f$r$9$Y$F:o=|$9$k(B
    -L, --no-line-adjust  $B9TF,!&9TKv$ND4@0=hM}$r9T$o$J$$(B

  $BMWLs(B:
    -U, --no-encode-uri       URI$B$N(Bencode$B$r9T$o$J$$(B
    -x, --no-heading-summary  HTML $B$N%X%G%#%s%0$K$h$kMWLs:n@.$r9T$o$J$$(B


  $B%$%s%G%C%/%9:n@.(B:
    -P, --no-heading-summary  $B%U%l!<%:8!:wMQ$N%$%s%G%C%/%9$r:n@.$7$J$$(B
    -R, --no-regex-index      $B@55,I=8=8!:wMQ$N%$%s%G%C%/%9$r:n@.$7$J$$(B
    -W, --no-date-index       $BF|IU$K$h$k%=!<%HMQ$N%$%s%G%C%/%9:n@.$7$J$$(B
    -X, --no-field-index      $B%U%#!<%k%I8!:wMQ$N%$%s%G%C%/%9$r:n@.$7$J$$(B
    -Y, --no-delete           $B:o=|$5$l$?J8=q$N8!=P$r9T$o$J$$(B
    -Z, --no-update           $BJ8=q$N99?7(B/$B:o=|$rH?1G$7$J$$(B

  $B$=$NB>(B:
    -s, --checkpoint        $B%A%'%C%/%]%$%s%H5!9=$r:nF0$5$;$k(B
    -q, --quiet             $B%$%s%G%C%/%9=hM}$N:GCf$K%a%C%;!<%8$rI=<($7$J$$(B
    -I, --include=file      $B%+%9%?%^%$%:MQ%U%!%$%k$rFI$_9~$`(B
    -O, --output-dir=dir    $B%$%s%G%C%/%9$N=PNO@h$r;XDj$9$k(B
    -T, --template-dir=dir  NMZ.{head,foot,body}.* $B$N%G%#%l%/%H%j$r;XDj$9$k(B
    -l, --lang=lang         $B8@8l$r@_Dj$9$k(B ('en' or 'ja')
    -v, --version           $B%t%!!<%8%g%s$rI=<($9$k(B
    -0, --help              $B$3$N%X%k%W$rI=<($9$k(B
EOFusage

##
## English usage (long)
##
$USAGE_EN = <<EOFusage;
  mknmz.pl v$VERSION -  indexer of Namazu
  $COPYRIGHT

   Usage: mknmz [options] [prefix] <target(s)>
      -a: target all files
      -c: use ChaSen as Japanese processor
      -e: exclude files which has robot exclusion
      -h: treat header part of Mail/News well
      -k: use KAKASI as Japanese processor
      -m: use ChaSen as Japanese processor with morphological processing
      -q: suppress status messages during execution
      -r: treat man files
      -u: decode uuencoded part and discard BinHex part
      -x: do not make summary with structure of HTML's headings
      -s: turn on the checkpoint mechanism (self exec periodically)
      -D: do not insert headers such as 'Date:' to summary (default: off)
      -E: delete symbols on edge of word (default: off)
      -G: delete Okurigana in word (default: off)
      -H: ignore words consist of Hiragana only (default: off)
      -K: delete all symbols (default: off)
      -L: do not adjust beginning and end of line (default: off)
      -M: do not do special processing for MHonArc (default: off)
      -P: do not make the index for phrase search (default: off)
      -R: do not make the index for regex search (default: off)
      -U: do not encode URI (default: off)
      -W: do not make the index for sort by date (default: off)
      -X: do not make the index for field search (default: off)
      -Y: do not detect deleted documents (default: off)
      -Z: do not detect update and deleted documents (default: off)
      -A: exclude files restricted by .htaccess
      -l (lang): specify the language ('en' or 'ja', default:en)
      -I (file): include user defined file in advance of index processing
      -F (file): load a file which contains list of target files
      -O (dir) : specify a directory to output the index
      -T (dir) : specify a directory where NMZ.{head,foot,body}.* are
      -t (regex): specify a regex for target files

EOFusage

1;
