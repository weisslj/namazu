#ifndef _NAMAZU_H
#define _NAMAZU_H

#ifdef HAVE_CONFIG_H
#  include "config.h"
#endif

#include <stdio.h>

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


#include "message.h"
#include "libnamazu.h"

#endif /* _NAMAZU_H */
