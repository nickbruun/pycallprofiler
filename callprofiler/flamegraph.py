from contextlib import contextmanager
from typing import IO, Iterator

from callprofilerext import Profiler, ProfilerFrame


def _concat_frame_stack_descriptions(frame: ProfilerFrame) -> str:
    result = []

    while frame:
        result.append(frame.description)
        frame = frame.parent

    return ';'.join(reversed(result))


def write_profile_as_flamegraph_log(profiler: Profiler, writer: IO[bytes]) -> None:
    """Write profile in the format of the original FlameGraph implementation.

    :param profiler: Profiler instance with a complete profile.
    :param writer: File-like writer to which the flamegraph log should be written.
    :raises RuntimeError: if the profiler has not run.
    """

    calls = profiler.calls
    if calls is None:
        raise RuntimeError('profiler has not yet run')

    # Iterate through all the calls and write the log entries in the single-line format.
    for call in calls:
        writer.write((f'{_concat_frame_stack_descriptions(call)} {call.duration}\n').encode('utf-8'))


@contextmanager
def profile_as_flamegraph_log(writer: IO[bytes]) -> Iterator[None]:
    """Profile to a FlameGraph format log.

    :param writer: File-like writer to which the flamegraph log should be written upon completion of the profile.
    """

    profiler = Profiler()
    profiler.start()

    try:
        yield
    finally:
        profiler.stop()

        write_profile_as_flamegraph_log(profiler, writer)
