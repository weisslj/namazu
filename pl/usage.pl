# -*- Perl -*-
# $Id: usage.pl,v 1.8 1999-12-19 00:55:12 makoto Exp $
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
require "var.pl";

##
## Mini usage
##
$USAGE_MINI = <<EOFusage;
Usage: mknmz [options] <target>...
Try `mknmz --help' for more information.
EOFusage

##
## Japanese usage (long)
##
$USAGE_JA = <<EOFusage;
mknmz $var::VERSION, Namazu �Υ���ǥå��������ץ���� 

�Ȥ���: mknmz [options] <target>...

�оݥե�����:
  -a, --all                ���٤ƤΥե�������оݤȤ���
  -e, --robots-txt         ��ܥåȤ褱����Ƥ���ե�������������
  -t, --media-type=mtype   �оݥե������ʸ���������ꤹ��
  -h, --mailnews           --media-type='message/rfc822' ��Ʊ��
      --mhonarc            --media-type='text/html; x-type=mhonarc' ��Ʊ��
  -A, --htaccess           .htaccess �����¤��줿�ե�������������
  -F, --target-list=file   ����ǥå����оݤΥե�����Υꥹ�Ȥ��ɤ߹���
      --allow=regex        �оݥե�����̾������ɽ������ꤹ��
      --deny=regex         ��������ե�����̾������ɽ������ꤹ��
      --exclude=regex      ��������ѥ�̾������ɽ������ꤹ��
  -M, --meta               HTML�� meta������ե�����ɻ��긡�����Ѥ���
  -r, --replace=code       URI���ִ����뤿��Υ����ɤ���ꤹ��
      --mtime=int          �ѹ������� (int �� ��ǺǶᡢ���ǸŤ���Τ�����
                           �㡣-50 �� 50 �����⡢50 �� 50 �����Ť���Τ���)

�����ǲ���:
  -c, --use-chasen        ���ܸ��ñ��Τ狼���񤭤� ChaSen ���Ѥ���
  -k, --use-kakasi        ���ܸ��ñ��Τ狼���񤭤� KAKASI ���Ѥ���
  -m, --use-chasen-morph  ̾��Τߤ���Ф���

ʸ�������:
  -E, --no-edge-symbol  ñ���ξü�ε���Ϻ������
  -G, --no-okurigana    ���겾̾��������
  -H, --no-hiragana     ʿ��̾�Τߤ�ñ�����Ͽ���ʤ�
  -K, --no-symbol       ����򤹤٤ƺ������

����:
  -U, --no-encode-uri       URI��encode��Ԥ�ʤ�
  -x, --no-heading-summary  HTML �Υإǥ��󥰤ˤ�����������Ԥ�ʤ�


����ǥå�������:
      --update=index        �������륤��ǥå�������ꤹ��
  -Y, --no-delete           ������줿ʸ��θ��Ф�Ԥ�ʤ�
  -Z, --no-update           ʸ��ι���/�����ȿ�Ǥ��ʤ�

����¾:
  -s, --checkpoint        �����å��ݥ���ȵ������ư������
  -f, --config=file       ����ե��������ꤹ��
  -I, --include=file      �������ޥ����ѥե�������ɤ߹���
  -O, --output-dir=dir    ����ǥå����ν��������ꤹ��
  -T, --template-dir=dir  NMZ.{head,foot,body}.* �Υǥ��쥯�ȥ����ꤹ��
  -L, --lang=lang         ��������ꤹ�� ('en' or 'ja')
  -q, --quiet             ����ǥå��������κ���˥�å�������ɽ�����ʤ�
  -v, --version           ������������ɽ������
  -V, --verbose           ���䤫�ޤ����⡼��
      --debug             �ǥХå��⡼��
      --help              ���Υإ�פ�ɽ������

�Х����� <bug-namazu\@namazu.org> �ؤɤ���
EOFusage

##
## English usage (long)
##
$USAGE_EN = <<EOFusage;
mknmz $var::VERSION, an indexer of Namazu.

Usage: mknmz [options] <target>...

Target files:
  -a, --all                target all files.
  -e, --robots-txt         exclude files which is excluded by robots.txt.
  -t, --media-type=mtype   set a media type for all target files explicitly.
  -h, --mailnews           same as --media-type='message/rfc822'
      --mhonarc            same as --media-type='text/html; x-type=mhonarc'
  -A, --htaccess           exclude files restricted by .htaccess.
  -F, --target-list=file   load a file which contains list of target files.
      --allow=regex        set a regex for file names which will be allowed.
      --deny=regex         set a regex for file names which will be denied.
      --exclude=regex      set a regex for pathnames which will be excluded.
  -M, --meta               handle HTML meta tags for field-specified search.
  -r, --replace=code       set a code for replacing URI.
      --mtime=int          limit by date, ( minus number for recent, i.e.,
                           -50 for recent 50 days, 50 for older than 50.)

Morphological Analysis:
  -c, --use-chasen         use ChaSen for analyzing Japanese.
  -k, --use-kakasi         use KAKASI for analyzing Japanese.
  -m, --use-chasen-morph   use ChaSen for collecting only nouns.

Text Operations:
  -E, --no-edge-symbol     remove symbols on edge of word.
  -G, --no-okurigana       remove Okurigana in word.
  -H, --no-hiragana        ignore words consist of Hiragana only.
  -K, --no-symbol          remove symbols.

Summarization:
  -U, --no-encode-uri      do not encode URI.
  -x, --no-heading-summary do not make summary with HTML's headings.

Index Construction:
      --update=index       set an index for updating.
  -Y, --no-delete          do not detect removed documents.
  -Z, --no-update          do not detect update and deleted documents.

Miscellaneous:
  -s, --checkpoint         turn on the checkpoint mechanism.
  -f, --config=file        set a pathname of a config file.
  -I, --include=file       include your customization file.
  -O, --output-dir=dir     set a directory to output the index.
  -T, --template-dir=dir   set a directory having NMZ.{head,foot,body}.*.
  -L, --lang=lang          set language (ja or en)
  -q, --quiet              suppress status messages during execution.
  -v, --version            show the version of namazu and exit.
  -V, --verbose            be verbose.
      --debug              be debug mode.
      --help               show this help and exit.

Report bugs to <bug-namazu\@namazu.org>.
EOFusage

##
## Version information
##
$VERSION_INFO = <<EOFversion;
mknmz of Namazu $var::VERSION
Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.

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

