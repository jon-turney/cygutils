## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS =
DIST_SUBDIRS =

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/lpr/lpr src/mkshortcut/mkshortcut
endif

if WITH_CYGIPC_PROGRAMS
cygipc_progs = src/ipc/semtool src/ipc/shmtool src/ipc/msgtool src/ipc/semstat
endif

bin_PROGRAMS = src/ascii/ascii $(windows_progs) src/cal/cal src/conv/conv \
	src/ddate/ddate src/dump/dump $(cygipc_progs) src/last/last \
	src/last/utmpdump src/namei/namei src/realpath/realpath src/col/col \
	src/getopt/getopt src/readlink/readlink

noinst_HEADERS = src/cal/errs.h src/cal/widechar.h src/last/oldutmp.h \
	src/col/err.h

man_MANS = src/cal/cal.1 src/cygstart/cygstart.1 src/ddate/ddate.1 \
	src/last/last.1 src/last/lastb.1 src/mkshortcut/mkshortcut.1 \
	src/namei/namei.1 src/col/col.1 src/getopt/getopt.1 src/readlink/readlink.1

EXTRA_PROGRAMS = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/ipc/semtool src/ipc/shmtool \
	src/ipc/msgtool src/ipc/semstat src/lpr/lpr \
	src/mkshortcut/mkshortcut

EXTRA_DIST = src/cal/README.cal src/ddate/ddate.doc src/namei/README.namei \
  	src/namei/README1.namei src/col/README.col \
	src/getopt/parse.bash src/getopt/parse.tcsh \
	src/getopt/test.bash src/getopt/test.tcsh

src_banner_banner_LDADD = -lkernel32 -lgdi32
src_lpr_lpr_LDADD = -lwinspool
src_mkshortcut_mkshortcut_LDADD = -lole32 -luuid

src_cal_cal_SOURCES = src/cal/cal.c src/cal/err.c src/cal/errs.h
src_last_last_SOURCES = src/last/last.c src/last/oldutmp.h
src_last_utmpdump_SOURCES = src/last/utmpdump.c src/last/oldutmp.h
src_col_col_SOURCES = src/col/col.c src/col/err.c src/col/err.h

install-exec-hook: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@for p in d2u u2d dos2unix unix2dos; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  f=`echo $$p1|sed '$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f"; \
	  $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f; \
	done
	 
