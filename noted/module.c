/*
 * module.c - wrapper for dynamic loading with ltdl
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2002, 2003 Tyler Berry.
 *
 * Based on sic/module.c from "GNU Autoconf, Automake and Libtool"
 * Copyright (C) 2000 Gary V. Vaughan, distributed under the GPL.
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

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "common.h"
#include "ltdl.h"

#define NEWTS_MODULE_PATH_ENV "NEWTS_MODULE_PATH"

static char multi_init_error[] = "module loader initialised more than once";

static const char *last_error = NULL;

const char *
module_error (void)
{
  return last_error;
}

int
module_init (void)
{
  static int initialized = 0;
  int errors = 0;

  /* Only perform initialization one time. */

  if (!initialized)
    {
      lt_dlmalloc = (lt_ptr (*) (size_t)) xmalloc;
      lt_dlfree = (void (*) (lt_ptr)) free;

      LTDL_SET_PRELOADED_SYMBOLS();

      errors = lt_dlinit();

      if (!errors)
	{
	  const char *path = getenv (NEWTS_MODULE_PATH_ENV);

	  if (path != NULL)
	    {
	      errors = lt_dladdsearchdir (path);
	    }
	}

      if (!errors)
	{
	  errors = lt_dladdsearchdir (MODULE_PATH);
	}

      if (!errors)
	{
	  last_error = lt_dlerror();
	}

      ++initialized;

      return errors ? EXIT_FAILURE : EXIT_SUCCESS;
    }

  last_error = multi_init_error;
  return EXIT_FAILURE;
}
