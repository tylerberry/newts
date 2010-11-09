/*
 * error.h - interface to handling errors in the backends
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

#ifndef NEWTS_ERROR_H
#define NEWTS_ERROR_H

#include "newts/config.h"

#define NEWTS_NO_ERROR                0
#define NEWTS_NF_DOESNT_EXIST        -1
#define NEWTS_NULL_POINTER           -2
#define NEWTS_UNABLE_TO_OPEN         -3
#define NEWTS_INCORRECT_DBVERSION    -4
#define NEWTS_ALREADY_COMPRESSING    -5
#define NEWTS_INVALID_NOTESFILE_NAME -6

#endif /* not NEWTS_ERROR_H */
