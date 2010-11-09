/*
 * open_nf.c - open and load a UIUC notesfile
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
uiuc_close_nf (struct notesfile *nf, short updatestats)
{
  struct io_f io;
  struct flock dlock;
  int error;
  time_t current_time;

  error = init (&io, nf->ref);
  if (error != NEWTS_NO_ERROR)
    return error;

  if (updatestats)
    {
      dlock.l_type = F_WRLCK;
      dlock.l_whence = SEEK_SET;
      dlock.l_start = 0;
      dlock.l_len = (off_t) sizeof (struct daddr_f);
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &dlock));

      getdescr (&io, &io.descr);

      time (&current_time);

      io.descr.entries++;
      io.descr.walltime += difftime (current_time, nf->time_entered);

      putdescr (&io, &io.descr);

      fdatasync (io.fidndx);
      dlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &dlock);
    }

  closenf (&io);
  return NEWTS_NO_ERROR;
}
