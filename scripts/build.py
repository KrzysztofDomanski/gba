#!/usr/bin/env python3
import subprocess
import sys
import argparse

def run_command(command):
    print(f"\n=> Running: {' '.join(command)}")
    result = subprocess.run(command)
    if (result.returncode != 0):
        print(f"\n[ERROR] Command failed with exit code {result.returncode}")
        sys.exit(result.returncode)

def main():
    parser = argparse.ArgumentParser(description="Build the GBA emulator")
    parser.add_argument('--clean', action='store_true', help="Clean the build directory before building")
    parser.add_argument('--skip-tests', action='store_true', help="Skip building tests")
    args = parser.parse_args()

    if args.clean:
        # CMake's cross-platform way to delete directories
        run_command(["cmake", "-E", "rm", "-rf", "build"])

    cmake_command = [
        "cmake", "-S", ".", "-B", "build",
    ]
    run_command(cmake_command)

    build_command = [
        "cmake", "--build", "build", "--parallel"
    ]
    run_command(build_command)

    if not args.skip_tests:
        # --test-dir is available in CMake 3.20+ and avoids needing to cd into the directory
        run_command(["ctest", "--test-dir", "build", "--output-on-failure"])

    print("\n[SUCCESS] Build and tests completed successfully.")

if __name__ == "__main__":
    main()