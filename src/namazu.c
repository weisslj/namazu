/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.64 1999-12-09 09:10:55 satoru Exp $
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
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <ctype.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>
#include <signal.h>

#if defined (WIN32) && !defined (__CYGWIN32__)
/* It's not Unix, really.  See?  Capital letters.  */
#include <windows.h>
#define alarm(sec)	SetTimer(NULL,1,((sec)*1000),NULL)
#define	SIGALRM	14	/* alarm clock */
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "getopt.h"
#include "util.h"
#include "codeconv.h"
#include "usage.h"
#include "rcfile.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "idxname.h"
#include "i18n.h"
#include "message.h"
#include "var.h"
#include "result.h"
#include "system.h"


/*
 *
 * Private functions
 *
 */

static int ck_atoi ( char const *str, int *out );
static int stdio2file ( char * fname );
static int parse_options ( int argc, char **argv );
static enum nmz_stat namazu_core ( char * query, char *subquery, char *argv0 );
static void make_fullpathname_msg ( void );
static void suicide ( int signum );

/* Imported from GNU grep-2.3 [1999-11-08] by satoru-t */
/* Convert STR to a positive integer, storing the result in *OUT.
   If STR is not a valid integer, return -1 (otherwise 0). */
static int
ck_atoi (str, out)
     char const *str;
     int *out;
{
  char const *p;
  for (p = str; *p; p++)
    if (*p < '0' || *p > '9')
      return -1;

  *out = atoi (optarg);
  return 0;
}

/* redirect stdio to specified file */
static int stdio2file(char * fname)
{
/*   int fd;
 *   if (-1 == (fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 00600))) {
 *	set_dyingmsg("stdio2file(cannot open)");
 *	return 1;
 *    }
 *    close(STDOUT);
 *    dup(fd);
 *    close(STDERR);
 *    dup(fd);
 *    close(fd);
 */
    if (freopen(fname, "wb", stdout) == NULL) {
	set_dyingmsg("stdio2file(cannot open)");
	return 1;
    }
    return 0;
}

/*
 * Command line options.
 */
static const char *short_options = "01:2345acCedf:FhHlL:n:o:qrRsSUvw:";
static struct option long_options[] = {
    {"help",             no_argument,       NULL, '0'},
    {"result",           required_argument, NULL, '1'},
    {"late",             no_argument,       NULL, '2'},
    {"early",            no_argument,       NULL, '3'},
    {"sort",             required_argument, NULL, '4'},
    {"ascending",        no_argument,       NULL, '5'},
    {"all",              no_argument,       NULL, 'a'},
    {"count",            no_argument,       NULL, 'c'},
    {"show-config",      no_argument,       NULL, 'C'},
    {"debug",            no_argument,       NULL, 'd'},
    {"config",           required_argument, NULL, 'f'},
    {"form",             no_argument,       NULL, 'F'},
    {"html",             no_argument,       NULL, 'h'},
    {"page",             no_argument,       NULL, 'H'},
    {"list",             no_argument,       NULL, 'l'},
    {"lang",             required_argument, NULL, 'L'},
    {"max",              required_argument, NULL, 'n'},
    {"output",           required_argument, NULL, 'o'},
    {"quiet",            no_argument,       NULL, 'q'},
    {"no-references",    no_argument,       NULL, 'r'},
    {"no-replace",       no_argument,       NULL, 'R'},
    {"short",            no_argument,       NULL, 's'},
    {"no-encode-uri",    no_argument,       NULL, 'U'},
    {"version",          no_argument,       NULL, 'v'},
    {"whence",           required_argument, NULL, 'w'},
    {NULL, 0, NULL, 0}
};

/* parse command line options */
static int parse_options(int argc, char **argv)
{
    int tmp;

    for (;;) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case '0':
	    show_usage();
	    exit(EXIT_SUCCESS);
	    break;
	case '1':
	    set_template(optarg);
	    break;
	case '2':
	    set_sortmethod(SORT_BY_DATE);
	    set_sortorder(DESCENDING);
	    break;	  
	case '3':	  
	    set_sortmethod(SORT_BY_DATE);
	    set_sortorder(ASCENDING);
	    break;
	case '4':  /* --sort */
	{
	    if (strcasecmp(optarg, "score") == 0) {
		set_sortmethod(SORT_BY_SCORE);
	    } else if (strcasecmp(optarg, "date") == 0) {
		set_sortmethod(SORT_BY_DATE);
	    } else if (strprefixcasecmp(optarg, "field:") == 0) {
		set_sortmethod(SORT_BY_FIELD);
		set_sort_field(optarg + strlen("field:"));
	    }
	}
	break;
	case '5':  /* --ascending */
	    set_sortorder(ASCENDING);
	    break;
	case 'f':
	    set_namazurc(optarg);
	    break;
	case 'n':
	    if (ck_atoi(optarg, &tmp)) {
		nmz_die("%s: invalid argument for -n, --max", optarg);
	    }
	    set_maxresult(tmp);
	    break;
	case 'w':
	    if (ck_atoi(optarg, &tmp)) {
		nmz_die("%s: invalid argument for -w, --whence", optarg);
	    }
	    set_listwhence(tmp);
	    break;
	case 'd':
	    set_debugmode(1);
	    break;
	case 's':
	    set_template("short");
	    break;
	case 'l':
	case 'S':  /* 'S' for backward compatibility */
	    set_listmode(1);
	    break;
	case 'q':
	    set_quietmode(1);
	    break;
	case 'c':
	    set_countmode(1);
	    break;
	case 'h':
	    set_htmlmode(1);
	    break;
	case 'H':
	    set_pageindex(1);
	    break;
	case 'F':
	    set_formprint(1);
	    break;
	case 'a':
	    set_allresult(1);
	    break;
	case 'R':
	    set_urireplace(0);
	    break;
	case 'r':
	    set_refprint(0);
	    break;
	case 'U':
	    set_uridecode(1);
	    break;
	case 'v':
	    show_version();
	    exit(EXIT_SUCCESS);
	    break;
	case 'C':
	    if (load_rcfile(argv[0]) != SUCCESS)
		nmz_die_with_msg();
	    show_rcfile();
	    exit(EXIT_SUCCESS);
	    break;
	case 'L':
	    set_lang(optarg);
	    break;
	case 'o':
	    if (stdio2file(optarg))
		nmz_die_with_msg();
	    break;
	}
    } 

    return optind;
}

