MAINTAINERCLEANFILES = Makefile.in

datadir = @datadir@
localedir = $(datadir)/locale

DEFS = -DLOCALEDIR=\"$(localedir)\" @DEFS@
INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib \
	-I$(top_srcdir)/lib

FRONTENDLIBS = $(top_builddir)/libnewtsclient/libnewtsclient.la \
	$(top_builddir)/clients/libnewtsutil/libnewtsutil.la \
	$(top_builddir)/gnulib/libgnu.la \
    $(LTLIBINTL) \
	$(LIBS)

bin_PROGRAMS = autoseq checknotes getnote mknf nfadmin nfdump nfload nfmail \
	nfpipe nfprint nfstats nftimestamp rmnf

autoseq_SOURCES = autoseq.c
autoseq_CFLAGS  = -DNOTESBINARY=\"$(bindir)/notes\"
autoseq_LDADD   = $(LTLIBINTL) $(LIBS)

checknotes_SOURCES = checknotes.c common.c
checknotes_LDADD   = $(FRONTENDLIBS)

getnote_SOURCES = getnote.c common.c
getnote_LDADD   = $(FRONTENDLIBS)

mknf_SOURCES = mknf.c common.c
mknf_LDADD   = $(FRONTENDLIBS)

nfadmin_SOURCES = nfadmin.c common.c
nfadmin_LDADD   = $(FRONTENDLIBS)

nfdump_SOURCES = nfdump.c dump-uiuc.c common.c
nfdump_LDADD   = $(FRONTENDLIBS)

nfload_SOURCES = nfload.c scan-uiuc.l common.c
nfload_LDADD   = $(FRONTENDLIBS)

nfmail_SOURCES = nfmail.c common.c
nfmail_LDADD   = $(FRONTENDLIBS)

nfpipe_SOURCES = nfpipe.c common.c
nfpipe_LDADD   = $(FRONTENDLIBS)

nfprint_SOURCES = nfprint.c common.c
nfprint_LDADD   = $(FRONTENDLIBS)

nfstats_SOURCES = nfstats.c common.c
nfstats_LDADD   = $(FRONTENDLIBS)

nftimestamp_SOURCES = nftimestamp.c common.c
nftimestamp_LDADD   = $(FRONTENDLIBS)

rmnf_SOURCES = rmnf.c common.c
rmnf_LDADD   = $(FRONTENDLIBS)

noinst_HEADERS = dump-uiuc.h frontend.h scan-uiuc.h

install-exec-hook:
	chgrp $(NOTESGROUP) $(bindir)/{checknotes,getnote,mknf,nfadmin,nfdump,nfload,nfmail,nfpipe,nfprint,nfstats,nftimestamp,rmnf}
	chmod g+s $(bindir)/{checknotes,getnote,mknf,nfadmin,nfdump,nfload,nfmail,nfpipe,nfprint,nfstats,nftimestamp,rmnf}
