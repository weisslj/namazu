/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu.c,v 1.38 1999-11-18 13:42:07 satoru Exp $
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

#include "namazu.h"
#include "libnamazu.h"
#include "getopt.h"
#include "util.h"
#include "codeconv.h"
#include "form.h"
#include "usage.h"
#include "conf.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "idxname.h"
#include "i18n.h"

/************************************************************
 *
 * Private functions
 *
 ************************************************************/

static int stdio2file(char*);
static int parse_options(int, char**);
static int namazu_core(char*, char*, char*);
static void suicide(int);

/* redirect stdio to specified file */
static int stdio2file(char * fname)
{
/*   int fd;
 *   if (-1 == (fd = open(fname, O_CREAT | O_TRUNC | O_WRONLY, 00600))) {
 *	diemsg("stdio2file(cannot open)");
 *	return 1;
 *    }
 *    close(STDOUT);
 *    dup(fd);
 *    close(STDERR);
 *    dup(fd);
 *    close(fd);
 */
    if (freopen(fname, "wb", stdout) == NULL) {
	diemsg("stdio2file(cannot open)");
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
    for (;;) {
        int ch = getopt_long(argc, argv, short_options, long_options, NULL);
        if (ch == EOF) {
            break;
	}
	switch (ch) {
	case '0':
	    show_long_usage();
	    return DIE_NOERROR;
	    break;
	case '1':
	    set_template(optarg);
	    break;
	case '2':
	    set_sortbydate();
	    set_sort_descending();
	    break;	  
	case '3':	  
	    set_sortbydate();
	    set_sort_ascending();
	    break;
	case '4':  /* --sort */
	{
	    if (strcasecmp(optarg, "score") == 0) {
		set_sortbyscore();
	    } else if (strcasecmp(optarg, "date") == 0) {
		set_sortbydate();
	    } else if (strprefixcasecmp(optarg, "field:") == 0) {
		set_sortbyfield();
		set_sort_field(optarg + 6);
	    }
	}
	break;
	case '5':  /* --ascending */
	    set_sort_ascending();
	    break;
	case 'f':
	    set_namazurc(optarg);
	    break;
	case 'n':
	    HListMax = atoi(optarg);
	    break;
	case 'w':
	    HListWhence = atoi(optarg);
	    break;
	case 'd':
	    set_debug();
	    break;
	case 's':
	    set_template("short");
	    break;
	case 'l':
	case 'S':  /* 'S' for backward compatibility */
	    ListFormat = 1;
	    break;
	case 'q':
	    Quiet = 1;
	    break;
	case 'c':
	    HitCountOnly = 1;
	    break;
	case 'h':
	    HtmlOutput = 1;
	    break;
	case 'H':
	    HidePageIndex = 0;
	    break;
	case 'F':
	    ForcePrintForm = 1;
	    break;
	case 'a':
	    AllList = 1;
	    break;
	case 'R':
	    NoReplace = 1;
	    break;
	case 'r':
	    NoReference = 1;
	    break;
	case 'U':
	    DecodeURI = 0;
	    break;
	case 'v':
	    show_version();
	    return DIE_NOERROR;
	    break;
	case 'C':
	    if (load_conf(argv[0]))
	      return DIE_ERROR;
	    show_conf();
	    return DIE_NOERROR;
	    break;
	case 'L':
	    set_lang(optarg);
	    break;
	case 'o':
	    if (stdio2file(optarg))
	      return DIE_ERROR;
	    break;
	}
    } 

    return optind;
}

/* namazu core routine */
static int namazu_core(char * query, char *subquery, char *av0)
{
    char query_with_subquery[BUFSIZE * 2];
    HLIST hlist;

    /* make full-pathname of NMZ.{head,foot,msg,body,slog}.?? */
    make_fullpathname_msg();

    codeconv_query(query);
    codeconv_query(subquery);
    strcpy(query_with_subquery, query);
    strcat(query_with_subquery, " ");
    strcat(query_with_subquery, subquery);

    /* if query is null, show NMZ.head,body,foot and exit with error */
    if (*query == '\0') {
        if (HtmlOutput) {
            print(MSG_MIME_HEADER);
            print_headfoot(NMZ.head, query, subquery);
            print_msgfile(NMZ.body);
            print_headfoot(NMZ.foot, query, subquery);
        }
	free_idxnames();
	free_aliases();
	free_replaces();
	/*	exit(1);*/
	return 1;
    }

    debug_printf(" -n: %d\n", HListMax);
    debug_printf(" -w: %d\n", HListWhence);
    debug_printf("query: [%s]\n", query);

    if (HtmlOutput && IsCGI) {
	print(MSG_MIME_HEADER);
    }

    if (HtmlOutput) {
	print_headfoot(NMZ.head, query, subquery);
    }

    /* search */
    hlist = search_main(query_with_subquery);
    if (hlist.n == DIE_HLIST)
        return DIE_ERROR;

    /* result1:  <h2>Results:</h2>, References:  */
    if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
        print_result1();

        if (Idx.num > 1) {
            print("\n");
            if (HtmlOutput)
                print("<ul>\n");
        }
    }

    print_hit_count();

    /* result2 */
    if (!HitCountOnly && !ListFormat && !NoReference && !Quiet) {
        if (Idx.num > 1 && HtmlOutput) {
            print("</ul>\n");
        }
	if (Idx.num == 1 && HtmlOutput) {
	    print("\n</p>\n");
	} else {
	    fputc('\n', stdout);
	}
    }

    if (hlist.n > 0) {
        if (!HitCountOnly && !ListFormat && !Quiet) {
            print_hitnum(hlist.n);  /* <!-- HIT -->%d<!-- HIT --> */
        }
	if (HitCountOnly) {
	    printf("%d\n", hlist.n);
	} else {
	    print_listing(hlist); /* summary listing */
	}
        if (!HitCountOnly && !ListFormat && !Quiet) {
            print_range(hlist);
        }
    } else {
        if (HitCountOnly) {
            print("0\n");
        } else if (!ListFormat) {
            html_print(_("	<p>No document matching your query.</p>\n"));
	    if (HtmlOutput) {
		print_msgfile(NMZ.tips);
	    }
        }
    }

    if (HtmlOutput) {
	print_headfoot(NMZ.foot, query, subquery);
    }

    free_hlist(hlist);
    free_idxnames();
    free_aliases();
    free_replaces();

    return 0;
}

