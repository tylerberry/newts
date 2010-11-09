/*
 * update_nf.c - update a struct newtsfile with current data
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "uiuc-backend.h"

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

int
uiuc_update_nf (struct notesfile *nf)
{
  struct io_f io;
  struct uiuc_opts *opts;
  int error;

  opts = (struct uiuc_opts *) nf->opts;

  error = init (&io, nf->ref);
  if (error != NEWTS_NO_ERROR)
    return error;

  nf->title = newts_nrealloc (nf->title, sizeof (io.descr.d_title) + 1,
                              sizeof (char));
  strncpy (nf->title, io.descr.d_title, sizeof (io.descr.d_title) + 1);

  nf->director_message = newts_nrealloc (nf->director_message,
                                         sizeof (io.descr.d_title) + 1,
                                         sizeof (char));
  strncpy (nf->director_message, io.descr.d_drmes,
           sizeof (io.descr.d_drmes) + 1);

  nf->total_notes = io.descr.d_nnote;

  nf->options = 0;
  if (io.descr.d_stat & ANONOK)
    nf->options |= NF_ANONYMOUS;
  if (!(io.descr.d_stat & ISOPEN))
    nf->options |= NF_LOCKED;
  if (io.descr.d_stat & ISARCHIVE)
    nf->options |= NF_ARCHIVE;
  if (io.descr.d_stat & ISMODERATED)
    nf->options |= NF_MODERATED;
  if (io.descr.d_plcy)
    nf->options |= NF_POLICY;

  nf->modified = convert_time (&io.descr.d_lastm);

  opts->deleted_notes = io.descr.d_delnote;
  opts->deleted_resps = io.descr.d_delresp;
  opts->expire_threshold = io.descr.d_archtime;
  opts->expire_action = io.descr.d_archkeep;
  opts->expire_by_dirmsg = io.descr.d_dmesgstat;
  opts->minimum_notes = io.descr.d_workset;
  opts->maximum_note_size = io.descr.d_longnote;

  nf->perms = io.access;

  closenf (&io);

  return NEWTS_NO_ERROR;
}
