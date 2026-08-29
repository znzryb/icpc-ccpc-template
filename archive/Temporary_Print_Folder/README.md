# Temporary_Print_Folder —— 单模块临时打印工位

`template-main.pdf` 是近 200 页的一整本书。改了一个模板类就整本重印，既费时间又费钱，
所以这里放一个「只印一小块」的入口。

## 用法

1. 把要印的片段贴进 `fragment.tex`（通常就是 `sections/*.tex` 里的一个 `\subsection`，
   连说明文字和 `minted` 代码块一起复制，**不要**带 `\section{...}`、也不要带 `multicols*`
   环境——双栏由 `temporary_print.tex` 统一包）。
2. 顺手把 `temporary_print.tex` 里居中标题那两行的模块名 / 日期改掉。
3. 在本目录编译（`claude-latex-compile` 规范的标准命令，跑两遍 + 扫四类 log 问题）：

   ```bash
   mkdir -p out \
   && xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
        -shell-escape -output-directory=out temporary_print.tex \
   && xelatex -halt-on-error -8bit -synctex=1 -interaction=nonstopmode -file-line-error \
        -shell-escape -output-directory=out temporary_print.tex
   ```

   收尾扫 log：

   ```bash
   awk '/^!/ || /^[^ ]+:[0-9]+: /'              out/temporary_print.log   # hard error
   awk '/^Overfull/'                            out/temporary_print.log   # 超宽行
   awk '/Missing character/'                    out/temporary_print.log   # 缺字
   awk '/Command requires any of the packages:/' out/temporary_print.log   # 未加载宏包
   ```

4. 拿 `out/temporary_print.pdf` 去打印。

## 注意

- **一次性工位**：`fragment.tex` 的内容随时会被下一个模块覆盖，不要当归档用；
  想留档请另存到 `archive/` 下带日期的文件里。
- `temporary_print.tex` 的 preamble 是 `template-main.tex` 的最小可用子集，
  页边距 / 字体 / `minted` 设置 / 双栏都对齐正式板子，所以折行位置跟印在书里时一致。
  如果贴进来的片段用到了 tikz / forest / tcolorbox，需要自己按主文档补对应宏包。
- minted 的 cachedir 是 `_minted-temporary-print`，与主文档分开，互不影响。
- `out/` 与 `_minted-*/` 都不进版本库。

## 当前内容

图论 · 最小费用最大流（MCMF），对应 `sections/10_graph.tex` 里的同名 subsection。
