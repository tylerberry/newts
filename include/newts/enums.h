/*
 * enums.h - definition of all enumerations used by other data structures
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

/** @file newts/enums.h
 * all enumeration types used by other data structures.
 */

#ifndef NEWTS_ENUMS_H
#define NEWTS_ENUMS_H

#include "newts/config.h"

/**
 * The different scopes available for defining access rules.
 */
enum newts_access_scopes
  {
    SCOPE_USER,  /**< Permissions for an individual user. */
    SCOPE_GROUP, /**< Permissions for a Unix group. */
    SCOPE_SYSTEM /**< Permissions for all users from a given host. */
  };

/**
 * Actions and access levels that can be specified for a given access rule.
 */
enum newts_access_permissions
  {
    READ     = 01,  /**< Allowed to read notes. */
    WRITE    = 02,  /**< Allowed to write new basenotes. */
    DIRECTOR = 04,  /**< Allowed to moderate the notesfile and perform other
                     * director actions. */
    REPLY    = 010, /**< Allowed to reply to existing basenotes. */
    OWNER    = 020  /**< Owner of the notesfile. */
  };

/**
 * Symbolic names representing notes protocols.
 */
enum newts_protocols
  {
    NEWTS_PROTOCOL_NCP  /**< The hypothetical Newts Client Protocol. */
  };

/**
 * Symbolic names representing the default ports for the various notes
 * protocols.
 */
enum newts_standard_ports
  {
    NEWTS_NCP_STANDARD_PORT = 12345 /**< The default port for NCP. */
  };

/* enum note_statuses - the various statuses which a note can have.
 *
 * ANONYMOUS      - Flag to indicate that note was anonymous.
 * CONFIDENTIAL   - Hide author from everybody but directors.
 * LOCKED         - Note is locked; no further responses allowed.
 * DELETED        - Note has been deleted.
 * UNAPPROVED     - Note needs moderator approval before displaying.
 * ANNOUNCEMENT   - Note is only reply-able by directors.
 *                  (Would like a better name for this.)
 * DIRECTORS_ONLY - Note is only viewable by directors.
 * WRITEONLY      - Note was written by a user without read permission.
 * STICKY         - Note should appear at the top of the index.
 * CORRUPTED      - Note has been determined to be corrupted.
 */

enum newts_note_statuses
  {
    NOTE_ANONYMOUS      = 01,
    NOTE_CONFIDENTIAL   = 02,
    NOTE_LOCKED         = 04,
    NOTE_DELETED        = 010,
    NOTE_UNAPPROVED     = 020,        /* Same value as NF_MODERATED. */
    NOTE_ANNOUNCEMENT   = 040,
    NOTE_DIRECTORS_ONLY = 0100,
    NOTE_WRITE_ONLY     = 0200,
    NOTE_STICKY         = 0400,
    NOTE_CORRUPTED      = 01000
  };

/* enum write_options - options to take into account when adding new notes.
 *
 * ADD_POLICY      - The note is the new policy note for the notesfile.
 * UPDATE_TIMES    - Set the 'last modified' time for the note to right now.
 * ADD_ID          - Generate a new unique ID for the note.  Otherwise, use the
 *                   ID in the note as provided.
 * SKIP_MODERATION - Set the moderation status of the note equal to the
 *                   moderation status of the note as provided.  Otherwise, set
 *                   the new note as unapproved (if the notesfile is moderated
 *                   in the first place).  This should only be used by the
 *                   nfload program, and internally (e.g. during notesfile
 *                   compression).
 */

enum newts_write_options
  {
    ADD_POLICY      = 01,
    UPDATE_TIMES    = 02,
    ADD_ID          = 04,
    SKIP_MODERATION = 010
  };

#endif /* not NEWTS_ENUMS_H */
