# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.52)
AC_INIT(cygutils, 0.9.9, cwilson@ece.gatech.edu)
AC_CONFIG_SRCDIR([config.h.in])
AM_CONFIG_HEADER([config.h])
AM_INIT_AUTOMAKE(cygutils, 0.9.6)
AM_MAINTAINER_MODE

# Checks for programs.
AC_PROG_CC
AC_PROG_INSTALL

# Checks for libraries.
AC_CHECK_LIB(popt, poptPrintHelp) 
AC_CHECK_LIB(cygipc, semget)
AC_CHECK_LIB(intl, gettext)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([stdlib.h stddef.h errno.h limits.h malloc.h \
ctype.h getopt.h popt.h windows.h])
AC_CHECK_HEADERS([string.h strings.h], break)
AC_CHECK_HEADERS([stdarg.h varargs.h], break)
AC_CHECK_HEADERS([utmp.h signal.h netinet/in.h netdb.h arpa/inet.h \
sys/socket.h])
AC_CHECK_HEADERS([sys/sem.h sys/shm.h sys/msg.h ])
AC_CHECK_HEADERS([langinfo.h libintl.h sys/param.h fcntl.h unistd.h])
AC_CHECK_HEADERS([wchar.h])
AC_CHECK_HEADERS(locale.h, AC_CHECK_FUNCS(setlocale))

# Checks for typedefs, structures, and compiler characteristics.
AC_C_CONST
AC_STRUCT_TM

# Checks for library functions.
AC_FUNC_MALLOC
AC_CHECK_FUNCS([strcasecmp strchr strdup strrchr strerror])
AC_CHECK_FUNCS([bzero getcwd memset nl_langinfo])
AC_FUNC_LSTAT
AC_FUNC_STAT
AC_FUNC_VPRINTF
dnl should only exist on windows, I hope
AC_CHECK_STDCALL_FUNC([OpenClipboard],[void *]) 

dnl Set Conditionals for Makefile.am
AM_CONDITIONAL(WITH_WINDOWS_PROGRAMS, test "$ac_cv_func_OpenClipboard" = yes)
AM_CONDITIONAL(WITH_CYGIPC_PROGRAMS, test "$ac_cv_lib_cygipc_semget" = yes)

AC_CONFIG_FILES([Makefile src-gpl/Makefile src-pd/Makefile src-bsd/Makefile])
AC_OUTPUT
