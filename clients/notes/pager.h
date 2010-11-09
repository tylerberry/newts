/*
 * pager.h - manage a logical text into screen-sized pieces
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

#ifndef PAGER_H
#define PAGER_H

#include "internal.h"

struct pager
{
  char *buffer;
  int textlen;
  int bufptr;
  int pagesout;
  int rot13;
  long pagecnt[51];
  int saved_pagesout;
  int highlight;
  unsigned number_of_columns;
  unsigned lines_output;
  unsigned columns_output;
};

extern void initialize_pager (struct pager *pager, char *text);
extern void free_pager (struct pager *pager);

extern inline int pager_at_beginning (struct pager *pager);
extern inline int pager_at_end (struct pager *pager);

extern inline void arrange_replot (struct pager *pager);

extern int page_up (struct pager *pager);
extern int page_down (struct pager *pager);

extern inline void save_position (struct pager *pager);
extern inline void restore_position (struct pager *pager);

extern inline void toggle_rot13 (struct pager *pager);
extern inline void set_highlight (struct pager *pager, int highlight);

extern int show_text (struct pager *pager);

#endif
