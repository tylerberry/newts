/*
 * read_note.c - read and handle reading notes and responses
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2006 Tyler Berry.
 *
 * Based in part on dsply.c and readem.c from the UIUC notes distribution by
 * Ray Essick and Rob Kolstad.  Any work derived from this source code is
 * required to retain this notice.
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

/****************************************************************************
 T O P  R E A S O N  W H Y  T H I S  F I L E  I S  E V I L  I N C A R N A T E

Is it because of its crushing length and sheer incomprehensibility? No.

Is it because get_note() is called no fewer than twice for every single
response we ever view? No.

It's because there are more screen displays than they ever should have made.
We don't know who they were.  We don't know who they are.  We certainly don't
know what they were thinking.  But one thing's for sure -- they never should
have made that much gun.  Er, screen displays.

The way to fix this is to rewrite this entire section (and probably the rest
of the Notes program) to separate the window into two Curses windows, the
main window and the "status-bar" window at the bottom.  That would let us
avoid all the nasty redisplays.
 ****************************************************************************/

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "notes.h"
#include "pager.h"

#include "dirname.h"
#include "gl_getline.h"
#include "which.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#if STDC_HEADERS
# include <signal.h>
#endif

#define BUFSIZE 4096   /* Display buffer */

static int display_note (struct notesfile *nf, struct newt *np,
                         struct pager *pager);
static void read_help (void);
static int save_text (struct newt *np, const char *filename);

/* read_note - the big one.  This controls the entire control interface while
 * reading a note.  The note is NOTENUM/RESPNUM in NF; FIRST is used to pass
 * back to the index which note we'd like to have on top.  SUPPRESS_BLACKLIST
 * is passed in from the index; it tells us if we want to suppress the
 * blacklist, and is also used to tell us if we have a keystroke we'd like to
 * process.
 *
 * Returns: -1 for redraw index page, or NEXTSEQ, NEXTNOSEQ, QUITSEQ, or
 * QUITNOSEQ.
 */

