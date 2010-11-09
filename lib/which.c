/*
 * which.c - print full path of executables
 * Copyright (C) 1999, 2000 Carlo Wood <carlo@gnu.org>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#if HAVE_PWD_H
# include <pwd.h>
#endif

#if STDC_HEADERS
# include <stddef.h>
# include <stdlib.h>
#endif

#if defined STDC_HEADERS || defined HAVE_STRING_H
# include <string.h>
#elif HAVE_STRINGS_H
# include <strings.h>
#endif

#if HAVE_SYS_STAT_H
# include <sys/stat.h>
#endif

#if HAVE_SYS_TYPES_H
# include <sys/types.h>
#endif

#if HAVE_UNISTD_H
# include <unistd.h>
#endif

#include "which.h"
#include "xalloc.h"

static int found_path_starts_with_dot;
static char cwd[256];
static size_t cwdlen;

static void get_current_working_directory (void);
static char *find_command_in_path
  (const char *name, const char *path_list, int *path_index);
static char *path_clean_up (const char *path);
static int absolute_program (const char *string);
static char *make_full_pathname
  (const char *path, const char *name, int name_len);
static int file_status (const char *name);
static char *get_next_path_element
  (const char *path_list, int *path_index_pointer);
static char *extract_colon_unit (const char *string, int *p_index);
static int tilde_find_prefix (char *string, int *len);
static int tilde_find_suffix (char *string);
static char *tilde_expand_word (char *filename);
static char *glue_prefix_and_suffix (char *prefix, char *suffix, int suffind);
static char *isolate_tilde_prefix (char *fname, int *lenp);
static char *get_home_dir (void);

#if !__USE_GNU
static int group_member (GETGROUPS_T gid);
#endif

#define FS_EXISTS       0x1
#define FS_EXECABLE     0x2

#ifndef NOGROUP
# define NOGROUP         (GETGROUPS_T) -1
#endif

#undef SHELL
#undef AFS

#define savestring(x) (char *)strcpy(xmalloc(1 + strlen (x)), (x))

char *
which (const char *name)
{
  const char *path_list = getenv ("PATH");
  char *full_path = NULL;
  char *result = NULL;

  if (!name || !*name)
    return NULL;

  get_current_working_directory ();

  if (path_list && *path_list != '\0')
    {
      int path_index = 0;

      result = find_command_in_path (name, path_list, &path_index);
      if (result)
        {
          full_path = path_clean_up (result);
          free (result);
          return xstrdup (full_path);
        }
      else
        {
          return NULL;
        }
    }
  return NULL;
}

/* Suitable only for programs that don't change directories. */

static void
get_current_working_directory (void)
{
  if (cwdlen)
    return;

  if (!getcwd (cwd, sizeof (cwd)))
    {
      const char *pwd = getenv ("PWD");
      if (pwd && strlen (pwd) < sizeof (cwd))
        strcpy (cwd, pwd);
    }

  if (*cwd != '/')
    {
      /* Error */
    }

  return;
}

static char *
find_command_in_path (const char *name, const char *path_list, int *path_index)
{
  static int absolute_path_given;
  static char *abs_path;
  char *found = NULL, *full_path;
  int status, name_len;

  name_len = strlen (name);

  if (!absolute_program (name))
    absolute_path_given = 0;
  else
  {
    char *p;
    absolute_path_given = 1;

    if (abs_path)
      free(abs_path);

    if (*name != '.' && *name != '/' && *name != '~')
    {
      abs_path = (char *) xmalloc ((size_t) 3 + name_len);
      strcpy (abs_path, "./");
      strcat (abs_path, name);
    }
    else
    {
      abs_path = (char *) xmalloc ((size_t) 1 + name_len);
      strcpy (abs_path, name);
    }

    path_list = abs_path;
    p = strrchr(abs_path, '/');
    *p++ = 0;
    name = p;
  }

  while (path_list && path_list[*path_index])
  {
    char *path;

    if (absolute_path_given)
    {
      path = savestring (path_list);
      *path_index = strlen (path);
    }
    else
      path = get_next_path_element (path_list, path_index);

    if (!path)
      break;

    if (*path == '~')
    {
      char *t = tilde_expand (path);
      free(path);
      path = t;
    }

    found_path_starts_with_dot = (*path == '.');

    full_path = make_full_pathname (path, name, name_len);
    free(path);

    status = file_status (full_path);

    if ((status & FS_EXISTS) && (status & FS_EXECABLE))
    {
      found = full_path;
      break;
    }

    free(full_path);
  }

  return (found);
}

