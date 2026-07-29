# ACL 线段树移植版对拍

验证 `atcoder-template/` 里 ACL `segtree` / `lazy_segtree` 的**自包含移植版**与官方
[ac-library](https://github.com/atcoder/ac-library) v1.6 行为一致。

```bash
bash template-check/acl_stress/run.sh    # 抽取 + 编译 + 跑，全过打印 ALL PASS
```

## 怎么工作

1. `extract.py` 从 `atcoder-template/acl_segtree.tex` / `acl_lazysegtree.tex` 里抽**第一个**
   `\begin{minted}{cpp}` 块（= 结构体定义），写成 `seg.inc` / `lazyseg.inc`。
   **测的是 .tex 里的真实内容**，改了模板不同步就会立刻暴露。
2. `stress.cpp` 把移植版、官方 `atcoder::segtree` / `atcoder::lazy_segtree`、朴素暴力三方对拍：
   - `segtree`：区间最大值，随机 `set` / `get` / `prod` / `all_prod` / `max_right` / `min_left`
   - `lazy_segtree` 区间加 + 区间和：`apply(p,f)` / `apply(l,r,f)` / `prod` / `get` / `set` / `max_right`
   - `lazy_segtree` 区间赋值 + 区间最大（`NONE` 哨兵那套 `mapping` / `composition`）
   - 空树 / 默认构造 / `prod(0,0)` 边界
3. 编译带 ASan + UBSan（`-fno-sanitize=vptr,function`，libstdc++ 下必须关）。

## 注意

- 官方 ac-library **只作参考答案**；模板里的移植版不依赖任何 `atcoder/` 头文件，赛场可直接手抄。
- `max_right` 要求谓词单调。区间加允许负数时 `sum <= x` 不单调，此时只与官方 ACL 对齐、跳过暴力比较
  （测试里靠 `nonneg` 标志区分，一半 iter 用非负数据走全量三方对拍）。
- 路径可用环境变量覆盖：`ACL_DIR` / `CXX` / `PY` / `SDK` / `GCC_INC` / `GCC_LIB`。
