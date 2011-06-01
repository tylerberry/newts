/*
 * director.c - handle the director options screen
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

#include "notes.h"

#include "gl_getline.h"
#include "newts/uiuc.h"
#include "newts/uiuc-compatibility.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if STDC_HEADERS
# include <limits.h>
#endif

/* Modes used for the multi_delete function. */

enum multi_modes
  {
    DELETE,
    UNDELETE
  };

static int display_director_screen (struct notesfile *nf);
static void director_help (void);
static void multi_delete (struct notesfile *nf, int first, int last,
                                  int mode);
static void frob (struct notesfile *nf, int flag);

/* run_director - handle the main loop for the director screen.
 *
 * Returns: -1 for redraw or QUITNOSEQ on a C-d.
 */

int
run_director (struct notesfile *nf)
{
  int c;
  short redraw = TRUE;
  int row = 0;
  int changed = FALSE;
  short policy_written = FALSE;
  struct newt policy;
  struct notesfile *save;
  struct uiuc_opts *opts;
  struct uiuc_opts saveopts;
  char *savetitle, *savedirmsg;

  /* This check should never pass; it should be caught at the index screen. */

  if (!allowed (nf, DIRECTOR))
    {
      clear ();
      if (traditional)
        mvprintw (LINES - 1, 0, _("Sorry, you are not a director"));
      else
        mvprintw (LINES - 1, 0, _("Only directors can access the director "
                                  "options screen."));
      refresh ();
      sleep (2);
      return -1;
    }

  /* Save existing values for a revert. */

  save = nf_alloc ();
  memcpy (save, nf, sizeof (struct notesfile));
  memcpy (&saveopts, nf->opts, sizeof (struct uiuc_opts));
  savetitle = newts_strdup (nf->title);
  savedirmsg = newts_strdup (nf->director_message);

  opts = (struct uiuc_opts *) nf->opts;

  while (1)
    {
      if (redraw)
        {
          /* In traditional mode, all changes happen immediately. */

          if (traditional && changed)
            {
              modify_nf (nf);
              changed = FALSE;
            }
          clear ();
          row = display_director_screen (nf) + 1;
        }
      redraw = FALSE;

      move (LINES - 2, traditional ? (int) strlen (_("Option: ")) : 0);
      refresh ();

      c = getch ();

      switch (c)
        {
        case 'r':  /* Redraw the screen. */
        case '\f':
        case KEY_RESIZE:
        case EOF:
          redraw = TRUE;
          break;

        case 'i':
        case 'k':
        case 'q':
          if (c == 'i' && traditional)  /* Traditional didn't use 'i'. */
            {
              beep ();
              break;
            }
          if (policy_written)
            write_note (nf, &policy, ADD_POLICY + UPDATE_TIMES + ADD_ID);
          modify_nf (nf);

          newts_free (savetitle);
          newts_free (savedirmsg);
          nf_free (save);
          return -1;

        case 'K':  /* Quit without saving. */
        case 'Q':
          if (traditional)  /* Traditional didn't have this either. */
            {
              beep ();
              break;
            }
          memcpy (nf, save, sizeof (struct notesfile));
          memcpy (nf->opts, &saveopts, sizeof (struct uiuc_opts));
          strncpy (nf->title, savetitle, strlen (savetitle) + 1);
          strncpy (nf->director_message, savedirmsg, strlen (savedirmsg) + 1);

          newts_free (savetitle);
          newts_free (savedirmsg);
          nf_free (save);
          return -1;

        case '\04':
          newts_free (savetitle);
          newts_free (savedirmsg);
          nf_free (save);
          return QUITNOSEQ;

        case '?':
          redraw = TRUE;
          director_help ();
          break;

        case '!':
          redraw = TRUE;
          spawn_subshell ();
          break;

        case 'a':
          redraw = TRUE;
          changed = TRUE;
          frob (nf, NF_ANONYMOUS);
          break;

        case 'o':
          redraw = TRUE;
          changed = TRUE;
          frob (nf, NF_LOCKED);
          break;

        case 'A':
          redraw = TRUE;
          changed = TRUE;
          frob (nf, NF_ARCHIVE);
          break;

        case 'M':
          redraw = TRUE;
          changed = TRUE;
          frob (nf, NF_MODERATED);
          break;

        case 'p':  /* Go to the permissions screen. */
          {
            int result;

            redraw = TRUE;
            result = run_access (nf->ref);
            if (result == QUITSEQ || result == QUITNOSEQ)
              {
                newts_free (savetitle);
                newts_free (savedirmsg);
                nf_free (save);
                return result;
              }
            else
              break;
          }

        case 't':  /* Edit nf title. */
          {
            int i;
            char *prompt, *title;

            redraw = TRUE;

            if (traditional)
              {
                clear ();
                display_director_screen (nf);
                mvprintw (row, 9, _("Enter new title for notefile"));
                move (row + 2, 9);
                for (i = 0; i < NNLEN; i++) printw ("-");
                prompt = "         ";
                move (row + 1, 0);
                clrtoeol ();
              }
            else
              {
                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();
                prompt = _("Enter new notesfile title: ");
              }

            refresh ();

            title = gl_getline (prompt);
            title[strlen (title) - 1] = '\0'; /* Chomp trailing '\n' */
            if (strlen (title) == 0)
              break;
            gl_histadd (title);
            changed = TRUE;
            strncpy (nf->title, title, NNLEN);
            nf->title[NNLEN - 1] = '\0';

            break;
          }

        case 'm':  /* New director message. */
          {
            int i;
            char *prompt, *dirmsg;

            redraw = TRUE;

            if (traditional)
              {
                clear ();
                display_director_screen (nf);
                mvprintw (row, 9, _("Enter new director message"));
                move (row + 2, 9);
                for (i = 0; i < DMLEN; i++) printw ("-");
                prompt = "         ";
                move (row + 1, 0);
                clrtoeol ();
              }
            else
              {
                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();
                prompt = _("Enter new director message: ");
              }

            refresh ();

            dirmsg = gl_getline (prompt);
            dirmsg[strlen (dirmsg) - 1] = '\0';
            if (strlen (dirmsg) == 0)
              break;
            gl_histadd (dirmsg);
            changed = TRUE;
            strncpy (nf->director_message, dirmsg, NNLEN);
            nf->director_message[DMLEN - 1] = '\0';

            break;
          }

        case 'z':  /* Delete ... */
        case 'u':  /* ... or undelete a list of notes. */
          {
            char *list, *prompt;
            int listptr, start, end;

            redraw = TRUE;
            if (traditional)
              {
                clear ();
                display_director_screen (nf);
                move (row, 0);
                prompt = c == 'z' ? _("Enter list of notes to delete: ")
                  : _("Enter list of notes to un-delete: ");
              }
            else
              {
                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();
                prompt = c == 'z' ? _("Enter list of notes to delete: ")
                  : _("Enter list of notes to undelete: ");
              }
            refresh ();

            list = gl_getline (prompt);
            list[strlen (list) - 1] = '\0';
            if (strlen (list) == 0)
              break;
            gl_histadd (list);
            if (traditional)
              {
                int yn;

                YES_OR_NO (yn,
                           mvprintw (row + 1, 0,
                                     c == 'z' ? "Going to delete: %s" :
                                     "Going to un-delete: %s", list);
                           mvprintw (row + 2, 0,
                                     "Do you really want to do that? ");
                           );
                if (!yn)
                  break;
                move (row + 3, 0);
              }
            else
              {
                if (changed)
                  {
                    int yn;

                    clear ();
                    display_director_screen (nf);
                    YES_OR_NO (yn,
                               mvprintw (LINES - 2, 0,
                                         _("Save previous changes? (y/n): "));
                               );
                    if (yn)
                      {
                        modify_nf (nf);
                        changed = FALSE;
                      }
                  }
                clear ();
                move (LINES - 1, 0);
                printw (c == 'z' ? _("Deleted: ") : _("Undeleted: "));
              }
            redraw = FALSE;
            listptr = 0;
            while (list_parse (list, &listptr, &start, &end))
              {
                if (start == end)
                  printw ("%d ", start);
                else
                  printw ("%d-%d ", start, end);
                multi_delete (nf, start, end, c == 'z' ? DELETE : UNDELETE);
              }
            update_nf (nf);
            display_director_screen (nf);
            refresh ();
            break;
          }

        case 'c':  /* Compress the notesfile. */

          /* Note on a change: this version, because POSIX file locks are used,
           * does not require the notesfile to be 'closed' or 'locked' prior to
           * compression.  This is a good thing.
           */

          {
            int yn, result;
            unsigned nnotes, nresps;

            YES_OR_NO (yn,
                       if (traditional)
                         {
                           clear ();
                           display_director_screen (nf);
                           move (row, 0);
                           mvprintw (row, 9, _("Really Compress? (y/n) "));
                         }
                       else
                         {
                           move (LINES - 1, 0);
                           clrtoeol ();
                           move (LINES - 2, 0);
                           clrtoeol ();
                           printw (_("Save previous changes and compress notesfile? (y/n): "));
                         }
                       );
            if (!yn)
              {
                if (traditional)
                  mvprintw (row + 1, 9, _("Compress not done"));
                break;
              }

            /* If we -didn't- do this, all the changes would revert;
             * that's not what we want.
             */

            modify_nf (nf);
            changed = FALSE;
            mvprintw (traditional ? row +1 : LINES - 1, 0,
                      _("Compressing..."));
            refresh ();

            result = compress_nf (nf, &nnotes, &nresps);
            if (result == 0)
              {
                clear ();
                display_director_screen (nf);
                if (traditional)
                  mvprintw (LINES - 4, 0,
                            _("Compress left %d notes and %d responses"),
                            nnotes, nresps);
                else
                  mvprintw (LINES - 1, 0,
                            _("%d notes and %d responses left after "
                              "compression."),
                            nnotes, nresps);
              }
            else
              {
                if (traditional)
                  {
                    clear ();
                    display_director_screen (nf);
                    mvprintw (LINES - 4, 0,
                              _("Compress not done"));
                  }
                else
                  {
                    move (LINES - 2, 0);
                    clrtoeol ();
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw (_("Compression aborted."));
                  }
              }
            }
          break;

        case 'w':  /* Write a new policy note. */
          {
            redraw = TRUE;

            if (nf->options & NF_POLICY || policy_written)
              {
                int yn;

                move (LINES - 1, 0);
                clrtoeol ();
                move (LINES - 2, 0);
                clrtoeol ();
                YES_OR_NO (yn,
                           if (traditional)
                             mvprintw (LINES - 1, 14,
                                       _("Rewrite policy? (y/n) :"));
                           else
                             printw (_("Rewrite policy note? (y/n): "));
                           );
                if (!yn)
                  break;
              }

            if (traditional)
              move (LINES - 1, 0);
            else
              move (LINES - 2, 0);
            clrtoeol ();
            printw (_("Edit New Policy Text:"));

            refresh ();

            memset (&policy, 0, sizeof (struct newt));

            policy.text = get_text (NULL, TRUE);
            if (policy.text == NULL)
              break;

            policy.title = "POLICY NOTE";
            policy.director_message = NULL;
            time (&policy.created);
            time (&policy.modified);

            nfref_copy (&policy.nr.nfr, nf->ref);
            policy.nr.notenum = -1;

            policy.auth.name = username;
            policy.auth.system = fqdn;
            policy.auth.uid = euid;

            nf->options |= NF_POLICY;
            changed = TRUE;
            policy_written = TRUE;

            break;
          }

        case 'e':  /* Change the expiration time. */
          {
            char *prompt, *input, *tail;
            long temp;

            if (traditional)
              {
                prompt = newts_nmalloc (strlen (_("New Expiration time: ")) + 10,
                                        sizeof (char));
                strcpy (prompt, "         ");
                strncat (prompt, _("New Expiration time: "),
                         strlen (_("New Expiration time: ")) + 1);
              }
            else
              prompt = newts_strdup (_("New Expiration Threshold: "));

            if (traditional)
              {
                clear ();
                display_director_screen (nf);
              }

            while (1)
              {
                if (traditional)
                  move (row, 0);
                else
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    move (LINES - 2, 0);
                    clrtoeol ();
                  }
                refresh ();
                input = gl_getline (prompt);
                input[strlen (input) - 1] = '\0';
                gl_histadd (input);

                if (strlen (input) == 0)
                  {
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Expiration Threshold Unchanged"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      redraw = TRUE;
                    break;
                  }

                changed = TRUE;

                if (!strcasecmp (input, "never") || !strcasecmp (input, "n"))
                  {
                    opts->expire_threshold = NEVER;
                    redraw = TRUE;
                    break;
                  }
                else if (!strcasecmp (input, "default") || !strcasecmp (input, "d"))
                  {
                    opts->expire_threshold = 0;
                    redraw = TRUE;
                    break;
                  }

                temp = strtol (input, &tail, 0);
                if (temp == 0 && input == tail)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Want 'default', 'never', or a number"));
                        move (row + 2, 9);
                        clrtoeol ();
                        printw (_("<return> to leave unchanged"));
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Please enter 'default', 'never', a number, or <RET> to leave value unchanged."));
                      }
                  }
                else if ((temp == LONG_MAX && errno == ERANGE) || temp > INT_MAX)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Maximum valid value exceeded"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Maximum valid value exceeded."));
                      }
                  }
                else if (temp < -1)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Negative values not allowed"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Negative values not allowed."));
                      }
                  }
                else
                  {
                    opts->expire_threshold = temp;
                    redraw = TRUE;
                    break;
                  }
              }

            newts_free (prompt);
            break;
          }

        case 'W':  /* Change the "Working Set Size" - minimum notes in nf. */
          {
            char *prompt, *input, *tail;
            long temp;

            if (traditional)
              {
                prompt = newts_nmalloc (strlen (_("New Working Set Size: ")) + 10,
                                        sizeof (char));
                strncpy (prompt, "         ", 10);
                strncat (prompt, _("New Working Set Size: "),
                         strlen (_("New Working Set Size: ")) + 1);
              }
            else
              prompt = newts_strdup (_("New minimum number of notes: "));

            if (traditional)
              {
                clear ();
                display_director_screen (nf);
              }

            while (1)
              {
                if (traditional)
                  move (row, 0);
                else
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    move (LINES - 2, 0);
                    clrtoeol ();
                  }
                refresh ();
                input = gl_getline (prompt);
                input[strlen (input) - 1] = '\0';
                gl_histadd (input);

                if (strlen (input) == 0)
                  {
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Working Set Size Unchanged"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      redraw = TRUE;
                    break;
                  }

                changed = TRUE;

                if (!strcasecmp (input, "default") || !strcasecmp (input, "d"))
                  {
                    opts->minimum_notes = 0;
                    redraw = TRUE;
                    break;
                  }

                temp = strtol (input, &tail, 0);
                if (temp == 0 && input == tail)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Want 'default' or a number"));
                        move (row + 2, 9);
                        clrtoeol ();
                        printw (_("<return> to leave unchanged"));
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Please enter 'default', a number, or <RET> "
                                  "to leave value unchanged."));
                      }
                  }
                else if ((temp == LONG_MAX && errno == ERANGE) ||
                         temp > INT_MAX)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Maximum valid value exceeded"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Maximum valid value exceeded."));
                      }
                  }
                else if (temp < 0)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Negative values not allowed"));
                        move (row + 2, 9);
                        clrtoeol ();
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Negative values not allowed."));
                      }
                  }
                else
                  {
                    opts->minimum_notes = temp;
                    redraw = TRUE;
                    break;
                  }
              }

            newts_free (prompt);
            break;
          }

        case 'l':  /* Change maximum note size. */
          {
            char *prompt, *input, *tail;
            long temp;

            if (traditional)
              {
                prompt = newts_nmalloc (strlen (_("New Maximum Message Size: ")) + 10,
                                        sizeof (char));
                strncpy (prompt, "         ", 10);
                strncat (prompt, _("New Maximum Message Size: "),
                         strlen (_("New Maximum Message Size: ")) + 1);
              }
            else
              prompt = newts_strdup (_("New Maximum Note Size: "));

            if (traditional)
              {
                clear ();
                display_director_screen (nf);
              }

            while (1)
              {
                if (traditional)
                  move (row, 0);
                else
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    move (LINES - 2, 0);
                    clrtoeol ();
                  }
                refresh ();
                input = gl_getline (prompt);
                input[strlen (input) - 1] = '\0';
                gl_histadd (input);

                if (strlen (input) == 0)
                  {
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Maximum Message Size Unchanged"));
                      }
                    else
                      redraw = TRUE;
                    break;
                  }

                changed = TRUE;

                temp = strtol (input, &tail, 0);
                if (!traditional)
                  temp *= 1024;
                if (temp == 0 && input == tail)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Enter an integer or <return>"));
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Please enter a number or <RET> to leave "
                                  "value unchanged."));
                      }
                  }
                else if ((temp == LONG_MAX && errno == ERANGE) ||
                         temp > HARDMAX)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Maximum valid value (%d) exceeded"),
                                HARDMAX);
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Maximum valid value (%dK) exceeded."),
                                HARDMAX / 1024);
                      }
                  }
                else if (temp < 0)
                  {
                    clear ();
                    display_director_screen (nf);
                    if (traditional)
                      {
                        move (row + 1, 9);
                        clrtoeol ();
                        printw (_("Negative values not allowed"));
                      }
                    else
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw (_("Negative values not allowed."));
                      }
                  }
                else
                  {
                    opts->maximum_note_size = temp;
                    redraw = TRUE;
                    break;
                  }
              }

            newts_free (prompt);
            break;
          }

        case 'E':  /* Toggle expiration action. */
          changed = TRUE;
          redraw = TRUE;
          switch (opts->expire_action)
            {
            case KEEPNO:
              opts->expire_action = KEEPYES;
              break;

            case KEEPYES:
              opts->expire_action = KEEPDFLT;
              break;

            case KEEPDFLT:
            default:
              opts->expire_action = KEEPNO;
              break;
            }
          break;

        /* I wish I could come up with a better name for this option. */

        case 'D':  /* Toggle expire-by-director-message. */
          changed = TRUE;
          redraw = TRUE;
          switch (opts->expire_by_dirmsg)
            {
            case DIRNOCARE:
              opts->expire_by_dirmsg = DIRON;
              break;

            case DIRON:
              opts->expire_by_dirmsg = DIROFF;
              break;

            case DIROFF:
              opts->expire_by_dirmsg = DIRANYON;
              break;

            case DIRANYON:
              opts->expire_by_dirmsg = DIRDFLT;
              break;

            case DIRDFLT:
              opts->expire_by_dirmsg = DIRNOCARE;
            default:
              break;
            }
          break;

        default:
          beep ();
          break;
        }
    }
}

