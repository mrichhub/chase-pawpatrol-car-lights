#!/usr/bin/env python3
"""Compile and run the production animation/engine sources without hardware."""

import argparse
import os
from pathlib import Path
import shlex
import subprocess
import tempfile


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--sanitize", action="store_true",
        help="enable AddressSanitizer and UndefinedBehaviorSanitizer (Clang/GCC)",
    )
    args = parser.parse_args()
    root = Path(__file__).resolve().parent.parent
    compiler = shlex.split(os.environ.get("CXX", "c++"))
    flags = ["-std=c++17", "-Wall", "-Wextra", "-Werror", "-pedantic", "-g"]
    if args.sanitize:
        flags += ["-fsanitize=address,undefined", "-fno-omit-frame-pointer"]
    sources = [
        root / "test/test_firmware.cpp",
        root / "src/animations/AlternatingColors.cpp",
        root / "src/animations/PoliceStrobe.cpp",
        root / "src/engine/AnimationScheduler.cpp",
    ]
    with tempfile.TemporaryDirectory(prefix="chase-led-tests-") as temporary:
        executable = Path(temporary) / "firmware_tests"
        command = compiler + flags
        # This shim directory is only included by this host test runner.
        for directory in ("test/support", "include", "src"):
            command += ["-I", str(root / directory)]
        command += [str(source) for source in sources] + ["-o", str(executable)]
        subprocess.run(command, check=True)
        subprocess.run([str(executable)], check=True)


if __name__ == "__main__":
    main()
