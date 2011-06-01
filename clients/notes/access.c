/*
 * access.c - manage access permission lists
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003, 2004, 2005, 2006 Tyler Berry
 *
 * Based in part on access.c from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad.  Any work derived from this source code is required to retain
 * this notice.
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

#include "notes.h"

#include "gl_getline.h"
#include "newts/uiuc-compatibility.h"

#if HAVE_GRP_H
# include <grp.h>
#endif

#if HAVE_NCURSES_H
# include <ncurses.h>
#elif HAVE_CURSES_H
# include <curses.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

/* This array maps permission classes to strings. */

const char *classmap[] =
  {
    "usr:", "grp:", "sys:"
  };

/* And this array maps bit-field permissions to strings. */

const char *permmap[] =
  {
    /* ----- */ "Null",
    /* ----r */ "Read Only",
    /* ---w- */ "(02)",        /* 012 is used instead. */
    /* ---wr */ "(03)",        /* 013 is used instead. */
    /* --d-- */ "(04)",        /* Nonsense. */
    /* --d-r */ "(05)",        /* Nonsense. */
    /* --dw- */ "(06)",        /* Nonsense. */
    /* --dwr */ "(07)",        /* Nonsense. */
    /* -a--- */ "Answer Only", /* How would you see it to reply? */
    /* -a--r */ "Read/Answer",
    /* -a-w- */ "Write Only",
    /* -a-wr */ "Read/Write",
    /* -ad-- */ "(014)",       /* Nonsense. */
    /* -ad-r */ "(015)",       /* Nonsense. */
    /* -adw- */ "(016)",       /* Nonsense. */
    /* -adwr */ "Director/R/W"
  };

static void access_help (void);
static void getmode_help (void);
static void display_access (List *access_list, int first, int total);
static void get_mode (struct access *access_entry, List *access_list,
                      int first, int total);

/* run_access - main loop for the permission edit screen.
 *
 * Returns: 0, QUITSEQ, or QUITNOSEQ.
 */

