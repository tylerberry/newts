# DO NOT EDIT! GENERATED AUTOMATICALLY!
# Copyright (C) 2002-2011 Free Software Foundation, Inc.
#
# This file is free software, distributed under the terms of the GNU
# General Public License.  As a special exception to the GNU General
# Public License, this file may be distributed as part of a program
# that contains a configuration script generated by Autoconf, under
# the same distribution terms as the rest of that program.
#
# Generated by gnulib-tool.
#
# This file represents the compiled summary of the specification in
# gnulib-cache.m4. It lists the computed macro invocations that need
# to be invoked from configure.ac.
# In projects that use version control, this file can be treated like
# other built files.


# This macro should be invoked from ./configure.ac, in the section
# "Checks for programs", right after AC_PROG_CC, and certainly before
# any checks for libraries, header files, types and library functions.
AC_DEFUN([gl_EARLY],
[
  m4_pattern_forbid([^gl_[A-Z]])dnl the gnulib macro namespace
  m4_pattern_allow([^gl_ES$])dnl a valid locale name
  m4_pattern_allow([^gl_LIBOBJS$])dnl a variable
  m4_pattern_allow([^gl_LTLIBOBJS$])dnl a variable
  AC_REQUIRE([AC_PROG_RANLIB])
  # Code from module alignof:
  # Code from module alloca-opt:
  # Code from module arg-nonnull:
  # Code from module btowc:
  # Code from module c++defs:
  # Code from module c-ctype:
  # Code from module clock-time:
  # Code from module configmake:
  # Code from module dirname:
  # Code from module dirname-lgpl:
  # Code from module dosname:
  # Code from module double-slash-root:
  # Code from module environ:
  # Code from module errno:
  # Code from module exitfail:
  # Code from module extensions:
  AC_REQUIRE([gl_USE_SYSTEM_EXTENSIONS])
  # Code from module fd-hook:
  # Code from module float:
  # Code from module getdelim:
  # Code from module getgroups:
  # Code from module gethostname:
  # Code from module getline:
  # Code from module gettext:
  # Code from module gettext-h:
  # Code from module gettime:
  # Code from module gettimeofday:
  # Code from module havelib:
  # Code from module include_next:
  # Code from module intprops:
  # Code from module langinfo:
  # Code from module localcharset:
  # Code from module malloc:
  # Code from module malloc-gnu:
  # Code from module malloc-posix:
  # Code from module malloca:
  # Code from module mbrtowc:
  # Code from module mbsinit:
  # Code from module mbtowc:
  # Code from module memchr:
  # Code from module mktime:
  # Code from module multiarch:
  # Code from module nl_langinfo:
  # Code from module parse-datetime:
  # Code from module readutmp:
  # Code from module realloc:
  # Code from module realloc-gnu:
  # Code from module realloc-posix:
  # Code from module regex:
  # Code from module rpmatch:
  # Code from module setenv:
  # Code from module size_max:
  # Code from module socketlib:
  # Code from module sockets:
  # Code from module socklen:
  # Code from module ssize_t:
  # Code from module stdbool:
  # Code from module stddef:
  # Code from module stdint:
  # Code from module stdio:
  # Code from module stdlib:
  # Code from module strcase:
  # Code from module streq:
  # Code from module string:
  # Code from module strings:
  # Code from module strndup:
  # Code from module strnlen:
  # Code from module strstr:
  # Code from module strstr-simple:
  # Code from module strtok_r:
  # Code from module sys_socket:
  # Code from module sys_time:
  # Code from module sys_uio:
  # Code from module time:
  # Code from module time_r:
  # Code from module timespec:
  # Code from module unistd:
  # Code from module unsetenv:
  # Code from module vasnprintf:
  # Code from module vasprintf:
  # Code from module verify:
  # Code from module warn-on-use:
  # Code from module wchar:
  # Code from module wcrtomb:
  # Code from module wctype-h:
  # Code from module xgethostname:
  # Code from module xsize:
  # Code from module xstrndup:
  # Code from module yesno:
])

