/*
 * structs.h - private struct definitions for opaque data types
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2008 Tyler Berry.
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

#ifndef NEWTS_STRUCTS_H
#define NEWTS_STRUCTS_H

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "newts/enums.h"

struct newts_nfref
{
  enum newts_protocols protocol; /**< The protocol to use to connect. */
  char *user;                    /**< The username to connect as. */
  char *system;                  /**< The system to connect to. */
  unsigned short port;           /**< The port to connect to. */
  char *owner;                   /**< The owner of the notesfile, as in
                                  * personal notesfiles. */
  char *name;                    /**< The name of the notesfile. */
  char *pretty_name;             /**< The cached name of the notesfile,
                                  * formatted according to the standards for
                                  * user input of nfref syntax. */
};

#endif /* not NEWTS_STRUCTS_H */
