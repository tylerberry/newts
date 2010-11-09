MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

noinst_LTLIBRARIES      = libnewtsutil.la
libnewtsutil_la_SOURCES = blacklist.c misc.c
libnewtsutil_la_LIBADD  = $(top_builddir)/gnulib/libgnu.la
