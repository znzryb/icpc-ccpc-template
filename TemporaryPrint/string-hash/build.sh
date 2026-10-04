#!/usr/bin/env bash
# 编译本目录的临时打印页 → out/temporary_print.pdf
# 规范同 claude-latex-compile：xelatex 两遍 + 扫四类 log 问题（结果全 0 才算干净）
set -euo pipefail
cd "$(dirname "$0")"
export PATH="$HOME/miniconda3/bin:/Library/TeX/texbin:/usr/local/texlive/2026/bin/universal-darwin:/opt/homebrew/bin:$PATH"

mkdir -p out \
&& xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
	-shell-escape -output-directory=out temporary_print.tex \
&& xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
	-shell-escape -output-directory=out temporary_print.tex

log=out/temporary_print.log
echo "==== log 扫描 ===="
echo "error:    $(awk '/^!/ || /^[^ ]+:[0-9]+: /' $log | wc -l)"
echo "overfull: $(awk '/^Overfull/' $log | wc -l)"
echo "missing:  $(awk '/Missing character/' $log | wc -l)"
echo "package:  $(awk '/Command requires any of the packages:/' $log | wc -l)"
echo "pdf:      $(pwd)/out/temporary_print.pdf"
