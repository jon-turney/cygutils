## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_CYGIPC_PROGRAMS
cygipc_progs = semtool shmtool msgtool semstat
endif
bin_PROGRAMS = $(cygipc_progs)

EXTRA_PROGRAMS = semtool shmtool msgtool semstat 
