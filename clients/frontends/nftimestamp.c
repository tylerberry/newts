/*
 * nftimestamp.c - update the sequencer times for the specified notesfiles
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
#include "getdate.h"
#include "getopt.h"

/* Whether to display debugging messages. */
int debug = FALSE;

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile nf;

  int fileflag = FALSE;
  int error_occurred = FALSE;
  int verbose = FALSE;
  char *seqname;
  char *subseq = NULL;

  time_t seqtime;
  time_t truetime;

  int opt;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {"alternate",1,0,'a'},
      {"debug",0,0,'D'},
      {"file",1,0,'f'},
      {"time",1,0,'o'},
      {"user",1,0,'u'},
      {"verbose",0,0,'v'},
      {"help",0,0,'h'},
      {"version",0,0,0},
      {0,0,0,0}
    };

  memset (&nf, 0, sizeof (struct notesfile));

  time (&seqtime);
  time (&truetime);

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

  seqname = newts_strdup (username);
  list_init (&nflist,
             (void * (*) (void)) nfref_alloc,
             (void (*) (void *)) nfref_free,
             NULL);

  while ((opt = getopt_long (argc, argv, "a:f:ho:u:v",
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nftimestamp"));

            newts_free (seqname);
            if (subseq)
              newts_free (subseq);
            list_destroy (&nflist);
            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'a':
          newts_nrealloc (seqname, strlen (username) + strlen (optarg) + 2,
                          sizeof (char));
          strcpy (seqname, username);
          strcat (seqname, ":");
          strcat (seqname, optarg);
          subseq = newts_strdup (optarg);
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'f':
          if (parse_file (optarg, &nflist))
            fileflag = TRUE;
          break;

        case 'o':
          {
            struct timespec result, now;
            now.tv_sec = truetime;

            if (get_date (&result, optarg, &now))
              {
                if (result.tv_sec <= truetime)
                  seqtime = result.tv_sec;
                else
                  {
                    fprintf (stderr, _("%s: parsed time '%s' in the future\n"),
                             program_name, optarg);
                    fprintf (stderr, _("See 'info newts' for more information.\n"));
                    exit (EXIT_FAILURE);
                  }
              }
            else
              {
                fprintf (stderr, _("%s: error parsing time '%s'\n"),
                         program_name, optarg);
                exit (EXIT_FAILURE);
              }

            break;
          }

        case 'u':
          if (notes_uid == euid)
            {
              if (subseq != NULL)
                {
                  newts_nrealloc (seqname, strlen (subseq) + strlen (optarg) + 2,
                                  sizeof (char));
                  strcpy (seqname, optarg);
                  strcat (seqname, ":");
                  strcat (seqname, subseq);
                }
              else
                {
                  newts_free (seqname);
                  seqname = newts_strdup (optarg);
                }
            }
          else
            {
              fprintf (stderr, _("%s: only user '%s' can use -u option\n"),
                       program_name, NOTES);
              fprintf (stderr, _("See 'info nftimestamp' for more information.\n"));

              newts_free (seqname);
              if (subseq)
                newts_free (subseq);
              list_destroy (&nflist);
              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                    "Update sequencer times for specified NOTESFILE(s).\n\n"),
                  program_name);

          printf (_("  -a, --alternate=SEQ   Use alternate sequencer SEQ\n"
                    "  -f, --file=FILE       Read list of notesfiles to update from specified file\n"
                    "  -o, --time=TIME       Use specified time (default is now)\n"
                    "  -u, --user=NAME       Set specified user's sequencer times (Notes owner only)\n"
                    "  -v, --verbose         Display each notesfile with new notes\n"
                    "      --debug           Display debugging messages\n\n"
                    "  -h, --help            Display this help and exit\n"
                    "      --version         Display version information and exit\n\n"));

          printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);

          newts_free (seqname);
          if (subseq)
            newts_free (subseq);
          list_destroy (&nflist);
          teardown ();

          if (fclose (stdout) == EOF)
            error (EXIT_FAILURE, errno, _("error writing output"));
          exit (EXIT_SUCCESS);

        case '?':
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          newts_free (seqname);
          if (subseq)
            newts_free (subseq);
          list_destroy (&nflist);
          teardown ();

          exit (EXIT_FAILURE);
        }
    }

  if (optind == argc && !fileflag)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      newts_free (seqname);
      if (subseq)
        newts_free (subseq);
      list_destroy (&nflist);
      teardown ();

      exit (EXIT_FAILURE);
    }

  while (optind < argc)
    parse_nf (argv[optind++], &nflist);

  {
    ListNode *node = list_head (&nflist);

    while (node != NULL)
      {
        int result = open_nf ((newts_nfref *) list_data (node), &nf);

        node = list_next (node);

        if (result != NEWTS_NO_ERROR)
          {
            fprintf (stderr, _("%s: error opening '%s'\n"), program_name,
                     nfref_pretty_name (nf.ref));
            error_occurred = TRUE;
            continue;
          }

        if (allowed (&nf, READ))
          {
            set_seqtime (nf.ref, seqname, seqtime);

            if (verbose)
              {
                printf (_("%s: timestamp set\n"), nfref_pretty_name (nf.ref));
              }
          }
        else
          {
            printf (_("%s: timestamp not set: no read access\n"),
                    nfref_pretty_name (nf.ref));
          }
      }
  }

  newts_free (seqname);
  if (subseq)
    newts_free (subseq);
  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}
