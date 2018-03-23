#include <sys/resource.h>
#include <sys/times.h>
#include <sys/time.h>
#include "Python.h"
#include "frameobject.h"
#include "structmember.h"

#include "profiler.h"
#include "description.h"


static void Profiler_dealloc(Profiler* self) {
    Py_XDECREF(self->frame_top);
    Py_XDECREF(self->calls);

    Py_TYPE(self)->tp_free((PyObject*)self);
}


/**
 * Current timestamp.
 */
static long long Profiler_current_timestamp(void) {
    struct timeval tv;
    long long ret;

#ifdef GETTIMEOFDAY_NO_TZ
    gettimeofday(&tv);
#else
    gettimeofday(&tv, (struct timezone *)NULL);
#endif

    ret = tv.tv_sec;
    ret = ret * 1000000 + tv.tv_usec;
    return ret;
}


static PyObject *Profiler_new(PyTypeObject *type, PyObject *args, PyObject *kwds) {
    Profiler *self;

    PyObject *calls = PyList_New(0);
    if (!calls) {
        return NULL;
    }

    self = (Profiler *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->state = PROFILER_STATE_READY;
        self->started_at = 0;
        self->ended_at = 0;
        self->frame_top = NULL;
        self->calls = calls;
    }

    return (PyObject *)self;
}


static int Profiler_init(Profiler *self, PyObject *args, PyObject *kwds) {
    static char *kwlist[] = {
        NULL
    };

    if (!PyArg_ParseTupleAndKeywords(args, kwds, "", kwlist)) {
        return -1;
    }

    return 0;
}


/**
 * Fail the profiler.
 */
static void Profiler_fail(Profiler *self) {
    // Reset the profiler.
    PyEval_SetProfile(NULL, NULL);

    // Adjust the state.
    self->state = PROFILER_STATE_FAILED;
}


/**
 * Log call.
 */
static int Profiler_log_call(Profiler *self, ProfilerFrame *frame_top) {
    return PyList_Append(self->calls, (PyObject *)frame_top);
}


/**
 * Push a function onto the call stack.
 *
 * If started_at is -1, this indicates the current time.
 */
static int Profiler_push(Profiler *self, PyObject *fn_desc, long long started_at) {
    ProfilerFrame *frame = ProfilerFrame_from_description_and_started_at(
        fn_desc,
        started_at == -1 ? Profiler_current_timestamp() : started_at,
        self->frame_top
    );

    Py_DECREF(self->frame_top);
    self->frame_top = frame;

    return 0;
}


/**
 * Push a function onto the call stack.
 */
static int Profiler_pop(Profiler *self) {
    // Remove the topmost stack frame.
    ProfilerFrame *frame_top = self->frame_top;
    if (frame_top == NULL) {
        Profiler_fail(self);
        return 0;
    }

    self->frame_top = frame_top->parent;

    // Log the popped action.
    frame_top->ended_at = Profiler_current_timestamp();
    int result = Profiler_log_call(self, frame_top);

    Py_DECREF(frame_top);

    return result;
}


/**
 * Recursively initialize stack frame from frame.
 */
static ProfilerFrame *Profiler_recursive_initial_frame_from_frame(Profiler *self, PyFrameObject *frame) {
    // Get the description of the frame.
    PyObject *description = describe_py_frame(frame);

    if (description == NULL) {
        return NULL;
    }

    // Recurse into the parent if necessary.
    ProfilerFrame *parent_profiler_frame = NULL;

    if (frame->f_back) {
        parent_profiler_frame = Profiler_recursive_initial_frame_from_frame(self, frame->f_back);
    }

    ProfilerFrame *profiler_frame = ProfilerFrame_from_description_and_started_at(
        description,
        self->started_at,
        parent_profiler_frame
    );

    Py_DECREF(description);
    Py_XDECREF(parent_profiler_frame);

    return profiler_frame;
}


/**
 * Profile callback.
 */
