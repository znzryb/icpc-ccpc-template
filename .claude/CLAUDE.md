# ICPC-CCPC 模板项目

这是一份用纯 LaTeX 书写的 ICPC/CCPC 算法竞赛模板。

## 项目结构

- `template-main.tex` —— 主文件（preamble + 目录 + 按序 `\input` 各章节），正文内容已拆到 `sections/`。
- `sections/` —— 各章节 `.tex`（`01_dream_start` … `19_misc_tricks`），所有算法/数据结构/几何代码块在这里，用 minted 排版。**编号是连续的**：新增章节要插在中间时，把后续文件一起顺延重命名并同步 `template-main.tex` 的 `\input` 列表，不要用 `05b` 这种后缀（2026-08-01 已把 `05b_advanced_math` 正名为 `06` 并顺延后续）。
- `_minted-template-main/` —— minted cachedir（编译产物，勿手改）。
- `out/` —— xelatex 输出目录。
- `TemplateDetailedExplain/` —— 部分模板的详细讲解。
- `template-check/`、`archive/`、`snippets/`、`image/` —— 辅助资源。
- `.autocp` —— autocp 插件元信息（题目样例等），勿手改。
- `TemporaryPrint/` —— 临时打印页，一个打印页一个子文件夹（见下节）。

## 临时打印（正式板子已送印）

`template-main.pdf` 已经整本送打印店印好了。之后用户要「临时打印 / 补印 / 打一页新改的」时，**不整本重印**：

- 先照常改 `sections/*.tex`（正式板子仍是唯一真身），再在 `TemporaryPrint/<主题>/` 下建一个**独立文件夹**放这一页：
  `<主题>_temporary_print.tex`（主题 = 文件夹名，`-` 换 `_`）+ `fragment.tex`（复制改好的 subsection）+ `build.sh` + `README.md`，`TemporaryPrint/` 顶层不散放 tex。
- 直接 `cp -R TemporaryPrint/mcmf TemporaryPrint/<主题>` 起步，主文件改名后跑 `./build.sh` 出 `out/<主题>_temporary_print.pdf`（`out/` 不入库）。
- 完整约定见 `TemporaryPrint/README.md`，新增一页时顺手把它的目录树补上。

## 编译

`template-main.tex` 用 xelatex + minted + `-shell-escape` 编译，cachedir = `_minted-template-main`。
完整 flag / out 目录 / PATH 等以 `claude-latex-compile` skill 为单一信息源，不在此复刻。

## 几何模板的真身、同步点、对拍

@geometry-test.md
