#include <bits/stdc++.h>

using namespace std;

typedef unsigned char BYTE;
typedef unsigned long long LL;

const int LRU = 1;
const int RANDOM = 2;
const int BINARYTREE = 3;

const int FULL = 0;
const int DIRECT = 1;
const int FOUR_WAY = 4;
const int EIGHT_WAY = 8;

const int WRITEALLOCATE_WRITEBACK = 1;
const int WRITEALLOCATE_WRITETHROUGH = 2;
const int WRITEAROUND_WRITEBACK = 3;
const int WRITEAROUND_WRITETHROUGH = 4;

void editBits(BYTE *bits, int pos, int len, LL value) {
	//cout << value << endl;
	for (int i = 0; i < len; i ++) {
		int id = (pos + i) / 8;
		int offset = (pos + i) & 7;
		bits[id] |= (1 << offset);
		if ((value & (1ll << i)) == 0)
			bits[id] ^= (1 << offset);
		//cout << (int)bits[id] << ' ';
	}
	//cout << endl;
}
LL readBits(BYTE *bits, int pos, int len) {
	LL value = 0;
	for (int i = 0; i < len; i ++) {
		int id = (pos + i) / 8;
		int offset = (pos + i) & 7;
		if (bits[id] & (1 << offset))
			value |= (1ll << i);
	}
	return value;
}

class Replace {
public:
	int ways;
	Replace(int _ways): ways(_ways) {}
	virtual void insert(int id) = 0;
	virtual void access(int id) = 0;
	virtual int replace() = 0;
};

class Random: public Replace {
public:
	Random(int _ways): Replace(_ways) {}
	~Random() {}
	void insert(int id) {}
	void access(int id) {}
	int replace() { return rand() % ways; }
};

class LeastRecentlyUsed: public Replace {
	BYTE *stack;
	int bitnum;
public:
	LeastRecentlyUsed(int _ways): Replace(_ways) {
		bitnum = 0;
		while (_ways != 1) {
			_ways >>= 1;
			bitnum ++;
		}
		//cout << bitnum << endl;
		stack = new BYTE[ways * bitnum / 8 + 1];
		memset(stack, 0, sizeof(BYTE) * (ways * bitnum / 8 + 1));
	}
	~LeastRecentlyUsed() { delete [] stack; }
	void insert(int id) {
		for (int i = 0; i < ways - 1; i ++) {
			int reg = (int)readBits(stack, (i + 1) * bitnum, bitnum);
			editBits(stack, i * bitnum, bitnum, (LL)reg);
		}
		editBits(stack, (ways - 1) * bitnum, bitnum, (LL)id);
	}
	void access(int id) {
		int reg = (int)readBits(stack, (ways - 1) * bitnum, bitnum);
		for (int i = ways - 1; i >= 0; i --) {
			if (reg == id) {
				editBits(stack, (ways - 1) * bitnum, bitnum, (LL)id);
				break;
			}
			int reg2 = reg;
			reg = (int)readBits(stack, (i - 1) * bitnum, bitnum);
			editBits(stack, (i - 1) * bitnum, bitnum, (LL)reg2);
		}
	}
	int replace() {
		int reg = (int)readBits(stack, 0, bitnum);
		access(reg);
		return reg;
	}
};

class BinaryTree: public Replace {
	BYTE *tree;
	int depth;
public:
	BinaryTree(int _ways): Replace(_ways) {
		depth = 0;
		while (_ways != 1) {
			_ways >>= 1;
			depth ++;
		}
		tree = new BYTE[ways / 8 + 1];
		memset(tree, 0, sizeof(BYTE) * (ways / 8 + 1));
	}
	~BinaryTree() { delete [] tree; }
	void insert(int id) {
		access(id);
	}
	void access(int id) {
		int pos = 1;
		for (int i = depth - 1; i >= 0; i --) {
			if ((id >> i) & 1 == 0) {
				editBits(tree, pos, 1, 1);
				pos <<= 1;
			} else {
				editBits(tree, pos, 1, 0);
				(pos <<= 1) |= 1;
			}
		}
	}
	int replace() {
		int pos = 1;
		int reg = 0;
		for (int i = 0; i < depth; i ++) {
			if (readBits(tree, pos, 1) == 0) {
				pos <<= 1;
				reg <<= 1;
			} else {
				(pos <<= 1) |= 1;
				(reg <<= 1) |= 1;
			}
		}
		access(reg);
		return reg;
	}
};

