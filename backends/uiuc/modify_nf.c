/*
 * modify_nf.c - modify an existing UIUC notesfile
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
uiuc_modify_nf (struct notesfile *nf)
{
  struct io_f io;
  struct flock lock;
  int error;
  struct uiuc_opts *opts;
  time_t timet;

  opts = (struct uiuc_opts *) nf->opts;

  error = init (&io, nf->ref);
  if (error != NEWTS_NO_ERROR)
    return error;

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = (off_t) sizeof (struct daddr_f);
  TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

  if (io.descr.d_stat & NFINVALID)
    {
      closenf (&io);
      return -1;
    }

  if (!allow (&io, DRCTOK))
    {
      closenf (&io);
      return -2;
    }

  strncpy (io.descr.d_title, nf->title, NNLEN);
  strncpy (io.descr.d_drmes, nf->director_message, DMLEN);

  if (nf->options & NF_ANONYMOUS)
    io.descr.d_stat |= ANONOK;
  else
    io.descr.d_stat &= ~ANONOK;

  if (!(nf->options & NF_LOCKED))
    io.descr.d_stat |= ISOPEN;
  else
    io.descr.d_stat &= ~ISOPEN;

  if (nf->options & NF_ARCHIVE)
    io.descr.d_stat |= ISARCHIVE;
  else
    io.descr.d_stat &= ~ISARCHIVE;

  if (nf->options & NF_MODERATED)
    io.descr.d_stat |= ISMODERATED;
  else
    io.descr.d_stat &= ~ISMODERATED;

  io.descr.d_archtime = opts->expire_threshold;
  io.descr.d_archkeep = opts->expire_action;
  io.descr.d_dmesgstat = opts->expire_by_dirmsg;
  io.descr.d_workset = opts->minimum_notes;
  io.descr.d_longnote = opts->maximum_note_size;

  time (&timet);
  get_uiuc_time (&io.descr.d_lastm, timet);

  lock.l_type = F_WRLCK;
  TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

  putdescr (&io, &io.descr);

  fdatasync (io.fidndx);
  lock.l_type = F_UNLCK;
  fcntl (io.fidndx, F_SETLK, &lock);

  closenf (&io);

  return 0;
}
