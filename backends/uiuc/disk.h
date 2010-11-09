/*
 * disk.h - disk-related function prototypes
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

#ifndef DISK_H
#define DISK_H

#include "uiuc-backend.h"

/* These prototypes are for disk-related functions used in various parts of the
 * UIUC module.
 */

extern int init (struct io_f *io, const newts_nfref *ref);
extern int closenf (struct io_f *io);
extern int getdescr (struct io_f *io, struct descr_f *descr);
extern int putdescr (struct io_f *io, struct descr_f *descr);
extern void getnoterec (struct io_f *io, int number, struct note_f *note);
extern void putnoterec (struct io_f *io, int number, struct note_f *note);
extern void getresprec (struct io_f *io, int number, struct resp_f *resp);
extern void putresprec (struct io_f *io, int number, struct resp_f *resp);
extern long puttextrec (struct io_f *io, char *text, struct daddr_f *daddr,
                        int flags);
extern long movetextrec (struct io_f *old, struct daddr_f *from,
           struct io_f *new, struct daddr_f *to);

#endif /* not DISK_H */
