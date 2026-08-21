// 模板自测：多项式全家桶的随机对拍
//
// 这个文件不对应任何一道 OJ 题，它把 sections/06_advanced_math.tex 里
// 「NTT 卷积 / 多项式全家桶 / 任意模数卷积 / 分治 FFT / 线性递推 /
//   拉格朗日插值 / FWT 与子集卷积」几节的代码原样搬过来，
// 逐项与朴素实现随机对拍。改动那一章的任何代码后跑一遍它。
//
// 本机若装了 ac-library（/Users/zzy/Desktop/DoProblemAsMyTaste/library/ac-library），
// 编译时加上 -I 那个路径，还会额外把移植版 NTT 与官方实现逐点比对。
//
//   c++ -std=c++20 -O2 -o poly_check poly_check.cpp
//   ./poly_check

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using ull = unsigned long long;

// ======== 以下与 sections/02_basic_math.tex「自动取模类」一致 ========
template <class T, T MOD, class Wide = ull>
struct modint {
    T v;
    modint(ll x = 0) {
        ll t = x % (ll)MOD;
        if (t < 0) t += MOD;
        v = (T)t;
    }
    T val() const { return v; }
    modint& operator+=(const modint &o) {
        T x = v + o.v;
        if (x >= MOD) x -= MOD;
        v = x;
        return *this;
    }
    modint& operator-=(const modint &o) {
        T x = v >= o.v ? v - o.v : (T)(v + MOD - o.v);
        v = x;
        return *this;
    }
    modint& operator*=(const modint &o) {
        Wide t = (Wide)v * (Wide)o.v;
        v = (T)(t % MOD);
        return *this;
    }
    static modint powmod(modint a, ll e) {
        modint r = 1;
        while (e) {
            if (e & 1) r *= a;
            a *= a;
            e >>= 1;
        }
        return r;
    }
    static modint inv(modint a) { return powmod(a, (ll)MOD - 2); }
    modint& operator/=(const modint &o) { return *this *= inv(o); }
    modint operator+() const { return *this; }
    modint operator-() const { return modint(0) - *this; }
    friend modint operator+(modint a, const modint &b) { return a += b; }
    friend modint operator-(modint a, const modint &b) { return a -= b; }
    friend modint operator*(modint a, const modint &b) { return a *= b; }
    friend modint operator/(modint a, const modint &b) { return a /= b; }
    friend bool operator==(const modint &a, const modint &b) { return a.v == b.v; }
    friend bool operator!=(const modint &a, const modint &b) { return a.v != b.v; }
    friend ostream& operator<<(ostream &os, const modint &x) { return os << x.v; }
    friend istream& operator>>(istream &is, modint &x) {
        ll t;
        if (!(is >> t)) return is;
        x = modint(t);
        return is;
    }
};

// ===== NTT 卷积：ac-library atcoder/convolution.hpp 精简移植 =====
// 参考（抄的） https://github.com/atcoder/ac-library/blob/master/atcoder/convolution.hpp
// 模板题 https://www.luogu.com.cn/problem/P3803
// 编译期快速幂 + 最小原根：换模数时不用查表，也就不会写错原根
constexpr ull powConst(ull b, ull e, ull m) {
    ull r = 1;
    for (b %= m; e; e >>= 1, b = b * b % m)
        if (e & 1) r = r * b % m;
    return r;
}
constexpr uint32_t primitiveRoot(uint32_t m) {
    uint32_t divs[20] = {2}, x = (m - 1) / 2;
    int cnt = 1;
    while (x % 2 == 0) x /= 2;
    for (uint32_t i = 3; (ull)i * i <= x; i += 2)
        if (x % i == 0) {
            divs[cnt++] = i;
            while (x % i == 0) x /= i;
        }
    if (x > 1) divs[cnt++] = x;
    for (uint32_t g = 2;; g++) {
        bool ok = true;
        for (int i = 0; i < cnt && ok; i++)
            ok = powConst(g, (m - 1) / divs[i], m) != 1;
        if (ok) return g;
    }
}

// MOD 必须形如 c * 2^k + 1（k 要够大），G 默认取最小原根
template <uint32_t MOD, uint32_t G = primitiveRoot(MOD)>
struct NTT {
    using mint = modint<uint32_t, MOD>;
    static constexpr int rk = __builtin_ctz(MOD - 1);   // 最长支持 2^rk
    struct Info {
        mint root[rk + 1], iroot[rk + 1];               // root[i]^(2^i) == 1
        mint rate2[rk - 1], irate2[rk - 1];
        mint rate3[rk - 2], irate3[rk - 2];
        Info() {
            root[rk] = mint::powmod(mint(G), (MOD - 1) >> rk);
            iroot[rk] = mint::inv(root[rk]);
            for (int i = rk - 1; i >= 0; i--) {
                root[i] = root[i + 1] * root[i + 1];
                iroot[i] = iroot[i + 1] * iroot[i + 1];
            }
            mint prod = 1, iprod = 1;
            for (int i = 0; i + 2 <= rk; i++) {
                rate2[i] = root[i + 2] * prod;
                irate2[i] = iroot[i + 2] * iprod;
                prod *= iroot[i + 2], iprod *= root[i + 2];
            }
            prod = 1, iprod = 1;
            for (int i = 0; i + 3 <= rk; i++) {
                rate3[i] = root[i + 3] * prod;
                irate3[i] = iroot[i + 3] * iprod;
                prod *= iroot[i + 3], iprod *= root[i + 3];
            }
        }
    };
    static const Info &info() { static Info x; return x; }

