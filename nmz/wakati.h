#ifndef _WAKATI_H
#define _WAKATI_H

enum {
    KANJI,
    KATAKANA,
    HIRAGANA,
    OTHER
};

int wakati(uchar*);
int split_query(uchar*);

#endif /* _WAKATI_H */
