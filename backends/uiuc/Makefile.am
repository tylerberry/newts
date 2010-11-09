MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

lib_LTLIBRARIES    = libuiuc.la
libuiuc_la_SOURCES = access.c access_list.c author_search.c close_nf.c \
	compress_nf.c create_nf.c delete_nf.c delete_note.c disk.c get_next_bug.c \
	get_note.c get_stats.c logical_resp.c misc.c modify_nf.c modify_note.c \
	modify_note_text.c open_nf.c sequencer.c text_search.c title_search.c \
	update_nf.c write_note.c
libuiuc_la_LIBADD  = $(top_builddir)/lib/libcommon.la \
	$(top_builddir)/gnulib/libgnu.la $(GETGROUPS_LIBS)
libuiuc_la_LDFLAGS = -version-info 1:0:0

noinst_HEADERS = access.h disk.h misc.h uiuc-backend.h
