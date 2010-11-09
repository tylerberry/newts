/*
 * write_note.c - write a note to a specified UIUC notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on note.c and resp.c from the UIUC notes distribution by Ray
 * Essick and Rob Kolstad.  Any work derived from this source code is required
 * to retain this notice.
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

static int get_new_resp_block (struct io_f *iop);

/* write_note - the master entrypoint for adding notes and responses to the
 * database; for UIUC this is rather complicated because of the bizarre
 * flatfile database that we use.  Until we work itrees into this, the
 * following convention is operative:
 *
 * One of the arguments we pass is NEWT, a pointer to struct newt.  If
 * NEWT->NR.NOTENUM is -1, we take that to mean that we are adding a new
 * basenote.  On the other hand, if NEWT->NR.NOTENUM is between 0 and
 * NF->TOTAL_NOTES, we assume that we're adding a new response to note NOTENUM.
 *
 * Currently, we're flagged to disallow responding to note 0.
 *
 * Returns: -1 on failure, -2 on a lack of permission of some sort, or the new
 * note or response number as appropriate.
 */

int
uiuc_write_note (struct notesfile *nf, struct newt *newt, int flags)
{
  struct io_f io;
  struct daddr_f daddr;
  int result;
  int error;

  if (newt->nr.notenum < -1 || newt->nr.notenum > nf->total_notes)
    return -1;

  flags &= ~SKIP_MODERATION;  /* Not allowed via the public interface. */

  error = init (&io, nf->ref);
  if (error != NEWTS_NO_ERROR)
    return error;

  if (io.descr.d_stat & NFINVALID)
    {
      closenf (&io);
      return -1;
    }

  if (io.descr.d_stat & ISARCHIVE && !allow (&io, DRCTOK))
    {
      closenf (&io);
      return -2;
    }

  if (newt->nr.notenum == -1)
    {
      /* We have a new basenote here. */

      if (!allow (&io, WRITOK))
        {
          closenf (&io);
          return -2;
        }

      puttextrec (&io, newt->text, &daddr, -1);

      result = put_note (&io, &daddr, newt, flags);

      if (result < 0)
        {
          closenf (&io);
          return result;
        }

      if (!(flags & ADD_POLICY))
      nf->total_notes++;

      closenf (&io);
      return result;
    }
  else
    {
      /* Reject responses to the policy note ... for now. */

      if (newt->nr.notenum == 0)
        return -1;

      if (!allow (&io, RESPOK))
        {
          closenf (&io);
          return -2;
        }

      puttextrec (&io, newt->text, &daddr, -1);
      result = put_resp (&io, &daddr, newt, flags);

      closenf (&io);
      return result;
    }
}

