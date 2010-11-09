/*
 * newtsmodule.c - Python bindings for Newts
 *
 * This file is part of the Newts notesfile system.
 * Copyright (C) 2005 David A. Mellis.
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

/*
QUESTIONS

Where does the identification authentication of the user happen?
- NF(self, nfname, username) to open a notesfile as a user, and the user
  be passed up into the Note and Responses
- setuser(username) to set a global user variable?

class NF: (or Newtsfile) (or Notesfile)
    __init__(self, nfname)
    getnotecount() (or getnewtcount()?) (or notecount?)
    getnote(notenum) (or getnewt()?)
    post(title, text) or newnote()? (or newnewt()?)

class Note: (or Newt?)
    text: str
    author: str
    created
    title: str
    getresponsecount() (or responsecount?)
    getresponse(respnum)
    reply(text) or post()? or respond()?

class Response: (or Reply?) (or Resp?)
    text
    author
    date
*/

#include <Python.h>
#include <structmember.h>

#include "newts/newts.h"

/* shouldn't these be defined somewhere else? */
#define FALSE 0
#define TRUE 1

/* the uiuc backend (in lib/error.c) expects the client to define these */
int euid = 0;

/* Doesn't dispose of note->nr.nfr, as its members are probably owned by a
 * notesfile nfref.  FIXME: are any of the pointers that freenewt does free
 * shared with note->nf.nfr?
 *
 * FIXME: this actually should be duplicated. -TB
 */
void
freenewt (struct newt *notep)
{
  if (notep->title != NULL)
    free (notep->title);

  if (notep->text != NULL)
    free (notep->text);

  if (notep->director_message != NULL)
    free (notep->director_message);

  if (notep->auth.name != NULL)
    free (notep->auth.name);

  if (notep->auth.system != NULL)
    free (notep->auth.system);

  if (notep->id.system != NULL)
    free (notep->id.system);

  free (notep);
}

/*Response*******************************************************************/

typedef struct {
  PyObject_HEAD
  PyObject *author;
  int created;
  PyObject *text;
  struct newt *note;
} Response;

static void
Response_dealloc (Response *self)
{
  Py_XDECREF (self->author);
  Py_XDECREF (self->text);

  freenewt (self->note);

  self->ob_type->tp_free ((PyObject *) self);
}

static PyMemberDef Response_members[] = {
  { "author", T_OBJECT_EX, offsetof (Response, author), RO, "the response author" },
  { "date", T_INT, offsetof (Response, created), RO, "the response date" },
  { "text", T_OBJECT_EX, offsetof (Response, text), RO, "the response text" },
  { NULL }
};

static PyMethodDef Response_methods[] = {
  { NULL }
};

static PyTypeObject ResponseType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "newts.Response",         /*tp_name*/
    sizeof (Response),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Response_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "A response to a note",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Response_methods,             /* tp_methods */
    Response_members,             /* tp_members */
};

/* Creates a new Response from the struct newt * (which it takes ownership
 * of), and returns a pointer to it.  On failure, sets an exception, disposes
 * of notep, and returns NULL.
 */
PyObject *
Response_fromnewt (struct newt *notep)
{
  Response *res;

  if (!(res = PyObject_New (Response, &ResponseType)))
    {
      freenewt (notep);
      return PyErr_NoMemory (); /* or does PyObject_New already do this? */
    }

  res->note = notep;
  res->author = NULL;
  res->created = notep->created;
  res->text = NULL;

  if (!(res->author = PyString_FromString (notep->auth.name)))
    {
      Py_DECREF (res);
      return NULL;
    }

  if (!(res->text = PyString_FromString (notep->text)))
    {
      Py_DECREF (res);
      return NULL;
    }

  return (PyObject *) res;
}

/*Note***********************************************************************/

typedef struct {
  PyObject_HEAD
  PyObject *author;
  PyObject *title;
  int date;
  PyObject *text;
  struct newt *note;
  int total_resps;
} Note;

static void
Note_dealloc (Note *self)
{
  Py_XDECREF (self->author);
  Py_XDECREF (self->title);
  Py_XDECREF (self->text);

  freenewt (self->note);

  self->ob_type->tp_free ((PyObject *) self);
}

static PyObject *
Note_getresponse (Note *self, PyObject *args, PyObject *kwds)
{
  struct newt *notep;
  int respnum;

  if (!PyArg_ParseTuple (args, "i", &respnum))
    return NULL;

  if (!(notep = malloc (sizeof (struct newt))))
    return PyErr_NoMemory();

  memset (notep, 0, sizeof (struct newt));
  nfref_copy (&notep->nr.nfr, &self->note->nr.nfr);
  notep->nr.notenum = self->note->nr.notenum;
  /*
  if (get_note (notep, FALSE) < 0)
    {
      free (notep);
      PyErr_Format (PyExc_IOError, "Couldn't read note %d from notesfile %s.",
                    notep->nr.notenum, notep->nr.nfr.name);
      return NULL;
    }
  */
  notep->nr.respnum = respnum;

  if (get_note (notep, TRUE) < 0)
    {
      freenewt (notep);
      PyErr_Format (PyExc_IOError, "Couldn't read response %d to note %d "
                    "from notesfile %s.", respnum, notep->nr.notenum,
                    notep->nr.nfr.name);
      return NULL;
    }

  return (PyObject *) Response_fromnewt (notep);
}

