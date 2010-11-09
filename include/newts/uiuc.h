/*
 * uiuc.h - Description of UIUC notes for the uiuc.so module
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
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

#ifndef NEWTS_UIUC_H
#define NEWTS_UIUC_H

#include "newts/newts.h"

/* These are the options specific to the UIUC backend. */

struct uiuc_opts
{
  int deleted_notes;
  int deleted_resps;
  int expire_threshold;
  int expire_action;
  int expire_by_dirmsg;
  int minimum_notes;
  int maximum_note_size;
  int notesfile_number;
  int current_note_id;
  int unused;
};

/* Expiration actions */

enum expire_actions
  {
    KEEPDFLT = 0,
    KEEPNO,
    KEEPYES
  };

/* Expire with director message options */

enum expire_with_dirmsg_opts
  {
    DIRDFLT = 0,
    DIRNOCARE,
    DIRON,
    DIROFF,
    DIRANYON
  };

/* Public function declarations; these are all implementations of the API
 * described in client.h. */

#ifdef __cplusplus
extern "C" {
#endif

extern int uiuc_author_search (struct newtref *nrp, const char *search);
extern int uiuc_close_nf (struct notesfile *nfp, short updatestats);
extern int uiuc_compress_nf (struct notesfile *nfp, unsigned *numnotes,
                             unsigned *numresps);
extern int uiuc_create_nf (const newts_nfref *ref, int flags);
extern int uiuc_delete_nf (const newts_nfref *ref);
extern int uiuc_delete_note (struct newtref *nrp);
extern int uiuc_get_access_list (const newts_nfref *ref, List *list);
extern int uiuc_get_next_bug (const struct notesfile *nf);
extern int uiuc_get_next_note (struct newtref *nrp, time_t seq);
extern int uiuc_get_next_resp (struct newtref *nrp, time_t seq);
extern int uiuc_get_note (struct newt *notep, short updatestats);
extern int uiuc_get_seqtime (const newts_nfref *ref, const char *name,
                             time_t *seq);
extern int uiuc_get_stats (const newts_nfref *ref, struct stats *stats);
extern int uiuc_modify_nf (struct notesfile *nfp);
extern int uiuc_modify_note (struct newt *notep, int flags);
extern int uiuc_modify_note_text (struct newt *notep);
extern int uiuc_open_nf (const newts_nfref *ref, struct notesfile *nf);
extern int uiuc_set_seqtime (const newts_nfref *ref, const char *name,
                             time_t seq);
extern int uiuc_text_search (struct newtref *nrp, const char *search);
extern int uiuc_title_search (struct newtref *nrp, const char *search);
extern int uiuc_update_nf (struct notesfile *nfp);
extern int uiuc_write_access_list (const newts_nfref *ref, List *list);
extern int uiuc_write_note (struct notesfile *nf, struct newt *note,
                            int flags);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_UIUC_H */
