/*
 * logical_resp.c - get the physical record of a logical response number
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based on lrsp.c from the UIUC notes distribution by Ray Essick and Rob
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

#include "error.h"
#include "uiuc-backend.h"

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

int
logical_resp (struct io_f *iop, int notenum, int respnum, struct resp_f *resp,
              int *offset, int *record)
{
  struct note_f note;
  struct flock lock;

  if (respnum <= 0)               /* That was silly. */
    return -1;

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = (off_t) (sizeof (struct descr_f) +
                          (notenum * sizeof (struct note_f)));
  lock.l_len = (off_t) sizeof (struct note_f);
  TEMP_FAILURE_RETRY (fcntl (iop->fidndx, F_SETLKW, &lock));

  getnoterec (iop, notenum, &note);

  lock.l_type = F_UNLCK;
  fcntl (iop->fidndx, F_SETLK, &lock);

  if (respnum > note.n_nresp)     /* That was also silly. */
    return -1;

  *record = note.n_rindx;         /* Record number of first response block. */
  *offset = 0;

  /* We choose to incrementally lock the response tree as we go through it.
   * This admittedly slows down this single access, but with 1000 or so people,
   * it's a win.
   */

  lock.l_type = F_RDLCK;
  lock.l_start = (off_t) (sizeof (int) + (*record * sizeof (struct resp_f)));
  lock.l_len = (off_t) sizeof (struct resp_f);
  TEMP_FAILURE_RETRY (fcntl (iop->fidrdx, F_SETLKW, &lock));

  getresprec (iop, *record, resp);
  while (respnum > resp->r_last)
    {
      if (*record == resp->r_next)
        {
          /* We're stuck in an infinite loop. Dodge the bullet. */

          //if (debug)
          //  error (0, 0, "logical_resp: Stuck in infinite record loop (%d)",
          //         *record);
          lock.l_type = F_UNLCK;
          fcntl (iop->fidrdx, F_SETLK, &lock);
          return -1;
        }

      if ((*record = resp->r_next) == -1)   /* Broken chain of resps. */
        {
          lock.l_type = F_UNLCK;
          fcntl (iop->fidrdx, F_SETLK, &lock);
          return -1;
        }

      lock.l_type = F_UNLCK;
      fcntl (iop->fidrdx, F_SETLK, &lock);
      lock.l_type = F_RDLCK;
      lock.l_start = (off_t) (sizeof (int) +
                              (*record * sizeof (struct resp_f)));
      lock.l_len = (off_t) sizeof (struct resp_f);
      TEMP_FAILURE_RETRY (fcntl (iop->fidrdx, F_SETLKW, &lock));

      getresprec (iop, *record, resp);
    }

  lock.l_type = F_UNLCK;
  fcntl (iop->fidrdx, F_SETLK, &lock);

  /* At this point, we've found the block of responses which contains the
   * response we're looking for.
   */

  {
    register int count = -1;
    *offset = 0;

    while (1)
      {
        while (resp->r_stat[*offset] & ISDELETED)
          ++*offset;
        if (*offset >= RESPSZ)  /* r_last lied to us. */
          return -1;
        count++;
        if (resp->r_first + count == respnum)
          break;
        ++*offset;
      }
  }

  return 0;
}
