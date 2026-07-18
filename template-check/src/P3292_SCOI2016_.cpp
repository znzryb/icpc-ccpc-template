// teamname: Gospel_rock
/**
 * Problem: P3292 [SCOI2016] 幸运数字
 * Contest: 
 * Judge: Luogu
 * URL: https://www.luogu.com.cn/problem/P3292
 * Created: 2026-04-06 09:45:29
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 * 
 * Powered by AutoCp https://github.com/Pushpavel/AutoCp
 */

#include <bits/stdc++.h>
using namespace std;

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
		for (int i = MAXL; i >= 0; i--) {
			if (val & (1LL << i)) {
				if (!d[i]) {
					d[i] = val;
					cnt++;
					return true;
				}
				val ^= d[i];
			}
		}
		zero = true;
		return false;
	}
	
	// 查询能异或出的最大值
	long long query_max() {
		long long res = 0;
		for (int i = MAXL; i >= 0; i--) {
			if ((res ^ d[i]) > res) {
				res ^= d[i];
			}
		}
		return res;
	}
	
	// 查询能异或出的最小值
	long long query_min() {
		if (zero) return 0;
		for (int i = 0; i <= MAXL; i++) {
			if (d[i]) return d[i];
		}
		return 0;
	}
	
	// 重构线性基，通过类似高斯消元化为简化阶梯型矩阵
	void rebuild() {
		for (int i = MAXL; i >= 0; i--) {
			for (int j = i - 1; j >= 0; j--) {
				if (d[i] & (1LL << j)) {
					d[i] ^= d[j];
				}
			}
		}
		p.clear();
		for (int i = 0; i <= MAXL; i++) {
			if (d[i]) p.push_back(d[i]);
		}
	}
	
	// 查询去重后的第 k 小值（要求先调用 rebuild）
	long long query_kth(long long k) {
		if (zero) k--; 
		if (k == 0) return 0;
		if (k >= (1LL << cnt)) return -1; 
		
		long long res = 0;
		for (int i = 0; i < (int)p.size(); i++) {
			if (k & (1LL << i)) res ^= p[i];
		}
		return res;
	}
	
	// 合并另一个线性基
	void merge(const LinearBasis& other) {
		for (int i = 0; i <= MAXL; i++) {
			if (other.d[i]) insert(other.d[i]);
		}
		zero |= other.zero;
	}
};

const int MAXN = 20005;
int n, q;
long long G[MAXN];
vector<int> adj[MAXN];

int depth[MAXN];
int fa[MAXN][16];
LinearBasis lb[MAXN][16];

void dfs(int u, int p) {
    depth[u] = depth[p] + 1;
    fa[u][0] = p;
    lb[u][0].insert(G[u]);
    lb[u][0].insert(G[p]);

    for (int i = 1; i < 16; i++) {
        fa[u][i] = fa[fa[u][i - 1]][i - 1];
        lb[u][i] = lb[u][i - 1];
        lb[u][i].merge(lb[fa[u][i - 1]][i - 1]);
    }

    for (int v : adj[u]) {
        if (v != p) {
            dfs(v, u);
        }
    }
}

int get_lca(int u, int v) {
    if (depth[u] < depth[v]) swap(u, v);
    for (int i = 15; i >= 0; i--) {
        if (depth[fa[u][i]] >= depth[v]) {
            u = fa[u][i];
        }
    }
    if (u == v) return u;
    for (int i = 15; i >= 0; i--) {
        if (fa[u][i] != fa[v][i]) {
            u = fa[u][i];
            v = fa[v][i];
        }
    }
    return fa[u][0];
}

int jump(int u, int d) {
    for (int i = 15; i >= 0; i--) {
        if (d & (1 << i)) {
            u = fa[u][i];
        }
    }
    return u;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    if (!(cin >> n >> q)) return 0;
    for (int i = 1; i <= n; i++) {
        cin >> G[i];
    }
    for (int i = 1; i < n; i++) {
        int u, v;
        cin >> u >> v;
        adj[u].push_back(v);
        adj[v].push_back(u);
    }
    
    G[0] = 0; // dummy parent for root
    dfs(1, 0);
    
    while (q--) {
        int u, v;
        cin >> u >> v;
        int lca = get_lca(u, v);
        int du = depth[u] - depth[lca];
        int dv = depth[v] - depth[lca];
        
        LinearBasis ans;
        if (du == 0) {
            ans.insert(G[u]);
        } else {
            int ku = 31 - __builtin_clz(du);
            int vu = jump(u, du - (1 << ku));
            ans.merge(lb[u][ku]);
            ans.merge(lb[vu][ku]);
        }
        
        if (dv == 0) {
            ans.insert(G[v]);
        } else {
            int kv = 31 - __builtin_clz(dv);
            int vv = jump(v, dv - (1 << kv));
            ans.merge(lb[v][kv]);
            ans.merge(lb[vv][kv]);
        }
        
        cout << ans.query_max() << "\n";
    }
    return 0;
}
// AC https://www.luogu.com.cn/record/272234814