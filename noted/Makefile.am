MAINTAINERCLEANFILES = Makefile.in

datadir = @datadir@
libdir = @libdir@
localedir = $(datadir)/locale

DEFS     = -DMODULE_PATH=\"$(libdir)/@PACKAGE@\" -DLOCALEDIR=\"$(localedir)\" \
	@DEFS@
INCLUDES = -I../intl -I$(top_srcdir)/intl -I$(top_srcdir)/lib @INCLTDL@

sbin_PROGRAMS =

# noted_SOURCES = noted.c socket.c
# noted_LDFLAGS = -dlopen force
# noted_LDADD   = @LIBLTDL@ $(top_builddir)/lib/libcommon.la

noinst_HEADERS = module.h
