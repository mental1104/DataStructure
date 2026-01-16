from __future__ import annotations

import argparse
import html
import json
import statistics
from pathlib import Path


def is_placeholder(status: str) -> bool:
    st = (status or "").lower()
    return st in ("placeholder", "unknown", "fail", "failure", "error", "na", "n/a")


def badge_color(pct: float, status: str) -> str:
    if is_placeholder(status):
        return "#9f9f9f"
    if pct >= 95:
        return "#4c1"
    if pct >= 85:
        return "#97CA00"
    if pct >= 70:
        return "#dfb317"
    if pct >= 50:
        return "#fe7d37"
    return "#e05d44"


def est_w(text: str) -> int:
    return max(40, int(len(text) * 7 + 18))


def svg_badge(label: str, msg: str, color: str) -> str:
    lw = est_w(label)
    mw = est_w(msg)
    w = lw + mw
    label_e = html.escape(label)
    msg_e = html.escape(msg)
    aria = html.escape(f"{label}: {msg}")
    return "\n".join(
        [
            f'<svg xmlns="http://www.w3.org/2000/svg" width="{w}" height="20" role="img" aria-label="{aria}">',
            '  <linearGradient id="s" x2="0" y2="100%">',
            '    <stop offset="0" stop-color="#bbb" stop-opacity=".1"/>',
            '    <stop offset="1" stop-opacity=".1"/>',
            "  </linearGradient>",
            f'  <clipPath id="r"><rect width="{w}" height="20" rx="3" fill="#fff"/></clipPath>',
            '  <g clip-path="url(#r)">',
            f'    <rect width="{lw}" height="20" fill="#555"/>',
            f'    <rect x="{lw}" width="{mw}" height="20" fill="{color}"/>',
            f'    <rect width="{w}" height="20" fill="url(#s)"/>',
            "  </g>",
            '  <g fill="#fff" text-anchor="middle" font-family="Verdana,Geneva,DejaVu Sans,sans-serif" font-size="11">',
            f'    <text x="{lw/2}" y="14">{label_e}</text>',
            f'    <text x="{lw + mw/2}" y="14">{msg_e}</text>',
            "  </g>",
            "</svg>",
        ]
    )


def pct_msg(pct: float, status: str) -> str:
    return "N/A" if is_placeholder(status) else f"{pct:.1f}%"


def write_badge(path: Path, label: str, msg: str, status: str) -> None:
    path.parent.mkdir(parents=True, exist_ok=True)
    pct = 0.0
    if msg.endswith("%"):
        try:
            pct = float(msg[:-1])
        except Exception:
            pct = 0.0
    path.write_text(svg_badge(label, msg, badge_color(pct, status)), encoding="utf-8")


def compiler_slug(comp: str) -> str:
    s = (comp or "unknown").lower()
    if "clang" in s:
        return "clangpp"
    if "g++" in s or "gcc" in s:
        return "gpp"
    if "msvc" in s:
        return "msvc"
    return "".join(ch for ch in s.replace("+", "p").replace(" ", "") if ch.isalnum() or ch in "-_")


