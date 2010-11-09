/*
 * autoseq.c - wrapper program for the Newts sequencer
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004 Tyler Berry
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

#if STDC_HEADERS
# include <stdlib.h>
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

int
main (int argc, char **argv)
{
  int i;

  char **newargv = (char **) malloc (sizeof (char *) * (argc + 2));
  if (newargv == NULL)
    exit (1);
  newargv[0] = NOTESBINARY;
  newargv[1] = "-s";
  for (i=1; i < argc; i++)
    newargv[i+1] = argv[i];
  newargv[argc + 1] = NULL;

  execv (NOTESBINARY, newargv);
  exit (1);
}
