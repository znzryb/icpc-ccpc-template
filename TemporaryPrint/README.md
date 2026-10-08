# TemporaryPrint —— 临时打印页

`template-main.pdf` 是近 200 页的一整本书，已经送去打印店印好了。之后改了某一个模板，
整本重印既费时间又费钱，所以只把改动的那一块单独排成一两页的 PDF 补印。

## 规范：一个临时打印页 = 一个文件夹

```
TemporaryPrint/
├── README.md              ← 本文件（顶层只放它，不散放 tex）
├── mcmf/                  ← 最小费用最大流（2026-08-29）
├── string-hash/           ← 字符串哈希精简版（2026-10-04）
├── dsu/                   ← 并查集 DSU 精简版（2026-10-07）
└── vandermonde/           ← 范德蒙德卷积（含中位数例题）+ 常用组合恒等式（2026-10-08）
```

每个子文件夹固定这几样：

| 文件 | 作用 |
|---|---|
| `<主题>_temporary_print.tex` | 主文件（主题 = 文件夹名，`-` 换 `_`，如 `string_hash_temporary_print.tex`；带前缀是为了几页 PDF 放一起时不混）：从 `template-main.tex` 裁出来的最小 preamble + 居中标题（模块名、来源 section、日期） |
| `fragment.tex` | 要印的片段：从 `sections/*.tex` 复制的一个 `\subsection`（说明文字 + `minted` 代码块），不带 `\section` 和 `multicols*` |
| `build.sh` | `./build.sh` 编译：xelatex 两遍 + 扫 error / Overfull / Missing character / 未加载宏包四类，结果应全 0 |
| `README.md` | 一两行：这页印的是什么、补印自哪里 |

## 新建一页

1. `cp -R mcmf <新主题>`，删掉复制过来的 `out/`；
2. 先改正式板子 `sections/*.tex`，再把改好的 subsection 复制进 `fragment.tex`（正式板子仍是唯一真身）；
3. 把主文件改名为 `<新主题>_temporary_print.tex`（`build.sh` 按文件夹名自动找它，不用改脚本），改标题那两行、改 `README.md`；
4. `./build.sh`，拿 `out/<主题>_temporary_print.pdf` 去打印。

## 约定

- 排版是**单栏**（补印页只看一个模块，单栏行更长、代码不用挤着折）；页边距 / 字体 / `minted` 设置与正式板子一致。
  片段用到 tikz / forest / tcolorbox 时自己按主文档补宏包。
- minted cachedir 是 `_minted-temporary-print`，与主文档分开。
- 各子目录的 `out/` 不进版本库（`.gitignore`：`TemporaryPrint/*/out/`）。
