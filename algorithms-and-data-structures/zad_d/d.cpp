#include <cstdio>
#include <vector>
#include <queue>
#include <algorithm>

using namespace std;
typedef vector < int > vi;
typedef vector < vi > vvi;
typedef pair < int, int > pii;
typedef queue < pii > qii;

vvi T1, T2, S1, S2, L1, L2;
vi prev1, prev2;


vi find_centroid(vvi &tree) {
	int n = tree.size();
	vi deg(n);
	vi level;
	qii q;

	for (auto i = 0u; i < tree.size(); i++) {
		deg[i] = tree[i].size();
		if (deg[i] < 2) {
			q.push(make_pair(i, 0));
			deg[i]--;
			n--;
		}
	}

	while (n > 0) {
		pii x = q.front();
		q.pop();
		int node = x.first;
		int dist = x.second;
		for (auto i = 0u; i < tree[node].size(); i++) {
			int next = tree[node][i];
			deg[next]--;
			if (deg[next] == 1) {
				q.push(make_pair(next, dist + 1));
				n--;
			}
		}
	}

	while (q.front().second != q.back().second)
		q.pop();

	while (!q.empty()) {
		level.push_back(q.front().first);
		q.pop();
	}

	return level;
}


void add_root_node(vvi &tree, vi &centroids) {
	int v1 = centroids[0];
	int v2 = centroids[1];
	tree.push_back({v1,v2});

	for (auto i = 0u; i < tree[v1].size(); i++) {
		if (tree[v1][i] == v2) {
			tree[v1].erase(tree[v1].begin() + i);
			break;
		}
	}

	for (auto i = 0u; i < tree[v2].size(); i++) {
		if (tree[v2][i] == v1) {
			tree[v2].erase(tree[v2].begin() + i);
			break;
		}
	}
}


int calc_depth(vvi &tree, vi &prev, vvi &level, int node, int depth = 0) {
	level[depth].push_back(node);
	int height = -1;
	for (auto i = 0u; i < tree[node].size(); i++) {
		int v = tree[node][i];
		if (prev[v] == -1) {
			prev[v] = node;
			height = max(height, calc_depth(tree, prev, level, v, depth + 1));
		}
	}
	return height + 1;
}


bool rooted_tree_isomorphism(int root1, int root2) {
	int n = T1.size();
	prev1.assign(n, -1);
	prev2.assign(n, -1);
	L1.assign(n, vi());
	L2.assign(n, vi());
	S1.assign(n, vi());
	S2.assign(n, vi());
	vi label1(n);
	vi label2(n);

	prev1[root1] = root1;
	prev2[root2] = root2;
	int depth1 = calc_depth(T1, prev1, L1, root1);
	int depth2 = calc_depth(T2, prev2, L2, root2);
	if(depth1 != depth2) return false;
	
	for (auto i = depth1 - 1; i >= 0; i--) {
		
		/* 1-st tree tuples */
		for (auto j = 0u; j < L1[i + 1].size(); j++) {
			int v = L1[i + 1][j];
			S1[prev1[v]].push_back(label1[v]);
		}

		/* 2-nd tree tuples */
		for (auto j = 0u; j < L2[i + 1].size(); j++) {
			int v = L2[i + 1][j];
			S2[prev2[v]].push_back(label2[v]);
		}
		
		if (L1[i].size() != L2[i].size())
			return false;

		/* sort vectors of tuples */
		sort(L1[i].begin(), L1[i].end(), 
			[](int a, int b) { return S1[a] < S1[b]; });
		
		sort(L2[i].begin(), L2[i].end(), 
			[](int a, int b) { return S2[a] < S2[b]; });

		/* assign level labels */
		int label = 0;
		if (S1[L1[i][0]] != S2[L2[i][0]])
			return false;
		label1[L1[i][0]] = label;
		label2[L2[i][0]] = label;

		for (auto j = 1u; j < L1[i].size(); j++) {
			if (S1[L1[i][j]] != S2[L2[i][j]])
				return false;
			if (S1[L1[i][j]] != S1[L1[i][j - 1]])
				label++;
			label1[L1[i][j]] = label;
			label2[L2[i][j]] = label;
		}
	}

	return (S1[root1] == S2[root2]);
}


bool tree_isomorphism() {
	if (T1.size() != T2.size())
		return false;

	vi c1 = find_centroid(T1);
	vi c2 = find_centroid(T2);
	
	if (c1.size() != c2.size())
		return false;

	int root1, root2;
	if(c1.size() == 2) {
		add_root_node(T1, c1);
		add_root_node(T2, c2);
		root1 = T1.size() - 1;
		root2 = root1;
	}
	else {
		root1 = c1[0];
		root2 = c2[0];
	}
	
	return rooted_tree_isomorphism(root1, root2);
}


int main() {
	int tests;
	scanf("%d", &tests);
	
	while (tests--) {
		int n;
		scanf("%d", &n);
		T1.assign(n, vi());
		T2.assign(n, vi());
		
		for (int i = 0; i < n - 1; i++) {
			int a, b;
			scanf("%d%d", &a, &b);
			a--; b--;
			T1[a].push_back(b);
			T1[b].push_back(a);
		}
		for (int i = 0; i < n - 1; i++) {
			int a, b;
			scanf("%d%d", &a, &b);
			a--; b--;
			T2[a].push_back(b);
			T2[b].push_back(a);
		}
		
		if (tree_isomorphism()) printf("TAK\n");
		else printf("NIE\n");
	}
	return 0;
}