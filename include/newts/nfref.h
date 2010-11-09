/*
 * nfref.h - definition of the newts_nfref data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2006, 2008 Tyler Berry.
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

/** @file newts/nfref.h
 * newts_nfref datatype and related functions.
 */

#ifndef NEWTS_NFREF_H
#define NEWTS_NFREF_H

#include "newts/config.h"
#include "newts/connection.h"
#include "newts/enums.h"

typedef struct newts_nfref newts_nfref;

/**
 * A reference to a notesfile. Specifically, all the information needed to
 * specify a particular notesfile.
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new notesfile.
 *
 * @param ref A reference that will describe the new notesfile after it has
 *            been created.
 * @param flags A bitmap of flags to set in the newly-created notesfile; can
 *              include NF_ANONYMOUS, NF_LOCKED, and NF_MODERATED.
 *
 * @return FIXME
 */
extern inline int create_nf (const newts_nfref *ref, int flags);

/**
 * Delete the existing notesfile specified by @e ref..
 *
 * @return FIXME
 */
extern inline int delete_nf (const newts_nfref *ref);

/**
 * Create a new newts_nfref.
 *
 * @return A newly allocated newts_nfref. This should be freed with nfref_free
 * when no longer needed.
 *
 * @sa nfref_free
 */
extern newts_nfref *nfref_alloc (void);

/**
 * Compare two nfrefs, @e one and @e two, first alphabetically by owner then
 * alphabetically by name. Both comparisons are case-sensitive, and NULL owner
 * is considered "less-than" any non-NULL owner. Suitable as an argument to
 * qsort(3) or other functions using the qsort interface.
 *
 * @return
 * @li -1 if @e one is "less than" than @e two.
 * @li 0 if @e one and @e two are "equal".
 * @li 1 if @e one is "greater" than @e two.
 */
extern int nfref_compare (const newts_nfref *one, const newts_nfref *two);

/**
 * Duplicate an existing nfref. This is a deep copy; strings from @e source
 * will be copied, and @e dest will point to those copied strings, not the
 * originals from @e source. This function is safe to use if overwriting
 * existing values in @e dest.
 *
 * @param dest The nfref to copy values to.
 * @param source The nfref to copy values from.
 *
 * @par Side effects:
 * The values stored in @e dest will be modified. Additionally, if @e dest was
 * previously populated, its existing string elements will be freed before
 * being overwritten.
 */
extern void nfref_copy (newts_nfref *dest, const newts_nfref *source);

/**
 * Deallocate @e ref.
 *
 * @par Side effects:
 * @e ref will be invalidated, and should no longer be used.
 *
 * @sa nfref_alloc
 */
extern void nfref_free (newts_nfref *ref);

/**
 * Return the name of the notesfile specified by @e ref.
 *
 * @sa nfref_set_name
 */
extern char *nfref_name (const newts_nfref *ref);

/**
 * Return the owner of the notesfile specified by @e ref. NULL means that the
 * notesfile is a global notesfile, not a private notesfile.
 *
 * @sa nfref_set_owner
 */
extern char *nfref_owner (const newts_nfref *ref);

/**
 * Return the port with which to access notesfile specified by @e ref.
 *
 * @sa nfref_set_port
 */
extern unsigned short nfref_port (const newts_nfref *ref);

/**
 * Return a user-readable pretty representation of the notesfile specified by
 * @e ref. The output of this function should be able to be parsed with @ref
 * parse_single_nf "parse_single_nf" or the other parsing functions to return
 * an equivalent (although not necessarily identical) nfref.
 */
extern char *nfref_pretty_name (newts_nfref *ref);

/**
 * Return the protocol to use to access the notesfile specified by @e ref.
 *
 * @sa nfref_set_protocol
 */
extern enum newts_protocols nfref_protocol (const newts_nfref *ref);

/**
 * Modify @e ref to refer to a notesfile with name @e new_name.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_name
 */
extern void nfref_set_name (newts_nfref *ref, const char *new_name);

/**
 * Modify @e ref to refer to a notesfile owned by @e new_owner.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_owner
 */
extern void nfref_set_owner (newts_nfref *ref, const char *new_owner);

/**
 * Modify @e ref to refer to a notesfile accessed with port @e new_port.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_port
 */
extern void nfref_set_port (newts_nfref *ref,
                            const unsigned short new_port);

/**
 * Modify @e ref to refer to a notesfile accessed with protocol @e
 * new_protocol.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_protocol
 */
extern void nfref_set_protocol (newts_nfref *ref,
                                const enum newts_protocols new_protocol);

/**
 * Modify @e ref to refer to a notesfile on system @e new_system.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_system
 */
extern void nfref_set_system (newts_nfref *ref, const char *new_system);

/**
 * Modify @e ref to refer to a notesfile to which we're going to connect as @e
 * new_user.
 *
 * @par Side effects:
 * @e ref will be modified.
 *
 * @sa nfref_user
 */
extern void nfref_set_user (newts_nfref *ref, const char *new_user);

/**
 * Return the system on which the notesfile specified by @e ref is found.
 *
 * @sa nfref_set_system
 */
extern char *nfref_system (const newts_nfref *ref);

/**
 * Return TRUE if the notesfile specified by @e ref refers to a notesfile on
 * localhost, or FALSE otherwise.
 *
 * This is defined to be true if @ref nfref_system "nfref_system (ref)" would
 * return NULL, "", or "localhost".
 *
 * FIXME: Obviously, this is a little braindead.
 *
 * @sa nfref_system
 */
extern int nfref_system_is_localhost (const newts_nfref *ref);

/**
 * Return the user with which we're going to connect to the notesfile specified
 * by @e ref.
 *
 * FIXME: Is this really the right place for this? It's here because this is
 * sort of a "how to get to this notesfile" spec, and if we're connecting to a
 * foreign server the user we're connecting as is just as necessary a piece as
 * the hostname, for example. But it still feels kludgy as heck.
 *
 * @sa nfref_set_user, nfref_system_is_localhost
 */
extern char *nfref_user (const newts_nfref *ref);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_NFREF_H */
