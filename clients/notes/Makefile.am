MAINTAINERCLEANFILES = Makefile.in

datadir = @datadir@
localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

bin_PROGRAMS = notes

notes_SOURCES = access.c compose_mail.c compose_note.c curses_wrapper.c \
	director.c display_index.c gl_getline.c get_number.c get_text.c \
	limited_index.c malloc_die.c master.c notes.c pager.c print_time.c \
	read_note.c run_index.c setup.c signals.c spawn.c utmp.c

notes_LDADD   = $(top_builddir)/lib/libcommon.la \
	$(top_builddir)/gnulib/libgnu.la \
	$(top_builddir)/libnewtsclient/libnewtsclient.la \
	$(top_builddir)/clients/libnewtsutil/libnewtsutil.la \
	$(CURSES_LIB) \
	$(LTLIBINTL) \
	$(LIBS) \
	-lm

noinst_HEADERS = curses_wrapper.h gl_getline.h notes.h pager.h signals.h

install-exec-hook:
	chgrp $(NOTESGROUP) $(bindir)/notes
	chmod g+s $(bindir)/notes
