/*
 * get_note.c - read a UIUC-format note (or response)
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

int
uiuc_get_note (struct newt *notep, short updatestats)
{
  struct io_f io;
  struct daddr_f daddr;
  struct flock lock;
  int result;

  if (notep == NULL)
    return -1;

  result = load_note (notep, &daddr, updatestats);

  if (result)
    return result;

  notep->text = newts_nrealloc (notep->text, daddr.textlen + 1, sizeof (char));

  init (&io, &notep->nr.nfr);

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = (off_t) daddr.addr;
  lock.l_len = (off_t) daddr.textlen;
  TEMP_FAILURE_RETRY (fcntl (io.fidtxt, F_SETLKW, &lock));

  lseek (io.fidtxt, (off_t) daddr.addr, SEEK_SET);
  TEMP_FAILURE_RETRY (read (io.fidtxt, notep->text, daddr.textlen));
  notep->text[daddr.textlen] = '\0';

  lock.l_type = F_UNLCK;
  fcntl (io.fidtxt, F_SETLK, &lock);

  closenf (&io);

  return 0;
}

int
load_note (struct newt *newtp, struct daddr_f *daddr, short updatestats)
{
  struct io_f io;
  struct note_f note;
  struct flock lock;
  struct stat statbuf;

  if (newtp == NULL)
    return -1;

  init (&io, &newtp->nr.nfr);

  if (io.descr.d_stat & NFINVALID)
    {
      closenf (&io);
      return -1;
    }

  if (!allow (&io, READOK) && newtp->nr.notenum != 0)
    {
      closenf (&io);
      return -2;
    }

  if (newtp->nr.notenum > io.descr.d_nnote || newtp->nr.notenum < 0
      || (newtp->nr.notenum == 0 && !io.descr.d_plcy))
    {
      closenf (&io);
      return -1;
    }

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = (off_t) (sizeof (struct descr_f) +
                          (newtp->nr.notenum * sizeof (struct note_f)));
  lock.l_len = (off_t) sizeof (struct note_f);
  TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

  getnoterec (&io, newtp->nr.notenum, &note);

  lock.l_type = F_UNLCK;
  fcntl (io.fidndx, F_SETLK, &lock);

  if (newtp->nr.respnum)
    {
      struct resp_f resp;
      int offset, record;
      time_t timet;

      if (logical_resp (&io, newtp->nr.notenum, newtp->nr.respnum, &resp,
                        &offset, &record) == -1)
        return -1;

      fstat (io.fidtxt, &statbuf);

      time (&timet);

      if (resp.r_addr[offset].textlen > HARDMAX ||
          convert_time (&resp.r_when[offset]) > timet ||
          convert_time (&resp.r_rcvd[offset]) > timet ||
          (off_t) (resp.r_addr[offset].textlen + resp.r_addr[offset].addr)
          > statbuf.st_size)
        {
          const char *error_text =
            "The data of this response has been corrupted in a way that "
            "makes it unsafe to\nuse. To avoid memory faults or other "
            "errors, it has not been loaded, and this\nnote has been "
            "marked as deleted.";

          newtp->title = newts_nrealloc (newtp->title,
                                         sizeof (note.ntitle) + 1,
                                         sizeof (char));
          strncpy (newtp->title, note.ntitle, sizeof (note.ntitle) + 1);
          newtp->title[sizeof (note.ntitle)] = '\0';

          if (newtp->director_message)
            newts_free (newtp->director_message);
          newtp->director_message = NULL;

          newtp->text = newts_nrealloc (newtp->text, strlen (error_text) + 1,
                                        sizeof (char));
          strncpy (newtp->text, error_text, strlen (error_text) + 1);

          newtp->auth.system = newts_nrealloc (newtp->auth.system,
                                               strlen (newts_get_fqdn ()) + 1,
                                               sizeof (char));
          strcpy (newtp->auth.system, newts_get_fqdn ());

          newtp->auth.name = newts_nrealloc (newtp->auth.name,
                                             strlen (NOTES) + 1,
                                             sizeof (char));
          strcpy (newtp->auth.name, NOTES);

          newtp->options = 0;
          newtp->options |= NOTE_DELETED + NOTE_CORRUPTED;
          newtp->total_resps = 0;

          closenf (&io);
          return -3;
        }

      newtp->title = newts_nrealloc (newtp->title, sizeof (note.ntitle) + 1,
                                     sizeof (char));
      strncpy (newtp->title, note.ntitle, sizeof (note.ntitle) + 1);
      newtp->title[sizeof (note.ntitle)] = '\0';

      if (resp.r_stat[offset] & DIRMES)
        {
          newtp->director_message = newts_nrealloc (newtp->director_message,
                                                    sizeof (io.descr.d_drmes) + 1,
                                                    sizeof (char));
          strncpy (newtp->director_message, io.descr.d_drmes,
                   sizeof (io.descr.d_drmes) + 1);
          newtp->director_message[sizeof (io.descr.d_drmes)] = '\0';
        }
      else
        {
          if (newtp->director_message)
            newts_free (newtp->director_message);
          newtp->director_message = NULL;
        }

      newtp->auth.system = newts_nrealloc (newtp->auth.system,
                                           sizeof (resp.r_auth[offset].asystem) + 1,
                                      sizeof (char));
      strncpy (newtp->auth.system, resp.r_auth[offset].asystem,
               sizeof (resp.r_auth[offset].asystem) + 1);

      newtp->auth.name = newts_nrealloc (newtp->auth.name,
                                         sizeof (resp.r_auth[offset].aname) + 1,
                                         sizeof (char));
      strncpy (newtp->auth.name, resp.r_auth[offset].aname,
               sizeof (resp.r_auth[offset].aname) + 1);

      newtp->auth.uid = (uid_t) resp.r_auth[offset].aid;

      newtp->created = convert_time (&resp.r_when[offset]);
      newtp->modified = convert_time (&resp.r_when[offset]);  /* Boo hiss. */

      newtp->id.system = newts_nrealloc (newtp->id.system,
                                         sizeof (resp.r_id[offset].sys) + 1,
                                         sizeof (char));
      strncpy (newtp->id.system, resp.r_id[offset].sys,
               sizeof (resp.r_id[offset].sys) + 1);

      newtp->id.number = resp.r_id[offset].uniqid;

      newtp->total_resps = note.n_nresp;

      if (daddr != NULL)
        {
          daddr->addr = resp.r_addr[offset].addr;
          daddr->textlen = resp.r_addr[offset].textlen;
        }

      newtp->options = 0;
      if (resp.r_stat[offset] & ISDELETED)
        newtp->options |= NOTE_DELETED;
      if (resp.r_stat[offset] & ISUNAPPROVED)
        {
          newtp->options |= NOTE_UNAPPROVED;

          if (!allow (&io, DRCTOK))
            {
              const char *mod_text =
                _("This response has not yet been approved by the notesfile "
                  "directors.");

              newtp->text = newts_nrealloc (newtp->text, strlen (mod_text) + 1,
                                            sizeof (char));
              strncpy (newtp->text, mod_text, strlen (mod_text) + 1);

              closenf (&io);
              return -3;
            }
        }

      if (updatestats)
        {
          struct flock ulock;

          ulock.l_type = F_WRLCK;
          ulock.l_whence = SEEK_SET;
          ulock.l_start = 0;
          ulock.l_len = (off_t) sizeof (struct descr_f);
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &ulock));

          getdescr (&io, &io.descr);
          io.descr.d_rspread++;
          putdescr (&io, &io.descr);

          fdatasync (io.fidndx);
          ulock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &ulock);
        }
    }
  else
    {
      time_t timet;

      /* Test for pathological notes and put in a placeholder note if we find
       * one.
       */

      fstat (io.fidtxt, &statbuf);

      time (&timet);

      if (note.n_addr.textlen > HARDMAX ||
          note.n_nresp < 0 || convert_time (&note.n_lmod) > timet ||
          convert_time (&note.n_date) > timet ||
          (off_t) (note.n_addr.textlen + note.n_addr.addr) > statbuf.st_size)
        {
          const char *error_text =
            "The data of this note has been corrupted in a way that "
            "makes it unsafe to\nuse. To avoid memory faults or other "
            "errors, it has not been loaded.";

          newtp->title = newts_nrealloc (newtp->title, 21, sizeof (char));
          strcpy (newtp->title, "** Corrupted Note **");

          if (newtp->director_message)
            newts_free (newtp->director_message);
          newtp->director_message = NULL;

          newtp->text = newts_nrealloc (newtp->text, strlen (error_text) + 1,
                                        sizeof (char));
          strncpy (newtp->text, error_text, strlen (error_text) + 1);

          newtp->auth.system = newts_nrealloc (newtp->auth.system,
                                               strlen (newts_get_fqdn ()) + 1,
                                               sizeof (char));
          strcpy (newtp->auth.system, newts_get_fqdn ());

          newtp->auth.name = newts_nrealloc (newtp->auth.name,
                                             strlen (NOTES) + 1,
                                             sizeof (char));
          strcpy (newtp->auth.name, NOTES);

          newtp->options = 0;
          newtp->options |= NOTE_DELETED + NOTE_CORRUPTED;
          newtp->total_resps = 0;

          closenf (&io);
          return -3;
        }

      newtp->title = newts_nrealloc (newtp->title, sizeof (note.ntitle) + 1,
                                     sizeof (char));
      strncpy (newtp->title, note.ntitle, sizeof (note.ntitle) + 1);

      if (note.n_stat & DIRMES)
        {
          newtp->director_message = newts_nrealloc (newtp->director_message,
                                                    sizeof (io.descr.d_drmes) + 1,
                                                    sizeof (char));
          strncpy (newtp->director_message, io.descr.d_drmes,
                   sizeof (io.descr.d_drmes) + 1);
        }
      else
        {
          if (newtp->director_message)
            newts_free (newtp->director_message);
          newtp->director_message = NULL;
        }

      newtp->auth.system = newts_nrealloc (newtp->auth.system,
                                      sizeof (note.n_auth.asystem) + 1,
                                           sizeof (char));
      strncpy (newtp->auth.system, note.n_auth.asystem,
               sizeof (note.n_auth.asystem) + 1);

      newtp->auth.name = newts_nrealloc (newtp->auth.name,
                                         sizeof (note.n_auth.aname) + 1,
                                         sizeof (char));
      strncpy (newtp->auth.name, note.n_auth.aname,
               sizeof (note.n_auth.aname) + 1);

      newtp->auth.uid = (uid_t) note.n_auth.aid;

      if (daddr != NULL)
        {
          daddr->addr = note.n_addr.addr;
          daddr->textlen = note.n_addr.textlen;
        }

      newtp->total_resps = note.n_nresp;
      newtp->created = convert_time (&note.n_date);
      newtp->modified = convert_time (&note.n_lmod);

      newtp->id.system = newts_nrealloc (newtp->id.system,
                                         sizeof (note.n_id.sys) + 1,
                                         sizeof (char));
      strncpy (newtp->id.system, note.n_id.sys,
               sizeof (note.n_id.sys) + 1);

      newtp->id.number = note.n_id.uniqid;

      newtp->options = 0;
      if (note.n_stat & ISDELETED)
        newtp->options |= NOTE_DELETED;
      if (note.n_stat & WRITONLY)
        newtp->options |= NOTE_WRITE_ONLY;
      if (note.n_stat & ISUNAPPROVED)
        {
          newtp->options |= NOTE_UNAPPROVED;

          if (!allow (&io, DRCTOK))
            {
              const char *mod_text =
                "This note has not yet been approved by the notesfile "
                "directors.";

              newtp->text = newts_nrealloc (newtp->text, strlen (mod_text) + 1,
                                            sizeof (char));
              strncpy (newtp->text, mod_text, strlen (mod_text) + 1);

              closenf (&io);
              return -3;
            }
        }

      if (updatestats)
        {
          struct flock ulock;

          ulock.l_type = F_WRLCK;
          ulock.l_whence = SEEK_SET;
          ulock.l_start = 0;
          ulock.l_len = (off_t) sizeof (struct descr_f);
          TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &ulock));

          getdescr (&io, &io.descr);
          io.descr.d_notread++;
          putdescr (&io, &io.descr);

          fdatasync (io.fidndx);
          ulock.l_type = F_UNLCK;
          fcntl (io.fidndx, F_SETLK, &ulock);
        }
    }

  closenf (&io);
  return 0;
}
