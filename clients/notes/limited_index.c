/*
 * limited_index.c - show a limited version of the index to select notes with
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on limindx.c from the UIUC notes distribution by Ray Essick
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

#include "notes.h"

#include "gl_getline.h"
#include "strcase.h"
#include "newts/uiuc-compatibility.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

static void limited_help (void);

int
limited_index (struct notesfile *nf)
{
  int c;
  int first = nf->total_notes - LINES + 13;
  int last;
  short redraw = TRUE;

  while (1)
    {
      if (redraw)
        {
          clear ();
          display_index (nf, &first, &last);
          mvprintw (1, 27, _("---- Limited Index ----"));
        }
      redraw = TRUE;

      if (traditional)
        move (LINES - 1, 0);
      else
        move (LINES - 2, 0);
#ifdef PROMPT
      printw (PROMPT);
#endif
      c = getch ();

      switch (c)
        {
        case KEY_LEFT:
        case KEY_RIGHT:
          redraw = FALSE;
          break;

        case 'r':
        case '\f':
        case KEY_RESIZE:
        case EOF:
          break;

        case KEY_UP:
          first -= 1;
          break;

        case '-':
        case '\b':
        case '\177':   /* ASCII DEL - used by xterm */
        case KEY_BACKSPACE:
        case KEY_PPAGE:
          first -= LINES - 13;
          break;

        case '=':
        case KEY_BEG:
        case KEY_HOME:
          first = 1;
          break;

        case KEY_DOWN:
          first += 1;
          break;

        case '+':
        case '\r':
        case '\n':
        case ' ':
        case KEY_ENTER:
        case KEY_NPAGE:
          first = last;
          break;

        case '*':
        case KEY_END:
          first = nf->total_notes - LINES + 13;
          break;

        case '?':
          limited_help ();
          break;

        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
          move (LINES - 1, 0);
          clrtoeol ();
          move (LINES - 2, 0);
          clrtoeol ();
          if (traditional)
            mvprintw (LINES - 1, 0, "  ");
          printw (_("Note number > "));
          refresh ();
          {
            int num = get_number (c, nf->total_notes);
            if (num <= nf->total_notes)
              {
                /* Reprint the index line for this note. */

                char buf[NAMESZ + SYSSZ + 2];
                char title[TITLEN];
                struct newt note;
                struct tm *tm;
                int yn;
                memset (&note, 0, sizeof (struct newt));

                nfref_copy (&note.nr.nfr, nf->ref);
                note.nr.notenum = num;
                note.nr.respnum = 0;

                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();

                get_note (&note, FALSE);

                if ((note.options & NOTE_DELETED ||
                     note.options & NOTE_DIRECTORS_ONLY ||
                     note.options & NOTE_UNAPPROVED) &&
                    !allowed (nf, DIRECTOR))
                  {
                    if (traditional)
                      move (LINES - 1, 14);
                    else
                      move (LINES - 1, 0);
                    printw (_("Note %d isn't available"));
                    if (!traditional) printw (".");
                    redraw = FALSE;
                    break;
                  }

                tm = localtime (&note.created);

                if (!(note.options & NOTE_CORRUPTED))
                  {
                    mvprintw (LINES - 2, 0, "%d/%d", tm->tm_mon + 1,
                              tm->tm_mday);
                    printw ("/%02d", (tm->tm_year + 1900) % 100);
                  }

                if (traditional)
                  mvprintw (LINES - 2, 8, "%4d", num);
                else
                  mvprintw (LINES - 2, 9, "%4d", num);
                if (note.options & NOTE_DIRECTORS_ONLY)
                  printw ("=");
                else if (note.options & NOTE_ANNOUNCEMENT)
                  printw ("+");
                else if (note.options & NOTE_UNAPPROVED)
                  printw (":");
                else if (note.options & NOTE_DELETED)
                  printw ("-");
                else if (note.director_message)
                  printw ("*");
                else
                  printw (" ");

                snprintf (title, TITLEN, "%s", note.title);
                title[TITLEN - 1] = '\0';
                printw ("%s", title);
                if (note.total_resps != 0)
                  {
                    if (traditional)
                      mvprintw (LINES - 2, TITLEN + 12, "%5d", note.total_resps);
                    else
                      mvprintw (LINES - 2, COLS - 27, "%5d", note.total_resps);
                  }

                if (strcmp (fqdn, note.auth.system) != 0 &&
                    strcasecmp ("anonymous", note.auth.name) != 0)
                  {
                    snprintf (buf, 27,
                              "%s@%s", note.auth.name, note.auth.system);
                  }
                else
                  {
                    if (strcasecmp ("anonymous", note.auth.name) == 0)
                      snprintf (buf, 27,
                                traditional ? "Anonymous" : "anonymous");
                    else
                      snprintf (buf, 27, "%s", note.auth.name);
                  }
                buf[26] = '\0';                                 /* don't overflow line */
                if (traditional || COLS <= 80)
                  mvprintw (LINES - 2, TITLEN + 18, "%s", buf);
                else
                  mvprintw (LINES - 2, COLS - 27, "%s", buf);

                YES_OR_NO (yn,
                           if (traditional)
                             mvprintw (LINES - 1, 0, _("Do you really want note %s? "), num);
                           else
                             mvprintw (LINES - 1, 0, _("Select note %d? (y/n): "), num);
                           );
                if (yn)
                  return num;
                break;
              }
            else
              {
                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();
                if (traditional)
                  move (LINES - 1, 14);
                else
                  move (LINES - 1, 0);
                printw (_("Note %d doesn't exist"));
                if (!traditional) printw (".");
                redraw = FALSE;
                break;
              }
          }

        case 'B':  /* Bitch, bitch, bitch. */
          if (traditional)
            break;
          {
            newts_nfref *gripesref = nfref_alloc ();
            struct notesfile gripes;
            memset (&gripes, 0, sizeof (struct notesfile));

            nfref_set_user (gripesref, username);
            nfref_set_system (gripesref, fqdn);
            nfref_set_owner (gripesref, NULL);
            nfref_set_name (gripesref, "newts.gripes");

            open_nf (gripesref, &gripes);
            compose_note (&gripes, NULL, traditional ? _("Edit Gripe text:") :
                          _("Edit Gripe Text:"), traditional ?
                          _("Gripe Header: ") : _("Gripe Title: "), -1,
                          NORMAL);
            close_nf (&gripes, FALSE);

            nfref_free (gripesref);
            return -1;
          }

        case 'a':  /* Author search. */
        case 'A':
          {
            char *prompt;
            char *temp;
            struct newtref searchref;
            int found;

            nfref_copy (&searchref.nfr, nf->ref);
            searchref.notenum = last;
            searchref.respnum = 0;

            if (c == 'a' || asearch == NULL || *asearch == '\0')
              {
                move (LINES - 1, 0);
                clrtoeol ();
                if (traditional)
                  {
                    prompt = newts_strdup (_("Search author: "));
                  }
                else
                  {
                    prompt = newts_strdup (_("Search by author: "));
                  }
                move (LINES - 2, 0);
                clrtoeol ();
                refresh ();
                temp = gl_getline (prompt);
                newts_free (prompt);

                temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                if (strlen (temp) == 0)
                  return -1;
                else
                  {
                    asearch = newts_nrealloc (asearch, strlen (temp) + 1,
                                              sizeof (char));
                    strcpy (asearch, temp);
                    gl_histadd (asearch);
                  }
                clear ();
                display_index (nf, &first, &last);
              }

            move (LINES - 2, 0);
            clrtoeol ();
            printw ("Searching for articles by %s", asearch);
            refresh ();
            do
              {
                found = author_search (&searchref, asearch);

                if (found > 0)
                  {
                    if (searchref.respnum != 0)  /* Ignore responses */
                      continue;
                    first = found;
                    break;
                  }
              }
            while (found > 0);
            if (traditional)
              {
                move (LINES - 1, 0);
                clrtoeol ();
                printw ("Can't find any articles titled '%s'", tsearch);
                refresh ();
              }
            redraw = FALSE;
            break;
          }

        case 'x':  /* Title search. */
        case 'X':
          {
            char *prompt;
            char *temp;
            struct newtref searchref;
            int found;

            nfref_copy (&searchref.nfr, nf->ref);
            searchref.notenum = last;
            searchref.respnum = 0;

            if (c == 'x' || tsearch == NULL || *tsearch == '\0')
              {
                move (LINES - 1, 0);
                clrtoeol ();
                if (traditional)
                  {
                    prompt = newts_strdup (_("Search String: "));
                  }
                else
                  {
                    prompt = newts_strdup (_("Search by title: "));
                  }
                move (LINES - 2, 0);
                clrtoeol ();
                refresh ();
                temp = gl_getline (prompt);
                newts_free (prompt);

                temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                if (strlen (temp) == 0)
                  return -1;
                else
                  {
                    tsearch = newts_nrealloc (tsearch, strlen (temp) + 1,
                                              sizeof (char));
                    strcpy (tsearch, temp);
                    gl_histadd (tsearch);
                  }
                clear ();
                display_index (nf, &first, &last);
              }

            move (LINES - 2, 0);
            clrtoeol ();
            if (!traditional)
              printw ("Searching for articles titled '%s'", tsearch);
            refresh ();
            found = title_search (&searchref, tsearch);
            if (found > 0)
              first = found;
            else
              {
                if (traditional)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw ("Can't find any articles titled '%s'", tsearch);
                    refresh ();
                  }
                redraw = FALSE;
              }
            break;
          }

        case 'q':
        case 'k':
        case 'Q':
        case 'K':
          return 0;

        case '\04':
          return QUITNOSEQ;

        case 'z':
          return QUITSEQ;

        default:
          if (!traditional)
            {
              move (LINES - 1, 0);
              clrtoeol ();
              mvprintw (LINES - 1, 9, _("Type ? for help, q to quit"));
              beep ();
              redraw = FALSE;
            }
          break;
        }
    }
}

