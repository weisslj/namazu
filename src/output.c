/*
 * $Id: output.c,v 1.77 2000-09-14 08:49:28 knok Exp $
 * 
 * Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.
 * Copyright (C) 2000 Namazu Project All rights reserved.
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
 * 
 */
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#ifdef HAVE_SUPPORT_H
#  include "support.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "codeconv.h"
#include "output.h"
#include "util.h"
#include "field.h"
#include "search.h"
#include "result.h"
#include "i18n.h"
#include "message.h"
#include "form.h"
#include "var.h"
#include "idxname.h"
#include "query.h"
#include "system.h"

static int htmlmode    = 0;
static int cgimode     = 0;
static int quietmode   = 0;

static int countmode   = 0;   /* like grep -c */
static int listmode    = 0;   /* like grep -l */
		      
static int allresult   = 0;   /* print all results */
static int pageindex   = 0;   /* print "Page: [1][2][3][4][5][6][7][8]" */
static int formprint   = 0;   /* print "<form> ... </form>" at cgimode */
static int refprint    = 0;   /* print "References:  [ foo: 123 ]" */
		      
static int maxresult   = 20;  /* max number of search results */
static int listwhence  = 0;   /* number which beginning of search results */
		      
static char template_suffix[BUFSIZE] = "normal"; /* suffix of NMZ.result.* */

/*
 * They are used for emphasizing keywords in html results. 
 */ 
static char emphasis_start_tag[BUFSIZE] = "<strong class=\"keyword\">";
static char emphasis_end_tag[BUFSIZE]   = "</strong>";

/*
 *
 * Private functions
 *
 */

static void emprint ( char *str, int entity_encode );
static void fputs_without_html_tag ( const char *str, FILE *fp );
static char *load_nmz_result(const char *basedir);
static void print_hitnum_each ( struct nmz_hitnumlist *hn );
static int is_allresult ( void );
static int is_pageindex ( void );
static int is_countmode ( void );
static int is_listmode ( void );
static int is_quietmode ( void );
static int is_refprint ( void );
static int is_cgimode ( void );
static enum nmz_stat print_hlist ( NmzResult hlist );
static enum nmz_stat print_listing ( NmzResult hlist );
static void print_query ( const char * qs, int w );
static void print_page_index ( int n );
static void print_current_range ( int listmax );
static void print_hitnum_all_idx ( void );
static void print_hitnum ( int n );
static void print_msgfile ( const char *fname );
static void print_range ( NmzResult hlist );
static void print_errmsg(int errid);

/*
 * Print s to stdout with processing for emphasizing keyword 
 * and entity encoding.
 */
