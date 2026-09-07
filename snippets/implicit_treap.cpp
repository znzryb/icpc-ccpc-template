#include <bits/stdc++.h>
using namespace std;

struct ImplicitTreap {
    using ll = long long;
    using ull = unsigned long long;
    static constexpr ll INF = (1LL << 60);

    // 外部位置：insert(pos, x) 在第 pos 个元素后插入，pos 为 0-based 间隙；
    // 其余位置和区间接口均为 1-based，区间均为闭区间 [l, r]。
    struct Node {
        int ch[2] = {0, 0};
        int sz = 1;
        ll val = 0, sum = 0, mn = 0, mx = 0;
        ull pri = 0;
        bool rev = false;
        bool has_set = false;
        ll set_tag = 0, add_tag = 0;
    };

    vector<Node> tr;
    int rt = 0;
    mt19937_64 rng;

    ImplicitTreap()
        : rng(chrono::steady_clock::now().time_since_epoch().count()) {
        tr.emplace_back();  // 0 号空节点
    }

    explicit ImplicitTreap(const vector<ll> &a)
        : rng(chrono::steady_clock::now().time_since_epoch().count()) {
        tr.emplace_back();
        build(a);
    }

    int size(int x) const { return x ? tr[x].sz : 0; }
    ll sum(int x) const { return x ? tr[x].sum : 0; }
    ll min_value(int x) const { return x ? tr[x].mn : INF; }
    ll max_value(int x) const { return x ? tr[x].mx : -INF; }

    int new_node(ll v) {
        Node t;
        t.val = t.sum = t.mn = t.mx = v;
        t.pri = splitmix64(rng());
        tr.push_back(t);
        return (int)tr.size() - 1;
    }

    void reserve(int n) { tr.reserve(n + 1); }

    void apply_set(int x, ll v) {
        if (!x) return;
        tr[x].val = tr[x].mn = tr[x].mx = v;
        tr[x].sum = v * tr[x].sz;
        tr[x].has_set = true;
        tr[x].set_tag = v;
        tr[x].add_tag = 0;
    }

    void apply_add(int x, ll v) {
        if (!x) return;
        tr[x].val += v;
        tr[x].sum += v * tr[x].sz;
        tr[x].mn += v;
        tr[x].mx += v;
        if (tr[x].has_set) tr[x].set_tag += v;
        else tr[x].add_tag += v;
    }

    void apply_reverse(int x) {
        if (!x) return;
        swap(tr[x].ch[0], tr[x].ch[1]);
        tr[x].rev ^= 1;
    }

    void pushup(int x) {
        if (!x) return;
        int l = tr[x].ch[0], r = tr[x].ch[1];
        tr[x].sz = size(l) + size(r) + 1;
        tr[x].sum = sum(l) + tr[x].val + sum(r);
        tr[x].mn = min({min_value(l), tr[x].val, min_value(r)});
        tr[x].mx = max({max_value(l), tr[x].val, max_value(r)});
    }

    // 赋值必须先于加法下传；翻转与二者可交换，这里统一最后下传。
    void pushdown(int x) {
        if (!x) return;
        if (tr[x].has_set) {
            apply_set(tr[x].ch[0], tr[x].set_tag);
            apply_set(tr[x].ch[1], tr[x].set_tag);
            tr[x].has_set = false;
        }
        if (tr[x].add_tag != 0) {
            apply_add(tr[x].ch[0], tr[x].add_tag);
            apply_add(tr[x].ch[1], tr[x].add_tag);
            tr[x].add_tag = 0;
        }
        if (tr[x].rev) {
            apply_reverse(tr[x].ch[0]);
            apply_reverse(tr[x].ch[1]);
            tr[x].rev = false;
        }
    }

    // 按数量切：左树为前 k 个元素，右树为其余元素。
    pair<int, int> split(int root, int k) {
        if (!root) return {0, 0};
        pushdown(root);
        int left_size = size(tr[root].ch[0]);
        if (left_size >= k) {
            auto [a, b] = split(tr[root].ch[0], k);
            tr[root].ch[0] = b;
            pushup(root);
            return {a, root};
        }
        auto [a, b] = split(tr[root].ch[1], k - left_size - 1);
        tr[root].ch[1] = a;
        pushup(root);
        return {root, b};
    }

