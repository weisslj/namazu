#ifndef _FIELD_H
#define _FIELD_H

#define FIELD_SAFE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

enum {
    FIELD_CACHE_SIZE = 8        /* Size of field caches */
};

int is_field_safe_character(int);
int isfield(uchar *);
void apply_field_alias(uchar *);
void get_field_name(uchar *, uchar *);
void get_field_data(int, int, uchar *, uchar *);

#endif /* _FIELD_H */
