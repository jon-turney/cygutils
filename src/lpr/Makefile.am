## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = lpr
endif
bin_PROGRAMS = $(windows_progs)

lpr_LDADD = -lwinspool

EXTRA_PROGRAMS = lpr
