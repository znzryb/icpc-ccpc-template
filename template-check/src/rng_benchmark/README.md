# RNG benchmark

对比 ICPC 模板里 `subtract_with_carry_engine<uint64_t,64,5,12>` + `splitmix64` 与
两条对照（`mt19937_64` + `splitmix64` / 裸 `mt19937_64`）的随机数质量。

三个测试：

1. **速度**：每种生成器跑 1e8 次调用，5 轮取均值 + 95% CI
2. **基础统计**：1e7 次输出做 bit balance / byte χ² / lag-1 自相关
3. **Berlekamp–Massey 攻击**：1e5 次输出，每路抽 5 个 bit lane，前半训练 BM、后半验证
   预期裸 `mt19937_64` 复盘度数 = 19937 且预测准确率 100%；两个 splitmix64 包装版度数饱和、准确率 ~50%

## 编译 & 运行

```bash
g++-15 -std=c++20 -O2 -o /tmp/rng_benchmark benchmark.cpp
/tmp/rng_benchmark | tee results.md
```

或用 CMake target：

```bash
cmake --build cmake-build-debug --target rng_benchmark
./cmake-build-debug/rng_benchmark | tee src/rng_benchmark/results.md
```

stdout 直接是 markdown，重定向进 `results.md` 即得归档。

## 怎么读 results.md

- **速度表**：单纯比 ns/call，相对裸 mt19937_64 看倍率
- **统计表**：三路应当全 PASS。FAIL 通常说明 splitmix64 包装写错了
- **BM 表**：`BROKEN` = LFSR 复盘成功（裸 mt19937_64 应当是这个）；`safe` = 抗住攻击
