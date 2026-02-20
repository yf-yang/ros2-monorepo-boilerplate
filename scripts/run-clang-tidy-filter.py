#!/usr/bin/env python3
# Copyright (c) The ros Contributors
# Licensed under the Apache License, Version 2.0.

"""Wrapper around run-clang-tidy that ignores compiler errors from non-project headers.

clang-tidy uses the clang frontend, which may hit hard compilation errors in
system/vendor headers when GCC and Clang versions diverge (e.g. Clang 21 +
GCC 14 libstdc++ ``<optional>``).  These are not actionable by project code.

This wrapper:
1. Forwards all arguments to ``run-clang-tidy``.
2. Suppresses full diagnostic blocks (error + notes + context) that originate
   from non-project paths.
3. Re-evaluates the exit code: only project-source diagnostics cause non-zero.
"""

from __future__ import annotations

import re
import subprocess
import sys

# Matches a clang diagnostic:  /path/to/file:line:col: error|warning: ... [check-name]
_DIAG_RE = re.compile(r"^(/.*?):\d+:\d+: (error|warning): .* \[.*\]$")

# Matches note/context lines:  /path/to/file:line:col: note: ...
_NOTE_RE = re.compile(r"^/.*?:\d+:\d+: note: ")

# Lines that are part of a diagnostic block (source snippets, carets, etc.)
# These are indented or contain source-pointer lines like "      |" or "      ^"
_CONTEXT_RE = re.compile(r"^[ \t]+\d*[ \t]*[|^~]")

# Paths that belong to the project source tree.
_PROJECT_PREFIX = "/workspaces/ros/ros/src/"


def main() -> int:
    proc = subprocess.Popen(
        ["run-clang-tidy", *sys.argv[1:]],
        stdout=subprocess.PIPE,
        stderr=subprocess.STDOUT,
        text=True,
    )

    assert proc.stdout is not None

    has_project_error = False
    suppressing = False

    for line in proc.stdout:
        stripped = line.rstrip()

        # Check if this is a new diagnostic (error/warning)
        m = _DIAG_RE.match(stripped)
        if m:
            filepath, severity = m.group(1), m.group(2)
            is_project = filepath.startswith(_PROJECT_PREFIX)
            if severity == "error" and not is_project:
                suppressing = True
                continue
            # New project diagnostic ends any ongoing suppression
            suppressing = False
            if severity == "error" and is_project:
                has_project_error = True
            sys.stdout.write(line)
            continue

        if suppressing:
            # Suppress: note lines, context/source lines, "Error while processing",
            # "Found compiler error(s).", and the "N warnings and M errors" summary
            if (
                _NOTE_RE.match(stripped)
                or _CONTEXT_RE.match(stripped)
                or stripped.startswith("Error while processing")
                or "Found compiler error(s)." in stripped
                or re.match(r"^\d+ warnings? and \d+ errors? generated\.", stripped)
                or stripped.startswith("Suppressed ")
                or stripped.startswith("Use -header-filter=")
                or stripped.startswith("Use -system-headers")
            ):
                continue
            # Non-diagnostic line (e.g. progress) ends suppression
            suppressing = False

        sys.stdout.write(line)

    proc.wait()

    if has_project_error:
        return 1
    return 0


if __name__ == "__main__":
    sys.exit(main())
