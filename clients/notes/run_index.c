/*
 * run_index.c - handle input while viewing an index
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on index.c from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad.  Any work derived from this source code is required to retain
 * this notice.
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
#include "parse-datetime.h"
#include "which.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

static void index_help (void);

/* run_index - the main processing loop for the index.  FIRST and LAST are the
 * pointers to the first and last notes displayed on the current page,
 * respectively; NF is the notesfile we're talking about, RESPNUM passes a
 * response number if we search to one, and SUPPRESS_BLACKLIST passed out
 * whether we ought to ignore the blacklist.
 *
 * Returns: -1 for replot or NEXTSEQ, NEXTNOSEQ, QUITSEQ, or QUITNOSEQ.
 */

int
run_index (struct notesfile *nf, int *first, int *last, int *respnum,
           short *suppress_blacklist)
{
  int c;

  *respnum = 0;

  while (1)
    {
      move (LINES - 2, 0);
#ifdef PROMPT
      printw (PROMPT);
#endif
      c = getch ();

      switch (c)
        {
        case KEY_LEFT:
        case KEY_RIGHT:
          break;

        case 'r':
        case '\f':
        case KEY_RESIZE:
        case EOF:
          return -1;

        case KEY_UP:
          *first -= 1;
          return -1;

        case '-':
        case '\b':
        case '\177':   /* ASCII DEL - used by xterm */
        case KEY_BACKSPACE:
        case KEY_PPAGE:
          *first -= LINES - 13;
          return -1;

        case '=':
        case KEY_BEG:
        case KEY_HOME:
          *first = 1;
          return -1;

        case KEY_DOWN:
          *first += 1;
          return -1;

        case '+':
        case '\r':
        case '\n':
        case ' ':
        case KEY_ENTER:
        case KEY_NPAGE:
          *first = *last;
          return -1;

        case '*':
        case KEY_END:
          *first = nf->total_notes - LINES + 13;
          return -1;

        case '?':
          index_help ();
          return -1;

        case '!':
          spawn_subshell ();
          return -1;

        case '1': case '2': case '3': case '4': case '5':
        case '6': case '7': case '8': case '9':
          move (LINES - 1, 0);
          clrtoeol ();
          move (LINES - 2, 0);
          clrtoeol ();
          printw (_("Read note > "));
          refresh ();
          {
            int num = get_number (c, nf->total_notes);
            if (num == 0)
              return (-1);
            else
              {
                *suppress_blacklist = 1;
                return num;
              }
          }

        case 'w':  /* Write a new basenote. */
        case 'W':
          if (allowed (nf, WRITE))
            {
              if (!(nf->options & NF_ARCHIVE) ||
                  allowed (nf, DIRECTOR))
                {
                  return compose_note (nf, NULL, traditional ?
                                       _("Edit Note text:") :
                                       _("Edit Note Text"),
                                       _("Note Title: "), -1, NORMAL);
                }
              else
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("Sorry, you can not write in an archive"));
                  else
                    printw (_("You are not allowed to write in an archive."));
                  refresh ();
                  if (traditional)
                    {
                      sleep (2);
                      return -1;
                    }
                  else break;
                }
            }
          else
            {
              move (LINES - 1, 0);
              clrtoeol ();
              if (traditional)
                mvprintw (LINES - 1, 14,
                          _("Sorry, you are not allowed to write"));
              else
                printw (_("You are not allowed to write basenotes in this notesfile."));
              refresh ();
              if (traditional)
                {
                  sleep (2);
                  return -1;
                }
              else break;
            }

        case 'B':  /* Bitch, bitch, bitch. */
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
                          _("Gripe Header: ") : _("Gripe Title: "), -1, NORMAL);
            close_nf (&gripes, FALSE);
            nfref_free (gripesref);

            return -1;
          }

        case 'd':  /* Director options screen or summary. */
          if (allowed (nf, DIRECTOR))
            return run_director (nf);
          else
            {
              move (LINES - 1, 0);
              clrtoeol ();
              mvprintw (LINES - 1, traditional ? 14 : 0,
                        _("Anonymous: %s   Moderated: %s"),
                        (nf->options & NF_ANONYMOUS) ? "YES" : "NO ",
                        (nf->options & NF_MODERATED) ? "YES" : "NO ");
              refresh ();
              continue;
            }

        case 'p':  /* See the policy note. */
          if (nf->options & NF_POLICY)
            return 0;
          else
            {
              move (LINES - 1, 0);
              clrtoeol ();
              if (traditional)
                mvprintw (LINES - 1, 14, _("There is no policy note"));
              else
                mvprintw (LINES - 1, 9, _("There is no policy note"));
              break;
            }

        case 'N':
          move (LINES - 1, 0);
          clrtoeol ();
          printw (_("Archives are currently unsupported."));
          break;

        case 'n':  /* Nest to a new notesfiles. */
          {
            List nestlist;
            char *nfname, *prompt, *tildename;
            int result;
            int saveseqmode = sequencer;

            move (LINES - 1, 0);
            clrtoeol ();
            move (LINES - 2, 0);
            clrtoeol ();

            if (traditional)
              {
                prompt = newts_nmalloc (10 + strlen (_("New notesfile: ")),
                                        sizeof (char));
                strcpy (prompt, "         ");
                strcat (prompt, _("New notesfile: "));
              }
            else
              prompt = newts_strdup (_("New Notesfile: "));

            refresh ();

            nfname = gl_getline (prompt);
            newts_free (prompt);

            nfname[strlen (nfname) - 1] = '\0'; /* Chomp trailing '\n' */
            if (strlen (nfname) == 0)
              return -1;
            tildename = tilde_expand (nfname);
            gl_histadd (nfname);

            list_init (&nestlist,
                       (void * (*) (void)) nfref_alloc,
                       (void (*) (void *)) nfref_free,
                       NULL);
            parse_nf (tildename, &nestlist);

            newts_free (tildename);

            {
              ListNode *node = list_head (&nestlist);

              sequencer = NONE;

              while (node != NULL)
                {
                  newts_nfref *ref = (newts_nfref *) list_data (node);

                  result = master (ref);

                  node = list_next (node);

                  if (result == QUITSEQ)
                    {
                      sequencer = saveseqmode;
                      return QUITSEQ;
                    }
                  if (result == QUITNOSEQ)
                    {
                      sequencer = saveseqmode;
                      return QUITNOSEQ;
                    }
                }

              sequencer = saveseqmode;
            }

            list_destroy (&nestlist);

            update_nf (nf);

            return -1;
          }

        case 'a':  /* Search for a given author. */
        case 'A':
          {
            char *prompt;
            char *temp;
            struct newtref searchref;
            int found;

            memset (&searchref, 0, sizeof (struct newtref));
            nfref_copy (&searchref.nfr, nf->ref);
            searchref.notenum = *last;
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
                display_index (nf, first, last);
              }

            move (LINES - 2, 0);
            clrtoeol ();
            printw (_("Searching for articles by %s"), asearch);
            refresh ();
            found = author_search (&searchref, asearch);
            if (found == -1)
              {
                move (LINES - 1, 0);
                clrtoeol ();
                printw (_("Can't find any articles by %s"), asearch);
                refresh ();
                break;
              }
            *respnum = searchref.respnum;
            *suppress_blacklist = 1;

            return found;
          }

        case 'x':  /* Search for a particular title. */
        case 'X':
          {
            char *prompt;
            char *temp;
            struct newtref searchref;
            int found;

            memset (&searchref, 0, sizeof (struct newtref));
            nfref_copy (&searchref.nfr, nf->ref);
            searchref.notenum = *last;
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
                display_index (nf, first, last);
              }

            move (LINES - 2, 0);
            clrtoeol ();
            if (!traditional)
              printw (_("Searching for articles titled '%s'"), tsearch);
            refresh ();
            found = title_search (&searchref, tsearch);
            if (found == -1)
              {
                move (LINES - 1, 0);
                clrtoeol ();
                if (traditional)
                  printw (_("%s: Not Found"), tsearch);
                else
                  printw (_("Can't find any articles titled '%s'"), tsearch);
                refresh ();
                break;
              }
            *respnum = 0;
            *suppress_blacklist = 1;

            return found;
          }

        case '/':  /* Text search. */
        case '\\':
          {
            char *prompt;
            char *temp;
            struct newtref searchref;
            int found;

            memset (&searchref, 0, sizeof (struct newtref));
            nfref_copy (&searchref.nfr, nf->ref);
            searchref.notenum = *last;
            searchref.respnum = 0;

            if (c == '/' || txtsearch == NULL || *txtsearch == '\0')
              {
                move (LINES - 1, 0);
                clrtoeol ();
                if (traditional)
                  {
                    prompt = newts_strdup (_("Search String: "));
                  }
                else
                  {
                    prompt = newts_strdup (_("Search for text: "));
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
                    txtsearch = newts_nrealloc (txtsearch, strlen (temp) + 1,
                                                sizeof (char));
                    strcpy (txtsearch, temp);
                    gl_histadd (txtsearch);
                  }
                clear ();
                display_index (nf, first, last);
              }

            move (LINES - 2, 0);
            clrtoeol ();
            if (!traditional)
              printw (_("Searching for articles with text '%s'"), txtsearch);
            refresh ();
            found = text_search (&searchref, txtsearch);
            if (found == -1)
              {
                move (LINES - 1, 0);
                clrtoeol ();
                if (traditional)
                  printw (_("%s: Not Found"), txtsearch);
                else
                  printw (_("Can't find any articles with text '%s'"),
                          txtsearch);
                refresh ();
                break;
              }
            *respnum = searchref.respnum;
            *suppress_blacklist = '/';  /* Pirating this for highlighting. */
            return found;
          }

        case 'j':  /* Find next updated note. */
        case 'J':
          {
            struct newtref nr;
            memset (&nr, 0, sizeof (struct newtref));
            nfref_copy (&nr.nfr, nf->ref);
            nr.notenum = 0;
            nr.respnum = 0;

            *suppress_blacklist = 'j';
            return get_next_note (&nr, seqtime);
          }

        case 'g':  /* On the index 'g' is identical to 'l'. */
        case 'l':  /* Find next updated note or quit from notesfile. */
        case 'L':
          {
            int seqnum;
            struct newtref nr;
            memset (&nr, 0, sizeof (struct newtref));
            nfref_copy (&nr.nfr, nf->ref);
            nr.notenum = 0;
            nr.respnum = 0;

            if ((seqnum = get_next_note (&nr, seqtime)) == -1)
              return NEXTSEQ;
            else
              {
                *suppress_blacklist = 'l';
                return seqnum;
              }
          }

        case 'O':  /* Set time to 12:00 midnight today. */
          {
            time_t timet;
            struct tm *stm;

            time (&timet);
            stm = localtime (&timet);
            stm->tm_sec = 0;
            stm->tm_min = 0;
            stm->tm_hour = 0;
            seqtime = mktime (stm);
            alt_time = TRUE;
            move (LINES - 1, 0);
            clrtoeol ();
            if (traditional)
              move (LINES - 1, 14);
            else
              move (LINES - 1, 0);
            stm = localtime (&seqtime);
            printw (_("Set to read notes since: "));
            printw_time (stm);
            continue;
          }

        case 'o':  /* Set time to an arbitrary time. */
          {
            struct timespec result, now;
            struct tm *stm;
            char *datestr, *prompt;
            short passed = FALSE;
            short stop = FALSE;

            now.tv_sec = time (NULL);

            if (traditional)
              {
                prompt = newts_nmalloc (strlen (_("New Date > ")) + 10,
                                        sizeof (char));
                strcpy (prompt, "         ");
                strcat (prompt, _("New Date > "));
              }
            else
              {
                prompt = newts_strdup (_("New Date > "));
              }

            move (LINES - 1, 0);
            clrtoeol ();
            if (traditional)
              move (LINES - 1, 14);
            stm = localtime (&seqtime);
            printw (_("Set to read notes since: "));
            printw_time (stm);

            while (!stop)
              {

                move (LINES - 2, 0);
                clrtoeol ();

                refresh ();

                datestr = gl_getline (prompt);
                datestr[strlen (datestr) - 1] = '\0'; /* Chomp trailing '\n' */
                if (strlen (datestr) == 0)
                  {
                    stop = TRUE;
                    continue;
                  }
                gl_histadd (datestr);

                if (parse_datetime (&result, datestr, &now))
                  if (result.tv_sec <= now.tv_sec)
                    {
                      seqtime = result.tv_sec;
                      alt_time = TRUE;
                      passed = TRUE;
                    }
                clear ();
                display_index (nf, first, last);
                if (traditional)
                  move (LINES - 1, 14);
                else
                  move (LINES - 1, 0);
                stm = localtime (&seqtime);
                printw (_("Set to read notes since: "));
                printw_time (stm);
              }

            newts_free (prompt);

            if (passed)
              {
                display_index (nf, first, last);
                if (traditional)
                  move (LINES - 1, 14);
                else
                  move (LINES - 1, 0);
                stm = localtime (&seqtime);
                printw (_("Set to read notes since: "));
                printw_time (stm);
                continue;
              }
            else
              return -1;
          }

        case 'q':
        case 'k':
          return NEXTSEQ;

        case 'Q':
        case 'K':
          return NEXTNOSEQ;

        case '\04':
          return QUITNOSEQ;

        case 'z':
          return QUITSEQ;

        default:
          move (LINES - 1, 0);
          clrtoeol ();
          if (traditional)
            mvprintw (LINES - 1, 9, _("type ? for help, q to quit"));
          else
            mvprintw (LINES - 1, 9, _("Type ? for help, q to quit"));
          beep ();
          break;
        }
    }
}

