## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects

SUBDIRS = po
DIST_SUBDIRS =

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/lpr/lpr src/mkshortcut/mkshortcut \
	src/readshortcut/readshortcut
endif

if WITH_NATIVE_IPC_PROGRAMS
ipc_progs = src/ipc/semtool src/ipc/shmtool \
            src/ipc/msgtool src/ipc/semstat
ipc_scripts = src/ipc/ipck
endif

bin_PROGRAMS = src/ascii/ascii $(windows_progs) src/conv/conv \
	src/dump/dump $(ipc_progs) \
	src/realpath/realpath

bin_SCRIPTS = $(ipc_scripts)

noinst_HEADERS = \
	src/lpr/Printer.hh src/lpr/PrinterException.hh src/lpr/Win32Utils.hh

man_MANS = src/cygstart/cygstart.1 \
	src/mkshortcut/mkshortcut.1 \
	src/readshortcut/readshortcut.1 \
	src/lpr/lpr.1 

EXTRA_PROGRAMS = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart \
	src/ipc/semtool src/ipc/shmtool \
	src/ipc/msgtool src/ipc/semstat \
	src/lpr/lpr src/mkshortcut/mkshortcut src/readshortcut/readshortcut

EXTRA_SCRIPTS = src/ipc/ipck

po_files = \
	po/ChangeLog po/LINGUAS po/Makefile.in.in po/Makevars po/POTFILES.in \
	po/Rules-quot po/boldquot.sed po/en@boldquot.header po/en@quot.header \
	po/insert-header.sin po/quot.sed po/remove-potcdate.sin

extra_docs = \
	src/lpr/README

licenses = licenses/COPYING.BSD-no-advert licenses/COPYING.GPL

EXTRA_DIST = config.rpath PROGLIST bootstrap common.h \
   HOW-TO-CONTRIBUTE BRANCHES $(man_MANS) $(EXTRA_SCRIPTS) \
   $(licenses) $(po_files) $(extra_docs)

src_banner_banner_LDADD = -lkernel32 -lgdi32
src_lpr_lpr_LDADD = -lwinspool
src_mkshortcut_mkshortcut_LDADD = -lole32 -luuid
src_readshortcut_readshortcut_LDADD = -lole32 -luuid

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

