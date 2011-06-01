/*
 * access.c - methods for handling the access data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2007 Tyler Berry
 *
 * Based in part on acssort.c from the UIUC notes distribution by Ray Essick
 * and Rob Kolstad.  Any work derived from this source code is required to
 * retain this notice.
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"
#include "newts/access.h"
#include "newts/memory.h"

void
access_add_permissions (struct access *access, unsigned permissions)
{
  access->permissions |= permissions;
}

struct access *
access_alloc (void)
{
  return (struct access *) newts_zalloc (sizeof (struct access));
}

void
access_clear_permissions (struct access *access)
{
  access->permissions = 0;
}

int
access_compare (const struct access *one, const struct access *two)
{
  if (one->scope < two->scope)
    return -1;
  if (one->scope > two->scope)
    return 1;
  if (strcasecmp ("other", one->name) == 0)
    {
      if (strcasecmp ("other", two->name) == 0)
        return 0;
      else
        return 1;
    }
  if (strcasecmp ("other", two->name) == 0)
    return -1;
  return strcmp (one->name, two->name);
}

void
access_free (struct access *access)
{
  if (access->name)
    newts_free (access->name);

  newts_free (access);
}

int
access_has_permissions (const struct access *access, unsigned permissions)
{
  return access->permissions & permissions;
}

char *
access_name (const struct access *access)
{
  return access->name;
}

unsigned
access_permissions (const struct access *access)
{
  return access->permissions;
}

void
access_remove_permissions (struct access *access, unsigned permissions)
{
  access->permissions &= ~permissions;
}

enum newts_access_scopes
access_scope (const struct access *access)
{
  return access->scope;
}

void
access_set_name (struct access *access, const char *new_name)
{
  if (access->name)
    newts_free (access->name);

  if (new_name)
    access->name = newts_strdup (new_name);
  else
    access->name = NULL;
}

void
access_set_permissions (struct access *access, unsigned new_permissions)
{
  access->permissions = new_permissions;
}

void
access_set_scope (struct access *access, enum newts_access_scopes new_scope)
{
  access->scope = new_scope;
}
