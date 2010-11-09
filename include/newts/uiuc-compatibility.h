/*
 * uiuc-compatibility.h - description of the UIUC notes environment
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2003 Tyler Berry.
 *
 * Adapted from structs.h from the UIUC notes distribution by Ray Essick and
 * Rob Kolstad.  Any work derived from this source code is required to
 * retain this notice.
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
 * Suite 330, Boston, MA 02111-1307 USA */

#ifndef UIUC_COMPATIBILITY_H
#define UIUC_COMPATIBILITY_H

/* These macros and structs are copied directly from the UIUC notes source so
 * we can deal with UIUC notesfiles written to disk directly.  Some of the
 * names have been changed to protect the innocent (and to avoid collisions
 * with global constants); these changes are noted below.
 *
 * Some macros and structs have been omitted; only definitions related to disk
 * operations were needed for the module.
 *
 * These are the default values as supplied in the release version of UIUC
 * notesfiles 1.7.  If your copy of notes was compiled with any of the defaults
 * changed in src/structs.h, you might need to change some of the macros
 * defined below for compatibility with your installation of UIUC notes.
 *
 * The UIUC macro "MSTDIR", which was defined in the Makefile, is now a
 * configure option: --with-notes-spool=[/var/spool/notes], and is represented
 * internally by the macro SPOOL.  This macro applies to all backends, not just
 * UIUC.
*/

/* FIXTIMES - you might want to toggle this on if you want to be paranoid about
 * your note times being correct in all cases.  Leaving it off should, in the
 * real world, be just fine.  FWIW, the UIUC notes source did not define
 * FIXTIMES by default.
 */

#undef FIXTIMES

/* Various miscellaneous constants and maximums */

#define BUFSIZE    1024
#define DBVERSION  19850101     /* Yes, it really is that old. */
#define DMLEN      40
#define HARDMAX    3000000
#define HOMESYSSZ  33
#define MAXMSG     100000
#define NAMESZ     17
#define NEVER      (-1)
#define NNLEN      40
#define NPERMS     35           /* Can be increased without breaking things. */
#define PASSWDLEN  128          /* ... I think. */
#define RESPSZ     5
#define SYSSZ      33
#define TITLEN     36
#define UNIQPLEX   100000
#define WDLEN      128

/* Filenames */

#define ACCESS     "access"
#define BUGCOUNT   "bugcount"
#define NOTEINDX   "note.indx"
#define RESPINDX   "resp.indx"
#define SEQ        ".SEQ"
#define SEQUENCER  ".sequencer"
#define TEXT       "text"

/* Permission types */

#define PERMUSER   00
#define PERMGROUP  01
#define PERMSYSTEM 02

/* Permissions */

#define READOK     01
#define WRITOK     02
#define DRCTOK     04
#define RESPOK     010

/* Note options - some names changed slightly */

#define FROMNEWS    01      /* FRMNEWS in UIUC notes. */
#define DIRMES      04
#define ISDELETED   010     /* DELETED in UIUC notes. */
#define CONTINUED   040
#define WRITONLY    0100

/* NF options - some names changed slightly */

#define ANONOK       01
#define ISOPEN       02      /* OPEN in UIUC notes. */
#define ISUNAPPROVED 02
#define NFINVALID    010
#define ISARCHIVE    0400    /* ISARCH in UIUC notes. */
#define ISMODERATED  01000   /* MODERATED in UIUC notes. */

/* Data structures */

struct auth_f
{
  char aname[NAMESZ];
  char asystem[HOMESYSSZ];
  int aid;
};

struct when_f
{
  short w_year;        /* tm_year + 1900 */
  short w_month;       /* tm_mon + 1 */
  short w_day;
  short w_hours;
  short w_mins;
  long w_gmttime;      /* This is a time_t masquerading as a long. */
};

struct id_f
{
  char sys[SYSSZ];
  long uniqid;
};

struct perm_f
{
  short ptype;         /* User, group, or system. */
  char name[NAMESZ];
  short perms;
};

struct daddr_f
{
  long addr;           /* For lseeks - this really ought to have been an off_t,
                        * but we work with what we're given. */
  unsigned long textlen;
};

