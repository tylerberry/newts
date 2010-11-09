MAINTAINERCLEANFILES = Makefile.in version.c

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

lib_LTLIBRARIES     = libnewts.la
libnewts_la_SOURCES = access.c author.c error.c getfqdn.c list.c memory.c \
	nfref.c notesfile.c parse.c stats.c version.c
libnewts_la_LIBADD  = $(top_builddir)/lib/libcommon.la \
	$(top_builddir)/gnulib/libgnu.la
libnewts_la_LDFLAGS = -version-info 1:0:0

version.c:
	if test ! -f version.c ; then make version ; fi

version:
	cd $(top_builddir) && \
	CONFIG_FILES= CONFIG_HEADERS= CONFIG_OTHER=libnewts/version.c \
	$(SHELL) ./config.status

dist-hook: version
	cp $(srcdir)/version.c $(distdir)/version.c
