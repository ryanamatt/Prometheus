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
    """Recursively find all .prm files in TESTS_DIR."""
    prm_files = []
    for root, _, files in os.walk(TESTS_DIR):
        for file in files:
            if file.endswith(".prm"):
                # Get the path relative to TESTS_DIR (e.g., 'math/addition.prm')
                rel_path = os.path.relpath(os.path.join(root, file), TESTS_DIR)
                prm_files.append(rel_path)
    return prm_files

@pytest.mark.parametrize("prm_rel_path", get_prm_files())
def test_prometheus_output(prm_rel_path, update_mode):
    input_path = os.path.join(TESTS_DIR, prm_rel_path)
    
    # Mirror the subfolder structure in the expected directory
    expected_path = os.path.join(EXPECTED_DIR, prm_rel_path + ".expected")

    # Run the interpreter
    result = subprocess.run(
        [PROMETHEUS, input_path],
        capture_output=True,
        text=True,
        timeout=5
    )

    # If --update flag is used, write current output to expected file
    if update_mode:
        # Create subfolders in 'expected' if they don't exist yet
        os.makedirs(os.path.dirname(expected_path), exist_ok=True)
        with open(expected_path, "w", encoding="utf-8") as f:
            f.write(result.stdout)
        pytest.skip(f"Updated expected file for {prm_rel_path}")

    # Standard check
    if not os.path.exists(expected_path):
        pytest.fail(f"Missing expected file: {expected_path}")

    with open(expected_path, "r", encoding="utf-8") as f:
        expected_output = f.read()

    # Pytest will show a beautiful diff if this fails
    assert result.stdout.strip() == expected_output.strip()