/*
 * notes.c - main routine for legacy notesfile interface
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2006 Tyler Berry.
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
#include "signals.h"

#include "curses_wrapper.h"
#include "dirname.h"
#include "getdate.h"
#include "getopt.h"
#include "strtok_r.h"
#include "vasprintf.h"

#if HAVE_LANGINFO_H
# include <langinfo.h>
#endif

#if HAVE_LOCALE_H
# include <locale.h>
#endif

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* FIXME: should be removed. */

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* Alternate sequencer name flag */
int alt_sequencer = FALSE;

/* Alternate sequencer name */
char *alt_seqname;

/* Alternate sequencer time flag */
int alt_time = FALSE;

/* Controls whether to skip entire threads with blacklisted basenotes */
int black_skip_seq = FALSE;

/* Controls whether to print out debug info to stderr. */
int debug = FALSE;

/* The name the program was called */
char *program_name;

/* Controls whether to use the blacklist */
int no_blacklist = FALSE;

/* Controls whether or not to append signatures to new notes/resps */
int signature = TRUE;

/* Original sequencer time */
time_t orig_seqtime;

/* Controls whether user's posts trip the sequencer */
int seq_own_notes = FALSE;

/* Sequencer time */
time_t seqtime;

/* If and what kind of sequencer we're using */
int sequencer = NONE;

/* Controls whether we want an old-school look */
int traditional = FALSE;

/* Controls whether to ignore blacklisting for basenotes */
int white_basenotes = FALSE;