int
put_note (struct io_f *io, struct daddr_f *where, struct newt *newt, int flags)
{
  static uid_t anon;
  static short anon_is_set = FALSE;

  struct note_f note;
  struct flock dlock, nlock;
  int notenum;

  if (!anon_is_set)
    {
      struct passwd *pw = getpwnam (ANON);

      anon = pw->pw_uid;
      anon_is_set = TRUE;
      endpwent ();
    }

  if (io == NULL || where == NULL || newt == NULL)
    return -1;

  /* Save the provided disk address information. */

  note.n_addr.addr = where->addr;
  note.n_addr.textlen = where->textlen;

  /* Save the author information. */

  strncpy (note.n_auth.aname, newt->auth.name, NAMESZ);
  strncpy (note.n_auth.asystem, newt->auth.system, HOMESYSSZ);
  note.n_auth.aid = (int) newt->auth.uid;
  strncpy (note.n_from, newt->auth.system, SYSSZ);
  note.n_from[SYSSZ - 1] = '\0';

  /* Save the title. */

  strncpy (note.ntitle, newt->title, TITLEN);
  note.ntitle[TITLEN - 1] = '\0';

  /* Initialize response storage. */

  note.n_nresp = 0;
  note.n_rindx = -1;

  /* Save note options - director message, write-only. */

  note.n_stat = 0;

  if (newt->director_message && allow (io, DRCTOK))
    note.n_stat |= DIRMES;

  if (newt->options & NOTE_WRITE_ONLY)
    note.n_stat |= WRITONLY;

  /* Adjust moderation flag appropriately. */

  if (flags & SKIP_MODERATION)
    {
      if (newt->options & NOTE_UNAPPROVED)
        note.n_stat |= ISUNAPPROVED;
    }
  else if (io->descr.d_stat & ISMODERATED && !allow (io, DRCTOK))
    note.n_stat |= ISUNAPPROVED;

  if (newt->options & NOTE_ANONYMOUS)
    {
      strncpy (note.n_auth.aname, "anonymous", NAMESZ);
      note.n_auth.aid = (int) anon;
    }

  /* Prepare to alter the descriptor. */

  dlock.l_type = F_RDLCK;
  dlock.l_whence = SEEK_SET;
  dlock.l_start = 0;
  dlock.l_len = (off_t) sizeof (struct descr_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &dlock));

  getdescr (io, &io->descr);

  /* If specified, generate a new ID for the note.  Otherwise, use the already
   * existing ID.
   */

  if (flags & ADD_ID)
    {
      strncpy (note.n_id.sys, newt->auth.system, SYSSZ);
      note.n_id.sys[SYSSZ - 1] = '\0';
      note.n_id.uniqid = ++io->descr.d_id.uniqid;
      note.n_id.uniqid += UNIQPLEX * io->descr.d_nfnum;
    }
  else
    {
      strncpy (note.n_id.sys, newt->id.system, SYSSZ);
      note.n_id.sys[SYSSZ - 1] = '\0';
      note.n_id.uniqid = newt->id.number;
    }

  /* Set up flags for a policy note or a regular note, depending. */

  if (flags & ADD_POLICY)
    {
      if (!allow (io, DRCTOK))
        {
          closenf (io);
          return -2;
        }
      io->descr.d_plcy = 1;
      notenum = 0;
    }
  else
    {
      notenum = ++io->descr.d_nnote;
    }

  /* Save the current time into the note. */

  if (flags & UPDATE_TIMES)
    get_uiuc_time (&io->descr.d_lastm, newt->created);
  get_uiuc_time (&note.n_date, newt->created);
  get_uiuc_time (&note.n_rcvd, newt->created);
  get_uiuc_time (&note.n_lmod, newt->modified);

  /* Save to disk. */

  nlock.l_type = F_WRLCK;
  nlock.l_whence = SEEK_SET;
  nlock.l_start = (off_t) (sizeof (struct descr_f) +
                           (notenum * sizeof (struct note_f)));
  nlock.l_len = (off_t) sizeof (struct note_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &nlock));
  dlock.l_type = F_WRLCK;
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &dlock));

  putnoterec (io, notenum, &note);

  nlock.l_type = F_UNLCK;
  fcntl (io->fidndx, F_SETLK, &nlock);

  io->descr.d_notwrit++;

  putdescr (io, &io->descr);

  fdatasync (io->fidndx);
  dlock.l_type = F_UNLCK;
  fcntl (io->fidndx, F_SETLK, &dlock);

  return notenum;
}

