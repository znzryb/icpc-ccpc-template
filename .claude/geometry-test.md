# 几何模板的真身、同步点、对拍

本项目（ICPC-CCPC 模板）的计算几何模板 **真身在 `sections/13_geometry.tex`**：
每个带 `% geometry-code: <name>` 标记的 minted 代码块就是一段真身代码。
Espanso 的 `computational_geometry.yml`（`~/Library/Application Support/espanso/match/`）
是由脚本从 tex 生成的镜像，**不要手改 yml**。

## 三个文件

- `sections/13_geometry.tex`：代码 + 讲解（真身）。
- `script/geometry_snippets.json`：代码块顺序、23 个 Espanso 触发词由哪些代码块拼成、触发词之间的依赖，
  外加别名触发词 `aliases`、espanso 搜索栏关键词 `search_terms`（都由 `--write-espanso` 落到 yml，别手改 yml）。
  新增 / 拆分代码块时要同步这里，否则 `geometry.py` 的校验会失败。
- `script/geometry.py`：从 tex 抽代码，校验 / 生成 Espanso，`--stats` 出缩减统计。

## 改模板的流程

1. 改 tex 里的代码块（改完顺手改旁边的讲解）。
2. 在仓库根目录跑对拍：`~/miniconda3/bin/python template-check/geometry/run.py`
   （23 个触发词依赖组合编译 + 1720 个 Shapely 对拍用例 + `DB=double/long double` 的 LOCAL 断言）。
   在没有 espanso 的机器上用 `--espanso <某份 yml>` 指定文件；`--write-espanso` 可以先把它生成出来。
3. 同步 Espanso：`~/miniconda3/bin/python script/geometry.py --write-espanso`。
   注意它用 `yaml.dump` 重写整个文件，yml 里的注释（含以前的 changelog 块）不会保留，
   沉淀记录（日期 + 来源 + AC 链接）写到 git 提交信息和 tex 里的 change log 链接。
4. 有 xelatex 的机器上重编 PDF（flag 见 `claude-latex-compile` skill），`out/template-main.pdf` 一起提交。

Espanso 侧还有一份独立的专项随机测试
`~/Library/Application Support/espanso/geometry_tests/gen_and_test.py [次数]`
（严格/弱凸、FULL/LOWER/UPPER、浮点近重合、重排），它从 yml 抽片段，同步完 yml 后再跑一次。

细节（能力清单、类型约定、缩减统计）见 `template-check/geometry/README.md`。
