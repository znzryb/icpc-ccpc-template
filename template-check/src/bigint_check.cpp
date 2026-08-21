// 模板自测：高精度大整数类 bigint 的随机对拍
//
// 这个文件不对应任何一道 OJ 题，它把 sections/08_basic_algorithm.tex
// 「高精度大整数类」一节的代码原样搬过来，用三条互相独立的线交叉验证：
//
//   1. 小数据与 __int128 逐点比对（+ - * / % 与六个比较运算符、字符串往返），
//      把「符号语义」钉死成和内建整数完全一致：除法向零取整、余数跟随被除数；
//   2. 大数据与 O(n^2) 竖式乘法比对，独立于 Karatsuba 那条路径；
//   3. 大数据查代数恒等式：(a/b)*b + a%b == a、|a%b| < |b|、
//      (a*b)/b == a、a*(b+c) == a*b+a*c、gcd*lcm == |a*b| 等。
//
// 改动那一节的任何代码后跑一遍它。
//
//   c++ -std=c++20 -O2 -o bigint_check bigint_check.cpp
//   ./bigint_check

#include <bits/stdc++.h>
using namespace std;
using ll = long long;
using i128 = __int128;

// ======== 以下与 sections/08_basic_algorithm.tex「高精度大整数类」一致 ========
// 高精度大整数 bigint：1e9 压位 + Karatsuba 乘法 + 试商长除法
//   加减 O(n)，乘 O(n^1.585)，divmod O(n(n-m+1))，n/m 为压位后的位数（1 位 = 9 个十进制位）
//   语义与 C++ 内建整数一致：除法向零取整，余数符号跟随被除数
//   零的唯一表示是 a 为空且 sign = 1（trim() 维持这个不变量）
struct bigint {
    static constexpr int BASE = 1000000000, W = 9;  // 压 9 位十进制
    // 与小整数运算的 O(n) 快路径上界：a[i] * v 不能爆 long long（9e9 * 1e9 < 9.22e18）
    static constexpr long long SMALL = 9000000000LL;
    vector<int> a;  // 低位在前，每位取值 [0, BASE)
    int sign = 1;   // +1 / -1

    bigint() {}
    bigint(long long v) { *this = v; }
    // 这条 int 重载不能删：字面量 0 同时是空指针常量，
    // 少了它，b < 0 这种写法会在 bigint(long long) 与 bigint(const char*) 之间二义
    bigint(int v) : bigint((long long)v) {}
    bigint(const string &s) { read(s); }
    bigint(const char *s) { read(s); }

    void operator=(long long v) {
        a.clear();
        sign = v < 0 ? -1 : 1;
        // 先转 unsigned 再取反，否则 v = LLONG_MIN 时 -v 溢出（UB）
        unsigned long long u =
            v < 0 ? -(unsigned long long)v : (unsigned long long)v;
        for (; u; u /= BASE) a.push_back(int(u % BASE));
        if (a.empty()) sign = 1;
    }
    void trim() {  // 去前导零，顺手把 -0 归一成 +0
        while (!a.empty() && a.back() == 0) a.pop_back();
        if (a.empty()) sign = 1;
    }
    bool isZero() const { return a.empty(); }
    bigint abs() const { bigint r = *this; r.sign = 1; return r; }
    bigint operator+() const { return *this; }
    bigint operator-() const {
        bigint r = *this;
        if (!r.isZero()) r.sign = -sign;
        return r;
    }

