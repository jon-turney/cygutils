# Process this file with autoconf to produce a configure script.
AC_PREREQ(2.63)
AC_INIT(cygutils, 1.4.1, cygwin@cygwin.com)
AC_CONFIG_SRCDIR([config.h.in])
AC_CONFIG_AUX_DIR([build-aux])
AC_CONFIG_MACRO_DIR([m4])
AM_INIT_AUTOMAKE([1.11 silent-rules -Wall -Werror dist-lzma no-dist-gzip])
AM_CONFIG_HEADER([config.h])
AM_MAINTAINER_MODE

VERSION_MAJOR=`echo $VERSION | cut -f1 -d.`
VERSION_MINOR=`echo $VERSION | cut -f2 -d.`
VERSION_PATCH=`echo $VERSION | cut -f3 -d.`

AC_SUBST(VERSION_MAJOR)
AC_SUBST(VERSION_MINOR)
AC_SUBST(VERSION_PATCH)

AC_CANONICAL_HOST

# Checks for programs.
AC_PROG_CC
AC_PROG_CXX
AM_PROG_CC_C_O
AC_PROG_INSTALL
AC_PROG_LN_S

LT_INIT([win32-dll dlopen disable-static])
LT_LANG([Windows Resource])

# Checks for libraries.
AC_CHECK_LIB(popt, poptPrintHelp,[
  AC_DEFINE(HAVE_LIBPOPT,1,[Define to 1 if you have the `popt' library (-lpopt).])])dnl '`
AC_CHECK_FUNC(shmget,[
  HAVE_INTRINSIC_IPC="yes"
  ],[
  HAVE_INTRINSIC_IPC="no"
  ])

# AC_CHECK_LIB(intl, gettext)
AM_GNU_GETTEXT(external,[],[])
AM_GNU_GETTEXT_VERSION([0.17])
BUILD_INCLUDED_LIBINTL=no
USE_INCLUDED_LIBINTL=no
AC_SUBST(BUILD_INCLUDED_LIBINTL)
AC_SUBST(USE_INCLUDED_LIBINTL)

CYGICONS_DLL_VERSION=0
AC_SUBST(CYGICONS_DLL_VERSION)

# Checks for header files.
AC_HEADER_STDC
AC_HEADER_TIME
AC_CHECK_HEADERS([stdlib.h stddef.h errno.h limits.h malloc.h \
ctype.h getopt.h popt.h windows.h])
AC_CHECK_HEADERS([string.h strings.h], break)
AC_CHECK_HEADERS([stdarg.h varargs.h], break)
AC_CHECK_HEADERS([sys/ipc.h sys/sem.h sys/shm.h sys/msg.h])
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
AC_CHECK_DECLS([cygwin_conv_path], [],[
case "$host" in
*cygwin* ) AC_MSG_ERROR([At least cygwin-1.7 is required]) ;;
esac],dnl
  [[#include <sys/cygwin.h>]])

dnl Set Conditionals for Makefile.am
AM_CONDITIONAL(WITH_WINDOWS_PROGRAMS, test "$ac_cv_func_OpenClipboard" = yes)
AM_CONDITIONAL(WITH_NATIVE_IPC_PROGRAMS, test "$HAVE_INTRINSIC_IPC" = yes)
host_is_cygwin=no
case "$host" in
*cygwin* ) host_is_cygwin=yes ;;
esac
AM_CONDITIONAL(WITH_CYGWIN_SPECIFIC_PROGRAMS, test "$host_is_cygwin" = yes)

AC_CONFIG_FILES([Makefile po/Makefile.in src/cygicons/cygicons.rc])

AC_OUTPUT
