/*
 * create_nf.c - create a UIUC notesfile
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based on buildnf.c from the UIUC notes distribution by Ray Essick and Rob
 * Kolstad.  Any work derived from this source code is required to retain this
 * notice.
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

static void init_descr (struct descr_f *descr);

/* create_nf actually creates the notesfile directory and files.  It performs
 * logic checks, to make sure creating the notesfile is actually possible, but
 * doesn't check access privileges or the like.
 *
 * Returns:  0 if sucessful,
 *          -1 if not,
 *          -2 if it already exists.
 */

int
uiuc_create_nf (const newts_nfref *ref, int flags)
{
  char *filename;
  size_t length;
  int fid;
  mode_t old_umask;

  if (checkpath (ref->name) != 0)
    return NEWTS_INVALID_NOTESFILE_NAME;

  /*
   * longest filename, we only need to use malloc once.  We add 3 to the sum
   * for the NUL character and the two slashes we interpose into the filename.
   */

  {
    size_t long_filename = strlen (NOTEINDX);
    if (strlen (RESPINDX) > long_filename) long_filename = strlen (RESPINDX);
    if (strlen (TEXT) > long_filename) long_filename = strlen (TEXT);
    if (strlen (ACCESS) > long_filename) long_filename = strlen (ACCESS);
    length = strlen (SPOOL) + (ref->owner != NULL ? strlen (ref->owner) + 1 : 0)
      + strlen (ref->name) + long_filename + 3;

    filename = newts_nmalloc (length, sizeof (char));
  }

  /* We stat the spool directory to make sure it's kosher, then create the
   * directory for the new notesfile.
   */

  {
    struct stat spoolstat;

    if (stat (SPOOL, &spoolstat))
      mkdir (SPOOL, 0755);

    if (ref->owner == NULL)
      snprintf (filename, length, "%s/%s", SPOOL, ref->name);
    else
      snprintf (filename, length, "%s/%s:%s", SPOOL, ref->owner, ref->name);

    /* If the directory already exists we want a different return value to
     * display an appropriate message.
     */

    if (stat (filename, &spoolstat) == 0)
      {
        newts_free (filename);
        return -2;
      }
  }

  /* We need to be able to create files with arbitrary file permissions, so we
   * clear the umask.
   */

  old_umask = umask (0);

  mkdir (filename, 0770);

  /* Create the note index file. */

  {
    struct descr_f descr;

    if (ref->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, ref->name, NOTEINDX);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, ref->owner,
                ref->name, NOTEINDX);
    fid = TEMP_FAILURE_RETRY (creat (filename, 0660));

    init_descr (&descr);
    if (flags & NF_ANONYMOUS)
      descr.d_stat |= ANONOK;
    if (~(flags & NF_LOCKED))
      descr.d_stat |= ISOPEN;
    if (flags & NF_MODERATED)
      descr.d_stat |= ISMODERATED;

    /* Update the .SEQ file, which keeps track of numerical ID values for
     * notesfiles; this ensures new notesfiles have totally unique IDs.
     */

    {
      FILE *seqfile;
      struct stat statbuf;
      struct flock lock;
      int i, count;

      snprintf (filename, length, "%s/%s", SPOOL, SEQ);

      lock.l_type = F_WRLCK;
      lock.l_whence = SEEK_SET;
      lock.l_start = 0;
      lock.l_len = 0;

      if (stat (filename, &statbuf))
        {
          i = 1;
          seqfile = fopen (filename, "w");
          TEMP_FAILURE_RETRY (fcntl (fileno (seqfile), F_SETLKW, &lock));
          fprintf (seqfile, "%d\n", i);
          lock.l_type = F_UNLCK;
          fcntl (fileno (seqfile), F_SETLK, &lock);
          lock.l_type = F_WRLCK;
          fclose (seqfile);
        }

      seqfile = fopen (filename, "r+");
      if (seqfile == NULL)
        {
          newts_free (filename);
          return -1;
        }

      TEMP_FAILURE_RETRY (fcntl (fileno (seqfile), F_SETLKW, &lock));

      count = fscanf (seqfile, "%d", &i);
      if (count != 1)
        {
          newts_free (filename);
          return -1;
        }
      descr.d_nfnum = i++;
      ftruncate (fileno (seqfile), (off_t) 0);
      fseek (seqfile, (off_t) 0, SEEK_SET);
      fprintf (seqfile, "%d\n", i);
      lock.l_type = F_UNLCK;
      fcntl (fileno (seqfile), F_SETLK, &lock);
      fclose (seqfile);
    }

    TEMP_FAILURE_RETRY (write (fid, &descr, sizeof descr));
    TEMP_FAILURE_RETRY (close (fid));
  }

  /* Create the text file. */

  {
    struct daddr_f daddr;

    if (ref->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, ref->name, TEXT);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, ref->owner,
                ref->name, TEXT);
    fid = TEMP_FAILURE_RETRY (creat (filename, 0660));
    daddr.addr = sizeof (struct daddr_f);
    if (daddr.addr & 1)           /* If the address is odd (it shouldn't be) */
      daddr.addr++;               /* then make it even. */
    TEMP_FAILURE_RETRY (write (fid, &daddr, sizeof daddr));
    TEMP_FAILURE_RETRY (close (fid));
  }

  /* Create the response index. */

  {
    int respptr;

    if (ref->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, ref->name, RESPINDX);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, ref->owner,
                ref->name, RESPINDX);
    fid = TEMP_FAILURE_RETRY (creat (filename, 0660));
    respptr = 0;
    TEMP_FAILURE_RETRY (write (fid, &respptr, sizeof (int)));
    TEMP_FAILURE_RETRY (close (fid));
  }

  /* Set up the access file, first by adding in default permissions and then
   * iterating over the access-template file (if it exists) and adding site
   * defined permissions.
   */

  {
    struct auth_f auth;
    struct perm_f perms[NPERMS];

    if (ref->owner == NULL)
      snprintf (filename, length, "%s/%s/%s", SPOOL, ref->name, ACCESS);
    else
      snprintf (filename, length, "%s/%s:%s/%s", SPOOL, ref->owner,
                ref->name, ACCESS);
    fid = TEMP_FAILURE_RETRY (creat (filename, 0660));
    getname (&auth, 0);
    perms[0].ptype = PERMUSER;
    strncpy (perms[0].name, auth.aname, NAMESZ);
    perms[0].perms = READOK + WRITOK + RESPOK + DRCTOK;
    perms[1].ptype = PERMGROUP;
    strncpy (perms[1].name, "other", NAMESZ);
    perms[1].perms = READOK + WRITOK + RESPOK;
    perms[2].ptype = PERMSYSTEM;
    strncpy (perms[2].name, "other", NAMESZ);
    perms[2].perms = READOK + WRITOK + RESPOK;

    TEMP_FAILURE_RETRY (write (fid, perms, sizeof perms[0] * 3));
    TEMP_FAILURE_RETRY (close (fid));
  }

  /* FIXME: Here we should have code for reading the access template and adding
   * permission entries accordingly.
   */

  newts_free (filename);
  umask (old_umask);

  return 0;
}

