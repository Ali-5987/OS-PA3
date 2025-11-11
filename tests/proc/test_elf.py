import pytest
pytestmark = pytest.mark.elf

def assert_passed(result):
    """Helper to check if test passed."""
    if "PASSED:" in result:
        return True
    if "FAILED:" in result:
        raise AssertionError(f"{result}")
    raise AssertionError(f"{result}")


def test_elf_check_valid_header(runner):
    """Test ELF header validation with a valid ELF file."""
    result = runner.send_serial("test_elf_check_valid_header")
    assert_passed(result)


def test_elf_load_program(runner):
    """Test loading a complete ELF program."""
    result = runner.send_serial("test_elf_load_program")
    assert_passed(result)


def test_elf_load_nonexistent(runner):
    """Test that loading a nonexistent file fails gracefully."""
    result = runner.send_serial("test_elf_load_nonexistent")
    assert_passed(result)