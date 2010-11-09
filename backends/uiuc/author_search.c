/*
 * author_search.c - search backwards for an article by a specific author
 *
 * This file is part of the Newts notesfiles system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based in part on asearch.c from the UIUC notes distribution by Ray Essick
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

#include "strpbrk.h"
#include "strstr.h"

#if STDC_HEADERS
# include <ctype.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* author_search will search half-backwards through a notesfile trying to find
 * a note with the specified author.
 *
 * By 'half-backwards' we mean that it will first search the last basenote,
 * then the first response to the last basenote, and on until the last response
 * to the last basenote, and then it will move to the second-to-last basenote,
 * and so on and so forth.
 *
 * Returns the note number if a match is found, or -1 if no match.
 */

int
uiuc_author_search (struct newtref *nrp, const char *author)
{
  struct passwd *pw;
  struct io_f io;
  struct note_f note;
  struct resp_f resp;
  struct flock lock;
  register int i;
  int offset, record;
  char buffer[NAMESZ + SYSSZ + 2];
  char *real = NULL;
  char *separator;

  if (author == NULL)
    return -1;

  init (&io, &nrp->nfr);

  strncpy (io.xauthor, author, NAMESZ + SYSSZ + 2);
  io.xauthor[NAMESZ + SYSSZ + 1] = '\0';

  if (nrp->notenum > io.descr.d_nnote)
    {
      nrp->respnum = 0;
      nrp->notenum = io.descr.d_nnote;
    }

  for (i=0; io.xauthor[i]; i++)
    if (isupper (io.xauthor[i]))
      io.xauthor[i] = tolower (io.xauthor[i]);

  lock.l_whence = SEEK_SET;
  lock.l_len = (off_t) sizeof (struct note_f);

  if (nrp->respnum != 0)
    {
      lock.l_type = F_RDLCK;
      lock.l_start = (off_t) (sizeof (struct descr_f) +
                              (nrp->notenum * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

      getnoterec (&io, nrp->notenum, &note);

      fdatasync (io.fidndx);
      lock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &lock);

      goto inloop;
    }

  /* We're currently not searching the policy note here. */

  while (nrp->notenum > 0)
    {
      lock.l_type = F_RDLCK;
      lock.l_start = (off_t) (sizeof (struct descr_f) +
                              (nrp->notenum * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

      getnoterec (&io, nrp->notenum, &note);

      fdatasync (io.fidndx);
      lock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &lock);

      if (note.n_stat & ISDELETED)
        {
          nrp->notenum--;
          continue;
        }

      snprintf (buffer, NAMESZ + SYSSZ + 2, "%s@%s", note.n_auth.aname,
                note.n_id.sys);
      for (i = strlen (buffer) - 1; i>=0; i--)
        if (isupper (buffer[i]))
          buffer[i] = tolower (buffer[i]);

      if (real)
        newts_free (real);

      pw = getpwnam (note.n_auth.aname);
      real = newts_strdup (pw->pw_gecos);
      endpwent ();

      if ((separator = strpbrk (real, ":,")) != NULL)
        *separator = '\0';
      for (i = strlen (real) - 1; i>=0; i--)
        if (isupper (real[i]))
          real[i] = tolower (real[i]);

      if (strstr (buffer, io.xauthor) != NULL || strstr (real, io.xauthor) != NULL)
        {
          if (real)
            newts_free (real);
          closenf (&io);
          return nrp->notenum;
        }
      nrp->respnum = 1;

    inloop:

      for (; nrp->respnum <= note.n_nresp; nrp->respnum++)
        {
          if (logical_resp (&io, nrp->notenum, nrp->respnum, &resp, &offset,
                            &record)
              == -1)
            break;
          snprintf (buffer, NAMESZ + SYSSZ + 2, "%s@%s",
                    resp.r_auth[offset].aname, resp.r_id[offset].sys);

          for (i = strlen (buffer) - 1; i>=0; i--)
            if (isupper (buffer[i]))
              buffer[i] = tolower (buffer[i]);

          if (real)
            newts_free (real);

          pw = getpwnam (resp.r_auth[offset].aname);
          real = newts_strdup (pw->pw_gecos);
          endpwent ();

          if ((separator = strpbrk (real, ":,")) != NULL)
            *separator = '\0';
          for (i = strlen (real) - 1; i>=0; i--)
            if (isupper (real[i]))
              real[i] = tolower (real[i]);

          if (strstr (buffer, io.xauthor) != NULL ||
              strstr (real, io.xauthor) != NULL)
            {
              if (real)
                newts_free (real);
              closenf (&io);
              return nrp->notenum;
            }
        }

      nrp->respnum = 0;
      nrp->notenum--;
    }

  if (real)
    newts_free (real);

  closenf (&io);
  return -1;
}
