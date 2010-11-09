/*
 * uiuc-backend.h - header file wrapper for UIUC backend
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

#ifndef UIUC_BACKEND_H
#define UIUC_BACKEND_H

#include "internal.h"

#include "newts/newts.h"
#include "newts/uiuc.h"
#include "newts/uiuc-compatibility.h"

#include "misc.h"
#include "access.h"
#include "disk.h"

extern void get_uiuc_time (struct when_f *when, time_t t);
extern int load_note (struct newt *newtp, struct daddr_f *daddr,
                      short updatestats);
extern int logical_resp (struct io_f *iop, int notenum, int respnum,
                         struct resp_f *resp, int *offset, int *record);
extern int put_note (struct io_f *io, struct daddr_f *where, struct newt *newt,
           int flags);
extern int put_resp (struct io_f *io, struct daddr_f *where, struct newt *newt,
           int flags);

#endif /* not UIUC_BACKEND_H */