int
read_note (struct notesfile *nf, int *first, int notenum, int respnum,
           short suppress_blacklist)
{
  struct pager pager;
  struct newt note;
  short forward = TRUE;       /* Moving forward or ... */
  short notechanged = FALSE;  /* Changed to a diff. note, controls stats */
  short reload = TRUE;        /* Reload the note from disk */
  short redraw = TRUE;        /* Redraw the screen */
  short topbounce, bottombounce;
  int skipprompt;
  int c = suppress_blacklist; /* Secret code for 'no keystrokes yet' */

  memset (&note, 0, sizeof (note));

  /* If we had '/' passed in as the value of SUPPRESS_BLACKLIST, it means we
   * are handling a text search initiated on the index screen.  Therefore, we
   * need to scroll forward to the location of the searched-for text and
   * highlight it.
   */

  /* Sanity checks. */

  if (notenum > 0 && nf->total_notes == 0)
    return 0;

  if (notenum > nf->total_notes)
    notenum = nf->total_notes;

  if (notenum == 0 && !(nf->options & NF_POLICY))
    return 0;

  /* Initial prep-work; we need to access the basenote for information so that
   * our bounds-checking is sane.
   */

  nfref_copy (&note.nr.nfr, nf->ref);
  note.nr.notenum = notenum;
  note.nr.respnum = 0;
  if (respnum < 0)
    respnum = 0;
  get_note (&note, respnum ? FALSE : TRUE);

  /* Now get the real note. */

  if (respnum)
    {
      if (respnum > note.total_resps)
        respnum = note.total_resps;
      note.nr.respnum = respnum;
      get_note (&note, TRUE);
    }

  /* Now that everything's set up, we can enter the main loop. */

  while (TRUE)
    {
      if (notenum > nf->total_notes)
        notenum = nf->total_notes;

      if (notechanged)
        {
          /* We can skip this entire section if we're in the same note. */

          note.nr.notenum = notenum;
          note.nr.respnum = 0;
          if (respnum < 0)
            respnum = 0;

          /* Be careful clearing the note structure. */

          if (note.text != NULL)
            {
              free (note.text);
              note.text = NULL;
            }
          if (note.title != NULL)
            {
              free (note.title);
              note.title = NULL;
            }
          if (note.director_message != NULL)
            {
              free (note.director_message);
              note.director_message = NULL;
            }
          if (note.auth.name != NULL)
            {
              free (note.auth.name);
              note.auth.name = NULL;
            }
          if (note.auth.system != NULL)
            {
              free (note.auth.system);
              note.auth.system = NULL;
            }
          if (note.id.system != NULL)
            {
              free (note.id.system);
              note.id.system = NULL;
            }

          get_note (&note, respnum ? FALSE : notechanged);
          if (respnum)
            {
              if (respnum > note.total_resps)
                respnum = note.total_resps;
              note.nr.respnum = respnum;

              if (note.text != NULL)
                {
                  free (note.text);
                  note.text = NULL;
                }
              if (note.title != NULL)
                {
                  free (note.title);
                  note.title = NULL;
                }
              if (note.director_message != NULL)
                {
                  free (note.director_message);
                  note.director_message = NULL;
                }
              if (note.auth.name != NULL)
                {
                  free (note.auth.name);
                  note.auth.name = NULL;
                }
              if (note.auth.system != NULL)
                {
                  free (note.auth.system);
                  note.auth.system = NULL;
                }
              if (note.id.system != NULL)
                {
                  free (note.id.system);
                  note.id.system = NULL;
                }

              get_note (&note, notechanged);
            }
        }

      topbounce = FALSE;
      bottombounce = FALSE;

      if (note.options & NOTE_DELETED && !allowed (nf, DIRECTOR))
        {
          if (notenum == 0)   /* Policy note */
            return 0;
          if (forward)
            {
              if (++notenum > nf->total_notes)
                {
                  /* Bounced off the top. */
                  if (bottombounce)
                    {
                      notenum++;
                      continue;
                    }
                  topbounce = TRUE;
                  notenum = nf->total_notes - 1;
                  forward = FALSE;
                }
              respnum = 0;
              notechanged = TRUE;
              continue;
            }
          else
            {
              if (--notenum < 1)
                {
                  /* Bounced off the bottom. */
                  if (topbounce)
                    {
                      notenum++;
                      continue;
                    }
                  bottombounce = TRUE;
                  notenum = 2;
                  forward = TRUE;
                }
              respnum = 0;
              notechanged = TRUE;
              continue;
            }
        }

      initialize_pager (&pager, note.text);

      set_highlight (&pager, (c == '/' || c == '\\') ? TRUE : FALSE);

      /* Set up variable defaults prior to the secondary loop. */

      forward = TRUE;
      notechanged = FALSE;
      reload = FALSE;
      skipprompt = FALSE;

      /* The next section handles blacklisting; what exactly we do is dependant
       * on what the most recent keystroke was.
       */

      if (blacklisted (&note))
        {
          switch (c)
            {
            case 0:    /* No keystroke; we just entered from the index. */
              /* Fallthrough */

            case ' ':
              c = ';'; /* We don't just want to go to the next page, we
                        * actually want the next note, so we alter the command
                        * to reflect that.
                        */
              skipprompt = TRUE;
              break;;

            case 'g':
              c = 'l'; /* As above, during blacklist logic, 'g' is equivalent
                        * to 'l'.
                        */
              skipprompt = TRUE;
              break;

            case 'j':
              if (black_skip_seq)
                c = 'J';
              skipprompt = TRUE;
              break;;

            case 'l':
              if (black_skip_seq)
                c = 'L';
              /* Fallthrough. */

            case '\n': case '\r': case '+': case ';': case '-': case 'J':
            case 'L': case '\b': case '/': case '\\': case '\177':
            case '1': case '2': case '3': case '4': case '5': case '6':
            case '7': case '8': case '9': case KEY_BACKSPACE: case KEY_ENTER:
              skipprompt = TRUE;
              break;

            /* All other commands, notably [aAxA=*], ignore the blacklist. */

            default:
            case 1:    /* Blacklist suppressed. */
              break;
            }
        }

      /* This is the secondary loop; it handles keystroke processing. */

      do
        {
          if (!skipprompt)
            {
              if (redraw)
                display_note (nf, &note, &pager);
              redraw = TRUE;   /* We usually have to. */

              move (LINES - 1, 0);
#ifdef PROMPT
              printw (PROMPT);
#endif
              c = getch ();
            }

          switch (c)
            {
            case KEY_NPAGE:
            case KEY_DOWN:
              page_down (&pager);
              break;

            case ' ':  /* Next page, response, or note. */
              if (page_down (&pager))
                break;
              /* Fallthrough */

            case '+':  /* Next response or note. */
            case ';':
              if (notenum == 0)   /* Policy note */
                {
                  free_pager (&pager);
                  return 0;
                }
              respnum++;
              if (respnum > note.total_resps)
                {
                  if (++notenum > nf->total_notes)
                    {
                      *first = nf->total_notes - LINES + 13;
                      free_pager (&pager);
                      return 0;
                    }
                  respnum = 0;
                }
              notechanged = TRUE;
              reload = TRUE;
              break;

            case '\n':  /* Next note. */
            case '\r':
            case KEY_ENTER:
              if (notenum == 0)   /* Policy note */
                {
                  free_pager (&pager);
                  return 0;
                }
              if (++notenum > nf->total_notes)
                {
                  *first = nf->total_notes - LINES + 13;
                  free_pager (&pager);
                  return 0;
                }
              respnum = 0;
              notechanged = TRUE;
              reload = TRUE;
              break;

            case KEY_UP:  /* Previous page. */
            case KEY_PPAGE:
              page_up (&pager);
              break;

            case '-':  /* Previous response or note. */
            case '\b':
            case '\177':   /* ASCII DEL - used by xterm */
            case KEY_BACKSPACE:
              if (page_up (&pager))
                break;
              if (respnum)
                {
                  respnum--;
                  notechanged = TRUE;
                  reload = TRUE;
                  break;
                }
              if (notenum == 0)   /* Policy note */
                {
                  free_pager (&pager);
                  return 0;
                }
              forward = FALSE;
              if (--notenum < 1)
                {

                  /* The old client will remain on Note 1 indefinitely if you
                   * hold down the backspace key, but will loop back to the
                   * index if you go the other direction (with return, etc).
                   * This is inconsistent; when not in traditional mode we
                   * return to the index if we get a backspace on note 1.
                   */

                  if (traditional)
                    {
                      notenum = 1;
                      reload = TRUE;
                      break;
                    }
                  else
                    {
                      *first = 1;
                      free_pager (&pager);
                      return 0;
                    }
                }

              notechanged = TRUE;
              reload = TRUE;
              break;

            case '=':  /* Basenote. */
              if (respnum != 0)
                notechanged = TRUE;
              respnum = 0;
              reload = TRUE;
              break;

            case '*':  /* Last response. */
              if (respnum != note.total_resps)
                notechanged = TRUE;
              respnum = note.total_resps;
              reload = TRUE;
              break;

            case '1': case '2': case '3': case '4': case '5':
            case '6': case '7': case '8': case '9':

              /* Skip forward <n> responses. */

              if (note.total_resps < 1)
                {
                  if (++notenum > nf->total_notes)
                    {
                      *first = nf->total_notes - LINES + 13;
                      free_pager (&pager);
                      return 0;
                    }
                  respnum = 0;
                  notechanged = TRUE;
                }
              else
                {
                  if (respnum != note.total_resps)
                    {
                      notechanged = TRUE;
                      respnum += c - '0';
                      if (respnum > note.total_resps)
                        respnum = note.total_resps;
                    }
                }
              reload = TRUE;
              break;

            case 'R':
              if (!(note.options & NOTE_CORRUPTED ||
                    (note.options & NOTE_UNAPPROVED &&
                     !allowed (nf, DIRECTOR))))
                toggle_rot13 (&pager);
              arrange_replot (&pager);
              break;

            case '!':
              spawn_subshell ();
              arrange_replot (&pager);
              break;

            case '?':
              read_help ();
              arrange_replot (&pager);
              break;

            case 'r':
            case '\f':
            case EOF:
            case KEY_LEFT:
            case KEY_RIGHT:
            case KEY_RESIZE:
              arrange_replot (&pager);
              break;

            case 'i':  /* Return to index. */
            case '\033':
              *first = notenum;
              free_pager (&pager);
              return 0;

            case 'd':  /* Toggle director message. */
              if (allowed (nf, DIRECTOR))
                {
                  if (note.options & NOTE_CORRUPTED)
                    {
                      move (LINES - 1, 0);
                      clrtoeol ();
                      if (traditional)
                        mvprintw (LINES - 1, 14,
                                  _("You cannot modify a corrupted note."));
                      else
                        printw (_("You cannot modify a corrupted note."));
                      redraw = FALSE;
                      break;
                    }

                  if (note.director_message == NULL)
                    {
                      note.director_message = newts_strdup (nf->director_message);
                    }
                  else
                    {
                      newts_free (note.director_message);
                      note.director_message = NULL;
                    }
                  time (&note.modified);
                  modify_note (&note, UPDATE_TIMES);
                  arrange_replot (&pager);
                  break;
                }
              else
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  mvprintw (LINES - 1, traditional ? 14 : 0,
                            _("Anonymous: %s   Moderated: %s"),
                            (nf->options & NF_ANONYMOUS) ? "YES" : "NO ",
                            (nf->options & NF_MODERATED) ? "YES" : "NO ");
                  redraw = FALSE;
                  break;
                }

           case ':':  /* Toggle moderation status. */
             if (allowed (nf, DIRECTOR) &&
                 (nf->options & NF_MODERATED ||
                  note.options & NOTE_UNAPPROVED))
               {
                 if (note.options & NOTE_CORRUPTED)
                   {
                     move (LINES - 1, 0);
                     clrtoeol ();
                     if (traditional)
                       mvprintw (LINES - 1, 14,
                                 _("You cannot modify a corrupted note."));
                     else
                       printw (_("You cannot modify a corrupted note."));
                     redraw = FALSE;
                     break;
                   }

                 if (note.options & NOTE_UNAPPROVED)
                   note.options &= ~NOTE_UNAPPROVED;
                 else
                   note.options |= NOTE_UNAPPROVED;
                 time (&note.modified);
                 modify_note (&note, UPDATE_TIMES);
                 arrange_replot (&pager);
                 break;
               }
             break;

            case 'e':  /* Edit note title. */
              if (note.options & NOTE_CORRUPTED)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("You cannot modify a corrupted note."));
                  else
                    printw (_("You cannot modify a corrupted note."));
                  redraw = FALSE;
                  break;
                }
              if (notenum == 0)
                {
                  if (traditional)
                    continue;
                  else
                    {
                      move (LINES - 1, 0);
                      clrtoeol ();
                      printw (_("You cannot edit the policy note's title."));
                      redraw = FALSE;
                      break;
                    }
                }
              if (respnum)
                {
                  beep ();
                  arrange_replot (&pager);
                  break;
                }
              if (allowed (nf, DIRECTOR) ||
                  (note.auth.uid == euid &&
                   strcmp (note.auth.system, fqdn) == 0))
                {
                  char *temp;
                  char *prompt = traditional ? _("New Title: ") : _("New title: ");

                  if (note.options & NOTE_UNAPPROVED &&
                      !allowed (nf, DIRECTOR))
                    {
                      move (LINES - 1, 0);
                      clrtoeol ();
                      if (traditional)
                        mvprintw (LINES - 1, 14,
                                  _("You cannot modify a note which is awaiting "
                                    "director approval."));
                      else
                        printw (_("You cannot modify a note which is awaiting "
                                  "director approval."));
                      redraw = FALSE;
                      break;
                    }

                  move (LINES - 1, 0);
                  clrtoeol ();
                  refresh ();
                  temp = gl_getline (prompt);
                  temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                  if (strlen (temp) == 0)
                    {
                      arrange_replot (&pager);
                      break;
                    }
                  else
                    {
                      newts_free (note.title);
                      note.title = newts_strdup (temp);
                      gl_histadd (note.title);
                    }
                  time (&note.modified);
                  modify_note (&note, UPDATE_TIMES);
                  newts_free (note.title);
                  note.title = NULL;
                  notechanged = TRUE;
                  reload = TRUE;
                  break;
                }
              else
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14, _("Not your note"));
                  else
                    printw (_("You are not allowed to edit "
                              "somebody else's note."));

                  redraw = FALSE;
                  break;
                }

            case 'E':  /* Edit note text. */
              if (note.options & NOTE_CORRUPTED)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("You cannot modify a corrupted note."));
                  else
                    printw (_("You cannot modify a corrupted note."));
                  redraw = FALSE;
                  break;
                }

              if (notenum == 0) /* Respnum ought to == 0 in this case. */
                {
                  arrange_replot (&pager);
                  break;
                }

              /* Nobody can edit anonymous posts. */

              if (strcasecmp (note.auth.name, "anonymous") == 0)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14, respnum ? _("Not your response") :
                              _("Not your note"));
                  else
                    printw (_("You are not allowed to edit an anonymous "
                              "note."));

                  redraw = FALSE;
                  break;
                }

              if (euid != note.auth.uid ||
                  strcmp (fqdn, note.auth.system))
                {
                  if (respnum)
                    {
                      move (LINES - 1, 0);
                      clrtoeol ();
                      if (traditional)
                        mvprintw (LINES - 1, 14, _("Not your response"));
                      else
                        printw (_("You are not allowed to edit "
                                  "somebody else's response."));

                      redraw = FALSE;
                      break;
                    }
                  else
                    {
                      move (LINES - 1, 0);
                      clrtoeol ();
                      if (traditional)
                        mvprintw (LINES - 1, 14, _("Not your note"));
                      else
                        printw (_("You are not allowed to edit "
                                  "somebody else's note."));

                      redraw = FALSE;
                      break;
                    }
                }

              if (note.options & NOTE_UNAPPROVED &&
                  !allowed (nf, DIRECTOR))
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("You cannot modify a note which is awaiting "
                                "director approval."));
                  else
                    printw (_("You cannot modify a note which is awaiting "
                              "director approval."));
                  redraw = FALSE;
                  break;
                }

              if (respnum && respnum != note.total_resps)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("Can't edit; not last response"));
                  else
                    printw (_("You are not allowed to edit any response "
                              "other than the last."));

                  redraw = FALSE;
                  break;
                }

              if (!respnum && note.total_resps)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("Can't edit; note has responses"));
                  else
                    printw (_("You are not allowed to edit a note which "
                              "has responses."));

                  redraw = FALSE;
                  break;
                }

              {
                struct newt edit;
                memcpy (&edit, &note, sizeof (struct newt));

                edit.text = get_text (&note, EDIT);

                if (edit.text == NULL)
                  {
                    arrange_replot (&pager);
                    break;
                  }

                if (nf->options & NF_ANONYMOUS)
                  {
                    int yn;

                    clear ();
                    YES_OR_NO (yn,
                               mvprintw (LINES - 1, 0, traditional ?
                                         _("Do you wish this to be anonymous (y/n): ") :
                                         _("Make this note anonymous? (y/n): "));
                               );
                    if (yn)
                      {
                        if (traditional)
                          {
                            clear ();
                            YES_OR_NO (yn,
                                       mvprintw (LINES - 1, 0,
                                                 _("Do you wish REALLY this to be anonymous (y/n): "));
                                       );
                            if (yn)
                              edit.options |= NOTE_ANONYMOUS;
                            else
                              edit.options &= ~NOTE_ANONYMOUS;
                          }
                        else
                          edit.options |= NOTE_ANONYMOUS;
                      }
                    else
                      edit.options &= ~NOTE_ANONYMOUS;
                  }

                edit.director_message = NULL;

                if (allowed (nf, DIRECTOR))
                  {
                    int yn;

                    clear ();
                    YES_OR_NO (yn,
                               mvprintw (LINES - 1, 0, traditional ?
                                         _("Director message (y/n): ") :
                                         _("Director message? (y/n): "));
                               );
                    if (yn)
                      edit.director_message = "t";
                  }

                if (edit.options & NOTE_ANONYMOUS)
                  edit.auth.name = "Anonymous";
                else
                  edit.auth.name = username;
                edit.auth.system = fqdn;
                if (edit.options & NOTE_ANONYMOUS)
                  edit.auth.uid = anon_uid;
                else
                  edit.auth.uid = euid;
                if (!allowed (nf, READ))
                  edit.options |= NOTE_WRITE_ONLY;

                time (&edit.modified);
                modify_note_text (&edit);
              }

              notechanged = TRUE;
              reload = TRUE;
              break;

            case 'w':  /* Write a new response. */
            case 'W':  /* Same, with quoting. */
              if (note.options & NOTE_CORRUPTED)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("You cannot reply to a corrupted note."));
                  else
                    printw (_("You cannot reply to a corrupted note."));
                  redraw = FALSE;
                  break;
                }
              if (note.options & NOTE_UNAPPROVED &&
                  !allowed (nf, DIRECTOR))
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("You cannot reply to a note which is awaiting "
                                "director approval."));
                  else
                    printw (_("You cannot reply to a note which is awaiting "
                              "director approval."));
                  redraw = FALSE;
                  break;
                }

              if (!allowed (nf, REPLY))
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("Sorry, you are not allowed to write"));
                  else
                    printw (_("You are not allowed to reply in this notesfile."));
                  redraw = FALSE;
                  break;
                }

              if (notenum == 0)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14, "%s",
                              _("No responses allowed to policy note"));
                  else
                    printw ("%s", _("No responses allowed for policy note"));
                  redraw = FALSE;
                  break;
                }

              {
                int temp;
                temp = compose_note (nf, c == 'W' ? &note : NULL,
                                     _("Edit Response Text:"), NULL, notenum,
                                     NORMAL);
                if (temp == -1)
                  {
                    arrange_replot (&pager);
                    break;
                  }
                if (temp > 0)
                  update_nf (nf);
                if (temp)
                  respnum = temp;
              }

              reload = TRUE;
              notechanged = TRUE;
              break;

            case 'B':  /* Bitch, bitch, bitch. */
              {
                newts_nfref *gripesref = nfref_alloc ();
                struct notesfile gripes;
                memset (&gripes, 0, sizeof (struct notesfile));

                nfref_set_user (gripesref, username);
                nfref_set_system (gripesref, fqdn);
                nfref_set_owner (gripesref, NULL);
                nfref_set_name (gripesref, "newts.gripes");

                move (LINES - 1, 0);
                clrtoeol ();

                open_nf (gripesref, &gripes);
                compose_note (&gripes, NULL, traditional ? _("Edit Gripe text:") :
                              _("Edit Gripe Text:"), traditional ?
                              _("Gripe Header: ") : _("Gripe Title: "), -1,
                              NORMAL);
                close_nf (&gripes, FALSE);
                nfref_free (gripesref);

                arrange_replot (&pager);
                break;
              }

            case 'N':
              move (LINES - 1, 0);
              clrtoeol ();
              printw (_("Archives are currently unsupported."));
              redraw = FALSE;
              break;

            case 'n':  /* Nest to a different notesfile. */
              {
                List nestlist;
                char *nfname, *prompt, *tildename;
                int result;

                move (LINES - 1, 0);
                clrtoeol ();

                if (traditional)
                  {
                    move (LINES - 2, 0);
                    clrtoeol ();
                    prompt = newts_nmalloc (12 + strlen (_("New notesfile: ")),
                                            sizeof (char));
                    strcpy (prompt, "           ");
                    strcat (prompt, _("New notesfile: "));
                  }
                else
                  prompt = newts_strdup (_("New Notesfile: "));

                refresh ();

                nfname = gl_getline (prompt);
                free (prompt);

                nfname[strlen (nfname) - 1] = '\0'; /* Chomp trailing '\n' */
                if (strlen (nfname) == 0)
                  {
                    arrange_replot (&pager);
                    break;
                  }
                tildename = tilde_expand (nfname);
                gl_histadd (nfname);

                list_init (&nestlist,
                           (void * (*) (void)) nfref_alloc,
                           (void (*) (void *)) nfref_free,
                           NULL);
                parse_nf (tildename, &nestlist);

                free (tildename);

                {
                  ListNode *node = list_head (&nestlist);

                  while (node)
                    {
                      newts_nfref *nestref = (newts_nfref *) list_data (node);

                      result = master (nestref);

                      node = list_next (node);

                      if (result == QUITSEQ)
                        {
                          list_destroy (&nestlist);
                          free_pager (&pager);
                          return QUITSEQ;
                        }
                      if (result == QUITNOSEQ)
                        {
                          list_destroy (&nestlist);
                          free_pager (&pager);
                          return QUITNOSEQ;
                        }
                    }
                }

                list_destroy (&nestlist);
                update_nf (nf);
                arrange_replot (&pager);
                break;
              }

            case 'a':  /* Search for a particular author. */
            case 'A':
              {
                char *prompt;
                char *temp;
                struct newtref searchref;
                int found;

                if (notenum == 0)  /* Not from policy note */
                  {
                    free_pager (&pager);
                    return 0;
                  }

                memset (&searchref, 0, sizeof (struct newtref));
                nfref_copy (&searchref.nfr, nf->ref);
                searchref.notenum = notenum;
                searchref.respnum = respnum;

                if (searchref.respnum == note.total_resps)
                  {
                    searchref.notenum--;
                    searchref.respnum = 0;
                  }
                else
                  searchref.respnum++;

                if (c == 'a' || asearch == NULL || *asearch == '\0')
                  {
                    if (traditional)
                      prompt = newts_strdup (_("Search author: "));
                    else
                      prompt = newts_strdup (_("Search by author: "));
                    move (traditional ? LINES - 2 : LINES - 1, 0);
                    clrtoeol ();
                    refresh ();
                    temp = gl_getline (prompt);
                    newts_free (prompt);

                    temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                    if (strlen (temp) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    else
                      {
                        asearch = newts_nrealloc (asearch, strlen (temp) + 1,
                                                  sizeof (char));
                        strcpy (asearch, temp);
                        gl_histadd (asearch);
                      }
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                  }

                move (traditional ? LINES - 2 : LINES - 1, 0);
                printw (_("Searching for articles by %s"), asearch);
                refresh ();

                found = author_search (&searchref, asearch);
                if (found == -1)
                  {
                    move (LINES - 1, 0);
                    if (!traditional)
                      clrtoeol ();
                    printw (_("Can't find any articles by %s"), asearch);
                    refresh ();
                    redraw = FALSE;
                    break;
                  }
                notenum = found;
                respnum = searchref.respnum;
                reload = TRUE;
                notechanged = TRUE;
                break;
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
                searchref.notenum = notenum - 1;
                searchref.respnum = 0;

                if (c == 'x' || tsearch == NULL || *tsearch == '\0')
                  {
                    if (traditional)
                      prompt = newts_strdup (_("Search String: "));
                    else
                      prompt = newts_strdup (_("Search by title: "));
                    move (traditional ? LINES - 2 : LINES - 1, 0);
                    clrtoeol ();
                    refresh ();
                    temp = gl_getline (prompt);
                    newts_free (prompt);

                    temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                    if (strlen (temp) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    else
                      {
                        tsearch = newts_nrealloc (tsearch, strlen (temp) + 1,
                                                  sizeof (char));
                        strcpy (tsearch, temp);
                        gl_histadd (tsearch);
                      }
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                  }
                move (traditional ? LINES - 2 : LINES - 1, 0);
                if (!traditional)
                  printw (_("Searching for articles titled '%s'"), tsearch);
                refresh ();
                found = title_search (&searchref, tsearch);
                if (found == -1)
                  {
                    move (LINES - 1, 0);
                    if (traditional)
                      printw (_("%s: Not Found"), tsearch);
                    else
                      {
                        clrtoeol ();
                        printw (_("Can't find any articles titled '%s'"),
                                tsearch);
                      }
                    refresh ();
                    redraw = FALSE;
                    break;
                  }
                notenum = found;
                respnum = 0;
                reload = TRUE;
                notechanged = TRUE;
                break;
              }

            case '/':  /* Text search. */
            case '\\':
              {
                char *prompt;
                char *temp;
                struct newtref searchref;
                int found;

                save_position (&pager);

                memset (&searchref, 0, sizeof (struct newtref));
                nfref_copy (&searchref.nfr, nf->ref);
                searchref.notenum = notenum;
                searchref.respnum = respnum;

                if (c == '/' || txtsearch == NULL || *txtsearch == '\0')
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      prompt = newts_strdup (_("Search String: "));
                    else
                      prompt = newts_strdup (_("Search for text: "));
                    move (traditional ? LINES - 2 : LINES - 1, 0);
                    clrtoeol ();
                    refresh ();
                    temp = gl_getline (prompt);
                    newts_free (prompt);

                    temp[strlen (temp) - 1] = '\0'; /* Chomp trailing \n */
                    if (strlen (temp) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    else
                      {
                        char *savesearch = NULL;

                        if (txtsearch != NULL)
                          savesearch = newts_strdup (txtsearch);

                        txtsearch = newts_nrealloc (txtsearch, strlen (temp) + 1,
                                                    sizeof (char));
                        strcpy (txtsearch, temp);
                        gl_histadd (txtsearch);

                        if (pager.highlight && strcmp (savesearch, txtsearch) == 0)
                          {
                            int highlighted;

                            if (pager.pagesout < 50)
                              pager.pagecnt[++pager.pagesout] = pager.bufptr;
                            highlighted = display_note (nf, &note, &pager);
                            if (highlighted == TRUE)
                              {
                                arrange_replot (&pager);
                                break;
                              }
                            else
                              {
                                if (searchref.respnum == note.total_resps)
                                  {
                                    searchref.notenum--;
                                    searchref.respnum = 0;
                                  }
                                else
                                  searchref.respnum++;
                                restore_position (&pager);
                                clear ();
                                display_note (nf, &note, &pager);
                              }
                          }

                        newts_free (savesearch);
                      }
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                  }
                else
                  {
                    int highlighted;

                    if (pager.pagesout < 50)
                      pager.pagecnt[++pager.pagesout] = pager.bufptr;
                    highlighted = display_note (nf, &note, &pager);
                    if (highlighted == TRUE)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    else
                      {
                        if (searchref.respnum == note.total_resps)
                          {
                            searchref.notenum--;
                            searchref.respnum = 0;
                          }
                        else
                          searchref.respnum++;
                        restore_position (&pager);
                        clear ();
                        display_note (nf, &note, &pager);
                      }
                  }

                move (traditional ? LINES - 2 : LINES - 1, 0);
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
                      printw (_("Can't find any notes with text '%s' after "
                                "this note"), txtsearch);
                    refresh ();
                    redraw = FALSE;
                    break;
                  }
                notenum = found;
                respnum = searchref.respnum;
                reload = TRUE;
                notechanged = TRUE;
                set_highlight (&pager, TRUE);
                break;
              }

            case 'D':  /* Delete your own note. */
              {
                int yn;

                if (note.options & NOTE_CORRUPTED)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14,
                                _("Corrupted notes are automatically "
                                  "deleted."));
                    else
                      printw (_("Corrupted notes are automatically deleted."));
                    redraw = FALSE;
                    break;
                  }
                if (notenum == 0)
                  {
                    if (allowed (nf, DIRECTOR))
                      {
                        move (LINES - 1, 0);
                        clrtoeol ();
                        if (traditional)
                          {
                            if (note.auth.uid != euid ||
                                strcmp (note.auth.system, fqdn) != 0)
                              mvprintw (LINES - 1, 14, _("Not your note"));
                            else
                              mvprintw (LINES - 1, 14,
                                        _("Use 'Z' to delete policy"));
                          }
                        else
                          printw (_("Use the 'Z' key to delete the policy "
                                    "note."));
                        redraw = FALSE;
                        break;
                      }
                    else
                      {
                        move (LINES - 1, 0);
                        clrtoeol ();
                        if (traditional)
                          mvprintw (LINES - 1, 14, _("Not your note"));
                        else
                          printw (_("You are not allowed to delete the "
                                    "policy note."));
                        redraw = FALSE;
                        break;
                      }
                  }
                if (note.auth.uid != euid ||
                    strcmp (note.auth.system, fqdn) != 0)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14, respnum ?
                                _("Not your response") : _("Not your note"));
                    else
                      printw (respnum ?
                              _("You are not allowed to delete somebody else's "
                                "response.") :
                              _("You are not allowed to delete somebody else's "
                                "note."));
                    redraw = FALSE;
                    break;
                  }
                if (!traditional && respnum && respnum != note.total_resps)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14,
                                _("Can't delete; not last response"));
                    else
                      printw (_("You are not allowed to delete any "
                                "response other than the last."));
                    redraw = FALSE;
                    break;

                  }
                if (!traditional && respnum == 0 && note.total_resps)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14,
                                _("Can't delete; note has responses"));
                    else
                      printw (_("You are not allowed to delete a note "
                                "with responses."));
                    redraw = FALSE;
                    break;
                  }

                move (LINES - 1, 0);
                clrtoeol ();
                YES_OR_NO (yn,
                           mvprintw (LINES - 1, 0, traditional ? _("Delete? (y/n): ") :
                                     respnum ? _("Delete response? (y/n): ") :
                                     _("Delete note? (y/n): "));
                           );
                if (yn)
                  {
                    if (respnum)
                      {
                        if (respnum != note.total_resps)
                          {
                            move (LINES - 1, 0);
                            clrtoeol ();
                            if (traditional)
                              mvprintw (LINES - 1, 14,
                                        _("Can't delete; not last response"));
                            else
                              printw (_("You are not allowed to delete any "
                                        "response other than the last."));
                            redraw = FALSE;
                            break;

                          }
                        delete_note (&note.nr);
                        update_nf (nf);
                        respnum--;
                        notechanged = TRUE;
                        reload = TRUE;
                        break;
                      }
                    else
                      {
                        if (note.total_resps)
                          {
                            move (LINES - 1, 0);
                            clrtoeol ();
                            if (traditional)
                              mvprintw (LINES - 1, 14,
                                        _("Can't delete; note has responses"));
                            else
                              printw (_("You are not allowed to delete a note "
                                        "with responses."));
                            redraw = FALSE;
                            break;
                          }
                        delete_note (&note.nr);
                        update_nf (nf);
                        notenum++;
                        if (notenum > nf->total_notes)
                          {
                            free_pager (&pager);
                            return 0;
                          }
                        respnum = 0;
                        notechanged = TRUE;
                        reload = TRUE;
                        break;
                      }
                  }
                else
                  {
                    arrange_replot (&pager);
                    break;
                  }
              }

            case 'Z':  /* As a director, delete any note. */
              {
                int yn;

                if (!allowed (nf, DIRECTOR))
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14, _("Not a director"));
                    else
                      printw (_("You are not a director; "
                                "only directors can use 'Z'."));
                    redraw = FALSE;
                    break;
                  }

                if (note.options & NOTE_CORRUPTED)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    if (traditional)
                      mvprintw (LINES - 1, 14,
                                _("Corrupted notes are automatically deleted."));
                    else
                      printw (_("Corrupted notes are automatically deleted."));
                    redraw = FALSE;
                    break;
                  }

                move (LINES - 1, 0);
                clrtoeol ();
                YES_OR_NO (yn,
                           mvprintw (LINES - 1, 0, traditional ? _("Delete? (y/n): ") :
                                     respnum ? _("Delete response? (y/n): ") :
                                     _("Delete note? (y/n): "));
                           );
                if (yn)
                  {
                    delete_note (&note.nr);
                    update_nf (nf);

                    if (respnum)
                      {
                        if (respnum == note.total_resps)
                          respnum--;
                        else
                          respnum++;
                      }
                    else
                      {
                        notenum++;
                        if (notenum > nf->total_notes)
                          {
                            free_pager (&pager);
                            return 0;
                          }
                        respnum = 0;
                      }
                    notechanged = TRUE;
                    reload = TRUE;
                    break;
                  }
              }

            case 'g':  /* Next page, or continue seqing. */
              if (page_down (&pager))
                break;
              /* Fallthrough */

            case 'j':  /* Seq to next response. */
            case 'l':  /* Or quit. */
              if (notenum <= 0)
                {
                  free_pager (&pager);
                  return 0;
                }
              if (respnum != note.total_resps)
                {
                  struct newtref nr;
                  memset (&nr, 0, sizeof (struct newtref));
                  nfref_copy (&nr.nfr, nf->ref);
                  nr.notenum = notenum;
                  nr.respnum = respnum;

                  if ((respnum =
                       get_next_resp (&nr, seqtime)) > 0)
                    {
                      reload = TRUE;
                      notechanged = TRUE;
                      break;
                    }
                }
              /* Fallthrough */

            case 'J':  /* Seq to next note. */
            case 'L':  /* Or quit. */
              {
                struct newtref nr;

                if (notenum <= 0)
                  {
                    free_pager (&pager);
                    return 0;
                  }

                memset (&nr, 0, sizeof (struct newtref));
                nfref_copy (&nr.nfr, nf->ref);
                nr.notenum = notenum;
                nr.respnum = respnum;

                if (notenum >= nf->total_notes)
                  {
                    free_pager (&pager);
                    if (c == 'g' || c == 'l' || c == 'L')
                      return NEXTSEQ;
                    return 0;
                  }
                respnum = 0;
                if ((notenum =
                     get_next_note (&nr, seqtime)) > 0)
                  {
                    reload = TRUE;
                    notechanged = TRUE;
                    break;
                  }
                else
                  {
                    free_pager (&pager);
                    if (c == 'g' || c == 'l' || c == 'L')
                      return NEXTSEQ;
                    *first = nf->total_notes - LINES + 13;
                    return 0;
                  }
              }

            case 's':  /* Save note to a file. */
            case 'S':  /* Or an entire string. */
              {
                int lines;
                char *p, *q, *prompt, *filename, *tildename;

                move (LINES - 1, 0);
                clrtoeol ();

                if (traditional)
                  {
                    move (LINES - 2, 0);
                    clrtoeol ();
                    prompt = newts_strdup (_("File name: "));
                  }
                else
                  prompt = newts_strdup (_("Filename: "));

                refresh ();

                filename = gl_getline (prompt);
                newts_free (prompt);

                filename[strlen (filename) - 1] = '\0';
                if (strlen (filename) == 0)
                  {
                    if (traditional)
                      {
                        clear ();
                        arrange_replot (&pager);
                        display_note (nf, &note, &pager);
                        mvprintw (LINES - 1, 14, _("No Text Saved"));
                        redraw = FALSE;
                        break;
                      }
                    arrange_replot (&pager);
                    break;
                  }
                if (filename[0] == '|')
                  tildename = newts_strdup (filename);
                else
                  tildename = tilde_expand (filename);
                gl_histadd (filename);

                p = q = tildename;

                /* Strip leading and trailing whitespace. */

                while (*p == ' ')
                  p++;
                while ((*q++ = *p++))
                  ;
                for (--q; ; q--)
                  {
                    if (*q != ' ')
                      break;
                    *q = '\0';
                  }

                if (tildename[0] == '|')
                  p = _("Pipe");
                else
                  {
                    struct stat sb;
                    if (stat (tildename, &sb) == 0)
                      p = _("Appended");
                    else
                      p = _("New File");
                  }

                /* Perform the save. */

                if (c == 's')
                  {
                    /* This note/response only. */

                    lines = save_text (&note, tildename);
                  }
                else
                  {
                    /* The whole string. */

                    struct newt savenote;
                    int i, total;

                    memset (&savenote, 0, sizeof (struct newt));
                    nfref_copy (&savenote.nr.nfr, &note.nr.nfr);
                    savenote.nr.notenum = note.nr.notenum;
                    savenote.nr.respnum = note.nr.respnum;
                    get_note (&savenote, FALSE);
                    total = savenote.total_resps;
                    lines = save_text (&savenote, tildename);
                    for (i=1; i<=total; i++)
                      {
                        savenote.nr.respnum = i;
                        get_note (&savenote, FALSE);
                        lines += save_text (&savenote, tildename);
                      }
                  }

                clear ();
                arrange_replot (&pager);
                display_note (nf, &note, &pager);
                mvprintw (LINES - 1, traditional ? 14 : 0,
                          _("Saved %d lines in \"%s\" [%s]"), lines,
                          filename, p);

                newts_free (tildename);

                redraw = FALSE;
                break;
              }

            case 'f':  /* Forward a note to another notesfile. */
            case 'F':  /* ...with editing. */
              if (respnum)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("f/F only allowed from base note"));
                  else
                    printw (_("Forwarding is only permitted from basenotes."));
                  redraw = FALSE;
                  break;
                }

              {
                char *prompt, *readin, *tildename, *nfname = NULL;
                newts_nfref *destref = nfref_alloc ();
                struct notesfile dest;
                struct newt copy;
                int i, new;

                memset (&dest, 0, sizeof (struct notesfile));
                memset (&copy, 0, sizeof (struct newt));
                nfref_copy (&copy.nr.nfr, nf->ref);
                copy.nr.notenum = note.nr.notenum;

                prompt = newts_strdup (_("Forward to: "));

                do
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();

                    refresh ();

                    readin = gl_getline (prompt);
                    readin[strlen (readin) - 1] = '\0';
                    if (strlen (readin) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    nfname = newts_strdup (readin);
                    tildename = tilde_expand (readin);
                    gl_histadd (nfname);

                    {
                      List fwlist;

                      list_init (&fwlist,
                                 (void * (*) (void)) nfref_alloc,
                                 (void (*) (void *)) nfref_free,
                                 NULL);

                      parse_nf (tildename, &fwlist);
                      nfref_copy (destref,
                                  (newts_nfref *) list_data (list_head (&fwlist)));
                      list_destroy (&fwlist);
                    }

                    newts_free (tildename);

                    if (open_nf (destref, &dest))
                      {
                        clear ();
                        arrange_replot (&pager);
                        display_note (nf, &note, &pager);
                        mvprintw (LINES - 2, 0, traditional ?
                                  _("No such notesfile: '%s'") :
                                  _("No such notesfile: '%s'"),
                                  nfname);
                        newts_free (nfname);
                        nfname = NULL;
                        continue;
                      }
                  }
                while (nfname == NULL);

                newts_free (nfname);
                newts_free (prompt);

                if (!allowed (&dest, WRITE))
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    mvprintw (LINES - 1, 0, traditional ?
                              _("You haven't permission") :
                              _("You do not have permission to write in %s."),
                              dest.ref->name);
                    redraw = FALSE;
                    break;
                  }
                if (!allowed (&dest, REPLY) && note.total_resps > 0)
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    mvprintw (LINES - 1, 0, traditional ?
                              _("You haven't permission") :
                              _("You do not have permission to create responses in %s."),
                              dest.ref->name);
                    redraw = FALSE;
                    break;
                  }

                new = compose_note
                  (&dest, &note, traditional ? _("Edit copy text:") :
                   _("Edit Copy Text:"), traditional ? _("Copy title:") :
                   _("Copy Title: "), -1, c == 'F' ? COPYEDIT : COPYNOEDIT);

                for (i=1; i<= note.total_resps; i++)
                  {
                    copy.nr.respnum = i;
                    get_note (&copy, FALSE);
                    compose_note
                      (&dest, &copy, traditional ? _("Edit copy text:") :
                       _("Edit Copy Text:"), traditional ? _("Copy title:") :
                       _("Copy Title: "), new, c == 'F' ? COPYEDIT : COPYNOEDIT);
                  }

                update_nf (nf);
                arrange_replot (&pager);
                break;
              }

            case 'v':  /* Vector a thread to a new notesfile. */
              if (respnum)
                {
                  move (LINES - 1, 0);
                  clrtoeol ();
                  if (traditional)
                    mvprintw (LINES - 1, 14,
                              _("v only allowed from base note"));
                  else
                    printw (_("Vectoring is only permitted from basenotes."));
                  redraw = FALSE;
                  break;
                }

              {
                char *prompt, *readin, *tildename, *nfname = NULL;
                newts_nfref *destref = nfref_alloc ();
                struct notesfile dest;
                struct newt copy;
                int i, new;

                memset (&dest, 0, sizeof (struct notesfile));
                memset (&copy, 0, sizeof (struct newt));
                nfref_copy (&copy.nr.nfr, nf->ref);
                copy.nr.notenum = note.nr.notenum;

                prompt = newts_strdup (_("Vector to: "));

                do
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();

                    refresh ();

                    readin = gl_getline (prompt);
                    readin[strlen (readin) - 1] = '\0';
                    if (strlen (readin) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    nfname = newts_strdup (readin);
                    tildename = tilde_expand (readin);
                    gl_histadd (nfname);

                    {
                      List fwlist;

                      list_init (&fwlist,
                                 (void * (*) (void)) nfref_alloc,
                                 (void (*) (void *)) nfref_free,
                                 NULL);

                      parse_nf (tildename, &fwlist);
                      nfref_copy (destref,
                                  (newts_nfref *) list_data (list_head (&fwlist)));
                      list_destroy (&fwlist);
                    }

                    newts_free (tildename);

                    if (open_nf (destref, &dest))
                      {
                        clear ();
                        arrange_replot (&pager);
                        display_note (nf, &note, &pager);
                        mvprintw (LINES - 2, 0, traditional ?
                                  _("No such notesfile: '%s'") :
                                  _("No such notesfile: '%s'"),
                                  nfname);
                        newts_free (nfname);
                        nfname = NULL;
                        continue;
                      }
                  }
                while (nfname == NULL);

                newts_free (nfname);
                newts_free (prompt);

                if (!allowed (&dest, WRITE))
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    mvprintw (LINES - 1, 0, traditional ?
                              _("You haven't permission") :
                              _("You do not have permission to write in %s."),
                              dest.ref->name);
                    redraw = FALSE;
                    break;
                  }
                if (!allowed (&dest, REPLY) && note.total_resps > 0)
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    mvprintw (LINES - 1, 0, traditional ?
                              _("You haven't permission") :
                              _("You do not have permission to create responses in %s."),
                              dest.ref->name);
                    redraw = FALSE;
                    break;
                  }

                new = compose_note (&dest, &note, NULL, NULL, -1, VECTOR);

                for (i=1; i<= note.total_resps; i++)
                  {
                    copy.nr.respnum = i;
                    get_note (&copy, FALSE);
                    compose_note (&dest, &copy, NULL, NULL, new, VECTOR);
                  }

                update_nf (nf);
                arrange_replot (&pager);
                break;
              }

            case 'V':  /* Vector a single note as a bug report. */
              {
                char *prompt, *readin, *tildename, *nfname = NULL;
                char *titlestring;
                newts_nfref *destref = nfref_alloc ();
                struct notesfile dest;
                struct newt copy;

                memset (&dest, 0, sizeof (struct notesfile));
                memset (&copy, 0, sizeof (struct newt));
                nfref_copy (&copy.nr.nfr, nf->ref);
                copy.nr.notenum = note.nr.notenum;

                prompt = newts_strdup (_("Report bug to: "));

                do
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();

                    refresh ();

                    readin = gl_getline (prompt);
                    readin[strlen (readin) - 1] = '\0';
                    if (strlen (readin) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    nfname = newts_strdup (readin);
                    tildename = tilde_expand (readin);
                    gl_histadd (nfname);

                    {
                      List fwlist;

                      list_init (&fwlist,
                                 (void * (*) (void)) nfref_alloc,
                                 (void (*) (void *)) nfref_free,
                                 NULL);

                      parse_nf (tildename, &fwlist);
                      nfref_copy (destref,
                                  (newts_nfref *) list_data (list_head (&fwlist)));
                      list_destroy (&fwlist);
                    }

                    newts_free (tildename);

                    if (open_nf (destref, &dest))
                      {
                        clear ();
                        arrange_replot (&pager);
                        display_note (nf, &note, &pager);
                        mvprintw (LINES - 2, 0, _("No such notesfile: '%s'"),
                                  nfname);
                        newts_free (nfname);
                        nfname = NULL;
                        continue;
                      }
                  }
                while (nfname == NULL);

                newts_free (prompt);

                if (!allowed (&dest, WRITE))
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    mvprintw (LINES - 1, 0, traditional ?
                              _("You haven't permission") :
                              _("You do not have permission to write in %s."),
                              dest.ref->name);
                    redraw = FALSE;
                    break;
                  }

                clear ();
                arrange_replot (&pager);
                display_note (nf, &note, &pager);

                {
                  char *assign, *descr;
                  char bugnumstr[30];
                  int bugnum;
                  int ch, y, x, priority = '-';

                  mvprintw (LINES - 1, traditional ? 14 : 0, _("Priority: "));

                  getyx (stdscr, y, x);
                  addch (priority);
                  move (y, x);
                  refresh ();
                  ch = getch ();
                  while (ch != '\n' && ch != '\r' && ch != KEY_ENTER && ch != EOF)
                    {
                      priority = ch;
                      addch (priority);
                      move (y, x);
                      refresh ();
                      ch = getch ();
                    }

                  move (LINES - 1, 0);
                  clrtoeol ();

                  if (traditional)
                    {
                      prompt = newts_nmalloc (strlen (_("Assign to: ")) + 15,
                                              sizeof (char));
                      strcpy (prompt, "              ");
                      strcat (prompt, _("Assign to: "));
                    }
                  else
                    prompt = newts_strdup (_("Assign to: "));

                  refresh ();

                  readin = gl_getline (prompt);
                  newts_free (prompt);

                  readin[strlen (readin) - 1] = '\0';
                  if (strlen (readin) == 0)
                    {
                      newts_free (nfname);
                      arrange_replot (&pager);
                      break;
                    }
                  assign = newts_nmalloc (strlen (readin) < 12 ? strlen (readin) + 1
                                                               : 13,
                                          sizeof (char));
                  strncpy (assign, readin, 12);
                  if (strlen (readin) >= 12)
                    assign[12] = '\0';
                  gl_histadd (readin);

                  clear ();
                  arrange_replot (&pager);
                  display_note (nf, &note, &pager);

                  move (LINES - 1, 0);

                  if (traditional)
                    {
                      prompt = newts_nmalloc (15 + strlen (_("Brief Descr: ")),
                                              sizeof (char));
                      strcpy (prompt, "              ");
                      strcat (prompt, _("Brief Descr: "));
                    }
                  else
                    prompt = newts_strdup (_("Brief Description: "));

                  refresh ();

                  readin = gl_getline (prompt);
                  newts_free (prompt);

                  readin[strlen (readin) - 1] = '\0';
                  if (strlen (readin) == 0)
                    {
                      newts_free (nfname);
                      newts_free (assign);
                      arrange_replot (&pager);
                      break;
                    }
                  descr = newts_strdup (readin);
                  gl_histadd (readin);

                  clear ();
                  arrange_replot (&pager);
                  display_note (nf, &note, &pager);

                  move (LINES - 1, 0);
                  clrtoeol ();
                  YES_OR_NO (ch,
                             printw (traditional ?
                                     _("Really send note \"%c:%s:%s\" to '%s' (y/n)? ") :
                                     _("Report bug as \"%c:%s:%s\" in %s? (y/n): "),
                                     priority, assign, descr, nfname);
                             );
                  if (!ch)
                    {
                      newts_free (nfname);
                      newts_free (assign);
                      newts_free (descr);
                      arrange_replot (&pager);
                      break;
                    }

                  bugnum = get_next_bug (nf);

                  snprintf (bugnumstr, 29, "%d", bugnum);

                  titlestring = newts_nmalloc (strlen (bugnumstr) + 5 +
                                               strlen (assign) + strlen (descr),
                                               sizeof (char));
                  sprintf (titlestring, "%s:%c:%s:%s", bugnumstr, priority,
                           assign, descr);

                  newts_free (nfname);
                  newts_free (assign);
                  newts_free (descr);
                }

                readin = note.title;
                note.title = titlestring;

                compose_note (&dest, &note, NULL, NULL, -1, VECTOR);

                note.title = readin;
                newts_free (titlestring);

                update_nf (nf);
                arrange_replot (&pager);
                break;
              }

            case 'c':  /* Copy a single note to another notesfile. */
            case 'C':  /* ...with editing. */
              {
                char *prompt, *readin, *tildename, *nfname = NULL;
                newts_nfref *destref = nfref_alloc ();
                struct notesfile dest;
                struct newt copy;

                int yn = FALSE;
                int replyto;

                memset (&dest, 0, sizeof (struct notesfile));
                memset (&copy, 0, sizeof (struct newt));
                nfref_copy (&copy.nr.nfr, nf->ref);
                copy.nr.notenum = note.nr.notenum;

                prompt = newts_strdup (_("Copy to: "));

                do
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();

                    refresh ();

                    readin = gl_getline (prompt);
                    readin[strlen (readin) - 1] = '\0';
                    if (strlen (readin) == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    nfname = newts_strdup (readin);
                    tildename = tilde_expand (readin);
                    gl_histadd (nfname);

                    {
                      List fwlist;

                      list_init (&fwlist,
                                 (void * (*) (void)) nfref_alloc,
                                 (void (*) (void *)) nfref_free,
                                 NULL);

                      parse_nf (tildename, &fwlist);
                      nfref_copy (destref,
                                  (newts_nfref *) list_data (list_head (&fwlist)));
                      list_destroy (&fwlist);
                    }

                    newts_free (tildename);

                    if (open_nf (destref, &dest))
                      {
                        clear ();
                        arrange_replot (&pager);
                        display_note (nf, &note, &pager);
                        mvprintw (LINES - 2, 0, traditional ?
                                  _("No such notesfile: '%s'") :
                                  _("No such notesfile: '%s'"),
                                  nfname);
                        newts_free (nfname);
                        nfname = NULL;
                        continue;
                      }
                  }
                while (nfname == NULL);

                newts_free (prompt);
                newts_free (nfname);

                if (allowed (&dest, READ) && allowed (&dest, WRITE) &&
                    allowed (&dest, REPLY))
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    YES_OR_NO (yn,
                               mvprintw (LINES - 1, 0, traditional ?
                                         _("Copy as response (y/n)? ") :
                                         _("Copy as response? (y/n): "));
                               );
                    if (yn)
                      {
                        if (!allowed (&dest, REPLY))
                          {
                            clear ();
                            arrange_replot (&pager);
                            display_note (nf, &note, &pager);
                            mvprintw (LINES - 1, 0, traditional ?
                                      _("You haven't permission") :
                                      _("You do not have permission to respond in %s."),
                                      dest.ref->name);
                            redraw = FALSE;
                            break;
                          }
                        if ((replyto = limited_index (&dest)))
                          {
                            if (replyto == QUITSEQ || replyto == QUITNOSEQ)
                              {
                                free_pager (&pager);
                                return replyto;
                              }
                            compose_note
                              (&dest, &note, traditional ? _("Edit copy text:") :
                               _("Edit Copy Text:"), traditional ?
                               _("Copy title:") : _("Copy Title: "), replyto,
                               c == 'C' ? COPYEDIT : COPYNOEDIT);
                          }
                      }
                    else
                      {
                        if (!allowed (&dest, WRITE))
                          {
                            clear ();
                            arrange_replot (&pager);
                            display_note (nf, &note, &pager);
                            mvprintw (LINES - 1, 0, traditional ?
                                      _("You haven't permission") :
                                      _("You do not have permission to write in %s."),
                                      dest.ref->name);
                            redraw = FALSE;
                            break;
                          }
                        compose_note
                          (&dest, &note, traditional ? _("Edit copied text:") :
                           _("Edit Copy Text:"), _("Copy title:"), -1,
                           c == 'C' ? COPYEDIT : COPYNOEDIT);
                      }

                    update_nf (nf);
                    arrange_replot (&pager);
                    break;
                  }
              }

            case 't':  /* UNIX talk to the author of this note. */
              {
                char *userarg;
                int status;

                if (strcasecmp ("anonymous", note.auth.name) == 0)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw (_("This message was posted anonymously; you can't "
                              "send messages to its author."));
                    redraw = FALSE;
                    break;
                  }
                if (strcmp (fqdn, note.auth.system) != 0)
                  {
                    /* Only allow remote system connect attempts for talk and
                     * friends.  These attempts might fail anyway.
                     */

                    if (strstr (talk, "talk") == 0)
                      {
                        arrange_replot (&pager);
                        break;
                      }
                    else
                      {
                        userarg = newts_nmalloc (strlen (note.auth.name) +
                                                 strlen (note.auth.system) + 2,
                                                 sizeof (char));
                        sprintf (userarg, "%s@%s", note.auth.name,
                                 note.auth.system);
                      }
                  }
                else
                  {
                    int utmp_status = get_author_utmp (note.auth.name);

                    if (utmp_status == 0)
                      {
                        move (LINES - 1, 0);
                        clrtoeol ();
                        printw (_("The user '%s' is not logged in."),
                                note.auth.name);
                        redraw = FALSE;
                        break;
                      }
                    else if (utmp_status == 1)
                      {
                        move (LINES - 1, 0);
                        clrtoeol ();
                        printw (_("The user '%s' is not accepting messages."),
                               note.auth.name);
                        redraw = FALSE;
                        break;
                      }

                    userarg = newts_strdup (note.auth.name);
                  }

                /* FIXME: this is invalid given the new enter/exit semantics. */

                //if (traditional)
                //  {
                //    printf ("%s %s\n", talk, userarg);
                //    fflush (stdout);
                //  }

                status = spawn_process (NULL, talk, note.auth.name, NULL);

                //if (traditional)
                //  {
                //    printf (_("--Hit any key to continue--"));
                //    getchar ();
                //  }

                if (status != 0)
                  {
                    clear ();
                    arrange_replot (&pager);
                    display_note (nf, &note, &pager);
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw (_("The '%s' program returned an error."),
                            base_name (talk));
                  }

                newts_free (userarg);
                arrange_replot (&pager);
                break;
              }

            case 'p':  /* Send UNIX mail to the author of this note. */
            case 'P':  /* ...with quoting. */
              {
                char *address;

                if (strcmp (note.auth.system, fqdn) == 0)
                  address = newts_strdup (note.auth.name);
                else
                  {
                    address = newts_nmalloc (strlen (note.auth.system) +
                                             strlen (note.auth.name) + 2,
                                             sizeof (char));
                    sprintf (address, "%s@%s", note.auth.name,
                             note.auth.system);
                  }

                compose_mail (address, c == 'P' ? &note : NULL);

                newts_free (address);
                arrange_replot (&pager);
                break;
              }

            case 'm':  /* Send UNIX mail. */
            case 'M':  /* ...quoting this note. */
              {
                char *sendto, *prompt;

                move (LINES - 1, 0);
                clrtoeol ();

                if (traditional)
                  {
                    prompt = _("Send to whom? ");
                  }
                else
                  {
                    prompt = _("Send mail to: ");
                  }

                refresh ();

                sendto = gl_getline (prompt);
                sendto[strlen (sendto) - 1] = '\0'; /* Chomp trailing '\n' */
                gl_histadd (sendto);

                compose_mail (sendto, c == 'M' ? &note : NULL);
                arrange_replot (&pager);
                break;
              }

            case 'q':
            case 'k':
              free_pager (&pager);
              return NEXTSEQ;

            case 'Q':
            case 'K':
              free_pager (&pager);
              return NEXTNOSEQ;

            case '\04':
              free_pager (&pager);
              return QUITNOSEQ;

            case 'z':
              free_pager (&pager);
              return QUITSEQ;

            default:
              beep ();
              arrange_replot (&pager);
              break;
            }
        }
      while ((!pager_at_end (&pager) && !reload) || !redraw);
    }
}

