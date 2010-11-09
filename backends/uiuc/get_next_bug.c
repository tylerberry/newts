/*
 * get_next_bug.c - get the next bug number for a given notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
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
#  include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

/* uiuc_get_next_bug - get and increment the bug counter for REF.
 *
 * Returns: negative on error, or the bug number.
 */

int
uiuc_get_next_bug (const struct notesfile *nf)
{
  newts_nfref *ref = nf->ref;
  struct io_f io;
  int bugfile;
  char *bugpath;
  int bugnum;
  int error;

  error = init (&io, ref);
  if (error != NEWTS_NO_ERROR)
    return error;

  bugpath = newts_nmalloc (sizeof (char), strlen (io.fullname) +
                      strlen (BUGCOUNT) + 2);
  sprintf (bugpath, "%s/%s", io.fullname, BUGCOUNT);

  bugfile = TEMP_FAILURE_RETRY (open (bugpath, O_RDONLY));
  if (bugfile == -1)
    {
      bugfile = creat (bugpath, 0660);
      newts_free (bugpath);
      if (bugfile == -1)
        return -1;
      bugnum = 1;
    }
  else
    {
      newts_free (bugpath);
      TEMP_FAILURE_RETRY (read (bugfile, &bugnum, sizeof (int)));
    }

  bugnum++;
  TEMP_FAILURE_RETRY (write (bugfile, &bugnum, sizeof (int)));
  TEMP_FAILURE_RETRY (close (bugfile));
  bugnum--;

  closenf (&io);

  return bugnum;
}
