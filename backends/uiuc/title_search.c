/*
 * title_search.c - search for words in a note's title string
 *
 * This file is part of the Newts notesfiles system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based in part on tsearch.c from the UIUC notes distribution by Ray Essick
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

#include "uiuc-backend.h"

#include "strcase.h"

#if STDC_HEADERS
# include <ctype.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

int
uiuc_title_search (struct newtref *nrp, const char *string)
{
  struct io_f io;
  struct note_f note;
  struct flock lock;
  register int i, length;

  init (&io, &nrp->nfr);
  strncpy (io.xstring, string, TITLEN + 1);

  length = strlen (io.xstring) > TITLEN ? TITLEN : strlen (io.xstring);

  if (nrp->notenum > io.descr.d_nnote)
    nrp->notenum = io.descr.d_nnote;

  lock.l_whence = SEEK_SET;
  lock.l_len = (off_t) sizeof (struct note_f);

  while (nrp->notenum > 0)
    {
      lock.l_type = F_RDLCK;
      lock.l_start = (off_t) (sizeof (struct descr_f) +
                              (nrp->notenum * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

      getnoterec (&io, nrp->notenum, &note);

      lock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &lock);

      for (i=0; i < TITLEN + 1 - length; i++)
        {
          if (strncasecmp (io.xstring, note.ntitle + i, (size_t) length) == 0)
            {
              closenf (&io);
              return nrp->notenum;
            }
        }
      nrp->notenum--;
    }

  closenf (&io);
  return -1;
}
