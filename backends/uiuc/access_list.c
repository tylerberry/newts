/*
 * get_access_list.c - build a list of access permissions from UIUC database
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
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

/* get_access_list - return a linked list of all the permission entries for the
 * notesfile referred to by REF.  LIST points to the new list.
 *
 * Returns: -1 on error, or the number of items in the list if successful.
 */

int
uiuc_get_access_list (const newts_nfref *ref, List *list)
{
  struct io_f io;
  struct perm_f perms[NPERMS];
  struct access *entry;
  struct flock lock;
  int accessfile;
  char *filename;
  short items, i;

  if (ref == NULL || list == NULL)
    return -1;

  init (&io, ref);

  filename = newts_nmalloc (sizeof (char), strlen (io.basedir) +
                            strlen (io.nf) + strlen (ACCESS) + 3);
  snprintf (filename, strlen (io.basedir) + strlen (io.nf) +
            strlen (ACCESS) + 3, "%s/%s/%s", io.basedir, io.nf, ACCESS);

  accessfile = TEMP_FAILURE_RETRY (open (filename, O_RDONLY));

  lock.l_type = F_RDLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;   /* All of it. */
  TEMP_FAILURE_RETRY (fcntl (accessfile, F_SETLKW, &lock));

  items = TEMP_FAILURE_RETRY (read (accessfile, perms,
                                    sizeof (struct perm_f) * (size_t) NPERMS));
  items /= sizeof (struct perm_f);

  fdatasync (accessfile);
  lock.l_type = F_UNLCK;
  fcntl (accessfile, F_SETLK, &lock);

  TEMP_FAILURE_RETRY (close (accessfile));

  list_init (list, (void * (*) (void)) access_alloc,
             (void (*) (void *)) access_free,
             (int (*) (const void *, const void *)) access_compare);

  for (i=0; i<items; i++)
    {
      entry = access_alloc ();
      access_set_name (entry, perms[i].name);
      access_set_scope (entry, perms[i].ptype);
      access_set_permissions (entry, perms[i].perms);
      list_insert_next (list, list_tail (list), (void *) entry);
    }

  newts_free (filename);
  closenf (&io);

  return items;
}

int
uiuc_write_access_list (const newts_nfref *ref, List *list)
{
  struct io_f io;
  struct perm_f perms[NPERMS];
  struct listnode *node;
  struct access *entry;
  struct flock lock;
  int accessfile;
  char *filename;
  short items = 0;
  short i;

  if (ref == NULL || list == NULL)
    return -1;

  init (&io, ref);

  filename = newts_nmalloc (sizeof (char), strlen (io.basedir) +
                            strlen (io.nf) + strlen (ACCESS) + 3);
  snprintf (filename, strlen (io.basedir) + strlen (io.nf) +
            strlen (ACCESS) + 3, "%s/%s/%s", io.basedir, io.nf, ACCESS);
  accessfile = TEMP_FAILURE_RETRY (open (filename, O_WRONLY | O_TRUNC));

  lock.l_type = F_WRLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;   /* All of it. */
  TEMP_FAILURE_RETRY (fcntl (accessfile, F_SETLKW, &lock));

  node = list_head (list);
  for (i=0; i < list_size (list); i++)
    {
      entry = (struct access *) list_data (node);
      strncpy (perms[i].name, access_name (entry), NAMESZ);
      perms[i].ptype = access_scope (entry);
      perms[i].perms = access_permissions (entry);
      TEMP_FAILURE_RETRY (write (accessfile, &perms[i],
                                 sizeof (struct perm_f)));
      items++;
      node = list_next (node);
      if (node == NULL)
        break;
    }

  fdatasync (accessfile);
  lock.l_type = F_UNLCK;
  fcntl (accessfile, F_SETLK, &lock);

  TEMP_FAILURE_RETRY (close (accessfile));

  newts_free (filename);
  closenf (&io);

  return items;
}
