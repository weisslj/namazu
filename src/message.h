#ifndef _MESSAGE_H
#define _MESSAGE_H

extern uchar *COPYRIGHT;
extern uchar *MSG_USAGE, *MSG_TOO_LONG_KEY, *MSG_TOO_MANY_KEYITEM,
*MSG_QUERY_STRING_TOO_LONG, *MSG_RESULT_HEADER, *MSG_NO_HIT, *MSG_HIT_1, 
*MSG_HIT_2, *MSG_TOO_MUCH_HIT, *MSG_TOO_MUCH_MATCH, *MSG_INDEXDIR_ERROR,
*MSG_REFERENCE_HEADER, *MSG_INVALID_DB_NAME, *MSG_INVALID_QUERY,
*MSG_CANNOT_OPEN_INDEX, *MSG_MIME_HEADER, *MSG_CANNOT_OPEN_REGEX_INDEX,
*MSG_CANNOT_OPEN_PHRASE_INDEX, *MSG_CANNOT_OPEN_FIELD_INDEX;

void init_message(void);

#endif /* _MESSAGE_H */
