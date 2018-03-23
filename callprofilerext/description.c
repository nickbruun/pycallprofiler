#include "description.h"


PyObject *describe_c_function(PyObject *fn_obj) {
    // Note: part of the following implementation is derived from the Python 3.6.5rc1 code base's implementation of the
    // internals of the standard Python profiler.

    // Validate that the provided method is in fact a C function.
    if (!PyCFunction_Check(fn_obj)) {
        PyErr_SetString(PyExc_TypeError, "provided C function is not a C function");

        return NULL;
    }

    // Handle unbound functions.
    const PyCFunctionObject *fn = (PyCFunctionObject *)fn_obj;

    if (fn->m_self == NULL) {
        PyObject *mod = fn->m_module;
        PyObject *mod_name = NULL;

        if (mod != NULL) {
            if (PyUnicode_Check(mod)) {
                mod_name = mod;
                Py_INCREF(mod_name);
            }
            else if (PyModule_Check(mod)) {
                mod_name = PyModule_GetNameObject(mod);
                if (mod_name == NULL)
                    PyErr_Clear();
            }
        }

        if (mod_name != NULL) {
            if (!_PyUnicode_EqualToASCIIString(mod_name, "builtins")) {
                PyObject *result;
                result = PyUnicode_FromFormat("<%U.%s>", mod_name, fn->m_ml->ml_name);
                Py_DECREF(mod_name);
                return result;
            }
            Py_DECREF(mod_name);
        }

        return PyUnicode_FromFormat("<%s>", fn->m_ml->ml_name);
    }
    else {
        // Attempt to use repr API.
        PyObject *self = fn->m_self;
        PyObject *name = PyUnicode_FromString(fn->m_ml->ml_name);
        PyObject *mod_name = fn->m_module;

        if (name != NULL) {
            PyObject *mod = _PyType_Lookup(Py_TYPE(self), name);
            Py_XINCREF(mod);
            Py_DECREF(name);

            if (mod != NULL) {
                PyObject *res = PyObject_Repr(mod);
                Py_DECREF(mod);

                if (res != NULL) {
                    return res;
                }
            }
        }

        // Otherwise, use __module__.
        PyErr_Clear();

        if (mod_name != NULL && PyUnicode_Check(mod_name)) {
            return PyUnicode_FromFormat("<built-in method %S.%s>", mod_name, fn->m_ml->ml_name);
        }
        else {
            return PyUnicode_FromFormat("<built-in method %s>", fn->m_ml->ml_name);
        }
    }
}


PyObject *describe_py_frame(PyFrameObject *frame) {
    PyCodeObject *code = frame->f_code;

    if (code->co_name && code->co_filename) {
        return PyUnicode_FromFormat("%S:%d(%S)", code->co_filename, frame->f_lineno, code->co_name);
    }

    if (code->co_name) {
        Py_INCREF(code->co_name);
        return code->co_name;
    }

    if (code->co_filename) {
        return PyUnicode_FromFormat("%S:%d", code->co_filename, frame->f_lineno);
    }

    return NULL;
}