static char *
path_clean_up (const char *path)
{
  static char result[256];

  const char *p1 = path;
  char *p2 = result;

  int saw_slash = 0, saw_slash_dot = 0, saw_slash_dot_dot = 0;

  if (*p1 != '/')
  {
    get_current_working_directory ();
    strcpy (result, cwd);
    saw_slash = 1;
    p2 = &result[cwdlen];
  }

  do
  {
    if (!saw_slash || *p1 != '/')
      *p2++ = *p1;
    if (saw_slash_dot && (*p1 == '/'))
      p2 -= 2;
    if (saw_slash_dot_dot && (*p1 == '/'))
    {
      int cnt = 0;
      do
      {
        if (--p2 < result)
          {
            strcpy (result, path);
            return result;
          }
        if (*p2 == '/')
          ++cnt;
      }
      while (cnt != 3);
      ++p2;
    }
    saw_slash_dot_dot = saw_slash_dot && (*p1 == '.');
    saw_slash_dot = saw_slash && (*p1 == '.');
    saw_slash = (*p1 == '/');
  }
  while (*p1++);

  return result;
}

/* absolute_program - return 1 if STRING is an absolute program name; it is
 * absolute if it contains any slashes. This is used to decide whether or not
 * to look up through $PATH.
 */

static int
absolute_program (const char *string)
{
  return ((char *) strchr (string, '/') != (char *)NULL);
}

/* make_full_pathname - turn PATH, a directory, and NAME, a filename, into a
 * full pathname. This allocates new memory and returns it.
 */

static char *
make_full_pathname (const char *path, const char *name, int name_len)
{
  char *full_path;
  int path_len;

  path_len = strlen (path);
  full_path = (char *) xmalloc ((size_t) 2 + path_len + name_len);
  strcpy (full_path, path);
  full_path[path_len] = '/';
  strcpy (full_path + path_len + 1, name);
  return (full_path);
}

/* file_status - return some flags based on information about this file.
 *
 * The EXISTS bit is non-zero if the file is found.
 * The EXECABLE bit is non-zero the file is executble.
 * Zero is returned if the file is not found.
 */

#define u_mode_bits(x) (((x) & 0000700) >> 6)
#define g_mode_bits(x) (((x) & 0000070) >> 3)
#define o_mode_bits(x) (((x) & 0000007) >> 0)
#define X_BIT(x) ((x) & 1)

