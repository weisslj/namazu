# -*- Perl -*-
# $Id: usage.pl,v 1.18 2000-02-16 12:57:45 satoru Exp $
# Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
# Copyright (C) 2000 Namazu Project All rights reserved.
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
require "var.pl";

# dummy function.
sub N_ {
    return $_[0];
}

#
# To deceive xgettext, add fake "\n\" at the end of each line.
# This "\n\" is handled in mknmz's show_usage().
#
$USAGE = N_("mknmz %s, an indexer of Namazu.\n\
\n\
Usage: mknmz [options] <target>...\n\
\n\
Target files:\n\
  -a, --all                target all files.\n\
  -e, --robots-txt         exclude files which is excluded by robots.txt.\n\
  -t, --media-type=MTYPE   set the media type for all target files to MTYPE.\n\
  -h, --mailnews           same as --media-type='message/rfc822'\n\
      --mhonarc            same as --media-type='text/html; x-type=mhonarc'\n\
  -A, --htaccess           exclude files restricted by .htaccess.\n\
  -F, --target-list=FILE   load FILE which contains a list of target files.\n\
      --allow=PATTERN      set PATTERN for file names which will be allowed.\n\
      --deny=PATTERN       set PATTERN for file names which will be denied.\n\
      --exclude=PATTERN    set PATTERN for pathnames which will be excluded.\n\
  -M, --meta               handle HTML meta tags for field-specified search.\n\
  -r, --replace=CODE       set CODE for replacing URI.\n\
  -o, --ole                use OLE document filters. (works on Windows only)\n\
      --mtime=NUM          limit by date, (minus NUM for recent, i.e.,\n\
                           -50 for recent 50 days, 50 for older than 50.)\n\
\n\
Morphological Analysis:\n\
  -c, --use-chasen         use ChaSen for analyzing Japanese.\n\
  -k, --use-kakasi         use KAKASI for analyzing Japanese.\n\
  -m, --use-chasen-noun    use ChaSen for extracting only nouns.\n\
\n\
Text Operations:\n\
  -E, --no-edge-symbol     remove symbols on edge of word.\n\
  -G, --no-okurigana       remove Okurigana in word.\n\
  -H, --no-hiragana        ignore words consist of Hiragana only.\n\
  -K, --no-symbol          remove symbols.\n\
\n\
Summarization:\n\
  -U, --no-encode-uri      do not encode URI.\n\
  -x, --no-heading-summary do not make summary with HTML's headings.\n\
\n\
Index Construction:\n\
      --update=INDEX       set INDEX for updating.\n\
  -Y, --no-delete          do not detect removed documents.\n\
  -Z, --no-update          do not detect update and deleted documents.\n\
\n\
Miscellaneous:\n\
  -s, --checkpoint         turn on the checkpoint mechanism.\n\
  -f, --config=FILE        set the config file to FILE.\n\
  -I, --include=FILE       include your customization FILE.\n\
  -O, --output-dir=DIR     set DIR to output the index.\n\
  -T, --template-dir=DIR   set DIR having NMZ.{head,foot,body}.*.\n\
  -q, --quiet              suppress status messages during execution.\n\
  -v, --version            show the version of namazu and exit.\n\
  -V, --verbose            be verbose.\n\
      --debug              be debug mode.\n\
      --help               show this help and exit.\n\
\n\
Report bugs to <%s>.\n\
");

##
## Version information
##
$VERSION_INFO = <<EOFversion;
mknmz of Namazu $var::VERSION
$var::COPYRIGHT

This is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2, or (at your option)
any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty
of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.
EOFversion

1;

