#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "namazu.h"
#include "codeconv.h"
#include "output.h"
#include "util.h"
#include "field.h"
#include "search.h"
#include "result.h"
#include "em.h"
#include "i18n.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

static void fputs_without_html_tag(uchar* , FILE*);
static void emprint(uchar*, int);
static void print_word_hit_count (uchar *key, int hitnum);

/* print s to stdout with processing for emphasizing and entity encoding  */
static void emprint(uchar *s, int entity_encode)
{
    int i;
    for (i = 0; *s && i < BUFSIZE; s++) {
	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    uchar *p;
	    int n = 0;
	    uchar buf[BUFSIZE];

	    strncpy(buf, s, 3);
	    n += 3;
	    s += 3;
	    p = strstr(s, "\033(");
	    if (p == NULL) {   /* non-terminating jis x 0208 */
		fputs(s, stdout);
		return; 
	    }
	    if (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H') {
		int len = p - s + 3;
		strncpy(buf + n, s, len);
		n += len;
		buf[n] = '\0';
		fputs(buf, stdout);
		s += len;
	    } else {  /* unknown charset designation */
		fputs(s, stdout);
		return;
	    }
	}
	if (*s == EM_START_MARK) {
	    fputs(EM_START_TAG, stdout);
	    continue;
	} else if (*s == EM_END_MARK) {
	    fputs(EM_END_TAG, stdout);
	    continue;
	} 
	if (entity_encode) {
	    /* < > & " are converted to entities like &lt; */
	    if (*s == '<') {
		fputs("&lt;", stdout);
	    } else if (*s == '>') {
		fputs("&gt;", stdout);
	    } else if (*s == '&') {
		fputs("&amp;", stdout);
	    } else if (*s == '"') {
		fputs("&quot;", stdout);
	    } else {
		fputc(*s, stdout);
	    }
	} else {
	    fputc(*s, stdout);
	}
    }
}

/* output string without HTML elements */
static void fputs_without_html_tag(uchar * s, FILE *fp)
{
    int f, i;
    uchar buf[BUFSIZE];

    for (f = 0, i = 0; *s && i < BUFSIZE; s++) {

	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    uchar *p;

	    strncpy(buf + i, s, 3);
	    i += 3;
	    s += 3;
	    p = strstr(s, "\033(");
	    if (p == NULL) {   /* non-terminating jis x 0208 */
		strcpy(buf + i, s);
		return; 
	    }
	    if (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H') {
		int len = p - s + 3;
		strncpy(buf + i, s, len);
		i += len;
		s += len;
	    } else {  /* unknown charset designation */
		strcpy(buf + i, s);
		return;
	    }
	}

	if (strncasecmp(s, "<br>", 4) == 0&& *(s + 4) != '\n') {
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
	if (f == 0) {
	    if (strncmp(s, "&lt;", 4) == 0) {
		buf[i++] = '<';
		s += 3;
	    } else if (strncmp(s, "&gt;", 4) == 0) {
		buf[i++] = '>';
		s += 3;
	    } else if (strncmp(s, "&amp;", 5) == 0) {
		buf[i++] = '&';
		s += 4;
	    } else {
		buf[i++] = *s;
	    }
	}
    }
    buf[i] = '\0';
    fputs(buf, fp);
}



/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/*
 * for pageindex
 */
void put_query(uchar * qs, int w)
{
    int foo = 0;
    while (*qs) {
	if (strprefixcmp(qs, "whence=") == 0) {
	    foo = 1;
	    printf("whence=%d", w);
	    for (qs += strlen("whence="); isdigit(*qs); qs++);
	} else {
	    fputc(*(qs++), stdout);
	}
    }
    if (foo == 0) {
	printf("&whence=%d", w);
    }
}


/* displayin page index */
void put_page_index(int n)
{
    int i;
    char *qs; /* QUERY_STRING */
    char *sn; /* SCRIPT_NAME  */

    qs = safe_getenv("QUERY_STRING");
    sn = safe_getenv("SCRIPT_NAME");

    html_print(_("	<strong>Page:</strong> "));

    for (i = 0; i < PAGE_MAX; i++) {
	if (i * HListMax >= n)
	    break;
	if (HtmlOutput) {
	    if (i * HListMax != HListWhence) {
		print("<a href=\"");
		fputs(sn, stdout);
		fputc('?', stdout);
		put_query(qs, i * HListMax);
		print("\">");
	    } else {
		print("<strong>");
	    }
	}
	printf("[%d]", i + 1);
	if (HtmlOutput) {
	    if (i * HListMax != HListWhence) {
		print("</A> ");
	    } else
		print("</strong> ");
	}
	if (AllList)
	    break;
    }
}

/* output current range */
void put_current_range(int listmax)
{
    if (HtmlOutput) {
	print("<strong>");
    }
    printf(_("Current List: %d"), HListWhence + 1);

    print(" - ");
    if (!AllList && ((HListWhence + HListMax) < listmax)) {
	printf("%d", HListWhence + HListMax);
    } else {
	printf("%d", listmax);
    }
    if (HtmlOutput) {
	print("</strong><br>\n");
    } else {
	fputc('\n', stdout);
    }
}

/* fputs Namazu version, it works with considereation of mode */
void html_print(uchar *str)
{
    uchar buf[BUFSIZE * 16];
    int is_nmz_html = 0;

    if ((int)*str == (int)'\t') { /* Namazu's HTML message */
        is_nmz_html = 1;
    }

    strcpy(buf, str + is_nmz_html);
    if (HtmlOutput) {
        /* if str is Namazu's HTML message, it will be printed with emprint,
           if not, it will be printed with entity conversion */
        emprint(buf, ! is_nmz_html);
    } else {
        /* if str is Namazu's HTML message, it will be printed without 
           HTML tag, if not, it will be printed as is */
        if (is_nmz_html) {
            fputs_without_html_tag(buf, stdout);
        } 
	else {
            fputs(buf, stdout);
        }
    }
}
/*
 * display message file such as NMZ.tips or NMZ.body.
 */

