/*
 * note.h - definition of the note datatype
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

#ifndef NEWTS_NOTE_H
#define NEWTS_NOTE_H

#include "newts/config.h"
#include "newts/author.h"
#include "newts/enums.h"
#include "newts/notesfile.h"
#include "structs.h"

/* struct newtid - a "unique id" for each newt created.
 *
 * SYSTEM - the system FQDN the note was created on.
 * NUMBER - the "unique" ID number.
 */

struct newtid
{
  char *system;
  long number;
};

/* struct newtref - a reference to a particular newt.
 *
 * NFR     - a reference to the notesfile the newt lives in.
 * NOTENUM - the number of the basenote this newt belongs to.
 * RESPNUM - the response number of the newt; if the actual basenote is
 *           desired, this should be 0.
 */

struct newtref
{
  newts_nfref nfr;
  int notenum;
  int respnum;
};

/* struct newt - metadata about a particular newt.
 *
 * NR               - the newt reference for this newt.
 * ID               - the "unique" ID of the newt.
 * TITLE            - the title of the basenote of this newt.
 * DIRECTOR_MESSAGE - the director message for this newt.
 * AUTH             - the author of the newt.
 * CREATED          - the time this newt was created.
 * MODIFIED         - the most recent time this newt was modified; used by the
 *                    sequencer.
 * TOTAL_RESPS      - the total number of responses to the current basenote.
 * TEXT             - the text of the newt.
 * OPTIONS          - a bitmap of enum note_statuses.
 */

struct newt
{
  struct newtref nr;
  struct newtid id;
  char *title;
  char *director_message;
  struct author auth;
  time_t created;
  time_t modified;
  int total_resps;
  char *text;
  int options;
};

#ifdef __cplusplus
extern "C" {
#endif

extern inline int delete_note (struct newtref *nrp);
extern inline int get_note (struct newt *notep, short updatestats);
extern inline int modify_note (struct newt *notep, int flags);
extern inline int modify_note_text (struct newt *notep);
extern inline int write_note (struct notesfile *nf, struct newt *notep,
                              int flags);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_NOTE_H */
