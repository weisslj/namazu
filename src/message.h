#ifndef _MESSAGE_H
#define _MESSAGE_H

#if defined(_WIN32) || defined(__EMX__)
#define MSG_MIME_HEADER  "Content-type: text/html\n\n"
#else
#define MSG_MIME_HEADER  "Content-type: text/html\r\n\r\n"
#endif

#endif /* _MESSAGE_H */
