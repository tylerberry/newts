/*
 * delete_note.c - delete a particular note or response
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

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

/* delete_note - delete note or response NRP. */

int
uiuc_delete_note (struct newtref *nrp)
{
  struct io_f io;
  struct note_f note;
  struct newt newt;
  struct flock nlock, dlock;

  memset (&newt, 0, sizeof (struct newt));
  newt.nr.nfr.owner = nrp->nfr.owner;
  newt.nr.nfr.name = nrp->nfr.name;
  newt.nr.notenum = nrp->notenum;
  newt.nr.respnum = nrp->respnum;

  uiuc_get_note (&newt, FALSE);

  if (newt.options & NOTE_CORRUPTED)
    return -1;

  init (&io, &nrp->nfr);

  if (io.descr.d_stat & NFINVALID)
    {
      closenf (&io);
      return -1;
    }

  if (nrp->notenum < -1 || nrp->notenum > io.descr.d_nnote)
    {
      closenf (&io);
      return -1;
    }

  nlock.l_whence = SEEK_SET;
  nlock.l_start = (off_t) (sizeof (struct descr_f) +
                           (nrp->notenum * sizeof (struct note_f)));
  nlock.l_len = (off_t) sizeof (struct note_f);

  if (nrp->respnum == 0)
    {
      nlock.l_type = F_RDLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

      getnoterec (&io, nrp->notenum, &note);
      if ((note.n_stat & ISDELETED) == 0)
        {
          note.n_stat |= ISDELETED;
          time (&newt.modified);
          get_uiuc_time (&note.n_lmod, newt.modified);

          dlock.l_type = F_WRLCK;
          dlock.l_whence = SEEK_SET;
          dlock.l_start = 0;
          dlock.l_len = (off_t) sizeof (struct daddr_f);
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

          nlock.l_type = F_WRLCK;
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

          putnoterec (&io, nrp->notenum, &note);

          nlock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &nlock);

          getdescr (&io, &io.descr);
          get_uiuc_time (&io.descr.d_lastm, newt.modified);
          io.descr.d_delnote++;
          io.descr.d_delresp += note.n_nresp;
          putdescr (&io, &io.descr);

          fdatasync (io.fidndx);
          dlock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &dlock);
        }
      else
        {
          nlock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &nlock);
        }

      closenf (&io);
      return 0;
    }
  else
    {
      struct resp_f resp;
      int i, offset, record;

      if (logical_resp (&io, nrp->notenum, nrp->respnum, &resp,
                        &offset, &record) == -1)
        {
          closenf (&io);
          return -1;
        }

      if ((resp.r_stat[offset] & ISDELETED) == 0)
        {
          struct flock rlock;

          dlock.l_type = F_WRLCK;
          dlock.l_whence = SEEK_SET;
          dlock.l_start = 0;
          dlock.l_len = (off_t) sizeof (struct daddr_f);
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

          rlock.l_type = F_WRLCK;
          rlock.l_whence = SEEK_SET;
          rlock.l_start = 0;
          rlock.l_len = 0;
          TEMP_FAILURE_RETRY (fcntl (io.fidrdx, F_SETLKW, &rlock));

          resp.r_stat[offset] |= ISDELETED;
          resp.r_last--;
          putresprec (&io, record, &resp);

          while ((i = resp.r_next) >= 0)
            {
              getresprec (&io, i, &resp);
              resp.r_first--;
              resp.r_last--;
              putresprec (&io, i, &resp);
            }

          fdatasync (io.fidrdx);
          rlock.l_type = F_UNLCK;
          fcntl (io.fidrdx, F_SETLK, &rlock);

          nlock.l_type = F_WRLCK;
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

          getnoterec (&io, nrp->notenum, &note);
          time (&newt.modified);
          get_uiuc_time (&note.n_lmod, newt.modified);
          note.n_nresp--;
          putnoterec (&io, nrp->notenum, &note);

          nlock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &nlock);

          getdescr (&io, &io.descr);
          get_uiuc_time (&io.descr.d_lastm, newt.modified);
          io.descr.d_delresp++;
          putdescr (&io, &io.descr);

          fdatasync (io.fidndx);
          dlock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &dlock);
        }

      closenf (&io);
      return 0;
    }
}