static void 
emprint(char *s, int entity_encode)
{
    int i;
    for (i = 0; *s && i < BUFSIZE; s++) {
	if (*s == EM_START_MARK) {
	    fputs(emphasis_start_tag, stdout);
	    continue;
	} else if (*s == EM_END_MARK) {
	    fputs(emphasis_end_tag, stdout);
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

/*
 * Output string without HTML elements
 */
static void 
fputs_without_html_tag(const char *str, FILE *fp)
{
    int f, i;
    char buf[BUFSIZE];

    for (f = 0, i = 0; *str && i < BUFSIZE; str++) {

	/* Iso-2022-jp handling */
	if ((strncmp(str, "\033$", 2) == 0)
	    && (*(str + 2) == 'B' || *(str + 2) == '@')) 
	{
	    char *p;

	    strncpy(buf + i, str, 3);
	    i += 3;
	    str += 3;
	    p = strstr(str, "\033(");
	    if (p == NULL) {   /* non-terminating jis x 0208 */
		strcpy(buf + i, str);
		return; 
	    }
	    if (*(p + 2) == 'J' || *(p + 2) == 'B' || *(p + 2) == 'H') {
		int len = p - str + 3;
		strncpy(buf + i, str, len);
		i += len;
		str += len;
	    } else {  /* unknown charset designation */
		strcpy(buf + i, str);
		return;
	    }
	}

	if (strncasecmp(str, "<br>", 4) == 0&& *(str + 4) != '\n') {
	    buf[i++] = '\n';
	    str += 3;
	    continue;
	}
	if (*str == '<') {
	    f = 1;
	    continue;
	}
	if (*str == '>') {
	    f = 0;
	    continue;
	}
	if (f == 0) {
	    if (nmz_strprefixcmp(str, "&lt;") == 0) {
		buf[i++] = '<';
		str += 3;
	    } else if (nmz_strprefixcmp(str, "&gt;") == 0) {
		buf[i++] = '>';
		str += 3;
	    } else if (nmz_strprefixcmp(str, "&amp;") == 0) {
		buf[i++] = '&';
		str += 4;
	    } else if (nmz_strprefixcmp(str, "&quot;") == 0) {
		buf[i++] = '"';
		str += 5;
	    } else {
		buf[i++] = *str;
	    }
	}
    }
    buf[i] = '\0';
    fputs(buf, fp);
}


static void 
print_hitnum_each (struct nmz_hitnumlist *hn)
{
    struct nmz_hitnumlist *hnptr = hn;

    if (hn->phrase != NULL) { /* it has phrases */
	hnptr = hn->phrase;
        if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode())
	{
	    printf(" { ");
	}
    }

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	do {
	    char *converted = nmz_codeconv_external(hnptr->word);
	    if (converted == NULL) {
		die("print_hitnum_each");
	    }

	    printf(" [ ");
	    html_print(converted);
	    free(converted);

	    if (hnptr->stat == SUCCESS) {
		printf(": %d", hnptr->hitnum);
	    } else {
		char *errmsg = nmz_strerror(hnptr->stat);
		printf(" (%s) ", errmsg);
	    }
	    printf(" ] ");
	    hnptr = hnptr->next;
	} while (hn->phrase && hnptr != NULL);
    }

    if (is_refprint() && !is_countmode() && !is_listmode() && 
	!is_quietmode() &&  hn->phrase != NULL) /* it has phrases */
    {
	printf(" :: %d } ", hn->hitnum);
    }
}

static int 
is_allresult(void)
{
    return allresult;
}

static int 
is_pageindex(void)
{
    return pageindex;
}

static int 
is_countmode(void)
{
    return countmode;
}

static int 
is_listmode(void)
{
    return listmode;
}

static int 
is_quietmode(void)
{
    return quietmode;
}

static int 
is_refprint(void)
{
    return refprint;
}

static int 
is_cgimode(void)
{
    return cgimode;

}

static char*
load_nmz_result(const char *basedir)
{
    char fname[BUFSIZE], lang_suffix[BUFSIZE], *buf;

    nmz_pathcat(basedir, NMZ.result);
    strcpy(fname, NMZ.result);
    strcat(fname, ".");
    strcat(fname, get_templatesuffix());  /* usually "normal" */

    if (nmz_choose_msgfile_suffix(fname, lang_suffix) != SUCCESS) {
	nmz_warn_printf("%s: %s", fname, strerror(errno));
	return NULL;
    } 
    strcat(fname, lang_suffix);

    /* buf is allocated in nmz_readfile. */
    buf = nmz_readfile(fname); 
    if (buf == NULL) { /* failed */
	return NULL;
    }

    return buf;
}

/*
 * Display one searched document according to NMZ.result.* file.
 * e.g.,
 * 
 *   <dt>1. <strong><a href="/foo/gunzip.1.gz">GZIP(1)</a></strong> (score: xx)
 *   <dd><strong>Author</strong>: <em>(unknown)</em>
 *   <dd><strong>Date</strong>: <em>Thu, 09 Apr 1998 12:59:59</em>
 *   <dd>gzip, gunzip, zcat - compress or expand files:: Gzip
 *   reduces the size of the named files using Lempel-Ziv
 *   coding (LZ77). Whenever possible, each file is replaced
 *   by one with the extension .gz, while 
 *   <dd><a href="/foo/gunzip.1.gz">/foo/gunzip.1.gz</a> (5,410 bytes)<br><br>
 */
static enum nmz_stat
print_hlist(NmzResult hlist)
{
    int i;
    char *the_template = NULL;    /* User-specified  template. */
    char *template_caches[INDEX_MAX];   /* For caching each NMZ.result */

    if (hlist.num <= 0 || get_maxresult() == 0) {
	return SUCCESS; /* No document searched but success. */
    }

    /* 
     * Clear pointers for caching NMZ.result.
     */
    for (i = 0; i < nmz_get_idxnum(); i++) {
	template_caches[i] = NULL;
    }

    /* 
     * Check whether user-specified templatedir is set or not. 
     */
    {
	char templdir[BUFSIZE];
	strcpy(templdir, get_templatedir());
	if (*templdir != '\0') { /* user-specified one is set. */
	    the_template = load_nmz_result(templdir);
	    if (the_template == NULL) {
		return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
	    }
	}
    }

    for (i = get_listwhence(); i < hlist.num; i++) {
	/* 
	 * Prepare large memory for replace_field() for safety.
	 * FIXME: static memory allocation may cause buffer overflow.
	 */
	char result[BUFSIZE * 128];
	int counter;
	char *template = the_template;

	counter = i + 1;

	if (!is_allresult() && (i >= get_listwhence() + get_maxresult()))
	    break;

	if (is_listmode()) {
	    template = "${uri}";
	} else {
	    int idxid = hlist.data[i].idxid;
	    /*
	     * If user-specified templatedir is not set. and 
	     */
	    if (template == NULL) {
		/* 
		 * If NMZ.result is not cached, load NMZ.result and cache it in
		 * template_caches[].
		 */ 
		if (template_caches[idxid] == NULL) {
		    char *basedir = nmz_get_idxname(idxid);
		    template_caches[idxid] = load_nmz_result(basedir);
		    if (template_caches[idxid] == NULL) {
			return ERR_CANNOT_OPEN_RESULT_FORMAT_FILE;
		    }
		} 
		template = template_caches[idxid];
	    }
	}

	compose_result(hlist.data[i], counter, template,  result);
	{
	    char *converted = nmz_codeconv_external(result);
	    if (converted == NULL) {
		die(nmz_get_dyingmsg());
	    }
	    html_print(converted);
	    free(converted);
	    printf("\n");
	}
    }

    /* Free user-specified template. */
    if (the_template != NULL) {
	free(the_template);
    }

    /* Free all template_caches[] */
    for (i = 0; i < nmz_get_idxnum(); i++) {
	if (template_caches[i] != NULL) {
	    free(template_caches[i]);
	}
    }

    return SUCCESS;
}

static enum nmz_stat 
print_listing(NmzResult hlist)
{
    enum nmz_stat ret;

    if (is_htmlmode()) {
        printf("<dl>\n");
    }

    ret = print_hlist(hlist);
    if (ret != SUCCESS) {
	return ret;
    }
    
    if (is_htmlmode()) {
        printf("</dl>\n");
    }
    return SUCCESS;
}

/*
 * For page_index().
 */
static void 
print_query(const char * qs, int w)
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

/*
 * Display page index.
 */
static void 
print_page_index(int n)
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
		printf("<a href=\"");
		fputs(sn, stdout);
		fputc('?', stdout);
		print_query(qs, i * max);
		printf("\">");
	    } else {
		printf("<strong>");
	    }
	}
	printf("[%d]", i + 1);
	if (is_htmlmode()) {
	    if (i * max != whence) {
		printf("</A> ");
	    } else
		printf("</strong> ");
	}
	if (is_allresult()) {
	    break;
	}
    }
}