int
run_access (newts_nfref *ref)
{
  List access_list;
  short changed = FALSE;
  short redraw = TRUE;
  int c;
  int first = 0;
  int entries;

  entries = get_access_list (ref, &access_list);

  while (1)
    {
      if (redraw)
        display_access (&access_list, first, entries);
      redraw = FALSE;

      mvprintw (LINES - 2, 0, _("Option: "));
      refresh ();

      c = getch ();

      switch (c)
        {
        case '?':
          access_help ();
          /* Fall through. */

        case 'r':  /* Redraw the screen. */
        case '\f':
          redraw = TRUE;
          break;

        case '!':
          redraw = TRUE;
          spawn_subshell ();
          break;

        case 's':
          move (LINES - 2, 0);
          clrtoeol ();
          printw (_("Sorting..."));
          refresh ();
          list_merge_sort (&access_list);
          redraw = TRUE;
          break;

        case 'Q':  /* Quit without saving. */
        case 'K':
          list_destroy (&access_list);
          return 0;

        case 'q':
        case 'k':
          if (changed)
            {
              list_merge_sort (&access_list);
              write_access_list (ref, &access_list);
            }
          list_destroy (&access_list);
          return 0;

        case '-':
        case KEY_UP:
        case KEY_PPAGE:
          first -= c == KEY_UP ? 1 : (LINES - 6) / 2 - 1;
          if (first < 0)
            first = 0;
          redraw = TRUE;
          break;

        case '+':
        case KEY_DOWN:
        case KEY_NPAGE:
          first += c == KEY_DOWN ? 1 : (LINES - 6) / 2 - 1;
          if (first >= entries - (LINES - 6) / 2)
            {
              first = entries - (LINES - 6) - 3;
              if (first < 0)
                first = 0;
            }
          redraw = TRUE;
          break;

        case 'i':  /* Insert new entries. */
          {
            char *persistent_error = NULL;
            short stop = FALSE;

            while (entries < NPERMS && !stop)
              {
                ListNode *node;
                struct access *access_entry;
                char *temp, *name, *prompt;
                int key, i;
                int y, x;
                enum newts_access_scopes scope;
                int scope_is_set = FALSE;
                int mode;
                short restart = FALSE;
                short advice_displayed = FALSE;

                if (traditional)
                  mvprintw (LINES - 5, 39, _("Entry type: "));
                else
                  {
                    clear ();
                    display_access (&access_list, first, entries);
                    if (persistent_error != NULL)
                      {
                        move (LINES - 3, 0);
                        clrtoeol ();
                        printw ("%s", persistent_error);
                        persistent_error = NULL;
                      }
                    move (LINES - 2, 0);
                    clrtoeol ();
                    printw (_("Entry type: "));
                  }
                refresh ();
                if (!traditional)
                  advice_displayed = FALSE;
                getyx (stdscr, y, x);
                while (scope_is_set == FALSE)
                  {
                    key = getch ();
                    if (key == '\n' || key == '\r' || key == KEY_ENTER ||
                        key == 'q' || key == 'k')
                      {
                        if (traditional && (key == 'k' || key == 'q'))
                          echochar (key);
                        stop = TRUE;
                        break;
                      }

                    switch (key)
                      {
                      case 'u':
                        if (traditional)
                          {
                            echochar (key);
                            move (y, x);
                          }
                        scope = SCOPE_USER;
                        scope_is_set = TRUE;
                        break;

                      case 'g':
                        if (traditional)
                          {
                            echochar (key);
                            move (y, x);
                          }
                        scope = SCOPE_GROUP;
                        scope_is_set = TRUE;
                        break;

                      case 's':
                        if (traditional)
                          {
                            echochar (key);
                            move (y, x);
                          }
                        scope = SCOPE_SYSTEM;
                        scope_is_set = TRUE;
                        break;

                      case KEY_RESIZE:
                        break;

                      case EOF:
                        clear ();
                        display_access (&access_list, first, entries);
                        if (traditional)
                          {
                            if (advice_displayed)
                              mvprintw (LINES - 5, 54, "(u,g,s, q,k,<cr>)");
                            move (LINES - 5, 51);
                          }
                        else
                          {
                            if (advice_displayed)
                              mvprintw (LINES - 3, 0,
                                        _("Please enter 'u', 'g', or 's'; or "
                                          "'q', 'k', or <RET> to exit."));
                            mvprintw (LINES - 2, 0, _("Entry type: "));
                          }
                        refresh ();
                        break;

                      default:
                        advice_displayed = TRUE;
                        if (traditional)
                          {
                            mvprintw (LINES - 5, 54, "(u,g,s, q,k,<cr>)");
                            move (LINES - 5, 51);
                          }
                        else
                          {
                            move (LINES - 3, 0);
                            clrtoeol ();
                            printw (_("Please enter 'u', 'g', or 's'; or 'q', "
                                      "'k', or <RET> to exit."));
                            move (LINES - 2, 0);
                            clrtoeol ();
                            printw (_("Entry type: "));
                          }
                        refresh ();
                        break;
                      }
                  }

                if (stop) continue;

                if (traditional)
                  {
                    prompt = newts_nmalloc (strlen (_("Name: ")) + 40,
                                            sizeof (char));
                    strcpy (prompt, "                                       ");
                    strncat (prompt, _("Name: "),
                             strlen (_("Name: ")) + 1);
                    move (LINES - 4, 0);
                    clrtoeol ();
                  }
                else
                  {
                    move (LINES - 3, 0);
                    clrtoeol ();
                    prompt = newts_strdup (scope == SCOPE_SYSTEM ? _("System name: ") :
                                           scope == SCOPE_GROUP ? _("Group name: ") :
                                           _("User name: "));
                    move (LINES - 2, 0);
                    clrtoeol ();
                  }
                refresh ();
                temp = gl_getline (prompt);
                temp[strlen (temp) - 1] = '\0';
                newts_free (prompt);

                if (strlen (temp) == 0)
                  continue;
                name = newts_strdup (temp);
                gl_histadd (name);

                if (scope == SCOPE_USER)
                  {
                    if (strcasecmp (name, "other") != 0)
                      {
                        struct passwd *pw = getpwnam (name);

                        if (pw == NULL)
                          {
                            if (traditional)
                              {
                                move (LINES - 3, 0);
                                clrtoeol ();
                                mvprintw (LINES - 3, 39, _("--No such user--"));
                              }
                            else
                              persistent_error = _("No such user.");
                            continue;
                          }

                        endpwent ();
                      }
                  }

                if (scope == SCOPE_GROUP)
                  {
                    if (strcasecmp (name, "other") != 0)
                      {
                        struct group *gp = getgrnam (name);

                        if (gp == NULL)
                          {
                            if (traditional)
                              {
                                move (LINES - 3, 0);
                                clrtoeol ();
                                mvprintw (LINES - 3, 39, _("--No such group--"));
                              }
                            else
                              persistent_error = _("No such group.");
                            continue;
                          }

                        endgrent ();
                      }
                  }

                node = list_head (&access_list);
                for (i=0; i<entries; i++)
                  {
                    access_entry = (struct access *) list_data (node);
                    if (access_scope (access_entry) == scope &&
                        strcmp (access_name (access_entry), name) == 0)
                      {
                        if (traditional)
                          {
                            move (LINES - 3, 0);
                            clrtoeol ();
                            mvprintw (LINES - 3, 39, _("%s entry exists"), name);
                          }
                        else
                          persistent_error =
                            scope == SCOPE_USER ? _("User already exists in "
                                             "permission table.") :
                            (scope == SCOPE_GROUP ? _("Group already exists in "
                                               "permission table.") :
                             _("System already exists in permission table."));
                        restart = TRUE;
                        continue;
                      }
                    node = list_next (node);
                    if (node == NULL)
                      continue;
                  }

                if (restart)
                  continue;

                {
                  struct access *new_access = access_alloc ();

                  access_set_permissions (new_access, READ | WRITE | REPLY);
                  access_set_scope (new_access, scope);
                  access_set_name (new_access, name);

                  get_mode (new_access, &access_list, first, entries);

                  list_insert_sorted (&access_list, (void *) new_access);
                }

                newts_free (name);

                entries++;
                redraw = TRUE;
                changed = TRUE;
                clear ();
                display_access (&access_list, first, entries);
              }

            if (!traditional)
              redraw = TRUE;
            break;
          }

        case 'd':  /* Delete existing entries. */
          {
            ListNode *node, *prev;
            struct access *data;
            int key, number, i;

            move (LINES - 2, 0);
            clrtoeol ();
            if (traditional)
              printw ("%s", _("Delete entry #: "));
            else
              printw ("%s", _("Delete entry number: "));

            key = getch ();
            while (key != '\n' && key != '\r' && key != KEY_ENTER &&
                   (key < '1' || key > '9'))
              key = getch ();

            if (key == '\n' || key == '\r' || key == KEY_ENTER)
              {
                redraw = TRUE;
                break;
              }

            number = get_number (key, entries);
            if (number < 0)
              {
                redraw = TRUE;
                break;
              }
            if (number > entries || key < '0' || key > '9' || number == 0)
              {
                clear ();
                display_access (&access_list, first, entries);
                if (traditional)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw ("%s", _("Bad entry"));
                  }
                else
                  {
                    move (LINES - 3, 0);
                    clrtoeol ();
                    printw ("%s", _("Invalid entry."));
                  }
                break;
              }

            number--;  /* Adjust to base zero. */

            prev = NULL;
            node = list_head (&access_list);
            for (i=0; i<number; i++)
              {
                prev = node;
                node = list_next (prev);
              }

            data = (struct access *) list_data (node);
            if (data->scope == SCOPE_USER && strcmp (data->name, username) == 0)
              {
                clear ();
                display_access (&access_list, first, entries);
                if (traditional)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw ("%s", _(" Can't Delete self"));
                  }
                else
                  {
                    move (LINES - 3, 0);
                    clrtoeol ();
                    printw ("%s", _("Can't delete own entry."));
                  }
                break;
              }

            list_remove_next (&access_list, prev, NULL);

            entries--;
            changed = TRUE;
            redraw = TRUE;
            break;
          }

        case 'm':  /* Modify existing entries. */
          {
            ListNode *node;
            struct access *existing_entry;
            int key, number, i;

            move (LINES - 2, 0);
            clrtoeol ();
            if (traditional)
              printw ("%s", _("Modify entry #: "));
            else
              printw ("%s", _("Modify entry number: "));

            key = getch ();
            while (key != '\n' && key != '\r' && key != KEY_ENTER &&
                   (key < '1' || key > '9'))
              key = getch ();

            if (key == '\n' || key == '\r' || key == KEY_ENTER)
              {
                redraw = TRUE;
                break;
              }

            number = get_number (key, entries);
            if (number < 0)
              {
                redraw = TRUE;
                break;
              }
            if (number > entries || key < '0' || key > '9' || number == 0)
              {
                clear ();
                display_access (&access_list, first, entries);
                if (traditional)
                  {
                    move (LINES - 1, 0);
                    clrtoeol ();
                    printw ("%s", _("Bad entry"));
                  }
                else
                  {
                    move (LINES - 3, 0);
                    clrtoeol ();
                    printw ("%s", _("Invalid entry."));
                  }
                break;
              }

            number--;  /* Adjust to base zero. */

            node = list_head (&access_list);
            for (i=0; i<number; i++)
              {
                node = list_next (node);
              }

            existing_entry = (struct access *) list_data (node);
            get_mode (existing_entry, &access_list, first, entries);
            changed = TRUE;
            redraw = TRUE;
            break;
          }

        case '\004':
          list_destroy (&access_list);
          return QUITNOSEQ;

        case 'z':
          list_destroy (&access_list);
          return QUITSEQ;

        default:
          beep ();
          break;
        }
    }

  return 0;
}