    // ---------- 比较 ----------
    static int cmpAbs(const bigint &x, const bigint &y) {  // 只比绝对值
        if (x.a.size() != y.a.size()) return x.a.size() < y.a.size() ? -1 : 1;
        for (int i = int(x.a.size()) - 1; i >= 0; --i)
            if (x.a[i] != y.a[i]) return x.a[i] < y.a[i] ? -1 : 1;
        return 0;
    }
    static int cmp(const bigint &x, const bigint &y) {
        if (x.sign != y.sign) return x.sign < y.sign ? -1 : 1;
        return cmpAbs(x, y) * x.sign;
    }
    // 六个比较运算符。写成 friend 是为了让两侧都能吃隐式转换，
    // b < 0 和 0 < b 都合法（写成成员函数就只有左侧能转）
    #define BIG_CMP(op) friend bool operator op( \
        const bigint &x, const bigint &y) { return cmp(x, y) op 0; }
    BIG_CMP(<) BIG_CMP(>) BIG_CMP(<=)
    BIG_CMP(>=) BIG_CMP(==) BIG_CMP(!=)
    #undef BIG_CMP

    // ---------- 加减 ----------
    static bigint addAbs(const bigint &x, const bigint &y) {  // |x| + |y|
        bigint r;
        r.a = x.a;
        for (int i = 0, carry = 0; i < int(y.a.size()) || carry; ++i) {
            if (i == int(r.a.size())) r.a.push_back(0);
            r.a[i] += carry + (i < int(y.a.size()) ? y.a[i] : 0);
            carry = r.a[i] >= BASE;
            if (carry) r.a[i] -= BASE;
        }
        return r;
    }
    static bigint subAbs(const bigint &x, const bigint &y) {  // |x| - |y|，要求 |x| >= |y|
        bigint r;
        r.a = x.a;
        for (int i = 0, borrow = 0; i < int(y.a.size()) || borrow; ++i) {
            r.a[i] -= borrow + (i < int(y.a.size()) ? y.a[i] : 0);
            borrow = r.a[i] < 0;
            if (borrow) r.a[i] += BASE;
        }
        r.trim();
        return r;
    }
    friend bigint operator+(const bigint &x, const bigint &y) {
        bigint r;
        if (x.sign == y.sign) r = addAbs(x, y), r.sign = x.sign;
        else if (cmpAbs(x, y) >= 0) r = subAbs(x, y), r.sign = x.sign;
        else r = subAbs(y, x), r.sign = y.sign;
        r.trim();
        return r;
    }
    friend bigint operator-(const bigint &x, const bigint &y) { return x + (-y); }

    // ---------- 与小整数运算（O(n) 快路径，divmod 内部靠它） ----------
    bigint &operator*=(long long v) {
        if (v > SMALL || v < -SMALL) return *this = *this * bigint(v);  // 超出快路径就退化成大整数乘
        if (v < 0) sign = -sign, v = -v;
        long long carry = 0;
        for (int i = 0; i < int(a.size()) || carry; ++i) {
            if (i == int(a.size())) a.push_back(0);
            long long cur = a[i] * v + carry;
            a[i] = int(cur % BASE);
            carry = cur / BASE;
        }
        trim();
        return *this;
    }
    bigint &operator/=(long long v) {  // 向零取整
        if (v > SMALL || v < -SMALL) return *this = *this / bigint(v);
        if (v < 0) sign = -sign, v = -v;
        long long rem = 0;
        for (int i = int(a.size()) - 1; i >= 0; --i) {
            long long cur = a[i] + rem * BASE;
            a[i] = int(cur / v);
            rem = cur % v;
        }
        trim();
        return *this;
    }
    int operator%(int v) const {  // 余数符号跟随被除数；v 取任意 int（含 INT_MIN）
        long long d = v < 0 ? -(long long)v : v, m = 0;
        for (int i = int(a.size()) - 1; i >= 0; --i) m = (a[i] + m * BASE) % d;
        return int(m) * sign;
    }
    friend bigint operator*(bigint x, long long v) { return x *= v; }
    friend bigint operator*(long long v, bigint x) { return x *= v; }
    friend bigint operator/(bigint x, long long v) { return x /= v; }

    // ---------- 大整数乘：拆成 1e6 压位后跑 Karatsuba ----------
    // 拆到 1e6 是为了让 karatsuba 内层 res[i+j] += a[i]*b[j] 累加不爆 long long
    static vector<int> convertBase(const vector<int> &v, int oldW, int newW) {
        vector<long long> p(max(oldW, newW) + 1);
        p[0] = 1;
        for (int i = 1; i < int(p.size()); ++i) p[i] = p[i - 1] * 10;
        vector<int> res;
        long long cur = 0;
        int curW = 0;
        for (int x : v) {
            cur += x * p[curW];
            curW += oldW;
            while (curW >= newW) {
                res.push_back(int(cur % p[newW]));
                cur /= p[newW];
                curW -= newW;
            }
        }
        res.push_back(int(cur));
        while (!res.empty() && res.back() == 0) res.pop_back();
        return res;
    }
    static vector<long long> karatsuba(const vector<long long> &x,
                                       const vector<long long> &y) {
        int n = int(x.size());
        vector<long long> res(n + n);
        if (n <= 32) {  // 小规模直接暴力，常数远小于递归
            for (int i = 0; i < n; ++i)
                for (int j = 0; j < n; ++j) res[i + j] += x[i] * y[j];
            return res;
        }
        int k = n >> 1;
        vector<long long> x1(x.begin(), x.begin() + k),
            x2(x.begin() + k, x.end()),
            y1(y.begin(), y.begin() + k),
            y2(y.begin() + k, y.end());
        vector<long long> x1y1 = karatsuba(x1, y1);
        vector<long long> x2y2 = karatsuba(x2, y2);
        for (int i = 0; i < k; ++i) x2[i] += x1[i], y2[i] += y1[i];
        vector<long long> r = karatsuba(x2, y2);  // (x1+x2)(y1+y2) - x1y1 - x2y2
        for (int i = 0; i < int(x1y1.size()); ++i) r[i] -= x1y1[i];
        for (int i = 0; i < int(x2y2.size()); ++i) r[i] -= x2y2[i];
        for (int i = 0; i < int(r.size()); ++i) res[i + k] += r[i];
        for (int i = 0; i < int(x1y1.size()); ++i) res[i] += x1y1[i];
        for (int i = 0; i < int(x2y2.size()); ++i) res[i + n] += x2y2[i];
        return res;
    }
    friend bigint operator*(const bigint &x, const bigint &y) {
        vector<int> x6 = convertBase(x.a, W, 6), y6 = convertBase(y.a, W, 6);
        vector<long long> p(x6.begin(), x6.end()), q(y6.begin(), y6.end());
        while (p.size() < q.size()) p.push_back(0);
        while (q.size() < p.size()) q.push_back(0);
        while (p.size() & (p.size() - 1)) p.push_back(0), q.push_back(0);  // 补到 2 的幂
        vector<long long> c = karatsuba(p, q);
        bigint r;
        r.sign = x.sign * y.sign;
        long long carry = 0;  // 必须 long long：c[i] 量级可达 n*1e12，进位早就爆 int
        for (int i = 0; i < int(c.size()); ++i) {
            long long cur = c[i] + carry;
            r.a.push_back(int(cur % 1000000));
            carry = cur / 1000000;
        }
        for (; carry; carry /= 1000000) r.a.push_back(int(carry % 1000000));
        r.a = convertBase(r.a, 6, W);
        r.trim();
        return r;
    }

    // ---------- 大整数除：Knuth 归一化 + 逐位试商 ----------
    // 返回 {商, 余数}；商向零取整，余数符号跟随被除数。除数为 0 是 UB，调用方自己保证
    friend pair<bigint, bigint> divmod(const bigint &x, const bigint &y) {
        int norm = BASE / (y.a.back() + 1);  // 归一化让最高位 >= BASE/2，试商误差 <= 2
        bigint u = x.abs() * norm, v = y.abs() * norm, q, r;
        q.a.assign(u.a.size(), 0);
        for (int i = int(u.a.size()) - 1; i >= 0; --i) {
            // r = r * BASE + u[i]：把被除数的下一位拽下来。
            // 直接在低位插一格是 O(n) 的 memmove，写成 r = r * BASE + u.a[i] 会慢一个数量级
            r.a.insert(r.a.begin(), u.a[i]);
            r.trim();
            int m = int(v.a.size());
            int s1 = int(r.a.size()) <= m ? 0 : r.a[m];
            int s2 = int(r.a.size()) <= m - 1 ? 0 : r.a[m - 1];
            int d = int((1LL * BASE * s1 + s2) / v.a.back());  // 试商，至多偏大 2
            r = r - v * d;
            for (; r < 0; --d) r += v;
            q.a[i] = d;
        }
        q.sign = x.sign * y.sign;
        r.sign = x.sign;
        q.trim();
        r.trim();
        return {q, r / norm};
    }
    friend bigint operator/(const bigint &x, const bigint &y) {
        return divmod(x, y).first;
    }
    friend bigint operator%(const bigint &x, const bigint &y) {
        return divmod(x, y).second;
    }

    bigint &operator+=(const bigint &v) { return *this = *this + v; }
    bigint &operator-=(const bigint &v) { return *this = *this - v; }
    bigint &operator*=(const bigint &v) { return *this = *this * v; }
    bigint &operator/=(const bigint &v) { return *this = *this / v; }
    bigint &operator%=(const bigint &v) { return *this = *this % v; }

    friend bigint gcd(bigint x, bigint y) {  // 写成迭代，避免大数递归爆栈
        x = x.abs(), y = y.abs();
        while (!y.isZero()) { bigint t = x % y; x = y, y = t; }
        return x;
    }
    friend bigint lcm(const bigint &x, const bigint &y) {
        return (x / gcd(x, y) * y).abs();
    }

    // ---------- 读写 ----------
    void read(const string &s) {
        sign = 1;
        a.clear();
        int pos = 0;
        while (pos < int(s.size()) && (s[pos] == '-' || s[pos] == '+')) {
            if (s[pos] == '-') sign = -sign;
            ++pos;
        }
        for (int i = int(s.size()) - 1; i >= pos; i -= W) {  // 从低位往高位每 W 个十进制位打一包
            int x = 0;
            for (int j = max(pos, i - W + 1); j <= i; ++j) x = x * 10 + (s[j] - '0');
            a.push_back(x);
        }
        trim();
    }
    string str() const {  // 不用 setw/setfill：那会把 cout 的 fill 字符永久改成 '0'
        string s = sign < 0 ? "-" : "";
        s += a.empty() ? "0" : to_string(a.back());
        for (int i = int(a.size()) - 2; i >= 0; --i) {
            string t = to_string(a[i]);
            s += string(W - t.size(), '0') + t;
        }
        return s;
    }
    friend istream &operator>>(istream &is, bigint &v) {
        string s;
        is >> s;
        v.read(s);
        return is;
    }
    friend ostream &operator<<(ostream &os, const bigint &v) {
        return os << v.str();
    }
};
// ======== 模板代码到此为止，下面是对拍脚手架 ========

