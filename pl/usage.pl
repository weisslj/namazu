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
mknmz $var::VERSION, Namazu のインデックス作成プログラム 

使い方: mknmz [options] <target>...

対象ファイル:
  -a, --all                すべてのファイルを対象とする
  -e, --robots-txt         ロボットよけされているファイルを除外する
  -t, --media-type=mtype   対象ファイルの文書形式を指定する
  -h, --mailnews           --media-type='message/rfc822' と同じ
      --mhonarc            --media-type='text/html; x-type=mhonarc' と同じ
  -A, --htaccess           .htaccess で制限されたファイルを除外する
  -F, --target-list=file   インデックス対象のファイルのリストを読み込む
      --allow=regex        対象ファイル名の正規表現を指定する
      --deny=regex         除外するファイル名の正規表現を指定する
      --exclude=regex      除外するパス名の正規表現を指定する
  -M, --meta               HTMLの metaタグをフィールド指定検索に用いる
  -r, --replace=code       URIを置換するためのコードを指定する
      --mtime=int          変更日制限 (int が 負で最近、正で古いものだけ、
                           例。-50 で 50 日以内、50 で 50 日より古いものだけ)

形態素解析:
  -c, --use-chasen        日本語の単語のわかち書きに ChaSen を用いる
  -k, --use-kakasi        日本語の単語のわかち書きに KAKASI を用いる
  -m, --use-chasen-morph  名詞のみを抽出する

文字列処理:
  -E, --no-edge-symbol  単語の両端の記号は削除する
  -G, --no-okurigana    送り仮名を削除する
  -H, --no-hiragana     平仮名のみの単語は登録しない
  -K, --no-symbol       記号をすべて削除する

要約:
  -U, --no-encode-uri       URIのencodeを行わない
  -x, --no-heading-summary  HTML のヘディングによる要約作成を行わない


インデックス作成:
      --update=index        更新するインデックスを指定する
  -Y, --no-delete           削除された文書の検出を行わない
  -Z, --no-update           文書の更新/削除を反映しない

その他:
  -s, --checkpoint        チェックポイント機構を作動させる
  -f, --config=file       設定ファイルを指定する
  -I, --include=file      カスタマイズ用ファイルを読み込む
  -O, --output-dir=dir    インデックスの出力先を指定する
  -T, --template-dir=dir  NMZ.{head,foot,body}.* のディレクトリを指定する
  -L, --lang=lang         言語を設定する ('en' or 'ja')
  -q, --quiet             インデックス処理の最中にメッセージを表示しない
  -v, --version           ヴァージョンを表示する
  -V, --verbose           口やかましいモード
      --debug             デバッグモード
      --help              このヘルプを表示する

バグ報告は <bug-namazu\@namazu.org> へどうぞ
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

