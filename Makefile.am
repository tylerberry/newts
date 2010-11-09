# FIXME: Is 1.7 actually the minimum? It's at *least* 1.7, it could be
# higher.
AUTOMAKE_OPTIONS = 1.7 gnits dist-bzip2
ACLOCAL_AMFLAGS = -I m4 -I m4/gnulib

EXTRA_DIST = BUGS HACKING NEWS.PRE SURVEY WANTED bootstrap gnulib/xalloc.h
MAINTAINERCLEANFILES = ChangeLog Makefile.in aclocal.m4 autom4te.cache \
	compile config.guess config.h.in config.rpath config.sub configure \
	depcomp install-sh ltmain.sh missing m4/libtool.m4 ylwrap

SUBDIRS = m4 doc include gnulib lib libnewts backends libnewtsclient clients \
	bindings contrib tests po

ChangeLog: dist-hook

version:
	cd libnewtsclient && make version

dist-hook:
	svn update >/dev/null 2>/dev/null
	svn log >/dev/null 2>/dev/null && svn log > ChangeLog

maintainer-clean-local:
	rm -rf libltdl

install-data-hook:
	mkdir -p ${DESTDIR}/${SPOOL}
	chown ${NOTES}:${NOTESGROUP} ${DESTDIR}/${SPOOL}
	chmod 775 ${DESTDIR}/${SPOOL}
	mkdir -p ${DESTDIR}/${SPOOL}/.sequencer
	chown ${NOTES}:${NOTESGROUP} ${DESTDIR}/${SPOOL}/.sequencer
	chmod 770 ${DESTDIR}/${SPOOL}/.sequencer
	if test ! -f ${DESTDIR}/${SPOOL}/.SEQ ; then echo "1" > ${DESTDIR}/${SPOOL}/.SEQ ; fi
	chown ${NOTES}:${NOTESGROUP} ${DESTDIR}/${SPOOL}/.SEQ
	chmod 660 ${DESTDIR}/${SPOOL}/.SEQ