class Group {
	int blockSize;
	int ways;
	int tagLen, metaLen;
	int replaceStrategy;
	int writeStrategy;
	BYTE **metaData;
	int data_num;
	Replace *replace;
public:
	Group(int _blockSize, int _ways, int _tagLen, int _replaceStrategy, int _writeStrategy):
		blockSize(_blockSize), ways(_ways), tagLen(_tagLen), replaceStrategy(_replaceStrategy), writeStrategy(_writeStrategy) {
		data_num = 0;
		metaData = new BYTE*[ways];
		metaLen = tagLen + 1;
		if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
			metaLen ++;
		for (int i = 0; i < ways; i ++) {
			metaData[i] = new BYTE[metaLen / 8 + 1];
			memset(metaData[i], 0, sizeof(metaData[i]));
		}
		//puts("group");
		if (replaceStrategy == RANDOM)
			replace = new Random(ways);
		else if (replaceStrategy == LRU)
			replace = new LeastRecentlyUsed(ways);
		else if (replaceStrategy == BINARYTREE)
			replace = new BinaryTree(ways);
	}
	~Group() {
		for (int i = 0; i < ways; i ++)
			delete [] metaData[i];
		delete [] metaData;
		delete replace;
	}
	bool read_byte(LL tag) {
		//cout << tag << endl;
		for (int i = 0; i < data_num; i ++) {
			LL meta_tag = readBits(metaData[i], 0, tagLen);
			//cout << meta_tag << endl;
			if (meta_tag == tag) {
				replace->access(i);
				return true;
			}
		}
		//cout << data_num << ' ' << ways << endl;
		if (data_num < ways) {
			replace->insert(data_num);
			editBits(metaData[data_num], 0, tagLen, tag);
			if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
				editBits(metaData[data_num], tagLen, 1, 0);
			data_num ++;
		} else {
			int pos = replace->replace();
			editBits(metaData[pos], 0, tagLen, tag);
			if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
				editBits(metaData[pos], tagLen, 1, 0);
		}
		return false;
	}
	bool write_byte(LL tag) {
		for (int i = 0; i < data_num; i ++) {
			LL meta_tag = readBits(metaData[i], 0, tagLen);
			if (meta_tag == tag) {
				replace->access(i);
				if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
					editBits(metaData[i], tagLen, 1, 1);
				return true;
			}
		}
		if (writeStrategy == WRITEAROUND_WRITEBACK || writeStrategy == WRITEAROUND_WRITETHROUGH)
			return false;
		if (data_num < ways) {
			replace->insert(data_num);
			editBits(metaData[data_num], 0, tagLen, tag);
			if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
				editBits(metaData[data_num], tagLen, 1, 0);
			data_num ++;
		} else {
			int pos = replace->replace();
			editBits(metaData[pos], 0, tagLen, tag);
			if (writeStrategy == WRITEALLOCATE_WRITEBACK || writeStrategy == WRITEAROUND_WRITEBACK)
				editBits(metaData[pos], tagLen, 1, 0);
		}
		return false;
	}
};

