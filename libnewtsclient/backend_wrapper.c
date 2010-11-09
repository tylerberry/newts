/*
 * backend-wrapper.c - functions translating client calls to UIUC calls
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry
 *
 * Newts is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Newts is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Newts; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"
#include "newts/newts.h"
#include "newts/uiuc.h"

inline int
author_search (struct newtref *nrp, const char *search)
{
  return uiuc_author_search (nrp, search);
}

inline int
close_nf (struct notesfile *nf, int updatestats)
{
  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_close_nf (nf, updatestats);
}

inline int
compress_nf (struct notesfile *nf, unsigned *numnotes, unsigned *numresps)
{
  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_compress_nf (nf, numnotes, numresps);
}

inline int
create_nf (const newts_nfref *ref, int flags)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_create_nf (ref, flags);
}

inline int
delete_nf (const newts_nfref *ref)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_delete_nf (ref);
}

inline int
delete_note (struct newtref *nrp)
{
  return uiuc_delete_note (nrp);
}

inline int
get_access_list (const newts_nfref *ref, List *list)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_get_access_list (ref, list);
}

inline int
get_next_bug (const struct notesfile *nf)
{
  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_get_next_bug (nf);
}

inline int
get_next_note (struct newtref *nrp, time_t seq)
{
  return uiuc_get_next_note (nrp, seq);
}

inline int
get_next_resp (struct newtref *nrp, time_t seq)
{
  return uiuc_get_next_resp (nrp, seq);
}

inline int
get_note (struct newt *notep, short updatestats)
{
  return uiuc_get_note (notep, updatestats);
}

inline int
get_seqtime (const newts_nfref *ref, const char *name, time_t *seq)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_get_seqtime (ref, name, seq);
}

inline int
get_stats (const newts_nfref *ref, struct stats *stats)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_get_stats (ref, stats);
}

inline int
modify_nf (struct notesfile *nf)
{
  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_modify_nf (nf);
}

inline int
modify_note (struct newt *notep, int flags)
{
  return uiuc_modify_note (notep, flags);
}

inline int
modify_note_text (struct newt *notep)
{
  return uiuc_modify_note_text (notep);
}

inline int
open_nf (const newts_nfref *ref, struct notesfile *nf)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_open_nf (ref, nf);
}

inline int
set_seqtime (const newts_nfref *ref, const char *name, time_t seq)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_set_seqtime (ref, name, seq);
}

inline int
text_search (struct newtref *nrp, const char *search)
{
  return uiuc_text_search (nrp, search);
}

inline int
title_search (struct newtref *nrp, const char *search)
{
  return uiuc_title_search (nrp, search);
}

inline int
update_nf (struct notesfile *nf)
{
  return uiuc_update_nf (nf);
}

inline int
write_access_list (const newts_nfref *ref, List *list)
{
  if (ref == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_write_access_list (ref, list);
}

inline int
write_note (struct notesfile *nf, struct newt *notep, int flags)
{
  if (nf == NULL)
    return NEWTS_NULL_POINTER;

  return uiuc_write_note (nf, notep, flags);
}
