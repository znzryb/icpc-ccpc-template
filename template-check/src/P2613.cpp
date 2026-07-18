// teamname: Gospel_rock
/**
 * Problem: 【模板】有理数取余
 * Judge: Luogu
 * URL: https://www.luogu.com.cn/problem/P2613
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 */

#include <bits/stdc++.h>

using namespace std;

using ll = long long;

static constexpr ll mod = 19260817;

void exgcd(ll a, ll b, ll &x, ll &y) {
	if (!b) { x = 1; y = 0; return; }
	exgcd(b, a % b, y, x);
	y -= a / b * x;
}

ll inv_exgcd(ll a, ll m) {
	ll x = 0, y = 0;
	exgcd(a % m, m, x, y);
	x %= m;
	if (x < 0) x += m;
	return x;
}

// 边读边取模，处理 10^10001 级别的大整数
ll read_mod() {
	char c = getchar();
	ll res = 0;
	while (!isdigit(c) && c != EOF) c = getchar();
	while (isdigit(c)) {
		res = (res * 10 + c - '0') % mod;
		c = getchar();
	}
	return res;
}

signed main() {
	ll a = read_mod();
	ll b = read_mod();
	if (b == 0) {
		puts("Angry!");
		return 0;
	}
	printf("%lld\n", a % mod * inv_exgcd(b, mod) % mod);
	return 0;
}
