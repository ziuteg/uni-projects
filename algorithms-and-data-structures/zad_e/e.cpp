#include <stdio.h>
#include <vector>
#include <queue>
#include <cstdint>
#include <cstring>
#include <array>

#define MAX_N 7
#define MOD 800029
#define MOD2 800011

using u16 = std::uint16_t;
using v16 = std::array < u16, MAX_N >;

int n, cnt, dist;
int size;
int collisions;

std::array < u16, MAX_N > capacity;
std::array < bool, MOD > used;
std::array < u16, MAX_N > tab[MOD];
std::queue < std::pair < v16, int > > q;

void print_vector(v16 &v) {
  for (int i = 0; i < size; i++) {
    printf("%hu", v[i]);
  }
  printf("\n");
}

v16 read() {
  scanf("%d", &size);
  for (int i = 0; i < size; i++) {
    scanf("%hu", &capacity[i]);
  }
  return capacity;
}

std::uint64_t hash_vector(v16 &v) {
  std::uint64_t hash = 0;
  for (int i = 0; i < size; i++) {
    hash <<= 10;
    hash |= v[i];
  }
  return hash;
}

void check_collision(v16 &v, int d) {
  std::uint64_t h = hash_vector(v);
  int hash = h % MOD;
  int step = (h % MOD2) + 1;

  while (used[hash]) {
    if (tab[hash] == v)
      return;
    hash = (hash + step) % MOD;
  }

  used[hash] = true;
  tab[hash] = v;
  q.push(make_pair(v, d + 1));
  cnt++;
  dist = std::max(dist, d + 1);
}

void generate_combinations(v16 &v, int d) {
  v16 next = v;
  for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) {
      // przelewamy z A do B
      // jesli z A do A to wylewamy
      if (i != j) {
        u16 left = capacity[j] - v[j]; // tyle mesci jeszcze B
        u16 move = std::min(v[i], left); // tyle mozna przelac z A
        if (move == 0)
          continue;
        next[i] -= move;
        next[j] += move;
      }
      else {
        if (next[i] == 0)
          continue;
        next[i] = 0;
      }
      check_collision(next, d);
      next[i] = v[i];
      next[j] = v[j];
    }
  }
}

void calc(v16 init) {
  check_collision(init, -1);
  while (!q.empty()) {
    v16 v = q.front().first;
    int d = q.front().second;
    q.pop();
    generate_combinations(v, d);
  }
}

void clear() {
  cnt = 0;
  dist = 0;
  collisions = 0;
  //for (int i = 0; i < MOD; i++) used[i] = false;
  std::memset(used.data(), 0, used.size() * sizeof(used[0]));
  while (!q.empty()) q.pop();
}

int main() {
  int tests;
  scanf("%d\n", &tests);
  while (tests--) {
    calc(read());
    printf("%d %d\n", cnt, dist);
    clear();
  }
  return 0;
}
