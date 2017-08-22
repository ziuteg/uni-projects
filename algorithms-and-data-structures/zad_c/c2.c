#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define ctoi(c) (c - 'A')

uint8_t t1[8][8]; 		/* produkcje typu I */
uint8_t t2[26]; 		/* produkcje typu II */
uint8_t g[256][256]; 	/* graf mozliwych przejsc z kombinacji produkcji typu I */
uint8_t dp[1009][1009];
char str[1009];

void clean() {
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			t1[i][j] = 0;
		}
	}
	for (int i = 0; i < 26; i++) {
		t2[i] = 0;
	}
	for (int i = 1; i < 256; i++) {
		for (int j = 1; j < 256; j++) {
			g[i][j] = 0;
		}
	}
}

void precalc1() {
	for (int x = 1; x < 256; x++) {
		for (int y = 1; y < 256; y++) {
			for (uint8_t i = 0; i < 8; i++) {

				uint8_t mx = x & (1 << i);
				for (uint8_t j = 0; j < 8; j++) {
					uint8_t my = y & (1 << j);
					if (mx && my)
						g[x][y] |= t1[i][j];
				}
			}
		}
	}
}

void precalc2(int a, int b, int c) {
	for (int x = 1; x < 256; x++) {
		for (int y = 1; y < 256; y++) {
			uint8_t mx = x & (1 << b);
			uint8_t my = y & (1 << c);
			if (mx && my) {
				g[x][y] |= (1 << a);
			}
		}
	}
}

int calc() {

	int sn = strlen(str);

	for (int i = 1; i <= sn; i++) {
		for (int j = 0; j <= sn - i; j++)
			dp[i][j] = 0;
	}

	for (int i = 0; i < sn; i++) {
		dp[1][i] = t2[str[i] - 'a'];
	}

	for (int len = 2; len <= sn; len++) {
		for (int i = 0; i < sn - len + 1; i++) {
			for (int k = 1; k < len; k++) {
				uint8_t mx = dp[k][i];
				uint8_t my = dp[len - k][i + k];
				dp[len][i] |= g[mx][my];
			}
		}
	}

	return (dp[sn][0] & 1);
}

int main() {

	int n, m1, m2;
	scanf("%d", &n);

	while (n--) {
		scanf("%d%d", &m1, &m2);

		clean();

		/* optymalizacja */
		if (m1 > 64) {
			for (int i = 0; i < m1; i++) {
				char a, b, c;
				scanf(" %c %c %c", &a, &b, &c);
				t1[ctoi(b)][ctoi(c)] |= (1 << (ctoi(a)));
			}
			precalc1();
		}
		else {
			for (int i = 0; i < m1; i++) {
				char a, b, c;
				scanf(" %c %c %c", &a, &b, &c);
				precalc2(ctoi(a), ctoi(b), ctoi(c));
			}
		}
		/* ********* */

		for (int i = 0; i < m2; i++) {
			char a, b;
			scanf(" %c %c", &a, &b);
			t2[b - 'a'] |= (1 << (ctoi(a)));
		}
		scanf("%s", str);


		if (calc()) printf("TAK\n");
		else printf("NIE\n");
	}

	return 0;
}