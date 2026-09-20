# 计算几何模板：C++17 精简版

维护来源是 `sections/13_geometry.tex` 中带 `% geometry-code:` 标记的代码块（真身）。
`script/geometry_snippets.json` 记录代码块顺序、23 个 Espanso 触发词的组合及依赖，
以及别名触发词（`aliases`）和搜索关键词（`search_terms`）；
`script/geometry.py` 只读取这些代码，不维护第二份 C++ 实现。

## 使用和同步

- `:Point` 仍包含点线核心、投影、对称、CCW、平行/垂直、相交、点点/点线/线段距离。
- 其余触发词保持原名；例如凸包需要 `:Point + :polygonArea + :makeConvexHull`。
- 卡壳组合仍同时包含直径、最远点对和最小矩形覆盖；直径与最远点对共用 `convex_diameter_indices`。
- 圆类仍带 `CircleRelation`、圆的输入/输出、直径圆构造，且支持 `DB=double` 和 `long double`。
- `:getCrossPointsCL` 同时带直线版和线段版；线段版 `getCrossPointsCS` / `circle_segment_relation` 只保留落在段内（含端点）的交点，返回的是与**圆周**的公共点个数（整段在圆内是 0）。
- 旧版只在 Espanso 中的凸多边形生成、最小圆覆盖、混合类型叉积和三参数点积已补进打印模板。
- 最近点对、两凸多边形距离、Pick 定理仍在打印模板中；没有为了统一触发词而丢掉这些功能。

在模板仓库根目录执行：

```sh
~/miniconda3/bin/python script/geometry.py                     # 23 个片段逐字校验
~/miniconda3/bin/python script/geometry.py --stats             # 有效代码统计
~/miniconda3/bin/python script/geometry.py --write-espanso     # 显式同步几何片段
~/miniconda3/bin/python template-check/geometry/run.py         # 编译矩阵 + 回归/对拍
```

`--espanso /path/to/computational_geometry.yml` 可指定另一份几何配置。
同步只更新该文件已有片段的 replace、`triggers`（主触发词 + `aliases`）、`search_terms` 字段，
保留 label 等元数据；主触发词集合不符则失败。别名和搜索关键词都在 json 里维护，不要手改 yml。
注意 `--write-espanso` 用 `yaml.dump` 重写整个文件，**yml 里的注释不会保留**；
沉淀记录写在 tex 的 change log 链接和 git 提交里，不要再往 yml 里写注释。
从新版代码改起，不要再回写旧做题目录。Espanso 测试可通过 `ICPC_TEMPLATE_ROOT` 指向此仓库。

## 设计约定（2026-09 精简版）

- 坐标类型 `T` 只用 `ll` 或 `DB`。`Wide<T>` 把整数乘积升为 `i128`，浮点保留原类型。
- `Promote` 只允许 `Point<ll> -> Point<DB>` 这种无损方向的隐式转换（`Line`、`Circle` 同）。
- `+ - cross dot` 只写同类型版本（类内 friend）；混合类型表达式靠隐式转换自动走 `DB` 版本，
  不再为每个运算符写混合类型模板。标量乘除返回公共类型，`Point<ll> / DB` 得到 `Point<DB>`，不会截断。
- `sqrtL` 分整数 / 浮点两个重载：整数先精确开方再修正，不再单独暴露 `isqrt`。
- 去掉了 `Circle` 的 `check_theta` / `check_on_circle` 调试断言，以及 `*=` `/=`。
- `min_circle_cover` 在 `outcircle_triangle` 返回退化 sentinel（`r = -1`）时退化为最远两点的直径圆，
  不再把 `r = -1` 当成半径 1 继续判包含。
- `polygon_circle_intersect_area` 的多边形参数改为模板（原只接受 `vector<Point<ll>>`）。
- 其他函数名、参数和返回语义不变。

## 能力清单与接口约定

| 模块 | 保留的能力 |
| --- | --- |
| 基础 | `sgn`、安全平方、修正开方、点/向量、混合类型加减与标量乘除、点积/叉积两参数及三参数、旋转、三种范数、输入/调试输出 |
| 点线 | 两点式/斜截式/一般式直线、投影、对称、五类 CCW、平行/垂直、线段相交、直线交点、L1/L2/L∞ 点距、点线/点段/段段距离 |
| 圆 | 五种测度、角度与圆上点互转、包含判定、位置关系与重合 sentinel、直径圆、内外接圆、线圆/圆圆交点、点切点、公切线切点、两圆/多边形圆交面积、最小圆覆盖 |
| 多边形 | 有向/无向面积、周长、正多边形面积、顺序整理、严格/弱凸判定、内外与边界判定、极角排序、Minkowski 和、半平面交 |
| 凸包/点对 | 完整/上/下凸包、严格/保留共线、直径、带原下标最远点对、最小矩形覆盖、最近点对、两不相交凸多边形距离 |
| 工具/结论 | 整数/浮点随机凸多边形、Pick 边界/内部点数、原有公式、结论、说明图与示例 |

共享辅助名：`Wide`、`Promote`、`convex_diameter_indices`。
`distancePPLinf` 返回坐标公共类型，避免整数实参在左时把混合类型距离截断。
拷贝和类型转换保留 `id`；算术派生点 `id=-1`。

数值约定：整数乘积升 `i128`，并不使坐标加减、四次表达式或无限累计免于溢出。
输入、坐标差、各级乘积与累计量必须处于对应类型范围内；圆线关系包含四次表达式。
浮点使用绝对 `eps`；近重复点的合并依赖输入尺度，不承诺任意缩放下相同拓扑。
整数转浮点仍可能舍入。`sqrtL` 返回浮点近似值。