void print_msgfile(uchar *fname) {
    uchar tmp_fname[BUFSIZE];

    strcpy(tmp_fname, fname);
    choose_msgfile(tmp_fname);
    cat(tmp_fname);
}

/* display the hlist */
void print_hlist(HLIST hlist)
{
    int i;
    uchar *templates[INDEX_MAX];
    /* prepare large memory for replace_field() and conv_ext() */
    uchar result[BUFSIZE * 128];

    if (hlist.n <= 0 || HListMax == 0) {
	return;
    }

    /* set NULL to all templates[] */
    for (i = 0; i < Idx.num; i++) {
	templates[i] = NULL;
    }

    for (i = HListWhence; i < hlist.n; i++) {
	int counter;

	counter = i + 1;

	if (!AllList && (i >= HListWhence + HListMax))
	    break;
	if (ListFormat) {
	    get_field_data(hlist.d[i].idxid, hlist.d[i].docid, "uri", result);
	} else {
	    if (templates[hlist.d[i].idxid] == NULL) {  /* not loaded */
		uchar fname[BUFSIZE];

		make_fullpathname_result(hlist.d[i].idxid);
		strcpy(fname, NMZ.result);
		strcat(fname, ".");
		strcat(fname, Template);  /* usually "normal" */
		templates[hlist.d[i].idxid] = readfile(fname);
	    }
	    compose_result(hlist.d[i], counter, 
			   templates[hlist.d[i].idxid],  result);
	}
	conv_ext(result);
	html_print(result);
	print("\n");
    }

    /* free all templates[] */
    for (i = 0; i < Idx.num; i++) {
	if (templates[i] != NULL) {
	    free(templates[i]);
	}
    }
}

void print_hit_count ()
{
    int i;
    for (i = 0; i < Idx.num; i ++) {
        PHRASERES *pr = Idx.pr[i];
	if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
	    if (Idx.num > 1) {
	        if (HtmlOutput) {
		    printf("<li><strong>%s</strong>: ",
			   Idx.names[i] + strlen(DEFAULT_INDEX) + 1);
		} else {
		    printf("(%s)", Idx.names[i]);
		}
	    }
	}
        if (!HitCountOnly && !ListFormat && 
	    !NoReference && !Quiet && Idx.mode[i] == PHRASE_MODE) 
	{
	    print(" { ");
	}
        while (pr != NULL) {
	    print_word_hit_count(pr->word, pr->hitnum);
	    pr = pr->next;
	}
	if (!HitCountOnly && !ListFormat && 
	    !NoReference && !Quiet && Idx.mode[i] == PHRASE_MODE) 
	{
	    printf(" :: %d } ", Idx.phrasehit[i]);
	}
	if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
	    if (Idx.num > 1 && Query.tab[1]) {
	        printf(_(" [ TOTAL: %d ]"), Idx.total[i]);
	    }
	    print("\n");
	}
    }
}

static void print_word_hit_count (uchar *key, int hitnum)
{
    if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
	uchar tmp_key[BUFSIZE];
	strcpy(tmp_key, key);
	conv_ext(tmp_key);

        print(" [ ");
        print(tmp_key);
        if (hitnum > 0) {
            printf(": %d", hitnum);
        } else { 
            uchar *msg = (uchar *)"";
            if (hitnum == 0) {
                msg = (uchar *)": 0 ";
            } else if (hitnum == ERR_TOO_MUCH_MATCH) {
                msg = _(" (Too many words matched. Ignored.)");
            } else if (hitnum == ERR_TOO_MUCH_HIT) {
                msg = _(" (Too many pages hit. Ignored.)");
            } else if (hitnum == ERR_REGEX_SEARCH_FAILED) {
                msg = _(" (cannot open regex index)");
            } else if (hitnum == ERR_PHRASE_SEARCH_FAILED) {
                msg = _(" (cannot open phrase index)");
            } else if (hitnum == ERR_CANNOT_OPEN_INDEX) {
		msg = _(" (cannot open this index)\n");
            } else if (hitnum == ERR_NO_PERMISSION) {
		msg = _("(You don\'t have a permission to access the index)");
	    }
            print(_(msg));
        }
        print(" ] ");
    }
}

void print_result1(void)
{
    html_print(_("	<h2>Results:</h2>\n"));

    if (HtmlOutput) {
	fputs("<p>\n", stdout);
    } else {
	fputc('\n', stdout);
    }
    print(_("References: "));
    if (Idx.num > 1 && HtmlOutput) {
	fputs("</p>\n", stdout);
    }
}

void print_hitnum(int n)
{
    html_print(_("	<p><strong> Total "));
    if (HtmlOutput) {
        printf("<!-- HIT -->%d<!-- HIT -->", n);
    }
    else {
        printf("%d", n);
    }
    html_print(_("	 documents matching your query.</strong></p>\n\n"));
}

void print_listing(HLIST hlist)
{
    if (HtmlOutput) {
        print("<dl>\n");
    }
    
    print_hlist(hlist);
    
    if (HtmlOutput) {
        print("</dl>\n");
    }
}

void print_range(HLIST hlist)
{
    if (HtmlOutput)
        print("<p>\n");
    put_current_range(hlist.n);
    if (!HidePageIndex) {
        put_page_index(hlist.n);
    }
    if (HtmlOutput) {
        print("</p>\n");
    } else {
        print("\n");
    }
}

