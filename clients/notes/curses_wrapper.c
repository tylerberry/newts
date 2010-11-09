/*
 * curses-wrapper.c - abstract wrapper around the curses library
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry
 *
 * Newts is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Newts is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Newts; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if STDC_HEADERS || __STDC__
# include <stdarg.h>
# define VA_START(ap, f) va_start (ap, f)
#elif HAVE_VARARGS_H
# include <varargs.h>
# define VA_START(ap, f) va_start (ap)
#else
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif

/* Has curses been entered? */
static bool in_curses = false;
static bool initialized = false;

inline bool
currently_in_curses (void)
{
  return in_curses;
}

inline bool
entered_curses (void)
{
  return initialized;
}

void
ensure_curses (void)
{
  if (!initialized)
    {
      initscr ();
      keypad (stdscr, TRUE);
      nonl ();
      cbreak ();
      noecho ();

      in_curses = true;
      initialized = true;
    }
}

void
exit_curses (void)
{
  if (initialized)
    {
      def_prog_mode ();
      endwin ();
      in_curses = false;
    }
}

void
reenter_curses (void)
{
  if (initialized)
    {
      reset_prog_mode ();
      refresh ();
      in_curses = true;
    }
  else
    ensure_curses ();
}

void
#if STDC_HEADERS
status_message (int column, const char *message, ...)
#else
status_message (column, message, va_alist)
     int column;
     const char *message;
     va_dcl
#endif
{
#ifdef VA_START
  va_list ap;
#endif

  move (LINES - 1, column);
  clrtoeol ();

#ifdef VA_START
  VA_START (ap, message);
  vwprintw (stdscr, message, ap);
  va_end (ap);
#else
  printw (message, va_alist);
#endif

  refresh ();
}
