/*
 * sequencer.h - methods used to interact with the sequencer
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2008 Tyler Berry.
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

#ifndef NEWTS_SEQUENCER_H
#define NEWTS_SEQUENCER_H

#include "newts/config.h"
#include "newts/nfref.h"
#include "newts/notesfile.h"

#ifdef __cplusplus
extern "C" {
#endif

extern inline int get_next_note (struct newtref *nrp, time_t seq);
extern inline int get_next_resp (struct newtref *nrp, time_t seq);
extern inline int get_seqtime (const newts_nfref *ref, const char *name,
                               time_t *seq);
extern inline int set_seqtime (const newts_nfref *ref, const char *name,
                               time_t seq);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_SEQUENCER_H */
