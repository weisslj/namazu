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
mknmz v$var::VERSION -  全文検索システム Namazu のインデックス作成プログラム
$var::COPYRIGHT

使い方: mknmz [options] <target(s)>

  対象ファイル:
    -a, --all                すべてのファイルを対象とする
    -e, --robots-txt         ロボットよけされているファイルを除外する
    -A, --htaccess           .htaccess で制限されたファイルを除外する
    -F, --target-list=file   インデックス対象のファイルのリストを読み込む
        --allow=regex        対象ファイル名の正規表現を指定する
        --deny=regex         除外するファイル名の正規表現を指定する
        --exclude=regex      除外するパス名の正規表現を指定する
    -r, --replace=code       URIを置換するためのコードを指定する

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
    -P, --no-heading-summary  フレーズ検索用のインデックスを作成しない
    -Y, --no-delete           削除された文書の検出を行わない
    -Z, --no-update           文書の更新/削除を反映しない

  その他:
    -s, --checkpoint        チェックポイント機構を作動させる
    -f, --config=file       設定ファイルを指定する
    -I, --include=file      カスタマイズ用ファイルを読み込む
    -O, --output-dir=dir    インデックスの出力先を指定する
    -T, --template-dir=dir  NMZ.{head,foot,body}.* のディレクトリを指定する
    -L, --lang=lang         言語を設定する ('en' or 'ja')
    -v, --version           ヴァージョンを表示する
    -q, --quiet             インデックス処理の最中にメッセージを表示しない
    -V, --verbose           口やかましいモード
        --debug             デバッグモード
        --help              このヘルプを表示する
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
