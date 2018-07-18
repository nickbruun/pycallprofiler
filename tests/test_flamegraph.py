import io
import re
from unittest import TestCase

from callprofiler import profile_as_flamegraph_log

from .fixtures import func_a, func_b


CALL_EXPR = re.compile(r'^(?:(?:.*?;)+)(.*?) \d+$')


class FlamegraphTestCase(TestCase):
    """Test case for FlameGraph rendering utilities.
    """

    def test_profile_as_flamegraph_log_not_throwing(self):
        """with profile_as_flamegraph_log(..) not throwing
        """

        # Invoke the profiler.
        writer = io.BytesIO()

        with profile_as_flamegraph_log(writer):
            func_b()
            func_a()

        # Assert the result.
        calls = []

        for line in writer.getvalue().decode('utf-8').strip().split('\n'):
            match = CALL_EXPR.match(line)
            if not match:
                self.fail(f'Unexpected format of log line: {line}')

            calls.append(match.group(1))

        self.assertEqual(6, len(calls))

        self.assertRegex(calls[0], r'\/flamegraph\.py:\d+\(profile_as_flamegraph_log\)$')
        self.assertRegex(calls[1], r'\/contextlib\.py:\d+\(__enter__\)$')
        self.assertRegex(calls[2], r'\/fixtures\.py:\d+\(func_b\)$')
        self.assertRegex(calls[3], r'\/fixtures\.py:\d+\(func_b\)$')
        self.assertRegex(calls[4], r'\/fixtures\.py:\d+\(func_a\)$')
        self.assertEqual(calls[5], "<method 'stop' of 'callprofiler.Profiler' objects>")

    def test_profile_as_flamegraph_log_throwing(self):
        """with profile_as_flamegraph_log(..) throwing
        """

        # Invoke the profiler.
        writer = io.BytesIO()

        with self.assertRaises(RuntimeError):
            with profile_as_flamegraph_log(writer):
                func_b()

                raise RuntimeError('synthetic error')

                func_a()

        # Assert the result.
        calls = []

        for line in writer.getvalue().decode('utf-8').strip().split('\n'):
            match = CALL_EXPR.match(line)
            if not match:
                self.fail(f'Unexpected format of log line: {line}')

            calls.append(match.group(1))

        self.assertEqual(4, len(calls))

        self.assertRegex(calls[0], r'\/flamegraph\.py:\d+\(profile_as_flamegraph_log\)$')
        self.assertRegex(calls[1], r'\/contextlib\.py:\d+\(__enter__\)$')
        self.assertRegex(calls[2], r'\/fixtures\.py:\d+\(func_b\)$')
        self.assertEqual(calls[3], "<method 'stop' of 'callprofiler.Profiler' objects>")