    // 正变换：一次推两层（radix-4），出来是位逆序，因此不需要 rev 置换数组
    static void butterfly(vector<mint> &a) {
        const Info &w = info();
        int n = a.size(), h = __builtin_ctz((unsigned)n), len = 0;
        while (len < h) {
            if (h - len == 1) {                       // 只剩一层，走 radix-2
                int p = 1 << (h - len - 1);
                mint rot = 1;
                for (int s = 0; s < (1 << len); s++) {
                    int off = s << (h - len);
                    for (int i = 0; i < p; i++) {
                        mint l = a[i + off], r = a[i + off + p] * rot;
                        a[i + off] = l + r, a[i + off + p] = l - r;
                    }
                    if (s + 1 != (1 << len))
                        rot *= w.rate2[__builtin_ctz(~(unsigned)s)];
                }
                len++;
            } else {                                  // radix-4
                int p = 1 << (h - len - 2);
                mint rot = 1, imag = w.root[2];
                for (int s = 0; s < (1 << len); s++) {
                    mint rot2 = rot * rot, rot3 = rot2 * rot;
                    int off = s << (h - len);
                    for (int i = 0; i < p; i++) {
                        ull m2 = (ull)MOD * MOD;
                        ull a0 = a[i + off].val();
                        ull a1 = (ull)a[i + off + p].val() * rot.val();
                        ull a2 = (ull)a[i + off + 2 * p].val() * rot2.val();
                        ull a3 = (ull)a[i + off + 3 * p].val() * rot3.val();
                        ull t = (ull)mint(a1 + m2 - a3).val() * imag.val();
                        ull na2 = m2 - a2;
                        a[i + off] = a0 + a2 + a1 + a3;
                        a[i + off + p] = a0 + a2 + (2 * m2 - (a1 + a3));
                        a[i + off + 2 * p] = a0 + na2 + t;
                        a[i + off + 3 * p] = a0 + na2 + (m2 - t);
                    }
                    if (s + 1 != (1 << len))
                        rot *= w.rate3[__builtin_ctz(~(unsigned)s)];
                }
                len += 2;
            }
        }
    }
    // 逆变换：吃位逆序输入，还原自然序（未除以 n，交给 conv 统一处理）
    static void butterflyInv(vector<mint> &a) {
        const Info &w = info();
        int n = a.size(), h = __builtin_ctz((unsigned)n), len = h;
        while (len) {
            if (len == 1) {
                int p = 1 << (h - len);
                mint irot = 1;
                for (int s = 0; s < (1 << (len - 1)); s++) {
                    int off = s << (h - len + 1);
                    for (int i = 0; i < p; i++) {
                        mint l = a[i + off], r = a[i + off + p];
                        a[i + off] = l + r;
                        a[i + off + p] =
                            (ull)((uint32_t)(l.val() - r.val()) + MOD) * irot.val();
                    }
                    if (s + 1 != (1 << (len - 1)))
                        irot *= w.irate2[__builtin_ctz(~(unsigned)s)];
                }
                len--;
            } else {
                int p = 1 << (h - len);
                mint irot = 1, iimag = w.iroot[2];
                for (int s = 0; s < (1 << (len - 2)); s++) {
                    mint irot2 = irot * irot, irot3 = irot2 * irot;
                    int off = s << (h - len + 2);
                    for (int i = 0; i < p; i++) {
                        ull a0 = a[i + off].val(), a1 = a[i + off + p].val();
                        ull a2 = a[i + off + 2 * p].val();
                        ull a3 = a[i + off + 3 * p].val();
                        ull t = mint((MOD + a2 - a3) * iimag.val()).val();
                        a[i + off] = a0 + a1 + a2 + a3;
                        a[i + off + p] = (a0 + (MOD - a1) + t) * irot.val();
                        a[i + off + 2 * p] =
                            (a0 + a1 + (MOD - a2) + (MOD - a3)) * irot2.val();
                        a[i + off + 3 * p] =
                            (a0 + (MOD - a1) + (MOD - t)) * irot3.val();
                    }
                    if (s + 1 != (1 << (len - 2)))
                        irot *= w.irate3[__builtin_ctz(~(unsigned)s)];
                }
                len -= 2;
            }
        }
    }
    static vector<mint> naive(const vector<mint> &a, const vector<mint> &b) {
        vector<mint> c(a.size() + b.size() - 1);
        for (size_t i = 0; i < a.size(); i++)
            for (size_t j = 0; j < b.size(); j++) c[i + j] += a[i] * b[j];
        return c;
    }
    // 短的一侧不超过 60 时直接朴素乘，省掉两次变换的常数
    static vector<mint> conv(vector<mint> a, vector<mint> b) {
        int n = a.size(), m = b.size();
        if (!n || !m) return {};
        if (min(n, m) <= 60) return naive(a, b);
        int z = 1;
        while (z < n + m - 1) z <<= 1;
        assert((MOD - 1) % z == 0);   // 长度超过 2^rk：换模数或走三模数版本
        a.resize(z), b.resize(z);
        butterfly(a), butterfly(b);
        for (int i = 0; i < z; i++) a[i] *= b[i];
        butterflyInv(a);
        a.resize(n + m - 1);
        mint iz = mint::inv(mint(z));
        for (auto &x : a) x *= iz;
        return a;
    }
};

