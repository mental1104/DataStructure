#!/usr/bin/env python3
"""Reject STL container dependencies and duplicated std algorithms in production DSA code."""

from __future__ import annotations

import re
import sys
from pathlib import Path

ROOT = Path(__file__).resolve().parents[1]
PRODUCTION_ROOT = ROOT / "src" / "dsa"

BANNED_CONTAINER_HEADERS = {
    "array",
    "deque",
    "forward_list",
    "list",
    "map",
    "queue",
    "set",
    "stack",
    "unordered_map",
    "unordered_set",
    "vector",
}

BANNED_CONTAINER_TYPES = {
    "array",
    "deque",
    "forward_list",
    "list",
    "map",
    "multimap",
    "multiset",
    "priority_queue",
    "queue",
    "set",
    "stack",
    "unordered_map",
    "unordered_multimap",
    "unordered_multiset",
    "unordered_set",
    "vector",
}

# These algorithms have repository-owned equivalents in src/dsa/algorithm or src/dsa/core.
BANNED_STD_ALGORITHMS = {
    "make_heap",
    "sort",
    "sort_heap",
    "stable_sort",
}

INCLUDE_PATTERN = re.compile(r"^\s*#\s*include\s*<([^>]+)>", re.MULTILINE)
STD_SYMBOL_PATTERN = re.compile(r"\bstd::([A-Za-z_][A-Za-z0-9_]*)\b")
BLOCK_COMMENT_PATTERN = re.compile(r"/\*.*?\*/", re.DOTALL)
LINE_COMMENT_PATTERN = re.compile(r"//[^\n]*")


def strip_comments(text: str) -> str:
    return LINE_COMMENT_PATTERN.sub("", BLOCK_COMMENT_PATTERN.sub("", text))


def line_number(text: str, offset: int) -> int:
    return text.count("\n", 0, offset) + 1


def check_file(path: Path) -> list[str]:
    relative = path.relative_to(ROOT)
    text = path.read_text(encoding="utf-8")
    code = strip_comments(text)
    violations: list[str] = []

    for match in INCLUDE_PATTERN.finditer(code):
        header = match.group(1)
        if header in BANNED_CONTAINER_HEADERS:
            violations.append(
                f"{relative}:{line_number(code, match.start())}: "
                f"禁止包含 STL 容器头文件 <{header}>"
            )

    for match in STD_SYMBOL_PATTERN.finditer(code):
        symbol = match.group(1)
        if symbol in BANNED_CONTAINER_TYPES:
            violations.append(
                f"{relative}:{line_number(code, match.start())}: "
                f"禁止在生产实现中使用 std::{symbol}"
            )
        elif symbol in BANNED_STD_ALGORITHMS:
            violations.append(
                f"{relative}:{line_number(code, match.start())}: "
                f"仓库已有对等算法，禁止使用 std::{symbol}"
            )

    return violations


def main() -> int:
    violations: list[str] = []
    for path in sorted(PRODUCTION_ROOT.rglob("*.h")):
        violations.extend(check_file(path))

    if violations:
        print("DSA STL dependency check failed:", file=sys.stderr)
        for violation in violations:
            print(f"  - {violation}", file=sys.stderr)
        return 1

    print("DSA STL dependency check passed.")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