/*
 * Output current range
 */
static void 
print_current_range(int listmax)
{
    int max, whence;

    max    = get_maxresult();
    whence = get_listwhence();

    if (is_htmlmode()) {
	printf("<strong>");
    }
    printf(_("Current List: %d"), whence + 1);

    printf(" - ");
    if (!is_allresult() && ((whence + max) < listmax)) {
	printf("%d", whence + max);
    } else {
	printf("%d", listmax);
    }
    if (is_htmlmode()) {
	printf("</strong><br>\n");
    } else {
	fputc('\n', stdout);
    }
}

static void 
print_hitnum_all_idx(void)
{
    int idxid;
    for (idxid = 0; idxid < nmz_get_idxnum(); idxid ++) {
        struct nmz_hitnumlist *hnlist = nmz_get_idx_hitnumlist(idxid);

	if (is_refprint() && !is_countmode() && 
	    !is_listmode() && !is_quietmode()) 
	{
	    if (nmz_get_idxnum() > 1) {
	        if (is_htmlmode()) {
		    char *idxname = nmz_get_idxname(idxid);
		    if (is_cgimode()) {
			/* For hiding a full pathname of an index */
			idxname =  nmz_get_idxname(idxid) 
			    + strlen(nmz_get_defaultidx()) + 1;
		    }
		    printf("<li><strong>%s</strong>: ", idxname);
		} else {
		    printf("(%s)", nmz_get_idxname(idxid));
		}
	    }
	}

        while (hnlist != NULL) {
	    print_hitnum_each(hnlist);
	    hnlist = hnlist->next;
	}

	if (is_refprint() && !is_countmode() && !is_listmode() && 
	    !is_quietmode()) {
	    if (nmz_get_idxnum() > 1 && nmz_get_querytokennum() > 1) {
	        printf(_(" [ TOTAL: %d ]"), nmz_get_idx_totalhitnum(idxid));
	    }
	    printf("\n");
	}
    }
}

