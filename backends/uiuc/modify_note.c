/*
 * modify_note.c - update aspects of a note unrelated to the text
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

/* uiuc_modify_note - update the non-text portions of a note. */

int
uiuc_modify_note (struct newt *newt, int flags)
{
  struct io_f io;
  struct note_f note, oldnote;
  struct stat statbuf;
  struct flock dlock, nlock;
  time_t timet;

  init (&io, &newt->nr.nfr);

  if (io.descr.d_stat & NFINVALID)
    {
      closenf (&io);
      return -1;
    }

  if (newt->nr.notenum < -1 || newt->nr.notenum > io.descr.d_nnote)
    {
      closenf (&io);
      return -1;
    }

  nlock.l_type = F_RDLCK;
  nlock.l_whence = SEEK_SET;
  nlock.l_start = (off_t) (sizeof (struct descr_f) +
                           (newt->nr.notenum * sizeof (struct note_f)));
  nlock.l_len = (off_t) sizeof (struct note_f);

  if (newt->nr.respnum == 0)
    {
      int savestat;

      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

      getnoterec (&io, newt->nr.notenum, &oldnote);

      fstat (io.fidtxt, &statbuf);

      time (&timet);
      if (oldnote.n_addr.textlen > HARDMAX ||
          oldnote.n_nresp < 0 || convert_time (&oldnote.n_lmod) > timet ||
          convert_time (&oldnote.n_date) > timet ||
          (off_t) (oldnote.n_addr.textlen + oldnote.n_addr.addr) >
          statbuf.st_size)
        {
          closenf (&io);
          return -1;
        }

      memcpy (&note, &oldnote, sizeof (struct note_f));

      strncpy (note.ntitle, newt->title, TITLEN);
      note.ntitle[TITLEN - 1] = '\0';

      savestat = (int) note.n_stat;
      note.n_stat = 0;
      if (newt->director_message) note.n_stat |= DIRMES;

      dlock.l_type = F_RDLCK;
      dlock.l_whence = SEEK_SET;
      dlock.l_start = 0;
      dlock.l_len = (off_t) sizeof (struct descr_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      getdescr (&io, &io.descr);

      if (newt->options & NOTE_DELETED)
        {
          if (!(savestat & NOTE_DELETED))
            {
              io.descr.d_delnote++;
              io.descr.d_delresp += newt->total_resps;
            }
        note.n_stat |= ISDELETED;
        }
      else if (savestat & ISDELETED)
        {
          io.descr.d_delnote--;
          io.descr.d_delresp -= newt->total_resps;
        }
      if (oldnote.n_stat & WRITONLY)
        note.n_stat |= WRITONLY;

      if (newt->options & NOTE_UNAPPROVED)
        note.n_stat |= ISUNAPPROVED;

      if (flags & UPDATE_TIMES)
        {
          get_uiuc_time (&io.descr.d_lastm, newt->modified);
          get_uiuc_time (&note.n_lmod, newt->modified);
        }

      nlock.l_type = F_WRLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));
      dlock.l_type = F_WRLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      putnoterec (&io, newt->nr.notenum, &note);

      nlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &nlock);

      putdescr (&io, &io.descr);

      fdatasync (io.fidndx);
      dlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &dlock);

      closenf (&io);
      return 0;
    }
  else
    {
      struct resp_f resp, oldresp;
      struct flock rlock;
      int offset, record;

      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

      getnoterec (&io, newt->nr.notenum, &note);

      fstat (io.fidtxt, &statbuf);

      time (&timet);
      if (note.n_addr.textlen > HARDMAX ||
          note.n_nresp < 0 || convert_time (&note.n_lmod) > timet ||
          convert_time (&note.n_date) > timet ||
          (off_t) (note.n_addr.textlen + note.n_addr.addr) > statbuf.st_size)
        {
          closenf (&io);
          return -1;
        }

      if (logical_resp (&io, newt->nr.notenum, newt->nr.respnum, &oldresp,
                        &offset, &record) == -1)
        return -1;

      rlock.l_type = F_RDLCK;
      rlock.l_whence = SEEK_SET;
      rlock.l_start = (off_t) (sizeof (int) +
                               (record * sizeof (struct resp_f)));
      rlock.l_len = (off_t) sizeof (struct resp_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidrdx, F_SETLKW, &rlock));

      memcpy (&resp, &oldresp, sizeof (struct resp_f));

      resp.r_stat[offset] = 0;

      if (newt->director_message)
        resp.r_stat[offset] |= DIRMES;

      if (newt->options & NOTE_UNAPPROVED)
        resp.r_stat[offset] |= ISUNAPPROVED;

      dlock.l_type = F_RDLCK;
      dlock.l_whence = SEEK_SET;
      dlock.l_start = 0;
      dlock.l_len = (off_t) sizeof (struct descr_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      getdescr (&io, &io.descr);

      if (flags & UPDATE_TIMES)
        {
          get_uiuc_time (&io.descr.d_lastm, newt->modified);
          get_uiuc_time (&note.n_lmod, newt->modified);
        }

      rlock.l_type = F_WRLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidrdx, F_SETLKW, &rlock));
      nlock.l_type = F_WRLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));
      dlock.l_type = F_WRLCK;
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      putresprec (&io, record, &resp);

      putnoterec (&io, newt->nr.notenum, &note);

      nlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &nlock);

      putdescr (&io, &io.descr);

      fdatasync (io.fidrdx);
      fdatasync (io.fidndx);
      dlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &dlock);
      rlock.l_type = F_UNLCK;
      fcntl (io.fidrdx, F_SETLK, &rlock);

      closenf (&io);
      return 0;
    }
}
