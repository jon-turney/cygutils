## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

bin_PROGRAMS = last utmpdump 

man_MANS = last.1 lastb.1 
last_SOURCES = last.c oldutmp.h
utmpdump_SOURCES = utmpdump.c oldutmp.h

noinst_HEADERS = oldutmp.h
