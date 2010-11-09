# Copyright 1999-2004 Gentoo Technologies, Inc.
# Distributed under the terms of the GNU General Public License v2
# $Header: $

inherit eutils subversion

IUSE="static"

ESVN_REPO_URI="http://svn.arete.cc/newts/trunk/"
ESVN_BOOTSTRAP="bootstrap"

DESCRIPTION="A notesfile system and client programs."
SRC_URI=""
HOMEPAGE="http://www.arete.cc/newts/"

SLOT="0"
LICENSE="GPL-2"
KEYWORDS="~x86 ~ppc ~ppc-macos"

DEPEND=">=sys-libs/ncurses-5.4
	virtual/editor
	>=sys-devel/autoconf-2.58
	>=sys-devel/automake-1.8
	>=sys-devel/libtool-1.5"

pkg_setup() {
	# This is mildly hackish; it would ideally be a fixed group and user known
	# by Gentoo.

	enewgroup notes
	enewuser notes -1 /bin/bash /var/spool/notes notes -c "Notes"
}

src_compile() {
	local myconf

	use static && myconf="--enable-static --disable-shared"

	econf \
		--with-notes-spool=/var/spool/notes \
		--with-notes-user=notes \
		--with-notes-group=notes \
		--with-anon-user=nobody \
		${myconf} || die "configure error"

	make -C po en@quot.po-update || die "compile error"
	make -C po en@boldquot.po-update || die "compile error"
	emake || die "compile error"
}

src_install() {
	into /usr

	dolib backends/uiuc/libuiuc.la || die "error installing libs"
	dolib backends/uiuc/.libs/libuiuc.* || die "error installing libs"
	dolib libnewtsclient/libnewtsclient.la || die "error installing libs"
	dolib libnewtsclient/.libs/libnewtsclient.* || die "error installing libs"

	dobin clients/notes/.libs/notes || die "error installing notes"
	dobin clients/frontends/autoseq || die "error installing autoseq"
	dobin clients/frontends/.libs/{checknotes,getnote,mknf,nfadmin} \
		|| die "error installing frontends"
	dobin clients/frontends/.libs/{nfdump,nfload,nfmail,nfpipe,nfprint,nfstats} \
		|| die "error installing frontends"
	dobin clients/frontends/.libs/{nftimestamp,rmnf} \
		|| die "error installing frontends"

	fowners root:notes /usr/bin/{notes,autoseq,checknotes,getnote,mknf,nfadmin}
	fowners root:notes /usr/bin/{nfdump,nfload,nfmail,nfpipe,nfprint,nfstats}
	fowners root:notes /usr/bin/{nftimestamp,rmnf}
	fperms 2755 /usr/bin/{notes,autoseq,checknotes,getnote,mknf,nfadmin}
	fperms 2755 /usr/bin/{nfdump,nfload,nfmail,nfpipe,nfprint,nfstats}
	fperms 2755 /usr/bin/{nftimestamp,rmnf}

	cd ${S}
	insinto /usr/include/newts
	doins include/newts/*.h

	dodoc AUTHORS BUGS ChangeLog HACKING NEWS NEWS.PRE README SURVEY THANKS \
		TODO WANTED || die "error installing docs"
	doman doc/notes.1 doc/autoseq.1 || die "error installing man pages"
	doinfo doc/newts.info* || die "error installing info pages"

	dodir /var/spool/notes
	keepdir /var/spool/notes
	fowners notes:notes /var/spool/notes
	fperms 775 /var/spool/notes
	dodir /var/spool/notes/.sequencer
	fowners notes:notes /var/spool/notes/.sequencer
	fperms 770 /var/spool/notes/.sequencer
}

pkg_postinst() {
	if test ! -f /var/spool/notes/.SEQ ; then echo "1" > /var/spool/notes/.SEQ ; fi
	chown notes:notes /var/spool/notes/.SEQ
	chmod 660 /var/spool/notes/.SEQ

	einfo "Please consider contributing to the future development of Newts"
	einfo "software by filling out and returning the included SURVEY, located"
	einfo "at /usr/share/doc/${P}/SURVEY.gz."
}
