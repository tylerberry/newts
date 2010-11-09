/*
 * notesfile.c - methods for handling the notesfile data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2006, 2007 Tyler Berry.
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
#include "structs.h"

#include "newts/memory.h"
#include "newts/notesfile.h"

struct notesfile *
nf_alloc (void)
{
  return (struct notesfile *) newts_zalloc (sizeof (struct notesfile));
}

void
nf_free (struct notesfile *nf)
{
  if (nf->ref)
    nfref_free (nf->ref);
  if (nf->title)
    newts_free (nf->title);
  if (nf->director_message)
    newts_free (nf->director_message);
  if (nf->opts)
    newts_free (nf->opts);
  newts_free (nf);
}

char *
nf_director_message (const struct notesfile *nf)
{
  return nf->director_message;
}

newts_nfref *
nf_nfref (const struct notesfile *nf)
{
  return nf->ref;
}

char *
nf_title (const struct notesfile *nf)
{
  return nf->title;
}

unsigned
nf_total_notes (const struct notesfile *nf)
{
  return nf->total_notes;
}

void
nf_set_director_message (struct notesfile *nf,
                         const char *new_director_message)
{
  if (nf->director_message)
    newts_free (nf->director_message);

  if (new_director_message)
    nf->director_message = newts_strdup (new_director_message);
  else
    nf->director_message = NULL;
}

void
nf_set_nfref (struct notesfile *nf, const newts_nfref *new_ref)
{
  if (nf->ref)
    nfref_free (nf->ref);

  if (new_ref)
    {
      nf->ref = nfref_alloc ();
      nfref_copy (nf->ref, new_ref);
    }
  else
    nf->ref = NULL;
}

void
nf_set_title (struct notesfile *nf, const char *new_title)
{
  if (nf->title)
    newts_free (nf->title);

  if (new_title)
    nf->title = newts_strdup (new_title);
  else
    nf->title = NULL;
}

void
nf_set_total_notes (struct notesfile *nf, const unsigned new_total_notes)
{
  nf->total_notes = new_total_notes;
}
