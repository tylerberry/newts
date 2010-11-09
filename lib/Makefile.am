MAINTAINERCLEANFILES = Makefile.in

INCLUDES = -I$(top_srcdir)/include -I$(top_srcdir)/gnulib

noinst_LTLIBRARIES   = libcommon.la
libcommon_la_SOURCES = getpeereid.c which.c
libcommon_la_LIBADD  = @LIB_CLOCK_GETTIME@

noinst_HEADERS = which.h
