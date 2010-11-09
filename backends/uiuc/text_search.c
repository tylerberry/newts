/*
 * title_search.c - search for words in a note's title string
 *
 * This file is part of the Newts notesfiles system.
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

#include "strcase.h"

#if STDC_HEADERS
# include <ctype.h>
#endif

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

int
uiuc_text_search (struct newtref *nrp, const char *string)
{
  struct io_f io;
  struct note_f note;
  struct resp_f resp;
  struct flock nlock, tlock;
  size_t notelen, searchlen = 0;
  char *xstring = newts_strdup (string);
  char *text = NULL;
  register int i;
  int offset, record;

  /* FIXME: switch to using a replacement function for strcasestr (which I'll
   * have to write).
   */

  init (&io, &nrp->nfr);

  if (nrp->notenum > io.descr.d_nnote)
    nrp->notenum = io.descr.d_nnote;


  if (nrp->respnum != 0)
    goto inloop;

  while (nrp->notenum > 0)
    {
      nlock.l_type = F_RDLCK;
      nlock.l_start = (off_t) (sizeof (struct descr_f) +
                              (nrp->notenum * sizeof (struct note_f)));
      TEMP_FAILURE_RETRY (fcntl (io.fidndx, F_SETLKW, &nlock));

      getnoterec (&io, nrp->notenum, &note);

      nlock.l_type = F_UNLCK;
      fcntl (io.fidndx, F_SETLK, &nlock);

      if (note.n_stat & ISDELETED)
        {
          nrp->notenum--;
          continue;
        }

      text = newts_nrealloc (text, note.n_addr.textlen + 1, sizeof (char));

      tlock.l_type = F_RDLCK;
      tlock.l_whence = SEEK_SET;
      tlock.l_start = (off_t) note.n_addr.addr;
      tlock.l_len = (off_t) note.n_addr.textlen;
      TEMP_FAILURE_RETRY (fcntl (io.fidtxt, F_SETLKW, &tlock));

      lseek (io.fidtxt, (off_t) note.n_addr.addr, SEEK_SET);
      TEMP_FAILURE_RETRY (read (io.fidtxt, text, note.n_addr.textlen));
      text[note.n_addr.textlen] = '\0';

      tlock.l_type = F_UNLCK;
      fcntl (io.fidtxt, F_SETLK, &tlock);

      searchlen = strlen (xstring);
      notelen = strlen (text);

      for (i=0; i < notelen + 1 - searchlen; i++)
        {
          if (strncasecmp (xstring, text + i, (size_t) searchlen) == 0)
            {
              closenf (&io);
              newts_free (xstring);
              newts_free (text);
              return nrp->notenum;
            }
        }

#if 0
      if (strcasestr (text, xstring) != NULL)
        {
          closenf (&io);
          newts_free (xstring);
          return nrp->notenum;
        }
#endif

      nrp->respnum = 1;

    inloop:

      for (; nrp->respnum <= note.n_nresp; )
        {
          if (logical_resp (&io, nrp->notenum, nrp->respnum, &resp, &offset,
                            &record)
              == -1)
            break;

          text = newts_nrealloc (text, resp.r_addr[offset].textlen + 1,
                                 sizeof (char));

          tlock.l_type = F_RDLCK;
          tlock.l_whence = SEEK_SET;
          tlock.l_start = (off_t) resp.r_addr[offset].addr;
          tlock.l_len = (off_t) resp.r_addr[offset].textlen;
          TEMP_FAILURE_RETRY (fcntl (io.fidtxt, F_SETLKW, &tlock));

          lseek (io.fidtxt, (off_t) resp.r_addr[offset].addr, SEEK_SET);
          TEMP_FAILURE_RETRY (read (io.fidtxt, text, resp.r_addr[offset].textlen));
          text[resp.r_addr[offset].textlen] = '\0';

          tlock.l_type = F_UNLCK;
          fcntl (io.fidtxt, F_SETLK, &tlock);

          notelen = strlen (text);

          for (i=0; i < notelen + 1 - searchlen; i++)
            {
              if (strncasecmp (xstring, text + i, (size_t) searchlen) == 0)
                {
                  closenf (&io);
                  newts_free (xstring);
                  newts_free (text);
                  return nrp->notenum;
                }
            }

          nrp->respnum++;
        }

      nrp->respnum = 0;
      nrp->notenum--;
    }

  closenf (&io);
  newts_free (xstring);
  newts_free (text);
  return -1;
}
