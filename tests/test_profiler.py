from unittest import TestCase

from callprofiler import Profiler

from .fixtures import func_a, func_b


class ProfilerTestCase(TestCase):
    """Test case for :class:`~callprofiler.Profiler`.
    """

    def test_start_started(self):
        """Profiler.start() when already started
        """

        profiler = Profiler()
        profiler.start()

        # Test starting after it has already been started.
        with self.assertRaises(RuntimeError):
            profiler.start()

        profiler.stop()

        # Test starting again after stop.
        with self.assertRaises(RuntimeError):
            profiler.start()

    def test_stop_not_started(self):
        """Profiler.stop() when not started
        """

        with self.assertRaises(RuntimeError):
            Profiler().stop()

    def test_stop_stopped(self):
        """Profiler.stop() when stopped
        """

        profiler = Profiler()
        profiler.start()
        profiler.stop()

        with self.assertRaises(RuntimeError):
            profiler.stop()

    def test_calls(self):
        """Profiler.calls after running
        """

        # Profile some stuff.
        profiler = Profiler()
        profiler.start()

        func_a()
        func_b()

        profiler.stop()

        # Check that everything was traced.
        calls = profiler.calls

        self.assertEqual(4, len(calls))

        self.assertGreaterEqual(calls[0].duration, 0)
        self.assertRegex(calls[0].description, r'\/fixtures\.py:\d+\(func_b\)$')
        self.assertRegex(calls[0].parent.description, r'\/fixtures\.py:\d+\(func_a\)$')
        self.assertRegex(calls[0].parent.parent.description, r'\/test_profiler\.py:\d+\(test_calls\)$')

        self.assertGreaterEqual(calls[1].duration, 0)
        self.assertRegex(calls[1].description, r'\/fixtures\.py:\d+\(func_a\)$')
        self.assertRegex(calls[1].parent.description, r'\/test_profiler\.py:\d+\(test_calls\)$')

        self.assertGreaterEqual(calls[2].duration, 0)
        self.assertRegex(calls[2].description, r'\/fixtures\.py:\d+\(func_b\)$')
        self.assertRegex(calls[2].parent.description, r'\/test_profiler\.py:\d+\(test_calls\)$')

        self.assertGreaterEqual(calls[3].duration, 0)
        self.assertEqual(calls[3].description, "<method 'stop' of 'callprofiler.Profiler' objects>")
