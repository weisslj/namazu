#ifndef _FIELD_H
#define _FIELD_H

#define FIELD_SAFE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum {
    FIELD_CACHE_SIZE = 8        /* Size of field caches */
};

extern int isfield ( const char *key );
extern void get_field_name ( char *field, char *str );
extern void get_field_data ( int idxid, int docid, const char *orig_field, char *data );

#endif /* _FIELD_H */