static PyMemberDef Note_members[] = {
  { "author", T_OBJECT_EX, offsetof (Note, author), RO, "the note author" },
  { "title", T_OBJECT_EX, offsetof (Note, title), RO, "the note title" },
  { "date", T_INT, offsetof (Note, date), RO, "the note timestamp" },
  { "text", T_OBJECT_EX, offsetof (Note, text), RO, "the note text" },
  { "responsecount", T_INT, offsetof (Note, total_resps), RO,
    "the number of responses to the note" },
  { NULL }
};

static PyMethodDef Note_methods[] = {
  { "getresponse", (PyCFunction)Note_getresponse, METH_VARARGS },
  { NULL }
};

static PyTypeObject NoteType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "newts.Note",         /*tp_name*/
    sizeof (Note),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Note_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "Note(author, title, date, text)\n\nA newt",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Note_methods,             /* tp_methods */
    Note_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    0,      /* tp_init */
};

/* Creates a new Note from the struct newt * (which it takes ownership of),
 * and returns a pointer to it.  On failure, sets an exception, disposes of
 * note, and returns NULL.
 */
PyObject *
Note_fromnewt (struct newt *notep)
{
  Note *res;

  if (!(res = PyObject_New (Note, &NoteType)))
    {
      freenewt (notep);
      return PyErr_NoMemory (); /* or does PyObject_New already do this? */
    }

  res->note = notep;
  res->title = NULL;
  res->author = NULL;
  res->date = notep->created;
  res->text = NULL;
  res->total_resps = notep->total_resps;

  if (!(res->title = PyString_FromString (notep->title)))
    {
      Py_DECREF (res);
      return NULL;
    }

  if (!(res->author = PyString_FromString (notep->auth.name)))
    {
      Py_DECREF (res);
      return NULL;
    }

  if (!(res->text = PyString_FromString (notep->text)))
    {
      Py_DECREF (res);
      return NULL;
    }

  return (PyObject *) res;
}

/*NF*************************************************************************/

typedef struct {
  PyObject_HEAD
  PyObject *nfname;

  /* nfp is NULL if no newtsfile is open */
  struct notesfile *nf;
  int total_notes;
} Newtsfile;

static void
Newtsfile_dealloc (Newtsfile *self)
{
  if (self->nf)
    {
      close_nf (self->nf, FALSE);
      free(self->nf);
    }

  Py_XDECREF (self->nfname);
  self->ob_type->tp_free ((PyObject *) self);
}

static PyObject *
Newtsfile_new (PyTypeObject *type, PyObject *args, PyObject *kwds)
{
  Newtsfile *self;

  self = (Newtsfile *) type->tp_alloc(type, 0);

  if (self != NULL)
    {
      self->nfname = PyString_FromString("");

      if (self->nfname == NULL)
        {
          Py_DECREF (self);
          return NULL;
        }
    }

  return (PyObject *) self;
}

static int
Newtsfile_init (Newtsfile *self, PyObject *args, PyObject *kwds)
{
  newts_nfref *ref;
  struct notesfile *nf;
  PyObject *nfname, *tmp;
  int res;

  if (!PyArg_ParseTuple (args, "S", &nfname))
    return -1;

  ref = nfref_alloc ();

  if (ref == NULL)
    {
      PyErr_NoMemory();
      return -1;
    }

  memset (ref, 0, sizeof (struct notesfile));

  nf = malloc (sizeof (struct notesfile));

  if (nf == NULL)
    {
      PyErr_NoMemory();
      return -1;
    }

  memset (nf, 0, sizeof (struct notesfile));

  nfref_set_user (ref, "");
  nfref_set_system (ref, "");
  nfref_set_owner (ref, NULL);
  nfref_set_name (ref, PyString_AsString (nfname));

  if (!nfref_name (ref))
    {
      nfref_free (ref);
      free (nf);
      return -1;
    }

  if ((res = open_nf (ref, nf)))
    {
      PyErr_SetFromErrnoWithFilename (PyExc_IOError, nfref_name (ref));
      nfref_free (ref);
      free (nf);
      return -1;
    }

  nfref_free (ref);

  /* if we already have a notesfile open (i.e. second call to __init__) */
  if (self->nf)
    {
      close_nf (self->nf, FALSE);
      free (self->nf);
    }

  self->nf = nf;
  self->total_notes = nf->total_notes;

  tmp = self->nfname;
  Py_INCREF (nfname);
  self->nfname = nfname;
  Py_XDECREF (tmp);

  return 0;
}

