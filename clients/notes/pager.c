/*
 * pager.c - manage a logical text into screen-sized pieces
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry.
 *
 * Based in part on dsply.c from the UIUC notes distribution by Ray Essick and
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
#include "pager.h"

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

static inline void display_backspace (struct pager *pager);
static inline void display_characters (struct pager *pager, unsigned n);
static inline void display_formfeed (struct pager *pager);
static void display_single_character (struct pager *pager);
static inline void display_newline (struct pager *pager);
static inline void display_tab (struct pager *pager);
static inline int next_word_fits (struct pager *pager);

void
initialize_pager (struct pager *pager, char *text)
{
  pager->number_of_columns = 0;
  pager->lines_output = 0;
  pager->columns_output = 0;
  pager->bufptr = 0;
  pager->pagesout = 0;
  pager->pagecnt[0] = 0;
  pager->saved_pagesout = 0;
  pager->rot13 = FALSE;
  pager->highlight = FALSE;
  pager->buffer = newts_strdup (text);
  pager->textlen = strlen (pager->buffer);
}

void
free_pager (struct pager *pager)
{
  newts_free (pager->buffer);
  pager->buffer = NULL;
}

inline int
pager_at_beginning (struct pager *pager)
{
  return (pager->pagesout == 0);
}

inline int
pager_at_end (struct pager *pager)
{
  return (pager->bufptr >= pager->textlen);
}

inline void
arrange_replot (struct pager *pager)
{
  pager->bufptr = pager->pagecnt[pager->pagesout];
}

int
page_up (struct pager *pager)
{
  int result;

  if (!pager_at_beginning (pager))
    {
      pager->pagesout--;
      result = 1;
    }
  else
    result = 0;

  arrange_replot (pager);
  return result;
}

int
page_down (struct pager *pager)
{
  if (!pager_at_end (pager))
    {
      if (pager->pagesout < 50)
        pager->pagecnt[++pager->pagesout] = pager->bufptr;
      return 1;
    }
  else
    {
      arrange_replot (pager);
      return 0;
    }
}

inline void
save_position (struct pager *pager)
{
  pager->saved_pagesout = pager->pagesout - 1;
}

inline void
restore_position (struct pager *pager)
{
  pager->pagesout = pager->saved_pagesout;
  pager->saved_pagesout = 0;
  arrange_replot (pager);
}

inline void
set_highlight (struct pager *pager, int highlight)
{
  pager->highlight = highlight;
}

inline void
toggle_rot13 (struct pager *pager)
{
  pager->rot13 = !pager->rot13;
}

int
show_text (struct pager *pager)
{
  int highlighted = FALSE;

  pager->lines_output = 4;
  pager->columns_output = 0;

  if (traditional)
    pager->number_of_columns = 80;
  else
    pager->number_of_columns = COLS;

  while (pager->lines_output < LINES - 1 && pager->bufptr < pager->textlen)
    {
      if (pager->highlight &&
          strncasecmp ((pager->buffer + pager->bufptr), txtsearch,
                       strlen (txtsearch)) == 0)
        {
          highlighted = TRUE;
          attron (A_REVERSE);

          display_characters (pager, strlen (txtsearch));

          attroff (A_REVERSE);
        }
      else
        {
          display_single_character (pager);
        }
    }

  return pager->highlight ? highlighted : TRUE;
}

static inline void
display_backspace (struct pager *pager)
{
  if (pager->columns_output > 0)
    {
      addch ('\b');
      pager->columns_output--;
    }
}

static inline void
display_characters (struct pager *pager, unsigned n)
{
  unsigned i;

  for (i = 0; i < n; i++)
    {
      display_single_character (pager);
    }
}

static inline void
display_formfeed (struct pager *pager)
{
  clrtobot ();
  pager->lines_output = LINES;   /* Force the next page */
}

static inline void
display_newline (struct pager *pager)
{
  addch ('\n');
  pager->columns_output = 0;
  pager->lines_output++;
}

static void
display_single_character (struct pager *pager)
{
  register int c = pager->buffer[pager->bufptr];
  int naive_wrap_flag = FALSE;

  if (pager->rot13)
    {
      if ((c >= 'a' && c <= 'm') || (c >= 'A' && c <= 'M'))
        c += 13;
      else if ((c >= 'n' && c <= 'z') || (c >= 'N' && c <= 'Z'))
        c -= 13;
    }

  pager->bufptr++;

  if (naive_wrap_flag == FALSE && c != '\n')
    {
      naive_wrap_flag = TRUE;

      if (next_word_fits (pager))
        {
          display_newline (pager);
          if (pager->lines_output == LINES - 1)
            {
              pager->bufptr--;
              return;
            }
        }
    }

  switch (c)
    {
    case '\n':
      naive_wrap_flag = FALSE;
      display_newline (pager);
      break;

    case '\014':
      display_formfeed (pager);
      break;

    case '\t':
      display_tab (pager);
      break;

    case '\b':
      display_backspace (pager);
      break;

    case ' ':
      naive_wrap_flag = FALSE;
      /* Fallthrough */

    default:
      addch (c);
      pager->columns_output++;
      break;
    }

  if (pager->columns_output >= pager->number_of_columns)
    {
      display_newline (pager);
    }
}

static inline void
display_tab (struct pager *pager)
{
  addch ('\t');
  pager->columns_output += TABSIZE - (TABSIZE % 8);
}

static inline int
next_word_fits (struct pager *pager)
{
  char *next_space = strpbrk (pager->buffer + pager->bufptr, " \n\r");

  return (next_space != NULL &&
          ((next_space - (pager->buffer + pager->bufptr)) >=
           (ptrdiff_t) (pager->number_of_columns - pager->columns_output) &&
           pager->columns_output));
}
