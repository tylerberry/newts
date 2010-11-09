/*
 * compose_note.c - handle writing a new note into a notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on adnote.c from the UIUC notes distribution by Ray Essick and
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

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

/* compose_note - edit and add a note or response to NFP.
 *
 * QUOTE can either be a particular note to quote into the note, or it can be
 * NULL; if it is NULL, that means "do not quote".  TEXTSTR is a string that
 * will be printed before the client forks to get the note text; TITLESTR is
 * the prompt for the note title.
 *
 * NOTENUM should be -1 if you are adding a new basenote; otherwise, it should
 * be the basenote you're replying to.
 *
 * MODE controls whether to prompt for text and title, or just to use what
 * we're given in QUOTE, and how to quote if we're quoting; to say COPYNOEDIT
 * or VECTOR for INTERACTIVE and to provide nothing to QUOTE is an error.
 *
 * TEXTSTR and TITLESTR are ignored if mode is COPYNOEDIT or VECTOR.
 *
 * Returns: -1 on error (or empty text), otherwise the number of the newly
 * created note or response.
 */

int
compose_note (struct notesfile *nf, struct newt *quote, char *textstr,
              char *titlestr, int notenum, short mode)
{
  struct newt note;
  int result;

  memset (&note, 0, sizeof (struct newt));

  if ((mode == COPYNOEDIT || mode == VECTOR) && quote == NULL)
    return -1;  /* Silly, you have to give us something to copy. */

  if (mode != COPYNOEDIT && mode != VECTOR)
    {
      move (LINES - 2, 0);
      clrtoeol ();
      printw (textstr);
      refresh ();
    }

  note.text = get_text (quote, mode);

  if (note.text == NULL)  /* Changed your mind. */
    return -1;

  if (mode == NORMAL && nf->options & NF_ANONYMOUS)
    {
      int yn;

      clear ();
      YES_OR_NO (yn,
                 if (traditional)
                   mvprintw (LINES - 1, 0,
                             _("Do you wish this to be anonymous (y/n): "));
                 else
                   mvprintw (LINES - 2, 0,
                             _("Make this note anonymous? (y/n): "));
                 );
      if (yn)
        {
          if (traditional)
            {
              clear ();
              YES_OR_NO (yn,
                         mvprintw (LINES - 1, 0,
                                   _("Do you REALLY wish this to be "
                                     "anonymous (y/n): "));
                         );
              if (yn)
                note.options |= NOTE_ANONYMOUS;
            }
          else
            note.options |= NOTE_ANONYMOUS;
        }
    }

  note.director_message = NULL;

  if (allowed (nf, DIRECTOR) && mode != COPYNOEDIT && mode != VECTOR)
    {
      int yn;

      clear ();
      YES_OR_NO (yn,
                 mvprintw (traditional ? LINES - 1 : LINES - 2, 0,
                           traditional ? _("Director message (y/n): ") :
                           _("Director message? (y/n): "));
                 );
      if (yn)
        note.director_message = "t";
    }

  /* Here, we're prompting for a title if applicable. */

  if (mode != COPYNOEDIT && mode != VECTOR)
    {
      clear ();
      move (LINES - 2, 0);
      clrtoeol ();

      if (notenum == -1)
        {
          char *prompt = NULL;

          /* Prepare the prompt. */

          if (traditional)
            {
              move (LINES - 1, 0);
              clrtoeol ();
              prompt = newts_nmalloc (strlen (titlestr) + 15, sizeof (char));
              strcpy (prompt, "              ");
              strcat (prompt, titlestr);
            }
          else
            {
              prompt = newts_nmalloc (strlen (titlestr) + 1, sizeof (char));
              strcpy (prompt, titlestr);
            }

          refresh ();

          note.title = gl_getline (prompt);
          note.title[strlen (note.title) - 1] = '\0'; /* Chomp trailing '\n' */
          gl_histadd (note.title);

          newts_free (prompt);
        }
      else
        {
          note.title = NULL;
        }
    }
  else
    {
      note.title = quote->title;
    }

  time (&note.created);
  time (&note.modified);

  if (note.options & NOTE_ANONYMOUS)
    note.auth.name = "Anonymous";
  else
    note.auth.name = username;
  note.auth.system = fqdn;
  if (note.options & NOTE_ANONYMOUS)
    note.auth.uid = anon_uid;
  else
    note.auth.uid = euid;

  if (!allowed (nf, READ))
    note.options |= NOTE_WRITE_ONLY;

  note.nr.notenum = notenum;

  result = write_note (nf, &note, UPDATE_TIMES + ADD_ID);

  newts_free (note.text);

  return result;
}
