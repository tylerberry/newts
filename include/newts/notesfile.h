/*
 * notesfile.h - definition of the notesfile data type
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2006 Tyler Berry.
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

/** @file newts/notesfile.h
 * Notesfile datatype and related functions.
 */

#ifndef NEWTS_NOTESFILE_H
#define NEWTS_NOTESFILE_H

#include "newts/config.h"
#include "newts/nfref.h"

/**
 * Various flags a notesfile can have set.
 */

enum nf_statuses
  {
    NF_ANONYMOUS    = 01,  /**< Notesfile allows truly anonymous notes. */
    NF_CONFIDENTIAL = 02,  /**< Notesfile allows authors to remain
                            * confidential.  (Directors will still be able to
                            * see the author of a note. */
    NF_LOCKED       = 04,  /**< Notesfile is locked for maintenance. */
    NF_ARCHIVE      = 010, /**< Notesfile is an archive. */
    NF_MODERATED    = 020, /**< New notes require a director's approval. */
    NF_POLICY       = 040  /**< Notesfile has a policy note. */
  };

/**
 * This structure stores metadata about a notesfile.
 */

struct notesfile
{
  newts_nfref *ref;       /**< The reference to the notesfile. */
  char *title;            /**< The title of the notesfile. */
  char *director_message; /**< The default director message for the notesfile.
                           * Given backends may provide per-note director
                           * messages. */
  unsigned total_notes;   /**< The number of threads in the notesfile. */
  time_t modified;        /**< The most recent time this notesfile was
                           * modified, for use by the sequencer. */
  time_t time_entered;    /**< The time that the user "entered" the notesfile.
                           * This is used for keeping track of how much time
                           * has been spent viewing the notesfile by users. */
  int options;            /**< A bitmap of @ref nf_statuses "statuses". */
  short perms;            /**< The permissions the user accessing the notesfile
                           * has. */
  struct opts *opts;      /**< Custom options for the notesfile; these options
                           * are backend-specific. */
};

/* struct opts - standard Newts option file.
 *
 * If your module needs custom options, you should define a struct for them in
 * your module and be prepared to cast it back and forth with this type. Size
 * is currently subject to change.
 *
 * Better way to handle this pending.
 */

struct opts
{
  int zero[10];
};

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Perform whatever actions are necessary to finish using a notesfile.
 *
 * @param nf An open notesfile.
 * @param updatestats A flag controlling whether to update the notesfile's
 *                    statistics; 0 means no, anything else means yes.
 *
 * @return FIXME
 *
 * @par Side effects:
 * May invalidate further connections to this notesfile until it is reopened
 * with open_nf.
 */
extern inline int close_nf (struct notesfile *nf, int updatestats);

/**
 * Commit note deletions and otherwise clean up unused notesfile data.
 *
 * @param nf An open notesfile.
 * @param numnotes A location to place the number of notes after compression.
 * @param numresps A location to place the number of responses after
 *                 compression.
 *
 * @return FIXME
 *
 * @par Side effects:
 * @e numnotes and @e numresps will be updated to hold the number of notes and
 * responses remaining in the notesfile after compression.  On error, the
 * value of these variables is undefined.
 *
 * @par
 * @ref update_nf "update_nf" will automatically be called on @e nf.  The
 * notesfile will potentially be seriously different in structure; if you
 * memoize anything about the notesfile, you should forget it after
 * compression.
 */
extern inline int compress_nf (struct notesfile *nf, unsigned *numnotes,
                               unsigned *numresps);

/**
 *
 */
extern inline int get_next_bug (const struct notesfile *nf);

/**
 *
 */
extern inline int modify_nf (struct notesfile *nf);

/**
 * Create a new struct notesfile.
 *
 * @return A newly allocated struct notesfile. This should be freed with
 * nf_free when no longer needed.
 *
 * @sa nf_free
 */
extern struct notesfile *nf_alloc (void);

/**
 * Deallocate @e nf.
 *
 * @par Side effects:
 * @e nf will be invalidated, and should no longer be used.
 *
 * @sa nf_alloc
 */
extern void nf_free (struct notesfile *nf);

/**
 *
 */
extern newts_nfref *nf_nfref (const struct notesfile *nf);

/**
 *
 */
extern inline int open_nf (const newts_nfref *ref, struct notesfile *nf);

/**
 * Refresh the metadata for an already opened notesfile.
 *
 * @param nf An open notesfile.
 *
 * @return FIXME
 *
 * @par Side effects:
 * The values stored in @e nf will be modified.
 */

extern inline int update_nf (struct notesfile *nf);

#ifdef __cplusplus
}
#endif

#endif /* not NEWTS_NOTESFILE_H */
