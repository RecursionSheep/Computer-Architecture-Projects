#include <bits/stdc++.h>

using namespace std;

const int ars_cnt = 6, mrs_cnt = 3, lb_cnt = 3;

#define LD 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define JUMP 6

class Operation {
public:
	int op;
	string reg1, reg2, reg3;
	int val1, val2;
	int issue, comp, writeback;
};
vector<Operation> ops;

class Register {
public:
	int val;
	int op, rs;
};
map<string, Register> regs;

class ReservationStation {
public:
	bool busy, run;
	int time_in, id;
	int op;
	int val1, val2;
	int rs1, rs2;
	int cycle_left;
	string target;
} ars[6], mrs[3], lb[3];

void set_reg(ReservationStation *rs, string reg1, string reg2) {
	
}

void tomasulo() {
	int op = 0;
	int cycle = 1;
	while (1) {
		if (op < ops.size()) {
			if (ops[op].op == LD) {
				for (int i = 0; i < lb_cnt; i++)
					if (lb[i].busy == false) {
						lb[i].busy = true;
						lb[i].run = false;
						lb[i].time_in = cycle;
						lb[i].id = op;
						lb[i].op == LD;
						lb[i].val1 = ops[op].val1;
						lb[i].target = ops[op].reg1;
						lb[i].issue = time_cnt;
						break;
					}
			} else if (ops[op].op == ADD || ops[op].op == SUB) {
				for (int i = 0; i < ars_cnt; i++)
					if (ars[i].busy == false) {
						ars[i].busy = true;
						ars[i].run = false;
						ars[i].time_in = cycle;
						ars[i].id = op;
						ars[i].op == ops[op].op;
						set_reg(&ars[i], ops[op].reg2, ops[op].reg3);
						ars[i].target = ops[op].reg1;
						ars[i].issue = time_cnt;
						break;
					}
			}
		}
	}
}

int main() {
	string statement;
	while (cin >> statement) {
		int pos1 = statement.find(",");
		string op = statement.substr(0, pos1 - 1);
		if (op == "LD") {
			int pos2 = statement.find(",", pos1 + 1);
			ops.push_back((Operation){LD, statement.substr(pos1 + 1, pos2 - pos1 - 1), "", "", atoi(statement.substr(pos2 + 1).c_str()), 0});
		} else if (op == "ADD") {
			int pos2 = statement.find(",", pos1 + 1);
			int pos3 = statement.find(",", pos2 + 1);
			ops.push_back((Operation){ADD, statement.substr(pos1 + 1, pos2 - pos1 - 1), statement.substr(pos2 + 1, pos3 - pos2 - 1), statement.substr(pos3 + 1), 0, 0});
		} else if (op == "SUB") {
			int pos2 = statement.find(",", pos1 + 1);
			int pos3 = statement.find(",", pos2 + 1);
			ops.push_back((Operation){SUB, statement.substr(pos1 + 1, pos2 - pos1 - 1), statement.substr(pos2 + 1, pos3 - pos2 - 1), statement.substr(pos3 + 1), 0, 0});
		} else if (op == "MUL") {
			int pos2 = statement.find(",", pos1 + 1);
			int pos3 = statement.find(",", pos2 + 1);
			ops.push_back((Operation){MUL, statement.substr(pos1 + 1, pos2 - pos1 - 1), statement.substr(pos2 + 1, pos3 - pos2 - 1), statement.substr(pos3 + 1), 0, 0});
		} else if (op == "DIV") {
			int pos2 = statement.find(",", pos1 + 1);
			int pos3 = statement.find(",", pos2 + 1);
			ops.push_back((Operation){DIV, statement.substr(pos1 + 1, pos2 - pos1 - 1), statement.substr(pos2 + 1, pos3 - pos2 - 1), statement.substr(pos3 + 1), 0, 0});
		} else if (op == "JUMP") {
			int pos2 = statement.find(",", pos1 + 1);
			int pos3 = statement.find(",", pos2 + 1);
			ops.push_back((Operation){JUMP, statement.substr(pos2 + 1, pos3 - pos2 - 1), "", "", atoi(statement.substr(pos1 + 1, pos2 - pos1 - 1).c_str()), atoi(statement.substr(pos3 + 1).c_str())});
		}
	}
	tomasulo();
	return 0;
}
