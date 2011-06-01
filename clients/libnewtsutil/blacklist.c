/*
 * blacklist.c - routines to handle blacklist/whitelist
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005 Tyler Berry
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

#include "newts/list.h"
#include "newts/newts.h"

extern short no_blacklist;
extern short white_basenotes;

/* Struct representing blacklist data. */
struct blacklist_entry
{
  char *username;
  char *nf;
};

/* The user's blacklist. */
List blacklist;

/* The user's whitelist. */
List whitelist;

inline short blacklisted (struct newt * note);
static struct blacklist_entry *alloc_blacklist_entry (void);
static void free_blacklist_entry (struct blacklist_entry *entry);
void init_blacklist (void);

/* init_blacklist - initialize the blacklist and whitelist structures,
 * including sifting through the relevant environment variables (NFWHITELIST
 * and NFBLACKLIST). */

void
init_blacklist (void)
{
  struct blacklist_entry *entry;
  char *temp, *parsestr, *token, *subtoken, *subsubtoken;
  char *list, *item, *subitem;
  char *username = NULL;

  temp = getenv ("NFBLACKLIST");
  if (temp != NULL)
    parsestr = newts_strdup (temp);
  else
    parsestr = NULL;

  list_init (&blacklist,
             (void * (*) (void)) alloc_blacklist_entry,
             (void (*) (void *)) free_blacklist_entry,
             NULL);

  if (parsestr != NULL)
    {
      token = strtok_r (parsestr, " ", &list);

      while (token != NULL)
        {
          if (*token == ':')
            {
              /* ":nf,nf, ..." format */

              token++;
              subtoken = strtok_r (token, ",", &item);

              while (subtoken != NULL)
                {
                  entry = list_alloc_item (&blacklist);
                  entry->username = NULL;
                  entry->nf = newts_strdup (subtoken);

                  list_insert_next (&blacklist, list_tail (&blacklist),
                                    (void *) entry);

                  subtoken = strtok_r (NULL, ",", &item);
                }
            }
          else
            {
              subtoken = strtok_r (token, ":", &item);
              username = newts_strdup (subtoken);
              subtoken = strtok_r (NULL, ":", &item);

              if (subtoken == NULL)
                {
                  /* "user" format */

                  entry = list_alloc_item (&blacklist);
                  entry->username = newts_strdup (username);
                  entry->nf = NULL;
                  list_insert_next (&blacklist, list_tail (&blacklist),
                                    (void *) entry);
                }
              else
                {
                  /* "user:nf,nf, ..." format */

                  subsubtoken = strtok_r (subtoken, ",", &subitem);

                  while (subsubtoken != NULL)
                    {
                      entry = list_alloc_item (&blacklist);
                      entry->username = newts_strdup (username);
                      entry->nf = newts_strdup (subsubtoken);

                      list_insert_next (&blacklist, list_tail (&blacklist),
                                        (void *) entry);

                      subsubtoken = strtok_r (NULL, ",", &subitem);
                    }
                }

              newts_free (username);
            }

          token = strtok_r (NULL, " ", &list);
        }
    }

  newts_free (parsestr);

  temp = getenv ("NFWHITELIST");
  if (temp != NULL)
    parsestr = newts_strdup (temp);
  else
    parsestr = NULL;

  list_init (&whitelist,
             (void * (*) (void)) alloc_blacklist_entry,
             (void (*) (void *)) free_blacklist_entry,
             NULL);

  if (parsestr != NULL)
    {
      token = strtok_r (parsestr, " ", &list);

      while (token != NULL)
        {
          if (*token == ':')
            {
              /* ":nf,nf, ..." format */

              subtoken = strtok_r (token, ",", &item);

              while (subtoken != NULL)
                {
                  entry = list_alloc_item (&whitelist);
                  entry->username = NULL;
                  entry->nf = newts_strdup (subtoken);

                  list_insert_next (&whitelist, list_tail (&whitelist),
                                    (void *) entry);

                  subtoken = strtok_r (NULL, ",", &item);
                }
            }
          else
            {
              subtoken = strtok_r (token, ":", &item);
              username = newts_strdup (subtoken);
              subtoken = strtok_r (NULL, ":", &item);

              if (subtoken == NULL)
                {
                  /* "user" format */

                  entry = list_alloc_item (&whitelist);
                  entry->username = newts_strdup (username);
                  entry->nf = NULL;
                  list_insert_next (&whitelist, list_tail (&whitelist),
                                    (void *) entry);
                }
              else
                {
                  /* "user:nf,nf, ..." format */

                  subsubtoken = strtok_r (subtoken, ",", &subitem);

                  while (subsubtoken != NULL)
                    {
                      entry = list_alloc_item (&whitelist);
                      entry->username = newts_strdup (username);
                      entry->nf = newts_strdup (subsubtoken);

                      list_insert_next (&whitelist, list_tail (&whitelist),
                                        (void *) entry);

                      subsubtoken = strtok_r (NULL, ",", &subitem);
                    }
                }

              newts_free (username);
            }

          token = strtok_r (NULL, " ", &list);
        }
    }

  newts_free (parsestr);

  return;
}

static struct blacklist_entry *
alloc_blacklist_entry (void)
{
  return (struct blacklist_entry *) newts_malloc (sizeof (struct blacklist_entry));
}

/* free_blacklist_entry - free a blacklist entry. */

static void
free_blacklist_entry (struct blacklist_entry *entry)
{
  if (entry->username != NULL)
    newts_free (entry->username);
  if (entry->nf != NULL)
    newts_free (entry->nf);
  newts_free (entry);

  return;
}

/* blacklisted - check a particular note against the blacklist.
 *
 * Returns: TRUE if the note's blacklisted, or FALSE otherwise.
 */

inline short
blacklisted (struct newt *note)
{
  ListNode *node;
  struct blacklist_entry *entry;

  if (no_blacklist)     /* Blacklisting turned off with command-line switch. */
    return FALSE;

  if (note->nr.notenum == 0)    /* Policy notes are immune to blacklisting. */
    return FALSE;

  /* This is a check for the "white-basenotes" command-line option. */

  if (note->nr.respnum == 0 && white_basenotes)
    return FALSE;

  /* The whitelist overrules the blacklist.  If we match in the whitelist,
   * return FALSE (no, not blacklisted) immediately.
   */

  node = list_head (&whitelist);

  while (node != NULL)
    {
      entry = list_data (node);

      if (entry->username == NULL)
    {
      if (entry->nf != NULL)
        if (strcmp (entry->nf, note->nr.nfr.name) == 0)
          return FALSE;
    }
      else
    {
      if (strcmp (note->auth.name, entry->username) == 0)
        if (entry->nf == NULL
        || strcmp (entry->nf, note->nr.nfr.name) == 0)
          return FALSE;
    }
      node = list_next (node);
    }

  /* It's not in the whitelist.  So recurse through the blacklist; if we find
   * it there, return TRUE.
   */

  node = list_head (&blacklist);

  while (node != NULL)
    {
      entry = list_data (node);

      if (entry->username == NULL)
    {
      if (entry->nf != NULL)
        if (strcmp (entry->nf, note->nr.nfr.name) == 0)
          return TRUE;
    }
      else
    {
      if (strcmp (note->auth.name, entry->username) == 0)
        if (entry->nf == NULL
        || strcmp (entry->nf, note->nr.nfr.name) == 0)
          return TRUE;
    }
      node = list_next (node);
    }

  /* If no match, it's not blacklisted. */

  return FALSE;
}
