// 对拍：.tex 里的自包含移植版 vs 官方 ACL vs 暴力
#include <bits/stdc++.h>
#include <atcoder/lazysegtree>
#include <atcoder/segtree>

using namespace std;
using ll = long long;

#include "lazyseg.inc"
#include "seg.inc"

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
                CHECK(mine.prod(l, r) == want, "seg.prod vs brute");
                CHECK(mine.prod(l, r) == off.prod(l, r), "seg.prod vs acl");
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
                    CHECK(got == want, "seg.max_right vs brute");
                    CHECK(got == off.max_right(l, f), "seg.max_right vs acl");

                    int r = rng() % (n + 1);
                    int got2 = mine.min_left(r, f);
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
                mine.apply(l, r, v);
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
                CHECK(mine.prod(l, r).sum == want, "lazy.prod vs brute");
                CHECK(mine.prod(l, r).sum == off.prod(l, r).sum,
                      "lazy.prod vs acl");
                CHECK(mine.prod(l, r).len == r - l, "lazy.prod len");
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
                        CHECK(got == want, "lazy.max_right vs brute");
                    }
                    CHECK(got == off.max_right(l, g), "lazy.max_right vs acl");
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
                mine.apply(l, r, v);
                off.apply(l, r, v);
            } else if (t == 1) {  // 区间最大
                ll want = e3();
                for (int i = l; i < r; i++) want = max(want, a[i]);
                CHECK(mine.prod(l, r) == want, "lazyset.prod vs brute");
                CHECK(mine.prod(l, r) == off.prod(l, r), "lazyset.prod vs acl");
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
        CHECK(s1.prod(0, 0) == e1(), "segtree prod(0,0)");
        MineAdd z0;
        CHECK(z0.all_prod().sum == 0, "empty lazy all_prod");
        MineAdd z1(5);
        z1.apply(0, 5, 3);
        CHECK(z1.prod(0, 5).sum == 0, "lazy default e() then apply");
    }

    printf(fails ? "TOTAL FAILS: %d\n" : "ALL PASS (fails=%d)\n", fails);
    return fails != 0;
}