static PyObject *
Newtsfile_getnote (Newtsfile *self, PyObject *args)
{
  int notenum;
  struct newt *notep;

  if (!PyArg_ParseTuple (args, "i", &notenum))
    return NULL;

  if (!(notep = malloc (sizeof (struct newt))))
    return PyErr_NoMemory();

  memset (notep, 0, sizeof (struct newt));
  nfref_copy (&notep->nr.nfr, self->nf->ref);
  notep->nr.notenum = notenum;

  if (get_note (notep, TRUE) < 0)
    {
      free (notep);
      PyErr_Format (PyExc_IOError, "Couldn't read note %d from %s.",
                    notenum, notep->nr.nfr.name);
      return NULL;
    }

  return Note_fromnewt (notep);
}

static PyObject *
Newtsfile_writenote (Newtsfile *self, PyObject *args)
{
  struct newt note;

  memset (&note, 0, sizeof (struct newt));
  nfref_copy (&note.nr.nfr, self->nf->ref);

  if (!PyArg_ParseTuple (args, "ss", &note.title, &note.text))
    return NULL;

  note.nr.notenum = -1;
  note.auth.name = "dmellis";
  note.auth.system = "python";
  note.auth.uid = 501;

  fprintf (stderr, "calling write_note(%s, %s)\n", note.title, note.text);

  if (write_note (self->nf, &note, 0) < 0)
    {
      PyErr_Format (PyExc_IOError, "Error posting to %s.",
                    nfref_name (self->nf->ref));
      return NULL;
    }

  Py_INCREF (Py_None);
  return Py_None;
}

static PyMemberDef Newtsfile_members[] = {
  { "name", T_OBJECT_EX, offsetof (Newtsfile, nfname), RO,
    "name of the newtsfile" },
  { "notecount", T_INT, offsetof (Newtsfile, total_notes), RO,
    "number of notes in the newtsfile" },
  { NULL }
};

static PyMethodDef Newtsfile_methods[] = {
  { "getnote", (PyCFunction) Newtsfile_getnote, METH_VARARGS,
    "getnote(int) -> Note\n\nGet the n'th note in the newtsfile." },
  { "post", (PyCFunction) Newtsfile_writenote, METH_VARARGS,
    "post(title, text)\n\nPost a new note" },
  { NULL }
};

static PyTypeObject NewtsfileType = {
    PyObject_HEAD_INIT(NULL)
    0,                         /*ob_size*/
    "newts.NF",         /*tp_name*/
    sizeof(Newtsfile),             /*tp_basicsize*/
    0,                         /*tp_itemsize*/
    (destructor)Newtsfile_dealloc, /*tp_dealloc*/
    0,                         /*tp_print*/
    0,                         /*tp_getattr*/
    0,                         /*tp_setattr*/
    0,                         /*tp_compare*/
    0,                         /*tp_repr*/
    0,                         /*tp_as_number*/
    0,                         /*tp_as_sequence*/
    0,                         /*tp_as_mapping*/
    0,                         /*tp_hash */
    0,                         /*tp_call*/
    0,                         /*tp_str*/
    0,                         /*tp_getattro*/
    0,                         /*tp_setattro*/
    0,                         /*tp_as_buffer*/
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /*tp_flags*/
    "NF(name)\n\nA particular newtsfile",           /* tp_doc */
    0,                     /* tp_traverse */
    0,                     /* tp_clear */
    0,                     /* tp_richcompare */
    0,                     /* tp_weaklistoffset */
    0,                     /* tp_iter */
    0,                     /* tp_iternext */
    Newtsfile_methods,             /* tp_methods */
    Newtsfile_members,             /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Newtsfile_init,      /* tp_init */
    0,                         /* tp_alloc */
    Newtsfile_new,                 /* tp_new */
};

static PyMethodDef NewtsMethods[] = {
  { NULL }
};

PyMODINIT_FUNC
initnewts (void)
{
  PyObject *m;

  if (PyType_Ready (&ResponseType) < 0)
    return;
  if (PyType_Ready (&NoteType) < 0)
    return;
  if (PyType_Ready (&NewtsfileType) < 0)
    return;

  m = Py_InitModule3 ("newts", NewtsMethods, "a discussion forum system");
  Py_INCREF (&NewtsfileType);
  PyModule_AddObject (m, "NF", (PyObject *) &NewtsfileType);
  Py_INCREF (&NoteType);
  PyModule_AddObject (m, "Note", (PyObject *) &NoteType);
  Py_INCREF (&ResponseType);
  PyModule_AddObject (m, "Response", (PyObject *) &ResponseType);
}
