/*
 * getfqdn.c - get this machine's fully-qualified domain name
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2007 Tyler Berry.
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

#if HAVE_NETDB_H
# include <netdb.h>
#endif

#if STDC_HEADERS
# include <stddef.h>
#endif

#if defined STDC_HEADERS || defined HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif

#include "xalloc.h"
#include "xgethostname.h"

/* newts_get_fqdn - return this system's FQDN if it can figure it out; if it
 * can't, it will return the non-FQDN hostname.
 *
 * Returns the FQDN.
 */

char *
newts_get_fqdn (void)
{
  static char *fqdn = NULL;
  struct hostent *hp;

  if (fqdn) return fqdn;

  /* Start by trying gethostname; if it contains a period, assume that
   * it is a valid FQDN.
   */

  fqdn = xgethostname ();
  if (strchr (fqdn, '.'))
    {
      return fqdn;
    }

#if HAVE_GETHOSTBYNAME

  /* Failing that, use the hostname for a DNS or /etc/hosts lookup. */

  hp = gethostbyname (fqdn);
  if (hp == NULL)
    {
      return fqdn;
    }

  /* See if HP->H_NAME is a FQDN (which it should be). */

  if (strchr (hp->h_name, '.'))
    {
      if (strcmp (hp->h_name, "localhost.localdomain"))
        fqdn = xstrdup (hp->h_name);
      else
        fqdn = xstrdup ("localhost");
      return fqdn;
    }

  /* Now try the HP->H_ALIASES, and if any of those are FQDNs, return
   * the first one we find.
   */

  {
    char **aliases = hp->h_aliases;
    char *entry;
    if (aliases)
      {
        while ((entry = *aliases++))
          {
            if (strchr (entry, '.'))
              {
                if (strcmp (entry, "localhost.localdomain"))
                  fqdn = xstrdup (entry);
                else
                  fqdn = xstrdup ("localhost");
                return fqdn;
              }
          }
      }
  }

#endif

  /* Failing everything else, return the non-FQDN hostname. */

  return fqdn;
}
