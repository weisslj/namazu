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
    uchar buf[BUFSIZE * 4];
    strcpy(buf, MSG_USAGE);
#if	defined(_WIN32) || defined(__EMX__)
    euctosjis(buf);
#endif
    printf(buf, COPYRIGHT, VERSION);
}


void show_mini_usage(void)
{
    printf("Usage: namazu [options] <query> [index dir(s)]\n");
    printf("Try `namazu --help' for more options.\n");
}


