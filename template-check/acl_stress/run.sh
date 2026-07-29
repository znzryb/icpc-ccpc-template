#!/bin/bash
# ACL 移植版线段树对拍：从 atcoder-template/*.tex 抽 minted 代码块 ->
# 与官方 ac-library + 暴力三方对拍（ASan + UBSan）
#
# 用法： bash template-check/acl_stress/run.sh
# 依赖： 官方 ac-library（只用于当参考答案，模板本身不依赖它）
#        ACL_DIR 可覆盖，默认指向本机 DoProblemAsMyTaste/library/ac-library
set -euo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
TEX_DIR="$ROOT/atcoder-template"

PY=${PY:-$HOME/miniconda3/bin/python}
CXX=${CXX:-/opt/homebrew/opt/llvm@22/bin/clang++}
ACL_DIR=${ACL_DIR:-/Users/zzy/Desktop/DoProblemAsMyTaste/library/ac-library}
SDK=${SDK:-/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX.sdk}
GCC_INC=${GCC_INC:-/opt/homebrew/opt/gcc/include/c++/15}
GCC_TRIPLE_INC=${GCC_TRIPLE_INC:-$GCC_INC/aarch64-apple-darwin25}
GCC_LIB=${GCC_LIB:-/opt/homebrew/opt/gcc/lib/gcc/current}

[ -d "$ACL_DIR/atcoder" ] || {
	echo "找不到官方 ac-library: $ACL_DIR（用 ACL_DIR=... 指定）" >&2
	exit 1
}

cd "$HERE"
"$PY" extract.py "$TEX_DIR/acl_segtree.tex" seg.inc
"$PY" extract.py "$TEX_DIR/acl_lazysegtree.tex" lazyseg.inc

# macOS clang + libstdc++ 工具链，见
# ~/Desktop/DoProblemAsMyTaste/.claude/rules/macos-clang-libstdcxx-toolchain.md
"$CXX" -std=c++20 -O1 -g -Wall -Wextra \
	-isysroot "$SDK" -nostdinc++ -nostdlib++ \
	-isystem "$GCC_INC" -isystem "$GCC_TRIPLE_INC" \
	-isystem "$ACL_DIR" \
	-fno-omit-frame-pointer -fsanitize=address,undefined \
	-fno-sanitize=vptr,function -fno-sanitize-recover=all \
	-L "$SDK/usr/lib" -Wl,-syslibroot,"$SDK" \
	-L "$GCC_LIB" -Wl,-rpath,"$GCC_LIB" -lstdc++ \
	stress.cpp -o stress

./stress