constexpr uint32_t P = 998244353;   // 原根 3，rk = 23（最长做到 2^23）
using Z = modint<uint32_t, P>;
vector<Z> convolution(const vector<Z> &a, const vector<Z> &b) {
    return NTT<P>::conv(a, b);
}

// ===== 多项式全家桶：系数下标从低到高，f = a[0] + a[1] x + a[2] x^2 + ... =====
struct Poly : vector<Z> {
    using vector<Z>::vector;
    Poly(const vector<Z> &a) : vector<Z>(a) {}
    Z get(int i) const { return i < 0 || i >= (int)size() ? Z(0) : (*this)[i]; }
    int deg() const {              // 最高非零次数，零多项式返回 -1
        int i = (int)size() - 1;
        while (i >= 0 && (*this)[i] == Z(0)) i--;
        return i;
    }
    Poly &norm() { resize(deg() + 1); return *this; }
    Poly trunc(int k) const {      // 取前 k 项，不足补零
        Poly f = *this;
        f.resize(k);
        return f;
    }
    Poly shift(int k) const {      // 乘上 x^k，k 为负则右移（丢掉低位）
        if (k >= 0) {
            Poly f = *this;
            f.insert(f.begin(), k, Z(0));
            return f;
        }
        if ((int)size() <= -k) return Poly();
        return Poly(begin() - k, end());
    }
    friend Poly operator+(const Poly &a, const Poly &b) {
        Poly c(max(a.size(), b.size()));
        for (int i = 0; i < (int)c.size(); i++) c[i] = a.get(i) + b.get(i);
        return c;
    }
    friend Poly operator-(const Poly &a, const Poly &b) {
        Poly c(max(a.size(), b.size()));
        for (int i = 0; i < (int)c.size(); i++) c[i] = a.get(i) - b.get(i);
        return c;
    }
    friend Poly operator-(const Poly &a) {
        Poly c = a;
        for (auto &x : c) x = -x;
        return c;
    }
    friend Poly operator*(const Poly &a, Z k) {
        Poly c = a;
        for (auto &x : c) x *= k;
        return c;
    }
    friend Poly operator*(Z k, const Poly &a) { return a * k; }
    friend Poly operator*(const Poly &a, const Poly &b) {
        return Poly(convolution(a, b));
    }
    Poly &operator+=(const Poly &b) { return *this = *this + b; }
    Poly &operator-=(const Poly &b) { return *this = *this - b; }
    Poly &operator*=(const Poly &b) { return *this = *this * b; }
    Poly &operator*=(Z k) { return *this = *this * k; }

    Poly deriv() const {           // 求导
        if (empty()) return Poly();
        Poly f(size() - 1);
        for (int i = 1; i < (int)size(); i++) f[i - 1] = (*this)[i] * Z(i);
        return f;
    }
    Poly integr() const {          // 积分，常数项取 0
        Poly f(size() + 1);
        for (int i = 0; i < (int)size(); i++) f[i + 1] = (*this)[i] / Z(i + 1);
        return f;
    }
    // 乘法逆，要求 a[0] != 0    模板题 https://www.luogu.com.cn/problem/P4238
    Poly inv(int m) const {
        Poly x{Z::inv((*this)[0])};
        for (int k = 1; k < m; k <<= 1)
            x = (x * (Poly{Z(2)} - trunc(k << 1) * x)).trunc(k << 1);
        return x.trunc(m);
    }
    // ln，要求 a[0] == 1        模板题 https://www.luogu.com.cn/problem/P4725
    Poly log(int m) const { return (deriv() * inv(m)).integr().trunc(m); }
    // exp，要求 a[0] == 0       模板题 https://www.luogu.com.cn/problem/P4726
    Poly exp(int m) const {
        Poly x{Z(1)};
        for (int k = 1; k < m; k <<= 1)
            x = (x * (Poly{Z(1)} - x.log(k << 1) + trunc(k << 1))).trunc(k << 1);
        return x.trunc(m);
    }
    // 开方，这里只处理 a[0] == 1；a[0] 是别的二次剩余时先解出 sqrt(a[0]) 再整体缩放
    // 模板题 https://www.luogu.com.cn/problem/P5205
    Poly sqrt(int m) const {
        Poly x{Z(1)};
        Z inv2 = Z::inv(Z(2));
        for (int k = 1; k < m; k <<= 1)
            x = (x + (trunc(k << 1) * x.inv(k << 1)).trunc(k << 1)) * inv2;
        return x.trunc(m);
    }
    // k 次幂，允许最低非零位不在 0 处、也允许 a[i] != 1
    // 模板题 https://www.luogu.com.cn/problem/P5245
    Poly pow(ll k, int m) const {
        if (k == 0) { Poly r(m); if (m) r[0] = 1; return r; }
        int i = 0;
        while (i < (int)size() && (*this)[i] == Z(0)) i++;
        if (i == (int)size() || (__int128)i * k >= m) return Poly(m);
        Z v = (*this)[i];
        Poly f = shift(-i) * Z::inv(v);
        int rest = m - i * (int)k;
        return (f.log(rest) * Z(k % (ll)P)).exp(rest).shift(i * (int)k)
               * Z::powmod(v, k);
    }
    // 转置卷积：取 (*this) * reverse(b) 的高位，多点求值的核心零件
    Poly mulT(const Poly &b) const {
        if (b.empty()) return Poly();
        Poly c = b;
        reverse(c.begin(), c.end());
        return (*this * c).shift(-((int)b.size() - 1));
    }
    // 多点求值 O(n log^2 n)   模板题 https://www.luogu.com.cn/problem/P5050
    vector<Z> eval(vector<Z> x) const {
        if (empty()) return vector<Z>(x.size(), Z(0));
        int n = max((int)x.size(), (int)size());
        vector<Poly> q(n << 2);
        vector<Z> ans(x.size());
        x.resize(n);
        auto build = [&](auto &self, int p, int l, int r) -> void {
            if (r - l == 1) { q[p] = Poly{Z(1), -x[l]}; return; }
            int m = (l + r) >> 1;
            self(self, p << 1, l, m), self(self, p << 1 | 1, m, r);
            q[p] = q[p << 1] * q[p << 1 | 1];
        };
        build(build, 1, 0, n);
        auto work = [&](auto &self, int p, int l, int r, const Poly &f) -> void {
            if (r - l == 1) { if (l < (int)ans.size()) ans[l] = f.get(0); return; }
            int m = (l + r) >> 1;
            self(self, p << 1, l, m, f.mulT(q[p << 1 | 1]).trunc(m - l));
            self(self, p << 1 | 1, m, r, f.mulT(q[p << 1]).trunc(r - m));
        };
        work(work, 1, 0, n, mulT(q[1].inv(n)));
        return ans;
    }
};

