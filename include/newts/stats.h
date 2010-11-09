/*
 * stats.h - definition of stats datatype
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2006, 2008 Tyler Berry.
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

/** @file newts/stats.h
 * Notesfile statistics datatype and related functions.
 */

#ifndef NEWTS_STATS_H
#define NEWTS_STATS_H

#include "newts/config.h"
#include "newts/nfref.h"

/**
 * Various statistics about a given notesfile.
 */

struct stats
{
  unsigned notes_read;       /**< Number of times any note in this notesfile
                              * has been read. */
  unsigned resps_read;       /**< Number of times any response in this
                              * notesfile has been read. */
  unsigned notes_written;    /**< Number of times a note has been written in
                              * this notesfile. */
  unsigned resps_written;    /**< Number of times a response has been written
                              * in this notesfile. */
  unsigned notes_received;   /**< Number of times a note has been received via
                              * the network and inserted into this notesfile.
                              * Currently unused. */
  unsigned resps_received;   /**< Number of times a response has been received
                              * via the network and inserted into this
                              * notesfile.  Currently unused. */
  unsigned notes_sent;       /**< Number of times a note from this notesfile
                              * has been transmitted via the network.
                              * Currently unused. */
  unsigned resps_sent;       /**< Number of times a response from this
                              * notesfile has been transmitted via the
                              * network.  Currently unused. */
  unsigned notes_dropped;    /**< Number of duplicate notes received and
                              * discarded.  Currently unused. */
  unsigned resps_dropped;    /**< Number of duplicate responses received and
                              * discarded.  Currently unused. */
  unsigned network_sends;    /**< Number of times an outgoing network
                              * connection has been established for this
                              * notesfile.  Currently unused. */
  unsigned network_receipts; /**< Number of times an incoming network
                              * connection has been established for this
                              * notesfile.  Currently unused. */
  unsigned orphans_received; /**< Number of 'orphaned responses', or responses
                              * for which the basenote could not be determined,
                              * received via the network for this notesfile.
                              * Currently unused. */
  unsigned orphans_adopted;  /**< Number of 'orphaned responses', or responses
                              * for which the basenote could not be determined,
                              * received via the network and adopted into a
                              * basenote in this notesfiles.  Currently
                              * unused. */
  unsigned entries;          /**< Number of times a user has entered this
                              * notesfile. */
  unsigned total_time;       /**< Total time in seconds spent in this notesfile
                              * by users. */
  time_t created;            /**< The date this notesfile was created. */
  time_t last_used;          /**< The last time this notesfile was entered by a
                              * user. */
  unsigned days_used;        /**< The number of distinct calendars days on
                              * which at least one user entered this
                              * notesfile. */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Add the statistics from one notesfiles to a running total.
 *
 * @param stats The statistics to add to the running total.
 * @param total The running total of statistics.
 *
 * @par Side effects:
 * The statistics stored in @e total will be modified.
 */
extern void stats_accumulate (const struct stats *stats,
                              struct stats *total);

/**
 * Create a new struct stats.
 *
 * @return A newly allocated struct stats. This should be freed with free_stats
 * when no longer needed.
 *
 * @sa free_stats
 */
extern struct stats *stats_alloc (void);

/**
 * Deallocate @e stats.
 *
 * @param stats The structure to deallocate.
 *
 * @par Side effects:
 * @e stats will be invalidated, and should no longer be used.
 *
 * @sa alloc_stats
 */
extern void stats_free (struct stats *stats);

/**
 * Retrieve usage statistics for the notesfile specified in @e ref. Guaranteed
 * to modify every value in @e stats; therefore, there is no need to call
 * zero_stats before this function.
 *
 * @param ref A reference to a notesfile.
 * @param stats The structure to populate with statistics for @e ref.
 *
 * @return FIXME
 *
 * @par Side effects:
 * The statistics stored in @e stats will be modified.
 */
extern inline int get_stats (const newts_nfref *ref, struct stats *stats);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_STATS_H */
