/*
 * 
 * namazu.c - search client of Namazu
 *
 * $Id: namazu-cgi.c,v 1.3 2000-01-27 07:35:21 satoru Exp $
 * 
 * Copyright (C) 1997-2000 Satoru Takabayashi  All rights reserved.
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
#include <stdarg.h>

#if defined (WIN32) && !defined (__CYGWIN32__)
/*
 * It's not Unix, really.  See?  Capital letters. 
 */
#include <windows.h>
#define alarm(sec)	SetTimer(NULL,1,((sec)*1000),NULL)
#define	SIGALRM	14	/* alarm clock */
#endif

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif
#include "namazu.h"
#include "libnamazu.h"
#include "util.h"
#include "codeconv.h"
#include "rcfile.h"
#include "output.h"
#include "search.h"
#include "cgi.h"
#include "hlist.h"
#include "idxname.h"
#include "i18n.h"
#include "message.h"
#include "system.h"
#include "namazu.h"
#include "result.h"

/*
 *
 * Private functions
 *
 */

static void suicide ( int signum );

static void 
suicide (int signum)
{
    die("processing time exceeds a limit: %d", SUICIDE_TIME);
}

/*
 *
 * Public functions
 *
 */

int 
main(int argc, char **argv)
{
    char query[BUFSIZE] = "", subquery[BUFSIZE] = "";

    bindtextdomain(PACKAGE, LOCALEDIR);
    textdomain(PACKAGE);

    nmz_set_lang("");
    if (load_rcfile(argv[0]) != SUCCESS) {
	die(nmz_get_dyingmsg());
    }

    /* Both environment variables are required. */
    if (!(getenv("QUERY_STRING") && getenv("SCRIPT_NAME"))) {
	die("environment variable QUERY_STRING and SCRIPT_NAME are required");
    }

    /* 
     * Set a suicide timer for safety.
     * namazu.cgi will suicide automatically when SUICIDE_TIME reached.
     */
    signal(SIGALRM, suicide);
    alarm(SUICIDE_TIME);

    /*
     * Setting up CGI mode.
     */
    set_cgimode(1);
    set_refprint(1);
    set_htmlmode(1);
    set_pageindex(1);	 /* Print page index */
    set_formprint(1);	 /* Print "<form> ... </form>"  */
    set_uridecode(0);        /* Do not decode URI in results. */

    init_cgi(query, subquery);

    if (namazu_core(query, subquery, argv[0]) == ERR_FATAL) {
        die(nmz_get_dyingmsg());
    }

    return 0;
}