// 带余除法 f = q * g + r，deg(r) < deg(g)
// 模板题 https://www.luogu.com.cn/problem/P4512
pair<Poly, Poly> divmod(Poly f, Poly g) {
    int n = f.deg(), m = g.deg();
    f.norm(), g.norm();
    if (n < m) return {Poly(), f};
    Poly fr = f, gr = g;
    reverse(fr.begin(), fr.end()), reverse(gr.begin(), gr.end());
    Poly q = (fr.trunc(n - m + 1) * gr.inv(n - m + 1)).trunc(n - m + 1);
    reverse(q.begin(), q.end());
    Poly r = (f - q * g).trunc(max(m, 1));
    return {q.norm(), r.norm()};
}

// 快速插值：给 n 个点值还原 n-1 次多项式 O(n log^2 n)
// 模板题 https://www.luogu.com.cn/problem/P5158
Poly interpolate(const vector<Z> &x, const vector<Z> &y) {
    int n = x.size();
    vector<Poly> m(n << 2);
    auto build = [&](auto &self, int p, int l, int r) -> void {
        if (r - l == 1) { m[p] = Poly{-x[l], Z(1)}; return; }
        int mid = (l + r) >> 1;
        self(self, p << 1, l, mid), self(self, p << 1 | 1, mid, r);
        m[p] = m[p << 1] * m[p << 1 | 1];
    };
    build(build, 1, 0, n);
    vector<Z> d = m[1].deriv().eval(x);      // d[i] = M'(x_i)
    auto solve = [&](auto &self, int p, int l, int r) -> Poly {
        if (r - l == 1) return Poly{y[l] / d[l]};
        int mid = (l + r) >> 1;
        return self(self, p << 1, l, mid) * m[p << 1 | 1]
             + self(self, p << 1 | 1, mid, r) * m[p << 1];
    };
    return solve(solve, 1, 0, n);
}

// ===== 任意模数 / 大整数卷积：三模数 NTT + Garner 合并 =====
// 模板题 https://www.luogu.com.cn/problem/P4245
// 三个模数乘积 ~5.9e25，足够容纳任意 int 系数、长度 1e6 级别的卷积真值
// 注意原根各不相同（167772161 与 469762049 是 3，754974721 是 11），
// 这里靠 primitiveRoot 编译期算出来，不必手写
constexpr uint32_t M1 = 167772161, M2 = 469762049, M3 = 754974721;

template <uint32_t MOD>
vector<ll> convRaw(const vector<ll> &a, const vector<ll> &b) {
    using mint = modint<uint32_t, MOD>;
    vector<mint> A(a.size()), B(b.size());
    for (size_t i = 0; i < a.size(); i++) A[i] = a[i];
    for (size_t i = 0; i < b.size(); i++) B[i] = b[i];
    auto C = NTT<MOD>::conv(A, B);
    vector<ll> c(C.size());
    for (size_t i = 0; i < C.size(); i++) c[i] = C[i].val();
    return c;
}