struct txtbuf_f
{
  char txtbuf[BUFSIZE];
};

struct resp_f
{
  short r_first, r_last;         /* Bounds of this resp_f block. */
  struct id_f r_id[RESPSZ];
  struct daddr_f r_addr[RESPSZ];
  struct when_f r_when[RESPSZ];  /* Date/time of response. */
  char r_from[RESPSZ][SYSSZ];    /* System that sent response to us. */
  struct when_f r_rcvd[RESPSZ];  /* Date/time for sequencer. */
  struct auth_f r_auth[RESPSZ];
  char r_stat[RESPSZ];           /* Director/status flags. */
  int r_next;                    /* Index of next response_ind. */
  int r_previous;                /* Backlinks (currently unused). */
};

struct note_f
{
  struct id_f n_id;
  short n_nresp;
  char ntitle[TITLEN];
  struct auth_f n_auth;
  struct when_f n_date;          /* 'Official' time. */
  struct when_f n_rcvd;          /* Time received. */
  struct when_f n_lmod;          /* Time modified. */
  char n_from[SYSSZ];            /* System that sent note to us. */
  int n_rindx;                   /* Where the first set of responses lies. */
  struct daddr_f n_addr;
  char n_stat;                   /* Director/status flags. */
};

struct descr_f
{
  long d_format;                 /* Database format identifier. */
  char d_title[NNLEN];
  char d_drmes[DMLEN];
  short d_plcy;                  /* Equal to 0 if no policy note. */
  struct when_f d_lastm;         /* Time modified. */
  short d_stat;                  /* Status flags. */
  short d_nnote;
  struct id_f d_id;              /* Contains unique ID counter. */
  struct when_f d_lstxmit;       /* Last network transmit. */
  struct when_f d_created;       /* Creation time. */
  struct when_f d_lastuse;       /* Last day used. */
  long d_daysused;               /* Number of days used. */
  long d_rspwrit;                /* Number of responses ever written. */
  long d_notwrit;                /* Number of notes ever written. */
  long entries;                  /* Number of entries into the notesfile. */
  long walltime;                 /* Total seconds spent in notesfile. */
  long d_rspread;
  long d_notread;
  long d_rsprcvd;
  long d_notrcvd;
  long d_rspxmit;
  long d_notxmit;
  long d_notdrop;
  long d_rspdrop;
  long d_orphans;
  long netwrkouts;
  long netwrkins;
  short d_nfnum;                 /* Unique to this notesfile. */
  long d_archtime;               /* Archive after X days. */
  long d_workset;                /* Minimum number of notes to keep. */
  long d_delnote;                /* Number of deleted notes. */
  long d_delresp;                /* Number of deleted responses. */
  long d_dmesgstat;              /* Use director message for archive? */
  long d_archkeep;               /* Keep or delete when archiving. */
  long d_adopted;
  long d_longnote;               /* Max text length for a note. */
  char d_filler[20];             /* Reserved for future use. */
};

struct io_f
{
  int fidtxt;                    /* 'text' file descriptor. */
  int fidndx;                    /* 'note.indx' file descriptor. */
  int fidrdx;                    /* 'resp.indx' file descriptor. */
  struct descr_f descr;          /* Kept updated by critical sections. */
  char nf[NNLEN];                /* Notesfile name. */
  char basedir[WDLEN];           /* Directory location. */
  char fullname[WDLEN];          /* Full pathname. */
  char xstring[TITLEN+1];        /* Title search string. */
  char xauthor[NAMESZ+SYSSZ+2];  /* Author search string. */
  struct when_f stime;
  short access;                  /* What sort of access user has. */
  int nrspwrit;
  int nnotwrit;
  long entered;                  /* When the notesfile was entered. */
  int nrspread;
  int nnotread;
  int nnotxmit;
  int nrspxmit;
  int nnotrcvd;
  int nrsprcvd;
  int nnotdrop;
  int nrspdrop;
  int norphans;
  int adopted;
};

struct seq_f
{
  char nfname[NNLEN];
  struct when_f lastin;           /* Last entry time. */
};

#endif /* not UIUC_COMPATIBILITY_H */
