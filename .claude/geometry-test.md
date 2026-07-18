# 几何模板的对拍测试在哪

本项目（ICPC-CCPC 模板）的计算几何模板 **真身在 espanso 片段**里
（`~/Library/Application Support/espanso/match/computational_geometry.yml`，
trigger `:Point` / `:makeConvexHull` / `:isConvex` / ... 一组），
`template-main.tex` 里的几何节是它的镜像副本。

## geometry test 位置（别再到处找了）

```
~/Library/Application Support/espanso/geometry_tests/gen_and_test.py
```

- 直接从 `computational_geometry.yml` 抽 `:Point` / `:polygonArea` /
  `:reorderPolygon` / `:makeConvexHull` 片段，拼成临时 C++ 驱动编译，
  对 `make_convex_hull` / `reorder_polygon` 做**性质化对拍**
  （凸性 / CCW / 起点 = (y 最小, 再 x 最小) / 集合相等 / 去重 等规格性质）。
- 参考答案用独立的 Python 单调链 + 整数精确谓词，**不复刻** C++ 单调栈写法，
  避免与实际展开内容漂移。
- 用法：`~/miniconda3/bin/python "~/Library/Application Support/espanso/geometry_tests/gen_and_test.py" [次数]`（默认 2000）

## 改模板的三处同步点

几何模板有改动（如某题 AC 后沉淀）时，要同步三处：

1. `computational_geometry.yml` 对应片段的 `replace` body（真身）
2. 同 yml 中 `:makeConvexHull` 上方的 `# changelog` 注释块（记日期 + 沉淀来源 + AC 链接）
3. `template-main.tex` 里的几何节镜像

改完跑上面的 `gen_and_test.py` 验证。
