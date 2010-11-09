/*
 * author.h - definition of the author datatype
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2007, 2008 Tyler Berry.
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

/** @file newts/author.h
 * author datatype and related functions.
 */

#ifndef NEWTS_AUTHOR_H
#define NEWTS_AUTHOR_H

#include "newts/config.h"

/**
 * Identifying information about the author of a note or reply.
 */
struct author
{
  char *name;   /**< The username of the author. */
  char *system; /**< The hostname of the system the author posted from. */
  uid_t uid;    /**< The uid of the author on the above system. */
};

#ifdef __cplusplus
extern "C" {
#endif

extern struct author *author_alloc (void);
extern void author_free (struct author *author);

extern char *author_name (const struct author *author);
extern void author_set_name (struct author *author, const char *new_name);
extern char *author_system (const struct author *author);
extern void author_set_system (struct author *author, const char *new_system);
extern uid_t author_uid (const struct author *author);
extern void author_set_uid (struct author *author, uid_t new_uid);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_AUTHOR_H */
