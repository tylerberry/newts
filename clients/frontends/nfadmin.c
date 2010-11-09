/*
 * nfadmin.c - command-line interface to director options
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on nfadmin.c from the UIUC notes distribution by Ray Essick
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
#include "error.h"
#include "getopt.h"
#include "newts/uiuc.h"
#include "newts/uiuc-compatibility.h"

#if STDC_HEADERS
# include <limits.h>
#endif

static void summary (struct notesfile *nf);

/* Whether to display debugging messages. */
int debug = FALSE;

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile nf;
  int error_occurred = FALSE;

  int anonymous = 0;
  int archive = 0;
  int locked = 0;
  int moderated = 0;
  int expire_dirmsg = -2;
  int expire_time = -2;
  int expire_action = -2;
  int max_note_size = -2;
  int minimum_notes = -2;

  int i;

  int opt;
  int option_index = 0;

  struct option long_options[] =
    {
      {N_("anonymous"),2,0,'{'},
      {N_("archive"),2,0,'}'},
      {N_("debug"),0,0,'D'},
      {N_("locked"),2,0,'['},
      {N_("moderated"),2,0,']'},
      {N_("open"),2,0,'('},
      {N_("expire-dirmsg"),1,0,'d'},
      {N_("expire-time"),1,0,'e'},
      {N_("expire-action"),1,0,'E'},
      {N_("help"),0,0,'h'},
      {N_("max-note-size"),1,0,'L'},
      {N_("minimum-notes"),1,0,'W'},
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

  /* Rewrite the command line options so that getopt can handle them. */

  for (i=1; i<argc; i++)
    {
      /* This won't catch compound options, but short of calling getopt twice
       * that's not going to happen anyway.
       */

      if (argv[i][0] == '-' &&    /* Only 'tweak' options. */
          (strcmp (argv[i], N_("-N")) == 0 || strcmp (argv[i], N_("-n")) == 0 ||
           strcmp (argv[i], N_("-Q")) == 0 || strcmp (argv[i], N_("-q")) == 0 ||
           strcmp (argv[i], N_("-P")) == 0 || strcmp (argv[i], N_("-p")) == 0 ||
           strcmp (argv[i], N_("-B")) == 0 || strcmp (argv[i], N_("-b")) == 0))
        {
          fprintf (stderr, _("%s: invalid option -- %c\n"), argv[0], argv[i][1]);
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);
          exit (EXIT_FAILURE);
        }
      else if (strcmp (argv[i], N_("-a+")) == 0 || strcmp (argv[i], N_("-a")) == 0)
        {
          argv[i] = N_("-N");
        }
      else if (strcmp (argv[i], N_("-a-")) == 0)
        {
          argv[i] = N_("-n");
        }
      else if (strcmp (argv[i], N_("-M+")) == 0 || strcmp (argv[i], N_("-M")) == 0)
        {
          argv[i] = N_("-Q");
        }
      else if (strcmp (argv[i], N_("-M-")) == 0)
        {
          argv[i] = N_("-q");
        }
      else if (strcmp (argv[i], N_("-o-")) == 0 || strcmp (argv[i], N_("-l")) == 0 ||
               strcmp (argv[i], N_("-l+")) == 0)
        {
          argv[i] = N_("-P");
        }
      else if (strcmp (argv[i], N_("-o+")) == 0 || strcmp (argv[i], N_("-o")) == 0 ||
               strcmp (argv[i], N_("-l-")) == 0)
        {
          argv[i] = N_("-p");
        }
      else if (strcmp (argv[i], N_("-A+")) == 0 || strcmp (argv[i], N_("-A")) == 0)
        {
          argv[i] = N_("-B");
        }
      else if (strcmp (argv[i], N_("-A-")) == 0)
        {
          argv[i] = N_("-b");
        }
    }

  while ((opt = getopt_long (argc, argv, N_("bBd:e:E:hl:nNpPqQW:"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfadmin"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case '{':  /* anonymous */
          if (optarg == NULL || strcmp (optarg, N_("yes")) == 0 ||
              strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("y")) == 0 ||
              strcmp (optarg, N_("+")) == 0)
            anonymous = 1;
          else if (strcmp (optarg, N_("no")) == 0 || strcmp (optarg, N_("off")) == 0 ||
                   strcmp (optarg, N_("n")) == 0 || strcmp (optarg, N_("-")) == 0)
            anonymous = -1;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case '}':  /* archive */
          if (optarg == NULL || strcmp (optarg, N_("yes")) == 0 ||
              strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("y")) == 0 ||
              strcmp (optarg, N_("+")) == 0)
            archive = 1;
          else if (strcmp (optarg, N_("no")) == 0 || strcmp (optarg, N_("off")) == 0 ||
                   strcmp (optarg, N_("n")) == 0 || strcmp (optarg, N_("-")) == 0)
            archive = -1;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case '[':  /* locked */
          if (optarg == NULL || strcmp (optarg, N_("yes")) == 0 ||
              strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("y")) == 0 ||
              strcmp (optarg, N_("+")) == 0)
            locked = 1;
          else if (strcmp (optarg, N_("no")) == 0 || strcmp (optarg, N_("off")) == 0 ||
                   strcmp (optarg, N_("n")) == 0 || strcmp (optarg, N_("-")) == 0)
            locked = -1;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case '(':  /* open -- locked in reverse */
          if (optarg == NULL || strcmp (optarg, N_("yes")) == 0 ||
              strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("y")) == 0 ||
              strcmp (optarg, N_("+")) == 0)
            locked = -1;
          else if (strcmp (optarg, N_("no")) == 0 || strcmp (optarg, N_("off")) == 0 ||
                   strcmp (optarg, N_("n")) == 0 || strcmp (optarg, N_("-")) == 0)
            locked = 1;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case ']':  /* moderated */
          if (optarg == NULL || strcmp (optarg, N_("yes")) == 0 ||
              strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("y")) == 0 ||
              strcmp (optarg, N_("+")) == 0)
            moderated = 1;
          else if (strcmp (optarg, N_("no")) == 0 || strcmp (optarg, N_("off")) == 0 ||
                   strcmp (optarg, N_("n")) == 0 || strcmp (optarg, N_("-")) == 0)
            moderated = -1;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case 'b':
          archive = -1;
          break;

        case 'B':
          archive = 1;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'd':
          if (strcmp (optarg, N_("on")) == 0 || strcmp (optarg, N_("yes")) == 0)
            expire_dirmsg = DIRON;
          else if (strcmp (optarg, N_("off")) == 0 || strcmp (optarg, N_("no")) == 0)
            expire_dirmsg = DIROFF;
          else if (strncmp (optarg, N_("ign"), 3) == 0)
            expire_dirmsg = DIRNOCARE;
          else if (strncmp (optarg, N_("any"), 3) == 0)
            expire_dirmsg = DIRANYON;
          else if (strncmp (optarg, N_("def"), 3) == 0 ||
                   strncmp (optarg, N_("dfl"), 3) == 0)
            expire_dirmsg = DIRDFLT;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case 'e':
          {
            long temp;
        char *tail;

        if (strncmp (optarg, N_("nev"), 3) == 0 || strcmp (optarg, N_("n")) == 0)
              expire_time = -1;
        else if (strncmp (optarg, N_("def"), 3) == 0 ||
                 strcmp (optarg, N_("d")) == 0 ||
                 strncmp (optarg, N_("dfl"), 3) == 0)
              expire_time = 0;
            else
              {
                temp = strtol (optarg, &tail, 10);
                if (temp == 0 && optarg == tail)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else if ((temp == LONG_MAX && errno == ERANGE) || temp > INT_MAX)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s (exceeded maximum value)\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else if (temp < 0)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s (no negative values)\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else
                  {
                    expire_time = temp;
                  }
              }

            break;
          }

        case 'E':
          if (strncmp (optarg, N_("arc"), 3) == 0)
            expire_action = KEEPYES;
          else if (strncmp (optarg, N_("del"), 3) == 0)
            expire_action = KEEPNO;
          else if (strncmp (optarg, N_("def"), 3) == 0 ||
                   strncmp (optarg, N_("dfl"), 3) == 0)
            expire_action = KEEPDFLT;
          else
            {
              fprintf (stderr, _("%s: invalid argument -- %s\n"),
                       argv[0], optarg);
              fprintf (stderr, _("Try '%s --help' for more information.\n"),
                       program_name);

              teardown ();

              exit (EXIT_FAILURE);
            }
          break;

        case 'f':
          break;

        case 'L':
          {
            long temp;
        char *tail;

            temp = strtol (optarg, &tail, 10);
            if (strcmp (tail, N_("k")) == 0 || strcmp (tail, N_("K")) == 0)
              temp *= 1024;
            if (temp == 0 && optarg == tail)
              {
                fprintf (stderr, _("%s: invalid argument -- %s\n"),
                         argv[0], optarg);
                fprintf (stderr, _("Try '%s --help' for more information.\n"),
                         program_name);

                teardown ();

                exit (EXIT_FAILURE);
              }
            else if ((temp == LONG_MAX && errno == ERANGE) || temp > HARDMAX)
              {
                fprintf (stderr, _("%s: invalid argument -- %s (exceeded maximum value)\n"),
                         argv[0], optarg);
                fprintf (stderr, _("Try '%s --help' for more information.\n"),
                         program_name);

                teardown ();

                exit (EXIT_FAILURE);
              }
            else if (temp < 0)
              {
                fprintf (stderr, _("%s: invalid argument -- %s (no negative values)\n"),
                         argv[0], optarg);
                fprintf (stderr, _("Try '%s --help' for more information.\n"),
                         program_name);

                teardown ();

                exit (EXIT_FAILURE);
              }
            else
              {
                max_note_size = temp;
              }

            break;
          }

        case 'n':
          anonymous = -1;
          break;

        case 'N':
          anonymous = 1;
          break;

        case 'p':
          locked = -1;
          break;

        case 'P':
          locked = 1;
          break;

        case 'q':
          moderated = -1;
          break;

        case 'Q':
          moderated = 1;
          break;

        case 'W':
          {
            long temp;
            char *tail;

            if (strncmp (optarg, N_("def"), 3) == 0 ||
                strcmp (optarg, N_("d")) == 0 ||
                strncmp (optarg, N_("dfl"), 3) == 0)
              expire_time = 0;
            else
              {
                temp = strtol (optarg, &tail, 10);
                if (temp == 0 && optarg == tail)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else if ((temp == LONG_MAX && errno == ERANGE) || temp > INT_MAX)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s (exceeded maximum value)\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else if (temp < 0)
                  {
                    fprintf (stderr, _("%s: invalid argument -- %s (no negative values)\n"),
                             argv[0], optarg);
                    fprintf (stderr, _("Try '%s --help' for more information.\n"),
                             program_name);

                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else
                  {
                    minimum_notes = temp;
                  }
              }

            break;
          }

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                    "Adjust director options for the specified NOTESFILE(s).\n\n"),
                  program_name);

          printf (_("General options:\n"
                    "  -h, --help                Display this help and exit\n"
                    "      --debug               Display debugging messages\n"
                    "      --version             Display version information and exit\n\n"));

          printf (_("Value options:\n"
                    "  -d, --expire-dirmsg=VAL   Set expiration by director message flag\n"
                    "  -e, --expire-time=VAL     Set number of days before note expiration\n"
                    "  -E, --expire-action=VAL   Set action to take upon expiration\n"
                    "  -L, --max-note-size=VAL   Set maximum note size in bytes or kilobytes (-k)\n"
                    "  -W, --minimum-notes=VAL   Set minimum notes to retain after archiving\n\n"));

          printf (_("Flag options:\n"
                    "  -a, --anonymous           Set value of 'anonymous' flag\n"
                    "  -A, --archive             Set value of 'archive' flag\n"
                    "  -l, --locked              Set value of 'locked' flag\n"
                    "  -M, --moderated           Set value of 'moderated' flag\n"
                    "  -o, --open                Set value of 'locked' flag backwards\n\n"));

          printf (_("Value options can be set to whatever the various options allow; for details,\n"
                    "see 'info nfadmin'. Flag options can be set with + or - following the short\n"
                    "options, or 'on', 'off', 'yes', and 'no' for the long options.\n\n"));

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

  /* TRANSLATORS: Please keep this aligned as is; I know that's annoying but the
   * layout is fussy.
   */
  printf (_("        Notesfile         : Lock Anon Arch Modr Minm Expr Expr Expr   #    Maxm\n"
            "------------------------- : -------Flags------- Note Time Actn DirM Notes  NtSz\n"));

  while (optind < argc)
    parse_nf (argv[optind++], &nflist);

  {
    ListNode *node = list_head (&nflist);

    while (node != NULL)
      {
        int error;
        struct uiuc_opts *opts;

        error = open_nf ((newts_nfref *) list_data (node), &nf);

        node = list_next (node);

        if (error != NEWTS_NO_ERROR)
          {
            printf (_("%-25.25s : Error opening notesfile\n"),
                    nfref_pretty_name (nf.ref));
            error_occurred = TRUE;
            continue;
          }

        if (!allowed (&nf, DIRECTOR))
          {
            printf (_("%-25.25s : Not a director\n"),
                    nfref_pretty_name (nf.ref));
            continue;
          }

        opts = (struct uiuc_opts *) nf.opts;

        if (anonymous)
          {
            if (anonymous > 0)
              nf.options |= NF_ANONYMOUS;
            else
              nf.options &= ~NF_ANONYMOUS;
          }

        if (locked)
          {
            if (locked > 0)
              nf.options |= NF_LOCKED;
            else
              nf.options &= ~NF_LOCKED;
          }

        if (archive)
          {
            if (archive > 0)
              nf.options |= NF_ARCHIVE;
            else
              nf.options &= ~NF_ARCHIVE;
          }

        if (moderated)
          {
            if (moderated > 0)
              nf.options |= NF_MODERATED;
            else
              nf.options &= ~NF_MODERATED;
          }

        if (expire_action != -2)
          opts->expire_action = expire_action;

        if (expire_dirmsg != -2)
          opts->expire_by_dirmsg = expire_dirmsg;

        if (expire_time != -2)
          opts->expire_threshold = expire_time;

    if (minimum_notes != -2)
          opts->minimum_notes = minimum_notes;

        if (max_note_size != -2)
          opts->maximum_note_size = max_note_size;

        modify_nf (&nf);

        summary (&nf);
      }
  }

  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (error_occurred ? EXIT_FAILURE : EXIT_SUCCESS);
}

