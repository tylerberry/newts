/*
 * sequencer.c - functions to manage the UIUC sequencer
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2008 Tyler Berry
 *
 * Based on next.c and times.c from the UIUC notes distribution by Ray Essick
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

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

int
uiuc_get_seqtime (const newts_nfref *ref, const char *name, time_t *seq)
{
  char *filename;
  off_t size;
  struct stat statbuf;
  struct flock lock;
  int fid = -1, i;
  int numentries = 0;
  struct seq_f *entries = (struct seq_f *) NULL;

  if (ref == NULL || seq == NULL)
    return -1;

  *seq = 0;

  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;    /* All of it. */

  filename = newts_nmalloc (sizeof (char), strlen (SPOOL) +
                            strlen (SEQUENCER) + strlen (name) + 3);

  snprintf (filename, strlen (SPOOL) + strlen (SEQUENCER) + 2, "%s/%s", SPOOL,
            SEQUENCER);

  if (stat (filename, &statbuf))
    mkdir (filename, 0775);

  snprintf (filename, strlen (SPOOL) + strlen (SEQUENCER) + strlen (name) + 3,
            "%s/%s/%s", SPOOL, SEQUENCER, name);

  if (stat (filename, &statbuf))
    size = 0;
  else
    size = statbuf.st_size;

  if (size)
    {
      int stop = FALSE;

      numentries = 0;

      entries = (struct seq_f *) newts_malloc ((size_t) size);
      if (entries == NULL)
        {
          newts_free (filename);
          return -1;
        }

      if (!stop && (fid = TEMP_FAILURE_RETRY (open (filename, O_RDONLY))) < 0)
        {
          stop = TRUE;
        }

      lock.l_type = F_RDLCK;
      TEMP_FAILURE_RETRY (fcntl (fid, F_SETLKW, &lock));
      lock.l_type = F_UNLCK;

      if (!stop && TEMP_FAILURE_RETRY (read (fid, entries, (size_t) size)) != size)
        {
          fcntl (fid, F_SETLK, &lock);
          TEMP_FAILURE_RETRY (close (fid));
          stop = TRUE;
        }

      if (!stop)
        {
          numentries = size / sizeof (struct seq_f);
          fcntl (fid, F_SETLK, &lock);
          TEMP_FAILURE_RETRY (close (fid));
        }
    }

  if (numentries == 0)
    {
      int seqfile;
      int result;
      struct seq_f entry;

      if (entries)
        newts_free (entries);

      if ((seqfile = TEMP_FAILURE_RETRY (open (filename, O_RDONLY))) == -1)
        {
          newts_free (filename);
          return -1;
        }

      lock.l_type = F_RDLCK;
      TEMP_FAILURE_RETRY (fcntl (seqfile, F_SETLKW, &lock));

      do
        result = TEMP_FAILURE_RETRY (read (seqfile, &entry,
                                           sizeof (struct seq_f)));
      while (result && result == (int) sizeof (struct seq_f) &&
             strcmp (entry.nfname, ref->name) != 0);

      lock.l_type = F_UNLCK;
      fcntl (seqfile, F_SETLK, &lock);
      TEMP_FAILURE_RETRY (close (seqfile));

      if (strcmp (entry.nfname, ref->name) == 0)
        *seq = convert_time (&entry.lastin);
      else
        {
          newts_free (filename);
          return -1;
        }
    }
  else
    {
      for (i=0; i<numentries; i++)
        if (strcmp (entries[i].nfname, ref->name) == 0)
          *seq = convert_time (&entries[i].lastin);

      newts_free (entries);
    }

  newts_free (filename);
  return 0;
}