static void 
print_hitnum(int n)
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

/* 
 * Output contents of a message file such as NMZ.tips or NMZ.body. 
 */
static void 
print_msgfile(const char *fname) {
    char suffix[BUFSIZE], tmpfname[BUFSIZE];

    if (nmz_choose_msgfile_suffix(fname, suffix) == SUCCESS) {
	char *buf;

	strcpy(tmpfname, fname);
	strcat(tmpfname, suffix);

	buf = nmz_readfile(tmpfname); /* buf is allocated in nmz_readfile. */
	if (buf == NULL) {
	    die(nmz_get_dyingmsg());
	}
	/* In case of suffix isn't equal to lang, we need code conversion */
	if (strcmp(suffix, nmz_get_lang()) != 0) {
            /* new is allocated in nmz_codeconv_external. */
	    char *new = nmz_codeconv_external(buf); 
	    free(buf);  /* Then we should free buf's memory */
	    buf = new;
	}

	fputs(buf, stdout);
	free(buf);
    } else {
	nmz_warn_printf("%s: %s", fname, strerror(errno));
    }
}

static void 
print_range(NmzResult hlist)
{
    if (is_htmlmode())
        printf("<p>\n");
    print_current_range(hlist.num);
    if (is_pageindex()) {
        print_page_index(hlist.num);
    }
    if (is_htmlmode()) {
        printf("</p>\n");
    } else {
        printf("\n");
    }
}

/*
 * Print the error message specified by errid while outputing the results.
 */
static void
print_errmsg(int errid)
{
    char *errmsg = nmz_strerror(errid);
    char buf[BUFSIZE];
    sprintf(buf, _("	<h2>Error!</h2>\n<p>%s</p>\n"), errmsg);
    html_print(buf);
}

/*
 *
 * Public functions
 *
 */
