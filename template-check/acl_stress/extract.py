#!/usr/bin/env python
"""从 .tex 抽第一个 \begin{minted}{cpp} 块（= 结构体定义）"""
import re
import sys

path, out = sys.argv[1], sys.argv[2]
src = open(path, encoding="utf-8").read()
m = re.search(r"\\begin\{minted\}\{cpp\}\n(.*?)\n\s*\\end\{minted\}", src, re.S)
assert m, f"no minted cpp block in {path}"
open(out, "w", encoding="utf-8").write(m.group(1) + "\n")
print(f"{path} -> {out}: {len(m.group(1).splitlines())} lines")
