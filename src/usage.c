/*
 * 
 * usage.c -
 * 
 * $Id: usage.c,v 1.9 1999-11-18 13:42:07 satoru Exp $
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
#include "namazu.h"
#include "codeconv.h"
#include "message.h"
#include "usage.h"

/************************************************************
 *
 * Public functions
 *
 ************************************************************/

/* display the usage and version info and exit */
void show_long_usage(void)
{
    char buf[BUFSIZE * 4];
    char *usage = (char *)
    N_("\
namazu of Namazu %s\n\n\
Usage: namazu [options] <query> [index]... \n\
    -n, --max=num        set number of documents shown at once.\n\
    -w, --whence=num     set first number of documents shown in results.\n\
    -l, --list           print results by listing format.\n\
    -s, --short          print results by short format.\n\
        --results=ext    set NMZ.result.ext for printing results.\n\
        --late           sort documents in late order.\n\
        --early          sort documents in early order.\n\
        --sort=method    set a sort method (score, date, field:name)\n\
        --ascending      sort in ascending order (default: descending)\n\
    -a, --all            print all results.\n\
    -c, --count          print only number of hits.\n\
    -h, --html           print in HTML format.\n\
    -r, --no-references  do not display reference hit counts.\n\
    -H, --page           print further result links. (nearly meaningless)\n\
    -F, --form           force to print <form> ... </form> region.\n\
    -R, --no-replace     do not replace URI string.\n\
    -U, --no-encode-uri  do not decode URI when printing in a plain format.\n\
    -o, --output=file    set output file name.\n\
    -f, --config=file    set a pathname of a config file.\n\
    -C, --show-config    print current configuration.\n\
    -q, --quiet          do not display extra messages except search results.\n\
    -L, --lang=lang      set language (ja or en)\n\
    -v, --version        show the version of namazu and exit.\n\
        --help           show this help and exit\n");

    strcpy(buf, _(usage));
    printf(buf, VERSION);
}


void show_mini_usage(void)
{
    printf(_("Usage: namazu [options] <query> [index]...\n"));
    printf(_("Try `namazu --help' for more information.\n"));
}

void show_version(void)
{
    printf("namazu of Namazu %s\n\n", VERSION);
    printf("Copyright (C) 1997-1999 Satoru Takabayashi All rights reserved.\n");
    printf("This is free software; you can redistribute it and/or modify\n");
    printf("it under the terms of the GNU General Public License as published by\n");
    printf("the Free Software Foundation; either version 2, or (at your option)\n");
    printf("any later version.\n\n");
    printf("This program is distributed in the hope that it will be useful,\n");
    printf("but WITHOUT ANY WARRANTY; without even the implied warranty\n");
    printf("of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\n");
    printf("GNU General Public License for more details.\n");
}
