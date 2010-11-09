/*
 * mknf.c - create new notesfiles
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

/* Whether to display debugging messages. */
int debug = FALSE;

int
main (int argc, char **argv)
{
  List nflist;
  int flags = 0;
  int verbose = FALSE;
  int error_occurred = FALSE;

  int opt;
  int option_index = 0;

  struct option long_options[] =
    {
      {N_("anonymous"),0,0,'a'},
      {N_("closed"),0,0,'c'},
      {N_("debug"),0,0,'D'},
      {N_("locked"),0,0,'l'},
      {N_("moderated"),0,0,'m'},
      {N_("networked"),0,0,'n'},
      {N_("open"),0,0,'o'},
      {N_("unlocked"),0,0,'u'},
      {N_("verbose"),0,0,'v'},
      {N_("help"),0,0,'h'},
      {N_("version"),0,0,0},
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

  while ((opt = getopt_long (argc, argv, N_("achlmnouv"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("mknf"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'a':
          flags |= NF_ANONYMOUS;
          break;

        case 'c':
        case 'l':
          flags |= NF_LOCKED;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'm':
          flags |= NF_MODERATED;
          break;

        case 'n':      /* For compatibility with the old UIUC notes mknf. */
          break;

        case 'o':
        case 'u':
          flags &= ~NF_LOCKED;
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                  "Create new NOTESFILE(s).\n\n"), program_name);

          printf (_("  -a, --anonymous   Allow anonymous notes in new notesfile\n"
                    "  -l, --locked      Create a locked notesfile\n"
                    "  -u, --unlocked    Create an unlocked notesfile (default)\n"
                    "  -v, --verbose     Display a message for each notesfile created\n"
                    "      --debug       Display debugging messages\n\n"
                    "  -h, --help        Display this help and exit\n"
                    "      --version     Display version information and exit\n\n"));

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
        newts_nfref *ref = (newts_nfref *) list_data (node);

        if (euid != notes_uid)
          {
            verbose = TRUE;
            nfref_set_owner (ref, username);
          }

        switch (create_nf (ref, flags))
          {
          case 0:
            if (verbose)
              printf (_("%s: created notesfile '%s'\n"), program_name,
                      nfref_pretty_name (ref));
            break;

          case -2:
            {
              fprintf (stderr, _("%s: error creating '%s': Already exists\n"),
                       program_name, nfref_pretty_name (ref));
              error_occurred = TRUE;
            }
            break;

          case -1:
          default:
            {
              fprintf (stderr, _("%s: error creating '%s'\n"), program_name,
                       nfref_pretty_name (ref));
              error_occurred = TRUE;
            }
            break;
          }

        node = list_next (node);
      }
  }

  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}
