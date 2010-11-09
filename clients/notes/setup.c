/*
 * setup.c - set up global variables for the client
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

#include "internal.h"

#include "newts/memory.h"
#include "newts/util.h"
#include "which.h"

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* "Anonymous"'s UID. */
uid_t anon_uid;

/* A string to hold the most recent author search. */
char *asearch;

/* Which editor to invoke for editing notes. */
char *editor;

/* The effective user ID. */
uid_t euid;

/* The fully-qualified domain name. */
char *fqdn;

/* Our user's home directory. */
char *homedir;

/* Which mailer our user prefers. */
char *mailer;

/* Messages to display after we're done fussing around with curses. */
char *messages;

/* The notes system's effective GID. */
gid_t notes_gid;

/* The notes administrator's UID. */
uid_t notes_uid;

/* The user's preferred pager. */
char *pager;

/* The real GID of the owner of this process. */
gid_t real_gid;

/* The real UID of the owner of this process. */
uid_t real_uid;

/* The sequencer or subsequencer username */
char *seqname;

/* Which shell our user prefers. */
char *shell;

/* Which program to execute for user-to-user talk. */
char *talk;

/* Which directory to create temporary files in. */
char *tmpdir;

/* A string to hold the most recent title search. */
char *tsearch;

/* A string to hold the most recent text search. */
char *txtsearch;

/* The effective username. */
char *username;

/* setup - set up global variables and clear the environment. */

void
setup (void)
{
  char *temp, *p;
  struct passwd *pw;
  size_t len;

  /* Store the real GID and UID for future use, along with the GID of the notes
   * system.*/

  real_uid = getuid ();
  real_gid = getgid ();
  notes_gid = getegid ();

  /* Drop setgid privileges for now. */

#ifdef _POSIX_SAVED_IDS
  //setegid (real_gid);
#else
  //setregid (getgid (), getegid ());
#endif

  /* If we're running as root, seteuid to notes. */

  pw = getpwnam (NOTES);
  notes_uid = pw->pw_uid;

  if (geteuid () == 0)
    seteuid (notes_uid);
  endpwent ();

  pw = getpwnam (ANON);
  anon_uid = pw->pw_uid;
  endpwent ();

  pw = getpwuid (geteuid ());
  homedir = newts_strdup (pw->pw_dir);
  euid = pw->pw_uid;
  username = newts_strdup (pw->pw_name);
  seqname = newts_strdup (pw->pw_name);
  shell = which ((temp = getenv ("SHELL")) ? temp : pw->pw_shell);
  endpwent ();

  tmpdir = newts_strdup ((temp = getenv ("TMPDIR")) ? temp : "/tmp");

  /* FIXME: Need a leetle more sophistication here.
   *
   * Specifically, we aren't handling arguments, and we ought to.
   */

  if ((temp = getenv ("NFEDITOR")) == NULL)
    if ((temp = getenv ("NFED")) == NULL)   /* Holdover from UIUC */
      if ((temp = getenv ("VISUAL")) == NULL)
        if ((temp = getenv ("EDITOR")) == NULL)
          temp = EDITOR;                       /* Set by Autoconf */

  p = strchr (temp, ' ');
  if (p == NULL)
    {
      editor = which (temp);
      if (editor == NULL)
        editor = which (EDITOR);
    }
  else
    {
      char *q;

      if (temp > p)
        len = (size_t) (temp - p);
      else
        len = (size_t) (p - temp);
      q = newts_nmalloc (len +1, sizeof (char));
      strncpy (q, temp, len);
      q[(int) len] = '\0';

      editor = which (q);
      newts_free (q);
    }

  fqdn = newts_get_fqdn ();

  if ((temp = getenv ("NFTALK")) == NULL)
    if ((temp = getenv ("TALK")) == NULL)
      temp = TALK;                                 /* Set by Autoconf */

  talk = which (temp);
  if (talk == NULL)
    talk = which (TALK);

  if ((temp = getenv ("NFMAILER")) == NULL)
    if ((temp = getenv ("MAILER")) == NULL)
      temp = MAILER;

  mailer = which (temp);
  if (mailer == NULL)
    mailer = which (MAILER);

  if ((temp = getenv ("NFPAGER")) == NULL)
    if ((temp = getenv ("PAGER")) == NULL)
      temp = PAGER;

  pager = which (temp);
  if (pager == NULL)
    pager = which (PAGER);

  asearch = tsearch = txtsearch = NULL;
  messages = newts_malloc (sizeof (char));
  *messages = '\0';

  /* FIXME: I should do something about the environment here.
   *
   * ... Namely, nuke it.
   */
}

void
teardown (void)
{
  newts_free (homedir);
  newts_free (tmpdir);
  newts_free (fqdn);
  newts_free (username);
  newts_free (seqname);
  newts_free (shell);
  newts_free (editor);
  newts_free (talk);
  newts_free (mailer);
  newts_free (pager);
  newts_free (messages);

  if (asearch)
    newts_free (asearch);
  if (tsearch)
    newts_free (tsearch);
  if (txtsearch)
    newts_free (txtsearch);
}
