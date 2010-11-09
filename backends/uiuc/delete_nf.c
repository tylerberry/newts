/*
 * delete_nf.c - delete a UIUC notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based on rmnf.c from the UIUC notes distribution by Ray Essick and Rob
 * Kolstad.  Any work derived from this source code is required to retain this
 * notice.
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

#include "uiuc-backend.h"

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

/* delete_nf - delete an already existing UIUC notesfile.
 *
 * Returns: 0 on success, -1 if an error occurs.
 */

int
uiuc_delete_nf (const newts_nfref *refp)
{
  char *filename;
  size_t length;
  /* If we determine which of the files which comprise the notesfile has the
   * longest filename, we only need to use malloc once.  We add 3 to the sum for
   * the NUL character and the two slashes we interpose into the filename.
   */

  {
    size_t long_filename = strlen (NOTEINDX);
    if (strlen (RESPINDX) > long_filename) long_filename = strlen (RESPINDX);
    if (strlen (TEXT) > long_filename) long_filename = strlen (TEXT);
    if (strlen (ACCESS) > long_filename) long_filename = strlen (ACCESS);
    length = strlen (SPOOL) + (refp->owner != NULL ? strlen (refp->owner) + 1 : 0)
      + strlen (refp->name) + long_filename + 3;

    filename = newts_nmalloc (sizeof (char), length);
  }

  /* We stat the nf directory to make sure it actually exists. */

  {
    struct stat nfstat;

    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s", SPOOL, refp->name);
    else
      snprintf (filename, length, "%s/%s:%s", SPOOL, refp->owner, refp->name);
    if (stat (filename, &nfstat))
      {
        newts_free (filename);
        return -1;
      }
  }

  /* FIXME: Now this isn't correct here.  What we should do is just delete the
   * whole damn directory all at once.  Right now, this could fail, and this
   * routine should never fail as long as the notesfile exists.
   */

  /* Delete the access file... */

  {
    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, refp->name, ACCESS);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, refp->owner,
                refp->name, ACCESS);
    unlink (filename);
  }

  /* ...the note.indx file... */

  {
    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, refp->name, NOTEINDX);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, refp->owner,
                refp->name, NOTEINDX);
    unlink (filename);
  }

  /* ...the resp.indx file... */

  {
    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, refp->name, RESPINDX);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, refp->owner,
                refp->name, RESPINDX);
    unlink (filename);
  }

  /* ...the text file... */

  {
    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, refp->name, TEXT);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, refp->owner,
                refp->name, TEXT);
    unlink (filename);
  }

  /* ...and the notesfile directory. */

  {
    if (refp->owner == NULL)
      snprintf (filename, length, "%s/%s", SPOOL, refp->name);
    else
      snprintf (filename, length, "%s/%s:%s", SPOOL, refp->owner, refp->name);
    rmdir (filename);
  }

  newts_free (filename);

  return 0;
}
