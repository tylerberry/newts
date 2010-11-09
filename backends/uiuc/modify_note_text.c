/*
 * modify_note_text.c - update the text of a note.
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

#if HAVE_PWD_H
# include <pwd.h>
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

/* uiuc_modify_note_text - update the text portions of a note. */

int
uiuc_modify_note_text (struct newt *newt)
{
  static uid_t anon;
  static short anon_is_set = FALSE;

  struct io_f io;
  struct daddr_f daddr;
  struct note_f note;
  struct stat statbuf;
  struct flock dlock, nlock;
  time_t timet;

  if (!anon_is_set)
    {
      struct passwd *pw = getpwnam (ANON);

      anon = pw->pw_uid;
      anon_is_set = TRUE;
      endpwent ();
    }

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

      /* We're allowed to change the director message here; saves time. */

      if (newt->director_message)
        note.n_stat |= DIRMES;
      else
        note.n_stat &= ~DIRMES;

      if (!allow (&io, DRCTOK))
        {
          if (io.descr.d_stat & ISMODERATED)
            note.n_stat |= ISUNAPPROVED;
          else
            note.n_stat &= ~ISUNAPPROVED;
        }
      else
        note.n_stat &= ~ISUNAPPROVED;

      /* Update the note's text record */

      puttextrec (&io, newt->text, &daddr, -1);
      note.n_addr.addr = daddr.addr;
      note.n_addr.textlen = daddr.textlen;

      /* We could be dealing with a newly anonymous note.  Stupid, huh? */

      if (newt->options & NOTE_ANONYMOUS)
        {
          strncpy (note.n_auth.aname, "anonymous", NAMESZ);
          note.n_auth.aid = (int) anon;
        }

      /* Set the descriptor lock; we'll be updating modification time. */

      dlock.l_type = F_RDLCK;
      dlock.l_whence = SEEK_SET;
      dlock.l_start = 0;
      dlock.l_len = (off_t) sizeof (struct descr_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      getdescr (&io, &io.descr);

      get_uiuc_time (&io.descr.d_lastm, newt->modified);
      get_uiuc_time (&note.n_lmod, newt->modified);
      io.descr.d_delnote++;

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

      /* We're allowed to change the director message here; it'll save time. */

      if (newt->director_message)
        resp.r_stat[offset] |= DIRMES;
      else
        resp.r_stat[offset] &= ~DIRMES;

      if (!allow (&io, DRCTOK))
        {
          if (io.descr.d_stat & ISMODERATED)
            note.n_stat |= ISUNAPPROVED;
          else
            note.n_stat &= ~ISUNAPPROVED;
        }
      else
        note.n_stat &= ~ISUNAPPROVED;
      /* Update the response's text record */

      puttextrec (&io, newt->text, &daddr, -1);
      resp.r_addr[offset].addr = daddr.addr;
      resp.r_addr[offset].textlen = daddr.textlen;

      /* We could be dealing with a newly anonymous note.  Stupid, huh? */

      if (newt->options & NOTE_ANONYMOUS)
        {
          strncpy (resp.r_auth[offset].aname, "anonymous", NAMESZ);
          note.n_auth.aid = (int) anon;
        }

      /* Set up the descriptor lock (we update the "updated time") */

      dlock.l_type = F_RDLCK;
      dlock.l_whence = SEEK_SET;
      dlock.l_start = 0;
      dlock.l_len = (off_t) sizeof (struct descr_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      getdescr (&io, &io.descr);

      get_uiuc_time (&io.descr.d_lastm, newt->modified);
      get_uiuc_time (&note.n_lmod, newt->modified);
      io.descr.d_delresp++;

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
