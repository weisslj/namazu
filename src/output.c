#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "namazu.h"

/*
 * for pageindex
 */
void put_query(uchar * qs, int w)
{
    int foo = 0;
    while (*qs) {
	if (!strncmp(qs, "whence=", 7)) {
	    foo = 1;
	    printf("whence=%d", w);
	    for (qs += 7; isdigit(*qs); qs++);
	} else
	    fputc(*(qs++), stdout);
    }
    if (foo == 0) {
	printf("&whence=%d", w);
    }
}


/* displayin page index */
void put_page_index(int n)
{
    int i;

    if (HtmlOutput)
	printf("<STRONG>Page:</STRONG> ");
    else
	printf("Page: ");
    for (i = 0; i < PAGE_MAX; i++) {
	if (i * HListMax >= n)
	    break;
	if (HtmlOutput) {
	    if (i * HListMax != HListWhence) {
		printf("<A HREF=\"");
		fputs(ScriptName, stdout);
		fputc('?', stdout);
		put_query(QueryString, i * HListMax);
		printf("\">");
	    } else {
		printf("<STRONG>");
	    }
	}
	printf("[%d]", i + 1);
	if (HtmlOutput) {
	    if (i * HListMax != HListWhence) {
		printf("</A> ");
	    } else
		printf("</STRONG> ");
	}
	if (AllList)
	    break;
    }
}

/* output current range */
void put_current_range(int listmax)
{
    if (HtmlOutput)
	printf("<STRONG>Current List: %d", HListWhence + 1);
    else
	printf("Current List: %d", HListWhence + 1);

    printf(" - ");
    if (!AllList && ((HListWhence + HListMax) < listmax))
	printf("%d", HListWhence + HListMax);
    else
	printf("%d", listmax);
    if (HtmlOutput)
	printf("</STRONG><BR>\n");
    else
	fputc('\n', stdout);
}

void fputs_with_codeconv(uchar * s, FILE *fp)
{
    uchar buf[BUFSIZE];

    strcpy(buf, s);
#if	defined(_WIN32) || defined(__EMX__)
    if (is_lang_ja(Lang)) { /* Lang == ja*/
        euctosjis(buf);
    }    
#endif
    fputs(buf, fp);
}

/* output string without HTML elements
 * if system is Win32 or OS/2, output in Shift_JIS encoding */
void fputs_without_html_tag(uchar * s, FILE *fp)
{
    int f, i;
    uchar buf[BUFSIZE];

    for (f = 0, i = 0; *s && i < BUFSIZE; s++) {
	if (!strncmp(s, "<br>", 4) && *(s + 4) != '\n') {
	    buf[i++] = '\n';
	    s += 3;
	    continue;
	}
	if (*s == '<') {
	    f = 1;
	    continue;
	}
	if (*s == '>') {
	    f = 0;
	    continue;
	}
	if (!f) {
	    if (!strncmp(s, "&lt;", 4)) {
		buf[i++] = '<';
		s += 3;
	    } else if (!strncmp(s, "&gt;", 4)) {
		buf[i++] = '>';
		s += 3;
	    } else if (!strncmp(s, "&amp;", 5)) {
		buf[i++] = '&';
		s += 4;
	    } else {
		buf[i++] = *s;
	    }
	}
    }
    buf[i] = '\0';
    fputs_with_codeconv(buf, fp);
}


void euctojisput(uchar *s, FILE *fp, int entity_encode, int ishtml)
{
    int c, c2, state = 0, non_ja;

    if (! is_lang_ja(Lang)) { /* Lang != ja */
        non_ja = 1;
    } else {
        non_ja = 0;
    }
    if (!(c = (int) *(s++)))
	return;
    while (1) {
	if (non_ja || c < 0x80) {
	    if (state) {
		set1byte();
		state = 0;
	    }
	    if (c == EMPHASIZING_START) {
		if (ishtml) {
		    printf("<strong class=\"keyword\">");
		} else {
		    printf(" **");
		}
		if (!(c = (int) *(s++))) {
		    return;
		}
		continue;
	    } else if (c == EMPHASIZING_END) {
		if (ishtml) {
		    printf("</strong>");
		} else {
		    printf("** ");
		}
		if (!(c = (int) *(s++))) {
		    return;
		}
		continue;
	    } 
	    if (entity_encode) {
		/* < > & " are converted to entities like &lt; */
		if (c == '<')
		    printf("&lt;");
		else if (c == '>')
		    printf("&gt;");
		else if (c == '&')
		    printf("&amp;");
		else if (c == '"')
		    printf("&quot;");
		else
		    fputc(c, fp);
	    } else {
		fputc(c, fp);
	    }
	} else if (iseuc(c)) {
	    if (!(c2 = (int) *(s++))) {
		fputc(c, fp);
		return;
	    }
	    if (!state) {
		set2byte();
		state = 1;
	    }
	    if (iseuc(c2)) {
		fputc(c & 0x7f, fp);
		fputc(c2 & 0x7f, fp);
	    } else {
		fputc(c, fp);
		set1byte();
		state = 0;
		fputc(c2, fp);
	    }
	} else {
	    if (state) {
		set1byte();
		state = 0;
	    }
	    fputc(c, fp);
	}
	if (!(c = (int) *(s++))) {
	    if (state)
		set1byte();
	    return;
	}
    }
}

/* fputs Namazu version, it works with considereation of mode */
void fputx(uchar *str, FILE *fp)
{
    uchar buf[BUFSIZE * 16];
    int ishtml = 0;

    if ((int)*str == (int)'\t') {
        ishtml = 1;
    }

    strcpy(buf, str + ishtml);
    if (HtmlOutput) {
        /* if string is Namazu's HTML message, it will be printed as is,
           if not, it will be printed with entity conversion */
        euctojisput(buf, fp, ! ishtml, ishtml);
    } else {
        /* if string is Namazu's HTML message, it will be printed without 
           HTML tag, if not, it will be printed as is */
        if (ishtml) {
            fputs_without_html_tag(buf, fp);
        } else {
            fputs_with_codeconv(buf, fp);
        }
    }
}
