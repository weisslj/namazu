# -*- Perl -*-
# $Id: usage.pl,v 1.9 1999-08-31 04:01:05 satoru Exp $
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
Usage: mknmz [options] <target(s)>
Try `mknmz --help' for more options.
EOFusage

##
## Japanese usage (long)
##
$USAGE_JA = <<EOFusage;
mknmz v$var::VERSION -  ��ʸ���������ƥ� Namazu �Υ���ǥå��������ץ����
$var::COPYRIGHT

�Ȥ���: mknmz [options] <target(s)>

  �оݥե�����:
    -a, --all                ���٤ƤΥե�������оݤȤ���
    -e, --robots-txt         ��ܥåȤ褱����Ƥ���ե�������������
    -A, --htaccess           .htaccess �����¤��줿�ե�������������
    -F, --target-list=file   ����ǥå����оݤΥե�����Υꥹ�Ȥ��ɤ߹���
        --allow=regex        �оݥե�����̾������ɽ������ꤹ��
        --deny=regex         ��������ե�����̾������ɽ������ꤹ��
        --exclude=regex      ��������ѥ�̾������ɽ������ꤹ��
    -r, --replace=code       URI���ִ����뤿��Υ����ɤ���ꤹ��

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
    -P, --no-heading-summary  �ե졼�������ѤΥ���ǥå�����������ʤ�
    -Y, --no-delete           ������줿ʸ��θ��Ф�Ԥ�ʤ�
    -Z, --no-update           ʸ��ι���/�����ȿ�Ǥ��ʤ�

  ����¾:
    -s, --checkpoint        �����å��ݥ���ȵ������ư������
    -f, --config=file       ����ե��������ꤹ��
    -I, --include=file      �������ޥ����ѥե�������ɤ߹���
    -O, --output-dir=dir    ����ǥå����ν��������ꤹ��
    -T, --template-dir=dir  NMZ.{head,foot,body}.* �Υǥ��쥯�ȥ����ꤹ��
    -L, --lang=lang         ��������ꤹ�� ('en' or 'ja')
    -v, --version           ������������ɽ������
    -q, --quiet             ����ǥå��������κ���˥�å�������ɽ�����ʤ�
    -V, --verbose           ���䤫�ޤ����⡼��
        --debug             �ǥХå��⡼��
        --help              ���Υإ�פ�ɽ������
EOFusage

##
## English usage (long)
##
$USAGE_EN = <<EOFusage;
  mknmz.pl v$var::VERSION -  indexer of Namazu
  $var::COPYRIGHT

   Usage: mknmz [options] <target(s)>
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