/* display_director_screen - actually do the work of printing out the director
 * screen.
 *
 * Returns: the number of the last row printed on.
 */

static int
display_director_screen (struct notesfile *nf)
{
  struct uiuc_opts *opts;
  int row, column, lastrow;

  opts = (struct uiuc_opts *) nf->opts;

  if (traditional)
    {
      mvprintw (0, 20 + (40 - strlen (nf->title)) / 2 - 1, nf->title);
      mvprintw (1, 20 + (40 - strlen (nf->director_message)) / 2 - 1,
                nf->director_message);

      /* First column */

      row = 3; column = 0;
    }
  else
    {
      mvprintw (0, (COLS - strlen (nf->title)) / 2 - 1, nf->title);
      mvprintw (1, (COLS - strlen (nf->director_message)) / 2 - 1,
                nf->director_message);

      /* First column */

      row = 3; column = (COLS - 80) / 2 + 8;
    }

  mvprintw (row++, column, "(a) Anonymous:   ");
  printw (nf->options & NF_ANONYMOUS ? "ON" : "OFF");
  if (traditional)
    {
      mvprintw (row++, column, "(o) Notesfile:   ");
      printw (nf->options & NF_LOCKED ? "CLOSED" : "OPEN  ");
    }
  else
    {
      mvprintw (row++, column, "(o) Locked:      ");
      printw (nf->options & NF_LOCKED ? "YES" : "NO ");
    }
  /* 'Networked' option removed. */
  mvprintw (row++, column, "(A) Is Archive:  ");
  printw (nf->options & NF_ARCHIVE ? "YES" : "NO ");
  mvprintw (row++, column, "(M) Moderated:   ");
  printw (nf->options & NF_MODERATED ? "YES" : "NO ");
  /* 'Local' option removed. */
  mvprintw (row++, column, "(e) Expiration Threshold: ");
  switch (opts->expire_threshold)
    {
    case NEVER:
      printw ("Never");
      break;
    case 0:
      printw ("Default");
      break;
    default:
      printw (ngettext ("%d day", "%d days", opts->expire_threshold),
              opts->expire_threshold);
      break;
    }
  mvprintw (row++, column, "(E) Expiration Action:    ");
  switch (opts->expire_action)
    {
    case KEEPYES:
      if (traditional)
        printw ("ARCHIVE");
      else
        printw ("Archive");
      break;
    case KEEPNO:
      if (traditional)
        printw ("DELETE");
      else
        printw ("Delete");
      break;
    case KEEPDFLT:
      printw ("Default");
      break;
    default:
      if (traditional)
        printw ("UNKNOWN");
      else
        printw ("Unknown");
      break;
    }
  mvprintw (row++, column, "(D) Expire with Dirmsg:   ");
  switch (opts->expire_by_dirmsg)
    {
    case DIRNOCARE:
      if (traditional)
        printw ("NOCARE");
      else
        printw ("Ignore");
      break;
    case DIRON:
      if (traditional)
        printw ("ON");
      else
        printw ("Set");
      break;
    case DIROFF:
      if (traditional)
        printw ("OFF");
      else
        printw ("Not set");
      break;
    case DIRANYON:
      if (traditional)
        printw ("ANYON");
      else
        printw ("Set (N/R)");
      break;
    case DIRDFLT:
      printw ("Default");
      break;
    default:
      if (traditional)
        printw ("UNKNOWN");
      else
        printw ("Unknown");
      break;
    }
  mvprintw (row++, column, "(W) Working Set Size:     ");
  switch (opts->minimum_notes)
    {
    case 0:
      printw ("Default");
      break;
    default:
      if (traditional)
        printw (ngettext ("%d Note", "%d Notes", opts->minimum_notes),
                opts->minimum_notes);
      else
        printw (ngettext ("%d note", "%d notes", opts->minimum_notes),
                opts->minimum_notes);
      break;
    }
  if (traditional)
    mvprintw (row++, column, "(l) Maximum text/article: ");
  else
    mvprintw (row++, column, "(l) Maximum Note Size:    ");
  if (traditional)
    printw (ngettext ("%d byte", "%d bytes", opts->maximum_note_size),
            opts->maximum_note_size);
  else
    printw ("%d K", opts->maximum_note_size / 1024);

  lastrow = row;

  /* Second column */

  if (traditional)
    {
      row = 3;
      column = 39;
    }
  else
    {
      row = 3;
      column = (COLS - 80) / 2 + 48;
    }

  mvprintw (row++, column, "Policy Note Exists: ");
  printw (nf->options & NF_POLICY ? "YES" : "NO ");
  if (traditional)
    mvprintw (row++, column, "Next note in slot: ");
  else
    mvprintw (row++, column, "Next Note-in Slot:  ");
  printw ("%d", nf->total_notes + 1);
  if (traditional)
    mvprintw (row++, column, "Deleted Notes (holes): ");
  else
    mvprintw (row++, column, "Deleted Notes:      ");
  printw ("%d", opts->deleted_notes);
  if (traditional)
    mvprintw (row++, column, "Deleted Responses (holes): ");
  else
    mvprintw (row++, column, "Deleted Responses:  ");
  printw ("%d", opts->deleted_resps);

  if (traditional)
    {
      move (LINES - 2, 0);
      clrtoeol ();
      printw (_("Option: "));
    }

  return (lastrow > row) ? lastrow : row;
}

