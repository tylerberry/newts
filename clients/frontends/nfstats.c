/*
 * nfstats.c - get usage statistics for a notesfile
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
 * You shouu have received a copy of the GNU General Public License along with
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

  int summary = FALSE;
  int error_occurred = FALSE;
  struct stats *total_stats = stats_alloc ();
  int processed = 0;

  int opt;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {"debug",0,0,'D'},
      {"summary",0,0,'s'},
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

  while ((opt = getopt_long (argc, argv, "sh",
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfstats"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'D':
          debug = TRUE;
          break;

        case 's':
          summary = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                    "Display usage statistics for NOTESFILE(s), including a total.\n\n"),
                  program_name);

          printf (_("  -s, --summary   Print only the total for all listed notesfiles\n"
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
    struct stats *stats = stats_alloc ();

    while (node != NULL && !error_occurred)
      {
        newts_nfref *ref = (newts_nfref *) list_data (node);
        int result = get_stats (ref, stats);

        if (result != NEWTS_NO_ERROR)
          {
            fprintf (stderr, _("%s: error getting stats for '%s'\n"),
                     program_name, nfref_pretty_name (ref));
            error_occurred = TRUE;
            break;
          }

        if (!summary)
          {
            if (processed) printf ("\n");
            printf (_("Usage statistics for %s\n"),
                    nfref_pretty_name (ref));
            printf (_("                         NOTES   RESPS  TOTALS\n"));
            printf (_("Local Reads:           %7u %7u %7u\n"),
                    stats->notes_read, stats->resps_read,
                    (stats->notes_read + stats->resps_read));
            printf (_("Local Writes:          %7u %7u %7u\n"),
                    (stats->notes_written - stats->notes_received),
                    (stats->resps_written - stats->resps_received),
                    (stats->notes_written + stats->resps_written -
                     stats->notes_received - stats->resps_received));
            printf (_("Entries into Notesfile:  %u\n"), stats->entries);
            printf (_("Total Time in Notesfile: %.2f minutes\n"),
                    ((float) stats->total_time / 60.0));
            if (stats->entries)
              printf (_("Average Time/Entry:      %.2f minutes\n"),
                      (((float) stats->total_time / 60.0) /
                       (float) stats->entries));
          }

        stats_accumulate (stats, total_stats);

        processed++;

        node = list_next (node);
      }

    stats_free (stats);
  }

  if (processed && (summary || processed != 1) && !error_occurred)
    {
      if (!summary) printf ("\n");
      printf (_("Total for all requested notesfiles\n"));
      printf (_("                          NOTES   RESPS  TOTALS\n"));
      printf (_("Local Reads:            %7u %7u %7u\n"),
              total_stats->notes_read, total_stats->resps_read,
              (total_stats->notes_read + total_stats->resps_read));
      printf (_("Local Writes:           %7u %7u %7u\n"),
              (total_stats->notes_written - total_stats->notes_received),
              (total_stats->resps_written - total_stats->resps_received),
              (total_stats->notes_written + total_stats->resps_written -
               total_stats->notes_received - total_stats->resps_received));
      printf (_("Entries into Notesfiles:  %u\n"), total_stats->entries);
      printf (_("Total Time in Notesfiles: %.2f minutes\n"),
              ((float) total_stats->total_time / 60.0));
      if (total_stats->entries)
        printf (_("Average Time/Entry:       %.2f minutes\n"),
                (((float) total_stats->total_time / 60.0)
                 / (float) total_stats->entries));
    }

  stats_free (total_stats);

  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}