// p > 0：结果对 p 取模，p 可以是 1e9+7 这种非 NTT 模数
// p <= 0：不取模，要求真实值 < 2^63（否则请自行换成 __int128 返回）
vector<ll> convolutionAnyMod(const vector<ll> &a, const vector<ll> &b, ll p) {
    if (a.empty() || b.empty()) return {};
    auto c1 = convRaw<M1>(a, b), c2 = convRaw<M2>(a, b), c3 = convRaw<M3>(a, b);
    ll i12 = modint<uint32_t, M2>::inv(modint<uint32_t, M2>(M1)).val();
    ll i3 = modint<uint32_t, M3>::inv(
                modint<uint32_t, M3>((ll)((ull)M1 * M2 % M3))).val();
    vector<ll> c(c1.size());
    for (size_t i = 0; i < c1.size(); i++) {
        ll k1 = (c2[i] - c1[i]) % (ll)M2 * i12 % (ll)M2;
        if (k1 < 0) k1 += M2;
        ll x12 = c1[i] + (ll)M1 * k1;                  // < M1*M2 ~ 7.9e16
        ll k2 = (c3[i] - x12) % (ll)M3 * i3 % (ll)M3;
        if (k2 < 0) k2 += M3;
        __int128 x = (__int128)x12 + (__int128)M1 * M2 * k2;
        c[i] = p > 0 ? (ll)(x % p) : (ll)x;
    }
    return c;
}

// ===== 分治 FFT：f[0] 已知，f[i] = sum_{j=1..i} f[i-j] * g[j] =====
// 模板题 https://www.luogu.com.cn/problem/P4721
// 也可以直接写成 f = f0 / (1 - g)，即 Poly{1} - g 求逆，常数更小、代码更短
vector<Z> divideConquerFFT(const vector<Z> &g, int n, Z f0 = 1) {
    vector<Z> f(n);
    if (!n) return f;
    f[0] = f0;
    auto cdq = [&](auto &self, int l, int r) -> void {
        if (r - l == 1) return;
        int m = (l + r) >> 1;
        self(self, l, m);
        vector<Z> a(f.begin() + l, f.begin() + m);
        vector<Z> b(g.begin(), g.begin() + min((int)g.size(), r - l));
        vector<Z> c = convolution(a, b);
        for (int i = m; i < r; i++)
            if (i - l < (int)c.size()) f[i] += c[i - l];
        self(self, m, r);
    };
    cdq(cdq, 0, n);
    return f;
}

// ===== Berlekamp-Massey：求最短线性递推式 O(n^2) =====
// 模板题 https://www.luogu.com.cn/problem/P5487
// 返回 c，满足 s[i] = sum_{j} c[j] * s[i-1-j]
vector<Z> berlekampMassey(const vector<Z> &s) {
    vector<Z> ls, cur;
    int lf = 0;
    Z ld = 0;
    for (int i = 0; i < (int)s.size(); i++) {
        Z t = 0;
        for (int j = 0; j < (int)cur.size(); j++) t += cur[j] * s[i - 1 - j];
        if (t == s[i]) continue;
        if (cur.empty()) {                    // 还没有任何递推式
            cur.resize(i + 1);
            lf = i, ld = t - s[i];
            continue;
        }
        Z k = (t - s[i]) / ld;
        vector<Z> c(i - lf - 1);
        c.push_back(k);
        for (int j = 0; j < (int)ls.size(); j++) c.push_back(-ls[j] * k);
        if (c.size() < cur.size()) c.resize(cur.size());
        for (int j = 0; j < (int)cur.size(); j++) c[j] += cur[j];
        if (i - (int)cur.size() > lf - (int)ls.size()) ls = cur, lf = i, ld = t - s[i];
        cur = c;
    }
    return cur;
}

// ===== Bostan-Mori：求 [x^n] P(x)/Q(x)，O(k log k log n) =====
Z bostanMori(Poly p, Poly q, ll n) {
    while (n) {
        Poly qm = q;                          // Q(-x)
        for (int i = 1; i < (int)qm.size(); i += 2) qm[i] = -qm[i];
        Poly u = p * qm, v = q * qm;          // V(x^2) = Q(x)Q(-x)，只剩偶次项
        Poly np(u.size() / 2 + 1), nq(v.size() / 2 + 1);
        for (int i = (int)(n & 1); i < (int)u.size(); i += 2) np[i >> 1] = u[i];
        for (int i = 0; i < (int)v.size(); i += 2) nq[i >> 1] = v[i];
        p = np, q = nq, n >>= 1;
    }
    return p.get(0) / q[0];
}

// 常系数齐次线性递推：给出前 k 项 a[0..k-1] 与系数 c（BM 的输出可直接喂进来），
// 求 a[n]。模板题 https://www.luogu.com.cn/problem/P4723
Z linearRecurrence(const vector<Z> &a, const vector<Z> &c, ll n) {
    int k = c.size();
    if (n < (ll)a.size()) return a[n];
    Poly q(k + 1);
    q[0] = 1;
    for (int i = 0; i < k; i++) q[i + 1] = -c[i];
    Poly p = (Poly(a) * q).trunc(k);
    return bostanMori(p, q, n);
}