    int merge(int a, int b) {
        if (!a || !b) return a ? a : b;
        if (tr[a].pri >= tr[b].pri) {
            pushdown(a);
            tr[a].ch[1] = merge(tr[a].ch[1], b);
            pushup(a);
            return a;
        }
        pushdown(b);
        tr[b].ch[0] = merge(a, tr[b].ch[0]);
        pushup(b);
        return b;
    }

    void clear() {
        tr.clear();
        tr.emplace_back();
        rt = 0;
    }

    void build(const vector<ll> &a) {
        clear();
        reserve((int)a.size());
        for (ll x : a) rt = merge(rt, new_node(x));
    }

    int root() const { return rt; }
    int length() const { return size(rt); }

    // 把当前序列切为 A=[1,l-1]、B=[l,r]、C=[r+1,n]。
    tuple<int, int, int> split_range(int l, int r) {
        auto [a, bc] = split(rt, l - 1);
        auto [b, c] = split(bc, r - l + 1);
        return {a, b, c};
    }

    void merge_range(int a, int b, int c) {
        rt = merge(a, merge(b, c));
    }

    // 在第 pos 个元素后插入；pos=0 表示插到最前面。
    void insert(int pos, ll v) {
        assert(0 <= pos && pos <= length());
        auto [a, b] = split(rt, pos);
        rt = merge(a, merge(new_node(v), b));
    }

    void push_back(ll v) { insert(length(), v); }
    void push_front(ll v) { insert(0, v); }

    void erase(int pos) {
        assert(1 <= pos && pos <= length());
        auto [a, bc] = split(rt, pos - 1);
        auto [b, c] = split(bc, 1);
        (void)b;  // 不回收节点；总插入次数应控制在可接受范围内。
        rt = merge(a, c);
    }

    void erase(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        (void)b;
        rt = merge(a, c);
    }

    ll kth(int k) {
        assert(1 <= k && k <= length());
        int x = rt;
        while (x) {
            pushdown(x);
            int left_size = size(tr[x].ch[0]);
            if (k == left_size + 1) return tr[x].val;
            if (k <= left_size) x = tr[x].ch[0];
            else k -= left_size + 1, x = tr[x].ch[1];
        }
        assert(false);
        return 0;
    }

    void set_value(int pos, ll v) {
        assert(1 <= pos && pos <= length());
        auto [a, b, c] = split_range(pos, pos);
        apply_set(b, v);
        merge_range(a, b, c);
    }

    ll get_value(int pos) { return kth(pos); }

    void range_set(int l, int r, ll v) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        apply_set(b, v);
        merge_range(a, b, c);
    }

    void range_add(int l, int r, ll v) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        apply_add(b, v);
        merge_range(a, b, c);
    }

    void range_reverse(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        apply_reverse(b);
        merge_range(a, b, c);
    }

    ll range_sum(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        ll ans = sum(b);
        merge_range(a, b, c);
        return ans;
    }

    ll range_min(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        ll ans = min_value(b);
        merge_range(a, b, c);
        return ans;
    }

    ll range_max(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        ll ans = max_value(b);
        merge_range(a, b, c);
        return ans;
    }

    struct Info {
        ll sum = 0, mn = INF, mx = -INF;
        int sz = 0;
    };

    Info range_info(int l, int r) {
        assert(1 <= l && l <= r && r <= length());
        auto [a, b, c] = split_range(l, r);
        Info ans{sum(b), min_value(b), max_value(b), size(b)};
        merge_range(a, b, c);
        return ans;
    }

    void dump_dfs(int x, vector<ll> &res) {
        if (!x) return;
        pushdown(x);
        dump_dfs(tr[x].ch[0], res);
        res.push_back(tr[x].val);
        dump_dfs(tr[x].ch[1], res);
    }

    vector<ll> to_vector() {
        vector<ll> res;
        res.reserve(length());
        dump_dfs(rt, res);
        return res;
    }

    void print(ostream &out = cout) {
        vector<ll> a = to_vector();
        for (int i = 0; i < (int)a.size(); ++i) {
            if (i) out << ' ';
            out << a[i];
        }
        out << '\n';
    }

private:
    static ull splitmix64(ull x) {
        x ^= x >> 30;
        x *= 0xbf58476d1ce4e5b9ULL;
        x ^= x >> 27;
        x *= 0x94d049bb133111ebULL;
        return x ^ (x >> 31);
    }
};
