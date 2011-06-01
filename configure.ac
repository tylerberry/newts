# Autoconf configure script for Newts.                         -*- Autoconf -*-

AC_INIT([newts], [0.15], [bug-newts@thoughtlocker.net])
AC_PREREQ([2.60])
AC_CONFIG_AUX_DIR([build-aux])

echo \
"
Configuring Newts 0.15
----------------------
"
AM_INIT_AUTOMAKE([dist-bzip2])
AM_CONFIG_HEADER([config.h])

# Options for configure
AM_WITH_DMALLOC

AC_ARG_WITH([default-editor],
   AC_HELP_STRING([--with-default-editor=EDITOR],
                  [which editor to use by default for the client @<:@emacs xemacs vim vi nano pico vile joe jed ed@:>@]),
                  [default_editor=$withval], [default_editor="no"])

AC_ARG_WITH([default-mailer],
   AC_HELP_STRING([--with-default-mailer=MAILER],
                  [which mailer to use by default for the client @<:@mutt pine elm nail mailx Mail mail@:>@]),
                  [default_mailer=$withval], [default_mailer="no"])

AC_ARG_WITH([default-pager],
   AC_HELP_STRING([--with-default-pager=PAGER],
                  [which pager to use by default for the client @<:@more less most pg@:>@]),
                  [default_pager=$withval], [default_pager="no"])

AC_ARG_WITH([talk-program],
   AC_HELP_STRING([--with-talk-program=PROGRAM],
                  [which program to invoke to talk to another user @<:@ytalk talk com write@:>@]),
                  [talk_program=$withval], [talk_program="no"])

AC_ARG_WITH([notes-spool],
   AC_HELP_STRING([--with-notes-spool=DIR],
                  [which directory to use as the notes spool @<:@/var/spool/notes@:>@]),
                  [notes_spool_dir=$withval], [notes_spool_dir="/var/spool/notes"])
AC_DEFINE_UNQUOTED([SPOOL], ["$notes_spool_dir"], [ Where the notes spool is located. ])
AC_SUBST([SPOOL], [${notes_spool_dir}])

AC_ARG_WITH([notes-user],
   AC_HELP_STRING([--with-notes-user=USER],
	              [user in charge of the notes system @<:@notes@:>@]),
		  [notes_user=$withval], [notes_user="notes"])
AC_DEFINE_UNQUOTED([NOTES], ["$notes_user"], [ Who the owner of the notes system is. ])
AC_SUBST([NOTES], [${notes_user}])

AC_ARG_WITH([notes-group],
   AC_HELP_STRING([--with-notes-group=GROUP],
	              [group that the notes user belongs to @<:@notes@:>@]),
		  [notes_group=$withval], [notes_group="notes"])
AC_SUBST([NOTESGROUP], [${notes_group}])

AC_ARG_WITH([anon-user],
   AC_HELP_STRING([--with-anon-user=USER],
                  [pseudo-user to represent anonymity @<:@nobody@:>@]),
          [anon_user=$withval], [anon_user="nobody"])
AC_DEFINE_UNQUOTED([ANON], ["$anon_user"], [ Pseudo-user to represent anonymity. ])
AC_SUBST([ANON], [${anon_user}])

AC_ARG_WITH([python],
   AC_HELP_STRING([--with-python=PATH],
                  [which python binary to use for bindings]),
          [enable_python=$withval], [enable_python="yes"])

echo \
"
Configuring the C compiler
--------------------------
"
AC_PROG_CC
dnl gl_EARLY calls AC_GNU_SOURCE, etc.
gl_EARLY
AM_PROG_CC_C_O
AC_PROG_GCC_TRADITIONAL
AC_PROG_CPP

echo \
"
Checking for programs and system services
-----------------------------------------
"
AC_PROG_INSTALL
AC_PROG_LN_S

# getdate.y only works with Bison. (For now.)
AC_PROG_YACC
if test "$YACC" != "bison -y"; then
    YACC="${am_missing_run}bison"
