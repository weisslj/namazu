#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "codeconv.h"
#include "output.h"
#include "util.h"
#include "field.h"
#include "search.h"
#include "result.h"
#include "em.h"
#include "i18n.h"
#include "message.h"
#include "form.h"
#include "var.h"


static int htmlmode    = 0;
static int cgimode     = 0;
static int quietmode   = 0;

static int countmode   = 0;   /* like grep -c */
static int listmode    = 0;   /* like grep -l */
		      
static int allresult   = 0;   /* print all results */
static int pageindex   = 1;   /* print "Page: [1][2][3][4][5][6][7][8]" */
static int formprint   = 1;   /* print "<form> ... </form>" at cgimode */
static int refprint    = 1;   /* print "References:  [ foo: 123 ]" */
		      
static int maxresult = 20;  /* max number of search results */
static int listwhence  = 0;   /* number which beginning of search results */
		      
static char template[BUFSIZE]     = "normal"; /* suffix of NMZ.result.* */

/*
 *
 * Private functions
 *
 */

static void emprint ( char *s, int entity_encode );
static void fputs_without_html_tag ( char * s, FILE *fp );
static void make_fullpathname_result ( int n );
static void print_hitnum_each ( struct nmz_hitnum *hn );
static int is_allresult ( void );
static int is_pageindex ( void );
static int is_countmode ( void );
static int is_listmode ( void );
static int is_quietmode ( void );
static int is_refprint ( void );
static void print_hlist ( NmzResult hlist );
static void print_query ( char * qs, int w );
static void print_page_index ( int n );
static void print_current_range ( int listmax );
static void print_hitnum_all_idx ( void );
static void print_hitnum ( int n );
static void print_listing ( NmzResult hlist );
static void print_msgfile ( char *fname );
static void print_range ( NmzResult hlist );

