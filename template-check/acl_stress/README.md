# ACL 移植版对拍

验证 `atcoder-template/` 里 ACL `segtree` / `lazy_segtree` / `z_algorithm` 的**自包含移植版**与官方
[ac-library](https://github.com/atcoder/ac-library) v1.6 行为一致。

```bash
bash template-check/acl_stress/run.sh    # 抽取 + 编译 + 跑，全过打印 ALL PASS
```

## 怎么工作

1. `extract.py` 从 `atcoder-template/acl_segtree.tex` / `acl_lazysegtree.tex` / `acl_z_algorithm.tex`
   里抽出**模板本体**和九个带 `ACL_EXAMPLE` marker 的示例代码块，并从图论章抽出 `HLDACL`，写成忽略提交的 `.inc` 文件。
   定位靠 marker（`struct segtree {` / `struct lazy_segtree {` / `int &k = z[i];`）而**不是**「第一个 cpp 块」——
   示例用法排在本体之前，取第一个会抽到示例；命中数 ≠ 1 直接报错，不静默抽错。
   **测的是 .tex 里的真实内容**，改了模板不同步就会立刻暴露。
2. `stress.cpp` 把移植版、官方 `atcoder::segtree` / `atcoder::lazy_segtree`、朴素暴力三方对拍：
   - `segtree`：区间最大值，随机 `set` / `get` / `prod` / `all_prod` / `max_right` / `min_left`
   - `lazy_segtree` 区间加 + 区间和：`apply(p,f)` / `apply(l,r,f)` / `prod` / `get` / `set` / `max_right`
   - `lazy_segtree` 区间赋值 + 区间最大（`NONE` 哨兵那套 `mapping` / `composition`）
   - 空树 / 默认构造 / `prod(0,0)` 边界
   - 打印稿中的四个 `segtree` 示例：单点最大值、最大子段和、第 k 个 1 / 左右边界、含负数的前缀二分
   - 打印稿中的五个 `lazy_segtree` 示例：区间加求和、仿射修改维护一至三次幂和、赋值/加维护统计、最小代价权值和、权值线段树众数
   - `HLDACL`：随机树上的路径 / 子树异或修改、路径 / 子树查询和子树边接口
   - `z_algorithm`：小字母表随机串（`string` / `vector<int>` 两个重载）对官方 + 暴力，
     20 万长度的全同串 / 周期串对官方，`vector<ll>` / `vector<pair>` 泛型重载对暴力，空串边界
3. 编译带 ASan + UBSan（`-fno-sanitize=vptr,function`，libstdc++ 下必须关）。

## 注意

- 官方 ac-library **只作参考答案**；模板里的移植版不依赖任何 `atcoder/` 头文件，赛场可直接手抄。
- `max_right` 要求谓词单调。区间加允许负数时 `sum <= x` 不单调，此时只与官方 ACL 对齐、跳过暴力比较
  （测试里靠 `nonneg` 标志区分，一半 iter 用非负数据走全量三方对拍）。
- 路径可用环境变量覆盖：`ACL_DIR` / `CXX` / `PY` / `SDK` / `GCC_INC` / `GCC_LIB`。
