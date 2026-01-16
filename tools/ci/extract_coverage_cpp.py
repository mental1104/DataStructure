from __future__ import annotations

import argparse
import json
import os
import sys
import time
import xml.etree.ElementTree as ET
from pathlib import Path
from typing import Optional, Tuple

ROOT = Path(__file__).resolve().parents[2]

LCOV_NAME_HINTS = [
    "lcov.info", "coverage.info", "coverage-lcov.info", "total.info"
]
XML_NAME_HINTS = [
    "coverage.xml", "cobertura.xml", "cobertura-coverage.xml"
]


def parse_lcov_info(p: Path) -> Optional[float]:
    lf = 0
    lh = 0
    try:
        for line in p.read_text(encoding="utf-8", errors="ignore").splitlines():
            if line.startswith("LF:"):
                lf += int(line.split(":", 1)[1])
            elif line.startswith("LH:"):
                lh += int(line.split(":", 1)[1])
        if lf <= 0:
            return None
        return round(lh * 100.0 / lf, 2)
    except Exception:
        return None


def parse_cobertura_xml(p: Path) -> Optional[float]:
    try:
        root = ET.parse(p).getroot()
        line_rate = root.attrib.get("line-rate")
        if line_rate is not None:
            return round(float(line_rate) * 100.0, 2)

        lc = root.attrib.get("lines-covered")
        lv = root.attrib.get("lines-valid")
        if lc is not None and lv is not None:
            lv_i = int(lv)
            if lv_i <= 0:
                return None
            return round(int(lc) * 100.0 / lv_i, 2)

        return None
    except Exception:
        return None


def find_candidates() -> list[Path]:
    bases = [
        ROOT / "dist",
        ROOT / "out",
        ROOT / "build",
        ROOT / "coverage",
        ROOT,
    ]
    seen = set()
    cands: list[Path] = []

    for b in bases:
        if not b.exists():
            continue
        for name in LCOV_NAME_HINTS + XML_NAME_HINTS:
            p = b / name
            if p.exists() and p.is_file():
                rp = p.resolve()
                if rp not in seen:
                    seen.add(rp)
                    cands.append(rp)

    now = time.time()
    for b in bases:
        if not b.exists():
            continue
        for p in b.rglob("*"):
            if not p.is_file():
                continue
            if p.suffix.lower() not in (".info", ".xml"):
                continue
            try:
                st = p.stat()
            except OSError:
                continue
            if now - st.st_mtime > 2 * 3600:
                continue
            rp = p.resolve()
            if rp in seen:
                continue
            seen.add(rp)
            cands.append(rp)

    cands.sort(key=lambda x: x.stat().st_mtime, reverse=True)
    return cands


def extract_pct() -> Tuple[float, str]:
    cands = find_candidates()
    for p in cands:
        if p.suffix.lower() == ".info":
            v = parse_lcov_info(p)
            if v is not None:
                return v, str(p.relative_to(ROOT))
        if p.suffix.lower() == ".xml":
            v = parse_cobertura_xml(p)
            if v is not None:
                return v, str(p.relative_to(ROOT))
    raise RuntimeError(
        "No supported coverage report found. "
        "Expected lcov tracefile (*.info) or Cobertura XML (*.xml)."
    )


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--os", required=True)
    ap.add_argument("--compiler", required=True)
    ap.add_argument("--cxx-std", required=True)
    ap.add_argument("--out", required=True)
    args = ap.parse_args()

    pct, src = extract_pct()
    data = {
        "os": args.os,
        "compiler": args.compiler,
        "cxx_std": int(args.cxx_std),
        "line_pct": pct,
        "source": src,
        "sha": os.environ.get("GITHUB_SHA", ""),
        "run_id": os.environ.get("GITHUB_RUN_ID", ""),
    }
    Path(args.out).write_text(json.dumps(data, ensure_ascii=False, indent=2), encoding="utf-8")
    print(f"coverage line_pct={pct}% source={src}")


if __name__ == "__main__":
    try:
        main()
    except Exception as e:
        print(str(e), file=sys.stderr)
        sys.exit(2)