static int Profiler_profile(PyObject *self_obj, PyFrameObject *frame, int what, PyObject *arg) {
    Profiler *self = (Profiler *)self_obj;

    // Note: we do not need to deal with exceptions originating from Python code, as PyTrace_RETURN will be generated
    // if the exception causes an exit from the function. This does not apply to calls to C API functions, and thus
    // requires us to handle both the PyTrace_C_RETURN and PyTrace_C_EXCEPTION cases.

    // If no stack is currently present, we need to build an initial stack.
    if (self->state == PROFILER_STATE_STARTING) {
        PyFrameObject *ref_frame = frame;

        if ((what == PyTrace_CALL) || (what == PyTrace_C_CALL)) {
            ref_frame = ref_frame->f_back;
        }

        self->frame_top = Profiler_recursive_initial_frame_from_frame(self, ref_frame);
        self->state = PROFILER_STATE_RUNNING;
    }

    // Handle the current profile callback.
    switch (what) {
    case PyTrace_CALL:
    case PyTrace_C_CALL: {
        PyObject *fn_desc = (
            what == PyTrace_CALL ?
            describe_py_frame(frame) :
            describe_c_function(arg)
        );

        if (!fn_desc) {
            Profiler_fail(self);
            return 0;
        }

        int result = Profiler_push(self, fn_desc, -1);

        Py_DECREF(fn_desc);
        return result;
    }

    case PyTrace_RETURN:
    case PyTrace_C_RETURN:
    case PyTrace_C_EXCEPTION:
        return Profiler_pop(self);

    default:
        return 0;
    }
}


/**
 * Start the profiler.
 */
static PyObject *Profiler_start(Profiler *self) {
    // Ensure that the profiler is ready to start.
    if (self->state != PROFILER_STATE_READY) {
        PyErr_SetString(PyExc_RuntimeError, "Profiler has already been started");
        return NULL;
    }

    // Set the profiler as the one being used.
    PyEval_SetProfile(Profiler_profile, (PyObject *)self);

    // Adjust the state.
    self->started_at = Profiler_current_timestamp();
    self->state = PROFILER_STATE_STARTING;

    Py_INCREF(Py_None);
    return Py_None;
}


/**
 * Stop the profiler.
 */
static PyObject *Profiler_stop(Profiler *self) {
    // Ensure that the profiler is running.
    if ((self->state != PROFILER_STATE_STARTING) && (self->state != PROFILER_STATE_RUNNING)) {
        PyErr_SetString(PyExc_RuntimeError, "Profiler is not running");
        return NULL;
    }

    // Reset the profiler.
    PyEval_SetProfile(NULL, NULL);

    // Adjust the state.
    self->ended_at = Profiler_current_timestamp();
    self->state = PROFILER_STATE_DONE;

    // Log the final call.
    ProfilerFrame *frame = self->frame_top;

    while (frame) {
        frame->ended_at = self->ended_at;

        if (frame == self->frame_top) {
            Profiler_log_call(self, frame);
        }

        frame = frame->parent;
    }

    Py_INCREF(Py_None);
    return Py_None;
}


static PyObject *Profiler_calls_getter(Profiler *self, void *closure) {
    if (self->state != PROFILER_STATE_DONE) {
        Py_INCREF(Py_None);
        return Py_None;
    }

    Py_INCREF(self->calls);
    return self->calls;
}


static PyMemberDef Profiler_members[] = {
    {NULL},
};


static PyGetSetDef Profiler_getsetters[] = {
    {"calls",
     (getter)Profiler_calls_getter,
     NULL,
     "Calls.",
     NULL},

    {NULL},
};


static PyMethodDef Profiler_methods[] = {
    {"start",
     (PyCFunction)Profiler_start,
     METH_NOARGS,
     "Start the profiler"},

    {"stop",
     (PyCFunction)Profiler_stop,
     METH_NOARGS,
     "Stop the profiler"},

    {NULL},
};


PyTypeObject ProfilerType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "callprofiler.Profiler",      /* tp_name */
    sizeof(Profiler),              /* tp_basicsize */
    0,                             /* tp_itemsize */
    (destructor)Profiler_dealloc,  /* tp_dealloc */
    0,                             /* tp_print */
    0,                             /* tp_getattr */
    0,                             /* tp_setattr */
    0,                             /* tp_reserved */
    0,                             /* tp_repr */
    0,                             /* tp_as_number */
    0,                             /* tp_as_sequence */
    0,                             /* tp_as_mapping */
    0,                             /* tp_hash  */
    0,                             /* tp_call */
    0,                             /* tp_str */
    0,                             /* tp_getattro */
    0,                             /* tp_setattro */
    0,                             /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "Profiler",                    /* tp_doc */
    0,                             /* tp_traverse */
    0,                             /* tp_clear */
    0,                             /* tp_richcompare */
    0,                             /* tp_weaklistoffset */
    0,                             /* tp_iter */
    0,                             /* tp_iternext */
    Profiler_methods,              /* tp_methods */
    Profiler_members,              /* tp_members */
    Profiler_getsetters,           /* tp_getset */
    0,                             /* tp_base */
    0,                             /* tp_dict */
    0,                             /* tp_descr_get */
    0,                             /* tp_descr_set */
    0,                             /* tp_dictoffset */
    (initproc)Profiler_init,       /* tp_init */
    0,                             /* tp_alloc */
    Profiler_new,                  /* tp_new */
};
