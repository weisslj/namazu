/*
 * 
 * messages.c -
 * 
 * $Id: messages.c,v 1.4 1999-05-30 10:37:20 satoru Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi  All rights reserved.
 * This is free software with ABSOLUTELY NO WARRANTY.
 * 
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
 * 02111-1307, USA
 * 
 * This file must be encoded in EUC-JP encoding.
 * 
 */

#include <stdio.h>
#include <string.h>
#include "namazu.h"

uchar *MSG_MIME_HEADER;

#ifdef LANGUAGE
uchar Lang[] = LANGUAGE;
#else
uchar Lang[] = (uchar *)"en";
#endif

/* information about Namazu */
uchar *COPYRIGHT = (uchar *)
"Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.";

uchar *MSG_USAGE, *MSG_TOO_LONG_KEY, *MSG_TOO_MANY_KEYITEM,
*MSG_RESULT_HEADER, *MSG_NO_HIT, *MSG_HIT_1, *MSG_HIT_2,
*MSG_TOO_MUCH_HIT, *MSG_TOO_MUCH_MATCH, *MSG_INDEXDIR_ERROR,
*MSG_REFERENCE_HEADER, *MSG_INVALID_DB_NAME, *MSG_INVALID_QUERY,
*MSG_CANNOT_OPEN_INDEX, *MSG_CANNOT_OPEN_REGEX_INDEX,
*MSG_CANNOT_OPEN_PHRASE_INDEX, *MSG_CANNOT_OPEN_FIELD_INDEX,
*MSG_QUERY_STRING_TOO_LONG;


/*
  beggening '	' means this string contains HTML tag.
  It's treated with special care as Namazu's HTML message.
 */