mt19937_64 rng(20260821);
ll rnd(ll l, ll r) { return l + (ll)(rng() % (unsigned long long)(r - l + 1)); }

string i128str(i128 x) {
    if (x == 0) return "0";
    bool neg = x < 0;
    // 先转 unsigned 再取反，避免 x = -2^127 溢出
    unsigned __int128 u = neg ? -(unsigned __int128)x : (unsigned __int128)x;
    string s;
    for (; u; u /= 10) s += char('0' + int(u % 10));
    if (neg) s += '-';
    reverse(s.begin(), s.end());
    return s;
}

int failed = 0;
void expect(bool ok, const string &what) {
    if (!ok) {
        ++failed;
        cerr << "FAIL: " << what << "\n";
        if (failed > 20) { cerr << "too many failures\n"; exit(1); }
    }
}

// 独立于 Karatsuba 的 O(n^2) 竖式乘法（只做绝对值，直接在十进制串上做）
string naiveMul(const string &x, const string &y) {
    int n = (int)x.size(), m = (int)y.size();
    vector<int> c(n + m, 0);
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < m; ++j) c[i + j] += (x[n - 1 - i] - '0') * (y[m - 1 - j] - '0');
    string s;
    int carry = 0;
    for (int i = 0; i < n + m; ++i) {
        int cur = c[i] + carry;
        s += char('0' + cur % 10);
        carry = cur / 10;
    }
    for (; carry; carry /= 10) s += char('0' + carry % 10);
    while (s.size() > 1 && s.back() == '0') s.pop_back();
    reverse(s.begin(), s.end());
    return s;
}

