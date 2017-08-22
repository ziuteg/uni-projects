/**
 * Autor: Łukasz Siudek
 * Indeks: 283493
 **/

#include <stdio.h>

#define mask(t) ((t[0]>>6) | ((t[1]>>6)<<1) | ((t[2]>>6)<<2))
#define hit1(m) (((m<<2)&4) | ((m>>2)&1))
#define hit2(m) (((m<<1)&6) | ((m>>1)&3))
typedef unsigned char byte;

// możliwe ustawienia skoczków na szachownicy 3x2
byte p[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 32, 34, 36, 38, 40, 42, 48, 50, 52, 54, 56, 58};
// blokowane pozycje 3x1
byte b[] = {0, 2, 5, 7, 2, 2, 7, 7, 4, 6, 5, 7, 0, 2, 5, 7, 2, 2, 7, 7, 4, 6, 5, 7, 1, 5, 3, 7, 5, 5, 1, 5, 3, 7, 5, 5};
// liczba skoczków w ustawieniach 3x1
byte num[] = {0, 1, 1, 2, 1, 2, 2, 3};
// indeksy ustawień skoczków
byte index[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0, 0, 0, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 0, 0, 0, 0, 24, 0, 25, 0, 26, 0, 27, 0, 28, 0, 29, 0, 0, 0, 0, 0, 30, 0, 31, 0, 32, 0, 33, 0, 34, 0, 35};

byte t[1<<20][4];
int dp[2][sizeof(p)];
byte ans[1<<20][sizeof(p)];

void debug() {

	for (auto i = 0u; i < sizeof(p); i++) {

		printf("\n==%d==\n", i);
		for (int j = 0; j < 3; j++) {
			if (p[i]&(1<<j)) printf("S");
			else printf(".");
		}
		printf("\n");
		for (int j = 3; j < 6; j++) {
			if (p[i]&(1<<j)) printf("S");
			else printf(".");
		}
	}

}

int main() {
	int n;
	scanf("%d", &n);

	//debug();
	//return 0;

	if (n > 0) {
		scanf("%s", t[0]);
		byte m0 = mask(t[0]);
		for (int k = 0; k < (1<<3); k++) {
			if (!(m0 & k)) {
				dp[0][index[k<<3]] = num[k];
			}
		}
	}

	for (int i = 1; i < n; i++) {
		scanf("%s", t[i]);
		byte m = mask(t[i]);

		for (auto j = 0u; j < sizeof(p); j++) { // j - indeks poprz. perm.
			// p[j] - poprzednia permutacja
			// b[j] - zablokowane przez poprzednią permutację
			for (int k = 0; k < (1<<3); k++) { // k - nowy wiersz skoczków
				if (!((b[j] | m) & k)) {
					byte next = index[(p[j]>>3)|(k<<3)];
					int val = dp[0][j] + num[k];
					if (dp[1][next] < val) {
						dp[1][next] = val;
						ans[i][next] = j;
					}
				}
			}
		}

		for (auto j = 0u; j < sizeof(p); j++) {
			dp[0][j] = dp[1][j];
			dp[1][j] = 0;
		}
	}

	int cnt = -1;
	int prev = -1;
	for (auto i = 0u; i < sizeof(p); i++) {
		if (cnt < dp[0][i]) {
			cnt = dp[0][i];
			prev = i;
		}
	}
	for (int i = n - 1; i >= 0; i--) {
		byte row = p[prev]>>3;
		prev = ans[i][prev];
		for (int j = 0; j < 3; j++) {
			if (row & (1<<j)) t[i][j] = 'S';
		}
	}

	printf("%d\n", cnt);
	for (int i = 0; i < n; i++) {
		printf("%s\n", t[i]);
	}

	return 0;
}
