/*
 * checknotes.c - check sequencer to see if there are any new notes
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on checknotes.c from the UIUC notes distribution by Ray Essick
 * and Rob Kolstad.  Any work derived from this source code is required to
 * retain this notice.
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
#include "getopt.h"
#include "strtok_r.h"

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

enum
  {
    NORMAL,
    SILENT,
    VERBOSE
  };

/* Whether to display debugging messages. */
int debug = FALSE;

/* Whether or not to ignore the blacklist. */
int no_blacklist = FALSE;

/* The sequencer time we're dealing with here. */
time_t seqtime;

/* Placeholder for the blacklist code. */
const short white_basenotes = FALSE;

static int verify_sequencer (struct notesfile *nf);

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile *nf;

  int fileflag = FALSE;
  int updated = 0;
  short verbosity = NORMAL;
  char *seqname;

  int opt;
  int option_index = 0;

  struct option long_options[] =
    {
      {N_("alternate"),1,0,'a'},
      {N_("debug"),0,0,'D'},
      {N_("file"),1,0,'f'},
      {N_("quiet"),0,0,'s'},
      {N_("silent"),0,0,'s'},
      {N_("verbose"),0,0,'v'},
      {N_("no-blacklist"),0,0,'z'},
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
  seqname = newts_strdup (username);
  list_init (&nflist,
             (void * (*) (void)) nfref_alloc,
             (void (*) (void *)) nfref_free,
             NULL);

  while ((opt = getopt_long (argc, argv, N_("a:f:hnqsv"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("checknotes"));

            newts_free (seqname);
            list_destroy (&nflist);
            teardown ();

            if (fclose (stdout) == EOF)
              error (1, errno, _("error writing output"));
            exit (0);
          }

        case 'a':
          newts_nrealloc (seqname, strlen (username) + strlen (optarg) + 2,
                          sizeof (char));
          strcpy (seqname, username);
          strcat (seqname, ":");
          strcat (seqname, optarg);
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'f':
          if (parse_file (optarg, &nflist))
            fileflag = TRUE;
          break;

        case 'n':
          verbosity = NORMAL;
          break;

        case 'q':
          verbosity = NORMAL;
          break;

        case 's':
          verbosity = SILENT;
          break;

        case 'v':
          verbosity = VERBOSE;
          break;

        case 'z':
          no_blacklist = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... [NOTESFILE]...\n"
                    "Check for new notes in your sequencer path or in listed notesfiles.\n\n"),
                    program_name);

          printf (_("  -a, --alternate=SEQ   Use alternate sequencer SEQ\n"
                    "  -f, --file=FILE       Read list of notesfiles to view from specified file\n"
                    "  -s, --silent          Display no output\n"
                    "  -v, --verbose         Display each notesfile with new notes\n"
                    "  -z, --no-blacklist    Ignore blacklist while checking notesfiles.\n"
                    "      --debug           Display debugging messages\n"
                    "      --quiet           Same as -s\n\n"
                    "  -h, --help            Display this help and exit\n"
                    "      --version         Display version information and exit\n\n"));

          printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);

          newts_free (seqname);
          list_destroy (&nflist);
          teardown ();

          if (fclose (stdout) == EOF)
            error (1, errno, _("error writing output"));

          exit (1); /* Because exit value 0 is significant. */

        case '?':
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          newts_free (seqname);
          list_destroy (&nflist);
          teardown ();

          if (fclose (stdout) == EOF)
            error (1, errno, _("error writing output"));

          exit (1);
        }
    }

  if (optind == argc && !fileflag)
    {
      char *copy, *list, *token, *nfseq;

      nfseq = getenv ("NFSEQ");
      if (nfseq == NULL)
        {
          fprintf (stderr, _("%s: NFSEQ environment variable not set\n"),
                   program_name);
          fprintf (stderr, _("See 'info newts' for more information.\n"));
          exit (1);
        }
      copy = newts_strdup (nfseq);

      token = strtok_r (copy, ", ", &list);
      while (token != NULL)
        {
          parse_nf (token, &nflist);
          token = strtok_r (NULL, ", ", &list);
        }

      newts_free (copy);
    }
  else
    {
      while (optind < argc)
        parse_nf (argv[optind++], &nflist);
    }

  {
    ListNode *node = list_head (&nflist);

    while (node != NULL)
      {
        int error = open_nf ((newts_nfref *) list_data (node), nf);

        node = list_next (node);

        if (error != NEWTS_NO_ERROR)
          {
            fprintf (stderr, _("%s: error opening '%s'\n"), program_name,
                     nfref_pretty_name (nf->ref));
            continue;
          }

        get_seqtime (nf->ref, seqname, &seqtime);

        if (difftime (seqtime, nf->modified) <= 0 &&
            verify_sequencer (nf))
          {
            updated++;
            if (verbosity == VERBOSE)
              {
                printf (N_("%s\n"), nfref_pretty_name (nf->ref));
              }
          }
      }
  }

  if (verbosity == NORMAL)
    {
      if (updated)
        printf (_("There are new notes\n"));
      else
        printf (_("There are no new notes\n"));
    }

  newts_free (seqname);
  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (1, errno, _("error writing output"));

  exit (updated ? 0 : 1);
}

/* verify_sequencer - look through a notesfile with the sequencer to see if it
 * really has new notes, with our blacklist taken into effect.
 *
 * Returns: TRUE if the notesfile really has new notes, FALSE otherwise.
 */

static int
verify_sequencer (struct notesfile *nf)
{
  struct newt note;
  memset (&note, 0, sizeof (struct newt));
  nfref_copy (&note.nr.nfr, nf->ref);
  note.nr.notenum = 0;
  note.nr.respnum = 0;

  note.nr.notenum = get_next_note (&note.nr, seqtime);

  if (note.nr.notenum < 0)
    return FALSE;

  while (TRUE)
    {
      get_note (&note, FALSE);

      if (difftime (note.modified, seqtime) > 0 &&
          (difftime (note.created, seqtime) > 0 || note.nr.respnum != 0))
        {
          if (!(euid == note.auth.uid && strcmp (fqdn, note.auth.system) == 0)
              && !blacklisted (&note))
            return TRUE;
        }

      if (note.nr.respnum != note.total_resps)
        {
          if ((note.nr.respnum = get_next_resp (&note.nr, seqtime)) > 0)
            continue;
        }

      note.nr.respnum = 0;
      if ((note.nr.notenum = get_next_note (&note.nr, seqtime)) > 0)
        continue;
      else
        return FALSE;
    }
}