static int
file_status (const char *name)
{
  struct stat finfo;
  static uid_t user_id = 0;

  /* Determine whether this file exists or not. */
  if (stat (name, &finfo) < 0)
    return (0);

  /* If the file is a directory, then it is not "executable" in the
     sense of the shell. */
  if (S_ISDIR (finfo.st_mode))
    return (FS_EXISTS);

#if defined (AFS)
  /* We have to use access(2) to determine access because AFS does not
     support Unix file system semantics.  This may produce wrong
     answers for non-AFS files when ruid != euid.  I hate AFS. */
  if (access (name, X_OK) == 0)
    return (FS_EXISTS | FS_EXECABLE);
  else
    return (FS_EXISTS);
#else /* !AFS */

  /* Find out if the file is actually executable.  By definition, the
     only other criteria is that the file has an execute bit set that
     we can use. */
  if (user_id == 0)
    user_id = geteuid (); /* CHANGED: bash uses: current_user.euid; */

  /* Root only requires execute permission for any of owner, group or
     others to be able to exec a file. */
  if (user_id == 0)
    {
      int bits;

      bits = (u_mode_bits (finfo.st_mode) |
              g_mode_bits (finfo.st_mode) |
              o_mode_bits (finfo.st_mode));

      if (X_BIT (bits))
        return (FS_EXISTS | FS_EXECABLE);
    }

  /* If we are the owner of the file, the owner execute bit applies. */
  if (user_id == finfo.st_uid && X_BIT (u_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);

  /* If we are in the owning group, the group permissions apply. */
  if (group_member (finfo.st_gid) && X_BIT (g_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);

  /* If `others' have execute permission to the file, then so do we,
     since we are also `others'. */
  if (X_BIT (o_mode_bits (finfo.st_mode)))
    return (FS_EXISTS | FS_EXECABLE);
  else
    return (FS_EXISTS);
#endif /* !AFS */
}

#if !__USE_GNU

static int ngroups = 0;
static GETGROUPS_T *group_array = (GETGROUPS_T *) NULL;
static int default_group_array_size = 0;

/* group_member - return non-zero if GID is one that we have in our groups
 * list.
 */

static int
group_member (GETGROUPS_T gid)
{
  static GETGROUPS_T pgid = (GETGROUPS_T) NOGROUP;
  static GETGROUPS_T egid = (GETGROUPS_T) NOGROUP;

  if (pgid == (GETGROUPS_T) NOGROUP)
#if defined (SHELL)
    pgid = (GETGROUPS_T) current_user.gid;
#else /* !SHELL */
    pgid = (GETGROUPS_T) getgid ();
#endif /* !SHELL */

  if (egid == (GETGROUPS_T) NOGROUP)
#if defined (SHELL)
    egid = (GETGROUPS_T) current_user.egid;
#else /* !SHELL */
    egid = (GETGROUPS_T) getegid ();
#endif /* !SHELL */

  if (gid == pgid || gid == egid)
    return (1);

#if defined (HAVE_GETGROUPS)
  /* getgroups () returns the number of elements that it was able to
     place into the array.  We simply continue to call getgroups ()
     until the number of elements placed into the array is smaller than
     the physical size of the array. */

  while (ngroups == default_group_array_size)
    {
      default_group_array_size += 64;

      group_array = (GETGROUPS_T *)
        xrealloc (group_array, default_group_array_size * sizeof (GETGROUPS_T));

      ngroups = getgroups (default_group_array_size, group_array);
    }

  /* In case of error, the user loses. */
  if (ngroups < 0)
    return (0);

  /* Search through the list looking for GID. */
  {
    register int i;

    for (i = 0; i < ngroups; i++)
      if (gid == group_array[i])
        return (1);
  }
#endif /* HAVE_GETGROUPS */

  return (0);
}

#endif

/* get_next_path_element - return the next element from PATH_LIST, a colon
 * separated list of paths.
 *
 * PATH_INDEX_POINTER is the address of an index into PATH_LIST; the index is
 * modified by this function. Return the next element of PATH_LIST or NULL if
 * there are no more.
 */

static char *
get_next_path_element (const char *path_list, int *path_index_pointer)
{
  char *path;

  path = extract_colon_unit (path_list, path_index_pointer);

  if (!path)
    return (path);

  if (!*path)
    {
      free (path);
      path = savestring (".");
    }

  return (path);
}

/* extract_colon_unit - given a string containing units of information
 * separated by colons, return the next one pointed to by (P_INDEX), or NULL if
 * there are no more. Advance (P_INDEX) to the character after the colon.
 */

static char *
extract_colon_unit (const char *string, int *p_index)
{
  int i, start;

  i = *p_index;

  if (!string || (i >= (int)strlen (string)))
    return ((char *)NULL);

  /* Each call to this routine leaves the index pointing at a colon if
     there is more to the path.  If I is > 0, then increment past the
     `:'.  If I is 0, then the path has a leading colon.  Trailing colons
     are handled OK by the `else' part of the if statement; an empty
     string is returned in that case. */
  if (i && string[i] == ':')
    i++;

  start = i;

  while (string[i] && string[i] != ':') i++;

  *p_index = i;

  if (i == start)
    {
      if (string[i])
        (*p_index)++;

      /* Return "" in the case of a trailing `:'. */
      return (savestring (""));
    }
  else
    {
      char *value;

      value = xmalloc ((size_t) 1 + i - start);
      strncpy (value, string + start, (size_t) i - start);
      value [i - start] = '\0';

      return (value);
    }
}

/* tilde_expand - return a new string which is the result of tilde expanding
 * STRING.
 */

char *
tilde_expand (char *string)
{
  char *result;
  int result_size, result_index;

  result_index = result_size = 0;
  if ((result = strchr (string, '~')))
    result = xmalloc ((size_t) (result_size = (strlen (string) + 16)));
  else
    result = xmalloc ((size_t) (result_size = (strlen (string) + 1)));

  /* Scan through STRING expanding tildes as we come to them. */
  while (1)
    {
      register int start, end;
      char *tilde_word, *expansion;
      int len;

      /* Make START point to the tilde which starts the expansion. */

      start = tilde_find_prefix (string, &len);

      /* Copy the skipped text into the result. */

      if ((result_index + start + 1) > result_size)
        result = xrealloc (result, (size_t) (1 + (result_size += (start + 20))));

      strncpy (result + result_index, string, (size_t) start);
      result_index += start;

      /* Advance STRING to the starting tilde. */

      string += start;

      /* Make END be the index of one after the last character of the
       * username.
       */

      end = tilde_find_suffix (string);

      /* If both START and END are zero, we are all done. */

      if (!start && !end)
        break;

      /* Expand the entire tilde word, and copy it into RESULT. */

      tilde_word = xmalloc ((size_t) 1 + end);
      strncpy (tilde_word, string, (size_t) end);
      tilde_word[end] = '\0';
      string += end;

      expansion = tilde_expand_word (tilde_word);
      free (tilde_word);

      len = strlen (expansion);
      if ((result_index + len + 1) > result_size)
        result = xrealloc (result, (size_t) 1 + (result_size += (len + 20)));

      strcpy (result + result_index, expansion);
      result_index += len;
      free (expansion);
    }

  result[result_index] = '\0';

  return (result);
}

/* The default value of tilde_additional_prefixes.  This is set to
 * whitespace preceding a tilde so that simple programs which do not
 * perform any word separation get desired behaviour.
 */

static const char *tilde_additional_prefixes[] =
  { " ~", "\t~", NULL };

/* The default value of tilde_additional_suffixes.  This is set to
 * whitespace or newline so that simple programs which do not
 * perform any word separation get desired behaviour.
 */

static const char *tilde_additional_suffixes[] =
  { " ", "\n", NULL };

/* tilde_find_prefix - find the start of a tilde expansion in STRING, and
 * return the index of the tilde which starts the expansion. Place the length
 * of the text which identified this tilde starter in LEN, excluding the tilde
 * itself.
 */

static int
tilde_find_prefix (char *string, int *len)
{
  register int i, j, string_len;
  register const char **prefixes = tilde_additional_prefixes;

  string_len = strlen (string);
  *len = 0;

  if (*string == '\0' || *string == '~')
    return (0);

  if (prefixes)
    {
      for (i = 0; i < string_len; i++)
    {
      for (j = 0; prefixes[j]; j++)
        {
          if (strncmp (string + i, prefixes[j], strlen (prefixes[j])) == 0)
        {
          *len = strlen (prefixes[j]) - 1;
          return (i + *len);
        }
        }
    }
    }
  return (string_len);
}

/* tilde_find_suffix - find the end of a tilde expansion in STRING, and return
 * the index of the character which ends the tilde definition.
 */

static int
tilde_find_suffix (char *string)
{
  register int i, j, string_len;
  register const char **suffixes;

  suffixes = tilde_additional_suffixes;
  string_len = strlen (string);

  for (i = 0; i < string_len; i++)
    {
      if (string[i] == '/' /* || !string[i] */)
        break;

      for (j = 0; suffixes && suffixes[j]; j++)
        {
          if (strncmp (string + i, suffixes[j], strlen (suffixes[j])) == 0)
            return (i);
        }
    }
  return (i);
}

/* tilde_expand_word - do the work of tilde expansion on FILENAME. FILENAME
 * starts with a tilde. If there is no expansion, call
 * tilde_expansion_failure_hook. This always returns a newly-allocated string,
 * never static storage.
 */

static char *
tilde_expand_word (char *filename)
{
  char *dirname, *expansion, *username;
  int user_len;
  struct passwd *user_entry;

  if (filename == 0)
    return ((char *)NULL);

  if (*filename != '~')
    return (savestring (filename));

  /* A leading `~/' or a bare `~' is *always* translated to the value of
     $HOME or the home directory of the current user, regardless of any
     preexpansion hook. */
  if (filename[1] == '\0' || filename[1] == '/')
    {
      /* Prefix $HOME to the rest of the string. */
      expansion = (char *) getenv ("HOME");

      /* If there is no HOME variable, look up the directory in
     the password database. */
      if (expansion == 0)
        expansion = get_home_dir ();

      return (glue_prefix_and_suffix (expansion, filename, 1));
    }

  username = isolate_tilde_prefix (filename, &user_len);

  /* No preexpansion hook, or the preexpansion hook failed.  Look in the
     password database. */
  dirname = (char *)NULL;
  user_entry = getpwnam (username);
  if (user_entry == 0)
    {
      free (username);
      /* If we don't have a failure hook, or if the failure hook did not
     expand the tilde, return a copy of what we were passed. */
      if (dirname == 0)
    dirname = savestring (filename);
    }
  else
    {
      free (username);
      dirname = glue_prefix_and_suffix (user_entry->pw_dir, filename, user_len);
    }

#if HAVE_ENDPWENT
  endpwent ();
#endif
  return (dirname);
}

/* glue_prefix_and_suffix - return a string that is PREFIX concatenated with
 * SUFFIX starting at SUFFIND.
 */

static char *
glue_prefix_and_suffix (char *prefix, char *suffix, int suffind)
{
  char *ret;
  int plen, slen;

  plen = (prefix && *prefix) ? strlen (prefix) : 0;
  slen = strlen (suffix + suffind);
  ret = xmalloc ((size_t) plen + slen + 1);
  if (prefix && *prefix)
    strcpy (ret, prefix);
  strcpy (ret + plen, suffix + suffind);
  return ret;
}

/* isolate_tilde_prefix - take FNAME and return the tilde prefix we want
 * expanded. If LENP is non-null, the index of the end of the prefix into FNAME
 * is returned in the location it points to.
 */

static char *
isolate_tilde_prefix (char *fname, int *lenp)
{
  char *ret;
  int i;

  ret = xmalloc (strlen (fname));
  for (i = 1; fname[i] && fname[i] != '/'; i++)
    ret[i - 1] = fname[i];
  ret[i - 1] = '\0';
  if (lenp)
    *lenp = i;
  return ret;
}

static char *
get_home_dir (void)
{
  char *home_dir;
  struct passwd *entry;

  home_dir = (char *)NULL;
  entry = getpwuid (getuid ());
  if (entry)
    home_dir = entry->pw_dir;
  return (home_dir);
}
