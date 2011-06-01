/*
 * compose_mail.c - prepare and send an e-mail from a quoted note
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based in part on mailit.c from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad.  Any work derived from this source code is required to retain
 * this notice.
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

#include "notes.h"

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

/* Various mailers we could run into, with customization for their command-line
 * calls to get them to DWIW.
 */

enum mailers
  {
    MUTT_OR_ELM, /* Mutt rules, and elm has the same command-line syntax. */
    PINE,
    EMACS,       /* Yes, really. */
    GNUCLIENT,   /* Accomplished using gnudoit. */
    EMACSCLIENT, /* Ted O'Connor says CVS emacsclient will work; I've not
                  * tested it. */
    OTHER
  };

/* compose_mail - send mail to ADDRESS, including QUOTE in the mail if
 * provided.  Note: we intentionally do not put in a signature here: that's the
 * mailer's job.
 *
 * Returns: 0 if successful, -1 on error.
 */

int
compose_mail (char *address, struct newt *quote)
{
  int status, result;
  char *subject = NULL;
  char *file = NULL;
  int fid;
  mode_t mask;
  FILE *tmpf;
  static int mailertype = -1;

  if (mailertype == -1)
    {
      if (strstr (mailer, "mutt") != NULL || strstr (mailer, "elm") != NULL)
        mailertype = MUTT_OR_ELM;
      else if (strstr (mailer, "pine") != NULL)
        mailertype = PINE;
      else if (strstr (mailer, "emacs") != NULL)
        mailertype = EMACS;
      /* gnuclient and emacsclient don't work yet. */
      else
        mailertype = OTHER;
    }

  mask = umask (077); /* Secure Programming HOWTO recommends this. */

  file = newts_nmalloc (strlen (tmpdir) + strlen (username) + 15,
                        sizeof (char));
  sprintf (file, "%s/newts-%s.XXXXXX", tmpdir, username);

  fid = mkstemp (file);
  if (fid == -1)
    return -1;   /* Must have been EEXIST, could not create unique file. */

  result = TEMP_FAILURE_RETRY (close (fid));
  if (result == -1)
    return -1;   /* Either EIO or EBADF, both of which are fatal. */

  umask (mask);

  /* Parse and include the quote. */

  if (quote != NULL)
    {
      int c;
      char *cursor = quote->text;
      struct passwd *pw = getpwnam (quote->auth.name);
      struct tm *tm = localtime (&quote->created);
      char *date = time_string (tm);

      tmpf = fopen (file, "w");

      if (pw)
        {
          char *real = newts_strdup (pw->pw_gecos);
          char *separator;

          if ((separator = strpbrk (real, ":,")) != NULL)
            *separator = '\0';

          fprintf (tmpf, _("%s (%s@%s) wrote in %s at %s:\n> "), real,
                   quote->auth.name, quote->auth.system,
                   nfref_pretty_name (&quote->nr.nfr), date);

          newts_free (real);
        }
      else
        fprintf (tmpf, _("%s@%s wrote in %s at %s\n> "),
                   quote->auth.name, quote->auth.system,
                 nfref_pretty_name (&quote->nr.nfr), date);

      newts_free (date);

      while ((c = *cursor++) && c != EOF)
        {
          switch (c)
            {
            case '\n':
              if (*cursor)
                {
                  if (*cursor == '\n')
                    fprintf (tmpf, "\n>");
                  else
                    fprintf (tmpf, "\n> ");
                }
              else
                fprintf (tmpf, "\n\n");
              break;
            default:
              fputc (c, tmpf);
              break;
            }
        }

      fclose (tmpf);

      subject = quote->title;
    }

  /* If we're using mailx or nail or old clients like those, the command-line
   * we construct below makes it think we've already got a completed e-mail,
   * and we just want to send it.  Therefore, we spawn an editor manually so we
   * can edit the note normally.
   */

  if (mailertype == OTHER)
    {
      status = spawn_process (NULL, editor, file, NULL);

      if (status != 0)
        return -1;
    }

  /* Now, run the mailer-specific send command. */

  switch (mailertype)
    {
    case MUTT_OR_ELM:
      if (quote)
        status = spawn_process (NULL, mailer, "-s", subject, "-i", file,
                                address, NULL);
      else
        status = spawn_process (NULL, mailer, address, NULL);
      break;

    case PINE:
      if (quote)
        status = spawn_process (file, mailer, address, NULL);
      else
        status = spawn_process (NULL, mailer, address, NULL);
      break;

    case EMACS:
      {
        char *evalarg, *insertarg;

        if (quote)
          {
            evalarg = newts_nmalloc (strlen (address) + strlen (subject) + 28,
                                     sizeof (char));
            sprintf (evalarg,
                     "--eval=(compose-mail \"%s\" \"%s\")",
                     address, subject);
            insertarg = newts_nmalloc (strlen (file) + 10, sizeof (char));
            sprintf (insertarg, "--insert=%s", file);
          }
        else
          {
            evalarg = newts_nmalloc (strlen (address) + 25, sizeof (char));
            sprintf (evalarg, "--eval=(compose-mail \"%s\")",
                     address);
            insertarg = NULL;
          }

        status = spawn_process (NULL, mailer, evalarg, insertarg, NULL);

        newts_free (evalarg);
        if (insertarg)
          newts_free (insertarg);

        break;
      }

    case OTHER:
    default:
      status = spawn_process (file, mailer, "-s", subject, address, NULL);
      break;
    }

  newts_free (file);

  return 0;
}
