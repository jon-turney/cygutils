# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)
AC_INIT(cygutils, 1.1.3, cwilson@ece.gatech.edu)
AC_CONFIG_SRCDIR([config.h.in])
AM_INIT_AUTOMAKE
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AM_PROG_CC_C_O
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB(popt, poptPrintHelp) 
AC_CHECK_LIB(cygipc, semget)
# AC_CHECK_LIB(intl, gettext)
AM_GNU_GETTEXT(external,[],[])
BUILD_INCLUDED_LIBINTL=no
USE_INCLUDED_LIBINTL=no
AC_SUBST(BUILD_INCLUDED_LIBINTL)
AC_SUBST(USE_INCLUDED_LIBINTL)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([stdlib.h stddef.h errno.h limits.h malloc.h \
ctype.h getopt.h popt.h windows.h])
AC_CHECK_HEADERS([string.h strings.h], break)
AC_CHECK_HEADERS([stdarg.h varargs.h], break)
AC_CHECK_HEADERS([sys/sem.h sys/shm.h sys/msg.h ])
AC_CHECK_HEADERS([langinfo.h sys/param.h fcntl.h unistd.h \
termios.h sys/ioctl.h])
AC_CHECK_HEADERS([wchar.h])
AC_CHECK_HEADERS([locale.h])
AC_CHECK_FUNCS([setlocale])
AC_HEADER_TIOCGWINSZ

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strcasecmp strchr strdup strrchr strerror])
AC_CHECK_FUNCS([bzero getcwd memset nl_langinfo gettimeofday])
AC_FUNC_LSTAT
AC_FUNC_STAT
AC_FUNC_VPRINTF
dnl should only exist on windows, I hope
AC_CHECK_STDCALL_FUNC([OpenClipboard],[void *]) 

dnl Set Conditionals for Makefile.am
AM_CONDITIONAL(WITH_WINDOWS_PROGRAMS, test "$ac_cv_func_OpenClipboard" = yes)
AM_CONDITIONAL(WITH_CYGIPC_PROGRAMS, test "$ac_cv_lib_cygipc_semget" = yes)

AC_CONFIG_FILES([Makefile po/Makefile.in])

AC_OUTPUT
