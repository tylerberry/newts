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
#include "newts/version.h"

#include "vasprintf.h"
#include "which.h"

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* "Anonymous"'s UID. */
uid_t anon_uid;

/* The effective user ID. */
uid_t euid;

/* The fully-qualified domain name. */
char *fqdn;

/* The notes administrator's UID. */
uid_t notes_uid;

/* The name the program was called */
char *program_name;

/* Which shell our user prefers. */
char *shell;

/* Which directory to create temporary files in. */
char *tmpdir;

/* The effective username. */
char *username;

void
printf_version_string (char *program_name)
{
  char *revision_string;

  asprintf (&revision_string, _("revision: %s"), newts_revision);
  printf (N_("%s - %s %s (%s)\n"), program_name, PACKAGE_NAME, VERSION,
          revision_string);

  free (revision_string);
}

void
setup (void)
{
  char *temp;
  struct passwd *pw;

  /* If we're running as root, seteuid to notes. */

  pw = getpwnam (NOTES);
  notes_uid = pw->pw_uid;

  if (geteuid () == 0)
    seteuid (notes_uid);

  pw = getpwnam (ANON);
  anon_uid = pw->pw_uid;

  pw = getpwuid (geteuid ());
  username = newts_strdup (pw->pw_name);
  euid = pw->pw_uid;

  fqdn = newts_get_fqdn ();

  tmpdir = newts_strdup ((temp = getenv ("TMPDIR")) ? temp : "/tmp");

  shell = which ((temp = getenv ("SHELL")) ? temp : pw->pw_shell);

  endpwent ();

  /* FIXME: I should do something about the environment here.
   *
   * ... Namely, nuke it.
   */
}

void
teardown (void)
{
  newts_free (username);
  newts_free (tmpdir);
  newts_free (shell);
}
