## Makefile.am -- Process this file with automake to produce Makefile.in

INCLUDES = -I$(top_builddir) -I$(top_srcdir)

bin_PROGRAMS = conv ascii dump getclip putclip \
	semtool shmtool msgtool semstat banner \
	last utmpdump realpath
man_MANS = last.1 lastb.1
last_SOURCES = last.c oldutmp.h
banner_LDADD = -lkernel32 -lgdi32
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
