#ifndef _SUPPORT_H
#define _SUPPORT_H

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

/* symbol mangling for libnmz.  */

#ifndef HAVE_MEMMOVE
#define memmove nmz_memmove
#endif

#ifndef HAVE_MEMSET
#define memset nmz_memset
#endif

#ifndef HAVE_STRCASECMP
#define strcasecmp nmz_strcasecmp
#endif

#ifndef HAVE_STRNCASECMP
#define strncasecmp nmz_strncasecmp
#endif

#ifndef HAVE_STRCSPN
#define strcspn nmz_strcspn
#endif

#ifndef HAVE_STRERROR
#define strerror nmz_strerror
#endif

#ifndef HAVE_VSNPRINTF
# ifdef HAVE__VSNPRINTF
# define vsnprintf _vsnprintf
# else
#  ifdef HAVE___VSNPRINTF
#  define vsnprintf __vsnprintf
#  else
#  define vsnprintf nmz_vsnprintf
#  endif
# endif
#endif

#endif /* _SUPPORT_H */
