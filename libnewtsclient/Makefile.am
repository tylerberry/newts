MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

lib_LTLIBRARIES           = libnewtsclient.la
libnewtsclient_la_SOURCES = backend_wrapper.c
libnewtsclient_la_LIBADD  = $(top_builddir)/libnewts/libnewts.la \
	$(top_builddir)/backends/uiuc/libuiuc.la
libnewtsclient_la_LDFLAGS = -version-info 1:0:0
