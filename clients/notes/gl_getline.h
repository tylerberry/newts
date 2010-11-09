/*
 * gl_getline.h - declarations for getline input editing processor
 * Copyright (C) 1991, 1992, 1993 Chris Thewalt.
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose and without fee is hereby granted, provided that the above copyright
 * notices appear in all copies and that both the copyright notice and this
 * permission notice appear in supporting documentation.  This software is
 * provided "as is" without express or implied warranty.
 *
 * Thanks to the following people who have provided enhancements and fixes:
 *   Ron Ueberschaer, Christoph Keller, Scott Schwartz, Steven List, DaviD W.
 *   Sanderson, Goran Bostrom, Michael Gleason, Glenn Kasten, Edin Hodzic, Eric
 *   J Bivona, Kai Uwe Rommel, Danny Quah, Ulrich Betzler
 *
 * This file is included as part of the Newts notesfile system.
 * Copyright (C) 2003 Tyler Berry.
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

#ifndef GETLINE_H
#define GETLINE_H

/* unix systems can #define POSIX to use termios, otherwise
 * the bsd or sysv interface will be used
 */

#define GL_BUF_SIZE 1024

typedef size_t (*gl_strwidth_proc)(char *);
typedef int (*gl_in_hook_proc)(char *);
typedef int (*gl_out_hook_proc)(char *);
typedef int (*gl_tab_hook_proc)(char *, int, int *, size_t);
typedef size_t (*gl_strlen_proc)(const char *);
typedef char * (*gl_tab_completion_proc)(const char *, int);

char *gl_getline (char *);             /* read a line of input */
void gl_setwidth (int);                /* specify width of screen */
void gl_histadd (char *);              /* adds entries to hist */
void gl_strwidth (gl_strwidth_proc);   /* to bind gl_strlen */
void gl_tab_completion (gl_tab_completion_proc);
char *gl_local_filename_completion_proc (const char *, int);
void gl_set_home_dir (const char *homedir);
void gl_histsavefile (const char *const path);
void gl_histloadfile (const char *const path);
char *gl_win_getpass (const char *const prompt, char *const pass, int dsize);

#ifndef _getline_c_

extern gl_in_hook_proc gl_in_hook;
extern gl_out_hook_proc gl_out_hook;
extern gl_tab_hook_proc gl_tab_hook;
extern gl_strlen_proc gl_strlen;
extern gl_tab_completion_proc gl_completion_proc;
extern int gl_filename_quoting_desired;
extern const char *gl_filename_quote_characters;
extern int gl_ellipses_during_completion;
extern int gl_completion_exact_match_extra_char;
extern char gl_buf[GL_BUF_SIZE];

#endif /* ! _getline_c_ */

#endif /* GETLINE_H */