# This macro should be invoked from ./configure.ac, in the section
# "Check for header files, types and library functions".
AC_DEFUN([gl_INIT],
[
  AM_CONDITIONAL([GL_COND_LIBTOOL], [true])
  gl_cond_libtool=true
  gl_m4_base='m4/gnulib'
  m4_pushdef([AC_LIBOBJ], m4_defn([gl_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gl_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gl_LIBSOURCES]))
  m4_pushdef([gl_LIBSOURCES_LIST], [])
  m4_pushdef([gl_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='gnulib'
gl_FUNC_ALLOCA
gl_FUNC_BTOWC
gl_WCHAR_MODULE_INDICATOR([btowc])
gl_CLOCK_TIME
gl_CONFIGMAKE_PREP
gl_DIRNAME
gl_MODULE_INDICATOR([dirname])
gl_DIRNAME_LGPL
gl_DOUBLE_SLASH_ROOT
gl_ENVIRON
gl_UNISTD_MODULE_INDICATOR([environ])
gl_HEADER_ERRNO_H
gl_FLOAT_H
gl_FUNC_GETDELIM
gl_STDIO_MODULE_INDICATOR([getdelim])
gl_FUNC_GETGROUPS
gl_UNISTD_MODULE_INDICATOR([getgroups])
gl_FUNC_GETHOSTNAME
gl_UNISTD_MODULE_INDICATOR([gethostname])
gl_FUNC_GETLINE
gl_STDIO_MODULE_INDICATOR([getline])
dnl you must add AM_GNU_GETTEXT([external]) or similar to configure.ac.
AM_GNU_GETTEXT_VERSION([0.18.1])
AC_SUBST([LIBINTL])
AC_SUBST([LTLIBINTL])
gl_GETTIME
gl_FUNC_GETTIMEOFDAY
gl_SYS_TIME_MODULE_INDICATOR([gettimeofday])
gl_LANGINFO_H
gl_LOCALCHARSET
LOCALCHARSET_TESTS_ENVIRONMENT="CHARSETALIASDIR=\"\$(top_builddir)/$gl_source_base\""
AC_SUBST([LOCALCHARSET_TESTS_ENVIRONMENT])
gl_FUNC_MALLOC_GNU
gl_MODULE_INDICATOR([malloc-gnu])
gl_FUNC_MALLOC_POSIX
gl_STDLIB_MODULE_INDICATOR([malloc-posix])
gl_MALLOCA
gl_FUNC_MBRTOWC
gl_WCHAR_MODULE_INDICATOR([mbrtowc])
gl_FUNC_MBSINIT
gl_WCHAR_MODULE_INDICATOR([mbsinit])
gl_FUNC_MBTOWC
gl_STDLIB_MODULE_INDICATOR([mbtowc])
gl_FUNC_MEMCHR
gl_STRING_MODULE_INDICATOR([memchr])
gl_FUNC_MKTIME
gl_TIME_MODULE_INDICATOR([mktime])
gl_MULTIARCH
gl_FUNC_NL_LANGINFO
gl_LANGINFO_MODULE_INDICATOR([nl_langinfo])
gl_PARSE_DATETIME
gl_READUTMP
gl_FUNC_REALLOC_GNU
gl_MODULE_INDICATOR([realloc-gnu])
gl_FUNC_REALLOC_POSIX
gl_STDLIB_MODULE_INDICATOR([realloc-posix])
gl_REGEX
gl_FUNC_RPMATCH
gl_STDLIB_MODULE_INDICATOR([rpmatch])
gl_FUNC_SETENV
gl_STDLIB_MODULE_INDICATOR([setenv])
gl_SIZE_MAX
gl_SOCKETLIB
gl_SOCKETS
gl_TYPE_SOCKLEN_T
gt_TYPE_SSIZE_T
AM_STDBOOL_H
gl_STDDEF_H
gl_STDINT_H
gl_STDIO_H
gl_STDLIB_H
gl_STRCASE
gl_HEADER_STRING_H
gl_HEADER_STRINGS_H
gl_FUNC_STRNDUP
gl_STRING_MODULE_INDICATOR([strndup])
gl_FUNC_STRNLEN
gl_STRING_MODULE_INDICATOR([strnlen])
gl_FUNC_STRSTR
gl_FUNC_STRSTR_SIMPLE
gl_STRING_MODULE_INDICATOR([strstr])
gl_FUNC_STRTOK_R
gl_STRING_MODULE_INDICATOR([strtok_r])
gl_HEADER_SYS_SOCKET
AC_PROG_MKDIR_P
gl_HEADER_SYS_TIME_H
AC_PROG_MKDIR_P
gl_HEADER_SYS_UIO
AC_PROG_MKDIR_P
gl_HEADER_TIME_H
gl_TIME_R
gl_TIME_MODULE_INDICATOR([time_r])
gl_TIMESPEC
gl_UNISTD_H
gl_FUNC_UNSETENV
gl_STDLIB_MODULE_INDICATOR([unsetenv])
gl_FUNC_VASNPRINTF
gl_FUNC_VASPRINTF
gl_STDIO_MODULE_INDICATOR([vasprintf])
m4_ifdef([AM_XGETTEXT_OPTION],
  [AM_][XGETTEXT_OPTION([--flag=asprintf:2:c-format])
   AM_][XGETTEXT_OPTION([--flag=vasprintf:2:c-format])])
gl_WCHAR_H
gl_FUNC_WCRTOMB
gl_WCHAR_MODULE_INDICATOR([wcrtomb])
gl_WCTYPE_H
gl_XSIZE
gl_XSTRNDUP
gl_YESNO
  # End of code from modules
  m4_ifval(gl_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gl_LIBSOURCES_DIR])[ ||
      for gl_file in ]gl_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gl_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gl_LIBSOURCES_DIR])
  m4_popdef([gl_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gl_libobjs=
    gl_ltlibobjs=
    if test -n "$gl_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gl_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gl_libobjs="$gl_libobjs $i.$ac_objext"
        gl_ltlibobjs="$gl_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gl_LIBOBJS], [$gl_libobjs])
    AC_SUBST([gl_LTLIBOBJS], [$gl_ltlibobjs])
  ])
  gltests_libdeps=
  gltests_ltlibdeps=
  m4_pushdef([AC_LIBOBJ], m4_defn([gltests_LIBOBJ]))
  m4_pushdef([AC_REPLACE_FUNCS], m4_defn([gltests_REPLACE_FUNCS]))
  m4_pushdef([AC_LIBSOURCES], m4_defn([gltests_LIBSOURCES]))
  m4_pushdef([gltests_LIBSOURCES_LIST], [])
  m4_pushdef([gltests_LIBSOURCES_DIR], [])
  gl_COMMON
  gl_source_base='tests'
changequote(,)dnl
  gltests_WITNESS=IN_`echo "${PACKAGE-$PACKAGE_TARNAME}" | LC_ALL=C tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ | LC_ALL=C sed -e 's/[^A-Z0-9_]/_/g'`_GNULIB_TESTS
changequote([, ])dnl
  AC_SUBST([gltests_WITNESS])
  gl_module_indicator_condition=$gltests_WITNESS
  m4_pushdef([gl_MODULE_INDICATOR_CONDITION], [$gl_module_indicator_condition])
  m4_popdef([gl_MODULE_INDICATOR_CONDITION])
  m4_ifval(gltests_LIBSOURCES_LIST, [
    m4_syscmd([test ! -d ]m4_defn([gltests_LIBSOURCES_DIR])[ ||
      for gl_file in ]gltests_LIBSOURCES_LIST[ ; do
        if test ! -r ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file ; then
          echo "missing file ]m4_defn([gltests_LIBSOURCES_DIR])[/$gl_file" >&2
          exit 1
        fi
      done])dnl
      m4_if(m4_sysval, [0], [],
        [AC_FATAL([expected source file, required through AC_LIBSOURCES, not found])])
  ])
  m4_popdef([gltests_LIBSOURCES_DIR])
  m4_popdef([gltests_LIBSOURCES_LIST])
  m4_popdef([AC_LIBSOURCES])
  m4_popdef([AC_REPLACE_FUNCS])
  m4_popdef([AC_LIBOBJ])
  AC_CONFIG_COMMANDS_PRE([
    gltests_libobjs=
    gltests_ltlibobjs=
    if test -n "$gltests_LIBOBJS"; then
      # Remove the extension.
      sed_drop_objext='s/\.o$//;s/\.obj$//'
      for i in `for i in $gltests_LIBOBJS; do echo "$i"; done | sed -e "$sed_drop_objext" | sort | uniq`; do
        gltests_libobjs="$gltests_libobjs $i.$ac_objext"
        gltests_ltlibobjs="$gltests_ltlibobjs $i.lo"
      done
    fi
    AC_SUBST([gltests_LIBOBJS], [$gltests_libobjs])
    AC_SUBST([gltests_LTLIBOBJS], [$gltests_ltlibobjs])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_LIBOBJ], [
  AS_LITERAL_IF([$1], [gl_LIBSOURCES([$1.c])])dnl
  gl_LIBOBJS="$gl_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gl_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gl_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gl_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gl_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gl_LIBSOURCES_DIR], [gnulib])
      m4_append([gl_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# Like AC_LIBOBJ, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_LIBOBJ], [
  AS_LITERAL_IF([$1], [gltests_LIBSOURCES([$1.c])])dnl
  gltests_LIBOBJS="$gltests_LIBOBJS $1.$ac_objext"
])

# Like AC_REPLACE_FUNCS, except that the module name goes
# into gltests_LIBOBJS instead of into LIBOBJS.
AC_DEFUN([gltests_REPLACE_FUNCS], [
  m4_foreach_w([gl_NAME], [$1], [AC_LIBSOURCES(gl_NAME[.c])])dnl
  AC_CHECK_FUNCS([$1], , [gltests_LIBOBJ($ac_func)])
])

# Like AC_LIBSOURCES, except the directory where the source file is
# expected is derived from the gnulib-tool parameterization,
# and alloca is special cased (for the alloca-opt module).
# We could also entirely rely on EXTRA_lib..._SOURCES.
AC_DEFUN([gltests_LIBSOURCES], [
  m4_foreach([_gl_NAME], [$1], [
    m4_if(_gl_NAME, [alloca.c], [], [
      m4_define([gltests_LIBSOURCES_DIR], [tests])
      m4_append([gltests_LIBSOURCES_LIST], _gl_NAME, [ ])
    ])
  ])
])

# This macro records the list of files which have been installed by
# gnulib-tool and may be removed by future gnulib-tool invocations.
AC_DEFUN([gl_FILE_LIST], [
  build-aux/arg-nonnull.h
  build-aux/c++defs.h
  build-aux/config.rpath
  build-aux/warn-on-use.h
  doc/parse-datetime.texi
  lib/alignof.h
  lib/alloca.in.h
  lib/asnprintf.c
  lib/asprintf.c
  lib/basename-lgpl.c
  lib/basename.c
  lib/btowc.c
  lib/c-ctype.c
  lib/c-ctype.h
  lib/config.charset
  lib/dirname-lgpl.c
  lib/dirname.c
  lib/dirname.h
  lib/dosname.h
  lib/errno.in.h
  lib/exitfail.c
  lib/exitfail.h
  lib/fd-hook.c
  lib/fd-hook.h
  lib/float+.h
  lib/float.in.h
  lib/getdelim.c
  lib/getgroups.c
  lib/gethostname.c
  lib/getline.c
  lib/gettext.h
  lib/gettime.c
  lib/gettimeofday.c
  lib/intprops.h
  lib/langinfo.in.h
  lib/localcharset.c
  lib/localcharset.h
  lib/malloc.c
  lib/malloca.c
  lib/malloca.h
  lib/malloca.valgrind
  lib/mbrtowc.c
  lib/mbsinit.c
  lib/mbtowc-impl.h
  lib/mbtowc.c
  lib/memchr.c
  lib/memchr.valgrind
  lib/mktime-internal.h
  lib/mktime.c
  lib/nl_langinfo.c
  lib/parse-datetime.h
  lib/parse-datetime.y
  lib/printf-args.c
  lib/printf-args.h
  lib/printf-parse.c
  lib/printf-parse.h
  lib/readutmp.c
  lib/readutmp.h
  lib/realloc.c
  lib/ref-add.sin
  lib/ref-del.sin
  lib/regcomp.c
  lib/regex.c
  lib/regex.h
  lib/regex_internal.c
  lib/regex_internal.h
  lib/regexec.c
  lib/rpmatch.c
  lib/setenv.c
  lib/size_max.h
  lib/sockets.c
  lib/sockets.h
  lib/stdbool.in.h
  lib/stddef.in.h
  lib/stdint.in.h
  lib/stdio.in.h
  lib/stdlib.in.h
  lib/str-two-way.h
  lib/strcasecmp.c
  lib/streq.h
  lib/string.in.h
  lib/strings.in.h
  lib/stripslash.c
  lib/strncasecmp.c
  lib/strndup.c
  lib/strnlen.c
  lib/strstr.c
  lib/strtok_r.c
  lib/sys_socket.in.h
  lib/sys_time.in.h
  lib/sys_uio.in.h
  lib/time.in.h
  lib/time_r.c
  lib/timespec.h
  lib/unistd.in.h
  lib/unsetenv.c
  lib/vasnprintf.c
  lib/vasnprintf.h
  lib/vasprintf.c
  lib/verify.h
  lib/w32sock.h
  lib/wchar.in.h
  lib/wcrtomb.c
  lib/wctype.in.h
  lib/xgethostname.c
  lib/xgethostname.h
  lib/xsize.h
  lib/xstrndup.c
  lib/xstrndup.h
  lib/yesno.c
  lib/yesno.h
  m4/00gnulib.m4
  m4/alloca.m4
  m4/bison.m4
  m4/btowc.m4
  m4/clock_time.m4
  m4/codeset.m4
  m4/configmake.m4
  m4/dirname.m4
  m4/double-slash-root.m4
  m4/eealloc.m4
  m4/environ.m4
  m4/errno_h.m4
  m4/extensions.m4
  m4/fcntl-o.m4
  m4/float_h.m4
  m4/getdelim.m4
  m4/getgroups.m4
  m4/gethostname.m4
  m4/getline.m4
  m4/gettext.m4
  m4/gettime.m4
  m4/gettimeofday.m4
  m4/glibc2.m4
  m4/glibc21.m4
  m4/gnulib-common.m4
  m4/iconv.m4
  m4/include_next.m4
  m4/intdiv0.m4
  m4/intl.m4
  m4/intldir.m4
  m4/intlmacosx.m4
  m4/intmax.m4
  m4/intmax_t.m4
  m4/inttypes-pri.m4
  m4/inttypes_h.m4
  m4/langinfo_h.m4
  m4/lcmessage.m4
  m4/lib-ld.m4
  m4/lib-link.m4
  m4/lib-prefix.m4
  m4/localcharset.m4
  m4/locale-fr.m4
  m4/locale-ja.m4
  m4/locale-zh.m4
  m4/lock.m4
  m4/longlong.m4
  m4/malloc.m4
  m4/malloca.m4
  m4/mbrtowc.m4
  m4/mbsinit.m4
  m4/mbstate_t.m4
  m4/mbtowc.m4
  m4/memchr.m4
  m4/mktime.m4
  m4/mmap-anon.m4
  m4/multiarch.m4
  m4/nl_langinfo.m4
  m4/nls.m4
  m4/parse-datetime.m4
  m4/po.m4
  m4/printf-posix.m4
  m4/printf.m4
  m4/progtest.m4
  m4/readutmp.m4
  m4/realloc.m4
  m4/regex.m4
  m4/rpmatch.m4
  m4/setenv.m4
  m4/size_max.m4
  m4/socketlib.m4
  m4/sockets.m4
  m4/socklen.m4
  m4/sockpfaf.m4
  m4/ssize_t.m4
  m4/stdbool.m4
  m4/stddef_h.m4
  m4/stdint.m4
  m4/stdint_h.m4
  m4/stdio_h.m4
  m4/stdlib_h.m4
  m4/strcase.m4
  m4/string_h.m4
  m4/strings_h.m4
  m4/strndup.m4
  m4/strnlen.m4
  m4/strstr.m4
  m4/strtok_r.m4
  m4/sys_socket_h.m4
  m4/sys_time_h.m4
  m4/sys_uio_h.m4
  m4/threadlib.m4
  m4/time_h.m4
  m4/time_r.m4
  m4/timespec.m4
  m4/tm_gmtoff.m4
  m4/uintmax_t.m4
  m4/unistd_h.m4
  m4/vasnprintf.m4
  m4/vasprintf.m4
  m4/visibility.m4
  m4/warn-on-use.m4
  m4/wchar_h.m4
  m4/wchar_t.m4
  m4/wcrtomb.m4
  m4/wctype_h.m4
  m4/wint_t.m4
  m4/xsize.m4
  m4/xstrndup.m4
  m4/yesno.m4
])
