/*
 * dump-uiuc.c - dump a UIUC-format notesfile to a UIUC-format dump image
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
 *
 * Based in part on dump.c from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad.  Any work derived from this source code is required to retain
 * this notice.
 *
 * Newts is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Newts is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
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

#include "dump-uiuc.h"
#include "newts/uiuc.h"
#include "newts/uiuc-compatibility.h"

static void uiuc_dump_descriptor (FILE *file, struct notesfile *nff);
static void uiuc_dump_access (FILE *file, struct notesfile *nf);
static void uiuc_dump_note (FILE *file, struct newt *np);
static void uiuc_dump_resp (FILE *file, struct newt *np, struct newt *rp,
                            int num);

int
uiuc_dump_nf (struct notesfile *nf)
{
  newts_nfref *ref = nf->ref;
  int i;
  char *filename;
  FILE *file;

  filename = newts_nmalloc (strlen (nfref_name (ref)) +
                            strlen (extension) + 2,
                            sizeof (char));

  strcpy (filename, nfref_name (ref));
  if (extension && extension[0])
    {
      strcat (filename, ".");
      strcat (filename, extension);
    }

  /* The seteuid back to root here guarantees that we can open the file with
   * root's permissions if we really are root.
   */

  if (getuid () == 0)
    seteuid (0);

  file = fopen (filename, "w");
  newts_free (filename);

  if (getuid () == 0)
    seteuid (notes_uid);

  if (file == NULL)
    return -1;

  {
    printf (_("Dumping '%s': "), nfref_pretty_name (ref));
  }

  uiuc_dump_descriptor (file, nf);
  uiuc_dump_access (file, nf);

  for (i = 1; i <= nf->total_notes; i++)
    {
      struct newt note;
      int j;

      memset (&note, 0, sizeof (struct newt));
      nfref_copy (&note.nr.nfr, ref);

      note.nr.notenum = i;
      note.nr.respnum = 0;

      if (get_note (&note, FALSE) == 0)
        uiuc_dump_note (file, &note);

      for (j = 1; j <= note.total_resps; j++)
        {
          struct newt resp;

          memset (&resp, 0, sizeof (struct newt));
          nfref_copy (&resp.nr.nfr, ref);

          resp.nr.notenum = i;
          resp.nr.respnum = j;

          if (get_note (&resp, FALSE) == 0)
            uiuc_dump_resp (file, &note, &resp, j);
        }
    }

  fclose (file);
  printf ("\n");

  return 0;
}

