// 来源: GoryK (截图收录, 2026-05-16)
// 用途备忘: 神秘优化 / 卡常暴力, O(n * max(vec)) 的 "数对差值频数" 暴力
//   - 对每个 i, 枚举 j in [vec[i], mx], 把 buc[j] 累加进 ans[j - vec[i]]
//   - 最后 ans[d] = #{(i, k) : i < k, vec[k] - vec[i] == d}
// 模板正文不收录, 只在 snippets/ 里留底备查.
// 关键点: 3 行 #pragma 把朴素 O(n * V) 双层循环靠 Ofast + avx2 + unroll-loops 拉过 OJ.
//   - "Ofast, fast-math" 同行写两个 token 是 GoryK 的写法, GCC 容忍 (Ofast 已含 fast-math),
//     第三行又单写 "O3,unroll-loops" 也是冗余但无害.
//   - target("avx, avx2") 在不支持的判题机 (老 CF / 部分校 OJ) 会 RE/CE, 用前看裁判机指令集.

#pragma GCC optimize("Ofast, fast-math")
#pragma GCC target("avx, avx2")
#pragma GCC optimize("O3,unroll-loops")
// @Author : GoryK
#include <bits/stdc++.h>

#ifdef LOCAL
#include "__DEBUG_TOOL.h"
#endif

void solve() {
    int n, mx = 0;
    std::cin >> n;
    std::vector<int> vec(n + 1), buc(n + 1, 0), ans(n + 1, 0);
    for (int i = 1; i <= n; i++) {
        std::cin >> vec[i];
        for (int j = vec[i]; j <= mx; j++) {
            ans[j - vec[i]] += buc[j];
        }
        buc[vec[i]]++;
        mx = std::max(mx, vec[i]);
    }

    for (int i = 0; i <= n; i++) {
        std::cout << ans[i] << " \n"[i == n];
    }
}

int32_t main() {
    std::ios::sync_with_stdio(false);
    std::cin.tie(nullptr);
    std::cout.tie(nullptr);

    int tt = 1, _ = 0;
    std::cin >> tt;
    while (tt-- && ++_)
        // std::cout << "TEST CASE : " << _ << endl,
        solve();

    return 0;
}