// ===== 拉格朗日插值 =====
// 任意 n 个点值求 f(k)，O(n^2)  模板题 https://www.luogu.com.cn/problem/P4781
Z lagrange(const vector<Z> &x, const vector<Z> &y, Z k) {
    int n = x.size();
    Z ans = 0;
    for (int i = 0; i < n; i++) {
        Z num = y[i], den = 1;
        for (int j = 0; j < n; j++)
            if (j != i) num *= k - x[j], den *= x[i] - x[j];
        ans += num / den;
    }
    return ans;
}
// 横坐标是 0,1,...,n-1 时可以做到 O(n)：前后缀积 + 阶乘逆元
// 典型用途：自然数幂和 sum_{i=1..m} i^t 是 t+1 次多项式，取 t+2 个点值即可
Z lagrangeConsecutive(const vector<Z> &y, ll k) {
    int n = y.size();
    if (k < n) return y[k];
    vector<Z> pre(n + 1), suf(n + 1), fac(n), ifac(n);
    pre[0] = suf[n] = 1;
    for (int i = 0; i < n; i++) pre[i + 1] = pre[i] * (Z(k) - Z(i));
    for (int i = n - 1; i >= 0; i--) suf[i] = suf[i + 1] * (Z(k) - Z(i));
    fac[0] = 1;
    for (int i = 1; i < n; i++) fac[i] = fac[i - 1] * Z(i);
    ifac[n - 1] = Z::inv(fac[n - 1]);
    for (int i = n - 1; i >= 1; i--) ifac[i - 1] = ifac[i] * Z(i);
    Z ans = 0;
    for (int i = 0; i < n; i++) {
        Z t = y[i] * pre[i] * suf[i + 1] * ifac[i] * ifac[n - 1 - i];
        ans += ((n - 1 - i) & 1) ? -t : t;
    }
    return ans;
}

// ===== FWT：位运算卷积 =====
// 模板题 https://www.luogu.com.cn/problem/P4717
// type: 0 = 或(OR)  1 = 与(AND)  2 = 异或(XOR)；数组长度必须是 2 的幂
void fwt(vector<Z> &a, int type, bool inv) {
    int n = a.size();
    for (int len = 1; len < n; len <<= 1)
        for (int i = 0; i < n; i += len << 1)
            for (int j = i; j < i + len; j++) {
                Z u = a[j], v = a[j + len];
                if (type == 0) a[j + len] = inv ? v - u : v + u;
                else if (type == 1) a[j] = inv ? u - v : u + v;
                else a[j] = u + v, a[j + len] = u - v;
            }
    if (type == 2 && inv) {
        Z iv = Z::inv(Z(n));
        for (auto &x : a) x *= iv;
    }
}
vector<Z> bitwiseConv(vector<Z> a, vector<Z> b, int type) {
    fwt(a, type, false), fwt(b, type, false);
    for (size_t i = 0; i < a.size(); i++) a[i] *= b[i];
    fwt(a, type, true);
    return a;
}

// ===== 子集卷积：c[S] = sum_{T subset S} a[T] * b[S\T]，O(n^2 2^n) =====
// 模板题 https://www.luogu.com.cn/problem/P6097
// 做法：给或卷积补一维「popcount」，占位后维度相加才是真正的不交并
vector<Z> subsetConv(const vector<Z> &a, const vector<Z> &b, int n) {
    int N = 1 << n;
    vector<vector<Z>> fa(n + 1, vector<Z>(N)), fb(n + 1, vector<Z>(N));
    vector<vector<Z>> fc(n + 1, vector<Z>(N));
    for (int s = 0; s < N; s++) {
        fa[__builtin_popcount(s)][s] = a[s];
        fb[__builtin_popcount(s)][s] = b[s];
    }
    for (int i = 0; i <= n; i++) fwt(fa[i], 0, false), fwt(fb[i], 0, false);
    for (int i = 0; i <= n; i++)
        for (int j = 0; i + j <= n; j++)
            for (int s = 0; s < N; s++) fc[i + j][s] += fa[i][s] * fb[j][s];
    for (int i = 0; i <= n; i++) fwt(fc[i], 0, true);
    vector<Z> c(N);
    for (int s = 0; s < N; s++) c[s] = fc[__builtin_popcount(s)][s];
    return c;
}

// ======== 以下是测试代码，不属于模板正文 ========
mt19937_64 rng(20260821);
Z rnd() { return Z((ll)(rng() % P)); }
Poly randPoly(int n) { Poly f(n); for (auto &x : f) x = rnd(); return f; }

Poly naiveMul(const Poly &a, const Poly &b, int m) {
    Poly c(a.size() + b.size());
    for (size_t i = 0; i < a.size(); i++)
        for (size_t j = 0; j < b.size(); j++) c[i + j] += a[i] * b[j];
    return c.trunc(m);
}
void same(const Poly &a, const Poly &b, const char *tag) {
    int n = max(a.size(), b.size());
    for (int i = 0; i < n; i++)
        if (a.get(i) != b.get(i)) {
            printf("FAIL %s at %d: %u vs %u\n", tag, i, a.get(i).val(), b.get(i).val());
            exit(1);
        }
}