int
uiuc_set_seqtime (const newts_nfref *ref, const char *name, time_t seq)
{
  struct seq_f entry;
  struct flock lock;
  struct stat dstat;
  char *filename;
  int fid;
  size_t endpt;

  filename = newts_nmalloc (sizeof (char), strlen (SPOOL) +
                            strlen (SEQUENCER) + strlen (name) + 3);

  snprintf (filename, strlen (SPOOL) + strlen (SEQUENCER) + 2, "%s/%s", SPOOL,
            SEQUENCER);

  if (stat (filename, &dstat))
    mkdir (filename, 0775);

  snprintf (filename, strlen (SPOOL) + strlen (SEQUENCER) + strlen (name) + 3,
            "%s/%s/%s", SPOOL, SEQUENCER, name);

  if ((fid = TEMP_FAILURE_RETRY (open (filename, O_RDWR))) == -1)
    {
      fid = TEMP_FAILURE_RETRY (creat (filename, 0666));
      TEMP_FAILURE_RETRY (close (fid));
      fid = TEMP_FAILURE_RETRY (open (filename, O_RDWR));
    }

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;   /* All of it. */
  TEMP_FAILURE_RETRY (fcntl (fid, F_SETLKW, &lock));

  while ((endpt = (size_t)
          TEMP_FAILURE_RETRY (read (fid, &entry, sizeof (struct seq_f))))
          == sizeof (struct seq_f))
    if (strcmp (entry.nfname, ref->name) == 0)
      break;

  if (endpt < 0)
    {
      newts_free (filename);
      return -1;
    }

  if (endpt == sizeof (struct seq_f))
    lseek (fid, -((off_t) sizeof (struct seq_f)), SEEK_CUR);
  else
    {
      strncpy (entry.nfname, ref->name, NNLEN);
      lseek (fid, (off_t) 0, SEEK_END);
    }

  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_CUR;
  lock.l_start = 0;
  lock.l_len = (off_t) sizeof (struct seq_f);
  TEMP_FAILURE_RETRY (fcntl (fid, F_SETLKW, &lock));

  get_uiuc_time (&entry.lastin, seq);
  TEMP_FAILURE_RETRY (write (fid, &entry, sizeof (struct seq_f)));

  fdatasync (fid);
  lock.l_type = F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;
  fcntl (fid, F_SETLK, &lock);

  TEMP_FAILURE_RETRY (close (fid));
  newts_free (filename);
  return 0;
}

int
uiuc_get_next_note (struct newtref *nrp, time_t seq)
{
  struct io_f io;
  struct note_f note;
  struct flock lock;
  if (nrp->notenum < 0)
    nrp->notenum = 0;

  init (&io, &nrp->nfr);

  lock.l_whence = SEEK_SET;
  lock.l_len = (off_t) sizeof (struct note_f);

  while (nrp->notenum <= io.descr.d_nnote)
    {
      nrp->notenum++;

      lock.l_type = F_RDLCK;
      lock.l_start = (off_t) (sizeof (struct descr_f) +
                              (nrp->notenum * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

      getnoterec (&io, nrp->notenum, &note);

      lock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &lock);

      if (note.n_stat & ISDELETED && !allow (&io, DRCTOK))
        {

          nrp->notenum++;
          continue;
        }

      if (difftime (convert_time (&note.n_lmod), seq) > 0)
        {
          closenf (&io);
          return nrp->notenum;
        }
    }
  closenf (&io);
  return -1;
}

int
uiuc_get_next_resp (struct newtref *nrp, time_t seq)
{
  struct io_f io;
  struct note_f note;
  struct resp_f resp;
  struct flock lock;
  int offset, record;
  if (nrp->respnum < 0)
    nrp->respnum = 0;

  init (&io, &nrp->nfr);

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = (off_t) (sizeof (struct descr_f) +
                          (nrp->notenum * sizeof (struct note_f)));
  lock.l_len = (off_t) sizeof (struct note_f);
  TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &lock));

  getnoterec (&io, nrp->notenum, &note);

  lock.l_type = F_UNLCK;
  fcntl (io.fidndx, F_SETLK, &lock);

  while (nrp->respnum <= note.n_nresp)
    {
      nrp->respnum++;
      if (logical_resp (&io, nrp->notenum, nrp->respnum, &resp, &offset,
                        &record) == -1)
        break;
      if (difftime (convert_time (&resp.r_rcvd[offset]), seq) > 0)
        {
          closenf (&io);
          return nrp->respnum;
        }
    }
  closenf (&io);
  return -1;
}
