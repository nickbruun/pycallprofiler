#ifndef CALLPROFILER_PROFILERFRAME
#define CALLPROFILER_PROFILERFRAME
#include <Python.h>


/**
 * Profiler frame.
 */
typedef struct ProfilerFrame_struct {
    PyObject_HEAD

    /**
     * Description.
     */
    PyObject *description;

    /**
     * Start time.
     */
    long long started_at;

    /**
     * End time.
     */
    long long ended_at;

    /**
     * Parent frame.
     */
    struct ProfilerFrame_struct *parent;
} ProfilerFrame;


/**
 * Profiler frame from description and start time.
 */
extern ProfilerFrame *ProfilerFrame_from_description_and_started_at(
    PyObject *description,
    long long started_at,
    ProfilerFrame *parent
);


extern PyTypeObject ProfilerFrameType;
#endif