#if __has_include(<atcoder/convolution>)
#include <atcoder/convolution>
#include <atcoder/modint>
// 与官方 ac-library 逐点比对，确认精简移植没有改坏语义
void testAgainstACL() {
    using mint = atcoder::static_modint<998244353>;
    for (int t = 0; t < 200; t++) {
        int n = rng() % 2000 + 1, m = rng() % 2000 + 1;
        vector<Z> a(n), b(m);
        vector<mint> A(n), B(m);
        for (int i = 0; i < n; i++) { ll v = rng() % P; a[i] = v; A[i] = v; }
        for (int i = 0; i < m; i++) { ll v = rng() % P; b[i] = v; B[i] = v; }
        auto c = convolution(a, b);
        auto C = atcoder::convolution(A, B);
        assert(c.size() == C.size());
        for (size_t i = 0; i < c.size(); i++)
            if (c[i].val() != C[i].val()) { puts("FAIL vs ac-library"); exit(1); }
    }
    puts("  [ok] NTT 与 ac-library 官方实现逐点一致（200 组）");
}
#else
void testAgainstACL() {
    puts("  [skip] 没找到 ac-library 头文件，跳过与官方实现的比对");
}
#endif

// 移植版 NTT 与朴素卷积、以及四个常用模数的正确性
void testNTT() {
    static_assert(primitiveRoot(998244353) == 3);
    static_assert(primitiveRoot(167772161) == 3);
    static_assert(primitiveRoot(469762049) == 3);
    static_assert(primitiveRoot(754974721) == 11);   // 注意不是 3
    for (int t = 0; t < 100; t++) {
        int n = rng() % 300 + 1, m = rng() % 300 + 1;
        vector<Z> a(n), b(m);
        for (auto &x : a) x = rnd();
        for (auto &x : b) x = rnd();
        auto c = convolution(a, b);
        auto d = NTT<P>::naive(a, b);
        for (size_t i = 0; i < c.size(); i++)
            if (c[i] != d[i]) { puts("FAIL ntt vs naive"); exit(1); }
    }
    puts("  [ok] NTT 卷积 vs 朴素卷积（100 组）");
    testAgainstACL();
}

void testPoly() {
    for (int t = 0; t < 60; t++) {
        int n = (int)(rng() % 200) + 2;
        // inv
        Poly f = randPoly(n);
        if (f[0] == Z(0)) f[0] = 1;
        same((f * f.inv(n)).trunc(n), Poly{Z(1)}.trunc(n), "inv");
        // log/exp 互逆
        Poly g = randPoly(n); g[0] = 0;
        same(g.exp(n).log(n).trunc(n), g.trunc(n), "exp->log");
        Poly h = randPoly(n); h[0] = 1;
        same(h.log(n).exp(n).trunc(n), h.trunc(n), "log->exp");
        // sqrt
        Poly s = h.sqrt(n);
        same((s * s).trunc(n), h.trunc(n), "sqrt");
        // pow：与朴素快速幂比
        ll k = (ll)(rng() % 1000) + 1;
        Poly base = randPoly((int)(rng() % 20) + 1);
        Poly want(1); want[0] = 1;
        Poly cur = base;
        for (ll e = k; e; e >>= 1) {
            if (e & 1) want = naiveMul(want, cur, n);
            cur = naiveMul(cur, cur, n);
        }
        same(base.pow(k, n).trunc(n), want.trunc(n), "pow");
        // divmod
        int df = (int)(rng() % 300) + 1, dg = (int)(rng() % 100) + 1;
        Poly A = randPoly(df + 1), B = randPoly(dg + 1);
        if (B.back() == Z(0)) B.back() = 1;
        auto [q, r] = divmod(A, B);
        same((q * B + r).trunc(df + 1), A.trunc(df + 1), "divmod");
        if (r.deg() >= B.deg()) { puts("FAIL divmod: deg(r) >= deg(g)"); exit(1); }
        // eval：与逐点 Horner 比
        int m = (int)(rng() % 200) + 1;
        vector<Z> xs(m);
        for (auto &x : xs) x = rnd();
        Poly F = randPoly((int)(rng() % 200) + 1);
        vector<Z> got = F.eval(xs);
        for (int i = 0; i < m; i++) {
            Z acc = 0;
            for (int j = (int)F.size() - 1; j >= 0; j--) acc = acc * xs[i] + F[j];
            if (acc != got[i]) { printf("FAIL eval at %d\n", i); exit(1); }
        }
        // interpolate：点值还原
        int p = (int)(rng() % 100) + 1;
        vector<Z> px(p);
        set<uint32_t> used;
        for (auto &x : px) { do x = rnd(); while (!used.insert(x.val()).second); }
        Poly src = randPoly(p);
        vector<Z> py = src.eval(px);
        same(interpolate(px, py).trunc(p), src.trunc(p), "interpolate");
    }
    
    puts("  [ok] inv / log / exp / sqrt / pow / divmod / eval / interpolate");
}

