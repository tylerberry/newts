/*
 * compress_nf.c - recreate a notesfile from scratch
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2008 Tyler Berry
 *
 * Based in part on compress.c from the UIUC notes distribution by Ray Essick
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

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#ifdef FIXTIMES
static void fixtime (struct when_f *when);
#endif

/* compress_nf - rewrite a notesfile's data files, eliminating unsightly holes
 * from previously deleted notes.  NF is the notesfile, and NUMNOTES and
 * NUMRESPS contain the number of notes and responses in the notesfile after
 * compresion.  On error, their values are undefined.
 *
 * Returns: -5 if somebody is already compressing this notesfile (hey, it could
 * happen), or 0 if compression was successful.  Or it segfaults.  *shrug* Or
 * so the UIUC source sez; I haven't been able to make it segfault.
 */

int
uiuc_compress_nf (struct notesfile *nf, unsigned *numnotes, unsigned *numresps)
{
  newts_nfref *ref = nf->ref;
  char *nindx, *rindx, *text, *cnindx, *crindx, *ctext;
  struct newt data;
  struct flock dlock, nlock;
  struct stat statbuf;
  time_t current_time;
  int error;
  int i, j;
  int respptr = 0;
  mode_t old_umask;
  int nnotes = 0;
  int nresps = 0;
  int savedresps;
  int offset, record;
  struct io_f old, new;
  struct daddr_f daddr;
  struct note_f note;
  struct resp_f resp;

  daddr.addr = sizeof (struct daddr_f);

  memset (&data, 0, sizeof (struct newt));

  /* Initialize the newtref. */

  data.nr.nfr.owner = ref->owner;
  data.nr.nfr.name = ref->name;

  /* Save the umask for later. */

  old_umask = umask (0);

  /* Allocate memory for all the filenames. */

  nindx = newts_nmalloc (sizeof (char),
                         (strlen (SPOOL) + strlen (ref->name) +
                          (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                          strlen (NOTEINDX) + 3));
  rindx = newts_nmalloc (sizeof (char),
                         (strlen (SPOOL) + strlen (ref->name) +
                          (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                          strlen (RESPINDX) + 3));
  text = newts_nmalloc (sizeof (char),
                        (strlen (SPOOL) + strlen (ref->name) +
                         (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                         strlen (TEXT) + 3));
  cnindx = newts_nmalloc (sizeof (char),
                          (strlen (SPOOL) + strlen (ref->name) +
                           (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                           strlen (NOTEINDX) + 12));
  crindx = newts_nmalloc (sizeof (char),
                          (strlen (SPOOL) + strlen (ref->name) +
                           (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                           strlen (RESPINDX) + 12));
  ctext = newts_nmalloc (sizeof (char),
                         (strlen (SPOOL) + strlen (ref->name) +
                          (ref->owner != NULL ? strlen (ref->owner) + 1 : 0) +
                          strlen (TEXT) + 12));

  /* Load up the filenames. */

  if (ref->owner == NULL)
    {
      snprintf (nindx, strlen (SPOOL) + strlen (NOTEINDX) +
                strlen (ref->name) + 3,
                "%s/%s/%s", SPOOL, ref->name, NOTEINDX);
      snprintf (rindx, strlen (SPOOL) + strlen (RESPINDX) +
                strlen (ref->name) + 3,
                "%s/%s/%s", SPOOL, ref->name, RESPINDX);
      snprintf (text, strlen (SPOOL) + strlen (TEXT) +
                strlen (ref->name) + 3,
                "%s/%s/%s", SPOOL, ref->name, TEXT);
      snprintf (cnindx, strlen (SPOOL) + strlen (NOTEINDX) +
                strlen (ref->name) + 12,
                "%s/%s/%s.compress", SPOOL, ref->name, NOTEINDX);
      snprintf (crindx, strlen (SPOOL) + strlen (RESPINDX) +
                strlen (ref->name) + 12,
                "%s/%s/%s.compress", SPOOL, ref->name, RESPINDX);
      snprintf (ctext, strlen (SPOOL) + strlen (TEXT) +
                strlen (ref->name) + 12,
                "%s/%s/%s.compress", SPOOL, ref->name, TEXT);
    }
  else
    {
      snprintf (nindx, strlen (SPOOL) + strlen (NOTEINDX) +
                strlen (ref->name) + strlen (ref->owner) + 4,
                "%s/%s:%s/%s", SPOOL, ref->owner, ref->name, NOTEINDX);
      snprintf (rindx, strlen (SPOOL) + strlen (RESPINDX) +
                strlen (ref->name) + strlen (ref->owner) + 4,
                "%s/%s:%s/%s", SPOOL, ref->owner, ref->name, RESPINDX);
      snprintf (text, strlen (SPOOL) + strlen (TEXT) +
                strlen (ref->name) + strlen (ref->owner) + 4,
                "%s/%s:%s/%s", SPOOL, ref->owner, ref->name, TEXT);
      snprintf (cnindx, strlen (SPOOL) + strlen (NOTEINDX) +
                strlen (ref->name) + strlen (ref->owner) + 13,
                "%s/%s:%s/%s.compress", SPOOL, ref->owner,
                ref->name, NOTEINDX);
      snprintf (crindx, strlen (SPOOL) + strlen (RESPINDX) +
                strlen (ref->name) + strlen (ref->owner) + 13,
                "%s/%s:%s/%s.compress", SPOOL, ref->owner,
                ref->name, RESPINDX);
      snprintf (ctext, strlen (SPOOL) + strlen (TEXT) +
                strlen (ref->name) + strlen (ref->owner) + 13,
                "%s/%s:%s/%s.compress", SPOOL, ref->owner,
                ref->name, TEXT);
    }

  /* Create and initialize the virgin files. */

  new.fidndx = TEMP_FAILURE_RETRY (creat (cnindx, 0660));
  new.fidrdx = TEMP_FAILURE_RETRY (creat (crindx, 0660));
  new.fidtxt = TEMP_FAILURE_RETRY (creat (ctext, 0660));

  TEMP_FAILURE_RETRY (write (new.fidrdx, &respptr, sizeof (int)));
  TEMP_FAILURE_RETRY (write (new.fidtxt, &daddr, sizeof (struct daddr_f)));

  closenf (&new);

  /* Set up the new notesfile to start receiving data. */

  new.fidndx = TEMP_FAILURE_RETRY (open (cnindx, O_RDWR));
  new.fidrdx = TEMP_FAILURE_RETRY (open (crindx, O_RDWR));
  new.fidtxt = TEMP_FAILURE_RETRY (open (ctext, O_RDWR));

  error = init (&old, ref);
  if (error != NEWTS_NO_ERROR)
    {
      closenf (&new);
      unlink (cnindx);
      unlink (crindx);
      unlink (ctext);

      newts_free (rindx);
      newts_free (text);
      newts_free (cnindx);
      newts_free (crindx);
      newts_free (ctext);

      umask (old_umask);
      return error;
    }

  /* Copy the directory and endpoint of the notesfile to the new nf, along with
   * access permissions.
   */

  strncpy (new.nf, old.nf, NNLEN);
  strncpy (new.basedir, old.basedir, WDLEN);
  new.access = old.access;

  /* Set up the descriptor, by copying all the old data and resetting the
   * values that need resetting.  But first, put a lock on the notesfile
   * descriptor, so that nobody will go mucking around in the notesfile
   * while we're doing things.
   */

  dlock.l_type = F_WRLCK;
  dlock.l_whence = SEEK_SET;
  dlock.l_start = 0;
  dlock.l_len = (off_t) sizeof (struct descr_f);
  TEMP_FAILURE_RETRY (fcntl (old.fidndx, F_SETLKW, &dlock));

  getdescr (&old, &new.descr);
  if (new.descr.d_stat & NFINVALID)
    {
      closenf (&new);
      unlink (cnindx);
      unlink (crindx);
      unlink (ctext);

      newts_free (nindx);
      newts_free (rindx);
      newts_free (text);
      newts_free (cnindx);
      newts_free (crindx);
      newts_free (ctext);

      umask (old_umask);
      return NEWTS_ALREADY_COMPRESSING;
    }
  new.descr.d_nnote = new.descr.d_delnote = new.descr.d_delresp = 0;
  putdescr (&new, &new.descr);

  /* Set up the lock. */

  nlock.l_whence = SEEK_SET;
  nlock.l_len = (off_t) sizeof (struct note_f);

  /* If there is a policy note, transfer it over. */

  if (old.descr.d_plcy)
    {
      int temp;

      data.nr.notenum = 0;
      data.nr.respnum = 0;

      nlock.l_type = F_RDLCK;
      nlock.l_start = (off_t) sizeof (struct descr_f);
      TEMP_FAILURE_RETRY (fcntl (old.fidndx, F_SETLKW, &nlock));

      getnoterec (&old, 0, &note);

      nlock.l_type = F_UNLCK;
      fcntl (old.fidndx, F_SETLK, &nlock);

      movetextrec (&old, &note.n_addr, &new, &daddr);

#ifdef FIXTIMES
      fixtime (&note.n_rcvd);
      fixtime (&note.n_date);
      fixtime (&note.n_lmod);
#endif

      temp = load_note (&data, NULL, FALSE);
      put_note (&new, &daddr, &data, ADD_POLICY | SKIP_MODERATION);
    }

  /* Recursively copy all the notes and responses. */

  for (i=1; i<=old.descr.d_nnote; i++)
    {
      data.nr.notenum = i;
      data.nr.respnum = 0;

      nlock.l_type = F_RDLCK;
      nlock.l_start = (off_t) (sizeof (struct descr_f) +
                              (i * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (old.fidndx, F_SETLKW, &nlock));

      getnoterec (&old, i, &note);

      nlock.l_type = F_UNLCK;
      fcntl (old.fidndx, F_SETLK, &nlock);

      /* Okay, here we're checking for two things.  One, if the note is marked
       * as deleted.  Two, if the note fails any of our sanity checks.  If
       * either of these happens, skip the note and move on to the next one.
       */

      fstat (old.fidtxt, &statbuf);
      time (&current_time);
      if (note.n_stat & ISDELETED || note.n_addr.textlen > HARDMAX ||
          note.n_nresp < 0 || convert_time (&note.n_lmod) > current_time ||
          convert_time (&note.n_date) > current_time ||
          (off_t) (note.n_addr.textlen + note.n_addr.addr) > statbuf.st_size)
        continue;

      /* Move over the text. */

      movetextrec (&old, &note.n_addr, &new, &daddr);

#ifdef FIXTIMES
      fixtime (&note.n_rcvd);
      fixtime (&note.n_date);
      fixtime (&note.n_lmod);
#endif

      /* Save the number of responses we had previously. */

      savedresps = note.n_nresp;

      load_note (&data, NULL, FALSE);
      put_note (&new, &daddr, &data, SKIP_MODERATION);

      nnotes++;

      for (j=1; j<=savedresps; j++)
        {
          data.nr.respnum = j;

          /* logical_resp has internal locking. */

          if (logical_resp (&old, i, j, &resp, &offset, &record) != 0)
            break;

          movetextrec (&old, &resp.r_addr[offset], &new, &daddr);

#ifdef FIXTIMES
          fixtime (&resp.r_when[offset]);
          fixtime (&resp.r_rcvd[offset]);
#endif

          load_note (&data, NULL, FALSE);
          put_resp (&new, &daddr, &data, SKIP_MODERATION);

          nresps++;
        }
    }

  /* Having copied all the notes and responses over, it's time to replace the
   * old files.  First mark the old set as invalid, then replace each file.
   */

  closenf (&new);

  getdescr (&old, &old.descr);
  old.descr.d_stat |= NFINVALID;
  putdescr (&old, &old.descr);

  /* We're ready to release the lock and replace the old files. */

  fdatasync (old.fidndx);
  dlock.l_type = F_UNLCK;
  fcntl (old.fidndx, F_SETLK, &dlock);

  unlink (rindx);
  link (crindx, rindx);
  unlink (crindx);
  unlink (text);
  link (ctext, text);
  unlink (ctext);
  unlink (nindx);
  link (cnindx, nindx);
  unlink (cnindx);

  /* Clean up. */

  uiuc_update_nf (nf);
  *numnotes = nnotes;
  *numresps = nresps;

  umask (old_umask);

  newts_free (nindx);
  newts_free (rindx);
  newts_free (text);
  newts_free (cnindx);
  newts_free (crindx);
  newts_free (ctext);

  return 0;
}

#ifdef FIXTIMES

/* fixtime - verify the w_gmttime field of WHEN to make sure that it is either
 * (1) zero or (2) a time_t corresponding to the other fields of WHEN.  If it
 * is neither of these, it is set to zero.
 */

static void
fixtime (struct when_f *when)
{
  struct when_f built;

  if (when->w_gmttime == 0)
    return;
  if (when->w_gmttime < 0)
    {
      when->w_gmttime = 0;
      return;
    }
  get_uiuc_time (&built, (time_t) when->w_gmttime);
  if (build.w_year != when->w_year ||
      built.w_month != when -> w_month ||
      built.w_day != when -> w_day ||
      built.w_hours != when -> w_hours ||
      built.w_mins != when -> w_mins)
    when -> w_gmttime = 0;
  return;
}

#endif
