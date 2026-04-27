import subprocess
import os
import sys
import difflib
import shutil
import argparse

TEST_DIR = os.path.join('tests', 'tests')
OUTPUT_DIR = os.path.join('tests', 'outputs')
EXPECTED_DIR = os.path.join('tests', 'expected')

def run_prometheous_on_test_files() -> None:
    os.makedirs(OUTPUT_DIR, exist_ok=True)
    # Filter only .prm files first
    test_files = [f for f in os.listdir(TEST_DIR) if f.endswith('.prm')]

    for test_file in test_files:
        full_test_path = os.path.join(TEST_DIR, test_file)
        output_file = os.path.join(OUTPUT_DIR, test_file + ".output")

        try:
            with open(output_file, 'w', encoding='utf-8') as f:
                subprocess.run(
                    ["./prometheus.exe", full_test_path],
                    stdout=f,
                    stderr=subprocess.STDOUT,
                    text=True,
                    timeout=5,
                    check=False # Set to True if you want an exception on non-zero exit
                )
        except subprocess.TimeoutExpired:
            print(f"Timeout: {test_file} took too long.")

def compare_output_files_to_expected_files() -> tuple[bool, str]:
    # It's safer to iterate based on the tests we actually ran
    test_files = [f for f in os.listdir(TEST_DIR) if f.endswith('.prm')]
    all_errors: list[str] = []

    for test_file in test_files:
        out_path: str = os.path.join(OUTPUT_DIR, test_file + ".output")
        exp_path: str = os.path.join(EXPECTED_DIR, test_file + ".expected")

        if not os.path.exists(exp_path):
            all_errors.append(f"Missing expected file for {test_file}")
            continue

        with open(out_path, 'r', encoding='utf-8') as f_out, \
             open(exp_path, 'r', encoding='utf-8') as f_exp:
            
            str_out: list[str] = f_out.read().splitlines()
            str_exp: list[str] = f_exp.read().splitlines()

            if str_out != str_exp:
                # Use unified_diff for a more standard output format
                diff = difflib.unified_diff(str_exp, str_out, fromfile='Expected', tofile='Actual')
                all_errors.append(f"Mismatch in {test_file}:\n" + "\n".join(diff))

    if all_errors:
        return False, "\n\n".join(all_errors)
    
    return True, "All tests passed."

def cleanup_tests() -> None:
    if os.path.exists(OUTPUT_DIR):
        shutil.rmtree(OUTPUT_DIR)

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Prometheus Test Runner")
    parser.add_argument("-c", "--clean", action="store_true", help="Cleans the Output Directory")

    args = parser.parse_args()

    if args.clean:
        cleanup_tests()
        sys.exit(0)

    run_prometheous_on_test_files()
    success, diff = compare_output_files_to_expected_files()

    print(diff)
    
    if not success:
        sys.exit(1)

    sys.exit(0)