static void
uiuc_dump_descriptor (FILE *file, struct notesfile *nf)
{
  newts_nfref *ref = nf->ref;
  struct uiuc_opts *opts = (struct uiuc_opts *) nf->opts;
  struct stats *stats = stats_alloc ();
  char buffer[128];

  get_stats (ref, stats);

  fprintf (file, "NF-Title: %s\n", nf->title);
  fprintf (file, "NF-Director-Message: %s\n", nf->director_message);

  sprint_time (buffer, gmtime (&nf->modified));
  fprintf (file, "NF-Last-Modified: %s\n", buffer);

  fprintf (file, "NF-Status:");            /* status */
  {
    if (nf->options & NF_ANONYMOUS)
      fprintf (file, " Anonymous");
    if (!(nf->options & NF_LOCKED))
      fprintf (file, " Open");
    if (nf->options & NF_ARCHIVE)
      fprintf (file, " Archive");
  }
  putc ('\n', file);               /* end status line */

  fprintf (file, "NF-Id-Sequence: %d@%s\n", opts->current_note_id,
           "elysium");
  fprintf (file, "NF-Number: %d\n", opts->notesfile_number);

  sprint_time (buffer, gmtime (&stats->created));
  fprintf (file, "NF-Last-Transmit: %s\n", buffer);

  sprint_time (buffer, gmtime (&stats->created));
  fprintf (file, "NF-Created: %s\n", buffer);

  sprint_time (buffer, gmtime (&stats->last_used));
  fprintf (file, "NF-Last-Used: %s\n", buffer);

  fprintf (file, "NF-Days-Used: %u\n", stats->days_used);
  fprintf (file, "NF-Notes-Written: %u\n", stats->notes_written);
  fprintf (file, "NF-Notes-Read: %u\n", stats->notes_read);
  fprintf (file, "NF-Notes-Transmitted: %u\n", stats->notes_sent);
  fprintf (file, "NF-Notes-Received: %u\n", stats->notes_received);
  fprintf (file, "NF-Notes-Dropped: %u\n", stats->notes_dropped);
  fprintf (file, "NF-Responses-Written: %u\n", stats->notes_written);
  fprintf (file, "NF-Responses-Read: %u\n", stats->notes_read);
  fprintf (file, "NF-Responses-Transmitted: %u\n", stats->notes_sent);
  fprintf (file, "NF-Responses-Received: %u\n", stats->notes_received);
  fprintf (file, "NF-Responses-Dropped: %u\n", stats->notes_dropped);
  fprintf (file, "NF-Entries: %u\n", stats->entries);
  fprintf (file, "NF-Walltime: %u seconds\n", stats->total_time);
  fprintf (file, "NF-Orphans-Received: %u\n", stats->orphans_received);
  fprintf (file, "NF-Orphans-Adopted: %u\n", stats->orphans_adopted);
  fprintf (file, "NF-Transmits: 0\n");
  fprintf (file, "NF-Receives: 0\n");
  fprintf (file, "NF-Expiration-Age: %d Days\n", opts->expire_threshold);
  switch ((int) opts->expire_action)
    {
    case KEEPYES:
      strcpy (buffer, "Archive");
      break;
    case KEEPNO:
      strcpy (buffer, "Delete");
      break;
    default:
      strcpy (buffer, "Default");
      break;
    }
  fprintf (file, "NF-Expiration-Action: %s\n", buffer);
  switch ((int) opts->expire_by_dirmsg)
    {
    case DIRON:
      strcpy (buffer, "On");
      break;
    case DIROFF:
      strcpy (buffer, "Off");
      break;
    case DIRNOCARE:
      strcpy (buffer, "Either");
      break;
    default:
      strcpy (buffer, "Default");
      break;
    }
  fprintf (file, "NF-Expiration-Status: %s\n", buffer);
  fprintf (file, "NF-Working-Set-Size: %d\n", opts->minimum_notes);
  fprintf (file, "NF-Longest-Text: %d bytes\n", opts->maximum_note_size);

  fprintf (file, "NF-Policy-Exists: %s\n",
           nf->options & NF_POLICY ? "Yes" : "No");
  fprintf (file, "NF-Descriptor: Finished\n");

  if (nf->options & NF_POLICY)
    {
      struct newt note;

      memset (&note, 0, sizeof (struct newt));
      nfref_copy (&note.nr.nfr, ref);

      note.nr.notenum = 0;
      note.nr.respnum = 0;

      if (get_note (&note, FALSE) == 0)
        uiuc_dump_note (file, &note);
    }

  stats_free (stats);

  return;
}

static void
uiuc_dump_access (FILE *file, struct notesfile *nf)
{
  List access_list;
  ListNode *node;
  struct access *access_entry;
  char mode[5];
  char *scope;
  int entries = get_access_list (nf->ref, &access_list);
  int i;

  node = list_head (&access_list);

  for (i = 0; i < entries; i++)
    {
      access_entry = (struct access *) list_data (node);

      switch (access_scope (access_entry))
        {
        case SCOPE_USER:
          scope = "User";
          break;

        case SCOPE_GROUP:
          scope = "Group";
          break;

        case SCOPE_SYSTEM:
          scope = "System";
          break;

        default:
          scope = "Bizarro";
          break;
        }

      mode[0] = '\0';

      if (access_has_permissions (access_entry, DIRECTOR))
        strcat (mode, "d");
      if (access_has_permissions (access_entry, READ))
        strcat (mode, "r");
      if (access_has_permissions (access_entry, WRITE))
        strcat (mode, "w");
      if (access_has_permissions (access_entry, REPLY))
        strcat (mode, "a");

      fprintf (file, "Access-Right: %s:%s=%s\n", scope, access_name (access_entry), mode);
      node = list_next (node);
    }

  list_destroy (&access_list);

  fprintf (file, "NF-Access-Finished:\n");

  return;
}

