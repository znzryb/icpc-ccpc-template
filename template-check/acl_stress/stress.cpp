// 对拍：.tex 里的自包含移植版 vs 官方 ACL vs 暴力
#include <bits/stdc++.h>
#include <atcoder/lazysegtree>
#include <atcoder/segtree>
#include <atcoder/string>

using namespace std;
using ll = long long;

#include "lazyseg.inc"
#include "seg.inc"
#include "z.inc"

namespace ex_seg_point_max {
#include "example_seg_point_max.inc"
}
namespace ex_seg_max_subarray {
#include "example_seg_max_subarray.inc"
}
namespace ex_seg_kth_boundary {
#include "example_seg_kth_boundary.inc"
}
namespace ex_seg_negative_prefix {
#include "example_seg_negative_prefix.inc"
}
namespace ex_lazy_range_add_sum {
#include "example_lazy_range_add_sum.inc"
}
namespace ex_lazy_affine_moments {
#include "example_lazy_affine_moments.inc"
}
namespace ex_lazy_assign_add_stats {
#include "example_lazy_assign_add_stats.inc"
}
namespace ex_lazy_min_weight {
#include "example_lazy_min_weight.inc"
}
namespace ex_lazy_value_mode {
#include "example_lazy_value_mode.inc"
}

#define FOR(i, l, r) for (int i = (l); i <= (r); ++i)
namespace ex_hld {
#include "hld.inc"
}
#undef FOR

// ================= segtree: 区间最大值 =================
using S1 = ll;
S1 op1(S1 a, S1 b) { return max(a, b); }
S1 e1() { return -4e18; }

// ================= lazy: 区间加 + 区间和 =================
struct S2 {
    ll sum;
    int len;
};
using F2 = ll;
S2 op2(S2 a, S2 b) { return {a.sum + b.sum, a.len + b.len}; }
S2 e2() { return {0, 0}; }
S2 mapping2(F2 f, S2 x) { return {x.sum + f * x.len, x.len}; }
F2 composition2(F2 f, F2 g) { return f + g; }
F2 id2() { return 0; }

// ================= lazy: 区间赋值 + 区间最大 =================
using S3 = ll;
using F3 = ll;
const F3 NONE = LLONG_MIN;
S3 op3(S3 a, S3 b) { return max(a, b); }
S3 e3() { return -4e18; }
S3 mapping3(F3 f, S3 x) { return f == NONE ? x : f; }
F3 composition3(F3 f, F3 g) { return f == NONE ? g : f; }
F3 id3() { return NONE; }

// ================= z_algorithm: 朴素暴力参考 =================
vector<int> z_brute(const vector<int> &s) {
    int n = s.size();
    if (n == 0) return {};
    vector<int> z(n);
    z[0] = n;
    for (int i = 1; i < n; i++) {
        int k = 0;
        while (i + k < n && s[k] == s[i + k]) k++;
        z[i] = k;
    }
    return z;
}

int fails = 0;
#define CHECK(cond, what)                                                    \
    do {                                                                     \
        if (!(cond)) {                                                       \
            printf("FAIL %s (iter %d, n %d)\n", what, iter, n);               \
            fails++;                                                          \
        }                                                                     \
    } while (0)

