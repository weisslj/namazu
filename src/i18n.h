#ifndef _I18N_H
#define _I18N_H

int is_lang_ja(void);
char *set_lang(char *);
char *get_lang(void);
uchar *choose_msgfile(uchar *fname);

#endif /* _I18N_H */