/* read_help - display a help page for reading notes. */

static void
read_help (void)
{
  int c, column;

  do
    {
      clear ();

      mvprintw (0, (COLS - 9) / 2 - 1, "View Help");

      column = (COLS - 80) / 2 - 3;

      mvprintw (2, column + 4, "<number>  skip number resps ahead");
      mvprintw (3, column + 4, "<RET>     display next note");
      mvprintw (4, column + 4, "+, ;      display next resp or note");
      mvprintw (5, column + 4, "<PGDN>    display next page");
      mvprintw (6, column + 4, "<SPC>     display next page or resp");
      mvprintw (7, column + 4, "<PGUP>    display prev page");
      mvprintw (8, column + 4, "-, <BS>   display prev page or resp");
      mvprintw (9, column + 4, "=         display current basenote");
      mvprintw (10, column + 4, "*         display last resp");
      mvprintw (11, column + 4, "<arrows>  move around current note");
      mvprintw (13, column + 4, "a, A      search by author (repeat)");
      mvprintw (14, column + 4, "x, X      search by title (repeat)");
      mvprintw (15, column + 4, "/, \\      search for text (repeat)");
      mvprintw (17, column + 4, "C-d, z    quit notes (update seq.)");
      mvprintw (18, column + 4, "i         return to notesfile index");
      mvprintw (19, column + 4, "q, k      exit nf and update seq.");
      mvprintw (20, column + 4, "Q, K      exit nf");
      mvprintw (21, column + 4, "!         fork a subshell");

      mvprintw (2, column + 43, "B       complain about Newts");
      mvprintw (3, column + 43, "c, C    copy note to other nf (edit)");
      mvprintw (4, column + 43, "d       change director message");
      mvprintw (5, column + 43, "D       delete your note");
      mvprintw (6, column + 43, "e, E    edit title (text) of your note");
      mvprintw (7, column + 43, "f, F    copy thread to other nf (edit)");
      mvprintw (8, column + 43, "j, l    next updated resp (or exit)");
      mvprintw (9, column + 43, "J, L    next updated note (or exit)");
      mvprintw (10, column + 43, "m, M    send mail (quote)");
      mvprintw (11, column + 43, "n, N    open new notesfile (archive)");
      mvprintw (12, column + 43, "p, P    send mail to author (quote)");
      mvprintw (13, column + 43, "R       rot-13 note text");
      mvprintw (14, column + 43, "r, C-l  redraw the screen");
      mvprintw (15, column + 43, "s, S    save note (string) to file");
      mvprintw (16, column + 43, "t       talk to author of current note");
      mvprintw (17, column + 43, "v       vector thread to other nf");
      mvprintw (18, column + 43, "V       report bug mentioned in note");
      mvprintw (19, column + 43, "w, W    write a response (quote)");
      mvprintw (20, column + 43, "Z       delete any note as director");
      mvprintw (21, column + 43, ":       approve a moderated note");

      mvprintw (23, column + 20, "If you have suggestions for the help wording");
      mvprintw (24, column + 20, "or the layout, please pass them on to Tyler.");

      move (LINES - 2, 0);

      refresh ();

      c = getch ();
      while (c == KEY_RESIZE)
        c = getch ();
    }
  while (c == EOF);

  return;
}

