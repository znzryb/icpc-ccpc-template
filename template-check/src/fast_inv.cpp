// teamname: Gospel_rock
// exemption-probe-should-pass
/**
 * Problem: Online Multiplicative Inverse Query
 * Contest: XXII 赛前模板训练赛
 * Judge: QOJ
 * URL: https://qoj.ac/contest/2646/problem/5
 * Created: 2026-04-10
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 *
 * Powered by AutoCp https://github.com/Pushpavel/AutoCp
 */

#include <bits/stdc++.h>
#define all(vec) vec.begin(),vec.end()
#define lson(o) (o<<1)
#define rson(o) (o<<1|1)
#define SZ(a) ((long long) a.size())
#define debug(var) cerr << #var <<" = ["<<var<<"]"<<"\n";
#define debug1d(a)    \
cerr << #a << " = [";   \
for (int i = 0; i < (int)(a).size(); i++) \
cerr << (i ? ", " : "") << a[i]; \
cerr << "]\n";
#define debug2d(a)  \
cerr << #a << " = [\n";  \
for (int i = 0; i < (int)(a).size(); i++)  \
{   \
cerr << "  [";  \
for (int j = 0; j < (int)(a[i]).size(); j++) \
cerr << (j ? ", " : "") << a[i][j];   \
cerr << "]\n";   \
}  \
cerr << "]\n";
#define cend cerr<<"\n-----------\n"
#define fsp(x) fixed<<setprecision(x)

using namespace std;

using ll = long long;
using ull = unsigned long long;
using DB = double;
using i128 = __int128;
using CD = complex<double>;

static constexpr ll MAXN = (ll) 1e6 + 10, INF = (1ll << 61) - 1;
static constexpr ll mod = 998244353; // (ll)1e9+7;
static constexpr double eps = 1e-8;
const double PI = acos(-1.0);

ll lT, testcase;

/*
 * O(1) 在线模逆元查询（Stern-Brocot 树 + 小值逆元表）
 * 预处理：O(2^21) 时间空间（约 20MB）
 * 查询：O(1)
 * 约束：mod 为质数，2*(1<<20) < mod < (1<<30)
 *       覆盖 998244353 和 1e9+7
 *
 * 原理：
 *   对于 x ∈ [1, mod-1]，用 Stern-Brocot 树找分数 a/b (b<2048)
 *   使得 a/b ≈ x/mod。令 r = x*b - a*mod，则 |r| ≤ 2*(1<<20)。
 *   由 x*b ≡ r (mod mod) 得 x^{-1} ≡ b * r^{-1} (mod mod)，
 *   其中 r^{-1} 由小值逆元表 O(1) 查找。
 */

constexpr bool is_prime_constexpr(int n) {
	if (n == 998244353 || n == 1000000007) return true;
	for (int i = 2; (ll) i * i <= n; i++)
		if (n % i == 0) return false;
	return true;
}

template<int mod>
struct FastInv {
	static_assert(2 * (1 << 20) < mod && mod < (1 << 30));
	static_assert(is_prime_constexpr(mod));

	// Stern-Brocot 树构建分数近似表
	static vector<pair<uint16_t, uint16_t>> build_frac() {
		vector<pair<uint16_t, uint16_t>> res(1 << 20);
		array<tuple<int, int, int, int>, 2048> st;
		int p = 0;
		st[p++] = {0, 1, 1, 1};
		while (p) {
			auto [a, b, c, d] = st[--p];
			if (b + d < 2048) {
				st[p++] = {a + c, b + d, c, d};
				st[p++] = {a, b, a + c, b + d};
			} else {
				int s = (ll) a * mod / (1024 * b);
				int t = (ll) c * mod / (1024 * d);
				res[s] = {(uint16_t) a, (uint16_t) b};
				res[t] = {(uint16_t) c, (uint16_t) d};
				if (a > c) a = c;
				if (b > d) b = d;
				for (int i = s + 1; i < t; i++)
					res[i] = {(uint16_t) a, (uint16_t) b};
			}
		}
		return res;
	}

	// 线性递推求小值逆元（同时覆盖正负值）
	static vector<uint32_t> build_sinv() {
		constexpr int M = 2 * (1 << 20);
		vector<uint32_t> res(4 * (1 << 20) + 1);
		res[M + 1] = 1;
		res[M - 1] = mod - 1;
		for (int i = 2; i <= M; i++) {
			uint32_t x = (ll) (mod - res[M + (mod % i)]) * (mod / i) % mod;
			res[M + i] = x;
			res[M - i] = mod - x;
		}
		return res;
	}

	inline static vector<pair<uint16_t, uint16_t>> frac = build_frac();
	inline static vector<uint32_t> sinv = build_sinv();

	static int inv(unsigned x) {
		auto [a, b] = frac[x >> 10];
		return (ll) sinv[2 * (1 << 20) + x * b - (unsigned) a * mod] * b % mod;
	}
};

// QOJ problem interface
void init(int) {}

int inv(int x) {
	return FastInv<998244353>::inv(x);
}

#ifndef ONLINE_JUDGE
signed main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);
#ifdef LOCAL
	cout.setf(ios::unitbuf);
#endif

	constexpr int MOD = 998244353;
	init(MOD);

	// 逐一验证 [1, 10^6] 的逆元正确性
	for (int x = 1; x <= 1000000; x++) {
		ll ans = inv(x);
		assert(ans * x % MOD == 1);
	}
	cerr << "All " << 1000000 << " tests passed!\n";

	// 随机大值验证
	mt19937 rng(42);
	for (int i = 0; i < 1000000; i++) {
		int x = rng() % (MOD - 1) + 1;
		ll ans = inv(x);
		assert(ans * x % MOD == 1);
	}
	cerr << "Random tests passed!\n";
	return 0;
}
#endif
