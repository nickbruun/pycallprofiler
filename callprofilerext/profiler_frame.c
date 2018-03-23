#include <Python.h>
#include "structmember.h"
#include "frameobject.h"

#include "profiler_frame.h"


static void ProfilerFrame_dealloc(ProfilerFrame* self) {
    if (self->description) {
        Py_DECREF(self->description);
    }

    if (self->parent) {
        Py_DECREF(self->parent);
    }

    Py_TYPE(self)->tp_free((PyObject*)self);
}


static PyObject *ProfilerFrame_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    ProfilerFrame *self;

    self = (ProfilerFrame *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->description = NULL;
        self->started_at = 0;
        self->ended_at = 0;
        self->parent = NULL;
    }

    return (PyObject *)self;
}


static int ProfilerFrame_init(ProfilerFrame *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
        return -1;
    }

    return 0;
}


static PyObject *ProfilerFrame_duration_getter(ProfilerFrame *self, void *closure) {
    if (self->ended_at == 0) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    return PyLong_FromLongLong(self->ended_at - self->started_at);
}


static PyObject *ProfilerFrame_description_getter(ProfilerFrame *self, void *closure) {
    Py_INCREF(self->description);
    return self->description;
}


static PyObject *ProfilerFrame_parent_getter(ProfilerFrame *self, void *closure) {
    if (self->parent) {
        Py_INCREF(self->parent);
        return (PyObject *)self->parent;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyMemberDef ProfilerFrame_members[] = {
    {NULL},
};


static PyGetSetDef ProfilerFrame_getsetters[] = {
    {"duration",
     (getter)ProfilerFrame_duration_getter,
     NULL,
     "Duration.",
     NULL},

    {"description",
     (getter)ProfilerFrame_description_getter,
     NULL,
     "Description.",
     NULL},

    {"parent",
     (getter)ProfilerFrame_parent_getter,
     NULL,
     "Parent frame.",
     NULL},

    {NULL},
};


static PyMethodDef ProfilerFrame_methods[] = {
    {NULL},
};


PyTypeObject ProfilerFrameType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "callprofiler.ProfilerFrame",      /* tp_name */
    sizeof(ProfilerFrame),              /* tp_basicsize */
    0,                                  /* tp_itemsize */
    (destructor)ProfilerFrame_dealloc,  /* tp_dealloc */
    0,                                  /* tp_print */
    0,                                  /* tp_getattr */
    0,                                  /* tp_setattr */
    0,                                  /* tp_reserved */
    0,                                  /* tp_repr */
    0,                                  /* tp_as_number */
    0,                                  /* tp_as_sequence */
    0,                                  /* tp_as_mapping */
    0,                                  /* tp_hash  */
    0,                                  /* tp_call */
    0,                                  /* tp_str */
    0,                                  /* tp_getattro */
    0,                                  /* tp_setattro */
    0,                                  /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,            /* tp_flags */
    "ProfilerFrame",                    /* tp_doc */
    0,                                  /* tp_traverse */
    0,                                  /* tp_clear */
    0,                                  /* tp_richcompare */
    0,                                  /* tp_weaklistoffset */
    0,                                  /* tp_iter */
    0,                                  /* tp_iternext */
    ProfilerFrame_methods,              /* tp_methods */
    ProfilerFrame_members,              /* tp_members */
    ProfilerFrame_getsetters,           /* tp_getset */
    0,                                  /* tp_base */
    0,                                  /* tp_dict */
    0,                                  /* tp_descr_get */
    0,                                  /* tp_descr_set */
    0,                                  /* tp_dictoffset */
    (initproc)ProfilerFrame_init,       /* tp_init */
    0,                                  /* tp_alloc */
    ProfilerFrame_new,                  /* tp_new */
};


ProfilerFrame *ProfilerFrame_from_description_and_started_at(
    PyObject *description,
    long long started_at,
    ProfilerFrame *parent
) {
    ProfilerFrame *frame = (ProfilerFrame *)ProfilerFrame_new(&ProfilerFrameType, NULL, NULL);
    Py_INCREF(frame);

    frame->description = description;
    Py_INCREF(description);
    frame->started_at = started_at;
    frame->parent = parent;
    if (frame->parent) {
        Py_INCREF(frame->parent);
    }

    return frame;
}
