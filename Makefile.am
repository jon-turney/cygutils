## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = po
DIST_SUBDIRS =

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/lpr/lpr src/mkshortcut/mkshortcut
endif

if WITH_IPC_PROGRAMS
ipc_progs = src/ipc/semtool src/ipc/shmtool src/ipc/msgtool \
	src/ipc/semstat src/ipc/ipcrm src/ipc/ipcs
ipc_scripts = src/ipc/ipck
ipc_mans = src/ipc/ipcs.8 src/ipc/ipcrm.8
endif

bin_PROGRAMS = src/ascii/ascii $(windows_progs) src/cal/cal src/conv/conv \
	src/ddate/ddate src/dump/dump $(ipc_progs) \
	src/namei/namei src/realpath/realpath src/col/col \
	src/getopt/getopt src/readlink/readlink src/rev/rev src/column/column \
	src/colcrt/colcrt src/colrm/colrm src/mcookie/mcookie

bin_SCRIPTS = $(ipc_scripts)

noinst_HEADERS = src/cal/errs.h src/cal/widechar.h \
	src/col/err.h src/rev/widechar.h \
	src/column/widechar.h src/column/errs.h \
	src/colcrt/widechar.h src/colrm/widechar.h src/mcookie/md5.h \
	src/lpr/Printer.hh src/lpr/PrinterException.hh src/lpr/Win32Utils.hh

man_MANS = src/cal/cal.1 src/cygstart/cygstart.1 src/ddate/ddate.1 \
	src/mkshortcut/mkshortcut.1 \
	src/namei/namei.1 src/col/col.1 src/getopt/getopt.1 \
	src/readlink/readlink.1 src/rev/rev.1 src/column/column.1 \
	src/colcrt/colcrt.1 src/colrm/colrm.1 src/mcookie/mcookie.1 \
	src/lpr/lpr.1 $(ipc_mans)

EXTRA_PROGRAMS = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/ipc/semtool src/ipc/shmtool \
	src/ipc/msgtool src/ipc/semstat src/ipc/ipcrm src/ipc/ipcs \
	src/lpr/lpr src/mkshortcut/mkshortcut

EXTRA_SCRIPTS = src/ipc/ipck

po_files = \
	po/ChangeLog po/LINGUAS po/Makefile.in.in po/Makevars po/POTFILES.in \
	po/Rules-quot po/boldquot.sed po/en@boldquot.header po/en@quot.header \
	po/insert-header.sin po/quot.sed po/remove-potcdate.sin

extra_docs = \
	src/cal/README.cal src/ddate/ddate.doc src/namei/README.namei \
	src/namei/README1.namei src/col/README.col \
	src/getopt/parse.bash src/getopt/parse.tcsh \
	src/getopt/test.bash src/getopt/test.tcsh \
	src/ipc/ipcs.8 src/ipc/ipcrm.8

licenses = licenses/COPYING.BSD-no-advert licenses/COPYING.GPL

EXTRA_DIST = config.rpath PROGLIST bootstrap common.h HOW-TO-CONTRIBUTE \
   $(licenses) $(po_files) $(extra_docs) $(man_MANS)

src_banner_banner_LDADD = -lkernel32 -lgdi32
src_lpr_lpr_LDADD = -lwinspool
src_mkshortcut_mkshortcut_LDADD = -lole32 -luuid
src_cal_cal_LDADD = @LIBINTL@
src_ddate_ddate_LDADD = @LIBINTL@
src_namei_namei_LDADD = @LIBINTL@
src_rev_rev_LDADD = @LIBINTL@
src_column_column_LDADD = @LIBINTL@
src_colcrt_colcrt_LDADD = @LIBINTL@
src_mcookie_mcookie_LDADD = @LIBINTL@

src_cal_cal_SOURCES = src/cal/cal.c src/cal/err.c src/cal/errs.h
src_col_col_SOURCES = src/col/col.c src/col/err.c src/col/err.h
src_rev_rev_SOURCES = src/rev/rev.c src/rev/widechar.h
src_column_column_SOURCES = src/column/column.c src/column/err.c src/column/errs.h src/column/widechar.h
src_colcrt_colcrt_SOURCES = src/colcrt/colcrt.c src/colcrt/widechar.h
src_colrm_colrm_SOURCES = src/colrm/colrm.c src/colrm/widechar.h
src_mcookie_mcookie_SOURCES = src/mcookie/mcookie.c src/mcookie/md5.c src/mcookie/md5.h
src_lpr_lpr_SOURCES = src/lpr/Printer.cc src/lpr/Win32Utils.cc src/lpr/lpr.cc src/lpr/Printer.hh src/lpr/PrinterException.hh src/lpr/Win32Utils.hh

install-exec-hook: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@for p in d2u u2d dos2unix unix2dos; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  f=`echo $$p1|sed '$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f"; \
	  $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f; \
	done

