/*
 * stats.c - methods for handling the stats data type
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

#include "internal.h"
#include "newts/memory.h"
#include "newts/stats.h"

void
stats_accumulate (const struct stats *stats, struct stats *total)
{
  total->notes_read += stats->notes_read;
  total->resps_read += stats->resps_read;
  total->notes_written += stats->notes_written;
  total->resps_written += stats->resps_written;
  total->notes_received += stats->notes_received;
  total->resps_received += stats->resps_received;
  total->notes_sent += stats->notes_sent;
  total->resps_sent += stats->resps_sent;
  total->notes_dropped += stats->notes_dropped;
  total->resps_dropped += stats->resps_dropped;
  total->network_sends += stats->network_sends;
  total->network_receipts += stats->network_receipts;
  total->orphans_received += stats->orphans_received;
  total->orphans_adopted += stats->orphans_adopted;
  total->entries += stats->entries;
  total->total_time += stats->total_time;
  total->days_used += stats->days_used;

  return;
}

struct stats *
stats_alloc (void)
{
  return (struct stats *) newts_zalloc (sizeof (struct stats));
}

void
stats_free (struct stats *stats)
{
  newts_free (stats);
}
