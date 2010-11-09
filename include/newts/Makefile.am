MAINTAINERCLEANFILES = Makefile.in config.h stamp-config

pkginclude_HEADERS = access.h author.h config.h connection.h enums.h error.h \
	list.h memory.h newts.h nfref.h note.h notesfile.h search.h sequencer.h \
	stats.h uiuc.h uiuc-compatibility.h util.h version.h

config.h: stamp-config
stamp-config: $(top_builddir)/config.status
	cd $(top_builddir) && \
	CONFIG_FILES= CONFIG_HEADERS= CONFIG_OTHER=include/newts/config.h \
	$(SHELL) ./config.status
	echo timestamp > $@

dist-hook:
	rm -f $(distdir)/config.h
