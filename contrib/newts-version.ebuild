# Copyright 1999-2006 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils

DESCRIPTION="A notesfile system and client programs."
SRC_URI="ftp://ftp.thoughtlocker.net/newts/${P}.tar.gz"
HOMEPAGE="http://www.arete.cc/newts/"

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86"

DEPEND=">=sys-libs/ncurses-5.4
	virtual/editor"

pkg_setup() {
	# This is mildly hackish; it would ideally be a fixed group and user known
	# by Gentoo.

	enewgroup notes
	enewuser notes -1 /bin/bash /var/spool/notes notes -c "Notes"
}

src_compile() {

	econf \
		--with-notes-spool=/var/spool/notes \
		--with-notes-user=notes \
		--with-notes-group=notes \
		--with-anon-user=nobody \
		|| die "configure error"

	emake || die "compile error"
}

src_install() {
	make DESTDIR="${D}" install || die "install error"

	dodoc AUTHORS BUGS ChangeLog HACKING NEWS NEWS.PRE README SURVEY THANKS \
		TODO WANTED || die "error installing docs"
}

#pkg_postinst() {
#	keepdir /var/spool/notes
#	keepdir /var/spool/notes/.sequencer
#
#	if test ! -f /var/spool/notes/.SEQ ; then echo "1" > /var/spool/notes/.SEQ ; fi
#	chown notes:notes /var/spool/notes/.SEQ
#	chmod 660 /var/spool/notes/.SEQ
#}
