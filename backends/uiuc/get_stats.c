/*
 * get_stats.c - get stats for a UIUC notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2008 Tyler Berry.
 *
 * Based on nfstats.c from the UIUC notes distribution by Ray Essick and Rob
 * Kolstad.  Any work derived from this source code is required to retain this
 * notice.
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
uiuc_get_stats (const newts_nfref *ref, struct stats *stats)
{
  struct io_f io;
  int error;

  error = init (&io, ref);
  if (error != NEWTS_NO_ERROR)
    {
      return error;
    }

  stats->notes_read = io.descr.d_notread;
  stats->resps_read = io.descr.d_rspread;
  stats->notes_written = io.descr.d_notwrit;
  stats->resps_written = io.descr.d_rspwrit;
  stats->notes_received = io.descr.d_notrcvd;
  stats->resps_received = io.descr.d_rsprcvd;
  stats->notes_sent = io.descr.d_notxmit;
  stats->resps_sent = io.descr.d_rspxmit;
  stats->notes_dropped = io.descr.d_notdrop;
  stats->resps_dropped = io.descr.d_rspdrop;
  stats->network_sends = io.descr.netwrkouts;
  stats->network_receipts = io.descr.netwrkins;
  stats->orphans_received = io.descr.d_orphans;
  stats->orphans_adopted = io.descr.d_adopted;
  stats->entries = io.descr.entries;
  stats->total_time = io.descr.walltime;
  stats->created = convert_time (&io.descr.d_created);
  stats->last_used = convert_time (&io.descr.d_lastuse);
  stats->days_used = io.descr.d_daysused;

  closenf (&io);

  return NEWTS_NO_ERROR;
}