void testMisc() {
    // 1. 任意模数卷积
    for (int t = 0; t < 20; t++) {
        int n = (int)(rng() % 200) + 1, m = (int)(rng() % 200) + 1;
        ll p = (t & 1) ? 1000000007LL : 1000000009LL;
        vector<ll> a(n), b(m);
        for (auto &x : a) x = rng() % p;
        for (auto &x : b) x = rng() % p;
        auto c = convolutionAnyMod(a, b, p);
        vector<ll> want(n + m - 1);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++)
                want[i + j] = (want[i + j] + (__int128)a[i] * b[j]) % p;
        for (size_t i = 0; i < want.size(); i++)
            if (c[i] != want[i]) { puts("FAIL anymod"); exit(1); }
    }
    // 无模版本（真值 < 2^63）
    for (int t = 0; t < 10; t++) {
        int n = (int)(rng() % 150) + 1, m = (int)(rng() % 150) + 1;
        vector<ll> a(n), b(m);
        for (auto &x : a) x = rng() % 1000000;
        for (auto &x : b) x = rng() % 1000000;
        auto c = convolutionAnyMod(a, b, 0);
        vector<ll> want(n + m - 1);
        for (int i = 0; i < n; i++)
            for (int j = 0; j < m; j++) want[i + j] += a[i] * b[j];
        for (size_t i = 0; i < want.size(); i++)
            if (c[i] != want[i]) { puts("FAIL anymod-ll"); exit(1); }
    }
    // 2. 分治 FFT vs 朴素递推 vs 求逆
    for (int t = 0; t < 20; t++) {
        int n = (int)(rng() % 500) + 2;
        vector<Z> g(n);
        for (int i = 1; i < n; i++) g[i] = rnd();
        auto f = divideConquerFFT(g, n);
        vector<Z> want(n);
        want[0] = 1;
        for (int i = 1; i < n; i++)
            for (int j = 1; j <= i; j++) want[i] += want[i - j] * g[j];
        for (int i = 0; i < n; i++) if (f[i] != want[i]) { puts("FAIL cdq"); exit(1); }
        Poly one{Z(1)};
        Poly inv = (one - Poly(g)).inv(n);      // f = 1 / (1 - g)
        for (int i = 0; i < n; i++) if (inv[i] != want[i]) { puts("FAIL cdq-inv"); exit(1); }
    }
    // 3. BM + 线性递推
    for (int t = 0; t < 20; t++) {
        int k = (int)(rng() % 20) + 1, len = 4 * k + 10;
        vector<Z> c(k);
        for (auto &x : c) x = rnd();
        vector<Z> s(len);
        for (int i = 0; i < k; i++) s[i] = rnd();
        for (int i = k; i < len; i++)
            for (int j = 0; j < k; j++) s[i] += c[j] * s[i - 1 - j];
        auto got = berlekampMassey(s);
        if ((int)got.size() > k) { puts("FAIL BM: recurrence too long"); exit(1); }
        int N = (int)(rng() % 100000) + len;
        vector<Z> big(N + 1);
        for (int i = 0; i < k; i++) big[i] = s[i];
        for (int i = k; i <= N; i++)
            for (int j = 0; j < k; j++) big[i] += c[j] * big[i - 1 - j];
        if (linearRecurrence(s, got, N) != big[N]) { puts("FAIL linearRecurrence"); exit(1); }
    }
    // 4. 拉格朗日插值
    for (int t = 0; t < 20; t++) {
        int n = (int)(rng() % 100) + 1;
        Poly f(n);
        for (auto &x : f) x = rnd();
        vector<Z> xs(n), ys(n);
        set<uint32_t> used;
        for (int i = 0; i < n; i++) {
            do xs[i] = rnd(); while (!used.insert(xs[i].val()).second);
        }
        ys = f.eval(xs);
        Z k = rnd(), acc = 0;
        for (int j = n - 1; j >= 0; j--) acc = acc * k + f[j];
        if (lagrange(xs, ys, k) != acc) { puts("FAIL lagrange"); exit(1); }
        vector<Z> cx(n), cy(n);
        for (int i = 0; i < n; i++) cx[i] = Z(i);
        cy = f.eval(cx);
        ll kk = (ll)(rng() % 1000000);
        Z want = 0;
        for (int j = n - 1; j >= 0; j--) want = want * Z(kk) + f[j];
        if (lagrangeConsecutive(cy, kk) != want) { puts("FAIL lagrangeConsecutive"); exit(1); }
    }
    // 5. FWT 三种 + 子集卷积
    for (int t = 0; t < 20; t++) {
        int n = (int)(rng() % 8) + 1, N = 1 << n;
        vector<Z> a(N), b(N);
        for (auto &x : a) x = rnd();
        for (auto &x : b) x = rnd();
        for (int type = 0; type < 3; type++) {
            auto got = bitwiseConv(a, b, type);
            vector<Z> want(N);
            for (int i = 0; i < N; i++)
                for (int j = 0; j < N; j++) {
                    int k = type == 0 ? (i | j) : type == 1 ? (i & j) : (i ^ j);
                    want[k] += a[i] * b[j];
                }
            for (int i = 0; i < N; i++)
                if (got[i] != want[i]) { printf("FAIL fwt type=%d\n", type); exit(1); }
        }
        auto got = subsetConv(a, b, n);
        vector<Z> want(N);
        for (int i = 0; i < N; i++)
            for (int j = 0; j < N; j++)
                if (!(i & j)) want[i | j] += a[i] * b[j];
        for (int i = 0; i < N; i++)
            if (got[i] != want[i]) { puts("FAIL subsetConv"); exit(1); }
    }
    
    puts("  [ok] 任意模数卷积 / 分治 FFT / BM + Bostan-Mori / 拉插 / FWT / 子集卷积");
}

int main() {
    puts("== 多项式模板自测 ==");
    testNTT();
    testPoly();
    testMisc();
    puts("全部通过");
}

