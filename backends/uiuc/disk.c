/*
 * disk.c - disk-related functions for the UIUC backend
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2008 Tyler Berry.
 *
 * Based in part on pagemove.c and recsio.c from the UIUC notes distribution by
 * Ray Essick and Rob Kolstad.  Any work derived from this source code is
 * required to retain this notice.
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

#if HAVE_SYS_STAT_H
#  include <sys/stat.h>
#endif

#if HAVE_FCNTL_H
#  include <fcntl.h>
#endif

static int opennf (struct io_f *io, const newts_nfref *ref);

/* FIXME: NOTE TO SELF!
 *
 * Half of this file needs to be error checked and currently ISN'T.  Fix that.
 */

int
init (struct io_f *io, const newts_nfref *ref)
{
  int result;
  struct auth_f ident;
  struct flock lock;

  if ((result = opennf (io, ref)) != NEWTS_NO_ERROR)
    {
      return result;
    }

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = (off_t) sizeof (struct daddr_f);
  TEMP_FAILURE_RETRY (fcntl (io->fidndx, F_SETLKW, &lock));

  getdescr (io, &io->descr);

  lock.l_type = F_UNLCK;
  fcntl (io->fidndx, F_SETLK, &lock);

  if (io->descr.d_format != DBVERSION)
    {
      closenf (io);
      return NEWTS_INCORRECT_DBVERSION;
    }

  getname (&ident, 0);
  getperms (io, ident.aname);

#if 0 /* Currently we don't use these at all. */
  io->nrspwrit = io->nnotwrit = 0;
  io->nrspread = io->nnotread = 0;
  io->nnotxmit = io->nrspxmit = 0;
  io->nnotrcvd = io->nrsprcvd = 0;
  io->nnotdrop = io->nrspdrop = 0;
  io->norphans = io->adopted = 0;
#endif

  io->xstring[0] = io->xauthor[0] = '\0';

  time (&io->entered);

  return NEWTS_NO_ERROR;
}

int
opennf (struct io_f *io, const newts_nfref *ref)
{
  char *filename;
  size_t length;

  /* If we're passed in a string in REF->NAME, we need to parse that string to
   * fill in various fields in IO.
   */

  if (ref->name != (char *) NULL)
    {
      strncpy (io->basedir, SPOOL, WDLEN);
      if (ref->owner == NULL)
        snprintf (io->nf, NNLEN, "%s", ref->name);
      else
        snprintf (io->nf, NNLEN, "%s:%s", ref->owner, ref->name);
      snprintf (io->fullname, WDLEN, "%s/%s", io->basedir, io->nf);
    }

  /* At this point, we have IO->BASEDIR, IO->NF, and IO->FULLNAME set up,
   * however we got there, either by parsing them or by having them already set
   * up in IO.
   *
   * We're ready to enter the long, dark night of file descriptors.
   */

  {
    size_t long_filename = strlen (NOTEINDX);
    if (strlen (RESPINDX) > long_filename) long_filename = strlen (RESPINDX);
    if (strlen (TEXT) > long_filename) long_filename = strlen (TEXT);
    length = strlen (io->fullname) + long_filename + 2;

    filename = newts_nmalloc (sizeof (char), length);
  }

  /* Make sure that his alleged notesfile actually exists. */

  {
    struct stat nfstat;

    snprintf (filename, length, "%s", io->fullname);
    if (stat (filename, &nfstat))
      {
        newts_free (filename);
        return NEWTS_NF_DOESNT_EXIST;
      }
  }

  snprintf (filename, length, "%s/%s", io->fullname, TEXT);
  if ((io->fidtxt = TEMP_FAILURE_RETRY (open (filename, O_RDWR))) < 0)
    {
      newts_free (filename);
      return NEWTS_UNABLE_TO_OPEN;
    }

  snprintf (filename, length, "%s/%s", io->fullname, NOTEINDX);
  if ((io->fidndx = TEMP_FAILURE_RETRY (open (filename, O_RDWR))) < 0)
    {
      newts_free (filename);
      TEMP_FAILURE_RETRY (close (io->fidtxt));
      return NEWTS_UNABLE_TO_OPEN;
    }

  snprintf (filename, length, "%s/%s", io->fullname, RESPINDX);
  if ((io->fidrdx = TEMP_FAILURE_RETRY (open (filename, O_RDWR))) < 0)
    {
      newts_free (filename);
      TEMP_FAILURE_RETRY (close (io->fidtxt));
      TEMP_FAILURE_RETRY (close (io->fidndx));
      return NEWTS_UNABLE_TO_OPEN;
    }

  newts_free (filename);
  return NEWTS_NO_ERROR;
}

