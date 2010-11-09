/*
 * module.h - ltdl wrapper function prototypes
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2002, 2003 Tyler Berry.
 *
 * Newts is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Newts is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Newts; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef MODULE_H
#define MODULE_H

BEGIN_C_DECLS

extern const char *module_error (void);
extern int module_init (void);

END_C_DECLS

#endif /* not MODULE_H */
