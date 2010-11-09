/*
 * parse.c - parse a string into a newts_nfref
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

#include "internal.h"

#include "error.h"
#include "getline.h"
#include "newts/list.h"
#include "newts/newts.h"
#include "strtok_r.h"
#include "which.h"

#if HAVE_GLOB_H
# include <glob.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if STDC_HEADERS
# include <ctype.h>
# include <limits.h>
#endif

static int pattern_flag = FALSE;
static short sense;

int parse_file (char *filename, List *list);
int parse_nf (char *string, List *list);
static int parse_pattern (char *text, List *list);

enum senses
  {
    PARSE_ADD,
    PARSE_DELETE
  };

struct protocol_name_map
{
  char *name;
  enum newts_protocols protocol;
};

static struct protocol_name_map protocol_maps[] =
  {
    {N_("newts"), NEWTS_PROTOCOL_NCP},
    {NULL, 0}
  };

/** Here is the format we purport to support:
 **
 ** protocol://[[user@]system[:port]]/[owner:]nf
 **
 ** [=][[user@]system[:port]/][owner:]nf ->
 **    newts://[[user@]system[:port]]/[owner:]nf
 **
 ** Basically, the = prefix (or an NF absent an explicit protocol declaration,
 ** which is the = syntax minus the actual =, implies the use of the newts://
 ** protocol.
 **/

int
parse_single_nf (char *string, newts_nfref *ref)
{
  char *copy;         /* The beginning of the copied string. */
  char *mark;         /* Tagged location in the being-parsed string. */
  char *working_part; /* The beginning of the current part of the string. */

  if (string == NULL || ref == NULL)
    return -1;

  copy = newts_strdup (string);

  /* This hack prevents a crash if the inputted notesfile name ends in a slash.
   * This will happen sometimes with tab-completion.
   */

  while (*(copy + strlen (copy) - 1) == '/')
    {
      *(copy + strlen (copy) - 1) = '\0';
    }

  mark = strstr (copy, N_("://"));
  if (mark == NULL)
    {
      /* "Short" reference; the prefix newts:// is implied. */

      nfref_set_protocol (ref, NEWTS_PROTOCOL_NCP);
      working_part = copy;

      if (*working_part == '=')
        working_part++;
    }
  else
    {
      /* The protocol was explicitly specified. */

      unsigned protocol_length = mark - copy;
      struct protocol_name_map *map = protocol_maps;
      int found_flag = FALSE;

      while (map && map->name)
        {
          if (protocol_length == strlen (map->name) &&
              strncmp (copy, map->name, protocol_length) == 0)
            {
              nfref_set_protocol (ref, map->protocol);
              found_flag = TRUE;
            }

          map++;
        }

      if (!found_flag)
        {
          newts_free (copy);
          return -1;
        }

      working_part = mark + 3; /* This will put us just past the "://". */
    }

  /* At this point, we've parsed the protocol and main_part has been set to
   * point at the section after the protocol:// or the optional `=' if it was
   * included.
   */

  mark = strchr (working_part, '/');
  if (mark == NULL)
    {
      /* No slash means just the notesfile was specified, implying localhost,
       * current user, and default port.
       */

      nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
      nfref_set_system (ref, NULL);
      nfref_set_user (ref, NULL);
    }
  else
    {
      /* If there's a slash, the user has included a hostname, and possibly a
       * username and port as well.
       */

      char *host_part = working_part;

      working_part = mark + 1; /* This will put us just past the slash for the
                                * next section of parsing.
                                */

      *mark = '\0';

      mark = strchr (host_part, '@');
      if (mark == NULL)
        {
          /* No `@' means no user was specified. */

          nfref_set_user (ref, NULL);
        }
      else
        {
          char *user_part = host_part;

          host_part = mark + 1;
          *mark = '\0';

          nfref_set_user (ref, user_part);
        }

      mark = strchr (host_part, ':');
      if (mark == NULL)
        {
          /* No `:' means no port was specified. */

          nfref_set_port (ref, NEWTS_NCP_STANDARD_PORT);
          nfref_set_system (ref, host_part);
        }
      else
        {
          char *port_part = mark + 1;
          long value = strtol (port_part, NULL, 10);

          if (value < 1 || value > USHRT_MAX)
            {
              newts_free (copy);
              return -1;
            }

          *mark = '\0';

          nfref_set_port (ref, (unsigned short) value);
          nfref_set_system (ref, host_part);
        }
    }

  mark = strchr (working_part, ':');
  if (mark == NULL)
    {
      /* Just a notesfile name. */

      nfref_set_owner (ref, NULL);
      nfref_set_name (ref, working_part);
    }
  else
    {
      /* An owner was specified, i.e. this is a personal notesfile name. */

      char *nf_part = mark + 1;

      *mark = '\0';

      nfref_set_owner (ref, working_part);
      nfref_set_name (ref, nf_part);
    }

  newts_free (copy);
  return 0;
}