string randDigits(int len) {  // 长度恰为 len、无前导零的十进制串
    string s;
    s += char('1' + rnd(0, 8));
    for (int i = 1; i < len; ++i) s += char('0' + rnd(0, 9));
    return s;
}

// ---------- 1. 与 __int128 逐点比对 ----------
void checkAgainstI128() {
    auto one = [&](i128 A, i128 B) {
        bigint a(i128str(A)), b(i128str(B));
        expect(a.str() == i128str(A), "str roundtrip A=" + i128str(A));
        expect((a + b).str() == i128str(A + B), "add " + i128str(A) + " " + i128str(B));
        expect((a - b).str() == i128str(A - B), "sub " + i128str(A) + " " + i128str(B));
        expect((a * b).str() == i128str(A * B), "mul " + i128str(A) + " " + i128str(B));
        expect((a < b) == (A < B), "lt " + i128str(A) + " " + i128str(B));
        expect((a > b) == (A > B), "gt " + i128str(A) + " " + i128str(B));
        expect((a <= b) == (A <= B), "le " + i128str(A) + " " + i128str(B));
        expect((a >= b) == (A >= B), "ge " + i128str(A) + " " + i128str(B));
        expect((a == b) == (A == B), "eq " + i128str(A) + " " + i128str(B));
        expect((a != b) == (A != B), "ne " + i128str(A) + " " + i128str(B));
        if (B != 0) {
            expect((a / b).str() == i128str(A / B), "div " + i128str(A) + " " + i128str(B));
            expect((a % b).str() == i128str(A % B), "mod " + i128str(A) + " " + i128str(B));
        }
        // 与小整数的快路径：*= /= %= 都要和内建语义一致
        for (ll v : {1LL, -1LL, 7LL, -7LL, 999999999LL, -999999999LL, 1000000000LL, -1000000007LL,
                     (ll)INT_MAX, (ll)INT_MIN}) {
            expect((a * v).str() == i128str(A * v), "mul_ll " + i128str(A) + " " + to_string(v));
            expect((v * a).str() == i128str(A * v), "ll_mul " + i128str(A) + " " + to_string(v));
            expect((a / v).str() == i128str(A / v), "div_ll " + i128str(A) + " " + to_string(v));
            if (v >= INT_MIN && v <= INT_MAX)
                expect(a % (int)v == (int)(A % v), "mod_int " + i128str(A) + " " + to_string(v));
        }
        // gcd / lcm
        if (!(A == 0 && B == 0)) {
            bigint g = gcd(a, b);
            expect(g >= 0, "gcd nonneg");
            if (!g.isZero()) {
                expect((a % g).isZero() && (b % g).isZero(), "gcd divides");
                expect((gcd(a, b) * lcm(a, b)).str() == (a * b).abs().str(), "gcd*lcm");
            }
        }
    };
    // 小数值穷举一圈：符号 / 零 / 进位边界（BASE 附近）全覆盖
    vector<i128> small;
    for (int v = -40; v <= 40; ++v) small.push_back(v);
    for (ll b : {999999998LL, 999999999LL, 1000000000LL, 1000000001LL, 1000000000000000000LL})
        small.push_back(b), small.push_back(-b), small.push_back(b - 1), small.push_back(b + 1);
    for (i128 A : small)
        for (i128 B : small) one(A, B);
    // 随机 128 位量级
    for (int it = 0; it < 3000; ++it) {
        auto pick = [&]() {
            // 上界 2^60：保证 A*B 与 A*v 都不会把 __int128 撑爆
            i128 v = (i128)(rng() % (1ULL << rnd(1, 60)));
            return rnd(0, 1) ? v : -v;
        };
        one(pick(), pick());
    }
}

