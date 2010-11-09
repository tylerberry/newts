/*
 * scan-uiuc.h - token constants for the UIUC nfload scanner
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

#ifndef UIUC_LOAD_H
#define UIUC_LOAD_H

#include "newts/newts.h"

/* The contents of the current token. */
extern char *contents;

/* The current parsed note. */
struct newt note;

/* The number of responses we expect to find. */
int responses_expected;

/* The number of responses we've currently found. */
int responses_found;

int yylex (void);

enum uiuc_tokens
  {
    NOP = 1,
    NOTE,
    RESPONSE,
    TITLE,
    DIRECTOR_MESSAGE,
    STATUS,
    EXPIRATION_AGE,
    EXPIRATION_ACTION,
    EXPIRATION_STATUS,
    WORKING_SET_SIZE,
    LONGEST_TEXT,
    POLICY_EXISTS,
    DESCRIPTOR_FINISHED,
    ACCESS_RIGHT,
    ACCESS_FINISHED,
    ERROR
  };

#endif /* not UIUC_LOAD_H */
