# ICPC-CCPC 模板项目

这是一份用纯 LaTeX 书写的 ICPC/CCPC 算法竞赛模板。

## 项目结构

- `template-main.tex` —— 模板主体（单文件，~480KB），所有算法/数据结构/几何代码块都在这里，用 minted 排版。
- `_minted-template-main/` —— minted cachedir（编译产物，勿手改）。
- `out/` —— xelatex 输出目录。
- `TemplateDetailedExplain/` —— 部分模板的详细讲解。
- `template-check/`、`archive/`、`snippets/`、`image/` —— 辅助资源。
- `.autocp` —— autocp 插件元信息（题目样例等），勿手改。

## 编译

`template-main.tex` 用 xelatex + minted + `-shell-escape` 编译，cachedir = `_minted-template-main`。
完整 flag / out 目录 / PATH 等以 `claude-latex-compile` skill 为单一信息源，不在此复刻。

## 几何模板的真身、同步点、对拍

@geometry-test.md
