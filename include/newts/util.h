/*
 * util.h - utility functions used internally and exported for convenience
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2007 Tyler Berry.
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

#ifndef NEWTS_UTIL_H
#define NEWTS_UTIL_H

#include "newts/config.h"

#ifdef __cplusplus
extern "C" {
#endif

extern char *newts_get_fqdn (void);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_UTIL_H */
