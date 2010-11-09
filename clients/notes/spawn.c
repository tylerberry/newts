/*
 * spawn.c - run a subprocess via execv or execl
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry
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

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif

#if STDC_HEADERS || __STDC__
# include <stdarg.h>
# define VA_START(ap, f) va_start (ap, f)
#elif HAVE_VARARGS_H
# include <varargs.h>
# define VA_START(ap, f) va_start (ap)
#else
# define va_alist a1, a2, a3, a4, a5, a6, a7, a8
# define va_dcl char *a1, *a2, *a3, *a4, *a5, *a6, *a7, *a8;
#endif

#include "curses_wrapper.h"
#include "dirname.h"
#include "notes.h"
#include "signals.h"

static inline void announce_error_forking (void);

/* spawn_process - spawn a new process, taking care of fork/exec and all other
 * necessary overhead.  If non-NULL, the pathname given as STDIN_FILE will be
 * used by the spawned process as stdin.  PATH is the binary to exec, and the
 * variadic parameters are string arguments to the binary.  It is not necessary
 * to provide the pathname a second time as the 0th argument; that name will be
 * extracted automatically.  The variadic list must end in NULL.
 *
 * Returns: -1 on inability to fork; otherwise, returns the return value of the
 * spawned process.
 */

int
#if STDC_HEADERS
spawn_process (const char *stdin_file, const char *path, ...)
#else
spawn_process (stdin_file, path, va_alist)
     const char *stdin_file;
     const char *path;
     va_dcl
#endif
{
  bool was_in_curses;
  int status;
  int pid;

#ifdef VA_START
  va_list ap;
#endif

  ignore_signals ();

  was_in_curses = currently_in_curses ();

  if (was_in_curses)
    {
      exit_curses ();
    }

  pid = fork ();
  switch (pid)
    {
    case -1:
      if (was_in_curses)
        {
          reenter_curses ();
        }
      handle_signals ();
      announce_error_forking ();
      return -1;

    case 0:
      setegid (real_gid);
      seteuid (real_uid);

      if (stdin_file)
        freopen (stdin_file, "r", stdin);

      {
#ifdef VA_START
#define INITIAL_ARGV_MAX 1024
        size_t argv_max = INITIAL_ARGV_MAX;
        const char *initial_argv[INITIAL_ARGV_MAX];
        const char *kludge;
        const char **argv = initial_argv;
        unsigned i = 0;

        VA_START (ap, path);

        argv[0] = base_name (path);

        while (argv[i++] != NULL)
          {
            if (i == argv_max)
              {
                argv_max *= 2;
                const char **nptr = realloc (argv == initial_argv ? NULL : argv,
                                             argv_max * sizeof (const char *));
                if (nptr == NULL)
                  {
                    if (argv != initial_argv)
                      free (argv);
                    _exit (EXIT_FAILURE);
                  }
                if (argv == initial_argv)
                  /* We have to copy the already filled-in data ourselves.  */
                  memcpy (nptr, argv, i * sizeof (const char *));

                argv = nptr;
              }

            kludge = va_arg (ap, const char *);
            argv[i] = kludge;
          }

        va_end (ap);

        execv (path, (char * const *) argv);

        if (argv != initial_argv)
          free (argv);
#else
        execl (path, va_alist);
#endif
        _exit (EXIT_FAILURE);
      }

    default:
      TEMP_FAILURE_RETRY (waitpid (pid, &status, 0));

      if (was_in_curses)
        {
          reenter_curses ();
        }

      handle_signals ();

      return status;
    }
}

/* spawn_subshell - spawn a subshell.
 *
 * Return values are the same as for spawn_process.
 */

inline int
spawn_subshell (void)
{
  return spawn_process (NULL, shell, NULL);
}

inline void
announce_error_forking (void)
{
  status_message (9, _("Error forking."));
}
