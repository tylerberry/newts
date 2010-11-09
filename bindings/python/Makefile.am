MAINTAINERCLEANFILES = Makefile.in

if HAVE_PYTHON
INCLUDES = -I$(top_srcdir)/include @PYTHONINCLUDE@

pyexec_LTLIBRARIES = newts.la
newts_la_SOURCES   = newtsmodule.c
newts_la_LIBADD    = $(top_builddir)/lib/libcommon.la \
			         $(top_builddir)/gnulib/libgnu.la \
					 $(top_builddir)/libnewts/libnewts.la \
			         $(top_builddir)/libnewtsclient/libnewtsclient.la
newts_la_LDFLAGS   = -module -avoid-version
endif
