/*
 * get_text.c - invoke editor to get text for a note
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on gtext.c from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad. Any work derived from this source code is required to retain
 * this notice.
 *
 * The thrashdir routine and the signature scheme in general are heavily
 * indebted to the tin newsreader.
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

#include "notes.h"
#include "signals.h"

#if HAVE_DIRENT_H
# include <dirent.h>
#endif

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

static int thrashdir (char *path, char *file, int len);

/* get_text - given which MODE we're in and possibly a note to QUOTE, generate
 * a char * pointing to a string containing the text of a new note. This may
 * invoke an editor if appropriate for the mode.
 *
 * Returns: the string we got, or NULL if no string.
 */

char *
get_text (struct newt *quote, short mode)
{
  FILE *tmpf;
  struct stat statbuf;
  char *text, *tmp;
  int fid, status, result;
  mode_t mask;
  time_t created;

  mask = umask (077); /* Secure Programming HOWTO recommends this. */

  tmp = newts_nmalloc (strlen (tmpdir) + strlen (username) + 15,
                       sizeof (char));
  sprintf (tmp, "%s/newts-%s.XXXXXX", tmpdir, username);

  fid = mkstemp (tmp);
  if (fid == -1)
    return NULL;   /* Must have been EEXIST, could not create unique file. */

  umask (mask);

  fstat (fid, &statbuf);
  created = statbuf.st_mtime;

  result = TEMP_FAILURE_RETRY (close (fid));
  if (result == -1)
    return NULL;   /* Either EIO or EBADF. */

  /* Parse and include the quote. */

  if (quote != NULL)
    {
      int c, wides = 0;
      char *cursor = quote->text;

      tmpf = fopen (tmp, "w");

      if (mode == NORMAL)
        {
          if (strcasecmp (quote->auth.system, fqdn) == 0 &&
              strcasecmp (quote->auth.name, "anonymous"))
            {
              struct passwd *pw = getpwnam (quote->auth.name);
              struct tm *tm = localtime (&quote->created);
              char *date = time_string (tm);

              if (pw)
                {
                  char *real = newts_strdup (pw->pw_gecos);
                  char *separator;

                  if ((separator = strpbrk (real, ":,")) != NULL)
                    *separator = '\0';

                  fprintf (tmpf, "%s (%s) wrote in %s at %s:\n> ", real,
                           quote->auth.name,
                           nfref_pretty_name (&quote->nr.nfr), date);

                  newts_free (real);
                }
              else
                fprintf (tmpf, "%s wrote in %s at %s:\n> ", quote->auth.name,
                         nfref_pretty_name (&quote->nr.nfr), date);

              endpwent ();

              newts_free (date);
            }
          else
            {
              struct passwd *pw = getpwnam (quote->auth.name);
              struct tm *tm = localtime (&quote->created);
              char *date = time_string (tm);

              if (pw)
                {
                  char *real = newts_strdup (pw->pw_gecos);
                  char *separator;

                  if ((separator = strpbrk (real, ":,")) != NULL)
                    *separator = '\0';

                  fprintf (tmpf, "%s (%s@%s) wrote in %s at %s:\n> ", real,
                           quote->auth.name, quote->auth.system,
                           nfref_pretty_name (&quote->nr.nfr), date);

                  newts_free (real);
                }
              else
                fprintf (tmpf, "%s@%s wrote in %s at %s:\n> ",
                         quote->auth.name, quote->auth.system,
                         nfref_pretty_name (&quote->nr.nfr), date);

              endpwent ();

              newts_free (date);
            }
        }
      else if (mode == COPYEDIT || mode == COPYNOEDIT)
        fprintf (tmpf, "(Copied from %s, originally written by %s@%s)\n\n",
                 quote->nr.nfr.name, quote->auth.name, quote->auth.system);

      while ((c = *cursor++) && c != EOF)
        {
          switch (c)
            {
            case '\n':
              if (mode != NORMAL)
                fputc (c, tmpf);
              else
                {
                  if (*cursor) /* If this isn't the last charcter. */
                    {
                      wides = 2;
                      if (*cursor == '\n')
                        fprintf (tmpf, "\n>");
                      else
                        fprintf (tmpf, "\n> ");
                    }
                  else /* If it is the last character. */
                    fprintf (tmpf, "\n\n");
                }
              break;

            default:
              wides++;
              fputc (c, tmpf);
              break;
            }
        }

      fclose (tmpf);
    }

  /* Signature handling! */

  if (mode == NORMAL && signature)
    {
      FILE *sigfile = NULL;
      int c;
      char *randomsigs = newts_nmalloc (strlen (homedir) + 7, sizeof (char));
      char *fixedsig = newts_nmalloc (strlen (homedir) + 11, sizeof (char));
      char *dotsignature = newts_nmalloc (strlen (homedir) + 12,
                                          sizeof (char));
      char *Sig = newts_nmalloc (strlen (homedir) + 5, sizeof (char));
      char *sigfilestr = newts_nmalloc (strlen (randomsigs) + 50,
                                        sizeof (char));

      sprintf (randomsigs, "%s/.sigs", homedir);
      sprintf (fixedsig, "%s/.sigfixed", homedir);
      sprintf (dotsignature, "%s/.signature", homedir);
      sprintf (Sig, "%s/.Sig", homedir);
      *sigfilestr = '.';

      if (stat (randomsigs, &statbuf) == 0 && S_ISDIR (statbuf.st_mode) &&
          thrashdir (randomsigs, sigfilestr, (int) strlen (randomsigs) + 50) != -1
          && *sigfilestr != '.')
        {
          /* Random .sigs directory! Extreme coolness. */

          tmpf = fopen (tmp, "a");

          fprintf (tmpf, "\n\n-- \n");

          if (stat (fixedsig, &statbuf) == 0)
            {
              sigfile = fopen (fixedsig, "r");

              while ((c = getc (sigfile)) != EOF)
                putc (c, tmpf);

              putc ('\n', tmpf);

              fclose (sigfile);
            }

          sigfile = fopen (sigfilestr, "r");

          while ((c = getc (sigfile)) != EOF)
            putc (c, tmpf);

          fclose (sigfile);
          fclose (tmpf);
        }
      else if (stat (dotsignature, &statbuf) == 0)
        {
          sigfile = fopen (dotsignature, "r");
          tmpf = fopen (tmp, "a");

          fprintf (tmpf, "\n\n-- \n");
          while ((c = getc (sigfile)) != EOF)
            putc (c, tmpf);

          fclose (sigfile);
          fclose (tmpf);
        }
      else if (stat (Sig, &statbuf) == 0)
        {
          sigfile = fopen (Sig, "r");
          tmpf = fopen (tmp, "a");

          fprintf (tmpf, "\n\n-- \n");
          while ((c = getc (sigfile)) != EOF)
            putc (c, tmpf);

          fclose (sigfile);
          fclose (tmpf);
        }

      newts_free (sigfilestr);
      newts_free (randomsigs);
      newts_free (fixedsig);
      newts_free (dotsignature);
      newts_free (Sig);
    }

  /* Unless we're in the non-editing modes COPYNOEDIT or VECTOR, we need to
   * fire up and editor.
   */

  if (mode != COPYNOEDIT && mode != VECTOR)
    {
      if (editor == NULL)
        {
          unlink (tmp);
          newts_free (tmp);
          return NULL;
        }

      status = spawn_process (NULL, editor, tmp, NULL);

      if (status != 0)
        {
          unlink (tmp);
          newts_free (tmp);
          return NULL;
        }
    }

  stat (tmp, &statbuf);
  if ((statbuf.st_mtime == created && mode == NORMAL) || statbuf.st_size == 0)
  {
    unlink (tmp);
    newts_free (tmp);
    return NULL;
  }

  /* Transfer the contents of the file into the text buffer. */

  tmpf = fopen (tmp, "r");
  {
    register int count = 0;
    register int c;
    register int bufsize = 1024;
    register char *buf = newts_nmalloc (bufsize, sizeof (char));

    while ((c = getc (tmpf)) != EOF)
      {
        if (count == bufsize)
          {
            bufsize += 1024;
            buf = newts_nrealloc (buf, bufsize, sizeof (char));
          }

        *(buf + count++) = c;
      }
    if (count == bufsize)
      buf = newts_nrealloc (buf, bufsize + 1, sizeof (char));
    *(buf + count) = '\0';

    text = newts_nmalloc (sizeof (char), strlen (buf) + 1);
    sprintf (text, "%s", buf);

    newts_free (buf);
  }

  fclose (tmpf);
  unlink (tmp);
  newts_free (tmp);

  return text;
}

