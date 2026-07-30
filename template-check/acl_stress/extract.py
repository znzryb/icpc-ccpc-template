#!/usr/bin/env python
r"""从 .tex 抽出「模板本体」那个 \begin{minted}{cpp} 块。

用法: extract.py <file.tex> <out.inc> <marker>

按 marker（如 "struct segtree {"）定位，而不是取「第一个 cpp 块」——
示例用法排在模板本体之前，取第一个会抽到示例。
命中数不等于 1 就直接报错，避免静默抽错。
"""
import re
import sys

path, out, marker = sys.argv[1], sys.argv[2], sys.argv[3]
src = open(path, encoding="utf-8").read()
blocks = re.findall(r"\\begin\{minted\}\{cpp\}\n(.*?)\n\s*\\end\{minted\}", src, re.S)
hit = [b for b in blocks if marker in b]
if len(hit) != 1:
    sys.exit(f"{path}: 含 {marker!r} 的 minted 块有 {len(hit)} 个（期望 1），"
             f"该文件共 {len(blocks)} 个 cpp 块 —— 模板本体被改名或重复了？")
open(out, "w", encoding="utf-8").write(hit[0] + "\n")
print(f"{path} -> {out}: {len(hit[0].splitlines())} lines "
      f"(marker {marker!r}; 该文件共 {len(blocks)} 个 cpp 块)")