enum nmz_stat 
print_result(NmzResult hlist, const char *query, const char *subquery)
{

    if (is_htmlmode() && is_cgimode()) {
	printf(MSG_MIME_HEADER);
    }

    if (is_htmlmode()) {
	print_headfoot(NMZ.head, query, subquery);
    }

    if (hlist.stat != SUCCESS) {
	print_errmsg(hlist.stat);
	return FAILURE;
    }

    /* Result1:  <h2>Results:</h2>, References:  */
    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) 
    {
	html_print(_("	<h2>Results:</h2>\n"));

	if (is_htmlmode()) {
	    fputs("<p>\n", stdout);
	} else {
	    fputc('\n', stdout);
	}
	printf(_("References: "));
	if (nmz_get_idxnum() > 1 && is_htmlmode()) {
	    fputs("</p>\n", stdout);
	}

        if (nmz_get_idxnum() > 1) {
            printf("\n");
            if (is_htmlmode())
                printf("<ul>\n");
        }
    }

    print_hitnum_all_idx(); /* print hit numbers for all index. */

    if (is_refprint() && !is_countmode() && 
	!is_listmode() && !is_quietmode()) {
        if (nmz_get_idxnum() > 1 && is_htmlmode()) {
            printf("</ul>\n");
        }
	if (nmz_get_idxnum() == 1 && is_htmlmode()) {
	    printf("\n</p>\n");
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
	    enum nmz_stat ret = print_listing(hlist);

	    if (ret != SUCCESS) { /* summary listing */
		print_errmsg(ret);
		return ret;
	    }
	}
        if (!is_countmode() && !is_listmode() && !is_quietmode()) {
            print_range(hlist);
        }
    } else {
        if (is_countmode()) {
            printf("0\n");
        } else if (!is_listmode() && !is_quietmode()) {
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

/* 
 * Print default page: NMZ.{head,body,foot} 
 */
void 
print_default_page (void) {
    if (is_htmlmode()) {
	printf(MSG_MIME_HEADER);
	print_headfoot(NMZ.head, "", "");
	print_msgfile(NMZ.body);
	print_headfoot(NMZ.foot, "", "");
    }
}

void 
set_emphasis_tags(const char *start_tag, const char *end_tag)
{
    strcpy(emphasis_start_tag, start_tag);
    strcpy(emphasis_end_tag,   end_tag);
}

char *
get_emphasis_tag_start(void)
{
    return emphasis_start_tag;
}

char *
get_emphasis_tag_end(void)
{
    return emphasis_end_tag;
}

void 
set_htmlmode(int mode)
{
    htmlmode = mode;
}

int 
is_htmlmode(void)
{
    return htmlmode;
}

void 
set_cgimode(int mode)
{
    cgimode = mode;
}

void 
set_quietmode(int mode)
{
    quietmode = mode;
}

void 
set_countmode(int mode)
{
    countmode = mode;
}

void 
set_listmode(int mode)
{
    listmode = mode;
}

void 
set_allresult(int mode)
{
    allresult = mode;
}

void 
set_pageindex(int mode)
{
    pageindex = mode;
}

void 
set_formprint(int mode)
{
    formprint = mode;
}

int 
is_formprint(void)
{
    return formprint;
}

void 
set_refprint(int mode)
{
    refprint = mode;
}

void 
set_maxresult(int num)
{
    maxresult = num;
}

int 
get_maxresult(void)
{
    return maxresult;
}

void 
set_listwhence(int num)
{
    listwhence = num;
}

int 
get_listwhence(void)
{
    return listwhence;
}


void 
set_templatesuffix(const char *tmpl)
{
    strcpy(template_suffix, tmpl);
}

char *
get_templatesuffix(void)
{
    return template_suffix;
}


/* 
 * Namazu version fputs, it works with considereation of html mode.
 */
void 
html_print(const char *str)
{
    char buf[BUFSIZE * 16];
    int is_nmz_html = 0;

    if ((int)*str == (int)'\t') { /* Namazu's HTML message */
        is_nmz_html = 1;
    }

    strcpy(buf, str + is_nmz_html);
    if (is_htmlmode()) {
        /* 
	 * If str is Namazu's HTML message, it will be printed with emprint.
	 * If not, it will be printed with entity conversion 
	 */
        emprint(buf, ! is_nmz_html);
    } else {
        /* 
	 * If str is Namazu's HTML message, it will be printed without 
	 * HTML tag, if not, it will be printed as is 
	 */
        if (is_nmz_html) {
            fputs_without_html_tag(buf, stdout);
        } 
	else {
            fputs(buf, stdout);
        }
    }
}


/* 
 * Print the error message and die.
 */
void 
die(const char *fmt, ...)
{
    va_list args;

    fflush(stdout);
    fflush(stderr);

    if (is_cgimode()) {
	printf(MSG_MIME_HEADER);
	printf(_("<h2>Error</h2>\n<p>"));
	va_start(args, fmt);
	vprintf(fmt, args);
	va_end(args);
	printf(_("</p>"));
	if (fmt[strlen(fmt) - 1] != '\n') {
	    printf("\n");
	}
    } else {
	fprintf(stderr, "%s: ", PACKAGE);
	va_start(args, fmt);
	vfprintf(stderr, fmt, args);
	va_end(args);
       if (fmt[strlen(fmt) - 1] != '\n') {
	   fprintf(stderr, "\n");
       }
    }

    exit(EXIT_FAILURE);
}