/* init_descr sets up a virgin descriptor. */

static void
init_descr (struct descr_f *descr)
{
  descr->d_format = DBVERSION;
  strncpy (descr->d_title, "** Notesfile Title **", NNLEN);
  strncpy (descr->d_drmes, "** Director Message **", DMLEN);
  descr->d_plcy = descr->d_id.uniqid = 0;
  gethostname (descr->d_id.sys, sizeof descr->d_id.sys);
  get_uiuc_time (&descr->d_lstxmit, 0);
  get_uiuc_time (&descr->d_lastm, 0);
  get_uiuc_time (&descr->d_created, 0);
  get_uiuc_time (&descr->d_lastuse, 0);
  descr->d_daysused = 1;
  descr->d_stat = descr->d_nnote = 0;
  descr->d_delnote = descr->d_delresp = 0;
  descr->d_archtime = descr->d_workset = 0;
  descr->d_archkeep = KEEPDFLT;
  descr->d_dmesgstat = DIRDFLT;
  descr->d_rspwrit = descr->d_notwrit = 0;
  descr->d_rspread = descr->d_notread = 0;
  descr->d_notdrop = descr->d_rspdrop = 0;
  descr->d_orphans = descr->d_adopted = 0;
  descr->entries = descr->walltime = 0;
  descr->d_rspxmit = descr->d_notxmit = 0;
  descr->d_rsprcvd = descr->d_notrcvd = 0;
  descr->netwrkouts = descr->netwrkins = 0;
  descr->d_longnote = MAXMSG;
}
