/*
 * nfload.c - load up a dumped notesfile image
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
#include "newts/uiuc.h"

#include "dirname.h"
#include "error.h"
#include "getopt.h"
#include "scan-uiuc.h"
#include "yesno.h"

/* Flag on whether we have a policy note. */
int policy_exists = FALSE;

/* Flag on whether to not ask permission on access entries. */
int force_access = FALSE;

/* Flag on whether to skip adding access entries. */
int skip_access = FALSE;

/* Flag on whether to cream existing access rights. */
int replace_access = FALSE;

/* Flag on whether to skip asking permission. */
int force = FALSE;

/* Flag on whether this is a notesfile that already existed, or if it's one
 * that we just created.
 */
int using_existing_nf = FALSE;

/* How much detail to print out. */
int debug = FALSE;
int verbose = FALSE;

static int load_uiuc_dump (struct notesfile *nf);
static int load_uiuc_access (struct notesfile *nf);
static int load_uiuc_descriptor (struct notesfile *nf);

extern int current_response;
extern int responses_expected;
extern FILE *yyin;
extern struct newt note;

int
main (int argc, char **argv)
{
  newts_nfref *ref;
  struct notesfile nf;
  char *dumpfile;
  char *fname = NULL;
  int result;
  char test[11];
  FILE *infile;
  int uiuc_format_flag = FALSE;

  int opt;
  int option_index = 0;

  struct option long_options[] =
    {
      {N_("force-access"),0,0,'a'},
      {N_("debug"),0,0,'D'},
      {N_("force"),0,0,'f'},
      {N_("replace-access"),0,0,'r'},
      {N_("skip-access"),0,0,'s'},
      {N_("verbose"),0,0,'v'},
      {N_("help"),0,0,'h'},
      {N_("version"),0,0,0},
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

  while ((opt = getopt_long (argc, argv, N_("afhrsv"),
                             long_options, &option_index)) != -1)
    {
      switch (opt)
        {
        case 0:
          {
            printf_version_string (N_("nfload"));

            teardown ();

            if (fclose (stdout) == EOF)
              error (EXIT_FAILURE, errno, _("error writing output"));
            exit (EXIT_SUCCESS);
          }

        case 'a':
          force_access = TRUE;
          break;

        case 'D':
          debug = TRUE;
          break;

        case 'f':
          force = TRUE;
          force_access = TRUE;
          break;

        case 'r':
          force_access = TRUE;
          replace_access = TRUE;
          break;

        case 's':
          skip_access = TRUE;
          break;

        case 'v':
          verbose = TRUE;
          break;

        case 'h':
          printf (_("Usage: %s [OPTION]... IMAGE NOTESFILE\n"
                    "Load notesfile IMAGE into NOTESFILE.\n\n"), program_name);

          printf (_("  -a, --force-access     Add access entries without asking for confirmation\n"
                    "  -f, --force            Make all changes without asking for confirmation\n"
                    "  -r, --replace-access   Replace existing access entries\n"
                    "  -s, --skip-access      Do not alter access entries\n"
                    "  -v, --verbose          Display extra status messages\n"
                    "      --debug            Display debugging messages\n\n"
                    "  -h, --help             Display this help and exit\n"
                    "      --version          Display version information and exit\n\n"));

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

  if (optind == argc || optind + 1 == argc)
    {
      fprintf (stderr, _("%s: too few arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      teardown ();

      exit (EXIT_FAILURE);
    }

  if (optind + 2 < argc)
    {
      fprintf (stderr, _("%s: too many arguments\n"), program_name);
      fprintf (stderr, _("Try '%s --help' for more information.\n"),
               program_name);

      teardown ();

      exit (EXIT_FAILURE);
    }

  dumpfile = argv[optind++];

  if (getuid () == 0)
    seteuid (0);

  infile = fopen (dumpfile, "r");

  if (getuid () == 0)
    seteuid (notes_uid);

  if (infile == NULL)
    {
      teardown ();

      error (EXIT_FAILURE, 0, _("file '%s' not found"), dumpfile);
    }

  /* FIXME: make this more robust. */

  fread (test, sizeof (char), 10, infile);
  test[10] = '\0';
  if (strcmp (test, N_("NF-Title: ")) == 0)
    {
      if (verbose || debug)
        printf (_("Looks like a UIUC-format nfdump...\n"));
      uiuc_format_flag = TRUE;
    }
  else
    {
      teardown ();

      error (EXIT_FAILURE, 0, _("nfdump file format not recognized"));
    }

  rewind (infile);

  ref = nfref_alloc ();
  parse_single_nf (argv[optind], ref);

  result = open_nf (ref, &nf);

  if (result == NEWTS_NF_DOESNT_EXIST)
    {
      if (euid != notes_uid)
        nfref_set_owner (ref, username);

      result = open_nf (ref, &nf);
      if (result == NEWTS_NF_DOESNT_EXIST)
        {
          result = create_nf (ref, 0);

          switch (result)
            {
            case NEWTS_NO_ERROR:
              if (!skip_access)
                {
                  force_access = TRUE;
                  replace_access = TRUE;
                }

              if (verbose || debug)
                printf (_("Created notesfile '%s'.\n"),
                        nfref_pretty_name (ref));

              if (open_nf (ref, &nf) != NEWTS_NO_ERROR)
                {
                  nfref_free (ref);
                  teardown ();

                  error (EXIT_FAILURE, 0,
                         _("error opening newly-created notesfile '%s'"),
                         nfref_pretty_name (ref));
                }

              break;

            case -2:
              fprintf (stderr, _("%s: error creating '%s': Already exists\n"),
                       program_name, nfref_pretty_name (ref));
              fprintf (stderr, _("You should probably never get this error message.\n"
                                 "If you do, it's probably a bug.\n"));

              nfref_free (ref);
              teardown ();

              exit (EXIT_FAILURE);

            case -1:
            default:
              nfref_free (ref);
              teardown ();

              error (EXIT_FAILURE, 0, _("error creating '%s'"), fname);
            }
        }
    }
  else
    using_existing_nf = TRUE;

  /* We need to prep the lexer to take the file. */
  yyin = infile;

  if (load_uiuc_dump (&nf))
    fprintf (stderr, _("%s: aborting attempt to load dump file"),
             program_name);
  else
    {
      if (!verbose) printf ("\n");
      printf (_("Successfully loaded '%s'.\n"), dumpfile);
    }

  fclose (infile);

  nfref_free (ref);
  teardown ();

  if (fclose (stdout) == EOF)
    error (EXIT_FAILURE, errno, _("error writing output"));

  exit (EXIT_SUCCESS);
}

int
load_uiuc_dump (struct notesfile *nf)
{
  /* Okay, so now we have an open notesfile, and the dumpfile is on stdin. */

  if (load_uiuc_descriptor (nf))
    {
      fprintf (stderr, _("%s: error loading UIUC descriptor"), program_name);
      return -1;
    }

  modify_nf (nf);
  if (verbose)
    printf (_("Loaded notesfile descriptor.\n"));

  if (policy_exists)
    {
      short replace_flag = force;
      int field = yylex ();
      if (field != NOTE)
        {
          fprintf (stderr, _("%s: error reading policy note"), program_name);
          return -1;
        }

      nfref_copy (&note.nr.nfr, nf->ref);
      note.nr.notenum = -1;

      if (!force && using_existing_nf && (nf->options & NF_POLICY))
        {
          printf (_("Replace existing policy note (y/n)? "));
          if (yesno ())
            replace_flag = TRUE;
        }
      if (!using_existing_nf || !(nf->options & NF_POLICY) || replace_flag)
        {
          write_note (nf, &note, ADD_POLICY + ADD_ID);

          if (verbose)
            printf (_("Loaded policy note.\n"));
        }
    }

  {
    if (load_uiuc_access (nf))
      {
        fprintf (stderr, _("%s: error reading access records"), program_name);
        return -1;
      }

    if (verbose)
      printf (_("Loaded notesfile access records.\n"));
  }

  {
    int i;
    int field;
    int current_note;

    while ((field = yylex ()))
      {
        if (field != NOTE)
          {
            fprintf (stderr, _("%s: error reading basenote"), program_name);
            return -1;
          }

        nfref_copy (&note.nr.nfr, nf->ref);
        note.nr.notenum = -1;

        current_note = write_note (nf, &note, ADD_ID);

        if (verbose)
          printf (_("Loaded note: '%s'\n"), note.title);
        else
          {
            printf (":");
            fflush (stdout);
          }

        for (i=0; i<responses_expected; i++)
          {
            int field = yylex ();
            if (field != RESPONSE)
              {
                fprintf (stderr, _("%s: error reading response"),
                         program_name);
                return -1;
              }

            nfref_copy (&note.nr.nfr, nf->ref);
            note.nr.notenum = current_note;

            write_note (nf, &note, ADD_ID);

            if (verbose)
              printf (_("Loaded response %d of %d.\n"), current_response,
                      responses_expected);
            else if (i % 5 == 0)
              {
                printf (".");
                fflush (stdout);
              }
          }
      }
  }

  return 0;
}

int
load_uiuc_descriptor (struct notesfile *nf)
{
  short replace_flag;
  register int field;
  struct uiuc_opts *opts = (struct uiuc_opts *) nf->opts;

  while (TRUE)
    {
      replace_flag = force;

      field = yylex ();
      switch (field)
        {
        case NOP:
          break;

        case TITLE:
          if (!force && using_existing_nf)
            {
              printf (_("Replace notesfile title (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              if (nf->title)
                newts_free (nf->title);
              nf->title = newts_strdup (contents);
            }
          break;

        case DIRECTOR_MESSAGE:
          if (!force && using_existing_nf)
            {
              printf (_("Replace notesfile director message (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              if (nf->director_message)
                newts_free (nf->director_message);
              nf->director_message = newts_strdup (contents);
            }
          break;

        case STATUS:
          {
            char *p = contents;
            char *statname = newts_nmalloc (11, sizeof (char));

            if (!force && using_existing_nf)
              {
                printf (_("Replace notesfile options (y/n)? "));
                if (yesno ())
                  replace_flag = TRUE;
              }
            if (!using_existing_nf || replace_flag)
              {
                nf->options |= NF_LOCKED;
                while (*p && *p != '\n')        /* end string */
                  {
                    if (sscanf (p, N_("%10s"), statname) != 1)
                      break;          /* no more tokens */
                    if (debug)
                      fprintf (stderr, _("  Option: '%s'\n"), statname);
                    if (strcmp (statname, N_("Anonymous")) == 0)
                      nf->options |= NF_ANONYMOUS;
                    else if (strcmp (statname, N_("Open")) == 0)
                      nf->options &= ~NF_LOCKED;
                    else if (strcmp (statname, N_("Archive")) == 0)
                      nf->options |= NF_ARCHIVE;
                    else if (strcmp (statname, N_("Moderated")) == 0)
                      nf->options |= NF_MODERATED;
                    else if (strcmp (statname, N_("Networked")) == 0 ||
                             strcmp (statname, N_("Local")) == 0)
                      ; /* Imtentionally ignore. */
                    else
                      {
                        fprintf (stderr, _("Invalid option in Status field: '%s'\n"),
                                 statname);
                        return -1;
                      }
                    p += strlen (statname) + 1; /* leading space */
                  }
              }

            newts_free (statname);

            break;
          }

        case EXPIRATION_AGE:
          if (!force && using_existing_nf)
            {
              printf (_("Replace expiration threshold (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              opts->expire_threshold = strtol (contents, NULL, 10);
              if (opts->expire_threshold < 0)
                {
                  fprintf (stderr, _("Invalid value for Expiration-Age field.\n"));
                  return -1;
                }
            }
          break;

        case EXPIRATION_ACTION:
          if (!force && using_existing_nf)
            {
              printf (_("Replace expiration action (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              if (strcmp (contents, N_("Archive")) == 0)
                opts->expire_action = KEEPYES;
              else if (strcmp (contents, N_("Delete")) == 0)
                opts->expire_action = KEEPNO;
              else if (strcmp (contents, N_("Default")) == 0)
                opts->expire_action = KEEPDFLT;
              else
                {
                  fprintf (stderr, _("Unknown value for Expiration-Action field.\n"));
                  return -1;
                }
            }
          break;

        case EXPIRATION_STATUS:
          if (!force && using_existing_nf)
            {
              printf (_("Replace expire-by-director-message setting (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              if (strcmp (contents, N_("On")) == 0)
                opts->expire_by_dirmsg = DIRON;
              else if (strcmp (contents, N_("Off")) == 0)
                opts->expire_by_dirmsg = DIROFF;
              else if (strcmp (contents, N_("Either")) == 0)
                opts->expire_by_dirmsg = DIRNOCARE;
              else if (strcmp (contents, N_("Default")) == 0)
                opts->expire_by_dirmsg = DIRDFLT;
              else
                {
                  fprintf (stderr, _("Unknown value for Expiration-Status field.\n"));
                  return -1;
                }
            }
          break;

        case WORKING_SET_SIZE:
          if (!force && using_existing_nf)
            {
              printf (_("Replace maximum number of notes in notesfile? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              opts->minimum_notes = strtol (contents, NULL, 10);
              if (opts->minimum_notes < 0)
                {
                  fprintf (stderr, _("Invalid value for Working-Set-Size field.\n"));
                  return -1;
                }
            }
          break;

        case LONGEST_TEXT:
          if (!force && using_existing_nf)
            {
              printf (_("Replace value for the maximum length of a note (y/n)? "));
              if (yesno ())
                replace_flag = TRUE;
            }
          if (!using_existing_nf || replace_flag)
            {
              opts->maximum_note_size = strtol (contents, NULL, 10);
              if (opts->maximum_note_size < 0)
                {
                  fprintf (stderr, _("Invalid value for Longest-Text field.\n"));
                  return -1;
                }
            }
          break;

        case POLICY_EXISTS:
          if (strcmp (contents, N_("Yes")) == 0)
            policy_exists = TRUE;
          break;

        case DESCRIPTOR_FINISHED:
          return 0;

        case ACCESS_RIGHT:
        case ACCESS_FINISHED:
          fprintf (stderr, _("Read an unexpected access entry.\n"));
          return -1;

        case NOTE:
        case RESPONSE:
          fprintf (stderr, _("Read an unexpected note or response.\n"));
          return -1;

        case ERROR:
          fprintf (stderr, _("Received an error parsing token.\n"));
          return -1;

        default:
          fprintf (stderr, _("Unknown token type.\n"));
          return -1;
        }
    }

  /* Should never get here. */
  return -1;
}

int
load_uiuc_access (struct notesfile *nf)
{
  register int field;
  List access_list;
  int entries = get_access_list (nf->ref, &access_list);
  int i;

  if (replace_access && !skip_access)
    {
      /* Clear out the old ... */

      for (i = 0; i < entries; i++)
        {
          list_remove_next (&access_list, NULL, NULL);
        }

      if (debug)
        fprintf (stderr, _("Cleared out old access privileges.\n"));
    }

  while (TRUE)
    {
      field = yylex ();
      switch (field)
        {
        case NOP:
          break;

        case ACCESS_RIGHT:
          {
            ListNode *node;
            int okay = force_access;
            char *token_name, *token_type, *token_mode;
            char *name;
            enum newts_access_scopes scope;
            int mode = 0;

            if (skip_access)
              {
                if (debug)
                  fprintf (stderr, _("Skipped an access privilege.\n"));
                break;
              }

            token_type = contents;
            token_mode = strrchr (contents, '=');
            *(token_mode++) = '\0';
            token_name = strrchr (contents, ':');
            *(token_name++) = '\0';

            if (strcmp (token_type, N_("User")) == 0)
              {
                scope = SCOPE_USER;
                *token_type = 'u';
              }
            else if (strcmp (token_type, N_("Group")) == 0)
              {
                scope = SCOPE_GROUP;
                *token_type = 'g';
              }
            else if (strcmp (token_type, N_("System")) == 0)
              {
                scope = SCOPE_SYSTEM;
                *token_type = 's';
              }
            else
              {
                fprintf (stderr, _("Read a bad access privilege; moving on.\n"));
                break;
              }

            if (strchr (token_mode, 'd') != NULL)
              {
                mode |= DIRECTOR;
              }
            if (strchr (token_mode, 'r') != NULL)
              {
                mode |= READ;
              }
            if (strchr (token_mode, 'w') != NULL)
              {
                mode |= WRITE;
              }
            if (strchr (token_mode, 'a') != NULL)
              {
                mode |= REPLY;
              }

            name = token_name;

            if (!force_access)
              {
                printf (_("Set %s permission for %s to %s (y/n)? "),
                        token_type, token_name, token_mode);
                if (yesno ())
                  okay = TRUE;
              }

            if (okay)
              {
                struct access *nodedata, *existing_access = NULL;

                node = list_head (&access_list);
                for (i=0; i<entries; i++)
                  {
                    if (node == NULL)
                      break;
                    nodedata = (struct access *) list_data (node);
                    if (nodedata->scope == scope &&
                        strcmp (nodedata->name, name) == 0)
                      {
                        existing_access = nodedata;
                        break;
                      }
                    node = list_next (node);
                  }

                if (existing_access)
                  {
                    access_set_permissions (existing_access, mode);
                  }
                else
                  {
                    struct access *new_access = access_alloc ();

                    access_set_permissions (new_access, mode);
                    access_set_scope (new_access, scope);
                    access_set_name (new_access, name);

                    list_insert_sorted (&access_list, (void *) new_access);
                    entries++;
                  }
                if (debug)
                  fprintf (stderr, _("Loaded access privilege for '%s'.\n"),
                           name);
              }
          }
          break;

        case ACCESS_FINISHED:
          if (!skip_access)
            write_access_list (nf->ref, &access_list);

          list_destroy (&access_list);
          return 0;

        case TITLE:
        case DIRECTOR_MESSAGE:
        case STATUS:
        case EXPIRATION_AGE:
        case EXPIRATION_ACTION:
        case EXPIRATION_STATUS:
        case WORKING_SET_SIZE:
        case LONGEST_TEXT:
        case POLICY_EXISTS:
        case DESCRIPTOR_FINISHED:
          fprintf (stderr, _("Read an unexpected descriptor field.\n"));
          list_destroy (&access_list);
          return -1;

        case NOTE:
        case RESPONSE:
          fprintf (stderr, _("Read an unexpected note or response.\n"));
          list_destroy (&access_list);
          return -1;

        case ERROR:
          fprintf (stderr, _("Received an error parsing token.\n"));
          list_destroy (&access_list);
          return -1;

        default:
          fprintf (stderr, _("Unknown token type.\n"));
          list_destroy (&access_list);
          return -1;
        }
    }

  /* Should never get here. */
  list_destroy (&access_list);
  return -1;
}