/* limited_help - print out help for the limited index. */

static void
limited_help (void)
{
  int c, column;

  do {
    clear ();

    mvprintw (0, (COLS - 18) / 2 - 1, "Limited Index Help");

    column = (COLS - 80) / 2 - 3;

    mvprintw (2, column + 4, "<number>  select note by number");
    mvprintw (3, column + 4, "<RET>     display next page");
    mvprintw (4, column + 4, "<SPC>     display next page");
    mvprintw (5, column + 4, "+         display next page");
    mvprintw (6, column + 4, "<PGDN>    display next page");
    mvprintw (7, column + 4, "-, <BS>   display previous page");
    mvprintw (8, column + 4, "<PGUP>    display previous page");
    mvprintw (9, column + 4, "=, <HOME> display first page");
    mvprintw (10, column + 4, "*, <END>  display last page");
    mvprintw (11, column + 4, "<arrows>  move around index");

    mvprintw (2, column + 43, "a, A      search by author (repeat)");
    mvprintw (3, column + 43, "B         complain about Newts");
    mvprintw (4, column + 43, "r, C-l    redraw the screen");
    mvprintw (5, column + 43, "x, X      search by title (repeat)");
    mvprintw (7, column + 43, "q, k      give up without selecting");
    mvprintw (8, column + 43, "Q, K      give up without selecting");
    mvprintw (9, column + 43, "z         quit notes, update seq");
    mvprintw (10, column + 43, "C-d       quit notes");

    mvprintw (13, column + 20, "If you have suggestions for the help wording");
    mvprintw (14, column + 20, "or the layout, please pass them on to Tyler.");

    move (LINES - 2, 0);

    refresh ();

    c = getch ();
    while (c == KEY_RESIZE)
      c = getch ();
  }
  while (c == EOF);

  return;
}
