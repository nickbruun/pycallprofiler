#ifndef CALLPROFILER_PROFILER
#define CALLPROFILER_PROFILER
#include <Python.h>

#include "profiler_frame.h"


/**
 * State.
 */
typedef enum {
    /**
     * Ready.
     */
    PROFILER_STATE_READY = 0,

    /**
     * Starting.
     *
     * The profiler has been started but has not yet initialized its stack.
     */
    PROFILER_STATE_STARTING = 1,

    /**
     * Running.
     */
    PROFILER_STATE_RUNNING = 2,

    /**
     * Done.
     */
    PROFILER_STATE_DONE = 3,

    /**
     * Failed.
     */
    PROFILER_STATE_FAILED = 4,
} ProfilerState;


/**
 * Profiler.
 */
typedef struct {
    PyObject_HEAD

    /**
     * State.
     */
    ProfilerState state;

    /**
     * Start time.
     */
    long long started_at;

    /**
     * End time.
     */
    long long ended_at;

    /**
     * Top-most frame.
     */
    ProfilerFrame *frame_top;

    /**
     * Calls.
     *
     * List of calls.
     */
    PyObject *calls;
} Profiler;


extern PyTypeObject ProfilerType;
#endif