static void suicide (int signum)
{
    diemsg("processing time exceeds a limit: %d", SUICIDE_TIME);
    diewithmsg();
}

int main(int argc, char **argv)
{
    int i = 0;
    int ret;
    char query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    set_lang("");
    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    getenv_namazurc();

    Idx.num = 0;

    if (argc == 1) {
	if (load_conf(argv[0]))
	    diewithmsg();
	IsCGI = 1;	/* if no argument, assume this session as CGI */
	HtmlOutput = 1;
    } else {
	HtmlOutput = 0;		 /* do not display result by HTML format */
	DecodeURI = 1;		 /* decode a URI */
	HidePageIndex = 1;	 /* do not diplay page index */

	i = parse_options(argc, argv); 
	if (i == DIE_ERROR)
	    diewithmsg();
	if (i == DIE_NOERROR)
	    exit(0);
	if (load_conf(argv[0]))
	    diewithmsg();

	if (i == argc) {
	    show_mini_usage();
	    exit(1);
	}
	if (strlen(argv[i]) > QUERY_MAX) {
	    html_print(_(MSG_TOO_LONG_QUERY));
	    return 1;
	}
        strcpy(query, argv[i++]);
        if (i < argc) {
	    int curidx = getidxnum();
            for (curidx = 0; i < argc && curidx < INDEX_MAX; i++) {
	        curidx = add_index(argv[i]);
		if (curidx == ERR_MALLOC) {
		    die("main: malloc(idxname)");
		    break;
		} else if (curidx == ERR_INDEX_MAX) {
		    break;
		}
            }
        } 
        if (getidxnum() == 0) {
	    add_index(DEFAULT_INDEX);
	}
    }
    if (IsCGI) {
	init_cgi(query, subquery);
    }

    uniq_idxnames();
    if (expand_idxname_aliases())
        diewithmsg();
    if (complete_idxnames())
        diewithmsg();

    if (Debug) {
        for (i = 0; i < Idx.num; i++) {
            debug_printf("Idx.names[%d]: %s\n", i, Idx.names[i]);
        }
    }

    if (IsCGI) {
	/* set a suicide timer */
	signal(SIGALRM, suicide);
	alarm(SUICIDE_TIME);
    }

    ret = namazu_core(query, subquery, argv[0]);
    if (ret == DIE_ERROR)
        diewithmsg();
    return ret;
}

