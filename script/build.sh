#!/usr/bin/env bash
# 编译本仓库的 LaTeX 模板：只需给出主文件名即可。
#
#   ./script/build.sh                        # 默认编译 template-main.tex
#   ./script/build.sh template-main          # 带不带 .tex 都行
#   ./script/build.sh atcoder-template/main  # 带路径也行，产物落在该文件所在目录的 out/
#   ./script/build.sh -1 template-main       # 只跑一遍（快速看语法错，交叉引用/目录可能不准）
#
# 产物：<主文件所在目录>/out/<basename>.pdf
# 编译后自动扫 log 的四类问题：Error / Overfull \hbox / Missing character / 宏包未加载。
# 有 Error 时退出码非 0。

set -uo pipefail

# minted 要调 pygmentize（miniconda 里），xelatex 在 texbin
export PATH="$HOME/miniconda3/bin:/Library/TeX/texbin:/opt/homebrew/bin:$PATH"

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

passes=2
if [[ "${1-}" == "-1" ]]; then
  passes=1
  shift
fi

target="${1-template-main}"
target="${target%.tex}"

# 相对路径按「先看当前目录，再看仓库根」解析，方便在任意目录下调用
if [[ -f "${target}.tex" ]]; then
  tex_path="$(cd "$(dirname "${target}.tex")" && pwd)/$(basename "${target}.tex")"
elif [[ -f "${REPO_ROOT}/${target}.tex" ]]; then
  tex_path="${REPO_ROOT}/${target}.tex"
else
  echo "找不到主文件：${target}.tex（已在当前目录和 ${REPO_ROOT} 下找过）" >&2
  exit 1
fi

work_dir="$(dirname "$tex_path")"
base="$(basename "$tex_path" .tex)"
log="${work_dir}/out/${base}.log"

command -v xelatex >/dev/null || { echo "PATH 里找不到 xelatex（mactex 装了吗？）" >&2; exit 1; }
command -v pygmentize >/dev/null || echo "警告：PATH 里没有 pygmentize，含 minted 的文档会编译失败" >&2

cd "$work_dir"
mkdir -p out

run_xelatex() {
  xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
    -shell-escape -output-directory=out "${base}.tex"
}

echo "==> 编译 ${tex_path}（${passes} 遍）"
for ((i = 1; i <= passes; i++)); do
  echo "--- pass ${i}/${passes} ---"
  if ! run_xelatex >/dev/null; then
    echo
    echo "!! 第 ${i} 遍编译失败，错误如下："
    awk '/^!/ || /^[^ ]+:[0-9]+: /' "$log" | head -30
    echo "完整日志：${log}"
    exit 1
  fi
done

# 交叉引用 / 目录没收敛时补跑一遍
if [[ $passes -ge 2 ]] && awk '/Rerun to get|rerunfilecheck Warning/' "$log" | grep -q .; then
  echo "--- 检测到 rerun 警告，补跑一遍 ---"
  if ! run_xelatex >/dev/null; then
    echo "!! 补跑失败，详见 ${log}" >&2
    exit 1
  fi
fi

# 以下检查一律用 awk：xelatex 的 log 含二进制控制字符，grep 会静默罢工
count() { awk "$1" "$log" | wc -l | tr -d ' '; }

n_err=$(count '/^!/ || /^[^ ]+:[0-9]+: /')
n_over=$(count '/^Overfull/')
n_miss=$(count '/Missing character/')
n_pkg=$(count '/Command requires any of the packages:/')

echo
echo "==> 完成：${work_dir}/out/${base}.pdf"
awk '/Output written on/ {print "    " $0}' "$log" | tail -1
echo "    Error           : ${n_err}"
echo "    Overfull \\hbox  : ${n_over}"
echo "    Missing character: ${n_miss}"
echo "    宏包未加载警告   : ${n_pkg}"

if [[ "$n_err" != "0" ]]; then
  echo
  echo "!! log 里仍有 error："
  awk '/^!/ || /^[^ ]+:[0-9]+: /' "$log" | head -20
  exit 1
fi
