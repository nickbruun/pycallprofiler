#include <Python.h>

#include "profiler.h"


static PyModuleDef callprofilerextmodule = {
    PyModuleDef_HEAD_INIT,
    "callprofilerext",
    "C extension implementing low-overhead call-tracing profiling.",
    -1,
    NULL, NULL, NULL, NULL, NULL
};


PyMODINIT_FUNC PyInit_callprofilerext(void) {
    PyObject* m;

    if (PyType_Ready(&ProfilerFrameType) < 0)
        return NULL;

    if (PyType_Ready(&ProfilerType) < 0)
        return NULL;

    m = PyModule_Create(&callprofilerextmodule);
    if (m == NULL)
        return NULL;

    Py_INCREF(&ProfilerType);
    PyModule_AddObject(m, "Profiler", (PyObject *)&ProfilerType);

    Py_INCREF(&ProfilerFrameType);
    PyModule_AddObject(m, "ProfilerFrame", (PyObject *)&ProfilerFrameType);

    return m;
}
