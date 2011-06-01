/*
 * display_index.c - print a notesfile index
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based on prtind.c from the UIUC notes distribution by Ray Essick and Rob
 * Kolstad.  Any work derived from this source code is required to retain this
 * notice.
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

#include "notes.h"

#include "newts/uiuc-compatibility.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_SYS_IOCTL_H
# include <sys/ioctl.h>
#endif

/* display_index - print out the note index for NFP, starting on note FIRST. */

void
display_index (struct notesfile *nf, int *first, int *last)
{
  time_t now;
  struct tm *tm;
  int row;
  int last_year, last_day, last_month;
  int i;
#ifdef FIONREAD
  long ioctlval;
#endif

  char title[TITLEN];
  char buf[NAMESZ + SYSSZ + 2];

  short leftoffset;
  short rightoffset;
  short authspace = 27;

  struct newt note;
  memset (&note, 0, sizeof (struct newt));
  nfref_copy (&note.nr.nfr, nf->ref);

  /* The following voodoo arranges things correctly on the screen in
   * non-traditional mode.
   */

  if (COLS > 113)
    {
      leftoffset = (COLS - 113) / 2 + 12;
      rightoffset = leftoffset + 13;
    }
  else if (COLS > 101)
    {
      leftoffset = COLS - 101;
      rightoffset = 26;
    }
  else if (COLS > 80)
    {
      leftoffset = 0;
      rightoffset = 26;
    }
  else
    {
      leftoffset = 0;
      rightoffset = COLS - 54;
      authspace -= 80 - COLS;
      if (authspace < 0)
        authspace = 0;
    }

  /* Bounds checking for FIRST. */

  if (*first > (int) nf->total_notes - LINES + 13)
    *first = nf->total_notes - LINES + 13;
  if (*first < 1)
    *first = 1;
  *last = *first + LINES - 13;

  /* Print the notesfile title, and possibly the notesfile name. */

  clear ();
  mvprintw (0, 1, "%s", nf->title);
  if (!traditional)
    printw (" (%s)", nfref_pretty_name (nf->ref));

  /* Print the current time. */

  now = time (NULL);
  tm = localtime (&now);
  if (traditional)
    move (0, 57);
  else
    move (0, COLS - 23);
  printw_time (tm);

  /* Print the subscript if appropriate. */

  if ((nf->options & NF_LOCKED) && !traditional)
    {
      if (nf->options & NF_ARCHIVE)
        mvprintw (1, 1, "[LOCKED ARCHIVE]");
      else
        mvprintw (1, 1, "[LOCKED]");
    }
  else if (nf->options & NF_ARCHIVE)
    {
#ifdef WRITEARCH
      mvprintw (1, 1, "[ARCHIVE]");
#else
      mvprintw (1, 1, "[ARCHIVE - NO WRITES]");
#endif
    }

  row = 4;

  /* We have no 'last day' yet before we run through the loop the first time.
   * After this is set, we check these values to determine if we're in the same
   * day, and therefore don't need to print a date.
   */

  last_year = last_day = last_month = 0;

  for (i = *first; (i <= *last) & (i <= nf->total_notes); i++)
    {
      note.nr.notenum = i;
      note.nr.respnum = 0;
      get_note (&note, FALSE);

      if ((note.options & NOTE_DELETED ||
           note.options & NOTE_DIRECTORS_ONLY ||
           note.options & NOTE_UNAPPROVED) &&
          !allowed (nf, DIRECTOR))
        {
          if (++(*last) > nf->total_notes)
            *last = nf->total_notes;
          continue;
        }

      tm = localtime (&note.created);

      /* The >s used to be !=s.  We changed this to match what UIUC notes
       * actually outputs.
       */

      if ((tm->tm_year + 1900 > last_year || (tm->tm_mon + 1) > last_month
           || tm->tm_mday > last_day) && !(note.options & NOTE_CORRUPTED))
        {
          if (traditional)
            mvprintw (row, 0, "%d/%d", last_month = (tm->tm_mon + 1),
                      last_day = tm->tm_mday);
          else
            mvprintw (row, 0 + leftoffset, "%d/%d", last_month =
                      (tm->tm_mon + 1), last_day = tm->tm_mday);
          if (tm->tm_year + 1900 != last_year)
            printw ("/%02d", (last_year = tm->tm_year + 1900) % 100);
        }

      /* Print the note number. */

      if (traditional)
        mvprintw (row, 8, "%4d", i);
      else
        mvprintw (row, 9 + leftoffset, "%4d", i);

      /* Print the graphic for director message or other status. */

      if (note.options & NOTE_DELETED)
        printw ("-");
      else if (note.options & NOTE_DIRECTORS_ONLY)
        printw ("=");
      else if (note.options & NOTE_ANNOUNCEMENT)
        printw ("+");
      else if (note.options & NOTE_UNAPPROVED)
        printw (":");
      else if (note.director_message)
        printw ("*");
      else
        printw (" ");

      /* Print the title. */

      snprintf (title, TITLEN, "%s", note.title);
      title[TITLEN - 1] = '\0';
      printw ("%s", title);

      /* Print the number of responses. */

      if (note.total_resps != 0)
        {
          if (traditional)
            mvprintw (row, TITLEN + 12, "%5d", note.total_resps);
          else
            mvprintw (row, COLS - rightoffset - 6, "%5d", note.total_resps);
        }

      /* Print the author.  If the author's system is the same as this system,
       * print only the username; otherwise, print both.
       */

      if (authspace)
        {
          if (strcmp (fqdn, note.auth.system) != 0 &&
              strcasecmp ("anonymous", note.auth.name) != 0)
            {
              snprintf (buf, authspace,
                        "%s@%s", note.auth.name, note.auth.system);
            }
          else
            {
              if (strcasecmp ("anonymous", note.auth.name) == 0)
                snprintf (buf, authspace,
                          traditional ? "Anonymous" : "anonymous");
              else
                snprintf (buf, authspace, "%s", note.auth.name);
            }
          buf[26] = '\0';                                 /* don't overflow line */
          if (traditional || COLS <= 80)
            mvprintw (row, TITLEN + 18, "%s", buf);
          else
            mvprintw (row, COLS - rightoffset, "%s", buf);
        }

#ifdef FIONREAD
      ioctl (0, FIONREAD, &ioctlval);
      if (ioctlval != 0) return;
#endif /* not FIONREAD */
      row++;
    }

  if (*last >= nf->total_notes)
    {
      if (traditional)
        mvprintw (++row, 13, _("**** End of Notes ****"));
      else
        mvprintw (++row, 14 + leftoffset, _("**** End of Notes ****"));
    }

  if (traditional || COLS < 81)
    mvprintw (row + 2, 24, "- - - - - - - - - - - - - - -");
  else
    mvprintw (row + 2, (COLS - 29) / 2, "- - - - - - - - - - - - - - -");

  refresh ();

  newts_free (note.title);
  newts_free (note.director_message);
  newts_free (note.auth.system);
  newts_free (note.auth.name);
  newts_free (note.text);
}
