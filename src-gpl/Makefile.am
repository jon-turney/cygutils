## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

if WITH_WINDOWS_PROGRAMS
windows_progs = getclip putclip banner mkshortcut lpr
endif
if WITH_CYGIPC_PROGRAMS
cygipc_progs = semtool shmtool msgtool semstat
endif
bin_PROGRAMS = conv ascii dump \
	last utmpdump realpath $(windows_progs) \
	$(cygipc_progs)

man_MANS = last.1 lastb.1 mkshortcut.1
last_SOURCES = last.c oldutmp.h
banner_LDADD = -lkernel32 -lgdi32
mkshortcut_LDADD = -lole32 -luuid
lpr_LDADD = -lwinspool

noinst_HEADERS = oldutmp.h
install-exec-hook: $(bin_PROGRAMS)
	@$(NORMAL_INSTALL)
	$(mkinstalldirs) $(DESTDIR)$(bindir)
	@for p in d2u u2d dos2unix unix2dos; do \
	  p1=`echo $$p|sed 's/$(EXEEXT)$$//'`; \
	  f=`echo $$p1|sed '$(transform);s/$$/$(EXEEXT)/'`; \
	  echo " $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) conv${EXEEXT} $(DESTDIR)$(bindir)/$$f"; \
	  $(INSTALL_PROGRAM_ENV) $(INSTALL_PROGRAM) conv${EXEEXT} $(DESTDIR)$(bindir)/$$f; \
	done

EXTRA_PROGRAMS = semtool shmtool msgtool semstat \
	getclip putclip banner mkshortcut lpr
