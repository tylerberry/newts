/*
 * curses-wrapper.h - abstract wrapper around the curses library
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

#ifndef CURSES_WRAPPER_H
#define CURSES_WRAPPER_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"

extern inline bool entered_curses (void);
extern inline bool currently_in_curses (void);
extern void ensure_curses (void);
extern void exit_curses (void);
extern void reenter_curses (void);

#if __STDC__
extern void status_message (int column, const char *message, ...);
#else
extern void status_message ();
#endif

#endif /* not CURSES_WRAPPER_H */
