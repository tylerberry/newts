# I'm not a coward, I've just never been tested.
# I'd like to think that if I was I would pass.
# Look at the tested and think there but for the grace go I.
# Might be a coward, I'm afraid of what I might find out.
#
# - "The Impression That I Get", The Mighty Mighty Bosstones

MAINTAINERCLEANFILES = Makefile.in

SUBDIRS = check .

INCLUDES = -I$(top_srcdir)/include

TESTS = access_tests nfref_tests
noinst_PROGRAMS = access_tests nfref_tests

access_tests_SOURCES = access_tests.c
access_tests_LDADD   = $(top_builddir)/libnewts/libnewts.la \
	check/libcheck.a

nfref_tests_SOURCES = nfref_tests.c
nfref_tests_LDADD   = $(top_builddir)/libnewts/libnewts.la \
	check/libcheck.a
