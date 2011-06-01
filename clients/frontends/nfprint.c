/*
 * nfprint.c - print formatted notesfile to stdout
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry.
 *
 * Based in part on nfprint.c and lprnote.c from the UIUC notes distribution by
 * Ray Essick and Rob Kolstad.  Any work derived from this source code is
 * required to retain this notice.
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
#include "newts/uiuc-compatibility.h"

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

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

/* Whether to display debugging messages. */
int debug = FALSE;

/* Use cat(1) instead of the default pr(1)? */
int use_cat = FALSE;

/* Date tracking. */
int last_year = 0, last_month = 0, last_day = 0;

/* Number of lines and remaining lines left on this 'page' */
int length = 66;
int left;

/* Page number we're currently on. */
int page = 1;

/* Printing a table of contents only? */
int index_only = FALSE;

static void lprnote (FILE *toc, struct notesfile *nf, struct newt *notep);
static void lprresp (struct newt *respp);

int
main (int argc, char **argv)
{
  List nflist;
  struct notesfile nf;
  struct newt note;

  int result;
  int i, pid;
  int director_only = FALSE;
  int exclude_director = FALSE;
  int single_page = FALSE;

  FILE *tocf;

  int opt;
  int option_index = 0;
  extern char *optarg;
  extern int optind, opterr, optopt;

  struct option long_options[] =
    {
      {"cat",0,0,'c'},
      {"debug",0,0,'D'},
      {"director",0,0,'d'},
      {"index-only",0,0,'i'},
      {"length",1,0,'l'},
      {"no-director",0,0,'n'},
      {"page-breaks",0,0,'p'},
      {"help",0,0,'h'},
      {"version",0,0,0},
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

  /* Catch the '-nd' option, replace it with '-n'. This is mostly a backwards-
   * compatibility thing in case people remember it working.
   */

  for (i=0; i < argc; i++)
    {
      if (strcmp (argv[i], "-nd") == 0)
        argv[i] = "-n";
    }

  while ((opt = getopt_long (argc, argv, "cdhil:npt",
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfprint"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'c':
          use_cat = TRUE;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'd':
          director_only = TRUE;
          exclude_director = FALSE;
          break;

        case 'i':
        case 't':   /* Backwards compatibility. */
          index_only = TRUE;
          break;

        case 'l':
          length = atoi (optarg);
          break;

        case 'n':
          director_only = FALSE;
          exclude_director = TRUE;
          break;

        case 'p':
          single_page = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... NOTESFILE [LIST]\n"
                    "Format and print notes (in LIST) from NOTESFILE.\n\n"),
                  program_name);

          printf (_("  -c, --cat           Use cat(1) instead of pr(1)\n"
                    "  -d, --director      Select only notes with director messages\n"
                    "  -i, --index-only    Print a table of note titles only\n"
                    "  -l, --length=LEN    Use a page length of LEN lines\n"
                    "  -n, --no-director   Select only notes without director messages\n"
                    "  -p, --page-breaks   Insert a page break after each thread\n"
                    "      --debug         Display debugging messages\n\n"
                    "  -h, --help          Display this help and exit\n"
                    "      --version       Display version information and exit\n\n"));

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
  else
    {
      list_init (&nflist,
                 (void * (*) (void)) nfref_alloc,
                 (void (*) (void *)) nfref_free,
                 NULL);
      parse_nf (argv[optind++], &nflist);
    }

  {
    newts_nfref *ref = (newts_nfref *) list_data (list_head (&nflist));

    /* Open the notesfile. */

    result = open_nf (ref, &nf);

    if (result != NEWTS_NO_ERROR)
      {
        list_destroy (&nflist);
        teardown ();

        error (EXIT_FAILURE, 0, _("error opening notesfile '%s'"),
               nfref_pretty_name (ref));
      }

    if (!(nf.perms & READ) && !(nf.perms & DIRECTOR))
      {
        list_destroy (&nflist);
        teardown ();

        error (EXIT_FAILURE, 0, _("you are not allowed to read notesfile '%s'"),
               nfref_pretty_name (ref));
      }
  }

  /* Open a temporary file for the table of contents. */

  tocf = tmpfile ();

  fprintf (tocf,
           _("================================ Index =================================\n\n"));


  /* Create a pipe to either cat or pr as appropriate.  Egads, doing this
   * without popen(3) is so much more annoying.
   */

  {
    int pipedesc[2];

    pipe (pipedesc);
    pid = fork ();
    switch (pid)
      {
      case -1:
        error (EXIT_FAILURE, 0, _("pipe failed"));

      case 0: /* Child process. */
        close (pipedesc[1]);
        dup2 (pipedesc[0], STDIN_FILENO);
        close (pipedesc[0]);
        if (use_cat)
          {
            execl (CAT, CAT, "-", NULL);
          }
        else
          {
            char *lenstr = newts_nmalloc (11, sizeof (char));
            char *header = newts_nmalloc (strlen (fqdn) + strlen (nf.title) + 4,
                                     sizeof (char));

            sprintf (lenstr, "%d", length);
            sprintf (header, "(%s) %s", fqdn, nf.title);
            execl (PR, PR, "-l", lenstr, "-h", header, NULL);
            newts_free (lenstr);
            newts_free (header);
          }
        _exit (EXIT_FAILURE);

      default: /* Parent process. */
        close (pipedesc[0]);
        dup2 (pipedesc[1], STDOUT_FILENO); /* Set up as stdout. */
        close (pipedesc[1]);
        break;
      }
  }

  length -= 10;   /* pr uses 10 for its header/footer. */
  left = length;

  if (optind == argc)
    {
      /* We're going to print every note.  We need to rewrite the arg string a
       * little to cause this.
       */

      char *default_range = newts_nmalloc (13, sizeof (char));
      if (nf.total_notes > 0)
        sprintf (default_range, "%d-%d", nf.options & NF_POLICY ? 0 : 1,
                 nf.total_notes);
      else
        sprintf (default_range, "0");

      optind--;

      argv[optind] = default_range; /* Wipes out the nf name, but we already
                                       * used that, so no problem.
                                       */
    }

  while (optind < argc)
    {
      int second_or_later = FALSE;
      int start, end;
      int bufptr = 0;
      struct newt note;
      memset (&note, 0, sizeof (struct newt));
      nfref_copy (&note.nr.nfr, nf.ref);

      while (list_parse (argv[optind], &bufptr, &start, &end))
        {
          if (start == 0 && end == 0)
            continue;
          if (start > end)
            {
              int temp = start;
              start = end;
              end = temp;
            }
          if (start > nf.total_notes)
            continue;

          if (second_or_later)
            {
              putc ('\n', tocf);
              last_year = last_month = last_day = 0;
            }
          second_or_later = TRUE;

          if ((start < 1 && !(nf.options & NF_POLICY)) || start < 0)
            start = 1;
          if (end > nf.total_notes)
            end = nf.total_notes;
          for (i = start; i <= end; i++)
            {
              note.nr.notenum = i;
              note.nr.respnum = 0;

              get_note (&note, FALSE);
              if ((note.options & NOTE_DELETED ||
                   note.options & NOTE_DIRECTORS_ONLY ||
                   note.options & NOTE_UNAPPROVED) &&
                  !allowed (&nf, DIRECTOR))
                continue;
              if ((director_only && (note.director_message == NULL)) ||
                  (exclude_director && (note.director_message != NULL)))
                continue;
              if (single_page && left != length)
                {
                  if (use_cat)
                    putchar ('\n');
                  else
                    putchar ('\f');
                  page++;
                  left = length;
                }

              lprnote (tocf, &nf, &note);
            }

          printf ("\n========================================================================\n");
        }

      optind++;
    }

  /* Copy the table of contents to the main pipe. */

  fseek (tocf, 0, SEEK_SET);
  if (!index_only)
    {
      if (use_cat)
        putchar ('\n');
      else
      putchar ('\f');
      page++;
      left = length;
    }
  while ((i = getc (tocf)) != EOF)
    {
      if (i == '\n')
        {
          left--;
          if (left == 0)
            {
              if (use_cat)
                putchar ('\n');
              else
                {
                  putchar ('\n');
                  putchar ('\f');
                }
              page++;
              left = length;
            }
          else
            putchar ('\n');
        }
      else
        putchar (i);
    }

  printf ("\n========================================================================\n");

  fclose (tocf);
  waitpid (pid, &i, WNOHANG);

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (EXIT_SUCCESS);
}

static void
lprnote (FILE *tocf, struct notesfile *nf, struct newt *notep)
{
  struct tm *tm = localtime (&notep->created);
  struct newt resp;
  int i;

  if (left < 7) /* We need seven to print a header and some text. */
    {
      if (use_cat)
        putchar ('\n');
      else
      putchar ('\f');
      page++;
      left = length;
    }

  if ((tm->tm_year + 1900 > last_year || (tm->tm_mon + 1) > last_month
       || tm->tm_mday > last_day) && !(notep->options & NOTE_CORRUPTED))
    {
      char *buffer = newts_nmalloc (9, sizeof (char));
      int len;

      if (tm->tm_year + 1900 != last_year)
        snprintf (buffer, 8, "%d/%d/%02d", last_month = (tm->tm_mon + 1),
                last_day = tm->tm_mday, (last_year = tm->tm_year + 1900) % 100);
      else
        snprintf (buffer, 8, "%d/%d", last_month = (tm->tm_mon + 1),
                 last_day = tm->tm_mday);
      fprintf (tocf, "%s", buffer);

      len = strlen (buffer);
      for (; len <= 8; len++)
        putc (' ', tocf);

      newts_free (buffer);
    }
  else
    fprintf (tocf, "         ");

  fprintf (tocf, "%4d", notep->nr.notenum);

  if (notep->options & NOTE_DIRECTORS_ONLY)
    putc ('=', tocf);
  else if (notep->options & NOTE_ANNOUNCEMENT)
    putc ('+', tocf);
  else if (notep->options & NOTE_UNAPPROVED)
    putc (':', tocf);
  else if (notep->options & NOTE_DELETED)
    putc ('-', tocf);
  else if (notep->director_message)
    putc ('*', tocf);
  else
    putc (' ', tocf);

  {
    char *buffer = newts_nmalloc (TITLEN + 1, sizeof (char));
    int len;

    snprintf (buffer, TITLEN, "%s", notep->title);
    fprintf (tocf, "%s", buffer);

    len = strlen (buffer);
    for (; len <= TITLEN; len++)
      putc (' ', tocf);

    newts_free (buffer);
  }

  if (notep->total_resps > 0)
    fprintf (tocf, "%5d ", notep->total_resps);
  else
    fprintf (tocf, "      ");

  if (strcasecmp (notep->auth.name, "anonymous") &&
      strcmp (notep->auth.system, fqdn))
    fprintf (tocf, "%s@%s", notep->auth.name, notep->auth.system);
  else
    if (strcasecmp (notep->auth.name, "anonymous"))
      fprintf (tocf, "%s", notep->auth.name);
    else
      fprintf (tocf, "anonymous");

  putc ('\n', tocf);

  if (index_only)  /* If we only want a table of contents, we're done. */
    return;

  {
    int hashes = 70 - strlen (notep->title);
    int first = hashes / 2;
    int i;

    putchar ('\n');
    for (i=0; i < first; i++)
      putchar ('=');
    putchar (' ');

    printf ("%s", notep->title);

    putchar (' ');
    for (i=0; i < hashes - first; i++)
      putchar ('=');
    putchar ('\n');
  }
  if (notep->director_message)
    {
      int hashes = 70 - strlen (notep->director_message);
      int first = hashes / 2;
      int i;

      for (i=0; i < first; i++)
        putchar ('-');
      putchar (' ');

      printf ("%s", notep->director_message);

      putchar (' ');
      for (i=0; i < hashes - first; i++)
        putchar ('-');
      putchar ('\n');
    }

  {
    int i;
    char *titlebuf, *respbuf;
    titlebuf = newts_nmalloc (80, sizeof (char));
    respbuf = newts_nmalloc (40, sizeof (char));

    sprintf (titlebuf, _("Note %d"), notep->nr.notenum);
    printf ("%s", titlebuf);

    if (notep->total_resps)
      {
        sprintf (respbuf, ngettext ("%d response", "%d responses",
                                    notep->total_resps), notep->total_resps);
        for (i=0; i < 72 - strlen (titlebuf) - strlen (respbuf); i++)
          putchar (' ');
        printf ("%s", respbuf);
      }
    putchar ('\n');

    newts_free (titlebuf);
    newts_free (respbuf);
  }

  {
    unsigned authsize = SYSSZ + NAMESZ + 2;
    char *authbuf = newts_nmalloc (authsize, sizeof (char));
    char *timebuf = newts_nmalloc (25, sizeof (char));

    if (strcasecmp (notep->auth.name, "anonymous") &&
        strcmp (notep->auth.system, fqdn))
      snprintf (authbuf, authsize,
                "%s@%s", notep->auth.name, notep->auth.system);
    else
      if (strcasecmp (notep->auth.name, "anonymous"))
        snprintf (authbuf, authsize, "%s", notep->auth.name);
      else
        snprintf (authbuf, authsize, "anonymous");

    printf ("%s", authbuf);
    sprint_time (timebuf, tm);
    for (i=0; i < 72 - strlen (authbuf) - strlen (timebuf); i++)
      putchar (' ');
    printf ("%s", timebuf);
    putchar ('\n'); putchar ('\n');

    newts_free (authbuf);
    newts_free (timebuf);
  }

  left -= 5; /* Used up by the header. */

  {
    int c;
    char *cursor = notep->text;

    while ((c = *cursor++) && c != EOF)
      {
        if (c == '\n')
          left--;
        putchar (c);
      }
  }

  while (left < length)
    {
      page++;
      left += length;
    }

  memset (&resp, 0, sizeof (struct newt));
  nfref_copy (&resp.nr.nfr, &notep->nr.nfr);
  resp.nr.notenum = notep->nr.notenum;

  for (i=1; i <= notep->total_resps; i++)
    {
      resp.nr.respnum = i;
      get_note (&resp, FALSE);
      lprresp (&resp);
    }

  return;
}

static void
lprresp (struct newt *notep)
{
  struct tm *tm = localtime (&notep->created);
  int i;

  if (left < 7) /* We need seven to print a header and some text. */
    {
      if (use_cat)
        putchar ('\n');
      else
      putchar ('\f');
      page++;
      left = length;
    }

  if (notep->director_message)
    {
      int hashes = 70 - strlen (notep->director_message);
      int first = hashes / 2;
      int i;

      putchar ('\n');
      for (i=0; i < first; i++)
        putchar ('-');
      putchar (' ');

      printf ("%s", notep->director_message);

      putchar (' ');
      for (i=0; i < hashes - first; i++)
        putchar ('-');
      putchar ('\n');
    }
  else
    printf ("\n------------------------------------------------------------------------\n");

  printf (_("Response %d"), notep->nr.respnum);
  putchar ('\n');

  {
    unsigned authsize = SYSSZ + NAMESZ + 2;
    char *authbuf = newts_nmalloc (authsize, sizeof (char));
    char *timebuf = newts_nmalloc (25, sizeof (char));

    if (strcasecmp (notep->auth.name, "anonymous") &&
        strcmp (notep->auth.system, fqdn))
      snprintf (authbuf, authsize, "%s@%s",
                notep->auth.name, notep->auth.system);
    else
      if (strcasecmp (notep->auth.name, "anonymous"))
        snprintf (authbuf, authsize, "%s", notep->auth.name);
      else
        snprintf (authbuf, authsize, "anonymous");

    printf ("%s", authbuf);
    sprint_time (timebuf, tm);
    for (i=0; i < 72 - strlen (authbuf) - strlen (timebuf); i++)
      putchar (' ');
    printf ("%s", timebuf);
    putchar ('\n'); putchar ('\n');

    newts_free (authbuf);
    newts_free (timebuf);
  }

  left -= 5; /* Used up by the header. */

  {
    int c;
    char *cursor = notep->text;

    while ((c = *cursor++) && c != EOF)
      {
        if (c == '\n')
          left--;
        putchar (c);
      }
  }

  while (left < length)
    {
      page++;
      left += length;
    }

  return;
}