/* display_access - Display ACCESS_LIST.  FIRST and TOTAL are the number to
 * start on and the bounds in the list, respectively.
 */

static void
display_access (List *access_list, int first, int total)
{
  ListNode *node;
  struct access *entry;
  register int row = 0, col = 0, i;

  clear ();

  if (first != 0)
    mvprintw (row++, col, _(" -- More -- "));

  node = list_head (access_list);

  for (i=0; i<first; i++)
    node = list_next (node);

  for (; i < total && node != NULL && row < LINES - 6; i++)
    {
      entry = (struct access *) list_data (node);

      /* FIXME: there is a access.h method to be made here. */

      mvprintw (row++, col, "%2d %s%-*s %s", i + 1, classmap[entry->scope],
                NAMESZ, entry->name, permmap[access_permissions (entry)]);

      node = list_next (node);
    }

  if (i < total)
    mvprintw (row++, col, _(" -- More -- "));

  refresh ();

  return;
}

/* get_mode - prompt and get a new MODE. */

static void
get_mode (struct access *access_entry, List *access_list, int first,
          int entries)
{
  int c;
  unsigned new_mode = access_permissions (access_entry);

  if (!traditional)
    {
      clear ();
      display_access (access_list, first, entries);
    }

  while (1)
    {
      if (traditional)
        {
          move (LINES - 3, 39);
          clrtoeol ();
          addch (' ');
          printw (_("Mode: %s"), permmap[new_mode]);
          move (LINES - 2, 39);
          printw (_("Mods: "));
        }
      else
        {
          move (LINES - 2, 0);
          clrtoeol ();
          printw (_("Mode: %s"), permmap[new_mode]);
        }
      refresh ();

      c = getch ();

      if (traditional) echochar (c);

      switch (c)
        {
        case KEY_RESIZE:
          break;

        case '!':
          spawn_subshell ();
          /* Fallthrough */

        case '?':
          if (c == '?')
            getmode_help ();
          /* Fallthrough */

        case EOF:
          clear ();
          display_access (access_list, first, entries);
          if (traditional)
            {
              move (LINES - 3, 39);
              clrtoeol ();
              addch (' ');
              printw (_("Mode: %s"), permmap[new_mode]);
              move (LINES - 2, 39);
              printw (_("Mods: "));
            }
          else
            {
              move (LINES - 2, 0);
              clrtoeol ();
              printw (_("Mode: %s"), permmap[new_mode]);
            }
          refresh ();
          break;

        case 'a':
          if (new_mode & WRITE)
            break;
          if (new_mode & REPLY)
            new_mode &= ~REPLY;
          else
            new_mode |= REPLY;
          break;

        case 'r':
          if (new_mode & DIRECTOR)
            break;
          if (new_mode & READ)
            new_mode &= ~READ;
          else
            new_mode |= READ;
          break;

        case 'w':
          if (new_mode & DIRECTOR)
            break;
          if (new_mode & WRITE)
            new_mode &= ~WRITE;
          else
            new_mode |= WRITE | REPLY;
          break;

        case 'd':
          if (new_mode & OWNER)
            break;
          if (new_mode & DIRECTOR)
            new_mode &= ~DIRECTOR;
          else
            new_mode |= DIRECTOR | READ | WRITE | REPLY;
          break;

        case 'n':
          new_mode = 0;
          break;

        case '\n': case '\r':
        case KEY_ENTER:
        case 'q': case 'k':
          access_set_permissions (access_entry, new_mode);
          return;

        case 'Q': case 'K':
          return;
        }
    }
}

