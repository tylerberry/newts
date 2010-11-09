/*
 * search.h - methods for searching a notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry.
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

#ifndef NEWTS_SEARCH_H
#define NEWTS_SEARCH_H

#include "newts/config.h"
#include "newts/notesfile.h"

#ifdef __cplusplus
extern "C" {
#endif

/* author_search - incremental search for a particular author.
 *
 * Params:
 * - NRP:    A completely filled out struct newtref.
 *           NRP.notenum and NRP.respnum are the note and response from which
 *           the search should begin.
 * - AUTHOR: The string to search for in each note's 'username@host' string..
 *
 * Returns:
 *   -1 if no author was found; otherwise returns the number of the basenote
 *   in which the note was found.
 *
 * Side effects:
 * - NRP.notenum and NRP.respnum are set to the note and response found.  If
 *   nothing was found, and therefore author_search returned -1, NRP.notenum
 *   and NRP.respnum are not guaranteed to have any meaningful value.
 */
extern inline int author_search (struct newtref *nrp, const char *author);

extern inline int text_search (struct newtref *nrp, const char *search);
extern inline int title_search (struct newtref *nrp, const char *search);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_SEARCH_H */
