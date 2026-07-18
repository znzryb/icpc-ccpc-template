// gp_hash_table vs cc_hash_table vs std::unordered_map
// 测 insert / lookup-hit / lookup-miss / erase 四阶段
// 同时对比 resize 预分配 vs 不 resize
//
// 编译：g++-15 -O2 -std=c++20 bench.cpp -o bench
// 运行：./bench [N]    (默认 N = 2e6)

#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
using namespace std;
using namespace __gnu_pbds;
using ll = long long;

struct chash {
    const int RANDOM
        = (long long)(make_unique<char>().get())
        ^ chrono::high_resolution_clock::now()
              .time_since_epoch().count();
    static unsigned long long hash_f(unsigned long long x) {
        x += 0x9e3779b97f4a7c15;
        x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
        x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
        return x ^ (x >> 31);
    }
    size_t operator()(int x) const { return hash_f(x) ^ RANDOM; }
};

template<class K, class V, class H = chash>
using GPResizable = gp_hash_table<K, V, H,
    equal_to<K>,
    direct_mask_range_hashing<>,
    linear_probe_fn<>,
    hash_standard_resize_policy<
        hash_exponential_size_policy<>,
        hash_load_check_resize_trigger<true>,
        true>>;

template<class K, class V, class H = chash>
using CCResizable = cc_hash_table<K, V, H,
    equal_to<K>,
    direct_mask_range_hashing<>,
    hash_standard_resize_policy<
        hash_exponential_size_policy<>,
        hash_load_check_resize_trigger<true>,
        true>>;

static double now_ms() {
    using namespace chrono;
    return duration<double, milli>(
        steady_clock::now().time_since_epoch()).count();
}

struct Result {
    double t_insert, t_hit, t_miss, t_erase;
    ll checksum;
};

template<class Map>
Result run(Map& mp, const vector<int>& keys, const vector<int>& miss_keys) {
    Result r{};
    int N = keys.size();

    double t0 = now_ms();
    for (int i = 0; i < N; i++) mp[keys[i]] = i;
    r.t_insert = now_ms() - t0;

    ll sum = 0;
    t0 = now_ms();
    for (int i = 0; i < N; i++) {
        auto it = mp.find(keys[i]);
        if (it != mp.end()) sum += it->second;
    }
    r.t_hit = now_ms() - t0;
    r.checksum = sum;

    int hit = 0;
    t0 = now_ms();
    for (int i = 0; i < N; i++) {
        if (mp.find(miss_keys[i]) != mp.end()) hit++;
    }
    r.t_miss = now_ms() - t0;
    r.checksum ^= hit;

    t0 = now_ms();
    for (int i = 0; i < N; i++) mp.erase(keys[i]);
    r.t_erase = now_ms() - t0;

    return r;
}

void print_row(const string& name, const Result& r, double total_baseline = 0) {
    double total = r.t_insert + r.t_hit + r.t_miss + r.t_erase;
    printf("  %-32s  ins=%8.1f  hit=%8.1f  miss=%8.1f  ers=%8.1f  | tot=%8.1f ms",
        name.c_str(), r.t_insert, r.t_hit, r.t_miss, r.t_erase, total);
    if (total_baseline > 0)
        printf("  (%.2fx)", total_baseline / total);
    printf("  [chk=%lld]\n", r.checksum);
}

int main(int argc, char** argv) {
    int N = (argc >= 2) ? atoi(argv[1]) : 2'000'000;
    printf("N = %d\n", N);

    mt19937 rng(20260510);
    vector<int> keys(N), miss(N);
    {
        // 用稀疏空间避免桶冲突偶然性，hit/miss 不重叠
        unordered_set<int> seen;
        seen.reserve(N * 2);
        while ((int)seen.size() < N) {
            int x = (int)(rng() & 0x3FFFFFFF);
            seen.insert(x);
        }
        int i = 0;
        for (int x : seen) keys[i++] = x;
        i = 0;
        while (i < N) {
            int x = (int)(rng() & 0x3FFFFFFF) | 0x40000000; // 高位置 1，与 keys 不交
            if (!seen.count(x)) miss[i++] = x;
        }
    }

    // 预分配桶数：取 >= N 的下一个 2 的幂，再 ×2 给负载留余地
    size_t bucket_hint = 1;
    while (bucket_hint < (size_t)N * 2) bucket_hint <<= 1;
    printf("bucket_hint (resize) = %zu\n\n", bucket_hint);

    printf("=== std::unordered_map (baseline) ===\n");
    Result base;
    {
        unordered_map<int,int> mp;
        base = run(mp, keys, miss);
    }
    double baseline_total = base.t_insert + base.t_hit + base.t_miss + base.t_erase;
    print_row("unordered_map (no reserve)", base);
    {
        unordered_map<int,int> mp;
        mp.reserve(bucket_hint);
        Result r = run(mp, keys, miss);
        print_row("unordered_map (reserve)", r, baseline_total);
    }

    printf("\n=== gp_hash_table<int,int,chash> ===\n");
    {
        gp_hash_table<int,int,chash> mp;
        Result r = run(mp, keys, miss);
        print_row("gp default (no resize)", r, baseline_total);
    }
    {
        GPResizable<int,int> mp;
        mp.resize(bucket_hint);
        Result r = run(mp, keys, miss);
        print_row("gp + resize(bucket_hint)", r, baseline_total);
    }

    printf("\n=== cc_hash_table<int,int,chash> ===\n");
    {
        cc_hash_table<int,int,chash> mp;
        Result r = run(mp, keys, miss);
        print_row("cc default (no resize)", r, baseline_total);
    }
    {
        CCResizable<int,int> mp;
        mp.resize(bucket_hint);
        Result r = run(mp, keys, miss);
        print_row("cc + resize(bucket_hint)", r, baseline_total);
    }

    return 0;
}
