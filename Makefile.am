## Makefile.am -- Process this file with automake to produce Makefile.in

AUTOMAKE_OPTIONS = subdir-objects
ACLOCAL_AMFLAGS = -I m4
AM_LDFLAGS = -Wl,--major-image-version=$(VERSION_MAJOR) -Wl,--minor-image-version=$(VERSION_MINOR)

SUBDIRS = po
DIST_SUBDIRS =

AM_CPPFLAGS = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart src/lpr/lpr src/mkshortcut/mkshortcut \
	src/readshortcut/readshortcut
windows_ltlibraries = src/cygicons/libicons.la
windows_headers = src/cygicons/cygicons.h
xdg_menus       = src/cygstart/cygstart.desktop
xdg_mimes       = src/cygstart/cygutils.xml
cygicons_docs   = src/cygicons/README
lpr_docs        = src/lpr/README
endif

if WITH_NATIVE_IPC_PROGRAMS
ipc_progs = src/ipc/semtool src/ipc/shmtool \
            src/ipc/msgtool src/ipc/semstat
ipc_scripts = src/ipc/ipck
endif

if WITH_CYGWIN_SPECIFIC_PROGRAMS
cygwin_specific_progs = src/cygdrop/cygdrop
endif

bin_PROGRAMS = $(windows_progs) src/conv/conv \
	src/dump/dump $(ipc_progs) \
	src/realpath/realpath $(cygwin_specific_progs)

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

xdg_menudir   = $(datadir)/applications
xdg_menu_DATA = $(xdg_menus)

xdg_mimedir   = $(datadir)/mime/packages
xdg_mime_DATA = $(xdg_mimes)

EXTRA_PROGRAMS = src/banner/banner src/clip/getclip src/clip/putclip \
	src/cygstart/cygstart \
	src/ipc/semtool src/ipc/shmtool \
	src/ipc/msgtool src/ipc/semstat \
	src/lpr/lpr \
	src/mkshortcut/mkshortcut src/readshortcut/readshortcut \
	src/cygdrop/cygdrop

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

licenses = licenses/COPYING.BSD-no-advert \
	licenses/COPYING.GPLv2 \
	licenses/COPYING.GPLv3

cygicons_list = \
  src/cygicons/cygwin-kde.ico  src/cygicons/cygwings.ico \
  src/cygicons/cygwin.ico      src/cygicons/cygwinpurple.ico \
  src/cygicons/cygwinblue.ico  src/cygicons/cygwinred.ico \
  src/cygicons/cygwincyan.ico  src/cygicons/cygwinshiny.ico \
  src/cygicons/cygwing.ico     src/cygicons/cygwinyellow.ico \
  src/cygicons/hippo.ico

EXTRA_DIST = bootstrap common.h \
   BRANCHES $(man_MANS) $(doc_DATA) $(EXTRA_SCRIPTS) \
   $(licenses) $(po_files) $(extra_docs) $(cygicons_list) \
   $(xdg_menus) $(xdg_mimes) \
   src/cygicons/hippo.svg

DISTCLEANFILES = \
	po/Makefile    \
	po/Makefile.in \
	po/POTFILES    \
	po/remove-potcdate.sed \
	src/cygicons/cygicons.rc

src_banner_banner_LDADD             = -lpopt -lkernel32 -lgdi32
src_clip_getclip_LDADD              = -lpopt
src_clip_putclip_LDADD              = -lpopt
src_conv_conv_LDADD                 = -lpopt
src_cygstart_cygstart_LDADD         = -lpopt
src_dump_dump_LDADD                 = -lpopt
src_lpr_lpr_LDADD                   = -lwinspool
src_mkshortcut_mkshortcut_LDADD     = -lpopt -lole32 /usr/lib/w32api/libuuid.a
src_readshortcut_readshortcut_LDADD = -lpopt -lole32 /usr/lib/w32api/libuuid.a
src_realpath_realpath_LDADD         = -lpopt

src_lpr_lpr_SOURCES = \
	src/lpr/Printer.cc    src/lpr/Win32Utils.cc \
	src/lpr/lpr.cc        src/lpr/Printer.hh \
	src/lpr/Win32Utils.hh src/lpr/PrinterException.hh

dist_src_cygicons_libicons_la_SOURCES = \
	src/cygicons/cygicons.h
nodist_src_cygicons_libicons_la_SOURCES = \
	src/cygicons/cygicons.rc

src_cygicons_libicons_la_LDFLAGS = -version-info 0:0:0 -no-undefined $(AM_LDFLAGS)

src_cygdrop_cygdrop_SOURCES  = src/cygdrop/cygdrop.cc
# This removes the unneeded dependency on cygstdc++-X.dll
src_cygdrop_cygdrop_CXXFLAGS = -fno-exceptions

install-exec-hook: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkdir_p) $(DESTDIR)$(bindir)
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
	if [ -e $(DESTDIR)$(libdir)/libcygicons.la ]; then \
	   echo " $(LIBTOOL) --mode=uninstall rm -f '$(DESTDIR)$(libdir)/libcygicons.la'"; \
	   $(LIBTOOL) --mode=uninstall rm -f "$(DESTDIR)$(libdir)/libcygicons.la"; \
	fi

# hacks for RC support
RCFLAGS = -I$(top_builddir)/src/cygicons -I$(top_srcdir)/src/cygicons
ltrc_verbose = $(ltrc_verbose_$(V))
ltrc_verbose_ = $(ltrc_verbose_$(AM_DEFAULT_VERBOSITY))
ltrc_verbose_0 = @echo "  CCRC  " $@;

LTRCCOMPILE = $(LIBTOOL) $(AM_V_lt) --tag=RC $(AM_LIBTOOLFLAGS) \
	$(LIBTOOLFLAGS) --mode=compile $(RC) \
	`echo $(DEFS) $(DEFAULT_INCLUDES) $(INCLUDES) $(AM_CPPFLAGS) \
	$(CPPFLAGS) $(AM_RCFLAGS) $(RCFLAGS) |\
	sed -e 's/-I/--include-dir /g;s/-D/--define /g'`

.rc.lo:
	$(ltrc_verbose)$(LTRCCOMPILE) -i $< -o $@

## Extra dependencies for cygicons.lo
src/cygicons/cygicons.lo: src/cygicons/cygicons.rc $(cygicons_list) src/cygicons/cygicons.h