/* print s to stdout with processing for emphasizing and entity encoding  */
static void emprint(char *s, int entity_encode)
{
    int i;
    for (i = 0; *s && i < BUFSIZE; s++) {
	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    char *p;
	    int n = 0;
	    char buf[BUFSIZE];

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
static void fputs_without_html_tag(char * s, FILE *fp)
{
    int f, i;
    char buf[BUFSIZE];

    for (f = 0, i = 0; *s && i < BUFSIZE; s++) {

	/* iso-2022-jp handling */
	if ((strncmp(s, "\033$", 2) == 0)
	    && (*(s + 2) == 'B' || *(s + 2) == '@')) 
	{
	    char *p;

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


static void make_fullpathname_result(int n)
{
    char *base;

    base = Idx.names[n];
    nmz_pathcat(base, NMZ.result);
}

static void print_hitnum_each (struct nmz_hitnum *hn)
{
    struct nmz_hitnum *hnptr = hn;

    if (hn->phrase != NULL) { /* it has phrases */
	hnptr = hn->phrase;
        if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode())
	{
	    nmz_print(" { ");
	}
    }

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	do {
	    char tmp_word[BUFSIZE];
	    strcpy(tmp_word, hnptr->word);
	    conv_ext(tmp_word);

	    nmz_print(" [ ");
	    nmz_print(tmp_word);
	    if (hnptr->stat == SUCCESS) {
		printf(": %d", hnptr->hitnum);
	    } else {
		char *errmsg = nmz_get_errmsg(hnptr->stat);
		printf(" (%s) ", errmsg);
	    }
	    nmz_print(" ] ");
	    hnptr = hnptr->next;
	} while (hn->phrase && hnptr != NULL);
    }

    if (is_refprint() && !is_countmode() && !is_listmode() && 
	!is_quietmode() &&  hn->phrase != NULL) /* it has phrases */
    {
	printf(" :: %d } ", hn->hitnum);
    }
}

static int is_allresult(void)
{
    return allresult;
}

static int is_pageindex(void)
{
    return pageindex;
}

static int is_countmode(void)
{
    return countmode;
}

static int is_listmode(void)
{
    return listmode;
}

static int is_quietmode(void)
{
    return quietmode;
}

static int is_refprint(void)
{
    return refprint;
}

/* display the hlist */
static void print_hlist(NmzResult hlist)
{
    int i;
    char *templates[INDEX_MAX];
    /* prepare large memory for replace_field() and conv_ext() */
    char result[BUFSIZE * 128];

    if (hlist.num <= 0 || get_maxresult() == 0) {
	return;
    }

    /* set NULL to all templates[] */
    for (i = 0; i < Idx.num; i++) {
	templates[i] = NULL;
    }

    for (i = get_listwhence(); i < hlist.num; i++) {
	int counter;

	counter = i + 1;

	if (!is_allresult() && (i >= get_listwhence() + get_maxresult()))
	    break;

	if (templates[hlist.data[i].idxid] == NULL) {  /* not loaded */
	    if (is_listmode()) {
		templates[hlist.data[i].idxid] = malloc(BUFSIZE);
		if (templates[hlist.data[i].idxid] == NULL) {
		    return;  /* FIXME: error status should be returned */
		}
		strcpy(templates[hlist.data[i].idxid], "${uri}");
	    } else {
		char fname[BUFSIZE];
		make_fullpathname_result(hlist.data[i].idxid);
		strcpy(fname, NMZ.result);
		strcat(fname, ".");
		strcat(fname, get_template());  /* usually "normal" */
		templates[hlist.data[i].idxid] = nmz_readfile(fname);
		if (templates[hlist.data[i].idxid] == NULL) {
		    return;  /* FIXME: error status should be returned */
		}
	    }
	}
	compose_result(hlist.data[i], counter, 
		       templates[hlist.data[i].idxid],  result);
	conv_ext(result);
	html_print(result);
	nmz_print("\n");
    }

    /* free all templates[] */
    for (i = 0; i < Idx.num; i++) {
	if (templates[i] != NULL) {
	    free(templates[i]);
	}
    }
}

/*
 * for pageindex
 */
static void print_query(char * qs, int w)
{
    int foo = 0;
    while (*qs) {
	if (nmz_strprefixcmp(qs, "whence=") == 0) {
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
static void print_page_index(int n)
{
    int i, max, whence;
    char *qs; /* QUERY_STRING */
    char *sn; /* SCRIPT_NAME  */

    qs = nmz_getenv("QUERY_STRING");
    sn = nmz_getenv("SCRIPT_NAME");

    html_print(_("	<strong>Page:</strong> "));

    max    = get_maxresult();
    whence = get_listwhence();
    for (i = 0; i < PAGE_MAX; i++) {
	if (i * max >= n)
	    break;
	if (is_htmlmode()) {
	    if (i * max != whence) {
		nmz_print("<a href=\"");
		fputs(sn, stdout);
		fputc('?', stdout);
		print_query(qs, i * max);
		nmz_print("\">");
	    } else {
		nmz_print("<strong>");
	    }
	}
	printf("[%d]", i + 1);
	if (is_htmlmode()) {
	    if (i * max != whence) {
		nmz_print("</A> ");
	    } else
		nmz_print("</strong> ");
	}
	if (is_allresult()) {
	    break;
	}
    }
}

/* output current range */
static void print_current_range(int listmax)
{
    int max, whence;

    max    = get_maxresult();
    whence = get_listwhence();

    if (is_htmlmode()) {
	nmz_print("<strong>");
    }
    printf(_("Current List: %d"), whence + 1);

    nmz_print(" - ");
    if (!is_allresult() && ((whence + max) < listmax)) {
	printf("%d", whence + max);
    } else {
	printf("%d", listmax);
    }
    if (is_htmlmode()) {
	nmz_print("</strong><br>\n");
    } else {
	fputc('\n', stdout);
    }
}

static void print_hitnum_all_idx(void)
{
    int i;
    for (i = 0; i < Idx.num; i ++) {
        struct nmz_hitnum *pr = Idx.pr[i];

	if (is_refprint() && !is_countmode() && 
	    !is_listmode() && !is_quietmode()) 
	{
	    if (Idx.num > 1) {
	        if (is_htmlmode()) {
		    printf("<li><strong>%s</strong>: ",
			   Idx.names[i] + strlen(DEFAULT_INDEX) + 1);
		} else {
		    printf("(%s)", Idx.names[i]);
		}
	    }
	}

        while (pr != NULL) {
	    print_hitnum_each(pr);
	    pr = pr->next;
	}

	if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode()) {
	    if (Idx.num > 1 && Query.tab[1]) {
	        printf(_(" [ TOTAL: %d ]"), Idx.total[i]);
	    }
	    nmz_print("\n");
	}
    }
}

static void print_hitnum(int n)
{
    html_print(_("	<p><strong> Total "));
    if (is_htmlmode()) {
        printf("<!-- HIT -->%d<!-- HIT -->", n);
    }
    else {
        printf("%d", n);
    }
    html_print(_("	 documents matching your query.</strong></p>\n\n"));
}

static void print_listing(NmzResult hlist)
{
    if (is_htmlmode()) {
        nmz_print("<dl>\n");
    }
    
    print_hlist(hlist);
    
    if (is_htmlmode()) {
        nmz_print("</dl>\n");
    }
}

/* display message file such as NMZ.tips or NMZ.body. */
static void print_msgfile(char *fname) {
    char tmp_fname[BUFSIZE];

    strcpy(tmp_fname, fname);
    choose_msgfile(tmp_fname);
    nmz_cat(tmp_fname);
}

static void print_range(NmzResult hlist)
{
    if (is_htmlmode())
        nmz_print("<p>\n");
    print_current_range(hlist.num);
    if (is_pageindex()) {
        print_page_index(hlist.num);
    }
    if (is_htmlmode()) {
        nmz_print("</p>\n");
    } else {
        nmz_print("\n");
    }
}

/*
 *
 * Public functions
 *
 */

enum nmz_stat print_result(NmzResult hlist, char *query, char *subquery)
{

    if (is_htmlmode() && is_cgimode()) {
	nmz_print(MSG_MIME_HEADER);
    }

    if (is_htmlmode()) {
	print_headfoot(NMZ.head, query, subquery);
    }

    if (hlist.stat != SUCCESS) {
	char *errmsg = nmz_get_errmsg(hlist.stat);
	char buf[BUFSIZE];
	sprintf(buf, _("	<h2>Error!</h2>\n<p>%s</p>\n"), errmsg);

	html_print(buf);
	return FAILURE;
    }

    /* result1:  <h2>Results:</h2>, References:  */
    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	html_print(_("	<h2>Results:</h2>\n"));

	if (is_htmlmode()) {
	    fputs("<p>\n", stdout);
	} else {
	    fputc('\n', stdout);
	}
	nmz_print(_("References: "));
	if (Idx.num > 1 && is_htmlmode()) {
	    fputs("</p>\n", stdout);
	}

        if (Idx.num > 1) {
            nmz_print("\n");
            if (is_htmlmode())
                nmz_print("<ul>\n");
        }
    }

    print_hitnum_all_idx(); /* print hit numbers for all index. */

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) {
        if (Idx.num > 1 && is_htmlmode()) {
            nmz_print("</ul>\n");
        }
	if (Idx.num == 1 && is_htmlmode()) {
	    nmz_print("\n</p>\n");
	} else {
	    fputc('\n', stdout);
	}
    }

    if (hlist.num > 0) {
        if (!is_countmode() && !is_listmode() && !is_quietmode()) {
            print_hitnum(hlist.num);  /* <!-- HIT -->%d<!-- HIT --> */
        }
	if (is_countmode()) {
	    printf("%d\n", hlist.num);
	} else {
	    print_listing(hlist); /* summary listing */
	}
        if (!is_countmode() && !is_listmode() && !is_quietmode()) {
            print_range(hlist);
        }
    } else {
        if (is_countmode()) {
            nmz_print("0\n");
        } else if (!is_listmode()) {
            html_print(_("	<p>No document matching your query.</p>\n"));
	    if (is_htmlmode()) {
		print_msgfile(NMZ.tips);
	    }
        }
    }

    if (is_htmlmode()) {
	print_headfoot(NMZ.foot, query, subquery);
    }

    return SUCCESS;
}

/* Print default page: NMZ.{head,body,foot} */
void print_default_page (void) {
    if (is_htmlmode()) {
	nmz_print(MSG_MIME_HEADER);
	print_headfoot(NMZ.head, "", "");
	print_msgfile(NMZ.body);
	print_headfoot(NMZ.foot, "", "");
    }
}

void set_htmlmode(int mode)
{
    htmlmode = mode;
}

int is_htmlmode(void)
{
    return htmlmode;
}

void set_cgimode(int mode)
{
    cgimode = mode;
}

int is_cgimode(void)
{
    return cgimode;
}

void set_quietmode(int mode)
{
    quietmode = mode;
}

void set_countmode(int mode)
{
    countmode = mode;
}

void set_listmode(int mode)
{
    listmode = mode;
}

void set_allresult(int mode)
{
    allresult = mode;
}

void set_pageindex(int mode)
{
    pageindex = mode;
}

void set_formprint(int mode)
{
    formprint = mode;
}

int is_formprint(void)
{
    return formprint;
}

void set_refprint(int mode)
{
    refprint = mode;
}

void set_maxresult(int num)
{
    maxresult = num;
}

int get_maxresult(void)
{
    return maxresult;
}

void set_listwhence(int num)
{
    listwhence = num;
}

int get_listwhence(void)
{
    return listwhence;
}


void set_template(char *tmpl)
{
    strcpy(template, tmpl);
}

char *get_template(void)
{
    return template;
}


/* fputs Namazu version, it works with considereation of mode */
void html_print(char *str)
{
    char buf[BUFSIZE * 16];
    int is_nmz_html = 0;

    if ((int)*str == (int)'\t') { /* Namazu's HTML message */
        is_nmz_html = 1;
    }

    strcpy(buf, str + is_nmz_html);
    if (is_htmlmode()) {
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