/* index_help - show help for the index page. */

static void
index_help (void)
{
  int c, column;

  do {
    clear ();

    mvprintw (0, (COLS - 10) / 2 - 1, "Index Help");

    column = (COLS - 80) / 2 - 3;

    mvprintw (2, column + 4, "<number>   view note by number");
    mvprintw (3, column + 4, "<RET>      display next page");
    mvprintw (4, column + 4, "+, <SPC>   display next page");
    mvprintw (5, column + 4, "<PGDN>     display next page");
    mvprintw (6, column + 4, "-, <BS>    display previous page");
    mvprintw (7, column + 4, "<PGUP>     display previous page");
    mvprintw (8, column + 4, "=, <HOME>  display first page");
    mvprintw (9, column + 4, "*, <END>   display last page");
    mvprintw (10, column + 4, "<arrows>   move around index");
    mvprintw (12, column + 4, "C-d, z     quit notes (update seq.)");
    mvprintw (13, column + 4, "q, k       exit nf and update seq.");
    mvprintw (14, column + 4, "Q, K       exit nf");
    mvprintw (15, column + 4, "!          fork a subshell");

    mvprintw (2, column + 43, "a, A    search by author (repeat)");
    mvprintw (3, column + 43, "B       complain about Newts");
    mvprintw (4, column + 43, "d       view/change director options");
    mvprintw (5, column + 43, "j, J    show next updated note");
    mvprintw (6, column + 43, "l, L    show next updated note or exit");
    mvprintw (7, column + 43, "n, N    open new notesfile (archive)");
    mvprintw (8, column + 43, "o       enter a new sequencer time");
    mvprintw (9, column + 43, "O       set sequencer to 12:00 a.m.");
    mvprintw (10, column + 43, "p       read policy note");
    mvprintw (11, column + 43, "r, C-l  redraw the screen");
    mvprintw (12, column + 43, "w, W    write a note");
    mvprintw (13, column + 43, "x, X    search by title (repeat)");
    mvprintw (14, column + 43, "/, \\    search for note text (repeat)");

    mvprintw (17, column + 20, "If you have suggestions for the help wording");
    mvprintw (18, column + 20, "or the layout, please pass them on to Tyler.");

    move (LINES - 2, 0);

    refresh ();

    c = getch ();
    while (c == KEY_RESIZE)
      c = getch ();
  }
  while (c == EOF);

  return;
}
