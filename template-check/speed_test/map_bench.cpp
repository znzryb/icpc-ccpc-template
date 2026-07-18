#include <bits/stdc++.h>
using namespace std;

mt19937_64 rng(chrono::steady_clock::now().time_since_epoch().count());
uint64_t splitmix64(uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    return x ^ (x >> 31);
}

int main() {
    const int N = 1 << 20;
    vector<uint64_t> keys(N);
    for (int i = 0; i < N; i++) keys[i] = splitmix64(rng());
    auto t0 = chrono::steady_clock::now();
    map<uint64_t, int> mp;
    for (int i = 0; i < N; i++) mp[keys[i]] = i;
    long long s = 0;
    for (int i = 0; i < N; i++) s += mp[keys[i]];
    assert(s != 0);
    auto dt = chrono::steady_clock::now() - t0;
    printf("%lldms\n", (long long)chrono::duration_cast<chrono::milliseconds>(dt).count());
}