void initialize_message(void)
{
    if (is_lang_ja(Lang)) {
#if	defined(_WIN32) || defined(__EMX__)
        MSG_MIME_HEADER = (uchar *)"Content-type: text/html; charset=ISO-2022-JP\n\n";
#else
        MSG_MIME_HEADER = (uchar *)"Content-type: text/html; charset=ISO-2022-JP\r\n\r\n";
#endif

      MSG_USAGE = (uchar *)"%s\n\
全文検索システム Namazu の検索プログラム v%s\n\n\
usage: namazu [options] <query> [index dir(s)] \n\
   -n, --max=num           一度に表示する件数\n\
   -w, --whence=num        表示するリストの先頭番号\n\
   -s, --short             短いフォーマットで出力\n\
   -S, --very-short        もっと短いフォーマット (リスト表示) で出力\n\
   -h, --html              HTML で出力する\n\
   -l, --late              新しい順にソートする\n\
   -e, --early             古い順にソートする\n\
   -a, --all               検索結果をすべて表示する\n\
   -c, --hit-count         ヒット数のみを表示する\n\
   -r, --no-references     参考ヒット数を表示しない\n\
   -H, --page              先の検索結果へのリンクを表示する (ほぼ無意味) \n\
   -F, --form              <FORM> ... </FORM> の部分を強制的に表示する\n\
   -R, --no-replace-url    URL の置き換えを行わない\n\
   -U, --no-encode-url     URL encode の復元を行わない\n\
   -o, --output-file=file  指定したファイルに検索結果を出力する\n\
   -f, --config=file       namazu.conf を指定する\n\
   -C, --show-config       コンフィギュレーション内容を表示する\n\
   -q, --quiet             検索結果以外のメッセージを表示しない\n\
   -L, --lang=lang         メッセージの言語を設定する ja または en\n\
   -v, --version           ヴァージョンを表示する\n\
   -0, --help              help を表示する (この表示)\n";

        /* output messages (Japanese message should be outputed by
           euctojisput function */
        MSG_TOO_LONG_KEY = (uchar *)
            "	<H2>エラー!</H2>\n<P>検索式が長すぎます。</P>\n";
        MSG_TOO_MANY_KEYITEM = (uchar *)
            "	<H2>エラー!</H2>\n<P>検索式の項目が多すぎます。</P>\n";
        MSG_QUERY_STRING_TOO_LONG = (uchar *)"CGIのクエリーが長すぎます";
        MSG_INVALID_QUERY = (uchar *)
            "	<H2>エラー!</H2>\n<P>検索式が不正です。</P>\n";
        MSG_RESULT_HEADER = (uchar *)"	<H2>検索結果</H2>\n";
        MSG_NO_HIT = (uchar *)"	<P>検索式にマッチする文書はありませんでした。</P>\n";
        MSG_HIT_1 = (uchar *)"	<P><STRONG>検索式にマッチする ";
        MSG_HIT_2 = (uchar *)"	 個の文書が見つかりました。</STRONG></P>\n\n";
        MSG_TOO_MUCH_HIT = (uchar *)" (ヒット数が多すぎるので無視しました)";
        MSG_TOO_MUCH_MATCH = (uchar *)" (マッチする単語が多すぎるので無視しました)";
        MSG_CANNOT_OPEN_INDEX = (uchar *)" (インデックスが開けませんでした)\n";
        MSG_CANNOT_OPEN_REGEX_INDEX = (uchar *)" (正規表現用インデックスが開けませんでした)";
        MSG_CANNOT_OPEN_FIELD_INDEX = (uchar *)" (フィールド検索用インデックスが開けませんでした)";
        MSG_CANNOT_OPEN_PHRASE_INDEX = (uchar *)" (フレーズ検索用インデックスが開けませんでした)";
        MSG_INDEXDIR_ERROR = (uchar *)"INDEXDIR の設定を確認してください\n";
        MSG_REFERENCE_HEADER = (uchar *)"	<STRONG>参考ヒット数:</STRONG> ";
        MSG_INVALID_DB_NAME = (uchar *)"不正な dbname の指定です";
    } else {
#if	defined(_WIN32) || defined(__EMX__)
        MSG_MIME_HEADER = (uchar *)"Content-type: text/html\n\n";
#else
        MSG_MIME_HEADER = (uchar *)"Content-type: text/html\r\n\r\n";
#endif

        MSG_USAGE = (uchar *)"%s\n\
  Search Program of Namazu v%s\n\n\
  usage: namazu [options] <query> [index dir(s)] \n\
     -n (num)  : set number of documents shown at once.\n\
     -w (num)  : set first number of documents shown.\n\
     -s        : output by short format.\n\
     -S        : output by more short format (simple listing).\n\
     -v        : print this help and exit.\n\
     -f (file) : set pathname of namazu.conf.\n\
     -h        : output by HTML format.\n\
     -l        : sort documents in reverse order.\n\
     -e        : sort documents in normal order.\n\
     -a        : output all documents.\n\
     -c        : output only hit conunts\n\
     -r        : do not display reference hit counts.\n\
     -q        : do not display extra messages except search results\n\
     -o (file) : set output file name.\n\
     -C        : print current configuration.\n\
     -H        : output further result link (nearly meaningless) .\n\
     -F        : force <FORM> ... </FORM> region to output.\n\
     -R        : do not replace URL string.\n\
     -U        : do not decode URL encode when plain text output.\n\
     -L (lang) : set output language (ja or en)\n";

        MSG_TOO_LONG_KEY = (uchar *)
            "	<H2>Error!</H2>\n<P>Too long query.</P>\n";
        MSG_TOO_MANY_KEYITEM = (uchar *)
            "	<H2>Error!</H2>\n<P>Too many queries.</P>\n";
        MSG_QUERY_STRING_TOO_LONG = (uchar *)"Too long CGI query length";
        MSG_INVALID_QUERY = (uchar *)
            "	<H2>Error!</H2>\n<P>Invalid query.</P>\n";
        MSG_RESULT_HEADER = (uchar *)"	<H2>Results:</H2>\n";
        MSG_NO_HIT = (uchar *)"	<P>No match.</P>\n";
        MSG_HIT_1 = (uchar *)"	<P><STRONG> Total ";
        MSG_HIT_2 = (uchar *)"	 documents match your query.</STRONG></P>\n\n";
        MSG_TOO_MUCH_HIT = (uchar *)" (Too many pages. Ignored.)";
        MSG_TOO_MUCH_MATCH = (uchar *)" (Too many words. Ignored.)";
        MSG_CANNOT_OPEN_INDEX = (uchar *)" (cannot open index)\n";
        MSG_CANNOT_OPEN_FIELD_INDEX = (uchar *)" (cannot open field index)";
        MSG_CANNOT_OPEN_REGEX_INDEX = (uchar *)" (cannot open regex index)";
        MSG_CANNOT_OPEN_PHRASE_INDEX = (uchar *)" (cannot open phrase index)";
        MSG_INDEXDIR_ERROR = (uchar *)
            "To Administrator:\nCheck the definition of INDEXDIR.\n";
        MSG_REFERENCE_HEADER = (uchar *)"Word count: ";
        MSG_INVALID_DB_NAME = (uchar *)"Invalid dbname.";
    }
    strcpy(HEADERFILE, "NMZ.head.");
    strcpy(FOOTERFILE, "NMZ.foot.");
    strcpy(BODYMSGFILE, "NMZ.body.");
    strcat(HEADERFILE, Lang);
    strcat(FOOTERFILE, Lang);
    strcat(BODYMSGFILE, Lang);
}



