#ifndef _MESSAGE_H
#define _MESSAGE_H

#if defined(_WIN32) || defined(__EMX__)
    #define MSG_MIME_HEADER  "Content-type: text/html\n\n"
#else
    #define MSG_MIME_HEADER  "Content-type: text/html\r\n\r\n"
#endif

extern uchar *COPYRIGHT;
extern uchar *MSG_USAGE;
extern uchar *MSG_TOO_LONG_KEY;
extern uchar *MSG_TOO_MANY_KEYITEM;
extern uchar *MSG_QUERY_STRING_TOO_LONG;
extern uchar *MSG_RESULT_HEADER;
extern uchar *MSG_NO_HIT;
extern uchar *MSG_HIT_1;
extern uchar *MSG_HIT_2;
extern uchar *MSG_ERR_TOO_MUCH_HIT;
extern uchar *MSG_ERR_TOO_MUCH_MATCH;
extern uchar *MSG_INDEXDIR_ERROR;
extern uchar *MSG_REFERENCE_HEADER;
extern uchar *MSG_INVALID_DB_NAME;
extern uchar *MSG_INVALID_QUERY;
extern uchar *MSG_CANNOT_OPEN_INDEX;
extern uchar *MSG_CANNOT_OPEN_REGEX_INDEX;
extern uchar *MSG_CANNOT_OPEN_PHRASE_INDEX;
extern uchar *MSG_CANNOT_OPEN_FIELD_INDEX;

void init_message(void);

#endif /* _MESSAGE_H */
