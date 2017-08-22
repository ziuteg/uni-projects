#include <cstdio>
#include <vector>
#include <algorithm>

#define MAXN 2000000

using pii = std::pair<int, int>;
using vii = std::vector<pii>;

vii v;
int starting[MAXN];
int ending[MAXN];
int tab[MAXN];
int n;

class tree {
  std::vector<int> t;
  int base;

  int find_max(int p, int q, int i, int j, int idx) {
    if (p == i && q == j)
      return t[idx];
    int avg = (i + j)/2;
    if (p > avg)
      return find_max(p, q, avg + 1, j, idx * 2 + 1);
    if (q <= avg)
      return find_max(p, q, i, avg, idx * 2);
    return std::max(
      find_max(p, avg, i, avg, idx * 2),
      find_max(avg + 1, q, avg + 1, j, idx * 2 + 1)
    );
  }

public:
  tree(int size) {
    int p = 1;
    while (p < size) p <<= 1;
    base = p;
    t.assign(p * 2, 0);
  }

  void insert(int x, int val) {
    int idx = base + x;
    while (idx > 0) {
      if (t[idx] < val) t[idx] = val;
      else break;
      idx /= 2;
    }
  }

  int find_max(int p, int q) {
    if (q < p) return 0;
    return find_max(p, q, 1, base, 1);
  }
};

void read() {
  v.clear();
  scanf("%d", &n);
  for (int i = 0; i < n; i++) {
    scanf("%d", &tab[i]);
    v.push_back(std::make_pair(tab[i], i));
  }
}

void precalc() {
  /* map values */
  std::sort(v.begin(), v.end());
  int val = 1;
  int last = v.front().first;
  for (auto x : v) {
    if (x.first != last) {
      val++;
      last = x.first;
    }
    tab[x.second] = val;
  }
  /* fill tables */
  ending[0] = 1;
  for (int i = 1; i < n; i++) {
    if (tab[i] > tab[i - 1]) {
      ending[i] = ending[i - 1] + 1;
    }
    else ending[i] = 1;
  }
  starting[n - 1] = 1;
  for (int i = n - 2; i >=0; i--) {
    if (tab[i] < tab[i + 1])
      starting[i] = starting[i + 1] + 1;
    else starting[i] = 1;
  }
}

void calc() {
  tree t(n);
  int ans = -1;
  for (int i = 0; i < n; i++) {
    ans = std::max(ans, t.find_max(1, tab[i] - 1) + starting[i]);
    t.insert(tab[i] - 1, ending[i]);
  }
  printf("%d\n", ans);
}


int main() {
  int t;
  scanf("%d", &t);
  while (t--) {
    read();
    precalc();
    calc();
  }
  return 0;
}
