/*
 * misc.c - assorted functions for the UIUC backend
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2007 Tyler Berry.
 *
 * Based in part on check.c, gname.c, gtime.c and misc.c from the UIUC notes
 * distribution by Ray Essick and Rob Kolstad.  Any work derived from this
 * source code is required to retain this notice.
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

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* checkpath - checks a string for usability in a path.
 *
 * Returns:  0 if the string is suitable,
 *          -1 if the string isn't.
 */

int
checkpath (const char *path)
{
  int count;

  if (*path == '.')
    return -1;

  count = 0;
  while (*path && (*path != '/') && (*path != ' ') && (*path != ':'))
    {
      count++;
      path++;
    }

  if (count > NNLEN)
    return -1;
  else if (*path == 0)
    return 0;
  else
    return -1;
}

/* getname - get the username and hostname using system calls and store them in
 * the provided struct auth_f.
 */

void
getname (struct auth_f *ident, const int anon_flag)
{
  register int count;
  register char *s, *d;
  char *temp;

  if (anon_flag)
    {
      s = "anonymous";
    }
  else
    {
      struct passwd *pw = getpwuid (getuid());

      if (pw == NULL)
        s = "";
      s = pw->pw_name;
      endpwent ();
    }

  d = ident->aname;
  count = NAMESZ;
  while (((*d++ = *s++) != '\0') && --count);
  *--d = '\0';

  /* FUTURE FIXME: Right now we're getting the author's system using a system
   * call.  This may be a mistake because getname() is run from the server,
   * which might be different from the client.  This will change depending on
   * future development.
   */

  temp = newts_get_fqdn ();
  strncpy (ident->asystem, temp, sizeof (ident->asystem));
}

/* get_uiuc_time - fill in the fields in the provided struct when_f. */

void
get_uiuc_time (struct when_f *when, time_t t)
{
  struct tm *local;
  if (t == 0)
    t = time ((time_t) 0);
  local = gmtime (&t);
  if (local == NULL)
    {
      memset (when, 0, sizeof (struct when_f));
      return;
    }
  when->w_mins = local->tm_min;
  when->w_hours = local->tm_hour;
  when->w_day = local->tm_mday;
  when->w_month = local->tm_mon + 1;
  when->w_year = local->tm_year + 1900;
  when->w_gmttime = (long) t;
}

/* convert_time - convert from UIUC when_f to time_t */

time_t
convert_time (struct when_f *when)
{
  struct tm local;
  time_t converted;

  memset (&local, 0, sizeof (struct tm));

  if (when->w_gmttime)
    return (time_t) when->w_gmttime;

  local.tm_min = when->w_mins;
  local.tm_hour = when->w_hours;
  local.tm_mday = when->w_day;
  local.tm_mon = when->w_month - 1;
  local.tm_year = when->w_year - 1900;
  local.tm_isdst = -1;  /* Make the computer figure it out. */

  converted = mktime (&local);
  if (converted == -1)
    return 0;
  else
    return converted;
}
