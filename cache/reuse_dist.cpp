#include <bits/stdc++.h>

using namespace std;

int main() {
	map<unsigned long long, int> last_access;
	map<int, int> dist_cnt;
	ios::sync_with_stdio(false);
	string op;
	unsigned long long addr;
	int cnt = 0;
	while (cin >> op >> hex >> addr) {
		//cout << hex << addr << endl;
		cnt ++;
		if (last_access[addr] != 0)
			dist_cnt[(cnt - last_access[addr] - 1) / 5] ++;
		last_access[addr] = cnt;
	}
	for (auto it = dist_cnt.begin(); it != dist_cnt.end(); it ++)
		printf("%d %d\n", (*it).first, (*it).second);
	return 0;
}