凸包 FULL 起点仍为 `(y,x)` 最小点并按 CCW 输出，LOWER 从左到右，UPPER 从右到左。
Minkowski 和仍整理输入并返回末点重复首点的闭合边界；半平面交求有界正面积区域，无界输入自行提供题目允许的边界。
两凸多边形最近距离保留原有“不相交、双向调用取 min”前提。
随机凸多边形生成采用拒绝采样，整数范围不足以容纳指定数量的顶点时可能不终止。

## 缩减统计

统计排除注释与空白；有效行数受排版影响，非空白字符数用于核实实际代码缩减。
这里比较所有实际 Espanso 展开内容（旧 = `archive/espanso_geometry_20260920_before_slim.yml.txt`），
未把移除 C++14 附录计入收益。用 `script/geometry.py --stats` 重新生成。

| 触发词 | 有效行数（旧 → 新） | 非空白字符（旧 → 新） | 字符缩减 |
| --- | ---: | ---: | ---: |
| `:Point` | 273 → 107 | 5774 → 4550 | 21.2% |
| `:reorderPolygon` | 14 → 6 | 323 → 266 | 17.6% |
| `:polarAngleSort` | 40 → 27 | 945 → 780 | 17.5% |
| `:isConvex` | 22 → 14 | 456 → 331 | 27.4% |
| `:makeConvexHull` | 45 → 26 | 929 → 799 | 14.0% |
| `:minkowski` | 22 → 14 | 527 → 390 | 26.0% |
| `:polygonArea` | 15 → 12 | 309 → 275 | 11.0% |
| `:polygonPerimeter` | 9 → 7 | 159 → 154 | 3.1% |
| `:regularPolygonArea` | 3 → 1 | 71 → 60 | 15.5% |
| `:isInPolygon` | 18 → 12 | 307 → 288 | 6.2% |
| `:gen_convex_polygon` | 54 → 21 | 1082 → 664 | 38.6% |
| `:convexRotatingCalipers` | 118 → 58 | 2659 → 1610 | 39.5% |
| `:halfPlaneCut` | 45 → 22 | 878 → 775 | 11.7% |
| `:ccwAngle` | 11 → 9 | 249 → 189 | 24.1% |
| `:Circle` | 80 → 22 | 1558 → 1017 | 34.7% |
| `:circleRelation` | 12 → 9 | 398 → 331 | 16.8% |
| `:incircleOutTriangleCircle` | 18 → 12 | 774 → 486 | 37.2% |
| `:getCrossPointsCL` | 22 → 14 | 561 → 428 | 23.7% |
| `:getCrossPointsCC` | 25 → 18 | 679 → 538 | 20.8% |
| `:circleTangent` | 37 → 22 | 905 → 646 | 28.6% |
| `:twoCircleArea` | 14 → 11 | 354 → 326 | 7.9% |
| `:polygonCircleArea` | 47 → 18 | 1004 → 627 | 37.5% |
| `:minCircleCover` | 21 → 20 | 385 → 521 | -35.3% |
| **全部片段合计** | **965 → 482** | **21286 → 16051** | **24.6%** |

`:minCircleCover` 变长是因为加入了近共线退化的修正分支。
打印模板（PDF 章节）里的非空白、非注释代码行：1011 → 552。
C++14 附录单独归档：原代码 158 行（含注释/空行）；不再进入正式 PDF。

## 回归与对拍

`driver.cpp` 只有调用和断言，不复制算法。运行器从实际 TeX 生成临时头文件，
对全部 Espanso 内容先逐字校验，再分别编译每个触发词及其依赖组合。
临时文件在系统临时目录中生成，退出后清理。

- C++17 编译 23 种触发词依赖组合；完整算法库用 ASan/UBSan 运行。
- `LOCAL` 开启，分别实例化 `DB=double` 和 `long double`，覆盖整数/浮点/混合类型。
- 1930 个固定种子用例（20260920），Python 整数穷举距离 + Shapely/GEOS 独立参考；线段与圆那组用整数二次方程精确判根。
- 50 组种子 × 圆、椭圆、spiked、heart、lemon 五类形状，验证直径、最近点对、最小矩形和最小圆。
- 线段含点退化；凸/凹多边形含水平边和顶点命中；Minkowski 对照点对和的凸包；半平面交对照多边形求交。
- 圆交面积参考使用 8192 边内接多边形，并将已知离散面积误差上界计入断言，不能将其声称为解析精确参考。
- Espanso `geometry_tests/gen_and_test.py` 保留原有严格/弱凸、FULL/LOWER/UPPER、浮点近重合、重排专项随机测试。

固定回归包括：退化点线段相交、整数左参的混合 L∞ 距离、混合类型加减与标量乘除的返回类型、
整数开方修正（含 2^100）、最近点对距离超过旧 `INF`、空最小圆、空/点/线段矩形、全共线直径、浮点派生点下标。
排序改用严格字典序；点在多边形内改用确定性半开射线计数；圆面积 acos 输入夹取到 [-1,1]。

## 归档

- `archive/geometry_section_20260920_before_slim.tex`：改造前完整章节，含 C++14。
- `archive/espanso_geometry_20260920_before_slim.yml.txt`：原 Espanso 几何文件原样快照，不会被 daemon 加载。

## 验收记录

- 2026-09-20 第一版精简：1720 用例、23 个依赖组合、LOCAL 双浮点类型通过；正式 PDF 184 → 178 页。
- 2026-09-20 第二版精简（去混合类型运算符模板、合并卡壳、圆类瘦身）：同一套回归全部通过，
  `g++ -Wall -Wextra` 无警告。本次未在提交环境重编 PDF，`out/template-main.pdf` 需要在有 xelatex 的机器上重编。