// ---------- 2. 与竖式乘法比对 + 3. 代数恒等式 ----------
void checkBig() {
    for (int it = 0; it < 200; ++it) {
        int la = (int)rnd(1, 400), lb = (int)rnd(1, 400);
        string sa = randDigits(la), sb = randDigits(lb);
        bigint a(sa), b(sb);
        expect((a * b).str() == naiveMul(sa, sb), "karatsuba vs naive " + to_string(la) + "x" + to_string(lb));
        int sgA = rnd(0, 1) ? 1 : -1, sgB = rnd(0, 1) ? 1 : -1;
        if (sgA < 0) a = -a;
        if (sgB < 0) b = -b;
        bigint c(randDigits((int)rnd(1, 400)));
        // 除法与取模的定义式
        auto [q, r] = divmod(a, b);
        expect(q * b + r == a, "divmod identity");
        expect(r.abs() < b.abs(), "|r| < |b|");
        expect(r.isZero() || (r < 0) == (a < 0), "sign of r follows a");
        expect(a / b == q && a % b == r, "operator/ % agree with divmod");
        // 环公理
        expect(a * b == b * a, "mul comm");
        expect(a * (b + c) == a * b + a * c, "distributive");
        expect((a * b) / b == a, "(a*b)/b == a");
        expect((a * b) % b == 0, "(a*b)%b == 0");
        expect(a + b - b == a, "a+b-b == a");
        expect(-(-a) == a, "double neg");
        expect((a - a).isZero() && (a - a).str() == "0", "a-a == 0");
        // gcd / lcm
        bigint g = gcd(a, b);
        expect((a % g).isZero() && (b % g).isZero(), "big gcd divides");
        expect(gcd(a, b) * lcm(a, b) == (a * b).abs(), "big gcd*lcm");
    }
}

