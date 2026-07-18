// teamname: Gospel_rock
/**
 * Problem: XOR
 * Contest: HDU
 * Judge: Virtual Judge
 * URL: https://vjudge.net/problem/HDU-3949
 * Created: 2026-04-06 09:30:36
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
 *
 */

/*
 * 2026-04-06: AC https://vjudge.net/solution/69022982
 * 主要是测试了 rebuild 和 query-kth
 * 2026-04-06: AC https://www.luogu.com.cn/record/272234814
 * 主要测试了 merge 功能和这个 query_max 功能
 */
struct LinearBasis {
	static const int MAXL = 60;
	vector<long long> d;
	vector<long long> p; // 正交化后的基底，用于求第 k 小
	int cnt; // 线性基内元素的数量（秩）
	bool zero; // 原数组是否能异或出 0

	LinearBasis() {
		d.assign(MAXL + 1, 0);
		cnt = 0;
		zero = false;
	}

	// 向极大线性无关组中添加新向量
	bool insert(long long val) {
		// 从高到低遍历二进制位
		for (int i = MAXL; i >= 0; i--) {
			// 若当前数字的第 i 位为 1
			if (val & (1LL << i)) {
				// 若不存在该位的基底，则作为新基底存入，插入成功
				if (!d[i]) {
					d[i] = val;
					cnt++;
					return true;
				}
				// 若已存在该位基底，则异或消去该位的 1
				val ^= d[i];
			}
		}
		// 若最终数字变为 0，说明它可由已有基底线性表出
		zero = true;
		return false;
	}

	// 查询能异或出的最大值
	long long query_max() {
		long long res = 0;
		// 基于最高位独占特性的贪心，从高位到低位遍历
		for (int i = MAXL; i >= 0; i--) {
			// 若当前答案异或基底 d[i] 后能变大，则必然选择异或
			if ((res ^ d[i]) > res) {
				res ^= d[i];
			}
		}
		return res;
	}

	// 查询能异或出的最小值
	long long query_min() {
		// 若在插入时出现过能被线性表出的数字（即能异或出 0）
		if (zero) return 0;
		// 否则，线性基数组中最小的非零基底就是能异或出的最小值
		for (int i = 0; i <= MAXL; i++) {
			if (d[i]) return d[i];
		}
		return 0;
	}

	// 重构线性基，通过类似高斯消元化为简化阶梯型矩阵
	void rebuild() {
		// 使所有基底互不干扰，达到完全正交状态
		for (int i = MAXL; i >= 0; i--) {
			for (int j = i - 1; j >= 0; j--) {
				// 如果 d[i] 的第 j 位为 1，则用 d[j] 消去这一位
				if (d[i] & (1LL << j)) {
					d[i] ^= d[j];
				}
			}
		}
		p.clear();
		// 将正交化后的非零基底提取出来，方便后续按字典序查询
		for (int i = 0; i <= MAXL; i++) {
			if (d[i]) p.push_back(d[i]);
		}
	}

	// 查询去重后的第 k 小值（要求先调用 rebuild）
	long long query_kth(long long k) {
		// 如果原集合能异或出 0，并且 0 算第一小，先扣除 0 的排名
		if (zero) k--;
		if (k == 0) return 0;
		// 若查询排名超过了能异或出的总个数（2^cnt - 1 个非零值）
		if (k >= (1LL << cnt)) return -1;

		long long res = 0;
		// 将排名 k 转化为二进制表示
		for (int i = 0; i < (int)p.size(); i++) {
			// 若第 i 位为 1，则异或上正交化后的第 i 小的非零基底
			if (k & (1LL << i)) res ^= p[i];
		}
		return res;
	}

	// 合并另一个线性基
	void merge(const LinearBasis& other) {
		for (int i = 0; i <= MAXL; i++) {
			// 将另一个线性基的非零基底全部插入当前线性基
			if (other.d[i]) insert(other.d[i]);
		}
		zero |= other.zero;
	}
};

struct Solve {
    ll N;

    Solve() {
        cin >> N;
        LinearBasis lb;
        for (int i = 0; i < N; ++i) {
            long long x;
            cin >> x;
            lb.insert(x);
        }
        lb.rebuild();
        
        cout << "Case #" << testcase << ":\n";
        
        int Q;
        cin >> Q;
        while (Q--) {
            long long k;
            cin >> k;
            cout << lb.query_kth(k) << "\n";
        }
    }
};

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    cout.tie(nullptr);
#ifdef LOCAL
    cout.setf(ios::unitbuf); // 无缓冲流，方便我们调试
#endif

    cin >> lT;
    for (testcase = 1; testcase <= lT; ++testcase)
        Solve solve;
    return 0;
}

/*

*/
