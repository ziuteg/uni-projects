/**
 *	Autor: Åukasz Siudek
 *	Indeks: 283493
 */

#include <stdio.h>
#include <stdlib.h>
#include <queue>

char t[2099][2099];
//bool vis[2099][2099];
bool con[6][6][3][3];


typedef std::pair < std::pair < int, int >, char > tuple3;
#define tmp(a,b,c) std::make_pair(std::make_pair(a,b),c)
#define t1(t) t.first.first
#define t2(t) t.first.second
#define t3(t) t.second 


std::queue < tuple3 > Q;

bool allowed(char block, int x, int y) {
	if (x != 0 && y != 0) {
		return false;
	}
	if (block == 'A') {
		return false;
	}
	if (block == 'B') {
 		if (x == -1 || y == 1) return false;
	}
	else if (block == 'C') {
		if (x == 1 || y == 1) return false;
	}
	else if (block == 'D') {
		if (x == 1 || y == -1) return false;
	}
	else if (block == 'E') {
		if (x == -1 || y == -1) return false;
	}
	return true;
}

void fill_con() {
	for (char i = 'A'; i <= 'F'; i++) {
		for (char j = 'A'; j <= 'F'; j++) {
			for (int a = -1; a <= 1; a++) {
				for (int b = -1; b <= 1; b++) {
					con[i-'A'][j-'A'][a+1][b+1] = (allowed(i, a, b) && allowed(j, -a, -b));
				}
			}
		}
	}
}

void bfs(int x, int y, int n, int m) {
	Q.push(tmp(x, y, t[x][y]));
	t[x][y] = 'A';
	while (!Q.empty()) {
		x = t1(Q.front());
		y = t2(Q.front());
		char block = t3(Q.front()) - 'A';
		Q.pop();

		t[x][y] = 'A';
		int i, j, a, b;

		i = 1; j = 0;
		a = x + i; b = y + j;
		if (con[block][t[a][b]-'A'][i+1][j+1]) {
			Q.push(tmp(a, b, t[a][b]));
			t[a][b] = 'A';
		}

		i = -1; j = 0;
		a = x + i; b = y + j;
		if (con[block][t[a][b]-'A'][i+1][j+1]) {
			Q.push(tmp(a, b, t[a][b]));
			t[a][b] = 'A';
		}

		i = 0; j = 1;
		a = x + i; b = y + j;
		if (con[block][t[a][b]-'A'][i+1][j+1]) {
			Q.push(tmp(a, b, t[a][b]));
			t[a][b] = 'A';
		}

		i = 0; j = -1;
		a = x + i; b = y + j;
		if (con[block][t[a][b]-'A'][i+1][j+1]) {
			Q.push(tmp(a, b, t[a][b]));
			t[a][b] = 'A';
		}
	}
}


int main() {
	int n, m;
	int cnt = 0;
	char *str = (char *)malloc(sizeof(char) * 20);
	fgets(str, 20, stdin);
	sscanf(str, "%d%d", &n, &m);
	free(str);

	for (int i = 1; i <= n; i++) {	
		fgets(t[i]+1, m + 2, stdin);
	}

	for (int i = 0; i <= n+1; i++) {
		t[i][0] = 'A';
		t[i][m+1] = 'A';
	}

	for (int i = 0; i <= m+1; i++) {
		t[0][i] = 'A';
		t[n+1][i] = 'A';
	}

	fill_con();

	//printf("HELLo\n");

	for (int i = 1; i <= n; i++) {
		for (int j = 1; j <= m; j++) {
			if (t[i][j] != 'A') {
				bfs(i, j, n, m);
				cnt++;
			}
		}
	}

	printf("%d\n", cnt);
	return 0;
}