/* access_help - display help screen for access. */

static void
access_help (void)
{
  int c, column;

  do
    {
      clear ();

      mvprintw (0, (COLS - 13) / 2 - 1, "Access Help");

      column = (COLS - 80) / 2 - 3;

      mvprintw (2, column + 21, "<arrows>   scroll around access list");
      mvprintw (3, column + 21, "-, <PGUP>  scroll up access list");
      mvprintw (4, column + 21, "+, <PGDN>  scroll down access list");
      mvprintw (5, column + 21, "d          delete an access entry");
      mvprintw (6, column + 21, "C-d        quit notes");
      mvprintw (7, column + 21, "i          insert a new access entry");
      mvprintw (8, column + 21, "m          modify an existing access entry");
      mvprintw (9, column + 21, "q, k       save changes and exit");
      mvprintw (10, column + 21, "Q, K       discard changes and exit");
      mvprintw (11, column + 21, "r, C-f     redraw the screen");
      mvprintw (12, column + 21, "s          sort the access list");
      mvprintw (13, column + 21, "z          quit notes, update sequencer");
      mvprintw (14, column + 21, "!          fork a subshell");

      mvprintw (16, column + 20, "If you have suggestions for the help wording");
      mvprintw (17, column + 20, "or the layout, please pass them on to Tyler.");

      move (LINES - 2, 0);

      refresh ();

      c = getch ();
      while (c == KEY_RESIZE)
        c = getch ();
    }
  while (c == EOF);

  return;
}

/* getmode_help - display help screen for getmode. */

static void
getmode_help (void)
{
  int c, column;

  do
    {
      clear ();

      mvprintw (0, (COLS - 13) / 2 - 1, "Mode Editing Help");

      column = (COLS - 80) / 2 - 3;

      mvprintw (2, column + 21, "<RET>  save changes");
      mvprintw (3, column + 21, "a      add/remove reply (answer) privilege");
      mvprintw (4, column + 21, "d      add/remove director privilege");
      mvprintw (5, column + 21, "n      reset to no privileges");
      mvprintw (6, column + 21, "q, k   save changes");
      mvprintw (7, column + 21, "Q, K   discard changes");
      mvprintw (8, column + 21, "r      add/remove read privilege");
      mvprintw (9, column + 21, "w      add/remove write privilege");
      mvprintw (10, column + 21, "!      fork a subshell");

      mvprintw (12, column + 20, "If you have suggestions for the help wording");
      mvprintw (13, column + 20, "or the layout, please pass them on to Tyler.");

      move (LINES - 2, 0);

      refresh ();

      c = getch ();
      while (c == KEY_RESIZE)
        c = getch ();
    }
  while (c == EOF);

  return;
}
