#ifndef _MESSAGE_H
#define _MESSAGE_H

#if defined(_WIN32) || defined(__EMX__)
    #define MSG_MIME_HEADER  "Content-type: text/html\n\n"
#else
    #define MSG_MIME_HEADER  "Content-type: text/html\r\n\r\n"
#endif


/*
 * beggening '	' (tab) means those string contains HTML tag.
 * It's treated with special care as Namazu's HTML message.
 */

#define MSG_TOO_LONG_QUERY \
    N_("	<h2>Error!</h2>\n<p>Too long Query.</p>\n")


#endif /* _MESSAGE_H */
