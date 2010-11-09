/*
 * getnote.c - print single notes to stdout
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
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

#include "frontend.h"

#include "dirname.h"
#include "error.h"
#include "getopt.h"
#include "strpbrk.h"

#if HAVE_FCNTL_H
# include <fcntl.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

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

/* Whether to display debugging messages. */
int debug = FALSE;

/* Whether to print a header. */
int print_header = FALSE;

int
main (int argc, char **argv)
{
  newts_nfref *ref;
  struct notesfile nf;
  struct newt note;

  int opt;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {N_("debug"),0,0,'D'},
      {N_("help"),0,0,'h'},
      {N_("print-header"),0,0,'p'},
      {N_("version"),0,0,0},
      {0,0,0,0}
    };

  memset (&nf, 0, sizeof (struct notesfile));
  memset (&note, 0, sizeof (struct newt));

#ifdef __GLIBC__
  program_name = program_invocation_short_name;
#else
  program_name = base_name (argv[0]);
#endif

  /* Initialize i18n. */

#ifdef HAVE_SETLOCALE
  setlocale (LC_ALL, "");
#endif

#if ENABLE_NLS
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

  setup ();

  while ((opt = getopt_long (argc, argv, N_("hp"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("getnote"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'D':
          debug = TRUE;
          break;

        case 'p':
          print_header = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE NOTENUM [RESPNUM]\n"
                    "Print the specified note from NOTESFILE.\n\n"),
                  program_name);

          printf (_("  -p, --print-header   Print header information (author, etc.) for the note\n"
                    "  -h, --help           Display this help and exit\n"
                    "      --debug          Display debugging messages\n"
                    "      --version        Display version information and exit\n\n"));

          printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);

          teardown ();

          if (fclose (stdout) == EOF)
            error (EXIT_FAILURE, errno, _("error writing output"));
          exit (EXIT_SUCCESS);

        case '?':
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          teardown ();

          exit (EXIT_FAILURE);
        }
    }

  if (optind == argc || optind + 1 == argc)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      teardown ();

      exit (EXIT_FAILURE);
    }
  else if (optind + 2 == argc) /* NF NOTENUM */
    {
      ref = nfref_alloc ();
      parse_single_nf (argv[optind++], ref);
      note.nr.notenum = atoi (argv[optind]);
      note.nr.respnum = 0;
    }
  else if (optind + 3 == argc) /* NF NOTENUM RESPNUM */
    {
      ref = nfref_alloc ();
      parse_single_nf (argv[optind++], ref);
      note.nr.notenum = atoi (argv[optind++]);
      note.nr.respnum = atoi (argv[optind]);
    }
  else
    {
      fprintf (stderr, _("%s: too many arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      teardown ();

      exit (EXIT_FAILURE);
    }

  if (open_nf (ref, &nf) != NEWTS_NO_ERROR)
    {
      nfref_free (ref);
      teardown ();

      error (EXIT_FAILURE, 0, _("error opening notesfile '%s'"),
             nfref_pretty_name (nf.ref));
    }

  if (!(nf.perms & READ) && !(nf.perms & DIRECTOR))
    {
      nfref_free (ref);
      teardown ();

      error (EXIT_FAILURE, 0, _("you are not allowed to read notesfile '%s'"),
             nfref_pretty_name (nf.ref));
    }

  nfref_copy (&note.nr.nfr, nf.ref);
  if (get_note (&note, TRUE) >= 0)
    {
      if (print_header)
        {
          struct passwd *pw = getpwnam (note.auth.name);

          printf (_("Notesfile: %s\n"), nfref_pretty_name (nf.ref));

          printf (_("Note: %d of %d\n"), note.nr.notenum, nf.total_notes);
          if (note.nr.respnum != 0)
            printf (_("Response: %d of %d\n"), note.nr.respnum, note.total_resps);
          printf (_("Title: %s\n"), note.title);
          if (note.director_message)
            printf (_("Director message: %s\n"), note.director_message);

          if (pw)
            {
              char *real = newts_strdup (pw->pw_gecos);
              char *separator;

              if ((separator = strpbrk (real, N_(":,"))) != NULL)
                *separator = '\0';

              printf (_("Author: %s <%s@%s>\n"), real, note.auth.name,
                      note.auth.system);

              newts_free (real);
            }
          else
            printf (_("Author: %s@%s\n"), note.auth.name, note.auth.system);

          putchar ('\n');
        }

      printf (N_("%s"), note.text);
    }
  else
    {
      fprintf (stderr, _("%s: no such note\n"), program_name);

      nfref_free (ref);
      teardown ();

      exit (EXIT_FAILURE);
    }

  nfref_free (ref);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (EXIT_SUCCESS);
}
