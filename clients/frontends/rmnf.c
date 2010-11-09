/*
 * rmnf.c - delete one or more notesfiles
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
#include "yesno.h"

/* Whether to display debugging messages. */
int debug = FALSE;

int
main (int argc, char **argv)
{
  List nflist;

  int error_occurred = FALSE;
  int opt;
  int option_index = 0;
  short force = FALSE;
  short verbose = FALSE;

  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {"debug",0,0,'D'},
      {"force",0,0,'f'},
      {"verbose",0,0,'v'},
      {"help",0,0,'h'},
      {"version",0,0,0},
      {0,0,0,0}
    };

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

  while ((opt = getopt_long (argc, argv, "fhv",
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("rmnf"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'D':
          debug = TRUE;
          break;

        case 'f':
          force = TRUE;
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                    "Delete existing NOTESFILE(s).\n\n"), program_name);

          printf (_("  -f, --force     Do not prompt to confirm removal\n"
                    "  -v, --verbose   Display a message for each notesfile processed\n"
                    "      --debug     Display debugging messages\n\n"
                    "  -h, --help      Display this help and exit\n"
                    "      --version   Display version information and exit\n\n"));

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

  if (optind == argc)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      teardown ();

      exit (EXIT_FAILURE);
    }

  list_init (&nflist,
             (void * (*) (void)) nfref_alloc,
             (void (*) (void *)) nfref_free,
             NULL);

  while (optind < argc)
    parse_nf (argv[optind++], &nflist);

  {
    ListNode *node = list_head (&nflist);

    while (node != NULL)
      {
        int result;
        struct notesfile nf;
        newts_nfref *ref = (newts_nfref *) list_data (node);

        memset (&nf, 0, sizeof (struct notesfile));

        /* Automatically adjust to private notesfiles if you are not notes. */

        if (euid != notes_uid)
          {
            if (nfref_owner (ref) == NULL)
              nfref_set_owner (ref, username);
          }

        node = list_next (node);

        result = open_nf (ref, &nf);

        if (result != NEWTS_NO_ERROR)
          {
            printf (_("%s: notesfile '%s' doesn't exist\n"), program_name,
                    nfref_pretty_name (ref));
            continue;
          }

        close_nf (&nf, FALSE);

        if (!force)
          {
            printf (_("%s: remove '%s'? (y/n) "), program_name,
                    nfref_pretty_name (ref));
            if (!yesno ())
              continue;
          }

        switch (delete_nf (ref))
          {
          case 0:
            if (verbose)
              printf (_("%s: removing notesfile '%s'\n"), program_name,
                      nfref_pretty_name (ref));
            break;

          case -1:
          default:
            fprintf (stderr, _("%s: error removing notesfile '%s'\n"),
                     program_name, nfref_pretty_name (ref));
            error_occurred = TRUE;
            break;
          }
      }
  }

  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}
