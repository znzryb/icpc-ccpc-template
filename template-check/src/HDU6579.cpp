// teamname: Gospel_rock
/**
 * Problem: Operation
 * Contest: HDU 6579 (2019 Multi-University Training Contest 1)
 * Judge: Virtual Judge
 * URL: https://vjudge.net/problem/HDU-6579
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 *
 * 区间异或线性基模板题：
 *   op 0 l r：求 a[l..r] 中任取若干元素异或的最大值（强制在线）
 *   op 1 x ：在数组末尾追加 x（强制在线）
 *
 * 解法：前缀线性基 + 「贪心保留靠右元素」
 *   对每个前缀 [1..i] 维护一个线性基 L[i]，插入时若当前位上已经有基底
 *   且其下标比新元素小，就把新元素「换上去」、把旧基底继续往低位消。
 *   这样 L[r] 中的每个基底都尽可能用下标靠右的元素表达。查询 [l, r] 时，
 *   只取 L[r] 中 pos[i] >= l 的基底贪心即可。
 */

#include <bits/stdc++.h>

using namespace std;

using ll = long long;
using ull = unsigned long long;

ll lT, testcase;

/*
 * 前缀线性基：支持「末尾追加 + 区间子集异或最大值」
 * 元素值域上限 2^BITS - 1
 */
struct PrefixLinearBasis {
	static constexpr int BITS = 30;
	using T = unsigned;

	struct Layer {
		T   d  [BITS] = {};   // d[i]：最高位为 i 的基底
		int pos[BITS] = {};   // pos[i]：当前 d[i] 对应的元素下标（1-indexed）
	};

	vector<Layer> L;          // L[k]：a[1..k] 的前缀线性基

	PrefixLinearBasis() { L.emplace_back(); }     // L[0] 为空基
	void reserve(int n) { L.reserve(n + 1); }
	void clear()        { L.clear(); L.emplace_back(); }

	int size() const { return (int)L.size() - 1; }   // 已插入元素个数

	// 末尾追加 val，自动成为 a[size()+1]
	void append(T val) {
		Layer cur = L.back();
		int idx   = (int)L.size();    // 新元素的下标
		for (int i = BITS - 1; i >= 0; --i) {
			if (!((val >> i) & 1)) continue;
			if (!cur.d[i]) {           // 该位空，落位即可
				cur.d[i] = val;
				cur.pos[i] = idx;
				break;
			}
			// 让 d[i] 始终保留下标更靠右的那一对
			if (cur.pos[i] < idx) {
				swap(cur.d[i], val);
				swap(cur.pos[i], idx);
			}
			val ^= cur.d[i];
		}
		L.push_back(cur);
	}

	// a[l..r] 子集异或最大值（1-indexed，l <= r）
	T query_max(int l, int r) const {
		T ans = 0;
		const Layer& b = L[r];
		for (int i = BITS - 1; i >= 0; --i) {
			if (b.pos[i] < l) continue;          // 这条基底用到了 l 之前的元素，跳过
			if ((ans ^ b.d[i]) > ans) ans ^= b.d[i];
		}
		return ans;
	}
};

/*
 * 2026-04-29: AC https://vjudge.net/solution/69555764
 */
struct Solve {
	int n, m;

	Solve() {
		cin >> n >> m;
		PrefixLinearBasis lb;
		lb.reserve(n + m);

		for (int i = 1; i <= n; ++i) {
			unsigned x; cin >> x;
			lb.append(x);
		}

		unsigned last = 0;
		while (m--) {
			int op; cin >> op;
			if (op == 0) {
				unsigned l, r; cin >> l >> r;
				int N = lb.size();
				int L = (int)((l ^ last) % N + 1);
				int R = (int)((r ^ last) % N + 1);
				if (L > R) swap(L, R);
				last = lb.query_max(L, R);
				cout << last << '\n';
			} else {
				unsigned x; cin >> x;
				lb.append(x ^ last);
			}
		}
	}
};

signed main() {
	ios::sync_with_stdio(false);
	cin.tie(nullptr);
	cout.tie(nullptr);
#ifdef LOCAL
	cout.setf(ios::unitbuf);
#endif

	cin >> lT;
	for (testcase = 1; testcase <= lT; ++testcase)
		Solve solve;
	return 0;
}
