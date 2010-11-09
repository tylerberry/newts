/*
 * notes.h - prototypes and global variables for the UIUC client
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2006 Tyler Berry.
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

#ifndef NOTES_H
#define NOTES_H

#include "internal.h"
#include "newts/newts.h"

/* enums used throughout the client. */

enum editmodes
  {
    NORMAL,
    COPYEDIT,
    COPYNOEDIT,
    VECTOR,
    EDIT
  };

enum sequencers
  {
    NONE,
    SEQUENCER,
    EXTENDED,
    INDEX
  };

enum quitcodes
  {
    NEXTSEQ = -2,
    NEXTNOSEQ = -3,
    QUITSEQ = -4,
    QUITNOSEQ = -5
  };

/* Global variables used throughout the client. */

extern int alt_time;
extern uid_t anon_uid;
extern char *asearch;
extern int black_skip_seq;
extern List blacklist;
extern int debug;
extern char *editor;
extern uid_t euid;
extern char *fqdn;
extern char *homedir;
extern char *program_name;
extern char *mailer;
extern char *messages;
extern int no_blacklist;
extern uid_t notes_uid;
extern time_t orig_seqtime;
extern char *pager;
extern gid_t real_gid;
extern uid_t real_uid;
extern char *seqname;
extern int seq_own_notes;
extern time_t seqtime;
extern int sequencer;
extern char *shell;
extern int signature;
extern char *talk;
extern char *tmpdir;
extern int traditional;
extern char *tsearch;
extern char *txtsearch;
extern char *username;
extern int white_basenotes;
extern List whitelist;

/* These macros are used to reduce the number of times certain things occurred
 * in the source code; I noticed myself repeating them a lot, so I did the
 * Lispy thing and made macros out of them.
 */

/* YES_OR_NO - ask a yes-or-no question, get an answer, and return it.
 *
 * Implemented as a macro rather than an inline because PRINT_QUESTION needs to
 * be evalled regularly.
 */

#define YES_OR_NO(variable, PRINT_QUESTION)                          \
{                                                                    \
  int ycoordinate, xcoordinate, confirm;                             \
  char foo[2];                                                       \
  foo[1] = '\0';                                                     \
  PRINT_QUESTION                                                     \
  confirm = getch ();                                                \
  getyx (stdscr, ycoordinate, xcoordinate);                          \
  foo[0] = (char) confirm;                                           \
  while (rpmatch (foo) == -1)                                        \
    {                                                                \
      if (traditional)                                               \
        {                                                            \
          addch (confirm);                                           \
          printw (_(" y or n please"));                              \
          move (ycoordinate, xcoordinate);                           \
          refresh ();                                                \
          beep ();                                                   \
        }                                                            \
      confirm = getch ();                                            \
      foo[0] = (char) confirm;                                       \
    }                                                                \
  if (rpmatch (foo) == 1)                                            \
    variable = TRUE;                                                 \
  else                                                               \
    variable = FALSE;                                                \
}

extern short allowed (struct notesfile *nf, short perm);
extern inline short blacklisted (struct newt *notep);
extern int compose_mail (char *sendto, struct newt *quote);
extern int compose_note (struct notesfile *nf, struct newt *quote,
                         char *textstr, char *titlestr, int notenum,
                         short mode);
extern void curses_malloc_die (void);
extern void display_index (struct notesfile *nf, int *first, int *last);
extern void fprint_time (FILE *file, struct tm *tm);
extern int get_author_utmp (const char *user);
extern int get_number (int first_char, int cap);
extern char *get_text (struct newt *quote, short mode);
extern void init_blacklist (void);
extern int limited_index (struct notesfile *nf);
extern inline int list_parse (char *buf, int *p, int *first, int *last);
extern inline int list_convert (char *buf, int *p);
extern int master (newts_nfref *ref);
extern int parse_file (char *filename, List *list);
extern int parse_nf (char *string, List *list);
extern void printw_time (struct tm *tm);
extern int read_note (struct notesfile *nf, int *first, int notenum,
                      int respnum, short suppress_blacklist);
extern int run_access (newts_nfref *ref);
extern int run_director (struct notesfile *nf);
extern int run_index (struct notesfile *nf, int *first, int *last, int *resp,
           short *suppress_blacklist);
extern void setup (void);

#if __STDC__
extern int spawn_process (const char *stdin_file, const char *path, ...);
#else
extern int spawn_process ();
#endif

extern inline int spawn_subshell (void);
extern void teardown (void);
extern int test_nf (newts_nfref *ref);
extern char *time_string (struct tm *tm);

#endif /* not NOTES_H */
