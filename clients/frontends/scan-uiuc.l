/*                                                                    -*- C -*-
 * scan-uiuc.l - scanner for a UIUC-style nfdump image
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

%{
#if HAVE_CONFIG_H
# include <config.h>
#endif

#include "internal.h"

#include "newts/uiuc-compatibility.h"
#include "scan-uiuc.h"

#include <assert.h>

#if STDC_HEADERS
# include <limits.h>
# include <stddef.h>
#endif

char *contents;
struct newt note;
int responses_expected = 0;
int current_response = 0;
int responses_found = 0;

extern int debug;

#define RETURN(x)                            \
  {                                          \
    char *t = newts_strdup (yytext);         \
    char *c = strchr (t, ':');               \
    if (contents) newts_free (contents);     \
    if (c)                                   \
      {                                      \
        if (strlen (c) > 2)                  \
          {                                  \
            contents = newts_strdup (c + 2); \
            contents[strlen (contents) - 1] = '\0'; \
          }                                  \
        *c = '\0';                           \
        if (debug)                           \
          printf (_("Successfully parsed token: '%s'\n"), t); \
      }                                      \
    newts_free (t);                          \
    return (x);                              \
  }

%}

%option noyywrap

%%

"NF-Title: ".*\n                  RETURN (TITLE);
"NF-Director-Message: ".*\n       RETURN (DIRECTOR_MESSAGE);
"NF-Status: ".*\n                 RETURN (STATUS);
"NF-Expiration-Age: ".*\n         RETURN (EXPIRATION_AGE);
"NF-Expiration-Action: ".*\n      RETURN (EXPIRATION_ACTION);
"NF-Expiration-Status: ".*\n      RETURN (EXPIRATION_STATUS);
"NF-Working-Set-Size: ".*\n       RETURN (WORKING_SET_SIZE);
"NF-Longest-Text: ".*\n           RETURN (LONGEST_TEXT);
"NF-Policy-Exists: ".*\n          RETURN (POLICY_EXISTS);
"NF-Descriptor: ".*\n             RETURN (DESCRIPTOR_FINISHED);

"NF-Access-Right: ".*\n           |
"Access-Right: ".*\n              RETURN (ACCESS_RIGHT);
"NF-Access-Finished:".*\n         return ACCESS_FINISHED;

N:.*\n  {
        /* We know buffer is big enough because we know the sizes of each piece
         * of data; check noted/backends/uiuc/uiuc-compatibility.h for details.
         */
        register int c, i = 0, index = 0;
        ptrdiff_t offset;
        int options = 0;
        size_t length = 0;
        char buffer[128];
        memset (buffer, 0, 128);

        if (responses_found != responses_expected)
          {
            fprintf (stderr, _("At least one response was missing.\n"));
            return ERROR;
          }

        responses_found = 0;

        /* Free existing allocs. */

        if (note.text) newts_free (note.text);
        if (note.title) newts_free (note.title);
        if (note.director_message) newts_free (note.director_message);
        if (note.auth.name) newts_free (note.auth.name);
        if (note.auth.system) newts_free (note.auth.system);

        memset (&note, 0, sizeof (struct newt));

        /* Here is the format we're parsing:
         *
         * N:id-system:id-number:number-of-resps
         * title
         * author-username:author-uid:author-system:
         * time-created, in the form y:m:d:h:m:unix-time:
         * time-received (same)
         * time-modified (same)
         * who-from-field
         * options:length-of-text
         * text
         */

        /* Parse and save the expected number of responses. */
        offset = strrchr (yytext, ':') - yytext + 1;
        while ((c = *(yytext + offset)) != '\n' && c != EOF && index < 127)
          {
            buffer[index++] = c;
            offset++;
          }
        buffer[index] = '\0';
        responses_expected = strtol (buffer, NULL, 10);
        index = 0;

        /* Parse and save the title. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        note.title = newts_strdup (buffer);
        index = 0;

        /* Parse and save the author's username. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        note.auth.name = newts_strdup (buffer);
        index = 0;

        /* Parse and save the author's user number. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        note.auth.uid = strtol (buffer, NULL, 10);
        if (note.auth.uid < 0 || note.auth.uid > USHRT_MAX)
          {
            fprintf (stderr, "Invalid author UID value.\n");
            return ERROR;
          }
        index = 0;

        /* Parse and save the author's system. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index - 1] = '\0';
        note.auth.system = newts_strdup (buffer);
        index = 0;

        /* Discard the first five fields of the creation time. */
        for (i=0; i<5; i++)
          {
            while ((c = input ()) != ':' && c != EOF)
              ;
          }

        /* Parse and save the creation time. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index - 1] = '\0';
        note.created = (time_t) strtol (buffer, NULL, 10);
        index = 0;

        /* Discard the first five fields of the 'received' time. */
        for (i=0; i<5; i++)
          {
            while ((c = input ()) != ':' && c != EOF)
              ;
          }

        /* Discard the 'received' time. */
        while ((c = input ()) != '\n' && c != EOF)
          ;

        /* Discard the first five fields of the modification time. */
        for (i=0; i<5; i++)
          {
            while ((c = input ()) != ':' && c != EOF)
              ;
          }

        /* Parse and save the modification time. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index - 1] = '\0';
        note.modified = (time_t) strtol (buffer, NULL, 10);
        index = 0;

        /* Parse and discard the 'who-from' field. */
        while ((c = input ()) != '\n' && c != EOF)
          ;

        /* Parse, translate, and save the note options. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        options = strtol (buffer, NULL, 8); /* Octal! */
        if (options & DIRMES)
          note.director_message = newts_strdup ("t");
        if (options & ISDELETED)
          note.options |= NOTE_DELETED;
        if (options & WRITONLY)
          note.options |= NOTE_WRITE_ONLY;
        if (strcasecmp (note.auth.name, "anonymous") == 0)
          note.options |= NOTE_ANONYMOUS;
        index = 0;

        /* Parse and save the length field. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        length = (size_t) strtol (buffer, NULL, 10);
        index = 0;

        /* Parse the text directly into the text field. */
        note.text = newts_nmalloc (length + 2, sizeof (char));
        while (index < length && (c = input ()) != EOF)
          note.text[index++] = c;
        note.text[index++] = '\n';
        note.text[index] = '\0';

        if (c == EOF)
          {
            fprintf (stderr, "Unexpected end of file.\n");
            return ERROR;
          }

        return NOTE;
      }

