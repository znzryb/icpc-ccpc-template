// teamname: Gospel_rock
/**
 * Problem: Prime Test
 * Contest: POJ 1811 / OpenJ_Bailian 1811
 * Judge: Virtual Judge
 * URL: https://vjudge.net/problem/OpenJ_Bailian-1811
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 *
 * Miller-Rabin + Pollard's Rho 模板题：
 *   T 组数据，每组一个 N (2 ≤ N < 2^54)，质数输出 "Prime"，
 *   否则输出 N 的最小质因子。
 *
 * 用 factor 把 N 拆成质因子列表，sort 取最小；若只有一项且等于 N 即素数。
 */

#include <bits/stdc++.h>

using namespace std;

using u64 = unsigned long long;
using i64 = long long;

// 浮点估商 mulmod：避免 __int128 硬件 DIV，x86-64 上比 (u128)a*b%m 快约 2x
// 要求 n < 2^63；long double 80-bit 尾数对 quotient 估计误差 ≤ ±1，最后两次修正
u64 mulmod(u64 a, u64 b, u64 m) {
	u64 q = (long double)a * b / m;
	i64 r = (i64)(a * b - m * q);
	return r < 0 ? r + m : ((u64)r >= m ? r - m : r);
}
u64 powmod(u64 a, u64 b, u64 m) {
	u64 r = 1 % m; a %= m;
	for (; b; b >>= 1, a = mulmod(a, a, m)) if (b & 1) r = mulmod(r, a, m);
	return r;
}
bool is_prime(u64 n) {
	if (n < 2) return false;
	for (u64 p : {2,3,5,7,11,13,17,19,23,29,31,37})
		if (n % p == 0) return n == p;
	int s = __builtin_ctzll(n - 1);
	u64 d = (n - 1) >> s;
	for (u64 a : {2ULL,325ULL,9375ULL,28178ULL,450775ULL,9780504ULL,1795265022ULL}) {
		u64 x = powmod(a % n, d, n);
		if (x <= 1 || x == n - 1) continue;
		bool composite = true;
		for (int i = 0; i < s - 1; ++i) {
			x = mulmod(x, x, n);
			if (x == n - 1) { composite = false; break; }
		}
		if (composite) return false;
	}
	return true;
}
// Pollard's Rho: f(x)=x^2+1, Floyd 双指针，每 256 步 gcd 一次；
// 撞环（x==y）用计数器 tot++ 重启 x，不用随机库；
// q==0 时保留旧 p 继续，避免重启
u64 pollard(u64 n) {
	for (u64 p : {2,3,5,7,11,13,17,19,23}) if (n % p == 0) return p;
	static u64 tot = 0;
	auto f = [&](u64 x) { x = mulmod(x, x, n) + 1; return x >= n ? x - n : x; };
	u64 x = 0, y = 0, p = 1, q, g;
	for (int i = 0; (i & 255) || (g = __gcd(p, n)) == 1; ++i, x = f(x), y = f(f(y))) {
		if (x == y) { x = tot++; y = f(x); }
		q = mulmod(p, x > y ? x - y : y - x, n);
		if (q) p = q;
	}
	return g;
}
void factor(u64 n, vector<u64>& res) {
	if (n < 2) return;
	if (is_prime(n)) { res.push_back(n); return; }
	u64 d = pollard(n);
	factor(d, res); factor(n / d, res);
}

int main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);

	int T; cin >> T;
	while (T--) {
		u64 n; cin >> n;
		if (is_prime(n)) {
			cout << "Prime\n";
		} else {
			vector<u64> fs;
			factor(n, fs);
			cout << *max_element(fs.begin(), fs.end()) << '\n';
		}
	}
	return 0;
}
