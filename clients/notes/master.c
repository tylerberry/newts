/*
 * master.c - master routine for viewing a notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on control.c from the UIUC notes distribution by Ray Essick
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

#include "curses_wrapper.h"

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

static int verify_sequencer (struct notesfile *nf);

#if __STDC__
static void printf_to_message_buffer (const char *format, ...)
     __attribute__ ((format (printf, 1, 2)));
#else
static void printf_to_message_buffer ();
#endif

/* master - the central driver for being in a notesfile.  This does
 * access-checking, handles setting up the sequencer, and goes into an
 * index-read loop.
 *
 * Returns: NEXTSEQ, NEXTNOSEQ, QUITSEQ, or QUITNOSEQ, or -1 if we couldn't get
 * into the notesfile at all.
 */

int
master (newts_nfref *ref)
{
  int first, last;
  int num, resp;
  int result;
  time_t entered;
  struct notesfile nf;
  memset (&nf, 0, sizeof (nf));

  if (ref == NULL)
    return -1;

  time (&entered);

  result = open_nf (ref, &nf);
  if (result)
    {
      switch (result)
        {
        case -1:
          if (entered_curses ())
            {
              status_message (0, _("No such notesfile: '%s'"),
                              nfref_pretty_name (ref));
              sleep (2);
            }
          printf_to_message_buffer (_("%s: no such notesfile"),
                                    nfref_pretty_name (ref));
          break;

        case -2:
          if (entered_curses ())
            {
              status_message (0, _("INTERNAL ERROR: unspecified notesfile"));
              sleep (2);
            }
          printf_to_message_buffer (_("INTERNAL ERROR: unspecified "
                                      "notesfile"));
          break;

        case -4:
          if (entered_curses ())
            {
              status_message (0, _("Invalid database format: '%s'"),
                              nfref_pretty_name (ref));
              sleep (2);
            }
          printf_to_message_buffer (_("%s: no invalid database format"),
                                    nfref_pretty_name (ref));
          break;

        case -3:
        default:
          if (entered_curses ())
            {
              status_message (0, _("Error opening notesfile: '%s'"),
                              nfref_pretty_name (ref));
              sleep (2);
            }
          printf_to_message_buffer (_("%s: error opening notesfile"),
                                    nfref_pretty_name (ref));
          break;
        }

      return -1;
    }

  /* This big mess is all to handle What To Do If We're Not Allowed. */

  if (!allowed (&nf, READ))
    {
      if (sequencer == NONE)  /* If sequencing, just skip all this. */
        {
          int yn;

          ensure_curses ();

          clear ();

          if (nf.options & NF_POLICY)
            {
              if (traditional)
                mvprintw (LINES - 2, 0, _("You aren't allowed to read %s"),
                          nfref_pretty_name (ref));
              else
                mvprintw (LINES - 2, 0, _("You are not allowed to read '%s'."),
                          nfref_pretty_name (ref));

              YES_OR_NO (yn,
                         mvprintw (LINES - 1, 0, traditional ?
                                   _("Do you wish to see the policy "
                                     "note (y/n)?") :
                                   _("Would you like to see the policy note? "
                                     "(y/n): "));
                         refresh ();
                         );
              if (yn)
                {
                  int nullfirst;

                  read_note (&nf, &nullfirst, 0, 0, TRUE);
                }

              clear ();

              if (allowed (&nf, WRITE))
                {
                  YES_OR_NO (yn,
                             if (traditional)
                               {
                                 mvprintw (LINES - 2, 0,
                                           _("You may leave a note in the "
                                             "notefile"));
                                 mvprintw (LINES - 1, 0,
                                           _("Do you wish to leave a note "
                                             "(y/n) ?"));
                                 refresh ();
                               }
                             else
                               {
                                 mvprintw (LINES - 1, 0,
                                           _("Would you like to leave a note in '%s'? (y/n): "),
                                           nfref_pretty_name (ref));
                                 refresh ();
                               }
                             );
                  if (yn)
                    {
                      int result;
                      result = compose_note (&nf, NULL, _("Edit Note Text:"),
                                             traditional ? _("Note title: ") :
                                         _("Note Title: "), -1, NORMAL);
                      clear ();
                      if (result != -1 && traditional)
                        mvprintw (LINES - 1, 0,
                                  traditional ?
                                  _("Your note has been registered") :
                                  _("Your note has been registered."));
                      refresh ();
                      sleep (2);
                    }
                }
              else if (traditional)
                {
                  mvprintw (LINES - 1, 0, _("Hit any key to continue"));
                  refresh ();
                  getch ();              /* Grab and drop a keystroke. */
                }
            }
          else
            {
              if (allowed (&nf, WRITE))
                {
                  YES_OR_NO (yn,
                             if (traditional)
                               {
                                 mvprintw (LINES - 4, 0, _("You aren't allowed to read %s"),
                                           nfref_pretty_name (ref));
                                 mvprintw (LINES - 3, 0, _("There is no policy note"));
                                 mvprintw (LINES - 2, 0,
                                           _("You may leave a note in the "
                                             "notefile"));
                                 mvprintw (LINES - 1, 0,
                                           _("Do you wish to leave a note "
                                             "(y/n) ?"));
                                 refresh ();
                               }
                             else
                               {
                                 mvprintw (LINES - 3, 0, _("You are not allowed to read '%s'."),
                                           nfref_pretty_name (ref));
                                 mvprintw (LINES - 2, 0, _("The notesfile has "
                                                           "no policy note to read."));
                                 mvprintw (LINES - 1, 0,
                                           _("Would you like to leave a note in the "
                                             "notesfile? (y/n): "));
                                 refresh ();
                               }
                             );
                  if (yn)
                    {
                      int result;
                      result = compose_note (&nf, NULL, _("Edit Note Text:"),
                                             traditional ? _("Note title: ") :
                                         _("Note Title: "), -1, NORMAL);
                      clear ();
                      if (result != -1 && traditional)
                        mvprintw (LINES - 1, 0,
                                  traditional ?
                                  _("Your note has been registered") :
                                  _("Your note has been registered."));
                      refresh ();
                      sleep (2);
                    }
                }
              else
                {
                  if (traditional)
                    {
                      mvprintw (LINES - 3, 0, _("You aren't allowed to read %s"),
                                nfref_pretty_name (ref));
                      mvprintw (LINES - 2, 0, _("There is no policy note"));
                      mvprintw (LINES - 1, 0, _("Hit any key to continue"));
                      refresh ();
                      getch ();              /* Grab and drop a keystroke. */
                    }
                  else
                    {
                      mvprintw (LINES - 2, 0, _("You are not allowed to read '%s'."),
                                nfref_pretty_name (ref));
                      mvprintw (LINES - 1, 0, _("The notesfile has "
                                                "no policy note to read."));
                      refresh ();
                      sleep (2);
                    }
                }
            }
        }
      return -1;
    }

  if ((nf.options & NF_LOCKED) && !(allowed (&nf, DIRECTOR)))
    {
      /* Up the airy mountain
       * Down the rushy glen
       * We daren't go a-hunting
       * For fear of little men
       *
       * "The Fairies", William Allingham (1824-1889)
       */

      if (entered_curses ())
        {
          if (traditional)
            status_message (0, _("Notefile %s is closed"),
                            nfref_pretty_name (ref));
          else
            status_message (0, _("%s is locked."),
                            nfref_pretty_name (ref));
          sleep (2);
        }
      printf_to_message_buffer ("%s: closed",
                                nfref_pretty_name (ref));

      return -1;
    }
  else
    {
      ensure_curses ();

      if ((nf.options & NF_LOCKED) && traditional)
        {
          status_message (0, _("Notefile %s is closed"),
                          nfref_pretty_name (ref));
          sleep (2);
        }

      seqtime = orig_seqtime;
      if (sequencer == NONE)
        seqtime = 0;
      else if (!alt_time)
        get_seqtime (ref, seqname, &seqtime);
      first = nf.total_notes - LINES + 13;
      resp = 0;

      if (sequencer != EXTENDED && sequencer != NONE &&
          ((difftime (seqtime, nf.modified) > 0) ||
           !verify_sequencer (&nf)))
        {
          /* Record that there was nothing new in this one. */

          printf_to_message_buffer ("%s...", nfref_pretty_name (ref));

          return 0;
        }
      else
        {
          short suppress_blacklist;  /* Used to control temporary ignores. */

          if (sequencer != INDEX && sequencer != NONE)
            {
              struct newtref nr;
              memset (&nr, 0, sizeof (struct newtref));
              nfref_copy (&nr.nfr, ref);
              nr.notenum = 0;
              nr.respnum = 0;

              suppress_blacklist = 'l';
              num = get_next_note (&nr, seqtime);
              if (num > 0)
                goto sequenced;
            }

          while (1)
            {
              suppress_blacklist = FALSE;

              display_index (&nf, &first, &last);

              num = run_index (&nf, &first, &last, &resp, &suppress_blacklist);
              if (num == -1)
                continue;     /* Redisplay. */
              if (num < -1)
                break;        /* Quit of some sort. */

            sequenced:
              num = read_note (&nf, &first, num, resp, suppress_blacklist);
              if (num < -1)
                break;
            }

          if ((num == NEXTSEQ || num == QUITSEQ) &&
              sequencer != NONE && !alt_time)
            set_seqtime (ref, seqname, entered);

          close_nf (&nf, TRUE);
        }
    }

  return num;
}