int
put_resp (struct io_f *io, struct daddr_f *where, struct newt *newt, int flags)
{
  struct note_f note;
  struct resp_f resp;
  struct flock dlock, nlock, rlock;
  int lastin, phys, i;

  rlock.l_whence = SEEK_SET;

  nlock.l_type = F_RDLCK;
  nlock.l_whence = SEEK_SET;
  nlock.l_start = (off_t) (sizeof (struct descr_f) +
                           (newt->nr.notenum * sizeof (struct note_f)));
  nlock.l_len = (off_t) sizeof (struct note_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &nlock));

  getnoterec (io, newt->nr.notenum, &note);

  if (note.n_rindx < 0)
    {
      lastin = note.n_rindx = get_new_resp_block (io);
      resp.r_first = 1;
      resp.r_last = 0;
      resp.r_previous = -1;
      resp.r_next = -1;
      for (i=0; i<RESPSZ; i++)
        resp.r_stat[i] = 0;
    }
  else
    {
      rlock.l_type = F_RDLCK;
      rlock.l_start = (off_t) (sizeof (int) +
                               (note.n_rindx * sizeof (struct resp_f)));
      rlock.l_len = (off_t) sizeof (struct resp_f);
      TEMP_FAILURE_RETRY (fcntl (io->fidrdx, F_SETLKW, &rlock));

      getresprec (io, lastin = note.n_rindx, &resp);

      rlock.l_type = F_UNLCK;
      fcntl (io->fidrdx, F_SETLK, &rlock);
    }

  i = phys = 0;

  while (i < note.n_nresp)
    {
      if (phys >= RESPSZ)
        {
          rlock.l_type = F_RDLCK;
          rlock.l_start = (off_t) (sizeof (int) +
                                   (resp.r_next * sizeof (struct resp_f)));
          rlock.l_len = (off_t) sizeof (struct resp_f);
          TEMP_FAILURE_RETRY (fcntl (io->fidrdx, F_SETLKW, &rlock));

          phys = 0;
          getresprec (io, lastin = resp.r_next, &resp);

          rlock.l_type = F_UNLCK;
          fcntl (io->fidrdx, F_SETLK, &rlock);
        }
      if ((resp.r_stat[phys] & NOTE_DELETED) == 0)
        i++;
      phys++;
    }

  if (phys >= RESPSZ)  /* We are in a new block of responses. */
    {
      phys = 0;
      resp.r_next = get_new_resp_block (io);

      rlock.l_type = F_WRLCK;
      rlock.l_start = (off_t) (sizeof (int) +
                               (lastin * sizeof (struct resp_f)));
      rlock.l_len = (off_t) sizeof (struct resp_f);
      TEMP_FAILURE_RETRY (fcntl (io->fidrdx, F_SETLKW, &rlock));

      putresprec (io, lastin, &resp);

      resp.r_previous = lastin;
      lastin = resp.r_next;
      resp.r_next = -1;
      resp.r_first = note.n_nresp + 1;
      resp.r_last = resp.r_first - 1;
      for (i=0; i<RESPSZ; i++)
        resp.r_stat[i] = 0;
    }

  fdatasync (io->fidrdx);
  rlock.l_type = F_RDLCK;
  rlock.l_start = (off_t) (sizeof (int) +
                           (lastin * sizeof (struct resp_f)));
  rlock.l_len = (off_t) sizeof (struct resp_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidrdx, F_SETLKW, &rlock));

  note.n_nresp++;

  resp.r_addr[phys].addr = where->addr;
  resp.r_addr[phys].textlen = where->textlen;

  strncpy (resp.r_auth[phys].aname, newt->auth.name, NAMESZ);
  strncpy (resp.r_auth[phys].asystem, newt->auth.system, HOMESYSSZ);
  resp.r_auth[phys].aid = (int) newt->auth.uid;
  strncpy (resp.r_from[phys], newt->auth.system, SYSSZ);

  resp.r_stat[phys] = 0;

  if (newt->director_message && allow (io, DRCTOK))
    resp.r_stat[phys] |= DIRMES;

  /* Adjust moderation flag appropriately. */

  if (flags & SKIP_MODERATION)
    {
      if (newt->options & NOTE_UNAPPROVED)
        resp.r_stat[phys] |= ISUNAPPROVED;
    }
  else if (io->descr.d_stat & ISMODERATED && !allow (io, DRCTOK))
    resp.r_stat[phys] |= ISUNAPPROVED;

  if (newt->options & NOTE_ANONYMOUS)
    strncpy (resp.r_auth[phys].aname, "anonymous", NAMESZ);

  /* Prepare to alter the descriptor. */

  dlock.l_type = F_RDLCK;
  dlock.l_whence = SEEK_SET;
  dlock.l_start = 0;
  dlock.l_len = (off_t) sizeof (struct descr_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &dlock));

  getdescr (io, &io->descr);

  if (flags & ADD_ID)
    {
      strncpy (resp.r_id[phys].sys, newt->auth.system, SYSSZ);
      resp.r_id[phys].uniqid = ++io->descr.d_id.uniqid;
      resp.r_id[phys].uniqid += UNIQPLEX * io->descr.d_nfnum;
    }
  else
    {
      strncpy (resp.r_id[phys].sys, newt->id.system, SYSSZ);
      resp.r_id[phys].sys[SYSSZ - 1] = '\0';
      resp.r_id[phys].uniqid = newt->id.number;
    }

  if (flags & UPDATE_TIMES)
    {
      get_uiuc_time (&io->descr.d_lastm, newt->created);
      get_uiuc_time (&note.n_lmod, newt->modified);
    }
  get_uiuc_time (&resp.r_when[phys], newt->created);
  get_uiuc_time (&resp.r_rcvd[phys], newt->created);

  resp.r_last++;

  rlock.l_type = F_WRLCK;
  TEMP_FAILURE_RETRY (fcntl (io->fidrdx, F_SETLKW, &rlock));
  nlock.l_type = F_WRLCK;
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &nlock));
  dlock.l_type = F_WRLCK;
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &dlock));

  putresprec (io, lastin, &resp);

  putnoterec (io, newt->nr.notenum, &note);
  io->descr.d_rspwrit++;

  nlock.l_type = F_UNLCK;
  fcntl (io->fidndx, F_SETLK, &nlock);

  putdescr (io, &io->descr);

  fdatasync (io->fidrdx);
  fdatasync (io->fidndx);
  dlock.l_type = F_UNLCK;
  fcntl (io->fidndx, F_SETLK, &dlock);
  rlock.l_type = F_UNLCK;
  fcntl (io->fidrdx, F_SETLK, &rlock);

  return note.n_nresp;
}

static int
get_new_resp_block (struct io_f *iop)
{
  struct flock lock;
  int i;

  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = (off_t) sizeof (int);
  TEMP_FAILURE_RETRY (fcntl (iop->fidrdx, F_SETLKW, &lock));

  lseek (iop->fidrdx, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (read (iop->fidrdx, &i, sizeof (int)));
  i++;
  lseek (iop->fidrdx, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (write (iop->fidrdx, &i, sizeof (int)));

  fdatasync (iop->fidrdx);
  lock.l_type = F_UNLCK;
  fcntl (iop->fidrdx, F_SETLK, &lock);

  return i - 1;
}
