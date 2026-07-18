// by Gospel_rock
/**
 * Problem: F. 连线博弈
 * Contest: 第十一届 CCPC 网络预选赛 (2025-09-20)
 * Judge: QOJ
 * Contest URL: https://qoj.ac/contest/2534
 * Submission URL: https://qoj.ac/submission/1387353
 *
 * 模板对应：博弈论 -> SG 函数的暴力打表 -> 实战范例
 *
 * 思路要点：
 *   1) 不相交弦把圆周上的"自由点"切成若干段，每段是独立子游戏；
 *      x 个自由点的子游戏一步操作占 2 个点，把段切成 (a, b)，a+b = x-2。
 *      故 SG(x) = mex_{a+b=x-2} SG(a) ^ SG(b)，SG(0)=SG(1)=0。
 *   2) 暴力打表至 500，发现 x>=36 后周期 34，但 idx==52 是异常项
 *      （sg[52]=2，但 x>=70 落到 idx=52 的实际值应取 sg[86]=9），
 *      故 SG(x) 在 idx==52 时特判 +34 偏移。
 *   3) 划分子游戏的 XOR-Hash 技巧：每条线段 (x, y) 给一个随机权 r，
 *      mp[x] ^= r, mp[y] ^= r；按下标排序扫一遍，维护前缀异或 cur，
 *      cur 相同的下标段 = 同一子游戏（因为弦端点同时翻转 cur 两次，
 *      段内自由点的 cur 等于"包含它的所有未闭合弦"的随机权异或）。
 *      然后对每个 cur 桶累加自由点数 → SG → 异或起来。
 */

#include <bits/stdc++.h>
#include <random>
#define all(vec) vec.begin(), vec.end()
#define PB push_back
#define PF push_front
#define EB emplace_back
#define EF emplace_front
#define EM emplace
#define FI first
#define SE second
#define pct __builtin_popcountll
#define ctz __builtin_ctzll
#define lson(o) (o << 1)
#define rson(o) (o << 1 | 1)
#define SZ(a) ((long long)a.size())
#define FOR(i, a, b) for (int i = (a); i <= (b); ++i)
#define ROF(i, a, b) for (int i = (a); i >= (b); --i)
#define debug(var) cerr << #var << ":" << var << "\n";
#define cend cerr << "\n-----------\n"
#define fsp(x) fixed << setprecision(x)
#define minn(vec) *min_element(vec.begin(), vec.end())
#define maxx(vec) *max_element(vec.begin(), vec.end())
#define yyy cout << "YES\n"
#define nnn cout << "NO\n"

using namespace std;
#ifdef LOCAL
template <typename T> void _print(T x) { cerr << x; }
template <typename T> void _print(vector<T> v) {
    cerr << "[ ";
    for (T i : v)
        _print(i), cerr << " ";
    cerr << "]";
}
template <typename T> void _print(set<T> s) {
    cerr << "{ ";
    for (T i : s)
        _print(i), cerr << " ";
    cerr << "}";
}
template <typename T, typename U> void _print(pair<T, U> p) {
    cerr << "{";
    _print(p.first);
    cerr << ", ";
    _print(p.second);
    cerr << "}";
}
template <typename T, typename U> void _print(map<T, U> m) {
    cerr << "{ ";
    for (auto &p : m)
        _print(p), cerr << " ";
    cerr << "}";
}
template <typename T> void _print(vector<vector<T>> v) {
    cerr << "{\n";
    for (auto i : v)
        _print(i), cerr << "\n";
    cerr << " }";
}
#endif

using ll = long long;
using ull = unsigned long long;
using DB = double;
using LD = long double;
using i128 = __int128;
using pll = pair<ll, ll>;
using vll = vector<ll>;
using vii = vector<int>;
using vpll = vector<pll>;
using vc = vector<char>;
using arr3 = array<ll, 3>;
using arr2 = array<ll, 2>;

static constexpr ll MAXN = (ll)1e6 + 10, INF = (ll)1e17 + 3;
static constexpr ll mod = (ll)1e9 + 7;

ll N, M, lT;

inline void __();

vii sg(1110);

// SG(x)：周期 34（从 x=36 起），但 idx==52 是异常项，
// 实际周期应使用 sg[idx+34]=sg[86]，否则 hack 数据 (n=8, 段 0-1 与 4-5) 会 WA
ll SG(ll x) {
    if (x <= 200) return sg[x];
    ll idx = 36 + (x - 36) % 34;
    if (idx == 52) return sg[idx + 34];
    return sg[idx];
}

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(0);
    cout.tie(0);

    cin >> lT;

    // 预处理 SG 至 500（足够覆盖周期识别 + 异常校验）
    sg[0] = 0;
    sg[1] = 0;
    FOR(i, 2, 500) {
        set<ll> s;
        FOR(j, 0, i - 2) { s.insert(sg[j] ^ sg[i - j - 2]); }
        ll mex = 0;
        while (s.count(mex)) mex++;
        sg[i] = mex;
    }
#ifdef LOCAL
    // 本地校验周期公式：把所有 SG(i) 与 sg[i] 对比，发现异常项 idx==52
    FOR(i, 0, 500) { cerr << i << ":" << sg[i] << "\n"; }
    FOR(i, 201, 500) {
        if (SG(i) != sg[i]) {
            cout << "错误出现在" << i << " SG:" << SG(i) << " sg:" << sg[i] << "\n";
            cout << " 36+(x-36)%34:" << 36 + (i - 36) % 34 << "\n";
        }
    }
#endif

    while (lT--) __();
    return 0;
}

inline void __() {
    cin >> N >> M;
    map<ll, ll> mp;
    mt19937_64 rng(time(0));

    // 关键技巧 1：每条线段两端各异或一个随机权
    FOR(i, 1, M) {
        ll x, y;
        cin >> x >> y;
        ll r = rng();
        mp[x] ^= r;
        mp[y] ^= r;
    }

    // 边界：M==0 直接整圈一个子游戏
    if (M == 0) {
        if (SG(N)) yyy;
        else       nnn;
        return;
    }

    // 关键技巧 2：扫描端点 + 维护前缀异或 cur，cur 相同的段属于同一子游戏
    map<ll, ll> mp2;
    ll up = -1, cur = 0;
    mp[N] = 0; // 末尾哨兵：把最后一段（最大端点 ~ N-1）一并结算
    for (auto &[u, v] : mp) {
        ll num = max(0ll, u - up - 1); // [up+1, u-1] 区间内的自由点数
        mp2[cur] += num;
        up = u;
        cur ^= v;
    }

    // 各子游戏 SG 异或
    ll ans = 0;
    for (auto &[u, v] : mp2) ans ^= SG(v);

    if (ans) yyy;
    else     nnn;
}

/*
AC https://qoj.ac/submission/1387353

Hack 数据（用"两端点之间整段累加"的朴素做法会 WA YES，正确答案 YES）：
1
8 2
0 1
4 5
*/
