/*
 * nfref.c - methods for handling the nfref data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 Tyler Berry.
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
# include <stddef.h>
#endif

#include "internal.h"
#include "newts/memory.h"

static inline void *newts_nmalloc_inline (size_t number, size_t size);
static inline void *newts_nrealloc_inline (void *pointer, size_t number,
                                           size_t size);
static inline void *newts_nrealloc2_inline (void *pointer, size_t *number,
                                            size_t size);

#ifndef SIZE_MAX
# define SIZE_MAX ((size_t) -1)
#endif

#define alloc_oversized(n, s) \
  ((size_t) (sizeof (ptrdiff_t) <= sizeof (size_t) ? -1 : -2) / (s) < (n))

void * (*newts_malloc_function) (size_t size) = malloc;
void * (*newts_realloc_function) (void *pointer, size_t size) = realloc;
void (*newts_free_function) (void *pointer) = free;
void (*newts_failed_malloc_hook) (void) = newts_malloc_die;
void (*newts_successful_malloc_hook) (void *pointer) = NULL;

void *
newts_calloc (size_t number, size_t size)
{
  return memset (newts_nmalloc (number, size), 0, size);
}

void
newts_free (void *pointer)
{
  newts_free_function (pointer);
}

void *
newts_malloc (size_t size)
{
  return newts_nmalloc_inline (1, size);
}

void
newts_malloc_die (void)
{
  abort ();
}

void *
newts_memdup (const void *pointer, size_t size)
{
  return memcpy (newts_malloc (size), pointer, size);
}

void *
newts_nmalloc (size_t number, size_t size)
{
  return newts_nmalloc_inline (number, size);
}

void *
newts_nrealloc (void *pointer, size_t number, size_t size)
{
  return newts_nrealloc_inline (pointer, number, size);
}

void *
newts_nrealloc2 (void *pointer, size_t *number, size_t size)
{
  return newts_nrealloc2_inline (pointer, number, size);
}

void *
newts_realloc (void *pointer, size_t size)
{
  return newts_nrealloc_inline (pointer, 1, size);
}

void *
newts_realloc2 (void *pointer, size_t *size)
{
  return newts_nrealloc2_inline (pointer, size, 1);
}

char *
newts_strdup (const char *string)
{
  return newts_memdup (string, strlen (string) + 1);
}

void *
newts_zalloc (size_t size)
{
  return memset (newts_malloc (size), 0, size);
}

static inline void *
newts_nmalloc_inline (size_t number, size_t size)
{
  void *pointer;

  if (alloc_oversized (number, size) ||
      (!(pointer = newts_malloc_function (number * size)) && number != 0))
    {
      if (newts_failed_malloc_hook)
        newts_failed_malloc_hook ();
    }
  else
    {
      if (newts_successful_malloc_hook)
        newts_successful_malloc_hook (pointer);
    }

  return pointer;
}

static inline void *
newts_nrealloc_inline (void *pointer, size_t number, size_t size)
{
  if (alloc_oversized (number, size) ||
      (!(pointer = newts_realloc_function (pointer, number * size)) &&
       number != 0))
    {
      if (newts_failed_malloc_hook)
        newts_failed_malloc_hook ();
    }
  else
    {
      if (newts_successful_malloc_hook)
        newts_successful_malloc_hook (pointer);
    }

  return pointer;
}

static inline void *
newts_nrealloc2_inline (void *pointer, size_t *number, size_t size)
{
  size_t n = *number;

  if (!pointer)
    {
      if (!n)
        {
          /* The approximate size to use for initial small allocation
           * requests, when the invoking code specifies an old size of
           * zero.  64 bytes is the largest "small" request for the
           * GNU C library malloc.
           */
          enum { DEFAULT_MXFAST = 64 };

          n = DEFAULT_MXFAST / size;
          n += !n;
        }
    }
  else
    {
      if (SIZE_MAX / 2 / size < n)
        {
          if (newts_failed_malloc_hook)
            newts_failed_malloc_hook ();
          return NULL;
        }
      n *= 2;
    }

  *number = n;
  return newts_realloc (pointer, n * size);
}
