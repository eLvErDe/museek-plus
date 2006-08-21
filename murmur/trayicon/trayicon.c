/* -- THIS FILE IS GENERATED - DO NOT EDIT *//* -*- Mode: C; c-basic-offset: 4 -*- */

#include <Python.h>



#line 4 "trayicon.override"
#include <Python.h>

#include "pygobject.h"
#include "eggtrayicon.h"
#line 13 "trayicon.c"


/* ---------- types from other modules ---------- */
static PyTypeObject *_PyGtkPlug_Type;
#define PyGtkPlug_Type (*_PyGtkPlug_Type)
static PyTypeObject *_PyGdkScreen_Type;
#define PyGdkScreen_Type (*_PyGdkScreen_Type)


/* ---------- forward type declarations ---------- */
PyTypeObject PyEggTrayIcon_Type;

#line 26 "trayicon.c"



/* ----------- EggTrayIcon ----------- */

static int
_wrap_egg_tray_icon_new(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "name", NULL };
    char *name;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "s:EggTrayIcon.__init__", kwlist, &name))
        return -1;
    self->obj = (GObject *)egg_tray_icon_new(name);

    if (!self->obj) {
        PyErr_SetString(PyExc_RuntimeError, "could not create EggTrayIcon object");
        return -1;
    }
    pygobject_register_wrapper((PyObject *)self);
    return 0;
}

#line 20 "trayicon.override"
static PyObject*
_wrap_egg_tray_icon_send_message(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = {"timeout", "message", NULL};
    int timeout, len, ret;
    char *message;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "is#:TrayIcon.send_message", kwlist, &timeout, &message, &len))
        return NULL;
    ret = egg_tray_icon_send_message(EGG_TRAY_ICON(self->obj), timeout, message, len);
    return PyInt_FromLong(ret);
}
#line 63 "trayicon.c"


static PyObject *
_wrap_egg_tray_icon_cancel_message(PyGObject *self, PyObject *args, PyObject *kwargs)
{
    static char *kwlist[] = { "id", NULL };
    PyObject *py_id = NULL;
    guint id = 0;

    if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O:EggTrayIcon.cancel_message", kwlist, &py_id))
        return NULL;
    if (py_id) {
        if (PyLong_Check(py_id))
            id = PyLong_AsUnsignedLong(py_id);
        else if (PyInt_Check(py_id))
            id = PyInt_AsLong(py_id);
        else
            PyErr_SetString(PyExc_TypeError, "Parameter 'id' must be an int or a long");
        if (PyErr_Occurred())
            return NULL;
    }
    egg_tray_icon_cancel_message(EGG_TRAY_ICON(self->obj), id);
    Py_INCREF(Py_None);
    return Py_None;
}

static PyMethodDef _PyEggTrayIcon_methods[] = {
    { "send_message", (PyCFunction)_wrap_egg_tray_icon_send_message, METH_VARARGS|METH_KEYWORDS },
    { "cancel_message", (PyCFunction)_wrap_egg_tray_icon_cancel_message, METH_VARARGS|METH_KEYWORDS },
    { NULL, NULL, 0 }
};

PyTypeObject PyEggTrayIcon_Type = {
    PyObject_HEAD_INIT(NULL)
    0,					/* ob_size */
    "trayicon.TrayIcon",			/* tp_name */
    sizeof(PyGObject),	        /* tp_basicsize */
    0,					/* tp_itemsize */
    /* methods */
    (destructor)0,	/* tp_dealloc */
    (printfunc)0,			/* tp_print */
    (getattrfunc)0,	/* tp_getattr */
    (setattrfunc)0,	/* tp_setattr */
    (cmpfunc)0,		/* tp_compare */
    (reprfunc)0,		/* tp_repr */
    (PyNumberMethods*)0,     /* tp_as_number */
    (PySequenceMethods*)0, /* tp_as_sequence */
    (PyMappingMethods*)0,   /* tp_as_mapping */
    (hashfunc)0,		/* tp_hash */
    (ternaryfunc)0,		/* tp_call */
    (reprfunc)0,		/* tp_str */
    (getattrofunc)0,	/* tp_getattro */
    (setattrofunc)0,	/* tp_setattro */
    (PyBufferProcs*)0,	/* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,                      /* tp_flags */
    NULL, 				/* Documentation string */
    (traverseproc)0,	/* tp_traverse */
    (inquiry)0,		/* tp_clear */
    (richcmpfunc)0,	/* tp_richcompare */
    offsetof(PyGObject, weakreflist),             /* tp_weaklistoffset */
    (getiterfunc)0,		/* tp_iter */
    (iternextfunc)0,	/* tp_iternext */
    _PyEggTrayIcon_methods,			/* tp_methods */
    0,					/* tp_members */
    0,		       	/* tp_getset */
    NULL,				/* tp_base */
    NULL,				/* tp_dict */
    (descrgetfunc)0,	/* tp_descr_get */
    (descrsetfunc)0,	/* tp_descr_set */
    offsetof(PyGObject, inst_dict),                 /* tp_dictoffset */
    (initproc)_wrap_egg_tray_icon_new,		/* tp_init */
    (allocfunc)0,           /* tp_alloc */
    (newfunc)0,               /* tp_new */
    (freefunc)0,             /* tp_free */
    (inquiry)0              /* tp_is_gc */
};



/* ----------- functions ----------- */

PyMethodDef trayicon_functions[] = {
    { NULL, NULL, 0 }
};

/* initialise stuff extension classes */
void
trayicon_register_classes(PyObject *d)
{
    PyObject *module;

    if ((module = PyImport_ImportModule("gtk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGtkPlug_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Plug");
        if (_PyGtkPlug_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Plug from gtk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk");
        return;
    }
    if ((module = PyImport_ImportModule("gtk.gdk")) != NULL) {
        PyObject *moddict = PyModule_GetDict(module);

        _PyGdkScreen_Type = (PyTypeObject *)PyDict_GetItemString(moddict, "Screen");
        if (_PyGdkScreen_Type == NULL) {
            PyErr_SetString(PyExc_ImportError,
                "cannot import name Screen from gtk.gdk");
            return;
        }
    } else {
        PyErr_SetString(PyExc_ImportError,
            "could not import gtk.gdk");
        return;
    }


#line 185 "trayicon.c"
    pygobject_register_class(d, "EggTrayIcon", EGG_TYPE_TRAY_ICON, &PyEggTrayIcon_Type, Py_BuildValue("(O)", &PyGtkPlug_Type));
}