int
main (int argc, char **argv)
{
  List nflist;

  short file_flag = FALSE;
  short quit_flag = FALSE;
  int result;

  newts_failed_malloc_hook = curses_malloc_die;

  srand ((unsigned) time (NULL));

  time (&orig_seqtime);
  time (&seqtime);

#ifdef __GLIBC__
  program_name = program_invocation_short_name;
#else
  program_name = base_name (argv[0]);
#endif

  /* Initialize i18n. */

#ifdef HAVE_SETLOCALE
  if (setlocale (LC_ALL, "") == NULL)
    {
      fprintf (stderr, _("%s: could not determine your locale\nCheck the "
                         "environment variables LANG, LC_ALL, etc.\n"),
               program_name);

      exit (EXIT_FAILURE);
    }
#endif

#if ENABLE_NLS
  bindtextdomain (PACKAGE, LOCALEDIR);
  textdomain (PACKAGE);
#endif

  /* Initial setup and global variable init. */

  init_blacklist ();
  list_init (&nflist,
             (void * (*) (void)) nfref_alloc,
             (void (*) (void *)) nfref_free,
             NULL);

  setup ();

  while (1)
    {
      int opt;
      short override_flag = FALSE;

      static struct option long_options[] =
        {
          {"alternate",       required_argument, 0, 'a'},
          {"black-threads",   no_argument,       0, 'b'},
          {"debug",           no_argument,       0, 'D'},
          {"seq-own-notes",   no_argument,       0, 'e'},
          {"file",            required_argument, 0, 'f'},
          {"no-signature",    no_argument,       0, 'g'},
          {"index",           no_argument,       0, 'i'},
          {"skip-own-notes",  no_argument,       0, 'k'},
          {"modern",          no_argument,       0, 'm'},
          {"no-sequencer",    no_argument,       0, 'n'},
          {"time",            required_argument, 0, 'o'},
          {"sequencer",       no_argument,       0, 's'},
          {"traditional",     no_argument,       0, 't'},
          {"user",            required_argument, 0, 'u'},
          {"white-basenotes", no_argument,       0, 'w'},
          {"extended",        no_argument,       0, 'x'},
          {"imsa",            no_argument,       0, 'y'},
          {"no-blacklist",    no_argument,       0, 'z'},
          {"help",            no_argument,       0, 'h'},
          {"version",         no_argument,       0, 0},
          {0, 0, 0, 0}
        };

      opt = getopt_long (argc, argv, "a:bDef:ghikmno:stu:wxz",
                         long_options, NULL);

      if (opt == -1)
        break;

      switch (opt)
        {
        case 0:
          {
            char *version_string;

            asprintf (&version_string, _("revision: %s"), newts_revision);
            printf (N_("notes - %s %s (%s)\n"), PACKAGE_NAME, VERSION,
                    version_string);

            free (version_string);

            list_destroy (&nflist);
            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));

            exit (EXIT_SUCCESS);
          }

        case 'a':
          {
            alt_sequencer = TRUE;

            if (sequencer == NONE)
              sequencer = SEQUENCER;

            newts_nrealloc (seqname, strlen (username) + strlen (optarg) + 2,
                            sizeof (char));
            strcpy (seqname, username);
            strcat (seqname, ":");
            strcat (seqname, optarg);

            if (alt_seqname)
              newts_free (alt_seqname);
            alt_seqname = newts_strdup (optarg);

            break;
          }

        case 'b':
          black_skip_seq = TRUE;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'f':
          if (parse_file (optarg, &nflist))
            file_flag = TRUE;
          break;

        case 'g':
          signature = FALSE;
          break;

        case 'i':
          sequencer = INDEX;
          break;

        case 'k':
          seq_own_notes = FALSE;
          override_flag = TRUE;
          break;

        case 'm':
          seq_own_notes = TRUE;
          break;

        case 'n':
          sequencer = NONE;
          break;

        case 'o':
          {
            struct timespec parsed_time, now;
            now.tv_sec = orig_seqtime;

            if (get_date (&parsed_time, optarg, &now))
              {
                if (alt_time)
                  {
                    fprintf (stderr, _("%s: cannot specify multiple alternate times\n"),
                             program_name);
                    fprintf (stderr, _("See 'info newts' for more information.\n"));

                    list_destroy (&nflist);
                    teardown ();

                    exit (EXIT_FAILURE);
                  }
                else if (parsed_time.tv_sec <= orig_seqtime)
                  {
                    orig_seqtime = seqtime = parsed_time.tv_sec;
                    if (sequencer == NONE)
                      sequencer = SEQUENCER;
                    alt_time = TRUE;
                  }
                else
                  {
                    fprintf (stderr, _("%s: parsed time '%s' in the future\n"),
                             program_name, optarg);
                    fprintf (stderr, _("See 'info newts' for more information.\n"));

                    list_destroy (&nflist);
                    teardown ();

                    exit (EXIT_FAILURE);
                  }
              }
            else
              {
                fprintf (stderr, _("%s: error parsing time '%s'\n"),
                         program_name, optarg);

                list_destroy (&nflist);
                teardown ();

                exit (EXIT_FAILURE);
              }

            break;
          }

        case 's':
          sequencer = SEQUENCER;
          break;

        case 't':
        case 'y':   /* Pseudo-option for --imsa */
          traditional = TRUE;
          if (!override_flag)
            seq_own_notes = TRUE;
          break;

        case 'u':
          {
            struct passwd *pw;

            if (getuid () != 0)
              {
                fprintf (stderr, _("%s: only root is allowed to use '--user'\n"),
                         program_name);

                list_destroy (&nflist);
                teardown ();

                exit (EXIT_FAILURE);
              }

            pw = getpwnam (optarg);
            if (pw)
              {
                seteuid (pw->pw_uid);

                newts_free (username);
                username = newts_strdup (pw->pw_name);

                if (alt_sequencer)
                  {
                    newts_nrealloc (seqname,
                                    strlen (username) + strlen (alt_seqname) + 2,
                                    sizeof (char));
                    strcpy (seqname, username);
                    strcat (seqname, ":");
                    strcat (seqname, alt_seqname);
                  }
                else
                  {
                    newts_free (seqname);
                    seqname = newts_strdup (username);
                  }
              }

            else
              {
                fprintf (stderr, _("%s: no such user: '%s'\n"), program_name,
                         optarg);

                list_destroy (&nflist);
                teardown ();

                exit (EXIT_FAILURE);
              }
          }
          break;

        case 'w':
          white_basenotes = TRUE;
          break;

        case 'x':
          sequencer = EXTENDED;
          break;

        case 'z':
          no_blacklist = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE...\n"
                    "Run the UIUC-compatible notesfile client.\n\n"),
                  program_name);

          printf (_("If an argument to a long option is mandatory, it is also mandatory "
                    "for the\ncorresponding short option.\n\n"));

          printf (_("General options:\n"
                    "  -f, --file=FILE         Read list of notesfiles to view from specified file\n"
                    "  -g, --no-signature      Turn off automatic signature inclusion\n"
                    "  -h, --help              Display this help and exit\n"
                    "  -u, --user=USER         As root, run notes as the specified user\n"
                    "      --debug             Print debugging messages to stderr\n"
                    "      --version           Display version information and exit\n\n"));

          printf (_("Display options:\n"
                    "  -m, --modern            Use modern, consistent display style (default)\n"
                    "  -t, --traditional       Use traditional UIUC-style display\n"
                    "      --imsa              Use IMSA-style display (same as -t)\n\n"));

          printf (_("Blacklist options:\n"
                    "  -b, --black-threads     Skip threads with blacklisted basenotes while seqing\n"
                    "  -w, --white-basenotes   Do not apply blacklist to basenotes\n"
                    "  -z, --no-blacklist      Do not use the blacklist\n\n"));

          printf (_("Sequencer options:\n"
                    "  -a, --alternate=SEQ     Use alternate sequencer SEQ\n"
                    "  -e, --seq-own-notes     Make the sequencer view your notes\n"
                    "  -i, --index             Use the index sequencer\n"
                    "  -k, --skip-own-notes    Make the sequencer ignore your notes (default)\n"
                    "  -n, --no-sequencer      Do not use the sequencer (default)\n"
                    "  -o, --time=TIME         Use this date and time for the sequencer\n"
                    "  -s, --sequencer         Use the sequencer to read notes\n"
                    "  -x, --extended          Use the extended sequencer\n\n"));

          printf (_("Report bugs to <%s>.\n"), PACKAGE_BUGREPORT);

          list_destroy (&nflist);
          teardown ();

          if (fclose (stdout) == EOF)
            error (EXIT_FAILURE, errno, _("error writing output"));

          exit (EXIT_SUCCESS);

        case '?':
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          list_destroy (&nflist);
          teardown ();

          exit (EXIT_FAILURE);
        }
    }

  if (optind == argc && !file_flag)
    {
      if (sequencer == NONE)
        {
          struct stat statbuf;

          fprintf (stderr, _("%s: too few arguments\n"), program_name);
          fprintf (stderr, _("Try '%s --help' for more information.\n"),
                   program_name);

          /* FIXME: should be a call to the notes system. */

          if (stat ("/etc/avail.notes", &statbuf) == 0)
            {
              fprintf (stderr,
                       _("Hit <RET> for a list of notesfiles on this system.\n"));
              getchar ();

              spawn_process (NULL, pager, "/etc/avail.notes", NULL);
            }

          list_destroy (&nflist);
          teardown ();

          exit (EXIT_FAILURE);
        }
      else
        {
          char *copy, *list, *token, *nfseq;

          nfseq = getenv ("NFSEQ");
          if (nfseq == NULL)
            {
              fprintf (stderr, _("%s: NFSEQ environment variable not set\n"),
                       program_name);
              fprintf (stderr, _("See 'info newts' for more information.\n"));

              list_destroy (&nflist);
              teardown ();

              exit (EXIT_FAILURE);
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
    }
  else
    {
      while (optind < argc)
        parse_nf (argv[optind++], &nflist);
    }

  handle_signals ();

  /* For each notesfile, start up the master routine and go. */

  {
    ListNode *node = list_head (&nflist);

    while (node && !quit_flag)
      {
        newts_nfref *ref = (newts_nfref *) list_data (node);

        result = master (ref);

        node = list_next (node);

        if (result == QUITSEQ || result == QUITNOSEQ)
          quit_flag = TRUE;
      }
  }

  ignore_signals ();
  exit_curses ();

  if (*messages != '\0')
    printf ("%s", messages);

  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (EXIT_SUCCESS);
}