fi

AM_PROG_LEX

if test x"${default_editor}" = xno; then
    tb_DEFINE_PROGS([EDITOR], [emacs xemacs vim vi nano pico vile joe jed ed],
                    [none])
else
    AC_DEFINE_UNQUOTED([EDITOR], ["$default_editor"],
                       [ Which editor to use by default. ])
fi
if test x"${EDITOR}" = xnone; then
    AC_MSG_ERROR(["cannot locate a default editor."])
fi

if test x"${default_mailer}" = xno; then
    tb_DEFINE_PROGS([MAILER], [mutt pine elm nail mailx Mail mail], [none])
else
    AC_DEFINE_UNQUOTED([MAILER], ["$default_mailer"],
                       [ Which mailer to use by default. ])
fi
if test x"${MAILER}" = xnone; then
    AC_MSG_WARN(["cannot locate a default mailer."])
    AC_DEFINE_UNQUOTED([MAILER], ["/bin/false"])
fi

if test x"${default_pager}" = xno; then
    tb_DEFINE_PROGS([PAGER], [more less most pg], [none])
else
    AC_DEFINE_UNQUOTED([PAGER], ["$default_pager"],
                       [ Which pager to use by default. ])
fi
if test x"${PAGER}" = xnone; then
    AC_MSG_WARN(["cannot locate a default pager."])
    AC_DEFINE_UNQUOTED([PAGER], ["/bin/false"])
fi

if test x"${talk_program}" = xno; then
    tb_DEFINE_PROGS([TALK], [ytalk talk com write], [none])
else
    AC_DEFINE_UNQUOTED([TALK], ["$talk_program"],
                       [ Which 'talk' program to use. ])
fi
if test x"${TALK}" = xnone; then
    AC_MSG_WARN("cannot locate a user-to-user 'talk' program.")
    AC_DEFINE_UNQUOTED(TALK,"/bin/false")
fi

AM_PATH_PYTHON

if test ! x"${enable_python}" = xno; then
    if test x"$PYTHON" = xnone; then
        enable_python=no
    else
        AC_MSG_CHECKING([for python include directory])
        python_includedir=`$PYTHON -c 'from distutils.sysconfig import get_python_inc
print get_python_inc()'`
        if test -n "${python_includedir}"; then
            AC_MSG_RESULT([$python_includedir])
            AC_SUBST([PYTHONINCLUDE], [-I$python_includedir])
            AC_CHECK_FILE([$python_includedir/Python.h], [enable_python=yes],
                          [enable_python=no])
        else
            AC_MSG_RESULT([not found])
            enable_python=no
        fi
    fi
fi

AM_CONDITIONAL([HAVE_PYTHON], [test ! x$enable_python = xno])

AH_TEMPLATE([CAT], [ Where the 'cat' program is located. ])
tb_DEFINE_PROGS([CAT], [cat], [AC_MSG_ERROR([could not find cat])])

AH_TEMPLATE([PR], [ Where the 'pr' program is located. ])
tb_DEFINE_PROGS([PR], [pr], [AC_MSG_ERROR([could not find pr])])

AC_SYS_LARGEFILE
AC_SYS_LONG_FILE_NAMES

echo \
"
Configuring GNU libtool
-----------------------
"
AC_LIBTOOL_DLOPEN
AC_LIBLTDL_CONVENIENCE
AC_PROG_LIBTOOL
AC_SUBST([INCLTDL])
AC_SUBST([LIBLTDL])
AC_CONFIG_SUBDIRS([libltdl])

echo \
"
Checking for libraries
----------------------
"
tb_CURSES

