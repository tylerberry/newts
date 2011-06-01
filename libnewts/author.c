/*
 * author.c - methods for handling the author data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2007 Tyler Berry
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
#include "newts/author.h"
#include "newts/memory.h"

struct author *
author_alloc (void)
{
  return (struct author *) newts_zalloc (sizeof (struct author));
}

int
author_compare (const struct author *one, const struct author *two)
{
  int result = strcmp (one->system, two->system);

  if (result == 0)
    return strcmp (one->name, two->name);
  else
    return result;
}

void
author_free (struct author *author)
{
  if (author->name)
    newts_free (author->name);
  if (author->system)
    newts_free (author->system);

  newts_free (author);
}

char *
author_name (const struct author *author)
{
  return author->name;
}

void
author_set_name (struct author *author, const char *new_name)
{
  if (author->name)
    newts_free (author->name);

  if (new_name)
    author->name = newts_strdup (new_name);
  else
    author->name = NULL;
}

char *
author_system (const struct author *author)
{
  return author->system;
}

void
author_set_system (struct author *author, const char *new_system)
{
  if (author->system)
    newts_free (author->system);

  if (new_system)
    author->system = newts_strdup (new_system);
  else
    author->system = NULL;
}

uid_t
author_uid (const struct author *author)
{
  return author->uid;
}

void
author_set_uid (struct author *author, uid_t new_uid)
{
  author->uid = new_uid;
}
