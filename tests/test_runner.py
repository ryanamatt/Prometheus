import subprocess
import os
import platform
import pytest

# Paths based on your PSTree
PROMETHEUS = "./prometheus"
if platform.system() == "Windows":
        PROMETHEUS += ".exe"
TESTS_DIR = os.path.join("tests", "tests")
EXPECTED_DIR = os.path.join("tests", "expected")

def get_prm_files():
    return [f for f in os.listdir(TESTS_DIR) if f.endswith(".prm")]

@pytest.mark.parametrize("prm_filename", get_prm_files())
def test_prometheus_output(prm_filename, update_mode):
    input_path = os.path.join(TESTS_DIR, prm_filename)
    # Map test_add_sub.prm to test_add_sub.prm.expected
    expected_path = os.path.join(EXPECTED_DIR, prm_filename + ".expected")

    # Run the interpreter
    result = subprocess.run(
        [PROMETHEUS, input_path],
        capture_output=True,
        text=True,
        timeout=5
    )

    # If --update flag is used, write current output to expected file
    if update_mode:
        with open(expected_path, "w", encoding="utf-8") as f:
            f.write(result.stdout)
        pytest.skip(f"Updated expected file for {prm_filename}")

    # Standard check
    if not os.path.exists(expected_path):
        pytest.fail(f"Missing expected file: {expected_path}")

    with open(expected_path, "r", encoding="utf-8") as f:
        expected_output = f.read()

    # Pytest will show a beautiful diff if this fails
    assert result.stdout.strip() == expected_output.strip()