static void
uiuc_dump_note (FILE *file, struct newt *np)
{
  struct tm *tm;
  int options = 0;

  if (np->options & NOTE_DELETED || np->options & NOTE_CORRUPTED)
    return;

  fprintf (file, "N:%s:%ld:%d\n",
           np->id.system, np->id.number, np->total_resps);

  fprintf (file, "%s\n", np->title);

  fprintf (file, "%s:%d:%s:\n", np->auth.name, np->auth.uid, np->auth.system);

  tm = gmtime (&np->created);
  fprintf (file, "%d:%d:%d:%d:%d:%ld:\n", tm->tm_year + 1900, tm->tm_mon + 1,
           tm->tm_mday, tm->tm_hour, tm->tm_min, np->created);
  fprintf (file, "%d:%d:%d:%d:%d:%ld:\n", tm->tm_year + 1900, tm->tm_mon + 1,
           tm->tm_mday, tm->tm_hour, tm->tm_min, np->created);
  tm = gmtime (&np->modified);
  fprintf (file, "%d:%d:%d:%d:%d:%ld:\n", tm->tm_year + 1900, tm->tm_mon + 1,
           tm->tm_mday, tm->tm_hour, tm->tm_min, np->modified);

  fprintf (file, "%s\n", np->id.system);

  if (np->director_message)
    options &= DIRMES;
  if (np->options & NOTE_WRITE_ONLY)
    options &= WRITONLY;
  if (np->options & NOTE_UNAPPROVED)
    options &= ISUNAPPROVED;

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
  fprintf (file, N_("0%03o:%zu\n"), options & 0377, strlen (np->text));
#else
  fprintf (file, N_("0%03o:%lu\n"), options & 0377,
           (unsigned long) strlen (np->text));
#endif

  fprintf (file, "%s", np->text);

  printf (":");

  return;
}

static void
uiuc_dump_resp (FILE *file, struct newt *np, struct newt *rp, int num)
{
  struct tm *tm;
  int options = 0;

  if (np->options & NOTE_DELETED || np->options & NOTE_CORRUPTED ||
      rp->options & NOTE_DELETED || rp->options & NOTE_CORRUPTED)
    return;

  fprintf (file, "R:%s:%ld:%s:%ld:%d\n", np->id.system, np->id.number,
           rp->id.system, rp->id.number, num);

  fprintf (file, "%s:%d:%s:\n", rp->auth.name, rp->auth.uid, rp->auth.system);

  tm = gmtime (&np->created);
  fprintf (file, "%d:%d:%d:%d:%d:%ld:\n", tm->tm_year + 1900, tm->tm_mon + 1,
           tm->tm_mday, tm->tm_hour, tm->tm_min, rp->created);
  fprintf (file, "%d:%d:%d:%d:%d:%ld:\n", tm->tm_year + 1900, tm->tm_mon + 1,
           tm->tm_mday, tm->tm_hour, tm->tm_min, rp->created);

  fprintf (file, "%s\n", rp->id.system);

  if (rp->director_message)
    options &= DIRMES;
  if (rp->options & NOTE_WRITE_ONLY)
    options &= WRITONLY;
  if (rp->options & NOTE_UNAPPROVED)
    options &= ISUNAPPROVED;

#if defined (__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
  fprintf (file, N_("%03o:%zu\n"), options & 0377, strlen (rp->text));
#else
  fprintf (file, N_("%03o:%lu\n"), options & 0377,
           (unsigned long) strlen (rp->text));
#endif

  fprintf (file, "%s", rp->text);

  if ((num - 1) % 10 == 0)
    printf (".");

  return;
}
