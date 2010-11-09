/*
 * misc.c - miscellaneous functions to make my life easier
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based in part on dropt.c and misc.c from the UIUC notes distribution by Ray
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

#include "newts/list.h"
#include "newts/newts.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

extern char *shell;

inline int list_convert (char *buf, int *p);

short allowed (struct notesfile *nf, short mode);
inline int list_parse (char *buf, int *p, int *first, int *last);

/* This should be pretty obvious. */

const char *mnames[12] = {
  N_("Jan"), N_("Feb"), N_("Mar"), N_("Apr"), N_("May"), N_("Jun"),
  N_("Jul"), N_("Aug"), N_("Sep"), N_("Oct"), N_("Nov"), N_("Dec")
};

/* allowed - check to see if we're allowed to do something.
 *
 * Please note that these checks are also performed on the server-side.  We
 * check in the client to avoid having to ask the server at all.
 */

short
allowed (struct notesfile *nf, short mode)
{
  switch (mode)
    {
    case REPLY:
      return nf->perms & (REPLY + WRITE + DIRECTOR + OWNER);

    case READ:
      return nf->perms & (READ + DIRECTOR + OWNER);

    case WRITE:
      return nf->perms & (WRITE + DIRECTOR + OWNER);

    case DIRECTOR:
      return nf->perms & (DIRECTOR + OWNER);

    case OWNER:
      return nf->perms & OWNER;
    }

  return 0;
}

/* list_parse - given a numeric list, possible including hyphen-delimited
 * ranges, parse the first space-separated part out into FIRST and LAST; store
 * the position in P.  This function is intended to be called iteratively.
 *
 * Returns: 2 if we found a range, 1 if we found an atomic number.
 */

inline int
list_parse (char *buf, int *p, int *first, int *last)
{
  if ((buf[*p] < '0' || buf[*p] > '9') && buf[*p] != ' ')
    return 0;
  *first = list_convert (buf, p);
  *last = *first;
  if (buf[*p] == '-')
    {
      ++*p;
      *last = list_convert (buf, p);
      ++*p;
      return 2;
    }
  else
    {
      if (buf[*p] != '\0')
        ++*p;
      return 1;
    }
}

/* list_convert - convert a text representation of a number into an int, and
 * return that value.
 */

inline int
list_convert (char *buf, int *p)
{
  int i = 0;

  while (buf[*p] == ' ')
    ++*p;
  while (buf[*p] >= '0' && buf[*p] <= '9')
    {
      i = 10 * i + buf[*p] - '0';
      ++*p;
    }
  return i;
}

/* sprint_time - construct a string representation of TIME and store it in
 * BUFFER.
 */

void
sprint_time (char *buffer, struct tm *time)
{
  char *m = "am";
  int h, i, j;
  h = time->tm_hour;
  if (h >= 12)
    m = "pm";
  if (h == 0)
    h = 12;
  if (h > 12)
    h -= 12;
  i = time->tm_min / 10;
  j = time->tm_min % 10;
  sprintf (buffer, "%2d:%d%d %2s  %3s %2d, %4d", h, i, j, m,
           _(mnames[time->tm_mon]), time->tm_mday, time->tm_year + 1900);
}
