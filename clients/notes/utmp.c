/*
 * utmp.c - reading author status out of the utmp file
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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Newts; if not, write to the Free Software Foundation, Inc., 59 Temple Place,
 * Suite 330, Boston, MA 02111-1307 USA
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"

#include "newts/memory.h"
#include "readutmp.h"

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

/* get_author_utmp - determine the online/offline and writable status of USER.
 *
 * Returns: 2 for author is online and writable, 1 for author is online and not
 * writable, 0 for author is not online or on error.
 */

int
get_author_utmp (const char *user)
{
  size_t num_users;
  STRUCT_UTMP *utmp_list, *ut;
  int fail;
  int result = 0;

  fail = read_utmp (UTMP_FILE, &num_users, &utmp_list, READ_UTMP_CHECK_PIDS);
  if (fail)
    return 0;

  ut = utmp_list;

  while (num_users--)
    {
      if (IS_USER_PROCESS (ut) && strcmp (UT_USER (ut), user) == 0)
        {
          struct stat st;
          char *line = newts_zalloc (5 + sizeof (ut->ut_line) + 1);

          if (ut->ut_line[0] == '/')
            {
              strcpy (line, ut->ut_line);
            }
          else
            {
              strcpy (line, "/dev/");
              strcat (line, ut->ut_line);
            }

          if (stat (line, &st) == 0)
            {
              if (st.st_mode & S_IWGRP)
                {
                  newts_free (line);
                  free (utmp_list);
                  return 2;
                }
            }

          newts_free (line);

          result = 1;
        }
      ut++;
    }

  free (utmp_list);

  return result;
}
