#!/usr/bin/env python3

# Copyright 2026 pugur
# This source code is licensed under the Apache License, Version 2.0
# which can be found in the LICENSE file.

import re
from datetime import datetime
from pathlib import Path

HOLDER = "pugur"
LICENSE_TEXT = f"""// Copyright {datetime.now().year} {HOLDER}
// This source code is licensed under the Apache License, Version 2.0
// which can be found in the LICENSE file.
"""

SOURCE_EXTENSIONS = {".c", ".cc", ".h"}

COPYRIGHT_RE = re.compile(r"//\s*Copyright\s+\d{4}\s+" + re.escape(HOLDER))


def apply_license(file_path: Path):
    try:
        content = file_path.read_text(encoding="utf-8")

        if COPYRIGHT_RE.search(content):
            return
        else:
            print(f"Applying license to: {file_path}")
            new_content = LICENSE_TEXT + "\n" + content

        file_path.write_text(new_content.lstrip(), encoding="utf-8")

    except Exception as e:
        print(f"Error processing {file_path}: {e}")


def main():
    base_dir = Path(__file__).resolve().parent.parent
    target_dirs = [base_dir / "src", base_dir / "tests"]

    for target in target_dirs:
        if not target.is_dir():
            print(f"Directory not found: {target}")
            continue

        for file_path in target.rglob("*"):
            if file_path.suffix in SOURCE_EXTENSIONS:
                apply_license(file_path)


if __name__ == "__main__":
    main()
