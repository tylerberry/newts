/*
 * connection.h - definition of the connection data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2007, 2008 Tyler Berry.
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

/** @file newts/connection.h
 * connection datatype and related functions.
 */

#ifndef NEWTS_CONNECTION_H
#define NEWTS_CONNECTION_H

#include "newts/config.h"
#include "newts/enums.h"

/**
 * A description of a notes server, including all of the information needed to
 * connect to that server.
 */
struct connection
{
  enum newts_protocols protocol; /**< The protocol to use to connect. */
  char *user;                    /**< The username to connect as. */
  char *system;                  /**< The system to connect to. */
  unsigned short port;           /**< The port to connect to. */
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Create a new struct connection.
 *
 * @return A newly allocated struct connection. This should be freed with
 * connection_free when no longer needed.
 *
 * @sa connection_free
 */
extern struct connection *connection_alloc (void);

/**
 * Duplicate an existing connection. This is a deep copy; strings from @e
 * source will be copied, and @e dest will point to those copied strings, not
 * the originals from @e source. This function is safe to use if overwriting
 * existing values in @e dest.
 *
 * @param dest The connection to copy values to.
 * @param source The connection to copy values from.
 *
 * @par Side effects:
 * The values stored in @e dest will be modified. Additionally, if @e dest was
 * previously populated, its existing string elements will be freed before
 * being overwritten.
 */
extern void connection_copy (struct connection *dest,
                             const struct connection *source);

/**
 * Deallocate @e connection.
 *
 * @par Side effects:
 * @e connection will be invalidated, and should no longer be used.
 *
 * @sa connection_alloc
 */
extern void connection_free (struct connection *conn);

/**
 * Return the port with which to access notesfile specified by @e conn.
 *
 * @sa connection_set_port
 */
extern unsigned short connection_port (const struct connection *conn);

/**
 * Return the protocol to use to access the notesfile specified by @e conn.
 *
 * @sa connection_set_protocolB
 */
extern enum newts_protocols connection_protocol (const struct connection *conn);

/**
 * Modify @e conn to refer to a notesfile accessed with port @e new_port.
 *
 * @par Side effects:
 * @e conn will be modified.
 *
 * @sa connection_port
 */
extern void connection_set_port (struct connection *conn,
                                 const unsigned short new_port);

/**
 * Modify @e conn to refer to a notesfile accessed with protocol @e
 * new_protocol.
 *
 * @par Side effects:
 * @e conn will be modified.
 *
 * @sa connection_protocol
 */
extern void connection_set_protocol (struct connection *conn,
                                     const enum newts_protocols new_protocol);

/**
 * Modify @e conn to refer to a notesfile on system @e new_system.
 *
 * @par Side effects:
 * @e conn will be modified.
 *
 * @sa connection_system, connection_system_is_localhost
 */
extern void connection_set_system (struct connection *conn,
                                   const char *new_system);

/**
 * Modify @e conn to refer to a notesfile to which we're going to connect as @e
 * new_user.
 *
 * @par Side effects:
 * @e conn will be modified.
 *
 * @sa connection_user
 */
extern void connection_set_user (struct connection *conn,
                                 const char *new_user);

/**
 * Return the system on which the notesfile specified by @e conn is found.
 *
 * @sa connection_set_system, connection_system_is_localhost
 */
extern char *connection_system (const struct connection *conn);

/**
 * Return TRUE if the notesfile specified by @e conn refers to a notesfile on
 * localhost, or FALSE otherwise.
 *
 * This is defined to be true if @ref connection_system "connection_system (conn)"
 * would return NULL, "", or "localhost".
 *
 * FIXME: Obviously, this is a little braindead. We can parse this way
 * better. We can cache the result if we decide to make this an immutable data
 * type ...
 *
 * @sa connection_set_system, connection_system
 */
extern int connection_system_is_localhost (const struct connection *conn);

/**
 * Return the user with which we're going to connect to the server specified
 * by @e conn.
 *
 * @sa connection_set_user
 */
extern char *connection_user (const struct connection *conn);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_CONNECTION_H */
