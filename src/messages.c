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
��ʸ���������ƥ� Namazu �θ����ץ���� v%s\n\n\
usage: namazu [options] <query> [index dir(s)] \n\
   -n, --max=num           ���٤�ɽ��������\n\
   -w, --whence=num        ɽ������ꥹ�Ȥ���Ƭ�ֹ�\n\
   -s, --short             û���ե����ޥåȤǽ���\n\
   -S, --very-short        ��ä�û���ե����ޥå� (�ꥹ��ɽ��) �ǽ���\n\
   -h, --html              HTML �ǽ��Ϥ���\n\
   -l, --late              ��������˥����Ȥ���\n\
   -e, --early             �Ť���˥����Ȥ���\n\
   -a, --all               ������̤򤹤٤�ɽ������\n\
   -c, --hit-count         �ҥåȿ��Τߤ�ɽ������\n\
   -r, --no-references     ���ͥҥåȿ���ɽ�����ʤ�\n\
   -H, --page              ��θ�����̤ؤΥ�󥯤�ɽ������ (�ۤ�̵��̣) \n\
   -F, --form              <FORM> ... </FORM> ����ʬ����Ū��ɽ������\n\
   -R, --no-replace-url    URL ���֤�������Ԥ�ʤ�\n\
   -U, --no-encode-url     URL encode ��������Ԥ�ʤ�\n\
   -o, --output-file=file  ���ꤷ���ե�����˸�����̤���Ϥ���\n\
   -f, --config=file       namazu.conf ����ꤹ��\n\
   -C, --show-config       ����ե�����졼��������Ƥ�ɽ������\n\
   -q, --quiet             ������̰ʳ��Υ�å�������ɽ�����ʤ�\n\
   -L, --lang=lang         ��å������θ�������ꤹ�� ja �ޤ��� en\n\
   -v, --version           ������������ɽ������\n\
   -0, --help              help ��ɽ������ (����ɽ��)\n";

        /* output messages (Japanese message should be outputed by
           euctojisput function */
        MSG_TOO_LONG_KEY = (uchar *)
            "	<H2>���顼!</H2>\n<P>��������Ĺ�����ޤ���</P>\n";
        MSG_TOO_MANY_KEYITEM = (uchar *)
            "	<H2>���顼!</H2>\n<P>�������ι��ܤ�¿�����ޤ���</P>\n";
        MSG_QUERY_STRING_TOO_LONG = (uchar *)"CGI�Υ����꡼��Ĺ�����ޤ�";
        MSG_INVALID_QUERY = (uchar *)
            "	<H2>���顼!</H2>\n<P>�������������Ǥ���</P>\n";
        MSG_RESULT_HEADER = (uchar *)"	<H2>�������</H2>\n";
        MSG_NO_HIT = (uchar *)"	<P>�������˥ޥå�����ʸ��Ϥ���ޤ���Ǥ�����</P>\n";
        MSG_HIT_1 = (uchar *)"	<P><STRONG>�������˥ޥå����� ";
        MSG_HIT_2 = (uchar *)"	 �Ĥ�ʸ�񤬸��Ĥ���ޤ�����</STRONG></P>\n\n";
        MSG_TOO_MUCH_HIT = (uchar *)" (�ҥåȿ���¿������Τ�̵�뤷�ޤ���)";
        MSG_TOO_MUCH_MATCH = (uchar *)" (�ޥå�����ñ�줬¿������Τ�̵�뤷�ޤ���)";
        MSG_CANNOT_OPEN_INDEX = (uchar *)" (����ǥå����������ޤ���Ǥ���)\n";
        MSG_CANNOT_OPEN_REGEX_INDEX = (uchar *)" (����ɽ���ѥ���ǥå����������ޤ���Ǥ���)";
        MSG_CANNOT_OPEN_FIELD_INDEX = (uchar *)" (�ե�����ɸ����ѥ���ǥå����������ޤ���Ǥ���)";
        MSG_CANNOT_OPEN_PHRASE_INDEX = (uchar *)" (�ե졼�������ѥ���ǥå����������ޤ���Ǥ���)";
        MSG_INDEXDIR_ERROR = (uchar *)"INDEXDIR ��������ǧ���Ƥ�������\n";
        MSG_REFERENCE_HEADER = (uchar *)"	<STRONG>���ͥҥåȿ�:</STRONG> ";
        MSG_INVALID_DB_NAME = (uchar *)"������ dbname �λ���Ǥ�";
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



