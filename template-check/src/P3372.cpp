// teamname: Gospel_rock
/**
 * Problem: P3372 【模板】线段树 1
 * Judge: Luogu
 * URL: https://www.luogu.com.cn/problem/P3372
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 *
 * 用 BIT（区间加 + 区间求和版本）通过线段树模板题
 */

#include <bits/stdc++.h>

using namespace std;

using ll = long long;

struct BIT {
	// 支持区间修改，区间查询
	// 维护两个差分：d[i] 和 i*d[i]
	// 前缀和 sum(p) = (p+1)*sum(d[1..p]) - sum(i*d[i], 1..p)
	vector<ll> tr1, tr2;
	int n;

	ll lowbit(ll x) {
		return x & (-x);
	}

	BIT(int n_) {
		n = n_;
		tr1.assign(n + 1, 0);
		tr2.assign(n + 1, 0);
	}

	void add(vector<ll> &tr, ll p, ll x) {
		while (p <= n) {
			tr[p] += x;
			p += lowbit(p);
		}
	}

	ll sum(vector<ll> &tr, ll p) {
		ll ret = 0;
		while (p) {
			ret += tr[p];
			p -= lowbit(p);
		}
		return ret;
	}

	// 单点加
	void add(ll p, ll x) {
		add(tr1, p, x);
		add(tr2, p, p * x);
	}

	// 区间加，[l, r] 每个位置加 x
	void add(ll l, ll r, ll x) {
		add(tr1, l, x);
		add(tr1, r + 1, -x);
		add(tr2, l, l * x);
		add(tr2, r + 1, -(r + 1) * x);
	}

	// 前缀和 a[1] + a[2] + ... + a[p]
	ll query(ll p) {
		return (p + 1) * sum(tr1, p) - sum(tr2, p);
	}

	// 区间和 a[l] + ... + a[r]
	ll query(ll l, ll r) {
		return query(r) - query(l - 1);
	}
};

signed main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);

	int n, m;
	cin >> n >> m;

	BIT bit(n);
	for (int i = 1; i <= n; ++i) {
		ll x;
		cin >> x;
		// 初始值用 [i,i] 的区间加实现，避免把 add 的两种语义混起来
		bit.add(i, i, x);
	}

	while (m--) {
		int op;
		cin >> op;
		if (op == 1) {
			ll l, r, x;
			cin >> l >> r >> x;
			bit.add(l, r, x);
		} else {
			ll l, r;
			cin >> l >> r;
			cout << bit.query(l, r) << '\n';
		}
	}
	return 0;
}