/* display_note - print the text of NOTE on the screen. */

int
display_note (struct notesfile *nf, struct newt *np, struct pager *pager)
{
  short highlighted = FALSE;

  clear ();

  if (np->nr.notenum != 0)
    {
      move (0, 0);
      if (pager->bufptr) printw (_("[Continued] "));
      printw (_("Note %d"), np->nr.notenum);
    }

  if (traditional)
    mvprintw (0, 20 + (40 - strlen (nf->title)) / 2 - 1, nf->title);
  else
    mvprintw (0, (COLS - strlen (nf->title)) / 2 - 1, nf->title);

  if (np->nr.respnum > 0)
    {
      if (!traditional)
        {
          char *buffer;

          asprintf (&buffer, _("Response %d of %d"),
                    np->nr.respnum, np->total_resps);

          if (buffer)
            {
              mvprintw (0, COLS - strlen (buffer), "%s", buffer);
              free (buffer);
            }
        }
    }
  else if (np->total_resps > 0)
    {
      if (traditional)
        mvprintw (0, 66, ngettext ("%d response", "%d responses",
                                   np->total_resps), np->total_resps);
      else
        {
          char *buffer;

          asprintf (&buffer, ngettext ("%d response", "%d responses",
                                       np->total_resps),
                    np->total_resps);

          if (buffer)
            {
              mvprintw (0, COLS - strlen (buffer), "%s", buffer);
              free (buffer);
            }
        }
    }

  if (strcasecmp ("anonymous", np->auth.name) == 0)
    mvprintw (1, 0, traditional ? _("Anonymous") : _("anonymous"));
  else
    {
      if (!traditional && strcasecmp (np->auth.system, fqdn) == 0 &&
          strcmp (np->auth.name, "anonymous"))
        {
          struct passwd *pw = getpwnam (np->auth.name);
          if (pw)
            {
              char *real = newts_strdup (pw->pw_gecos);
              char *separator;
              char *namebuf = newts_nmalloc (strlen (real) +
                                             strlen (np->auth.name) + 4,
                                             sizeof (char));
              int title_loc = (COLS - strlen (np->title)) / 2 - 1;

              if ((separator = strpbrk (real, ":,")) != NULL)
                *separator = '\0';

              sprintf (namebuf, "%s (%s)", real, np->auth.name);
              if (strlen (namebuf) > title_loc)
                {
                  int offset = namebuf[title_loc - 6] == '(' ? 2 : 0;
                  namebuf[title_loc - offset - 5] = '.';
                  namebuf[title_loc - offset - 4] = '.';
                  namebuf[title_loc - offset - 3] = '.';
                  namebuf[title_loc - offset - 2] = ' ';
                  namebuf[title_loc - offset - 1] = '\0';
                }

              mvprintw (1, 0, "%s", namebuf);

              newts_free (namebuf);
              newts_free (real);
            }
          else
            mvprintw (1, 0, "%s", np->auth.name);
        }
      else
        mvprintw (1, 0, "%s", np->auth.name);
    }

  if (!traditional)
    {
      attron (A_BOLD);
      mvprintw (1, (COLS - strlen (np->title)) / 2 - 1, np->title);
      attroff (A_BOLD);
    }
  else
    if (np->nr.respnum > 0)
      mvprintw (1, 30, _("Response %2d of %2d"), np->nr.respnum,
                np->total_resps);
    else
      mvprintw (1, 20 + (40 - strlen (np->title)) / 2 - 1, np->title);

  if (!(np->options & NOTE_CORRUPTED))
    {
      if (traditional)
        move (1, 58);
      else
        move (1, COLS - 22);
      printw_time (localtime (&np->created));
    }

  if (strcasecmp (np->auth.system, fqdn) &&
      strcmp (np->auth.name, "anonymous"))
    mvprintw (2, 0, "(at %s)", np->auth.system);
  else
    {
      switch (get_author_utmp (np->auth.name))
        {
        case 0:
          mvprintw (2, 0, "(Offline)");
          break;

        case 1:
          mvprintw (2, 0, "(Online Mesg N)");
          break;

        case 2:
          mvprintw (2, 0, "(Online Mesg Y)");
          break;
        }
    }

  if (np->director_message)
    {
      if (traditional)
        mvprintw (2, 20 + (40 - strlen (np->director_message)) / 2 - 1,
                  np->director_message);
      else
        mvprintw (2, (COLS - strlen (np->director_message)) / 2 - 1,
                  np->director_message);
    }
  else if (np->options & NOTE_CORRUPTED && np->nr.respnum)
    {
      if (traditional)
        mvprintw (2,
                  20 + (40 - strlen (_("** Corrupted Response **"))) / 2 - 1,
                  _("** Corrupted Response **"));
      else
        mvprintw (2, (COLS - strlen (_("** Corrupted Response **"))) / 2 - 1,
                  _("** Corrupted Response **"));
    }
  else if (np->options & NOTE_UNAPPROVED && traditional)
    mvprintw (2, 28, _("-- (Moderated) --"));
  else if (np->options & NOTE_WRITE_ONLY && traditional)
    mvprintw (2, 28, _("-- (Write Only) --"));

  if (!traditional && np->options & NOTE_DELETED)
    mvprintw (2, COLS - strlen (_("[Deleted]")), _("[Deleted]"));
  else if (!traditional && np->options & NOTE_UNAPPROVED)
    mvprintw (2, COLS - strlen (_("[Approval Pending]")),
              _("[Approval Pending]"));
  else if (!traditional && np->options & NOTE_WRITE_ONLY)
    mvprintw (2, COLS - strlen (_("[Write-only]")),
              _("[Write-only]"));

  move (4, 0);
  clrtobot ();

  if (pager->highlight)
    {
      highlighted = show_text (pager);
      if (!highlighted)
        {
          move (0, 0);
          printw (_("[Continued] "));
          printw (_("Note %d"), np->nr.notenum);
        }
      while (!highlighted && pager->bufptr < pager->textlen)
        {
          if (pager->pagesout < 50)
            pager->pagecnt[++pager->pagesout] = pager->bufptr;
          move (4, 0);
          clrtobot ();
          highlighted = show_text (pager);
        }

      if (!highlighted && pager->bufptr >= pager->textlen)
        return -1;

      pager->highlight = FALSE;
    }
  else
    show_text (pager);

  if (pager->bufptr < pager->textlen)
    {
      if (traditional)
        {
          mvprintw (LINES - 1, 51, "-- %2d% [%ld/%ld] --",
                    (int) ((long) (pager->bufptr * 100L) /
                           (long) pager->textlen),
                    (long) pager->bufptr, (long) pager->textlen);
        }
      else
        {
          char *buffer;

          asprintf (&buffer, "%2d%% [%ld/%ld]",
                    (int) ((long) (pager->bufptr * 100L) /
                           (long) pager->textlen),
                    (long) pager->bufptr, (long) pager->textlen);

          if (buffer)
            {
              mvprintw (LINES - 1, COLS - strlen (buffer) - 1 -
                        strlen (_("-- More --")), "%s", buffer);
              attron (A_REVERSE);
              mvprintw (LINES - 1, COLS - strlen (_("-- More --")),
                        _("-- More --"));
              attroff (A_REVERSE);

              free (buffer);
            }
        }
    }
  else if (!traditional)
    {
      attron (A_REVERSE);
      if (np->nr.respnum != np->total_resps && np->nr.notenum != 0)
        mvprintw (LINES - 1, COLS - strlen (_("-- Next response --")),
                  _("-- Next response --"));
      else if (np->nr.notenum != nf->total_notes && np->nr.notenum != 0)
        mvprintw (LINES - 1, COLS - strlen (_("-- Next note --")),
                  _("-- Next note --"));
      else
        mvprintw (LINES - 1, COLS - strlen (_("-- Return to index --")),
                  _("-- Return to index --"));
      attroff (A_REVERSE);
    }

  return pager->highlight ? highlighted : TRUE;
}

