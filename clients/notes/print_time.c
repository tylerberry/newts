/*
 * print_time.c - print a date, supplied as a struct tm *
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004 Tyler Berry.
 *
 * Based in part on prdate in misc.c from the UIUC notes distribution by Ray
 * Essick and Rob Kolstad.  Any work derived from this source code is required
 * to retain this notice.
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

#include "vasprintf.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

extern const char *mnames[12];

/* print_time - print a time notes-style to the ncurses screen. */

void printw_time (struct tm *tm);

void
printw_time (struct tm *tm)
{
  char *m = "am";
  int h, i, j;
  h = tm->tm_hour;
  if (h >= 12)
    m = "pm";
  if (h == 0)
    h = 12;
  if (h > 12)
    h -= 12;
  i = tm->tm_min / 10;
  j = tm->tm_min % 10;
  printw ("%2d:%d%d %2s  %3s %2d, %4d", h, i, j, m, _(mnames[tm->tm_mon]),
          tm->tm_mday, tm->tm_year + 1900);
}

/* fprint_time - print a time notes-style to the given FILE stream. */

void fprint_time (FILE *file, struct tm *tm);

void
fprint_time (FILE *file, struct tm *tm)
{
  char *m = "am";
  int h, i, j;
  h = tm->tm_hour;
  if (h >= 12)
    m = "pm";
  if (h == 0)
    h = 12;
  if (h > 12)
    h -= 12;
  i = tm->tm_min / 10;
  j = tm->tm_min % 10;
  fprintf (file, "%d:%d%d %s  %s %d, %d", h, i, j, m,
           _(mnames[tm->tm_mon]), tm->tm_mday, tm->tm_year + 1900);
}

/* time_string - return a string containing a "12:00 am on Jul 13, 2003"-esque
 * string.
 */

char *time_string (struct tm *tm);

char *
time_string (struct tm *tm)
{
  char *date;
  char *m = "am";
  int h, i, j;
  h = tm->tm_hour;
  if (h >= 12)
    m = "pm";
  if (h == 0)
    h = 12;
  if (h > 12)
    h -= 12;
  i = tm->tm_min / 10;
  j = tm->tm_min % 10;
  asprintf (&date, "%d:%d%d %s on %s %d, %d", h, i, j, m,
            _(mnames[tm->tm_mon]), tm->tm_mday, tm->tm_year + 1900);
  return date;
}