/* director_help - print help for director options screen. */

static void
director_help (void)
{
  int c, column;

  do
    {
      clear ();

      mvprintw (0, (COLS - 13) / 2 - 1, "Director Help");

      column = (COLS - 80) / 2 - 3;

      mvprintw (3, column + 14, "a        toggle anonymous notes");
      mvprintw (4, column + 14, "A        toggle archive status");
      mvprintw (5, column + 14, "c        compress the notesfile");
      mvprintw (6, column + 14, "D        change expire with director message option");
      mvprintw (7, column + 14, "C-d      quit notes");
      mvprintw (8, column + 14, "e        change expiration threshold");
      mvprintw (9, column + 14, "E        change expiration action");
      mvprintw (10, column + 14, "m        change director message");
      mvprintw (11, column + 14, "o        lock/unlock notesfile");
      mvprintw (12, column + 14, "p        view/edit access permissions");
      mvprintw (13, column + 14, traditional ?
                "q, k     return to index screen" :
                "q, k, i  return to index screen");
      if (!traditional)
        mvprintw (14, column + 14, "Q, K     return to index screen without saving changes");
      mvprintw (traditional ? 14 : 15, column + 14, "r, C-l   redraw the screen");
      mvprintw (traditional ? 15 : 16, column + 14, "t        change notesfile title");
      mvprintw (traditional ? 16 : 17, column + 14, "u        undelete a list of notes");
      mvprintw (traditional ? 17 : 18, column + 14, "w        write a new policy note");
      mvprintw (traditional ? 18 : 19, column + 14, "W        change minimum number of notes after archiving");
      mvprintw (traditional ? 19 : 20, column + 14, "z        delete a list of notes");
      mvprintw (traditional ? 20 : 21, column + 14, "!        fork a subshell");

      mvprintw (traditional ? 22 : 23, column + 20, "If you have suggestions for the help wording");
      mvprintw (traditional ? 23 : 24, column + 20, "or the layout, please pass them on to Tyler.");

      move (LINES - 2, 0);

      refresh ();

      c = getch ();
      while (c == KEY_RESIZE)
        c = getch ();
    }
  while (c == EOF);

  return;
}

