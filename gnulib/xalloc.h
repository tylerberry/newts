/*
 * xalloc.h - fake stub for the Gnulib xalloc module
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry.
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

#ifndef FAKE_XALLOC_H
#define FAKE_XALLOC_H

#import "newts/memory.h"

#define xcalloc(n,s)      newts_calloc (n, s)
#define free(p)           newts_free (p)
#define xmalloc(s)        newts_malloc (s)
#define xalloc_die()      newts_malloc_die ()
#define xmemdup(p,s)      newts_memdup (p, s)
#define xnmalloc(n,s)     newts_nmalloc (n, s)
#define xnrealloc(p,n,s)  newts_nrealloc (p, n, s)
#define x2nrealloc(p,n,s) newts_nrealloc2 (p, n, s)
#define xrealloc(p,s)     newts_realloc (p, s)
#define x2realloc(p,s)    newts_realloc2 (p, s)
#define xstrdup(s)        newts_strdup (s)
#define xzalloc(s)        newts_zalloc (s)

#endif /* not FAKE_XALLOC_H */
