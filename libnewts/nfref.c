/*
 * nfref.c - methods for handling the nfref data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005, 2006 Tyler Berry.
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

#include "newts/memory.h"
#include "newts/nfref.h"
#include "newts/util.h"

newts_nfref *
nfref_alloc (void)
{
  return (newts_nfref *) newts_zalloc (sizeof (newts_nfref));
}

/* FIXME: this is incorrectly implemented. */
int
nfref_compare (const newts_nfref *one, const newts_nfref *two)
{
  if (one == NULL && two == NULL) return 0;
  if (one == NULL || two == NULL) return -1;

  if ((!(one->name == NULL && two->name == NULL)
       && ((one->name == NULL || two->name == NULL)
           || strcmp (one->name, two->name)))
      || (!(one->owner == NULL && two->owner == NULL)
          && ((one->owner == NULL || two->owner == NULL)
              || strcmp (one->owner, two->owner)))
      || (!(one->system == NULL && two->system == NULL)
          && ((one->system == NULL || two->system == NULL)
              || strcmp (one->system, two->system)))
      || one->protocol != two->protocol
      || one->port != two->port)
    return -1;

  return 0;
}

void
nfref_copy (newts_nfref *dest, const newts_nfref *source)
{
  if (dest->name)
    newts_free (dest->name);
  if (dest->owner)
    newts_free (dest->owner);
  if (dest->system)
    newts_free (dest->system);
  if (dest->user)
    newts_free (dest->user);
  if (dest->pretty_name)
    {
      newts_free (dest->pretty_name);
      dest->pretty_name = NULL;
    }

  memset (dest, 0, sizeof (newts_nfref));

  dest->port = source->port;
  dest->protocol = source->protocol;

  if (source->name)
    dest->name = newts_strdup (source->name);
  if (source->owner)
    dest->owner = newts_strdup (source->owner);
  if (source->system)
    dest->system = newts_strdup (source->system);
  if (source->user)
    dest->user = newts_strdup (source->user);
}

void
nfref_free (newts_nfref *ref)
{
  if (ref->name)
    newts_free (ref->name);
  if (ref->owner)
    newts_free (ref->owner);
  if (ref->system)
    newts_free (ref->system);
  if (ref->user)
    newts_free (ref->user);
  if (ref->pretty_name)
    newts_free (ref->pretty_name);

  newts_free (ref);
}

char *
nfref_name (const newts_nfref *ref)
{
  return ref->name;
}

char *
nfref_owner (const newts_nfref *ref)
{
  return ref->owner;
}

unsigned short
nfref_port (const newts_nfref *ref)
{
  return ref->port;
}

char *
nfref_pretty_name (newts_nfref *ref)
{
  if (ref == NULL)
    return NULL;

  if (ref->pretty_name)
    newts_free (ref->pretty_name);
  ref->pretty_name = NULL;

  if (ref->name == NULL)
    return NULL;

  if (nfref_system_is_localhost (ref))
    {
      if (ref->owner == NULL)
        {
          ref->pretty_name = newts_nmalloc (strlen (ref->name) + 2,
                                            sizeof (char));
          sprintf (ref->pretty_name, N_("=%s"), ref->name);
        }
      else
        {
          ref->pretty_name = newts_nmalloc (strlen (ref->owner) +
                                            strlen (ref->name) + 3,
                                            sizeof (char));
          sprintf (ref->pretty_name, N_("=%s:%s"), ref->owner, ref->name);
        }
    }
  else
    {
      if (ref->port == NEWTS_NCP_STANDARD_PORT)
        {
          if (ref->owner == NULL)
            {
              ref->pretty_name = newts_nmalloc (strlen (ref->name) +
                                                strlen (ref->system) + 2,
                                                sizeof (char));
              sprintf (ref->pretty_name, N_("=%s/%s"), ref->system, ref->name);
            }
          else
            {
              ref->pretty_name = newts_nmalloc (strlen (ref->owner) +
                                                strlen (ref->system) +
                                                strlen (ref->name) + 3,
                                                sizeof (char));
              sprintf (ref->pretty_name, N_("=%s/%s:%s"),
                       ref->system, ref->owner, ref->name);
            }
        }
      else
        {
          if (ref->owner == NULL)
            {
              ref->pretty_name = newts_nmalloc (strlen (ref->name) +
                                                strlen (ref->system) + 8,
                                                sizeof (char));
              sprintf (ref->pretty_name, N_("=%s:%d/%s"),
                       ref->system, ref->port, ref->name);
            }
          else
            {
              ref->pretty_name = newts_nmalloc (strlen (ref->owner) +
                                                strlen (ref->system) +
                                                strlen (ref->name) + 9,
                                                sizeof (char));
              sprintf (ref->pretty_name, N_("=%s:%d/%s:%s"),
                       ref->system, ref->port, ref->owner, ref->name);
            }
        }
    }

  return ref->pretty_name;
}

enum newts_protocols
nfref_protocol (const newts_nfref *ref)
{
  return ref->protocol;
}

void
nfref_set_name (newts_nfref *ref, const char *new_name)
{
  if (ref->name)
    newts_free (ref->name);

  if (new_name)
    ref->name = newts_strdup (new_name);
  else
    ref->name = NULL;
}

void
nfref_set_owner (newts_nfref *ref, const char *new_owner)
{
  if (ref->owner)
    newts_free (ref->owner);

  if (new_owner)
    ref->owner = newts_strdup (new_owner);
  else
    ref->owner = NULL;
}

void
nfref_set_port (newts_nfref *ref, const unsigned short new_port)
{
  ref->port = new_port;
}

void
nfref_set_protocol (newts_nfref *ref, const enum newts_protocols new_protocol)
{
  ref->protocol = new_protocol;
}

void
nfref_set_system (newts_nfref *ref, const char *new_system)
{
  if (ref->system)
    newts_free (ref->system);

  if (new_system)
    ref->system = newts_strdup (new_system);
  else
    ref->system = NULL;
}

void
nfref_set_user (newts_nfref *ref, const char *new_user)
{
  if (ref->user)
    newts_free (ref->user);

  if (new_user)
    ref->user = newts_strdup (new_user);
  else
    ref->user = NULL;
}

char *
nfref_system (const newts_nfref *ref)
{
  return ref->system;
}

int
nfref_system_is_localhost (const newts_nfref *ref)
{
  if (!ref->system
      || strcmp (ref->system, "localhost") == 0
      || strcmp (ref->system, "") == 0
      || strcmp (ref->system, newts_get_fqdn ()) == 0)
    return TRUE;
  else
    return FALSE;
}

char *
nfref_user (const newts_nfref *ref)
{
  return ref->user;
}