/* multi_delete - either delete or undelete notes from FIRST to LAST in NF
 * according to MODE.
 */

static void
multi_delete (struct notesfile *nf, int first, int last, int mode)
{
  int which;

  if (first > last)
    {
      which = last;
      last = first;
      first = which;
    }
  if (first > nf->total_notes)
    first = nf->total_notes;
  if (first < 1)
    first = 1;
  if (last > nf->total_notes)
    last = nf->total_notes;
  if (last < 1)
    last = 1;
  for (which = first; which <= last; which++)
    {
      if (mode == DELETE)
        {
          struct newtref nr;
          nfref_copy (&nr.nfr, nf->ref);
          nr.notenum = which;
          nr.respnum = 0;

          delete_note (&nr);
        }
      else if (mode == UNDELETE)
        {
          struct newt note;
          memset (&note, 0, sizeof (struct newt));
          nfref_copy (&note.nr.nfr, nf->ref);
          note.nr.notenum = which;
          note.nr.respnum = 0;

          get_note (&note, FALSE);
          note.options &= ~NOTE_DELETED;
          time (&note.modified);
          modify_note (&note, UPDATE_TIMES);
        }
    }

  return;
}

/* frob - frob the given flag. */

static void
frob (struct notesfile *nf, int flag)
{
  if (nf->options & flag)
    nf->options &= ~flag;
  else
    nf->options |= flag;
}
