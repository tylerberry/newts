/*
 * access.c - access-related functions for the UIUC backend
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on permit.c and perms.c from the UIUC notes distribution by
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

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_GRP_H
# include <grp.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* getperms - fill in a struct perm_f for IO for username NAME.
 *
 * Whoo, the UIUC version of this puppy was a mess.
 *
 * UIUC notes had a lot of nonsense in here about 'sysflag', which let you
 * check for system permissions.  Cool, but we're doing all network
 * transmissions in a more abstract way.  'sysflag' may have to creep back in
 * here in the future, but bygones.
 */

/* FIXME: this is a hack.  This is not in this module's scope. */
extern uid_t euid;

void
getperms (struct io_f *io, char *name)
{
  static uid_t notes;
  static short notes_is_set = FALSE;

  int permissions = 0;
  int matches = 0;
  int perfectmatch = 0;
  int ngroups = 0;        /* Actual number of groups the user belongs to. */
  GETGROUPS_T *gid;
  char **gname;
  struct flock alock;
  struct group *gr;
  char *filename;
  size_t length;
  int fid;
  struct perm_f entry;

  if (io == NULL || name == NULL)
    return;

  /* Notes is God.  If you're the notes user, you get all privs. */

  if (!notes_is_set)
    {
      struct passwd *pw = getpwnam (NOTES);

      notes = pw->pw_uid;
      notes_is_set = TRUE;
      endpwent ();
    }

  if (euid == notes)
    {
      io->access = READOK + RESPOK + WRITOK + DRCTOK;
      return;
    }

  /* Iterate through all the groups this user belongs to and grab the names.
   * If it doesn't work, we don't really care too much, because we can still
   * get a result.
   */

  ngroups = sysconf (_SC_NGROUPS_MAX);
  gid = newts_nmalloc (sizeof (GETGROUPS_T), ngroups);
  gname = newts_nmalloc (sizeof (char *), ngroups);

  if ((ngroups = getgroups (ngroups, gid)) >= 0)
    {
      register int i, j;

      for (i = 0, j = 0; i < ngroups; i++)
        {
          if ((gr = getgrgid (gid[i])) == NULL)
            {
              continue;   /* Bogus group, skip it and move on. */
            }
          gname[j++] = newts_strdup (gr->gr_name);
        }
      ngroups = j;
    }

  io->access = 0;         /* Clear the official list. */

  length = strlen (io->basedir) + strlen (io->nf) + strlen (ACCESS) + 3;
  filename = newts_nmalloc (sizeof (char), length);
  snprintf (filename, length, "%s/%s/%s", io->basedir, io->nf, ACCESS);

  fid = TEMP_FAILURE_RETRY (open (filename, O_RDONLY));

  alock.l_type = F_RDLCK;
  alock.l_whence = SEEK_SET;
  alock.l_start = 0;
  alock.l_len = 0;    /* All of it. */
  TEMP_FAILURE_RETRY (fcntl (fid, F_SETLKW, &alock));

  while ((TEMP_FAILURE_RETRY (read (fid, &entry, sizeof (struct perm_f))) ==
          sizeof (struct perm_f)) && !perfectmatch)
    {
      /* We're not dealing with system permissions yet. */

      if (entry.ptype == PERMSYSTEM)
        continue;

      /* In the actual UIUC notes distribution, "other" was capitalized.  We
       * use strcasecmp just to be sure.
       */

      if (strcasecmp (entry.name, "other") == 0)
        {
          if (matches == 0)
            {
              permissions = entry.perms;
              matches++;
            }
        }

      switch (entry.ptype)
        {
        case PERMUSER:
          if (strcmp (name, entry.name) == 0)
            {
              /* Specific user permissions are the last word. */

              permissions = entry.perms;
              matches++;
              perfectmatch++;
            }
          break;

        case PERMGROUP:
          {
            register int i;

            for (i = 0; i < ngroups; i++)
              {
                if (strcmp (gname[i], entry.name) == 0)
                  {
                    permissions |= entry.perms;
                    matches++;
                    break;
                  }
              }
          }
          break;

        case PERMSYSTEM:
          break;

        default:
          /* FIXME: error out somehow. */

          break;
        }
    }

  io->access = permissions;

  fdatasync (fid);
  alock.l_type = F_UNLCK;
  fcntl (fid, F_SETLK, &alock);

  TEMP_FAILURE_RETRY (close (fid));
  newts_free (filename);
  newts_free (gid);

  {
    register int i;

    for (i = 0; i < ngroups; i++)
      {
        newts_free (gname[i]);
      }
  }

  newts_free (gname);
}

/* allow - An internal macro to verify permissions; an equivalent of the
 * client-side 'allowed'.  Can the user who opened IO do things with permission
 * MODE?
 *
 * Returns: 0 for false, anything else for true.
 */

int
allow (struct io_f *io, int mode)
{
  if (io == NULL)
    return 0;

  switch (mode)
    {
    case RESPOK:
      return (io->access & (RESPOK + WRITOK + DRCTOK));
      break;
    case READOK:
      return (io->access & (READOK + DRCTOK));
      break;
    case WRITOK:
      return (io->access & (WRITOK + DRCTOK));
      break;
    case DRCTOK:
      return (io->access & DRCTOK);
      break;
    default:
      return 0;
    }
}