/* namazu core routine */
static enum nmz_stat namazu_core(char * query, char *subquery, char *argv0)
{
    char query_with_subquery[BUFSIZE * 2];
    NmzResult hlist;

    /* make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    codeconv_query(query);
    codeconv_query(subquery);
    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* if query is null, show NMZ.head,body,foot and exit with error */
    if (*query == '\0') {
	print_default_page();
	free_idxnames();
	free_aliases();
	free_replaces();
	return FAILURE;
    }

    nmz_debug_printf(" -n: %d\n", get_maxresult());
    nmz_debug_printf(" -w: %d\n", get_listwhence());
    nmz_debug_printf("query: [%s]\n", query);

    /* search */
    hlist = nmz_search(query_with_subquery);

    if (hlist.stat == ERR_FATAL) {
	nmz_die_with_msg();
    }

    /* result printing */
    if (print_result(hlist, query, subquery) != SUCCESS) {
	return FAILURE;
    }

    free_hlist(hlist);
    free_idxnames();
    free_aliases();
    free_replaces();

    return SUCCESS;
}

static void make_fullpathname_msg(void)
{
    char *base;
    
    if (Idx.num == 1) {
        base = Idx.names[0];
    } else {
        base = DEFAULT_INDEX;
    }
    
    nmz_pathcat(base, NMZ.head);
    nmz_pathcat(base, NMZ.foot);
    nmz_pathcat(base, NMZ.body);
    nmz_pathcat(base, NMZ.lock);
    nmz_pathcat(base, NMZ.tips);
}

static void suicide (int signum)
{
    nmz_die("processing time exceeds a limit: %d", SUICIDE_TIME);
}

int main(int argc, char **argv)
{
    int i = 0;
    int ret;
    char query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    set_lang("");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    Idx.num = 0;

    if (argc == 1) { /* if no argument, assume this session as CGI */
	if (load_rcfile(argv[0]) != SUCCESS)
	    nmz_die_with_msg();
	set_cgimode(1);
	set_htmlmode(1);
    } else {
	set_htmlmode(0);	 /* do not display result in HTML format */
	set_uridecode(1);	 /* decode a URI */
	set_pageindex(0);	 /* do not diplay page index */
	set_formprint(0);	 /* do not print "<form> ... </form>" */

	i = parse_options(argc, argv); 
	if (load_rcfile(argv[0]) != SUCCESS)
	    nmz_die_with_msg();

	if (i == argc) {
	    show_mini_usage();
	    exit(EXIT_FAILURE);
	}
	if (strlen(argv[i]) > QUERY_MAX) {
	    html_print(_(MSG_TOO_LONG_QUERY));
	    return 1;
	}
        strcpy(query, argv[i++]);
        if (i < argc) {
	    int curidx = getidxnum();
            for (curidx = 0; i < argc && curidx < INDEX_MAX; i++) {
	        if (add_index(argv[i]) != SUCCESS) {
		    nmz_die("invalid idxname: %s", argv[i]);
		}
            }
        } 
        if (getidxnum() == 0) {
	    if (add_index(DEFAULT_INDEX) != SUCCESS) {
		nmz_die("invalid idxname: %s", argv[i]);
	    }
	}
    }
    if (is_cgimode()) {
	init_cgi(query, subquery);
    }

    uniq_idxnames();
    if (expand_idxname_aliases() != SUCCESS)
        nmz_die_with_msg();
    if (complete_idxnames() != SUCCESS)
        nmz_die_with_msg();

    if (is_debugmode()) {
        for (i = 0; i < Idx.num; i++) {
            nmz_debug_printf("Idx.names[%d]: %s\n", i, Idx.names[i]);
        }
    }

    if (is_cgimode()) {
	/* set a suicide timer */
	signal(SIGALRM, suicide);
	alarm(SUICIDE_TIME);
    }

    ret = namazu_core(query, subquery, argv[0]);
    if (ret != SUCCESS)
        nmz_die_with_msg();
    return ret;
}