static void
summary (struct notesfile *nf)
{
  struct uiuc_opts *opts = (struct uiuc_opts *) nf->opts;

  printf (N_("%-25.25s : "), nfref_pretty_name (nf->ref));
  printf (N_("%s %s %s %s "),
          nf->options & NF_LOCKED ? _("YES ") : _(" NO "),
          nf->options & NF_ANONYMOUS ? _("YES ") : _(" NO "),
          nf->options & NF_ARCHIVE ? _("YES ") : _(" NO "),
          nf->options & NF_MODERATED ? _("YES ") : _(" NO "));
  printf (N_("%4d "), opts->minimum_notes);
  if (opts->expire_threshold == -1)
    printf (_("NEVR "));
  else if (opts->expire_threshold == 0)
    printf (_("DFLT "));
  else
    printf (N_("%4d "), opts->expire_threshold);
  printf (N_("%s "), opts->expire_action == KEEPNO ? _("DEL ") :
          opts->expire_action == KEEPYES ? _("ARCH") : _("DFLT"));
  printf (N_("%s "), opts->expire_by_dirmsg == DIRON ? _(" ON ") :
          opts->expire_by_dirmsg == DIROFF ? _("OFF ") :
          opts->expire_by_dirmsg == DIRANYON ? _("ANY ") :
          opts->expire_by_dirmsg == DIRNOCARE ? _("IGNR") : _("DFLT"));
  printf (N_("%5d "), nf->total_notes);
  printf (N_("%4dk\n"), opts->maximum_note_size / 1024);

  return;
}
