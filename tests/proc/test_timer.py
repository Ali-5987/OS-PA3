import pytest

pytestmark = pytest.mark.timer

def assert_passed(result):
    """Helper to check if test passed."""
    if "PASSED:" in result:
        return True
    if "FAILED:" in result:
        raise AssertionError(f"{result}")
    raise AssertionError(f"{result}")


def test_tick_count_incrementing(runner):
    """Test that the system tick count increments over time."""
    result = runner.send_serial("test_tick_count_incrementing")
    assert_passed(result)


def test_sleep_duration(runner):
    """Test that sleep() suspends execution for approximately the correct duration."""
    result = runner.send_serial("test_sleep_duration", timeout=5.0)
    assert_passed(result)


def test_multiple_sleeps(runner):
    """Test multiple consecutive sleep() calls."""
    result = runner.send_serial("test_multiple_sleeps", timeout=5.0)
    assert_passed(result)