/* parse_nf - parse the given string, which may include patterns or sense
 * changes, into LIST.
 *
 * Returns: 0 if successful, -1 on error.
 */

int
parse_nf (char *string, List *list)
{
  newts_nfref *ref = nfref_alloc ();
  char *copy;

  if (string == NULL || list == NULL)
    return -1;

  copy = newts_strdup (string);

  if (!pattern_flag)
    sense = PARSE_ADD;

  if (*copy == ':')
    {
      return parse_file (copy + 1, list);
    }

  if (*copy == '!')
    {
      char *change = newts_strdup (copy + 1);
      newts_free (copy);
      copy = change;
      sense = PARSE_DELETE;
    }

  if (!pattern_flag &&
      (strchr (copy, '?') || strchr (copy, '[') || strchr (copy, '*') ||
       strchr (copy, ' ')))
    {
      /* Pattern matching. */

      int result;

      pattern_flag = TRUE;
      result = parse_pattern (copy, list);
      pattern_flag = FALSE;
      return result;
    }

  parse_single_nf (copy, ref);

  if (sense == PARSE_ADD)
    list_insert_next (list, list_tail (list), (void *) ref);
  else
    {
      list_remove_match (list, (void *) ref);
      nfref_free (ref);
    }

  newts_free (copy);

  return 0;
}

/* parse_pattern - given a string with wildcard characters in it Somewhere,
 * parse into a list of notesfiles.
 *
 * Returns: -1 for failure, 0 for okay.
 */

static int
parse_pattern (char *string, List *list)
{
  char *copy;

  if (string == NULL || list == NULL)
    return -1;

  copy = newts_strdup (string);

  /* No grepping for hidden files. That's cheating. */

  if (*copy == '.')
    return -1;

  if (strchr (copy, '/') == NULL)
    {
      /* "Short" reference: nf and (possibly) owner. */

      {
        /* Simplest case; just a notesfile name. */

        int i;
        glob_t parsed_list;
        char directory[PATH_MAX];

        getcwd (directory, PATH_MAX);
        chdir (SPOOL);

        glob (copy, 0, NULL, &parsed_list);

        chdir (directory);

        for (i = 0; i < parsed_list.gl_pathc; i++)
          {
            if (strchr (copy, ':') == NULL &&
                strchr (parsed_list.gl_pathv[i], ':') == NULL)
              parse_nf (parsed_list.gl_pathv[i], list);
          }

        globfree (&parsed_list);
      }
    }
  else
    {
      /* Long references; not supported yet. */

      newts_free (copy);
      return -1;
    }

  newts_free (copy);
  return 0;
}

/* parse_file - parse a file specified as ':FILE' for a further list of note
 * specifications.
 *
 * Returns: 0 if successful. On error, the program is terminated by a call to
 * the error library function.
 */

int
parse_file (char *filename, List *nflist)
{
  FILE *file;
  char *copy, *list, *token, *expanded_filename;
  char *line = NULL;
  size_t size;

  expanded_filename = tilde_expand (filename);
  if (expanded_filename == NULL)
    {
      error (-1, 0, "could not expand %s", filename);
    }

  file = fopen (expanded_filename, "r");
  if (file == NULL)
    {
      error (-1, 0, "could not open filename '%s'", filename);
    }

  while (getline (&line, &size, file) >= 0)
    {
      copy = newts_strdup (line);

      token = strtok_r (copy, ", \n", &list);
      while (token != NULL)
        {
          parse_nf (token, nflist);
          token = strtok_r (NULL, ", \n", &list);
        }

      newts_free (copy);
    }

  newts_free (line);
  newts_free (expanded_filename);
  return 0;
}
