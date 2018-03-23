import io
from unittest import TestCase

from callprofiler import profile_as_flamegraph_log

from .fixtures import func_a, func_b


class FlamegraphTestCase(TestCase):
    """Test case for FlameGraph rendering utilities.
    """

    def test_profile_as_flamegraph_log(self):
        """with profile_as_flamegraph_log(..)
        """

        # Invoke the profiler.
        writer = io.BytesIO()

        with profile_as_flamegraph_log(writer):
            func_b()
            func_a()

        # Assert the result.
        lines = writer.getvalue().decode('utf-8').strip().split('\n')

        self.assertEqual(4, len(lines))
