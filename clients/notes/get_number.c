/*
 * get_number.c - parse a number from the input stream
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based on getnum in miscio.c from the UIUC notes distribution by Ray Essick
 * and Rob Kolstad.  Any work derived from this source code is required to
 * retain this notice.
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

#include <math.h>

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

int get_number (int c, int cap);

/* get_number - prompt for a number in an intelligent way.
 *
 * C is a character gotten from getch(3).  That is, it is a textual
 * representation of a numberic character, like '1'.  CAP is the maximum number
 * to be allowed as a result; if there is no particular cap, call the function
 * with CAP equal to -1.
 *
 * Returns: the number entered.
 */

int
get_number (int c, int cap)
{
  int number, digits;

  if (cap == -1)
    cap = INT_MAX;

  number = c - '0';
  digits = 1;
  echochar (c);
  while (1)
    {
      c = getch ();
      switch (c)
        {
        case KEY_BACKSPACE:
        case '\177': /* ? ASCII DEL - used by xterm */
        case '\b':
          if (digits > 0)
            {
              int y, x;
              getyx (stdscr, y, x);

              digits--;
              number /= 10;
              move (y, x - 1);
              delch ();
              refresh ();
            }
          break;

        case '\n':
        case '\r':
          return number;

        default:
          if (c < '0' || c > '9')
            {
              continue;
            }
          digits++;

          if (number > ((cap - (c - '0')) / 10))
            {
              int y, x;
              getyx (stdscr, y, x);

              number = cap;
              move (y, x - digits + 1);
              digits = (int) log10 (cap) + 1;
              clrtoeol ();
              printw ("%d", number);
              refresh ();
            }
          else
            {
              number = 10 * number + (c - '0');
              echochar (c);
            }
          break;
        }
    }
}
