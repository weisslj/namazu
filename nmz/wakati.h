#ifndef _WAKATI_H
#define _WAKATI_H

enum {
    KANJI,
    KATAKANA,
    HIRAGANA,
    OTHER
};

int wakati(char*);
int split_query(char*);

#endif /* _WAKATI_H */