int main() {
    mt19937 rng(20260730);
    const int ITER = 300;

    // ---------- 测试 1: segtree 区间最大 ----------
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 40;
        vector<ll> a(n);
        for (auto &x : a) x = (ll) (rng() % 2000) - 1000;

        segtree<S1, op1, e1> mine(a);
        atcoder::segtree<S1, op1, e1> off(a);

        for (int q = 0; q < 80; q++) {
            int t = rng() % 5;
            if (t == 0) {  // set
                int p = rng() % n;
                ll v = (ll) (rng() % 2000) - 1000;
                a[p] = v;
                mine.set(p, v);
                off.set(p, v);
            } else if (t == 1) {  // get
                int p = rng() % n;
                CHECK(mine.get(p) == a[p], "seg.get vs brute");
                CHECK(mine.get(p) == off.get(p), "seg.get vs acl");
            } else if (t == 2) {  // prod
                int l = rng() % (n + 1), r = rng() % (n + 1);
                if (l > r) swap(l, r);
                ll want = e1();
                for (int i = l; i < r; i++) want = max(want, a[i]);
                CHECK(mine.prod(l, r - 1) == want, "seg.prod vs brute");
                CHECK(mine.prod(l, r - 1) == off.prod(l, r), "seg.prod vs acl");
            } else if (t == 3) {  // all_prod
                ll want = e1();
                for (int i = 0; i < n; i++) want = max(want, a[i]);
                CHECK(mine.all_prod() == want, "seg.all_prod vs brute");
                CHECK(mine.all_prod() == off.all_prod(), "seg.all_prod vs acl");
            } else {  // max_right / min_left, 谓词 max <= x
                ll x = (ll) (rng() % 2400) - 1200;
                auto f = [&](S1 v) { return v <= x; };
                if (f(e1())) {
                    int l = rng() % (n + 1);
                    int got = mine.max_right(l, f);
                    int want = l;
                    ll cur = e1();
                    while (want < n && f(max(cur, a[want]))) {
                        cur = max(cur, a[want]);
                        want++;
                    }
                    CHECK(got == want - 1, "seg.max_right vs brute");
                    CHECK(got == off.max_right(l, f) - 1, "seg.max_right vs acl");

                    int r = rng() % (n + 1);
                    int got2 = mine.min_left(r - 1, f);
                    int want2 = r;
                    cur = e1();
                    while (want2 > 0 && f(max(cur, a[want2 - 1]))) {
                        cur = max(cur, a[want2 - 1]);
                        want2--;
                    }
                    CHECK(got2 == want2, "seg.min_left vs brute");
                    CHECK(got2 == off.min_left(r, f), "seg.min_left vs acl");
                }
            }
        }
    }

    // ---------- 测试 2: lazy_segtree 区间加 + 区间和 ----------
    using MineAdd =
        lazy_segtree<S2, op2, e2, F2, mapping2, composition2, id2>;
    using OffAdd =
        atcoder::lazy_segtree<S2, op2, e2, F2, mapping2, composition2, id2>;
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 40;
        // max_right 要求谓词单调: sum <= x 只有在元素非负时才单调,
        // 所以一半的 iter 用非负数据(能跟 brute 对), 一半允许负数(只跟 acl 对)
        bool nonneg = iter % 2 == 0;
        auto randv = [&]() -> ll {
            return nonneg ? (ll) (rng() % 100) : (ll) (rng() % 200) - 100;
        };
        vector<ll> a(n);
        vector<S2> init(n);
        for (int i = 0; i < n; i++) {
            a[i] = randv();
            init[i] = {a[i], 1};
        }
        MineAdd mine(init);
        OffAdd off(init);

        for (int q = 0; q < 80; q++) {
            int t = rng() % 5;
            int l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r) swap(l, r);
            if (t == 0) {  // 区间加
                ll v = randv();
                for (int i = l; i < r; i++) a[i] += v;
                mine.apply(l, r - 1, v);
                off.apply(l, r, v);
            } else if (t == 1) {  // 单点加
                int p = rng() % n;
                ll v = randv();
                a[p] += v;
                mine.apply(p, v);
                off.apply(p, v);
            } else if (t == 2) {  // 区间和
                ll want = 0;
                for (int i = l; i < r; i++) want += a[i];
                CHECK(mine.prod(l, r - 1).sum == want, "lazy.prod vs brute");
                CHECK(mine.prod(l, r - 1).sum == off.prod(l, r).sum,
                      "lazy.prod vs acl");
                CHECK(mine.prod(l, r - 1).len == r - l, "lazy.prod len");
            } else if (t == 3) {  // 单点 get / set
                int p = rng() % n;
                CHECK(mine.get(p).sum == a[p], "lazy.get vs brute");
                CHECK(mine.get(p).sum == off.get(p).sum, "lazy.get vs acl");
                ll v = randv();
                a[p] = v;
                mine.set(p, {v, 1});
                off.set(p, {v, 1});
            } else {  // 树上二分: 最长前缀和 <= x
                ll x = (ll) (rng() % 400) - 100;
                auto g = [&](S2 s) { return s.sum <= x; };
                if (g(e2())) {
                    int got = mine.max_right(l, g);
                    if (nonneg) {  // 谓词单调才有唯一答案
                        int want = l;
                        ll cur = 0;
                        while (want < n && cur + a[want] <= x) cur += a[want++];
                        CHECK(got == want - 1, "lazy.max_right vs brute");
                    }
                    CHECK(got == off.max_right(l, g) - 1, "lazy.max_right vs acl");
                    CHECK(mine.all_prod().sum == off.all_prod().sum,
                          "lazy.all_prod vs acl");
                }
            }
        }
    }

    // ---------- 测试 3: lazy_segtree 区间赋值 + 区间最大 ----------
    using MineSet =
        lazy_segtree<S3, op3, e3, F3, mapping3, composition3, id3>;
    using OffSet =
        atcoder::lazy_segtree<S3, op3, e3, F3, mapping3, composition3, id3>;
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 40;
        vector<ll> a(n);
        for (auto &x : a) x = (ll) (rng() % 200) - 100;
        vector<S3> init(a.begin(), a.end());
        MineSet mine(init);
        OffSet off(init);

        for (int q = 0; q < 80; q++) {
            int t = rng() % 3;
            int l = rng() % (n + 1), r = rng() % (n + 1);
            if (l > r) swap(l, r);
            if (t == 0) {  // 区间赋值
                ll v = (ll) (rng() % 200) - 100;
                for (int i = l; i < r; i++) a[i] = v;
                mine.apply(l, r - 1, v);
                off.apply(l, r, v);
            } else if (t == 1) {  // 区间最大
                ll want = e3();
                for (int i = l; i < r; i++) want = max(want, a[i]);
                CHECK(mine.prod(l, r - 1) == want, "lazyset.prod vs brute");
                CHECK(mine.prod(l, r - 1) == off.prod(l, r), "lazyset.prod vs acl");
            } else {  // 单点
                int p = rng() % n;
                CHECK(mine.get(p) == a[p], "lazyset.get vs brute");
                CHECK(mine.get(p) == off.get(p), "lazyset.get vs acl");
            }
        }
    }

    // ---------- 边界: 空树 / 默认构造 ----------
    {
        int iter = -1, n = 0;
        segtree<S1, op1, e1> s0;
        CHECK(s0.all_prod() == e1(), "empty segtree all_prod");
        segtree<S1, op1, e1> s1(0);
        CHECK(s1.prod(0, -1) == e1(), "segtree prod(0,0)");
        MineAdd z0;
        CHECK(z0.all_prod().sum == 0, "empty lazy all_prod");
        MineAdd z1(5);
        z1.apply(0, 5, 3);
        CHECK(z1.prod(0, 5).sum == 0, "lazy default e() then apply");
    }

    // ---------- 打印稿示例: segtree 四种配置 ----------
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 35;

        vector<ll> raw(n + 1);
        vector<ll> mx_init(n + 1, ex_seg_point_max::e_max());
        vector<ex_seg_max_subarray::S> sub_init(
            n + 1, ex_seg_max_subarray::e());
        vector<ex_seg_negative_prefix::S> pre_init(
            n + 1, ex_seg_negative_prefix::e());
        for (int i = 1; i <= n; i++) {
            raw[i] = (ll)(rng() % 101) - 50;
            mx_init[i] = raw[i];
            sub_init[i] = ex_seg_max_subarray::make_leaf(raw[i]);
            pre_init[i] = ex_seg_negative_prefix::make_leaf(raw[i]);
        }
        ex_seg_point_max::SegMax seg_max(mx_init);
        ex_seg_max_subarray::SegMaxSubarray seg_sub(sub_init);
        ex_seg_negative_prefix::SegPrefix seg_pre(pre_init);

        vector<ll> bits(n + 1);
        for (int i = 1; i <= n; i++) bits[i] = rng() & 1;
        ex_seg_kth_boundary::SegCount seg_bits(bits);

        for (int q = 0; q < 100; q++) {
            int p = 1 + rng() % n;
            ll x = (ll)(rng() % 101) - 50;
            raw[p] = x;
            seg_max.set(p, x);
            seg_sub.set(p, ex_seg_max_subarray::make_leaf(x));
            seg_pre.set(p, ex_seg_negative_prefix::make_leaf(x));

            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);
            CHECK(seg_max.prod(l, r) ==
                      *max_element(raw.begin() + l, raw.begin() + r + 1),
                  "example point max");

            ll want_best = LLONG_MIN;
            for (int i = l; i <= r; i++) {
                ll sum = 0;
                for (int j = i; j <= r; j++) {
                    sum += raw[j];
                    want_best = max(want_best, sum);
                }
            }
            CHECK(seg_sub.prod(l, r).best == want_best,
                  "example max subarray");

            ll target = (ll)(rng() % 101) - 50;
            int want_prefix = -1;
            ll sum = 0;
            for (int i = l; i <= n; i++) {
                sum += raw[i];
                if (sum >= target) {
                    want_prefix = i;
                    break;
                }
            }
            CHECK(ex_seg_negative_prefix::first_prefix_at_least(
                      seg_pre, l, n, target) == want_prefix,
                  "example negative prefix");

            bits[p] ^= 1;
            seg_bits.set(p, bits[p]);
            ll k = rng() % (n + 2);
            int want_k = -1;
            ll cnt = 0;
            if (k > 0)
                for (int i = 1; i <= n; i++)
                    if ((cnt += bits[i]) >= k) {
                        want_k = i;
                        break;
                    }
            CHECK(ex_seg_kth_boundary::kth_one(seg_bits, k) == want_k,
                  "example kth one");

            ll limit = rng() % (n + 1);
            int want_r = l - 1;
            sum = 0;
            while (want_r + 1 <= n &&
                   sum + bits[want_r + 1] <= limit)
                sum += bits[++want_r];
            CHECK(ex_seg_kth_boundary::farthest_right(
                      seg_bits, l, limit) == want_r,
                  "example max_right");

            int want_l = r + 1;
            sum = 0;
            while (want_l - 1 >= 1 &&
                   sum + bits[want_l - 1] <= limit)
                sum += bits[--want_l];
            CHECK(ex_seg_kth_boundary::farthest_left(
                      seg_bits, r, limit) == want_l,
                  "example min_left");
        }
    }

    // ---------- 打印稿示例: lazy_segtree 五种配置 ----------
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 30;

        vector<ll> sum_raw(n + 1);
        vector<ex_lazy_range_add_sum::S> sum_init(
            n + 1, ex_lazy_range_add_sum::e());
        for (int i = 1; i <= n; i++) {
            sum_raw[i] = (ll)(rng() % 101) - 50;
            sum_init[i] = {sum_raw[i], 1};
        }
        ex_lazy_range_add_sum::SegAddSum seg_sum(sum_init);

        const ll mod = ex_lazy_affine_moments::MOD;
        vector<ll> moment_raw(n + 1);
        vector<ex_lazy_affine_moments::S> moment_init(
            n + 1, ex_lazy_affine_moments::e());
        for (int i = 1; i <= n; i++) {
            ll x = moment_raw[i] = rng() % 100;
            moment_init[i] = {x, x * x % mod,
                              x * x % mod * x % mod, 1};
        }
        ex_lazy_affine_moments::SegMoments seg_moment(moment_init);

        vector<ll> stats_raw(n + 1);
        vector<ex_lazy_assign_add_stats::S> stats_init(
            n + 1, ex_lazy_assign_add_stats::e());
        for (int i = 1; i <= n; i++) {
            ll x = stats_raw[i] = (ll)(rng() % 101) - 50;
            stats_init[i] = {x, x, x, i, i, 1};
        }
        ex_lazy_assign_add_stats::SegStats seg_stats(stats_init);

        vector<ll> costs(n + 1), weights(n + 1);
        vector<ex_lazy_min_weight::S> weight_init(
            n + 1, ex_lazy_min_weight::e());
        for (int i = 1; i <= n; i++) {
            costs[i] = (ll)(rng() % 101) - 50;
            weights[i] = rng() % 1000;
            weight_init[i] = {costs[i], weights[i], false};
        }
        ex_lazy_min_weight::SegMinWeight seg_weight(weight_init);

        vector<ll> counts(n + 1);
        vector<ex_lazy_value_mode::S> mode_init(
            n + 1, ex_lazy_value_mode::e());
        for (int i = 1; i <= n; i++)
            mode_init[i] = {0, 3LL * i + 1, false};
        ex_lazy_value_mode::SegMode seg_mode(mode_init);

        for (int q = 0; q < 100; q++) {
            int l = 1 + rng() % n, r = 1 + rng() % n;
            if (l > r) swap(l, r);

            ll delta = (ll)(rng() % 41) - 20;
            seg_sum.apply(l, r, delta);
            for (int i = l; i <= r; i++) sum_raw[i] += delta;
            ll want_sum = accumulate(
                sum_raw.begin() + l, sum_raw.begin() + r + 1, 0LL);
            auto sum_got = seg_sum.prod(l, r);
            CHECK(sum_got.sum == want_sum &&
                      sum_got.len == r - l + 1,
                  "example range add sum");

            int affine_type = rng() % 3;
            ll affine_x = rng() % 50;
            ex_lazy_affine_moments::F affine =
                affine_type == 0
                    ? ex_lazy_affine_moments::F{1, affine_x}
                : affine_type == 1
                    ? ex_lazy_affine_moments::F{affine_x, 0}
                    : ex_lazy_affine_moments::F{0, affine_x};
            seg_moment.apply(l, r, affine);
            for (int i = l; i <= r; i++)
                moment_raw[i] =
                    (affine.mul * moment_raw[i] + affine.add) % mod;
            ll s1 = 0, s2 = 0, s3 = 0;
            for (int i = l; i <= r; i++) {
                s1 = (s1 + moment_raw[i]) % mod;
                s2 = (s2 + moment_raw[i] * moment_raw[i]) % mod;
                s3 = (s3 + moment_raw[i] * moment_raw[i] % mod *
                           moment_raw[i]) % mod;
            }
            auto moment_got = seg_moment.prod(l, r);
            CHECK(moment_got.s1 == s1 && moment_got.s2 == s2 &&
                      moment_got.s3 == s3 &&
                      moment_got.len == r - l + 1,
                  "example affine moments");

            bool assign = rng() & 1;
            ll stats_x = (ll)(rng() % 101) - 50;
            ex_lazy_assign_add_stats::F stats_f{
                assign, stats_x, assign ? 0 : stats_x};
            if (!assign) stats_f.assign = 0;
            seg_stats.apply(l, r, stats_f);
            for (int i = l; i <= r; i++)
                stats_raw[i] = assign ? stats_x : stats_raw[i] + stats_x;
            ll stats_sum = 0, stats_mn = LLONG_MAX, stats_mx = LLONG_MIN;
            int stats_pos = -1;
            for (int i = l; i <= r; i++) {
                stats_sum += stats_raw[i];
                stats_mn = min(stats_mn, stats_raw[i]);
                if (stats_raw[i] > stats_mx)
                    stats_mx = stats_raw[i], stats_pos = i;
            }
            auto stats_got = seg_stats.prod(l, r);
            CHECK(stats_got.sum == stats_sum &&
                      stats_got.mn == stats_mn &&
                      stats_got.mx == stats_mx &&
                      stats_got.mx_pos == stats_pos &&
                      stats_got.left == l &&
                      stats_got.len == r - l + 1,
                  "example assign add stats");

            seg_weight.apply(l, r, delta);
            for (int i = l; i <= r; i++) costs[i] += delta;
            ll min_cost = *min_element(
                costs.begin() + l, costs.begin() + r + 1);
            ll weight_sum = 0;
            for (int i = l; i <= r; i++)
                if (costs[i] == min_cost)
                    weight_sum =
                        (weight_sum + weights[i]) % ex_lazy_min_weight::MOD;
            auto weight_got = seg_weight.prod(l, r);
            CHECK(!weight_got.empty && weight_got.mn == min_cost &&
                      weight_got.weight == weight_sum,
                  "example min weight");

            ll count_delta = (ll)(rng() % 11) - 5;
            seg_mode.apply(l, r, count_delta);
            for (int i = l; i <= r; i++) counts[i] += count_delta;
            ll best_count = *max_element(counts.begin() + 1, counts.end());
            int best_pos = 1;
            while (counts[best_pos] != best_count) best_pos++;
            auto mode_got = seg_mode.all_prod();
            CHECK(mode_got.cnt == best_count &&
                      mode_got.value == 3LL * best_pos + 1,
                  "example value mode");
        }
    }

    // ---------- HLD + ACL: 路径、子树与边接口 ----------
    for (int iter = 0; iter < ITER; iter++) {
        int n = 1 + rng() % 30;
        ex_hld::HLDACL tree(n);
        for (int v = 2; v <= n; v++)
            tree.addEdge(v, 1 + rng() % (v - 1));
        tree.build();
        vector<ll> value(n + 1);

        auto path_nodes = [&](int u, int v) {
            vector<int> left, right;
            while (tree.hld.dep[u] > tree.hld.dep[v]) {
                left.push_back(u);
                u = tree.hld.fa[u];
            }
            while (tree.hld.dep[v] > tree.hld.dep[u]) {
                right.push_back(v);
                v = tree.hld.fa[v];
            }
            while (u != v) {
                left.push_back(u);
                right.push_back(v);
                u = tree.hld.fa[u];
                v = tree.hld.fa[v];
            }
            left.push_back(u);
            reverse(right.begin(), right.end());
            left.insert(left.end(), right.begin(), right.end());
            return left;
        };

        auto check_info = [&](const ex_hld::Info &got,
                              const vector<int> &nodes) {
            ex_hld::Info want;
            for (int u : nodes) {
                int p = tree.hld.dep[u] & 1;
                want.cnt[p]++;
                for (int b = 0; b < ex_hld::LOG; b++)
                    want.bit_num[p][b] += (value[u] >> b) & 1;
            }
            for (int p = 0; p < 2; p++) {
                if (got.cnt[p] != want.cnt[p]) return false;
                for (int b = 0; b < ex_hld::LOG; b++)
                    if (got.bit_num[p][b] != want.bit_num[p][b])
                        return false;
            }
            return true;
        };

        for (int q = 0; q < 100; q++) {
            int u = 1 + rng() % n, v = 1 + rng() % n;
            ex_hld::Tag tag(rng() % (1 << 12), rng() % (1 << 12));
            int type = rng() % 5;
            if (type == 0) {
                auto nodes = path_nodes(u, v);
                tree.applyPathNode(u, v, tag);
                for (int x : nodes)
                    value[x] ^= tag.xor_val[tree.hld.dep[x] & 1];
            } else {
                vector<int> nodes;
                int lo = tree.hld.dfn[u];
                int hi = lo + tree.hld.siz[u] - 1;
                for (int x = 1; x <= n; x++)
                    if (lo <= tree.hld.dfn[x] &&
                        tree.hld.dfn[x] <= hi)
                        nodes.push_back(x);
                if (type == 1) {
                    tree.applySubtreeNode(u, tag);
                    for (int x : nodes)
                        value[x] ^= tag.xor_val[tree.hld.dep[x] & 1];
                } else if (type == 2) {
                    tree.applySubtreeEdge(u, tag);
                    for (int x : nodes)
                        if (x != u)
                            value[x] ^= tag.xor_val[tree.hld.dep[x] & 1];
                } else if (type == 3) {
                    CHECK(check_info(tree.querySubtreeNode(u), nodes),
                          "HLD ACL subtree query");
                } else {
                    auto path = path_nodes(u, v);
                    CHECK(check_info(tree.queryPathNode(u, v), path),
                          "HLD ACL path query");
                }
            }
        }
    }

    // ---------- z_algorithm: 移植版 vs 官方 vs 暴力 ----------
    {
        int iter = -1, n = 0;
        // 空串
        CHECK(z_algorithm(string("")).empty(), "z empty string");
        CHECK(z_algorithm(vector<int>{}).empty(), "z empty vector");

        // 小字母表随机串（逼出周期 / 重叠 border）
        for (iter = 0; iter < 20000; iter++) {
            n = rng() % 40;
            int sigma = 1 + rng() % 3;
            string s;
            vector<int> v;
            for (int i = 0; i < n; i++) {
                char c = 'a' + rng() % sigma;
                s += c;
                v.push_back(c);
            }
            auto bf = z_brute(v);
            CHECK(z_algorithm(s) == bf, "z(string) vs brute");
            CHECK(z_algorithm(v) == bf, "z(vector) vs brute");
            CHECK(z_algorithm(s) == atcoder::z_algorithm(s), "z(string) vs acl");
            CHECK(z_algorithm(v) == atcoder::z_algorithm(v), "z(vector) vs acl");
        }

        // 大串: 全同 / 每 7 位一个断点
        for (iter = 0; iter < 20; iter++) {
            n = 200000;
            string s(n, 'a');
            if (iter & 1)
                for (int i = 0; i < n; i += 7) s[i] = 'b';
            CHECK(z_algorithm(s) == atcoder::z_algorithm(s), "z big vs acl");
        }

        // 泛型: ll 序列 / pair 序列（官方无此重载，只对暴力）
        for (iter = 0; iter < 2000; iter++) {
            n = rng() % 30;
            vector<ll> a(n);
            vector<pair<int, int>> p(n);
            for (int i = 0; i < n; i++) {
                a[i] = rng() % 3;
                p[i] = {int(a[i]), int(a[i])};
            }
            vector<int> ai(a.begin(), a.end());
            CHECK(z_algorithm(a) == z_brute(ai), "z(vector<ll>) vs brute");
            CHECK(z_algorithm(p) == z_brute(ai), "z(vector<pair>) vs brute");
        }
    }

    printf(fails ? "TOTAL FAILS: %d\n" : "ALL PASS (fails=%d)\n", fails);
    return fails != 0;
}
