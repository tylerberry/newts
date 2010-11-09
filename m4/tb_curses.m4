dnl @synopsis MP_WITH_CURSES
dnl
dnl Detect SysV compatible curses, such as ncurses.
dnl
dnl Defines HAVE_CURSES_H or HAVE_NCURSES_H if curses is found.
dnl CURSES_LIB is also set with the required libary, but is not appended
dnl to LIBS automatically. If no working curses libary is found CURSES_LIB
dnl will be left blank.
dnl
dnl This macro adds the option "--with-ncurses" to configure which can
dnl force the use of ncurses or nothing at all.
dnl
dnl @version $Id: tb_curses.m4,v 1.1 2003/08/01 23:51:40 aidan Exp $
dnl @author Mark Pulford <mark@kyne.com.au>
dnl
dnl Modified to always prefer ncurses to curses.

AC_DEFUN([tb_CURSES],
   [tb_save_LIBS="$LIBS"
   CURSES_LIB=""
   AC_CACHE_CHECK([for working ncurses], [tb_cv_ncurses],
     [LIBS="$mp_save_LIBS -lncurses"
      AC_TRY_LINK(
        [#include <ncurses.h>],
        [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
        tb_cv_ncurses=yes, tb_cv_ncurses=no)])
   if test "$tb_cv_ncurses" = yes
   then
       AC_DEFINE(HAVE_NCURSES_H, 1,
                 [Define to 1 if you have the <ncurses.h> header file.])
       CURSES_LIB="-lncurses"
   fi
   if test ! "$CURSES_LIB"
   then
     AC_CACHE_CHECK([for working curses], [tb_cv_curses],
       [LIBS="$LIBS -lcurses"
        AC_TRY_LINK(
          [#include <curses.h>],
          [chtype a; int b=A_STANDOUT, c=KEY_LEFT; initscr(); ],
          tb_cv_curses=yes, tb_cv_curses=no)])
     if test "$tb_cv_curses" = yes
     then
       AC_DEFINE(HAVE_CURSES_H, 1,
                 [Define to 1 if you have the <curses.h> header file])
       CURSES_LIB="-lcurses"
     fi
   fi
   AC_SUBST(CURSES_LIB)
   LIBS="$tb_save_LIBS"
])dnl
