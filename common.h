#ifndef CYGUTILS_COMMON_H
#define CYGUTILS_COMMON_H 1

#if HAVE_CONFIG_H
#  include <config.h>
#endif

#include <stdio.h>

#if HAVE_SYS_TYPES_H
#  include <sys/types.h>
#endif /*HAVE_SYS_TYPES_H*/

#if STDC_HEADERS
#  include <stdlib.h>
#  include <string.h>
#elif HAVE_STRINGS_H
#  include <strings.h>
#endif /*STDC_HEADERS*/

#if HAVE_UNISTD_H
#  include <unistd.h>
#endif /*HAVE_UNISTD_H*/

#if HAVE_ERRNO_H
#  include <errno.h>
#endif /*HAVE_ERRNO_H*/

#if HAVE_SYS_FCNTL_H
#  include <sys/fcntl.h>
#endif /*HAVE_SYS_FCNTL_H*/

#if HAVE_LIMITS_H
#  include <limits.h>
#endif /*HAVE_LIMITS_H*/

#if HAVE_POPT_H
#  include <popt.h>
#endif /*HAVE_POPT_H*/

#if HAVE_WINDOWS_H
#  include <windows.h>
#else
#  ifndef O_BINARY
#    define O_BINARY 0
#  endif
#  define setmode(a, b) 
#endif /*HAVE_WINDOWS_H*/

#endif /* !CYGUTILS_COMMON_H */