/* thrashdir - in RANDOMDIR, go through and pick a random file. Store its name
 * in RANDOMFILE (which has LEN characters).
 *
 * Returns: -1 on error or not-found; 0 if successful.
 */

static int
thrashdir (char *randomdir, char *randomfile, int len)
{
  DIR *dir;
  struct dirent *de;
  struct stat statbuf;
  int i, numentries = 0;

  if ((dir = opendir (randomdir)) == NULL)
    return -1;

  while ((de = readdir (dir)) != NULL)
    numentries++;

  if (numentries < 3)  /* We only have . and .. */
    {
      closedir (dir);
      return -1;
    }

  /* We try 1000 times at most. That really ought to be enough; any statistical
   * freaks who get rejected because rand() selected .. 1000 times in a row
   * ought to consider themselves lucky.
   */

  for (i = 0, de = NULL; i < 1000 && de == NULL; i++)
    {
      int pick;

#if HAVE_REWINDDIR
      rewinddir (dir);
#else
      closedir (dir);
      dir = opendir (randomdir);
#endif

      pick = rand () % numentries + 1;

      while (pick--)
        if ((de = readdir (dir)) == NULL)
          break;

      if (de != NULL)
        {
          /* We reject anything starting with a dot. */

          if (strcmp (de->d_name, randomdir) == 0 || (de->d_name[0] == '.'))
            de = NULL;
          else
            {
              snprintf (randomfile, len, "%s/%s", randomdir, de->d_name);

              if (stat (randomfile, &statbuf) == -1)
                {
                  closedir (dir);   /* Something's messed up pretty bad, so */
                  return -1;        /* we abort. */
                }

              if (S_ISDIR(statbuf.st_mode))
                de = NULL;

            }
        }
    }

  closedir (dir);
  return de == NULL ? -1 : 0;
}
