/*
 * getpeereid.c - gets a local connection's euid and egid
 *
 * This file is part of Libibby.
 * Copyright (C) 2003 Tyler Berry
 *
 * Libibby is free software; you can redistribute it and/or modify it under the
 * terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * Libibby is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * Libibby; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#ifndef HAVE_GETPEEREID

#include <sys/socket.h>

/* getpeereid is available as a system call on certain systems,
 * such as recent releases of FreeBSD. (FIXME: check that.)
 * Linux doesn't provide the equivalent system call, but provides
 * the SO_PEERCRED option to getsockopt that allows user-space
 * emulation.
 */

int
getpeereid (int sock, uid_t *euid, gid_t *egid)
{
#ifdef SO_PEERCRED
  struct ucred credential;
  socklen_t len = sizeof (credential);

  if (getsockopt (sock, SOL_SOCKET, SO_PEERCRED, &credential, &len) == 0)
    {
      *euid = credential.uid;
      *egid = credential.gid;
      return 0;
    }
#endif

  return -1;
}

#endif

