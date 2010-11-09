/*
 * memory.h - memory allocation functions and hooks
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

/** @file newts/memory.h
 * Memory allocation functions and hooks.
 *
 * These functions are used for all memory management performed by
 * libnewtsclient.  All other functions in the library dealing with memory
 * allocation or deallocation call a function defined in this file.
 *
 * The global function pointer variables in this file define the basic
 * interface to memory management.  If those variables are reassigned, the
 * higher-level interface to memory management in this file, and by extension
 * all other memory-related functions in libnewtsclient, will automatically use
 * the new functions stored in those variables.  If you need to handle memory
 * allocation in a special way, for example to accommodate requirements for
 * writing extensions for dynamic languages like Python, you can accomplish
 * that by redefining these variables.
 *
 * The higher-level interface provides error checking as well as numerous
 * specialized variations of the basic memory allocation functions.  If you're
 * directly allocating memory (as opposed to using structure-specific
 * allocation functions such as @ref alloc_nfref "alloc_nfref"), you should use
 * this higher-level interface.
 */

#ifndef NEWTS_MEMORY_H
#define NEWTS_MEMORY_H

#include "newts/config.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * A function that should be called if @ref newts_malloc_function
 * "newts_malloc_function" or @ref newts_realloc_function
 * "newts_realloc_function" fails to successfully allocate or reallocate
 * memory.  Defaults to @ref newts_malloc_die "newts_malloc_die".
 *
 * This function is permitted to abort execution, but is not required to, if
 * the client application prefers to raise an exception, attempt to recover
 * memory, or otherwise handle the allocation error gracefully.
 */
extern void (*newts_failed_malloc_hook) (void);

/**
 * The function that should be used to deallocate pointers allocated using
 * either @ref newts_malloc_function "newts_malloc_function" or @ref
 * newts_realloc_function "newts_realloc_function".  Defaults to free(3).
 *
 * This function is called by @ref newts_free "newts_free".
 */
extern void (*newts_free_function) (void *pointer);

/**
 * The function that should be used to allocate new blocks of memory.  Defaults
 * to malloc(3).
 *
 * This function is called by @ref newts_calloc "newts_calloc", @ref
 * newts_malloc "newts_malloc", @ref newts_memdup "newts_memdup", @ref
 * newts_nmalloc "newts_nmalloc", @ref newts_strdup "newts_strdup", and @ref
 * newts_zalloc "newts_zalloc".
 *
 * @sa newts_free_function
 */
extern void * (*newts_malloc_function) (size_t size);

/**
 * The function that should be used to re-allocate blocks of memory previously
 * allocated by @ref newts_malloc_function "newts_malloc_function".  Defaults
 * to realloc(3).
 *
 * This function is called by @ref newts_nrealloc "newts_nrealloc", @ref
 * newts_nrealloc2 "newts_nrealloc2", @ref newts_realloc "newts_realloc", and
 * @ref newts_realloc2 "newts_realloc2".
 *
 * @sa newts_free_function
 */
extern void * (*newts_realloc_function) (void *pointer, size_t size);

/**
 * A function that should be called if @ref newts_malloc_function
 * "newts_malloc_function" or @ref newts_realloc_function
 * "newts_realloc_function" successfully allocates or reallocates memory.
 * No default value.
 *
 * @param pointer The address of the newly allocated or reallocated memory.
 */
extern void (*newts_successful_malloc_hook) (void *pointer);

/**
 * Allocate and return sufficient memory for @e number items of @e size bytes
 * each, all initialized to 0.  @e size must be nonzero.
 */
extern void *newts_calloc (size_t number, size_t size);

/**
 * Free memory allocated by any of the allocation functions in this file.
 */
extern void newts_free (void *pointer);

/**
 * Allocate and return @e size bytes of uninitializd memory.
 */
extern void *newts_malloc (size_t size);

/**
 * Abort execution in the case of failed memory allocation.  This function is
 * the default value of @ref newts_failed_malloc_hook
 * "newts_failed_malloc_hook".
 */
extern void newts_malloc_die (void);

/**
 * Allocate and return @e size bytes of memory, and fill that memory with the
 * contents of the first @e size bytes of the memory starting at @e pointer.
 */
extern void *newts_memdup (const void *pointer, size_t size);

/**
 * Allocate and return sufficient uninitialized memory for @e number items of @e size
 * bytes each.  @e size must be nonzero.
 */
extern void *newts_nmalloc (size_t number, size_t size);

/**
 * If @e pointer is NULL, allocate and return sufficient uninitialized memory
 * for @e number items of @e size bytes each.  Otherwise, change the size of @e
 * pointer to accommodate @e number items of @e size bytes each, and return the
 * location of the newly resized block of memory.  @e size must be nonzero.
 */
extern void *newts_nrealloc (void *pointer, size_t number, size_t size);

/**
 * If @e pointer is NULL, allocate and return sufficient uninitialized memory
 * to accommodate, at minimum, @e number items of @e size bytes each.  If @e
 * pointer is NULL and @e number is also NULL, allocate and return memory for
 * some implementation-defined number of items of @e size bytes each.
 *
 * If @e pointer is not NULL, change the size of @e pointer to accommodate at
 * least @e number items of @e size bytes each, and return the location of the
 * newly resized block of memory.  If @e pointer is not NULL, @e size must not
 * be NULL.
 *
 * Repeated reallocations are guaranteed to "make progress", either by
 * allocating an initial block with a nonzero size, or by reallocating a larger
 * block.
 *
 * @par Side effects:
 * @e number is set to the number of items of size @e size allocated; this may
 * be greater than the number requested, but will never be fewer.  On return,
 * @e number will never contain a value of zero unless memory failed to be
 * allocated.
 */
extern void *newts_nrealloc2 (void *pointer, size_t *number, size_t size);

/**
 * If @e pointer is NULL, allocate and return @e size bytes of uninitialized
 * memory.  Otherwise, change the size of @e pointer to @e size bytes, and
 * return the location of the newly resized block of memory.
 */
extern void *newts_realloc (void *pointer, size_t size);

/**
 * If @e pointer is NULL, allocate and return sufficient uninitialized memory
 * to accommodate, at minimum, @e size bytes.  If @e pointer is NULL and @e
 * size is also NULL, allocate and return memory for some
 * implementation-defined number of bytes.
 *
 * If @e pointer is not NULL, change the size of @e pointer to accommodate at
 * least @e size bytes, and return the location of the newly resized block of
 * memory.  If @e pointer is not NULL, @e size must not be NULL.
 *
 * Repeated reallocations are guaranteed to "make progress", either by
 * allocating an initial block with a nonzero size, or by reallocating a larger
 * block.
 *
 * @par Side effects:
 * @e size is set to the number of bytes allocated; this may be greater than
 * the number requested, but will never be fewer.  On return, @e size will
 * never contain a value of zero unless memory failed to be allocated.
 */
extern void *newts_realloc2 (void *pointer, size_t *size);

/**
 * Allocate and return a copy of @e string.
 */
extern char *newts_strdup (const char *string);

/**
 * Allocate and return @e size bytes of memory, initialized to 0.
 */
extern void *newts_zalloc (size_t size);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_MEMORY_H */
