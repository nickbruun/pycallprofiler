#ifndef CALLPROFILER_DESCRIPTION
#define CALLPROFILER_DESCRIPTION
#include <Python.h>
#include "frameobject.h"


/**
 * Describe C function.
 */
extern PyObject *describe_c_function(PyObject *fn_obj);


/**
 * Describe Python frame.
 */
extern PyObject *describe_py_frame(PyFrameObject *frame);
#endif
