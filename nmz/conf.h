#ifndef _CONF_H
#define _CONF_H

#define DIRECTIVE_CHARS "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-_"

extern void show_conf(void);
extern enum nmz_stat load_conf(char *);

#endif /* _CONF_H */