echo \
"
Checking for header files
-------------------------
"
AC_HEADER_STDC
AC_HEADER_DIRENT
AC_HEADER_STAT
AC_HEADER_SYS_WAIT
AC_HEADER_TIME
AC_CHECK_HEADERS([dirent.h fcntl.h float.h getopt.h glob.h grp.h \
    langinfo.h libintl.h netdb.h netinet/in.h pwd.h sgtty.h stdbool.h \
    strings.h sys/ioctl.h sys/param.h sys/socket.h sys/stat.h sys/time.h \
    sys/types.h termio.h termios.h unistd.h wchar.h wctype.h])

echo \
"
Checking language and compiler characteristics
----------------------------------------------
"
AC_C_CONST
AC_C_INLINE
AC_STRUCT_TIMEZONE
AC_STRUCT_TM
AC_TYPE_MODE_T
AC_TYPE_OFF_T
AC_TYPE_PID_T
AC_CHECK_TYPES([ptrdiff_t])
AC_TYPE_SIGNAL
AC_TYPE_SIZE_T
AC_TYPE_UID_T

echo \
"
Checking for C library functions and syscalls
---------------------------------------------
"
AC_FUNC_CLOSEDIR_VOID
AC_CHECK_FUNCS([endpwent fdatasync])
AC_FUNC_FORK
AC_CHECK_FUNCS([gethostbyname getpeereid index])
adl_FUNC_MKDIR
AC_FUNC_MMAP
AC_CHECK_FUNCS([rewinddir rindex select socket strchr strrchr])

echo \
"
Configuring Gnulib
------------------
"
gl_INIT

echo \
"
Configuring GNU gettext
-----------------------
"
AM_GNU_GETTEXT([external], [need-ngettext])
AM_GNU_GETTEXT_VERSION([0.16.1])

# This section generates include/newts/config.h, which has site-specific
# includes and declarations.
AC_CONFIG_COMMANDS([include/newts/config.h],[[
    if test -n "$CONFIG_FILES" && test -n "$CONFIG_HEADERS"; then
        # If both these vars are non-empty, then config.status wasn't run by
        # automake rules (which always set one or the other to empty).
        CONFIG_OTHER=${CONFIG_OTHER-include/newts/config.h}
    fi
    case "$CONFIG_OTHER" in
    *include/newts/config.h*)

        outfile=include/newts/config.h
        stampfile=include/newts/stamp-config
        tmpfile=${outfile}T
        dirname="sed ',^.*/,,g'"

        echo config.status: creating $outfile
        cat > $tmpfile << EOF
/*  -*- Mode: C -*-
 * -------------------------------------------------------------------------
 * DO NOT EDIT THIS FILE!  It has been automatically generated for the host:
 *   `(hostname --fqdn || uname -n) 2>/dev/null | sed 1q`
 * -------------------------------------------------------------------------
 */

#ifndef NEWTS_CONFIG_H
#define NEWTS_CONFIG_H

EOF

        if test x$ac_cv_header_time = xyes; then
            echo '#include <sys/time.h>' >> $tmpfile
            echo '#include <time.h>' >> $tmpfile
        else
            if test x$ac_cv_header_sys_time_h = xyes; then
                echo '#include <sys/time.h>' >> $tmpfile
            else
                echo '#include <time.h>' >> $tmpfile
            fi
        fi
        if test x$ac_cv_header_sys_types_h = xyes; then
            echo '#include <sys/types.h>' >> $tmpfile
        fi
        if test x$ac_cv_header_unistd_h = xyes; then
            echo '#include <unistd.h>' >> $tmpfile
        fi

        cat >> $tmpfile << EOF

#endif /* not NEWTS_CONFIG_H */
EOF

        if cmp -s $tmpfile $outfile; then
            echo config.status: $outfile is unchanged
            rm -f $tmpfile
        else
            mv $tmpfile $outfile
            echo timestamp > $stampfile
        fi
    ;;
    esac
]],[[
    ac_cv_header_sys_time_h=$ac_cv_header_sys_time_h
    ac_cv_header_sys_types_h=$ac_cv_header_sys_types_h
    ac_cv_header_time=$ac_cv_header_time
    ac_cv_header_unistd_h=$ac_cv_header_unistd_h
]])

