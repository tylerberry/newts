/*
 * misc.h - miscellaneous function prototypes
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

#ifndef MISC_H
#define MISC_H

#include "uiuc-backend.h"

/* These prototypes are for utility functions used in various parts of the UIUC
 * module.
 */

extern int checkpath (const char *name);
extern void getname (struct auth_f *ident, const int anon_flag);
extern void gettime (struct when_f *when, time_t setto);
extern time_t convert_time (struct when_f *when);

#endif /* not MISC_H */