// ---------- 4. 零、前导零、正负号输入等边界 ----------
void checkEdge() {
    expect(bigint().str() == "0", "default ctor is 0");
    expect(bigint(0).str() == "0", "bigint(0)");
    expect(bigint("-0").str() == "0", "-0 归一成 0");
    expect(bigint("0000").str() == "0", "全零串");
    expect(bigint("+0000123").str() == "123", "前导零 + 正号");
    expect(bigint("-0000123").str() == "-123", "前导零 + 负号");
    expect((bigint(0) - bigint(5)).str() == "-5", "0-5");
    expect((bigint(5) * bigint(0)).str() == "0", "5*0");
    expect((bigint(0) * bigint(0)).str() == "0", "0*0");
    expect((bigint(0) / bigint(7)).str() == "0", "0/7");
    expect((bigint(0) % bigint(7)).str() == "0", "0%7");
    expect((-bigint(0)).str() == "0", "-0 不出现负号");
    expect(bigint(LLONG_MIN).str() == "-9223372036854775808", "LLONG_MIN");
    expect(bigint(LLONG_MAX).str() == "9223372036854775807", "LLONG_MAX");
    expect((bigint(1) * -3).str() == "-3", "乘负的小整数（原版 check(v) 传值在这里翻车）");
    expect((bigint(-1) * -3).str() == "3", "负 * 负");
    expect((bigint(100) / -7).str() == "-14", "除负的小整数，向零取整");
    expect((bigint(-100) / 7).str() == "-14", "负数除法向零取整");
    expect(bigint(-100) % 7 == -2, "负数取模跟随被除数");
    expect(bigint(100) % -7 == 2, "模数取绝对值");
    // 进位链：99...9 + 1 == 10...0
    for (int len : {8, 9, 10, 17, 18, 19, 100}) {
        bigint x(string(len, '9'));
        expect((x + 1).str() == "1" + string(len, '0'), "carry chain " + to_string(len));
        expect((x + 1 - 1).str() == string(len, '9'), "borrow chain " + to_string(len));
    }
    // 大幂：2^1000 的低几位与十进制长度
    bigint p(1);
    for (int i = 0; i < 1000; ++i) p *= 2;
    expect(p.str().size() == 302, "2^1000 有 302 位");
    expect(p.str().substr(0, 10) == "1071508607", "2^1000 高 10 位");
    expect(p % 1000000007 == 688423210, "2^1000 mod 1e9+7");
    // 阶乘 500!，尾零个数 = floor(500/5)+floor(500/25)+... = 124
    bigint f(1);
    for (int i = 2; i <= 500; ++i) f *= i;
    string fs = f.str();
    int z = 0;
    while (fs[fs.size() - 1 - z] == '0') ++z;
    expect(z == 124, "500! 末尾 124 个零");
    // cout 的 fill 字符不能被 operator<< 污染（原版用 setfill('0') 且不还原）
    ostringstream os;
    os << bigint("1000000001") << ' ' << setw(4) << 7;
    expect(os.str() == "1000000001    7", "operator<< 不留下 setfill 副作用: [" + os.str() + "]");
    // 流读入
    istringstream is("-1234567890123456789012345678901234567890 0");
    bigint u, v;
    is >> u >> v;
    expect(u.str() == "-1234567890123456789012345678901234567890" && v.str() == "0", "operator>>");
}

// ---------- 5. 规模 / 计时 ----------
void checkScale() {
    // 1.2 万位：还能跑竖式，用来卡住 Karatsuba 的合并进位（此规模下进位已超 int）
    {
        string sa = randDigits(12000), sb = randDigits(12000);
        bigint a(sa), b(sb);
        expect((a * b).str() == naiveMul(sa, sb), "12000 位: Karatsuba vs 竖式");
    }
    // 6 万位：只查恒等式并计时
    string sa = randDigits(60000), sb = randDigits(60000);
    bigint a(sa), b(sb);
    auto t0 = chrono::steady_clock::now();
    bigint c = a * b;
    auto t1 = chrono::steady_clock::now();
    bigint q = c / b;
    auto t2 = chrono::steady_clock::now();
    expect(q == a, "60000 位: (a*b)/b == a");
    cerr << "  60000 位乘法 " << chrono::duration_cast<chrono::milliseconds>(t1 - t0).count()
         << " ms, 12 万位除以 6 万位 "
         << chrono::duration_cast<chrono::milliseconds>(t2 - t1).count() << " ms\n";
}

int main() {
    cerr << "[1/5] 与 __int128 逐点比对 ...\n"; checkAgainstI128();
    cerr << "[2/5] 边界 ...\n";                 checkEdge();
    cerr << "[3/5] Karatsuba vs 竖式 + 代数恒等式 ...\n"; checkBig();
    cerr << "[4/5] 规模与计时 ...\n";           checkScale();
    cerr << "[5/5] done\n";
    if (failed) { cerr << failed << " checks FAILED\n"; return 1; }
    cerr << "all checks passed\n";
    return 0;
}
