/*
 * newts.h - definitions of newts internal structures
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

#ifndef FRONTEND_H
#define FRONTEND_H

#include "internal.h"

#include "newts/newts.h"

#if HAVE_LOCALE_H
# include <locale.h>
#endif

/* Global variables used by the frontends. */

extern uid_t anon_uid;
extern uid_t euid;
extern char *fqdn;
extern uid_t notes_uid;
extern char *program_name;
extern char *tmpdir;
extern char *username;

extern short allowed (struct notesfile *nf, short mode);
extern inline short blacklisted (struct newt *note);
extern void init_blacklist (void);
extern inline int list_parse (char *buf, int *p, int *first, int *last);
extern inline int list_convert (char *buf, int *p);
extern int parse_nf (char *text, List *list);
extern void printf_version_string (char *program_name);
extern void setup (void);
extern void sprint_time (char *buffer, struct tm *time);
extern void teardown (void);

#endif /* not FRONTEND_H */
