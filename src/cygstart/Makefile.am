## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = cygstart
endif
bin_PROGRAMS = $(windows_progs)

man_MANS = cygstart.1

EXTRA_PROGRAMS = cygstart