int
closenf (struct io_f *io)
{
  /* FIXME: these should not use the TEMP_FAILURE_RETRY macro. */

  TEMP_FAILURE_RETRY (close (io->fidtxt));
  TEMP_FAILURE_RETRY (close (io->fidndx));
  TEMP_FAILURE_RETRY (close (io->fidrdx));

  return NEWTS_NO_ERROR;
}

int
getdescr (struct io_f *io, struct descr_f *descr)
{
  int error = NEWTS_NO_ERROR;

  lseek (io->fidndx, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (read (io->fidndx, descr, sizeof *descr));

  return error;
}

int
putdescr (struct io_f *io, struct descr_f *descr)
{
  int error = NEWTS_NO_ERROR;

  lseek (io->fidndx, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (write (io->fidndx, descr, sizeof *descr));
  fsync (io->fidndx);

  return error;
}

void
getnoterec (struct io_f *io, int n, struct note_f *note)
{
  off_t where;

  if (n >= 0)
    {
      where = (off_t) (sizeof (struct descr_f) + (n * sizeof *note));
      lseek (io->fidndx, where, SEEK_SET);
      TEMP_FAILURE_RETRY (read (io->fidndx, note, sizeof *note));
    }
}

void
putnoterec (struct io_f *io, int n, struct note_f *note)
{
  off_t where;

  if (n >= 0)
    {
      where = (off_t) (sizeof (struct descr_f) + (n * sizeof *note));
      lseek (io->fidndx, where, SEEK_SET);
      TEMP_FAILURE_RETRY (write (io->fidndx, note, sizeof *note));
      fsync (io->fidndx);
    }
}

void
getresprec (struct io_f *io, int n, struct resp_f *resp)
{
  off_t where;

  if (n >= 0)
    {
      where = (off_t) (sizeof (int) + (n * sizeof *resp));
      lseek (io->fidrdx, where, SEEK_SET);
      TEMP_FAILURE_RETRY (read (io->fidrdx, resp, sizeof *resp));
    }
}

void
putresprec (struct io_f *io, int n, struct resp_f *resp)
{
  off_t where;

  if (n >= 0)
    {
      where = (off_t) (sizeof (int) + (n * sizeof *resp));
      lseek (io->fidrdx, where, SEEK_SET);
      TEMP_FAILURE_RETRY (write (io->fidrdx, resp, sizeof *resp));
      fsync (io->fidrdx);
    }
}

long
puttextrec (struct io_f *io, char *text, struct daddr_f *where, int max)
{
  register int c;
  register unsigned i = 0;
  register long nchars = 0;
  register long ignored = 0;
  register int ignoring = FALSE;
  struct daddr_f nwhere;
  struct txtbuf_f buf;
  struct flock tlock;
  struct flock nlock;

  tlock.l_type = F_WRLCK;
  tlock.l_whence = SEEK_SET;
  tlock.l_start = 0;
  tlock.l_len = (off_t) sizeof (struct daddr_f);   /* The free pointer block. */

  TEMP_FAILURE_RETRY (fcntl (io->fidtxt, F_SETLKW, &tlock));

  lseek (io->fidtxt, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (read (io->fidtxt, where, sizeof (struct daddr_f)));

  nlock.l_type = F_WRLCK;
  nlock.l_whence = SEEK_SET;
  nlock.l_start = (off_t) where->addr;     /* The new text address. */
  nlock.l_len = 0;
  TEMP_FAILURE_RETRY (fcntl (io->fidtxt, F_SETLKW, &nlock));

  lseek (io->fidtxt, (off_t) where->addr, SEEK_SET);

  where->textlen = 0;

  while ((c = *text++))
    {
      if (!ignoring)
        {
          if (i == BUFSIZE)
            {
              TEMP_FAILURE_RETRY (write (io->fidtxt, buf.txtbuf, BUFSIZE));
              i = 0;
            }
          buf.txtbuf[i++] = c;
          if (++nchars >= io->descr.d_longnote)
            {
              ignoring++;
            }
        }
      else
        {
          ignored++;
        }
    }

  if (i != 0)
    {
      TEMP_FAILURE_RETRY (write (io->fidtxt, buf.txtbuf, (size_t) i));
    }

  /* fdatasync here unnecessary because we have a lock over the pointer block.
   */

  nlock.l_type = F_UNLCK;
  fcntl (io->fidtxt, F_SETLK, &nlock);    /* Unlock new text. */

  where->textlen = nchars;
  lseek (io->fidtxt, (off_t) 0, SEEK_SET);
  nwhere.addr = where->addr + nchars;
  if (nwhere.addr & 1)
    nwhere.addr++;

  TEMP_FAILURE_RETRY (write (io->fidtxt, &nwhere, sizeof nwhere));

  fdatasync (io->fidtxt);
  tlock.l_type = F_UNLCK;
  fcntl (io->fidtxt, F_SETLK, &tlock);    /* Unlock free pointer. */

  return (long) nchars;
}

long
movetextrec (struct io_f *old, struct daddr_f *from,
             struct io_f *new, struct daddr_f *to)
{
  char buf[BUFSIZE];
  register int bufchars;
  register long moved;
  register long total;
  register long need;
  struct daddr_f next;
  struct flock flock;
  struct flock nlock;
  struct flock tlock;

  /* Handle the trivial case quickly. */

  if (from->addr == 0 || from->textlen == 0)
    {
      to->addr = 0;
      to->textlen = 0;
      return 0;
    }

  /* Set everything up for the move. */

  nlock.l_type = F_WRLCK;
  nlock.l_whence = SEEK_SET;
  nlock.l_start = 0;
  nlock.l_len = (off_t) sizeof (struct daddr_f);
  TEMP_FAILURE_RETRY (fcntl (new->fidtxt, F_SETLKW, &nlock));

  lseek (old->fidtxt, (off_t) from->addr, SEEK_SET);
  lseek (new->fidtxt, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (read (new->fidtxt, to, sizeof (struct daddr_f)));
  lseek (new->fidtxt, (off_t) to->addr, SEEK_SET);
  moved = 0;
  total = from->textlen;
  bufchars = 0;
  to->textlen = 0;

  flock.l_type = F_RDLCK;
  flock.l_whence = SEEK_SET;
  flock.l_start = from->addr;
  flock.l_len = from->textlen;
  TEMP_FAILURE_RETRY (fcntl (old->fidtxt, F_SETLKW, &flock));

  tlock.l_type = F_WRLCK;
  tlock.l_whence = SEEK_SET;
  tlock.l_start = to->addr;
  tlock.l_len = from->textlen;
  TEMP_FAILURE_RETRY (fcntl (new->fidtxt, F_SETLKW, &tlock));

  while (moved < total)
    {
      need = total - moved;
      if (need > BUFSIZE)
        need = BUFSIZE;
      bufchars = TEMP_FAILURE_RETRY (read (old->fidtxt, &buf, (size_t) need));
      if (bufchars != need)
        ; /* FIXME: Handle an error. */
      TEMP_FAILURE_RETRY (write (new->fidtxt, &buf, (size_t) bufchars));
      moved += bufchars;
      to->textlen += bufchars;
    }

  flock.l_type = F_UNLCK;
  fcntl (old->fidtxt, F_SETLK, &flock);

  fdatasync (new->fidtxt);
  tlock.l_type = F_UNLCK;
  fcntl (new->fidtxt, F_SETLK, &tlock);

  if (from->textlen != to->textlen)
    ; /* FIXME: Handle an error. */

  /* Now that we've moved things, we need to update the pointer to the next
   * available block in the "new" text file.
   */

  next.textlen = 0;
  next.addr = to->addr + to->textlen;
  if (next.addr & 1)
    next.addr++;          /* Align on a 2-bit boundary. */

  lseek (new->fidtxt, (off_t) 0, SEEK_SET);
  TEMP_FAILURE_RETRY (write (new->fidtxt, &next, sizeof (struct daddr_f)));

  fdatasync (new->fidtxt);
  nlock.l_type = F_UNLCK;
  fcntl (new->fidtxt, F_SETLK, &nlock);

  return moved;
}