R:.*\n  {
        /* See above on size. */
        register int c, i, index = 0;
        ptrdiff_t offset;
        int options = 0;
        size_t length = 0;
        char buffer[128];
        memset (buffer, 0, 128);

        responses_found++;

        if (note.text) newts_free (note.text);
        if (note.title) newts_free (note.title);
        if (note.director_message) newts_free (note.director_message);
        if (note.auth.name) newts_free (note.auth.name);
        if (note.auth.system) newts_free (note.auth.system);

        memset (&note, 0, sizeof (struct newt));

        /* Here is the format we're parsing:
         *
         * R:note-id-system:note-id-number:resp-id-system:resp-id-number:resp-number
         * <NO TITLE FIELD HERE>
         * author-username:author-uid:author-system
         * time-created, in the form y:m:d:h:m:unix-time
         * time-modified (same)
         * who-from-field
         * options:length-of-text
         * text
         */

        /* Parse and save the number of the current response. */
        offset = strrchr (yytext, ':') - yytext + 1;
        while ((c = *(yytext + offset)) != '\n' && c != EOF && index < 127)
          {
            buffer[index++] = c;
            offset++;
          }
        buffer[index] = '\0';
        current_response = strtol (buffer, NULL, 10);
        index = 0;

        if (responses_found != current_response)
          {
            fprintf (stderr, "%d %d %d\n", current_response, responses_found,
                     responses_expected);
            fprintf (stderr, _("Out of sequence response.\n"));
            return ERROR;
          }

        /* Parse and save the author's username. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        note.auth.name = newts_strdup (buffer);
        index = 0;

        /* Parse and save the author's user number. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        note.auth.uid = strtol (buffer, NULL, 10);
        if (note.auth.uid < 0 || note.auth.uid > USHRT_MAX)
          {
            fprintf (stderr, _("Invalid author UID value.\n"));
            return ERROR;
          }
        index = 0;

        /* Parse and save the author's system. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index - 1] = '\0';
        note.auth.system = newts_strdup (buffer);
        index = 0;

        /* Discard the first five fields of the creation time. */
        for (i=0; i<5; i++)
          {
            while ((c = input ()) != ':' && c != EOF)
              ;
          }

        /* Parse and save the creation time. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index - 1] = '\0';
        note.modified = note.created = (time_t) strtol (buffer, NULL, 10);
        index = 0;

        /* Discard the first five fields of the 'received' time. */
        for (i=0; i<5; i++)
          {
            while ((c = input ()) != ':' && c != EOF)
              ;
          }

        /* Discard the 'received' time. */
        while ((c = input ()) != '\n' && c != EOF)
          ;

        /* Parse and discard the 'who-from' field. */
        while ((c = input ()) != '\n' && c != EOF)
          ;

        /* Parse, translate, and save the note options. */
        while ((c = input ()) != ':' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        options = strtol (buffer, NULL, 8); /* Octal! */
        if (options & DIRMES)
          note.director_message = newts_strdup ("t");
        if (options & ISDELETED)
          note.options |= NOTE_DELETED;
        if (options & WRITONLY)
          note.options |= NOTE_WRITE_ONLY;
        if (strcasecmp (note.auth.name, "anonymous") == 0)
          note.options |= NOTE_ANONYMOUS;
        index = 0;

        /* Parse and save the length field. */
        while ((c = input ()) != '\n' && c != EOF && index < 127)
          buffer[index++] = c;
        buffer[index] = '\0';
        length = (size_t) strtol (buffer, NULL, 10);
        index = 0;

        /* Parse the text directly into the text field. */
        note.text = newts_nmalloc (length + 2, sizeof (char));
        while (index < length && (c = input ()) != EOF)
          note.text[index++] = c;
        note.text[index++] = '\n';
        note.text[index] = '\0';

        if (c == EOF)
          {
            fprintf (stderr, "Unexpected end of file.\n");
            return ERROR;
          }

        return RESPONSE;
      }

"NF-Last-Modified: ".*\n          |
"NF-Id-Sequence: ".*\n            |
"NF-Number: ".*\n                 |
"NF-Last-Transmit: ".*\n          |
"NF-Created: ".*\n                |
"NF-Last-Used: ".*\n              |
"NF-Days-Used: ".*\n              |
"NF-Notes-Written: ".*\n          |
"NF-Notes-Read: ".*\n             |
"NF-Notes-Transmitted: ".*\n      |
"NF-Notes-Received: ".*\n         |
"NF-Notes-Dropped: ".*\n          |
"NF-Responses-Written: ".*\n      |
"NF-Responses-Read: ".*\n         |
"NF-Responses-Transmitted: ".*\n  |
"NF-Responses-Received: ".*\n     |
"NF-Responses-Dropped: ".*\n      |
"NF-Entries: ".*\n                |
"NF-Walltime: ".*\n               |
"NF-Orphans-Received: ".*\n       |
"NF-Orphans-Adopted: ".*\n        |
"NF-Transmits: ".*\n              |
"NF-Receives: ".*\n               return NOP;

.*\n                              return ERROR;
