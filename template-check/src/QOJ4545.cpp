// teamname: Gospel_rock
/**
 * Problem: H. Triangle Game
 * Judge: QOJ
 * URL: https://qoj.ac/problem/4545
 * Author: Gospel_rock
 * My blog: https://znzryb.com/
 *
 * 模板对应：基础数学 -> 三角形博弈（带约束的 Nim）
 *   先手胜 <=> (a-1) ^ (b-1) ^ (c-1) != 0
 */

#include <bits/stdc++.h>

using namespace std;

using ll = long long;

signed main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int T;
    cin >> T;
    while (T--) {
        ll a, b, c;
        cin >> a >> b >> c;
        cout << (((a - 1) ^ (b - 1) ^ (c - 1)) ? "Win" : "Lose") << '\n';
    }
    return 0;
}
/*
AC https://vjudge.net/solution/69428059
 */