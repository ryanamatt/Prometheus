import subprocess
import os
import sys
import argparse
from pathlib import Path

def run_command(command: list[str], shell: bool = False):
    """Utility to run shell commands and catch errors."""
    try:
        subprocess.run(command, check=True, shell=shell)
    except subprocess.CalledProcessError as e:
        print(f"Error executing {e}: {' '.join(command)}")
        sys.exit(1)

def main():
    parser = argparse.ArgumentParser(description="Prometheus AST Visualizer Wrapper")
    parser.add_argument("input", help="Path to the .prm source file")
    parser.add_argument("-m", "--make", action="store_true", help="run the command make visualize before making visual.")
    parser.add_argument("-f", "--format", choices=["png", "svg", "pdf"], default="png", help="Output format (default: png)")
    parser.add_argument("--keep-dot", action="store_true", help="Keep the intermediate .dot file")
    
    args = parser.parse_args()
    input_path = Path(args.input)
    
    if not input_path.exists():
        print(f"Error: File {args.input} not found.")
        sys.exit(1)

    # Ensure the project is built with visualization support
    if args.make:
        run_command(["make", "visualize"])

    # Define output paths
    dot_file = input_path.with_suffix(".dot")
    output_img = input_path.with_suffix(f".{args.format}")

    # Run the prometheus-viz executable
    subprocess.run(["./prometheus-viz", str(input_path)], check=True)

    # Use Graphviz 'dot' to create the image
    run_command(["dot", f"-T{args.format}", str(dot_file), "-o", str(output_img)])

    # Clean up dot file if requested
    if not args.keep_dot:
        os.remove(dot_file)

    print(f"Success! Created: {output_img}")

    # Attempt to open the file automatically
    if sys.platform == "win32":
        os.startfile(output_img)
    elif sys.platform == "darwin":
        subprocess.run(["open", str(output_img)], check=False)
    else:
        subprocess.run(["xdg-open", str(output_img)])

if __name__ == "__main__":
    main()