#!/usr/bin/env python3

import os

header_license = """// Copyright 2026 pugur
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.

"""

source_extensions = (".c", ".cc", ".h")


def write_license_header_if_needed(file_path: str):
    if not os.path.isfile(file_path):
        print(f"file not found: {file_path}")
        return
    try:
        with open(file_path, "r", encoding="utf-8") as f:
            content = f.read()

        if not content.startswith(header_license):
            print(f"applying license to: {file_path}")
            with open(file_path, "w", encoding="utf-8") as w:
                w.write(header_license + content)
    except Exception as e:
        print(f"error processing {file_path}: {e}")


def apply_directory(dir: str):
    if not os.path.isdir(dir):
        print(f"directory not found: {dir}")
        return
    for root, _, files in os.walk(dir):
        for file in files:
            if file.endswith(source_extensions):
                file_path = os.path.join(root, file)
                write_license_header_if_needed(file_path)


def main():
    script_dir = os.path.dirname(os.path.abspath(__file__))
    src_dir = os.path.join(script_dir, "..", "src")
    apply_directory(src_dir)

    tests_dir = os.path.join(script_dir, "..", "tests")
    apply_directory(tests_dir)


if __name__ == "__main__":
    main()