class Cache {
	int cacheSize;
	int blockSize;
	int groupNum, offsetLen, indexLen, tagLen;
	int replaceStrategy;
	int organization;
	int ways;
	int writeStrategy;
	Group** groups;
public:
	Cache(int _cacheSize, int _blockSize, int _replaceStrategy, int _organization, int _writeStrategy):
		cacheSize(_cacheSize), blockSize(_blockSize), replaceStrategy(_replaceStrategy), organization(_organization), writeStrategy(_writeStrategy) {
		groupNum = cacheSize / blockSize;
		if (organization == FULL) {
			ways = groupNum;
			groupNum = 1;
		} else {
			ways = organization;
			groupNum /= ways;
		}
		offsetLen = 0;
		while ((1 << offsetLen) != blockSize)
			offsetLen++;
		indexLen = 0;
		while ((1 << indexLen) != groupNum)
			indexLen++;
		tagLen = 64 - indexLen - offsetLen;
		groups = new Group*[groupNum];
		for (int i = 0; i < groupNum; i++)
			groups[i] = new Group(blockSize, ways, tagLen, replaceStrategy, writeStrategy);
	}
	~Cache();
	bool read(LL addr) {
		LL tag = addr >> (indexLen + offsetLen);
		LL index = (addr ^ (tag << (indexLen + offsetLen))) >> offsetLen;
		return groups[(int)index]->read_byte(tag);
	}
	bool write(LL addr) {
		LL tag = addr >> (indexLen + offsetLen);
		LL index = (addr ^ (tag << (indexLen + offsetLen))) >> offsetLen;
		return groups[(int)index]->write_byte(tag);
	}
};

int main(int argc, char **argv) {
	srand(time(0));
	int cacheSize = 128 * 1024;
	int blockSize = 8;
	int writeStrategy = WRITEALLOCATE_WRITEBACK;
	int organization = EIGHT_WAY;
	int replaceStrategy = LRU;
	char *log_file = nullptr;
	for (int i = 1; i < argc; i ++) {
		if (strcmp(argv[i], "block") == 0)
			blockSize = atoi(argv[i + 1]);
		if (strcmp(argv[i], "alloback") == 0)
			writeStrategy = WRITEALLOCATE_WRITEBACK;
		if (strcmp(argv[i], "allothro") == 0)
			writeStrategy = WRITEALLOCATE_WRITETHROUGH;
		if (strcmp(argv[i], "aroback") == 0)
			writeStrategy = WRITEAROUND_WRITEBACK;
		if (strcmp(argv[i], "arothro") == 0)
			writeStrategy = WRITEAROUND_WRITETHROUGH;
		if (strcmp(argv[i], "full") == 0)
			organization = FULL;
		if (strcmp(argv[i], "direct") == 0)
			organization = DIRECT;
		if (strcmp(argv[i], "4-way") == 0)
			organization = FOUR_WAY;
		if (strcmp(argv[i], "8-way") == 0)
			organization = EIGHT_WAY;
		if (strcmp(argv[i], "lru") == 0)
			replaceStrategy = LRU;
		if (strcmp(argv[i], "random") == 0)
			replaceStrategy = RANDOM;
		if (strcmp(argv[i], "tree") == 0)
			replaceStrategy = BINARYTREE;
		if (strcmp(argv[i], "log") == 0)
			log_file = argv[i + 1];
	}
	FILE *LOG = nullptr;
	if (log_file != nullptr)
		LOG = fopen(log_file, "w");
	Cache *cache = new Cache(cacheSize, blockSize, replaceStrategy, organization, replaceStrategy);
	ios::sync_with_stdio(false);
	string op;
	LL addr;
	//puts("Start running ...");
	int hit_cnt = 0;
	int op_cnt = 0;
	while (cin >> op >> hex >> addr) {
		//cout << hex << addr << endl;
		bool hit;
		if (op == "r" || op == "l")
			hit = cache->read(addr);
		else if (op == "w" || op == "s")
			hit = cache->write(addr);
		if (LOG != nullptr) fputs(hit ? "Hit\n" : "Miss\n", LOG);
		hit_cnt += hit ? 1 : 0;
		op_cnt ++;
		if (op_cnt % 10000 == 0) printf("%d operations ...\n", op_cnt);
	}
	fclose(LOG);
	printf("Hit rate: %.2lf%%\n", ((double)hit_cnt / op_cnt) * 100.);
	return 0;
}
