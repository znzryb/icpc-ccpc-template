# mcmf —— 临时打印页：最小费用最大流

`template-main.pdf` 是近 200 页的一整本书。改了一个模板类就整本重印，既费时间又费钱，
所以这里放一个「只印一小块」的入口。

## 用法

1. 把要印的片段贴进 `fragment.tex`（通常就是 `sections/*.tex` 里的一个 `\subsection`，
   连说明文字和 `minted` 代码块一起复制，**不要**带 `\section{...}`、也不要带 `multicols*`
   环境——补印页是**单栏**排版，双栏留给正式板子）。
2. 顺手把 `temporary_print.tex` 里居中标题那两行的模块名 / 日期改掉。
3. 运行 `./build.sh`（xelatex 两遍 + 扫 error / Overfull / Missing character / 未加载宏包四类，结果应全 0）。

4. 拿 `out/temporary_print.pdf` 去打印。

## 注意

- 本目录只放这一个临时打印页；新的补印内容在 `TemporaryPrint/` 下另建文件夹（规范见上级 `README.md`），不覆盖这里。
- `temporary_print.tex` 的 preamble 是 `template-main.tex` 的最小可用子集，
  页边距 / 字体 / `minted` 设置都对齐正式板子，但**排成单栏**——补印页只是拿来看一个模块，
  单栏行更长、代码不用挤着折，比照搬书里的双栏好读。
  如果贴进来的片段用到了 tikz / forest / tcolorbox，需要自己按主文档补对应宏包。
- minted 的 cachedir 是 `_minted-temporary-print`，与主文档分开，互不影响。
- `out/` 与 `_minted-*/` 都不进版本库。

## 当前内容

图论 · 最小费用最大流（MCMF），对应 `sections/10_graph.tex` 里的同名 subsection。