def build_cpp_module(cov_root: Path, out_root: Path) -> None:
    module_root = out_root / "modules/cpp-coverage"
    badges = module_root / "badges"
    data = module_root / "data"
    badges.mkdir(parents=True, exist_ok=True)
    data.mkdir(parents=True, exist_ok=True)

    raw = []
    cpp_root = cov_root / "cpp"
    if cpp_root.exists():
        for p in cpp_root.rglob("cov.json"):
            key = p.parent.name
            try:
                d = json.loads(p.read_text(encoding="utf-8"))
            except Exception:
                d = {}
            os_name = str(d.get("os", "unknown"))
            comp = str(d.get("compiler", "unknown"))
            try:
                cxx = int(d.get("cxx_std", 0) or 0)
            except Exception:
                cxx = 0
            try:
                pct = float(d.get("line_pct", 0.0) or 0.0)
            except Exception:
                pct = 0.0
            status = str(d.get("status", "ok"))
            raw.append(
                {
                    "key": key,
                    "os": os_name,
                    "compiler": comp,
                    "compiler_slug": compiler_slug(comp),
                    "cxx_std": cxx,
                    "line_pct": pct,
                    "status": status,
                    "src": str(p),
                }
            )

    expected = []
    for std in (11, 14, 17, 20, 23):
        expected.extend(
            [
                ("linux", "clangpp", std),
                ("linux", "gpp", std),
                ("macos", "clangpp", std),
                ("macos", "gpp", std),
                ("windows", "msvc", std),
            ]
        )

    def score(e):
        s = 0
        if not is_placeholder(e["status"]):
            s += 1000
        return s + e["line_pct"]

    matrix_entries = [e for e in raw if "integration" not in e["key"]]
    best = {}
    for e in matrix_entries:
        k = (e["os"], e["compiler_slug"], e["cxx_std"])
        if k not in best or score(e) > score(best[k]):
            best[k] = e
    for e in raw:
        if "integration" not in e["key"]:
            continue
        k = (e["os"], e["compiler_slug"], e["cxx_std"])
        if k not in best:
            best[k] = e

    selected = []
    for (osn, cs, std) in expected:
        e = best.get((osn, cs, std))
        if e:
            msg = pct_msg(e["line_pct"], e["status"])
            write_badge(badges / f"{osn}-{cs}-cxx{std}.svg", "cov", msg, e["status"])
            selected.append(e)
        else:
            write_badge(badges / f"{osn}-{cs}-cxx{std}.svg", "cov", "N/A", "na")

    real = [e["line_pct"] for e in selected if not is_placeholder(e["status"])]
    overall = statistics.mean(real) if real else 0.0
    overall_msg = "N/A" if not real else f"{overall:.1f}%"
    write_badge(badges / "overall.svg", "cov", overall_msg, "placeholder" if not real else "ok")

    data_entries = []
    for (osn, cs, std) in expected:
        e = best.get((osn, cs, std))
        if e:
            data_entries.append(e)
        else:
            data_entries.append(
                {
                    "key": f"{osn}-{cs}-cxx{std}",
                    "os": osn,
                    "compiler": cs,
                    "compiler_slug": cs,
                    "cxx_std": std,
                    "line_pct": 0.0,
                    "status": "na",
                    "src": "",
                }
            )

    (data / "coverage.json").write_text(
        json.dumps({"overall": overall if real else None, "entries": data_entries}, ensure_ascii=False, indent=2),
        encoding="utf-8",
    )

    os_list = ["linux", "macos", "windows"]
    stds = [11, 14, 17, 20, 23]
    comp_by_os = {"linux": ["clangpp", "gpp"], "macos": ["clangpp", "gpp"], "windows": ["msvc"]}

    def cell(osn, cs, st):
        fn = f"badges/{osn}-{cs}-cxx{st}.svg"
        return f'<img alt="{osn}-{cs}-cxx{st}" src="{fn}">'

    rows = []
    for osn in os_list:
        for cs in comp_by_os[osn]:
            tds = "".join(f"<td>{cell(osn, cs, st)}</td>" for st in stds)
            rows.append(f"<tr><th>{osn} / {cs}</th>{tds}</tr>")

    index = f"""<!doctype html>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>C++ Coverage</title>
<h1>C++ Coverage</h1>
<p><img alt="overall" src="badges/overall.svg"></p>

<h2>Matrix</h2>
<table border="1" cellpadding="6" cellspacing="0">
  <thead>
    <tr><th>platform / compiler</th><th>C++11</th><th>C++14</th><th>C++17</th><th>C++20</th><th>C++23</th></tr>
  </thead>
  <tbody>
    {''.join(rows)}
  </tbody>
</table>

<p>Data: <a href="data/coverage.json">coverage.json</a></p>
"""
    module_root.mkdir(parents=True, exist_ok=True)
    (module_root / "index.html").write_text(index, encoding="utf-8")


def write_root_index(out_root: Path) -> None:
    out_root.mkdir(parents=True, exist_ok=True)
    index = """<!doctype html>
<meta charset="utf-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Coverage Dashboard</title>
<h1>Coverage Dashboard</h1>
<p><a href="modules/cpp-coverage/"><img alt="overall" src="modules/cpp-coverage/badges/overall.svg"></a></p>
<p><a href="modules/cpp-coverage/">Open C++ Coverage Dashboard</a></p>
"""
    (out_root / "index.html").write_text(index, encoding="utf-8")


def main() -> None:
    ap = argparse.ArgumentParser()
    ap.add_argument("--out", required=True)
    ap.add_argument("--cov-root", default="_cov")
    args = ap.parse_args()

    out_root = Path(args.out)
    cov_root = Path(args.cov_root)
    build_cpp_module(cov_root, out_root)
    write_root_index(out_root)


if __name__ == "__main__":
    main()