/* save_text - save text from NOTE into the file at FN.
 *
 * Returns: the number of lines written.
 */

int
save_text (struct newt *note, const char *fn)
{
  FILE *file;
  struct tm *tm;
  int c;
  int lines = 6;
  char *cursor = note->text;
  char *filename = newts_strdup (fn);

  short pipe = FALSE;

  if (*filename == '|')
    {
      pipe = TRUE;
      while (*++filename == ' ');
    }

  /* The seteuid back to root here guarantees that we can open the file with
   * root's permissions if we really are root.
   */

  if (getuid () == 0)
    seteuid (0);

  if (pipe)
    file = popen (filename, "w");
  else
    file = fopen (filename, "a");

  if (getuid () == 0)
    seteuid (notes_uid);

  if (note->nr.respnum)
    fprintf (file, _("Response %d/%d to "), note->nr.respnum,
      note->total_resps);
  fprintf (file, _("\"%s\" (Note %d) from =%s\n"), note->title,
           note->nr.notenum, note->nr.nfr.name);
  fprintf (file, _("Author: %s@%s\nDate: "), note->auth.name,
           note->auth.system);

  tm = localtime (&note->created);
  fprint_time (file, tm);

  fprintf (file, "\n\n");

  while ((c = *cursor++) && c != EOF)
    switch (c)
      {
      case '\n':
        lines++;
        /* Fallthrough */

      default:
        fputc (c, file);
      }

  if (*(cursor - 2) != '\n')
    {
      lines++;
      fprintf (file, "\n");
    }
  fprintf (file, "\n\n");

  if (pipe)
    pclose (file);
  else
    fclose (file);

  newts_free (filename);
  return lines;
}
