#ifndef _WAKATI_H
#define _WAKATI_H

enum {
    KANJI,
    KATAKANA,
    HIRAGANA,
    OTHER
};

extern int wakati(char*);
extern int split_query(char*);

#endif /* _WAKATI_H */

