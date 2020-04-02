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
	for (int i = 0; i < len; i ++) {
		int id = (pos + i) / 8;
		int offset = (pos + i) & 7;
		bits[id] |= (1 << offset);
		if (value & (1ll << i) == 0)
			bits[id] ^= (1 << offset);
	}
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
		while (_ways != 1) {
			_ways >>= 1;
			bitnum ++;
		}
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
		for (int i = 0; i < data_num; i ++) {
			LL meta_tag = readBits(metaData[i], 0, tagLen);
			if (meta_tag == tag) {
				replace->access(i);
				return true;
			}
		}
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
	vector<Group*> groups;
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
		for (int i = 0; i < groupNum; i++)
			groups.push_back(new Group(blockSize, ways, tagLen, replaceStrategy, writeStrategy));
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
	
	return 0;
}