# This section generates libnewts/version.c, which creates a source file
# holding the current SVN revision number.
AC_CONFIG_COMMANDS([libnewts/version.c],[[
    if test -n "$CONFIG_FILES" && test -n "$CONFIG_HEADERS"; then
        # If both these vars are non-empty, then config.status wasn't run by
        # automake rules (which always set one or the other to empty).
        CONFIG_OTHER=${CONFIG_OTHER-libnewts/version.c}
    fi
    case "$CONFIG_OTHER" in
    *libnewts/version.c*)

        outfile=libnewts/version.c
        tmpfile=${outfile}T
        dirname="sed ',^.*/,,g'"

        echo config.status: creating $outfile
        cat > $tmpfile << EOF
/*  -*- Mode: C -*-
 * -------------------------------------------------------------------------
 *   Maintainers: To regenerate this file, type 'make version'.
 * -------------------------------------------------------------------------
 */

EOF

        if type -pa svnversion > /dev/null ; then
            svn_version=`svnversion`
            if test x$svn_version = "xexported" ; then
                if test -f $outfile ; then
                    echo config.status: $outfile is unchanged
                    rm -f $tmpfile
                else
                    echo "char *newts_revision = \"unavailable\";" >> $tmpfile
                    mv $tmpfile $outfile
                fi
            else
                echo "char *newts_revision = \"${svn_version}\";" >> $tmpfile

                if cmp -s $tmpfile $outfile ; then
                    echo config.status: $outfile is unchanged
                    rm -f $tmpfile
                else
                    mv $tmpfile $outfile
                fi
            fi
        else
            if test -f $outfile ; then
                echo config.status: $outfile is unchanged
                rm -f $tmpfile
            else
                echo "char *newts_revision = \"unavailable\";" >> $tmpfile
                mv $tmpfile $outfile
            fi
        fi
    ;;
    esac
]],[[]])

echo \
"
Generating Makefiles and support files
--------------------------------------
"
# Output files
AC_CONFIG_FILES([Makefile \
    backends/Makefile \
    backends/uiuc/Makefile \
    bindings/Makefile \
    bindings/python/Makefile \
    clients/Makefile \
    clients/frontends/Makefile \
    clients/libnewtsutil/Makefile \
    clients/notes/Makefile \
    contrib/Makefile \
    doc/Makefile \
    gnulib/GNUmakefile \
    gnulib/Makefile \
    include/Makefile \
    include/newts/Makefile \
    lib/Makefile \
    libnewts/Makefile \
    libnewtsclient/Makefile \
    m4/Makefile \
    m4/gnulib/Makefile \
    po/Makefile.in \
    tests/Makefile \
    tests/check/Makefile])
AC_OUTPUT

if ! test x${enable_python} = xno; then
    pretty_enable_python=enabled
else
    pretty_enable_python=disabled
fi

echo \
"
-------------------------------------------------------------------------------
Newts configuration summary:

  Source directory:           ${srcdir}
  Compiler:                   ${CC}
  Compiler flags:             ${CFLAGS}
  Host system type:           ${host}
  Install prefix:             ${prefix}

  Notes spool directory:      ${notes_spool_dir}
  Notes user, group:          ${notes_user}, ${notes_group}
  Anonymous user:             ${anon_user}
  Default editor:             ${ac_cv_prog_EDITOR}
  Default mailer:             ${ac_cv_prog_MAILER}
  Default pager:              ${ac_cv_prog_PAGER}
  Default talk program:       ${ac_cv_prog_TALK}

  Python bindings:            ${pretty_enable_python}

  See config.h for detailed configuration information.

  If you are migrating from UIUC notesfiles to Newts, you may need to
  edit include/newts/uiuc-compatibility.h if you customized struct.h or
  param.h for your UIUC notesfiles installation.
-------------------------------------------------------------------------------
"