/* verify_sequencer - look through a notesfile with the sequencer to see if it
 * really has new notes, with our blacklist taken into effect.
 *
 * Returns: TRUE if the notesfile really has new notes, FALSE otherwise.
 */

int
verify_sequencer (struct notesfile *nf)
{
  struct newt note;
  memset (&note, 0, sizeof (struct newt));

  nfref_copy (&note.nr.nfr, nf->ref);
  note.nr.notenum = 0;
  note.nr.respnum = 0;

  note.nr.notenum = get_next_note (&note.nr, seqtime);

  if (note.nr.notenum < 0)
    return FALSE;

  while (TRUE)
    {
      get_note (&note, FALSE);

      if (difftime (note.modified, seqtime) > 0 &&
          (difftime (note.created, seqtime) > 0 || note.nr.respnum != 0))
        {
          if ((euid == note.auth.uid && strcmp (fqdn, note.auth.system) == 0))
            {
              if (seq_own_notes)
                return TRUE;
            }
          else if (!blacklisted (&note))
            return TRUE;
        }

      if (note.nr.respnum != note.total_resps)
        {
          if ((note.nr.respnum = get_next_resp (&note.nr, seqtime)) > 0)
            continue;
        }

      note.nr.respnum = 0;
      if ((note.nr.notenum = get_next_note (&note.nr, seqtime)) > 0)
        continue;
      else
        return FALSE;
    }
}

/* printf_to_message_buffer - this hairy (but portable!) variadic function adds
 * a new message to the current message queue to be printed after we're done
 * with curses.
 */

void
#if STDC_HEADERS
printf_to_message_buffer (const char *format, ...)
#else
printf_to_message_buffer (format, va_alist)
     const char *format;
     va_dcl
#endif
{
  char *buffer = NULL;
  char *copy = newts_strdup (messages);
  int result;
#ifdef VA_START
  va_list ap;
  VA_START (ap, format);

  result = vasprintf (&buffer, format, ap);
  if (result == -1 || buffer == NULL)
    {
      newts_free (copy);
      return;
    }

  va_end (ap);
#else
  result = asprintf (&buffer, format, va_alist);
  if (result == -1 || buffer == NULL)
    {
      newts_free (copy);
      return;
    }
#endif

  messages = newts_nrealloc (messages, strlen (buffer) + strlen (messages) + 2,
                             sizeof (char));
  strcpy (messages, copy);
  strcat (messages, buffer);
  strcat (messages, "\n");

  newts_free (copy);
  newts_free (buffer);
}
