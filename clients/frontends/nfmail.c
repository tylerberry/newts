/*
 * nfmail.c - write a note with text from a saved mail message
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
#include "getline.h"
#include "getopt.h"

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

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile nf;
  struct newt note;

  struct author auth;
  struct passwd *pw;
  short anonymous = FALSE;
  short director = FALSE;
  short override_title = FALSE;
  short strip_headers = FALSE;
  short verbose = FALSE;

  int result;

  int opt;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {"anonymous",0,0,'a'},
      {"debug",0,0,'D'},
      {"director-msg",2,0,'d'},
      {"strip-headers",0,0,'s'},
      {"title",1,0,'t'},
      {"verbose",1,0,'v'},
      {"help",0,0,'h'},
      {"version",0,0,0},
      {0,0,0,0}
    };

  memset (&nf, 0, sizeof (struct notesfile));
  memset (&note, 0, sizeof (struct newt));

  note.director_message = NULL;

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

  while ((opt = getopt_long (argc, argv, "ad::hst:v",
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfmail"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'a':
          anonymous = TRUE;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'd':
          director = TRUE;
          if (note.director_message)
            newts_free (note.director_message);
          if (optarg)
            note.director_message = newts_strdup (optarg);
          break;

        case 's':
          strip_headers = TRUE;
          break;

        case 't':
          override_title = TRUE;
          if (note.title)
            newts_free (note.title);
          note.title = newts_strdup (optarg);
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... [FILE] NOTESFILE\n"
                    "Create a note in NOTESFILE with text from saved mail message in FILE (or stdin\n"
                    "if unspecified)\n\n"),
                  program_name);

          printf (_("If an argument to a long option is mandatory, it is also mandatory for the\n"
                    "corresponding short option; the same is true for optional arguments.\n\n"));

          printf (_("  -a, --anonymous            Make note anonymous\n"
                    "  -d, --director-msg[=MSG]   Give the note a director message\n"
                    "  -s, --strip-headers        Strip the mail headers from FILE\n"
                    "  -t, --title=TITLE          Set note's title to TITLE\n"
                    "  -v, --verbose              Print extra status messages\n"
                    "      --debug                Display debugging messages\n\n"
                    "  -h, --help                 Display this help and exit\n"
                    "      --version              Display version information and exit\n\n"));

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

  list_init (&nflist,
             (void * (*) (void)) nfref_alloc,
             (void (*) (void *)) nfref_free,
             NULL);

  if (optind == argc)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      list_destroy (&nflist);
      teardown ();

      exit (EXIT_FAILURE);
    }
  else if (optind + 1 == argc)  /* nfpipe NOTESFILE */
    {
      parse_nf (argv[optind], &nflist);
    }
  else if (optind + 2 == argc)  /* nfpipe FILE NOTESFILE */
    {
      /* We replace stdin with the specified file. */

      freopen (argv[optind], "r", stdin);

      parse_nf (argv[optind + 1], &nflist);
    }
  else
    {
      fprintf (stderr, _("%s: too many arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      list_destroy (&nflist);
      teardown ();

      exit (EXIT_FAILURE);
    }

  result = open_nf ((newts_nfref *) list_data (list_head (&nflist)), &nf);

  if (result != NEWTS_NO_ERROR)
    {
      list_destroy (&nflist);
      teardown ();

      error (EXIT_FAILURE, 0, _("error opening notesfile '%s'"),
             nfref_pretty_name (nf.ref));
    }

  if (!(nf.perms & WRITE) && !(nf.perms & DIRECTOR))
    {
      list_destroy (&nflist);
      teardown ();

      error (EXIT_FAILURE, 0, _("permission denied: can't write to notesfile '%s'"),
             nfref_pretty_name (nf.ref));
    }

  if (nf.options & NF_ARCHIVE && !(nf.perms & DIRECTOR))
    {
      list_destroy (&nflist);
      teardown ();

      error (EXIT_FAILURE, 0, _("permission denied: can't write to archive '%s'"),
             nfref_pretty_name (nf.ref));
    }

  if (!note.title)
    note.title = newts_strdup ("** From nfmail **");

  if (director)
    {
      if (!(nf.perms & DIRECTOR))
        note.director_message = NULL;
      else if (!note.director_message)
        note.director_message = newts_strdup ("** Director Message **");
    }

  pw = getpwuid (geteuid ());
  auth.name = pw->pw_name;
  auth.system = newts_get_fqdn ();
  auth.uid = pw->pw_uid;
  note.auth = auth;
  time (&note.created);
  time (&note.modified);
  note.nr.nfr.owner = nfref_owner (nf.ref);
  note.nr.nfr.name = nfref_name (nf.ref);
  note.nr.notenum = -1;

  {
    short headerflag = TRUE;
    short matched_subject = FALSE;
    register int blocks = 1;
    register int count = 0;
    size_t length;
    char *line = NULL;
    register char *c;

    char *buf = newts_nmalloc (1024, sizeof (char));

    while (getline (&line, &length, stdin) >= 0)
      {
        if (count + length + 1 >= blocks * 1024)
          {
            blocks++;
            buf = newts_nrealloc (buf, 1024 * blocks, sizeof (char));
          }

        if (*line == '\n')
          {
            headerflag = FALSE;
            if (!strip_headers)    /* We need to add a newline. */
              buf[count++] = '\n';
            newts_free (line);
            continue;
          }

        if (headerflag && !override_title &&
            strncmp (line, "Subject: ", 9) == 0)
          {
            if (!matched_subject)
              {
                matched_subject = TRUE;
                if (note.title)
                  newts_free (note.title);
                note.title = newts_strdup (line + 9);
              }
          }

        if (headerflag && strip_headers)
          {
            newts_free (line);
            continue;
          }

        c = line;

        while (*c != '\0')
          buf[count++] = *(c++);

        newts_free (line);
      }
    buf[count++] = '\0';

    note.text = newts_nmalloc (strlen (buf) + 1, sizeof (char));
    sprintf (note.text, "%s", buf);

    newts_free (buf);
  }

  write_note (&nf, &note, UPDATE_TIMES + ADD_ID);

  if (verbose)
    {
      printf (_("Added '%s' to %s.\n"), note.title,
              nfref_pretty_name (nf.ref));
    }

  list_destroy (&nflist);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (EXIT_SUCCESS);
}
