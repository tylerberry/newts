/*
 * internal.h - internal-use utility and portability functions and definitions
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2006 Tyler Berry.
 *
 * Originally based on common.h from "GNU Autoconf, Automake and Libtool"
 * Copyright (C) 2000 Gary V. Vaughan, distributed under the GPL.
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

#ifndef NEWTS_INTERNAL_H
#define NEWTS_INTERNAL_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if STDC_HEADERS
# include <errno.h>
# include <stdio.h>
# include <stdlib.h>
#endif

#ifndef errno
extern int errno;
#endif

#if STDC_HEADERS || HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_STDBOOL_H
# include <stdbool.h>
#endif

#if HAVE_SYS_PARAM_H
# include <sys/param.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "gettext.h"
#define N_(string) gettext_noop (string)
#define _(string) gettext (string)

#include "structs.h"

#ifndef FALSE
# define FALSE 0
# define TRUE  1
#endif

#ifndef TEMP_FAILURE_RETRY
# define TEMP_FAILURE_RETRY(expression) \
  (__extension__                                                              \
    ({ long int __result;                                                     \
       do __result = (long int) (expression);                                 \
       while (__result == -1L && ((errno == EINTR)|| (errno==EAGAIN)));       \
       __result; }))
#endif

#if HAVE_MKDIR
# if MKDIR_TAKES_ONE_ARG /* Mingw32 */
#  define mkdir(a,b) mkdir(a)
# endif
#else
# if HAVE__MKDIR /* plain Win32 */
#  define mkdir(a,b) _mkdir(a)
# else
#  error "Don't know how to create a directory on this system."
# endif
#endif

#if !HAVE_FDATASYNC
# define fdatasync(f) fsync ((f))
#endif

#if !HAVE_STRCHR
# if HAVE_INDEX
#  define strchr(s,c) index ((s),(c))
# else
#  error "No strchr or index."
# endif
#endif

#if !HAVE_STRRCHR
# if HAVE_RINDEX
#  define strrchr(s,c) rindex ((s),(c))
# else
#  error "No strrchr or rindex."
# endif
#endif

extern int getpeereid (int sock, uid_t *euid, gid_t *egid);

#if WITH_DMALLOC
# undef malloc
# undef realloc
# include <dmalloc.h>
#endif

#endif /* not NEWTS_INTERNAL_H */
