/*
 * access.h - definition of the access datatype
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

/** @file newts/access.h
 * access rule datatype and related functions.
 */

#ifndef NEWTS_ACCESS_H
#define NEWTS_ACCESS_H

#include "newts/config.h"
#include "newts/enums.h"
#include "newts/list.h"
#include "newts/notesfile.h"

/**
 * An individual access rule.
 */
struct access
{
  char *name;               /**< The username, group name, or host FQDN,
                             * depending on which @e scope. */
  enum newts_access_scopes scope; /**< The scope of this access rule. */
  unsigned permissions;     /**< The actual permissions, as a bitmap of @e enum
                             * newts_access_permissions. */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new struct access.
 *
 * @return A newly allocated struct access. This should be freed with
 * access_free when no longer needed.
 *
 * @sa access_free
 */
extern struct access *access_alloc (void);

/**
 * Compare two access rules, @e one and @e two, by the standard sorting rules
 * used by the original notesfiles: first sorting by scope, where the ordering
 * is user < group < system, then alphabetically by name. The name "other" is
 * treated specially, and always sorts to the bottom. The name comparison is
 * case-sensitive. Suitable as an argument to qsort(3) or other functions using
 * the qsort interface.
 *
 * @return
 * @li -1 if @e one is "less than" than @e two.
 * @li 0 if @e one and @e two are "equal".
 * @li 1 if @e one is "greater" than @e two.
 */
extern int access_compare (const struct access *one,
                           const struct access *two);

/**
 * Deallocate @e ref.
 *
 * @par Side effects:
 * @e ref will be invalidated, and should no longer be used.
 *
 * @sa access_alloc
 */
extern void access_free (struct access *access);

extern int access_has_permissions (const struct access *access,
                                   unsigned permissions);
extern void access_clear_permissions (struct access *access);
extern void access_add_permissions (struct access *access,
                                    unsigned permissions);
extern void access_remove_permissions (struct access *access,
                                       unsigned permissions);

extern char *access_name (const struct access *access);
extern unsigned access_permissions (const struct access *access);
extern enum newts_access_scopes access_scope (const struct access *access);
extern void access_set_name (struct access *access, const char *new_name);
extern void access_set_permissions (struct access *acess,
                                    unsigned new_permissions);
extern void access_set_scope (struct access *access,
                              enum newts_access_scopes new_scope);

extern inline int get_access_list (const newts_nfref *ref, List *list);
extern inline int write_access_list (const newts_nfref *ref, List *list);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_ACCESS_H */
