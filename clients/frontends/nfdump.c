/*
 * nfdump.c - dump a notesfile to disk
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
#include "dump-uiuc.h"

/* How much detail to print out. */
int debug = FALSE;
int verbose = FALSE;

/* What extension to use for the created image files. */
char *extension = NULL;

int dump_nf (struct notesfile *nf);

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile nf;

  int opt;
  int option_index = 0;
  int error_occurred = FALSE;

  struct option long_options[] =
    {
      {N_("debug"),0,0,'D'},
      {N_("extension"),1,0,'e'},
      {N_("verbose"),0,0,'v'},
      {N_("help"),0,0,'h'},
      {N_("version"),0,0,0},
      {0,0,0,0}
    };

  memset (&nf, 0, sizeof (struct notesfile));

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
  extension = newts_strdup (N_("dump"));

  while ((opt = getopt_long (argc, argv, N_("e:hv"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfdump"));

            newts_free (extension);
            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'D':
          debug = TRUE;
          break;

        case 'e':
          newts_free (extension);
          extension = newts_strdup (optarg);
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                  "Create a saved image file for each NOTESFILE.\n\n"), program_name);

          printf (_("  -e, --extension=EXT   Specify an extension for the images\n"
                    "  -v, --verbose         Display extra status messages\n"
                    "      --debug           Display debugging messages\n\n"
                    "  -h, --help            Display this help and exit\n"
                    "      --version         Display version information and exit\n\n"));

          printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);

          newts_free (extension);
          teardown ();

          if (fclose (stdout) == EOF)
            error (EXIT_FAILURE, errno, _("error writing output"));
          exit (EXIT_SUCCESS);

        case '?':
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          newts_free (extension);
          teardown ();

          exit (EXIT_FAILURE);
        }
    }

  if (optind == argc)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      newts_free (extension);
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
        int error;

        error = open_nf ((newts_nfref *) list_data (node), &nf);

        node = list_next (node);

        if (error != NEWTS_NO_ERROR)
          {
            printf (_("%s: couldn't open notesfile '%s'\n"), program_name,
                    nfref_pretty_name (nf.ref));
            error_occurred = TRUE;
            continue;
          }

        if (!(nf.perms & READ) && !(nf.perms & DIRECTOR))
          {
            printf (_("%s: you are not allowed to read notesfile '%s'\n"),
                    program_name, nfref_pretty_name (nf.ref));
            error_occurred = TRUE;
            continue;
          }

        switch (dump_nf (&nf))
          {
          case 0:
            if (verbose)
              printf (_("%s: dumped notesfile '%s'\n"), program_name,
                      nfref_pretty_name (nf.ref));
            break;

          case -1:
          default:
            {
              fprintf (stderr, _("%s: error dumping '%s'\n"), program_name,
                       nfref_pretty_name (nf.ref));
              error_occurred = TRUE;
            }
            break;
          }
      }
  }

  free (extension);
  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}

int
dump_nf (struct notesfile *nf)
{
  return uiuc_dump_nf (nf);
}
