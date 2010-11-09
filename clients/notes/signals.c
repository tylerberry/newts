/*
 * signals.c - handlers for various signals that notes could get sent
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
#include "curses_wrapper.h"

#if HAVE_SIGNAL_H
# include <signal.h>
#endif

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

static void sigcont_handler (int signum);
static void sigint_handler (int signum);
static void sigtstp_handler (int signum);
static sig_atomic_t was_in_curses;

void
sigcont_handler (int signum)
{
  int saved_errno = errno;

  if (was_in_curses)
    {
      reenter_curses ();
    }

  errno = saved_errno;
}

void
sigint_handler (int signum)
{
  return;
}

void
sigtstp_handler (int signum)
{
  int saved_errno = errno;

  if (currently_in_curses ())
    {
      exit_curses ();
      was_in_curses = 1;
    }
  else
    {
      was_in_curses = 0;
    }

  kill (0, SIGSTOP);

  errno = saved_errno;
}

void
handle_signals (void)
{
  struct sigaction cont_action;
  struct sigaction hup_action;
  struct sigaction int_action;
  struct sigaction quit_action;
  struct sigaction tstp_action;

  cont_action.sa_handler = sigcont_handler;
  sigemptyset (&cont_action.sa_mask);
  cont_action.sa_flags = 0;

  hup_action.sa_handler = SIG_DFL;
  sigemptyset (&hup_action.sa_mask);
  hup_action.sa_flags = 0;

  int_action.sa_handler = sigint_handler;
  sigemptyset (&int_action.sa_mask);
  int_action.sa_flags = 0;

  quit_action.sa_handler = SIG_DFL;
  sigemptyset (&quit_action.sa_mask);
  quit_action.sa_flags = 0;

  tstp_action.sa_handler = sigtstp_handler;
  sigemptyset (&tstp_action.sa_mask);
  tstp_action.sa_flags = 0;

  sigaction (SIGCONT, &cont_action, NULL);
  sigaction (SIGHUP, &hup_action, NULL);
  sigaction (SIGINT, &int_action, NULL);
  sigaction (SIGQUIT, &quit_action, NULL);
  sigaction (SIGTSTP, &tstp_action, NULL);
}

void
ignore_signals (void)
{
  struct sigaction cont_action;
  struct sigaction hup_action;
  struct sigaction int_action;
  struct sigaction quit_action;
  struct sigaction tstp_action;

  cont_action.sa_handler = SIG_IGN;
  sigemptyset (&cont_action.sa_mask);
  cont_action.sa_flags = 0;

  hup_action.sa_handler = SIG_IGN;
  sigemptyset (&hup_action.sa_mask);
  hup_action.sa_flags = 0;

  int_action.sa_handler = SIG_IGN;
  sigemptyset (&int_action.sa_mask);
  int_action.sa_flags = 0;

  quit_action.sa_handler = SIG_IGN;
  sigemptyset (&quit_action.sa_mask);
  quit_action.sa_flags = 0;

  tstp_action.sa_handler = SIG_DFL;
  sigemptyset (&tstp_action.sa_mask);
  tstp_action.sa_flags = 0;

  sigaction (SIGCONT, &cont_action, NULL);
  sigaction (SIGHUP, &hup_action, NULL);
  sigaction (SIGINT, &int_action, NULL);
  sigaction (SIGQUIT, &quit_action, NULL);
  sigaction (SIGTSTP, &tstp_action, NULL);
}
