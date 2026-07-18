// RNG 质量对比：SWB+splitmix64 vs mt19937_64+splitmix64 vs 裸 mt19937_64
// 三项测试：速度（ns/call）、基础统计（bit balance / byte 卡方 / lag-1 自相关）、F2 上的 Berlekamp–Massey 攻击。
// 输出：人类可读的 markdown 直接打到 stdout。编译：g++-15 -std=c++20 -O2 -o rb benchmark.cpp

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using u64 = uint64_t;

// ============================================================================
// 三种待测 RNG
// ============================================================================

using SWB = std::subtract_with_carry_engine<uint64_t, 64, 5, 12>;

static inline u64 splitmix64(u64 x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

constexpr u64 SEED = 12345;

enum Variant { V_SWB_SM = 0, V_MT_SM = 1, V_MT_RAW = 2, V_COUNT = 3 };
const char* VNAME[V_COUNT] = {
    "SWB + splitmix64",
    "mt19937_64 + splitmix64",
    "mt19937_64（裸）"
};

vector<u64> generate(int v, u64 seed, size_t N) {
    vector<u64> out(N);
    if (v == V_SWB_SM) {
        SWB rng(seed);
        for (size_t i = 0; i < N; i++) out[i] = splitmix64(rng());
    } else if (v == V_MT_SM) {
        mt19937_64 rng(seed);
        for (size_t i = 0; i < N; i++) out[i] = splitmix64(rng());
    } else {
        mt19937_64 rng(seed);
        for (size_t i = 0; i < N; i++) out[i] = rng();
    }
    return out;
}

// ============================================================================
// 测试 1：速度（ns/call），每路 5 轮取均值 + 95% CI
// ============================================================================

template<class Gen>
double bench_run(Gen&& gen, size_t N) {
    volatile u64 sink = 0;
    auto t0 = chrono::steady_clock::now();
    for (size_t i = 0; i < N; i++) sink ^= gen();
    auto t1 = chrono::steady_clock::now();
    return chrono::duration<double, nano>(t1 - t0).count() / double(N);
}

struct Stat { double mean, ci95; };

Stat bench_variant(int v, size_t N, int trials) {
    vector<double> samples;
    for (int t = 0; t < trials; t++) {
        double r;
        if (v == V_SWB_SM) {
            SWB rng(SEED + t);
            r = bench_run([&]{ return splitmix64(rng()); }, N);
        } else if (v == V_MT_SM) {
            mt19937_64 rng(SEED + t);
            r = bench_run([&]{ return splitmix64(rng()); }, N);
        } else {
            mt19937_64 rng(SEED + t);
            r = bench_run([&]{ return rng(); }, N);
        }
        samples.push_back(r);
    }
    double m = 0; for (double s : samples) m += s; m /= samples.size();
    double var = 0; for (double s : samples) var += (s - m) * (s - m);
    var /= max<size_t>(1, samples.size() - 1);
    double sd = sqrt(var);
    // trials=5 时 t_{0.975, df=4} = 2.776
    double tval = 2.776;
    return {m, tval * sd / sqrt((double)samples.size())};
}

void print_speed_table(size_t N, int trials) {
    cout << "## 测试 1：速度（ns/call）\n\n";
    cout << "每轮 N = " << N << " 次调用，共 " << trials << " 轮，"
         << "95% CI 用 Student's t 估（df=" << trials - 1 << "）。\n\n";
    cout << "| RNG | ns/call | 95% CI | 相对裸 mt19937_64 |\n";
    cout << "|-----|--------:|-------:|------------------:|\n";
    Stat s[V_COUNT];
    for (int v = 0; v < V_COUNT; v++) s[v] = bench_variant(v, N, trials);
    double base = s[V_MT_RAW].mean;
    for (int v = 0; v < V_COUNT; v++) {
        cout << "| " << VNAME[v]
             << " | " << fixed << setprecision(2) << s[v].mean
             << " | ±" << s[v].ci95
             << " | " << setprecision(3) << s[v].mean / base << "x |\n";
    }
    cout << "\n";
}

// ============================================================================
// 测试 2：基础统计 —— bit balance、byte 卡方、lag-1 自相关
// ============================================================================

struct StatsResult {
    int max_bit_imbalance;
    double max_byte_chi2;
    double byte_chi2_threshold;
    long long lag1_pop_diff;
    double lag1_z;
    bool pass_bit, pass_byte, pass_lag1;
};

StatsResult basic_stats(const vector<u64>& v) {
    size_t N = v.size();
    StatsResult r{};

    // bit balance：64 个 bit 位置各自统计 1 的次数
    array<long long, 64> bitcnt{};
    for (u64 x : v) for (int b = 0; b < 64; b++) if ((x >> b) & 1) bitcnt[b]++;
    long long expected = (long long)N / 2;
    long long max_dev = 0;
    for (int b = 0; b < 64; b++) max_dev = max(max_dev, llabs(bitcnt[b] - expected));
    r.max_bit_imbalance = (int)max_dev;
    // 阈值：4√N（约 4σ）
    double bit_threshold = 4.0 * sqrt((double)N);
    r.pass_bit = (max_dev < bit_threshold);

    // byte 卡方：拆 8 个字节流，每流 256 桶
    double max_chi2 = 0;
    for (int byte = 0; byte < 8; byte++) {
        array<long long, 256> bin{};
        for (u64 x : v) bin[(x >> (8 * byte)) & 0xff]++;
        double exp = (double)N / 256.0;
        double chi2 = 0;
        for (int i = 0; i < 256; i++) {
            double d = bin[i] - exp;
            chi2 += d * d / exp;
        }
        max_chi2 = max(max_chi2, chi2);
    }
    r.max_byte_chi2 = max_chi2;
    // df=255、p<0.001 时 χ² 上分位约 330.5
    r.byte_chi2_threshold = 330.5;
    r.pass_byte = (max_chi2 < r.byte_chi2_threshold);

    // lag-1 自相关：相邻输出 XOR 的 popcount，期望 32(N-1)
    long long pop = 0;
    for (size_t i = 1; i < N; i++) pop += __builtin_popcountll(v[i] ^ v[i - 1]);
    long long exp_pop = 32LL * (long long)(N - 1);
    r.lag1_pop_diff = pop - exp_pop;
    // sd = sqrt(64 * (N-1) * 0.25) = 4√(N-1)
    double lag1_sd = 4.0 * sqrt((double)(N - 1));
    r.lag1_z = double(pop - exp_pop) / lag1_sd;
    r.pass_lag1 = (fabs(r.lag1_z) < 4.0);

    return r;
}

void print_stats_table(size_t N) {
    cout << "## 测试 2：基础统计\n\n";
    cout << "每路 N = " << N << " 个输出。\n";
    cout << "PASS 阈值：bit 偏差最大值 < 4√N ≈ " << fixed << setprecision(0)
         << 4.0 * sqrt((double)N) << "；byte 卡方最大值 < 330.5（df=255, p<0.001）；|lag-1 z| < 4。\n\n";
    cout << "| RNG | bit 最大偏差 | byte 卡方（8 路最大） | lag-1 z | 判定 |\n";
    cout << "|-----|------------:|--------------------:|--------:|-----|\n";
    for (int v = 0; v < V_COUNT; v++) {
        auto out = generate(v, SEED, N);
        auto r = basic_stats(out);
        bool pass = r.pass_bit && r.pass_byte && r.pass_lag1;
        cout << "| " << VNAME[v]
             << " | " << r.max_bit_imbalance
             << " | " << fixed << setprecision(2) << r.max_byte_chi2
             << " | " << setprecision(3) << r.lag1_z
             << " | " << (pass ? "PASS" : "FAIL") << " |\n";
    }
    cout << "\n";
}

// ============================================================================
// 测试 3：F2 上的 Berlekamp–Massey 攻击
// ============================================================================

// bitset 多项式 / 序列辅助。
// 约定：word[k] 第 b 位 = x^(64k + b) 系数（或序列下标 64k + b 处的位）。

static inline int bit_get(const vector<u64>& v, int i) { return (v[i >> 6] >> (i & 63)) & 1; }

// dst ^= src << shift（F2 多项式，固定字数）
static void shift_xor(vector<u64>& dst, const vector<u64>& src, int shift) {
    int W = (int)dst.size();
    int ws = shift >> 6;
    int bs = shift & 63;
    if (ws >= W) return;
    if (bs == 0) {
        for (int i = W - 1; i >= ws; i--) dst[i] ^= src[i - ws];
    } else {
        for (int i = W - 1; i > ws; i--) {
            dst[i] ^= (src[i - ws] << bs) | (src[i - ws - 1] >> (64 - bs));
        }
        dst[ws] ^= src[0] << bs;
    }
}

// v <<= 1（整个 bitvector 上移一位）
static void shl1(vector<u64>& v) {
    int W = (int)v.size();
    for (int i = W - 1; i > 0; i--) v[i] = (v[i] << 1) | (v[i - 1] >> 63);
    v[0] <<= 1;
}

// W 个字范围内 parity(a & b)
static inline int parity_and(const vector<u64>& a, const vector<u64>& b) {
    u64 acc = 0;
    int W = (int)a.size();
    for (int i = 0; i < W; i++) acc ^= a[i] & b[i];
    return __builtin_parityll(acc);
}

struct BMResult {
    vector<u64> C;   // 最小多项式，bit i = c_i，c_0 = 1
    int L;           // LFSR 长度（递推阶数）
    double seconds;
};

// F2 上的 Berlekamp–Massey，输入 bitpacked s[0..N)。
// 标准写法：维护 C（当前多项式）、B（上次失配的多项式）、L（LFSR 长度）、m（上次更新步号）。
// 失配 d=1 时 C ^= B << (n-m)；若 2L<=n 还要交换 B/C 并更新 L,m。
BMResult bm_f2(const vector<u64>& s, int N) {
    auto t0 = chrono::steady_clock::now();
    int W = (N + 63) / 64 + 2;
    vector<u64> C(W, 0), B(W, 0), bs(W, 0);
    C[0] = 1;
    B[0] = 1;
    int L = 0, m = -1;

    for (int n = 0; n < N; n++) {
        // 维护滑窗：bs 第 0..L_max 位 = s[n], s[n-1], ..., s[n-L_max]
        shl1(bs);
        if (bit_get(s, n)) bs[0] |= 1ULL;

        // 失配量 d = XOR_{i=0..L} C[i] AND s[n-i] = parity(C & bs)
        int d = parity_and(C, bs);
        if (d == 0) continue;

        int shift = n - m;
        if (2 * L <= n) {
            vector<u64> T = C;
            shift_xor(C, B, shift);
            L = n + 1 - L;
            m = n;
            B = std::move(T);
        } else {
            shift_xor(C, B, shift);
        }
    }
    auto t1 = chrono::steady_clock::now();
    return {std::move(C), L, chrono::duration<double>(t1 - t0).count()};
}

// 给定复盘出的 C 和 L：用 s[n] = XOR_{i=1..L} C[i] s[n-i] 预测 s[N_train..N_total) 与真值比对，返回命中率
double verify_lfsr(const vector<u64>& C, int /*L*/,
                   const vector<u64>& s_full, int N_train, int N_total) {
    int W = (int)C.size();
    vector<u64> bs(W, 0);
    long long matches = 0;
    long long checked = 0;

    for (int n = 0; n < N_total; n++) {
        shl1(bs);
        if (bit_get(s_full, n)) bs[0] |= 1ULL;

        if (n >= N_train) {
            // 预测 = parity( (C 清掉第 0 位) AND bs )
            //（C 第 0 位对应 s[n] 自身，是要预测的目标，不能进 XOR 求和）
            u64 acc = 0;
            for (int i = 0; i < W; i++) {
                u64 ci = C[i];
                if (i == 0) ci &= ~1ULL;
                acc ^= ci & bs[i];
            }
            int pred = __builtin_parityll(acc);
            int actual = bit_get(s_full, n);
            if (pred == actual) matches++;
            checked++;
        }
    }
    return checked ? double(matches) / double(checked) : 0.0;
}

vector<u64> extract_lane(const vector<u64>& outs, int lane) {
    int N = (int)outs.size();
    vector<u64> bits((N + 63) / 64 + 1, 0);
    for (int i = 0; i < N; i++) {
        if ((outs[i] >> lane) & 1) bits[i >> 6] |= (1ULL << (i & 63));
    }
    return bits;
}

void print_bm_table(int N_total) {
    cout << "## 测试 3：F₂ 上的 Berlekamp–Massey 攻击\n\n";
    cout << "每路 RNG × 每条 bit lane：取 N_total=" << N_total
         << " 个输出，抽一条 bit lane，前 " << N_total / 2
         << " bit 喂给 BM 训练，后 " << N_total - N_total / 2 << " bit 验证。\n";
    cout << "**复盘度数 d** = BM 找到的最小多项式度数。\n";
    cout << "**命中率** 在验证半段：~50% 表示 LFSR 只是在训练段过拟合噪声（RNG 抗住）；100% 表示 LFSR 完美预测（RNG 被破）。\n";
    cout << "mt19937_64 在 F₂ 上的线性复杂度 = 19937；安全 RNG 上 d 应饱和到 N_train/2 = "
         << N_total / 4 << "。\n\n";
    int lanes[] = {0, 7, 31, 32, 63};
    cout << "| RNG | lane | 复盘 d | 命中率 | BM 耗时（秒） | 判定 |\n";
    cout << "|-----|-----:|------:|------:|-------------:|-----|\n";
    int N_train = N_total / 2;
    for (int v = 0; v < V_COUNT; v++) {
        auto outs = generate(v, SEED, N_total);
        for (int lane : lanes) {
            auto bits = extract_lane(outs, lane);
            // 健全性：BM 之前粗看 bit balance
            long long pop = 0;
            for (int i = 0; i < N_total; i++) pop += bit_get(bits, i);
            (void)pop;
            BMResult r = bm_f2(bits, N_train);
            double mr = verify_lfsr(r.C, r.L, bits, N_train, N_total);
            const char* verdict;
            if (mr > 0.95 && r.L <= 25000) verdict = "已破";
            else if (mr < 0.55 && r.L > N_train / 4) verdict = "抗住";
            else verdict = "?";
            cout << "| " << VNAME[v]
                 << " | " << lane
                 << " | " << r.L
                 << " | " << fixed << setprecision(4) << mr
                 << " | " << setprecision(3) << r.seconds
                 << " | " << verdict << " |\n";
        }
    }
    cout << "\n";
}

// ============================================================================
// main
// ============================================================================

int main() {
    cout.setf(ios::unitbuf);
    cout << "# RNG 对比：SWB+splitmix64 vs mt19937_64+splitmix64 vs 裸 mt19937_64\n\n";
    cout << "Seed = " << SEED << "。三路共享同一 seed。\n\n";

    print_speed_table(/*N=*/100'000'000, /*trials=*/5);
    print_stats_table(/*N=*/10'000'000);
    print_bm_table(/*N_total=*/100'000);

    return 0;
}
