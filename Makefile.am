## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4

SUBDIRS = po
DIST_SUBDIRS =

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/lpr/lpr src/mkshortcut/mkshortcut \
	src/readshortcut/readshortcut
windows_ltlibraries = src/cygicons/libicons.la
windows_headers = src/cygicons/cygicons.h
cygicons_docs   = src/cygicons/README
lpr_docs        = src/lpr/README
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

lib_LTLIBRARIES = $(windows_ltlibraries)

noinst_HEADERS = \
	src/lpr/Printer.hh src/lpr/PrinterException.hh src/lpr/Win32Utils.hh

include_HEADERS = $(windows_headers)

man_MANS = src/cygstart/cygstart.1 \
	src/mkshortcut/mkshortcut.1 \
	src/readshortcut/readshortcut.1 \
	src/lpr/lpr.1 

doc_DATA = \
	AUTHORS COPYING ChangeLog HOW-TO-CONTRIBUTE \
	NEWS PROGLIST README TODO

licensesdir = $(docdir)/licenses
licenses_DATA = $(licenses)

EXTRA_PROGRAMS = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart \
	src/ipc/semtool src/ipc/shmtool \
	src/ipc/msgtool src/ipc/semstat \
	src/lpr/lpr src/mkshortcut/mkshortcut src/readshortcut/readshortcut

EXTRA_HEADERS = src/cygicons/cygicons.h
EXTRA_SCRIPTS = src/ipc/ipck

lpr_docdir        = $(docdir)/lpr
lpr_doc_DATA      = $(lpr_docs)
cygicons_docdir   = $(docdir)/cygicons
cygicons_doc_DATA = $(cygicons_docs)

po_files = \
	po/ChangeLog po/LINGUAS po/Makefile.in.in po/Makevars \
	po/Makevars.template po/POTFILES.in \
	po/Rules-quot po/boldquot.sed po/en@boldquot.header po/en@quot.header \
	po/insert-header.sin po/quot.sed po/remove-potcdate.sin

extra_docs = \
	src/lpr/README src/cygicons/README

licenses = licenses/COPYING.BSD-no-advert licenses/COPYING.GPL

cygicons_list = \
  src/cygicons/cygwin-kde.ico  src/cygicons/cygwings.ico \
  src/cygicons/cygwin.ico      src/cygicons/cygwinpurple.ico \
  src/cygicons/cygwinblue.ico  src/cygicons/cygwinred.ico \
  src/cygicons/cygwincyan.ico  src/cygicons/cygwinshiny.ico \
  src/cygicons/cygwing.ico     src/cygicons/cygwinyellow.ico

EXTRA_DIST = bootstrap common.h \
   BRANCHES $(man_MANS) $(doc_DATA) $(EXTRA_SCRIPTS) \
   $(licenses) $(po_files) $(extra_docs) $(cygicons_list)

DISTCLEANFILES = \
	po/Makefile    \
	po/Makefile.in \
	po/POTFILES    \
	po/remove-potcdate.sed \
	src/cygicons/cygicons.rc

src_ascii_ascii_LDADD               = -lpopt
src_banner_banner_LDADD             = -lpopt -lkernel32 -lgdi32
src_clip_getclip_LDADD              = -lpopt
src_clip_putclip_LDADD              = -lpopt
src_conv_conv_LDADD                 = -lpopt
src_cygstart_cygstart_LDADD         = -lpopt
src_dump_dump_LDADD                 = -lpopt
src_lpr_lpr_LDADD                   = -lwinspool
src_mkshortcut_mkshortcut_LDADD     = -lpopt -lole32 -luuid
src_readshortcut_readshortcut_LDADD = -lpopt -lole32 -luuid
src_realpath_realpath_LDADD         = -lpopt

src_lpr_lpr_SOURCES = \
	src/lpr/Printer.cc    src/lpr/Win32Utils.cc \
	src/lpr/lpr.cc        src/lpr/Printer.hh \
	src/lpr/Win32Utils.hh src/lpr/PrinterException.hh

src_cygicons_libicons_la_SOURCES = \
	src/cygicons/cygicons.rc \
	src/cygicons/cygicons.h
src_cygicons_libicons_la_LDFLAGS = -version-info 0:0:0 -no-undefined

install-exec-hook: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkdir_p) $(DESTDIR)$(bindir)
	@for p in d2u u2d dos2unix unix2dos; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  f=`echo $$p1|sed '$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f"; \
	  $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) src/conv/conv${EXEEXT} $(DESTDIR)$(bindir)/$$f; \
	done
	if [ -e $(DESTDIR)$(libdir)/libicons.dll.a ]; then \
	  (cd $(DESTDIR)$(libdir) && mv libicons.dll.a libcygicons.dll.a) ;\
	fi
	if [ -e $(DESTDIR)$(libdir)/libicons.a ]; then \
	  (cd $(DESTDIR)$(libdir) && mv libicons.a libcygicons.a) ;\
	fi
	if [ -e $(DESTDIR)$(libdir)/libicons.la ]; then \
	  (cd $(DESTDIR)$(libdir) && \
	   cat libicons.la | sed -e 's/libicons.a/libcygicons.a/g' \
	     -e 's/libicons.dll.a/libcygicons.dll.a/g' > \
	     libcygicons.la && \
	   rm -f libicons.la) ;\
	fi

uninstall-hook:
	@$(NORMAL_UNINSTALL)
	@for p in d2u u2d dos2unix unix2dos; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  f=`echo $$p1|sed '$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " rm -f $(DESTDIR)$(bindir)/$$f"; \
	  rm -f $(DESTDIR)$(bindir)/$$f; \
	done
	if [ -e $(DESTDIR)$(libdir)/libcygicons.la ]; then \
	   echo " $(LIBTOOL) --mode=uninstall rm -f '$(DESTDIR)$(libdir)/libcygicons.la'"; \
	   $(LIBTOOL) --mode=uninstall rm -f "$(DESTDIR)$(libdir)/libcygicons.la"; \
	fi

# hacks for RC support
RCFLAGS = -I$(top_builddir)/src/cygicons -I$(top_srcdir)/src/cygicons

LTRCCOMPILE = $(LIBTOOL) --mode=compile $(RC) \
	`echo $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(RCFLAGS) |\
	sed -e 's/-I/--include-dir /g;s/-D/--define /g'`

.rc.lo:
	$(LTRCCOMPILE) -i $< -o $@

## Extra dependencies for cygicons.lo
src/cygicons/cygicons.lo: src/cygicons/cygicons.rc $(cygicons_list) src/cygicons/cygicons.h

