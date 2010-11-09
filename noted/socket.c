/*
 * socket.c - functions to handle the BSD socket interface
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2002, 2003 Tyler Berry.
 *
 * This file is heavily indebted to the select(2) info manual and
 * various other info nodes.
 *
 * Newts is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * Newts is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Newts; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifdef HAVE_CONFIG_H
# include <config.h>
#endif

#include "common.h"

#ifdef HAVE_SYS_SOCKET_H
# include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
# include <netinet/in.h>
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

#include <sys/un.h>

#define PORT 9734

static int read_from_client (int fd);

int
create_socket (uint16_t port)
{
  int sock;
  struct sockaddr_un name;

  sock = socket (PF_UNIX, SOCK_STREAM, 0);

  if (sock < 0)
    {
      perror ("noted: socket");
      exit (EXIT_FAILURE);
    }

  name.sun_family = AF_UNIX;
  strcpy (name.sun_path, "/tmp/newts-sock");

  if (bind (sock, (struct sockaddr *) &name, sizeof name) < 0)
    {
      perror ("noted: bind");
      exit (EXIT_FAILURE);
    }

  return sock;
}
     
int
run_server (int sock)
{
  fd_set active_fd_set, read_fd_set;
  int i;
  struct sockaddr_in clientname;
  size_t size;
     
  if (listen (sock, 1) < 0)
    {
      perror ("noted: listen");
      exit (EXIT_FAILURE);
    }
     
  FD_ZERO (&active_fd_set);
  FD_SET (sock, &active_fd_set);
     
  while (1)
    {
      read_fd_set = active_fd_set;

      fprintf (stderr, "Server waiting.\n");

      if (select (FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0)
	{
	  perror ("noted: select");
	  exit (EXIT_FAILURE);
	}
     
      for (i = 0; i < FD_SETSIZE; ++i)
	if (FD_ISSET (i, &read_fd_set))
	  {
	    if (i == sock)
	      {
		int new;

		size = sizeof (clientname);
		new = accept (sock, (struct sockaddr *) &clientname,
			      &size);
		if (new < 0)
		  {
		    perror ("noted: accept");
		    exit (EXIT_FAILURE);
		  }
		fprintf (stderr, "Connect found on %d assigned to %d\n", i, new);
		FD_SET (new, &active_fd_set);
	      }
	    else
	      {
		/* Data arriving on an already-connected socket. */
		if (read_from_client (i) < 0)
		  {
		    fprintf (stderr, "Closing fd %d\n", i);
		    close (i);
		    FD_CLR (i, &active_fd_set);
		  }
	      }
	  }
    }
  exit (EXIT_FAILURE);           /* We should never get here. */
}

static int
read_from_client (int sock)
{
  char buffer;
  int nbytes;
  int uid, gid;

  nbytes = read (sock, &buffer, 1);

  if (nbytes < 0)
    {
      perror ("noted: read");
      exit (EXIT_FAILURE);
    }
  else if (nbytes == 0)
    {
      return -1;
    }
  else
    {
      getpeereid (sock, &uid, &gid);
      fprintf (stderr, "Serving client uid=%d on %d: %c to %c\n", uid, sock,
	       buffer, buffer + 1);
      buffer++;
      sleep (5);
      write (sock, &buffer, 1);
      return 0;
    }
}
