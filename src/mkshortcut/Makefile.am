## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = mkshortcut
endif
bin_PROGRAMS = $(windows_progs)

man_MANS = mkshortcut.1
mkshortcut_LDADD = -lole32 -luuid

EXTRA_PROGRAMS = mkshortcut
