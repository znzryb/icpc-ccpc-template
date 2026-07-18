# icpc-ccpc-template

纯 LaTeX 书写的 ICPC / CCPC 算法竞赛模板（板子），minted 排版代码，编译产物为可打印的 PDF。

成品：[`out/template-main.pdf`](out/template-main.pdf)

## 结构

- `template-main.tex` —— 主文件（input 列表 + preamble）
- `sections/` —— 各章节正文（数学 / 计数 / 博弈 / 数据结构 / 图论 / 字符串 / DP / 计算几何 / STL / 常用技巧等 16 章）
- `TemplateDetailedExplain/` —— 部分模板的详细讲解
- `template-check/` —— 模板代码的本地验证工程（CMake）
- `archive/` / `snippets/` / `image/` —— 快照、代码片段与图片资源

## 编译

xelatex + minted，需要 `-shell-escape`，跑两遍：

```bash
mkdir -p out
xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
  -shell-escape -output-directory=out template-main.tex \
  && xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
  -shell-escape -output-directory=out template-main.tex
```

依赖：TeX Live（xelatex）、Pygments（`pygmentize`，minted 用）、中文字体 Noto Serif CJK SC。

## 来源

本仓库由 `DoProblemAsMyTaste` 私有做题仓库中的 `模版/纯 latex 书写的模板/ICPC-CCPC-模板/` 目录迁出（2026-07-19 快照迁移），此后以本仓库为唯一维护点，原目录已归档。
