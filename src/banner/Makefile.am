## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = banner
endif
bin_PROGRAMS = $(windows_progs) 

banner_LDADD = -lkernel32 -lgdi32

EXTRA_PROGRAMS = banner
