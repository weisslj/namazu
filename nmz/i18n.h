#ifndef _I18N_H
#define _I18N_H

/*
 * settings for gettext (libintl)
 */
#if ENABLE_NLS
# include <libintl.h>
# define _(Text) gettext (Text)
#else
# undef bindtextdomain
# define bindtextdomain(Domain, Directory) /* empty */
# undef textdomain
# define textdomain(Domain) /* empty */
# define _(Text) Text
#endif
#define N_(Text) Text


extern int is_lang_ja(void);
extern char *set_lang(char *);
extern char *get_lang(void);
extern char *choose_msgfile(char *fname);

#endif /* _I18N_H */
