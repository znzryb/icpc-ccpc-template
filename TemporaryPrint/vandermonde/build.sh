#!/usr/bin/env bash
# 编译本目录的临时打印页 → out/<主题>_temporary_print.pdf（主题 = 文件夹名，- 换成 _）
# 主文件名带主题前缀，几页 PDF 放一起打印时不会混
# 规范同 claude-latex-compile：xelatex 两遍 + 扫四类 log 问题（结果全 0 才算干净）
set -euo pipefail
cd "$(dirname "$0")"
export PATH="$HOME/miniconda3/bin:/Library/TeX/texbin:/usr/local/texlive/2026/bin/universal-darwin:/opt/homebrew/bin:$PATH"
name="$(basename "$PWD" | tr - _)_temporary_print"

mkdir -p out \
&& xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
	-shell-escape -output-directory=out "$name.tex" \
&& xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
	-shell-escape -output-directory=out "$name.tex"

log="out/$name.log"
echo "==== log 扫描 ===="
echo "error:    $(awk '/^!/ || /^[^ ]+:[0-9]+: /' "$log" | wc -l)"
echo "overfull: $(awk '/^Overfull/' "$log" | wc -l)"
echo "missing:  $(awk '/Missing character/' "$log" | wc -l)"
echo "package:  $(awk '/Command requires any of the packages:/' "$log" | wc -l)"
echo "pdf:      $(pwd)/out/$name.pdf"
