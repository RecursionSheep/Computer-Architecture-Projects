#include <bits/stdc++.h>

using namespace std;

const int ars_cnt = 6, mrs_cnt = 3, lb_cnt = 3;
const int adder_cnt = 3, multer_cnt = 2, loader_cnt = 2;

#define LD 1
#define ADD 2
#define SUB 3
#define MUL 4
#define DIV 5
#define JUMP 6

inline int str2int(string s) {
	int x = 0;
	for (int i = 2; i < s.length(); i ++) {
		if (s[i] <= '9')
			x = (x << 4) | (s[i] - '0');
		else
			x = (x << 4) | (s[i] - 'A' + 10);
	}
	return x;
}

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
	int op1, op2;
	int rs1, rs2;
	int cycle_left;
	string target;
} ars[6], mrs[3], lb[3];

void set_reg(ReservationStation *rs, string reg1, string reg2) {
	if (regs[reg1].op == 0) {
		rs->val1 = regs[reg1].val;
		rs->op1 = 0;
	} else {
		rs->val1 = 0;
		rs->op1 = regs[reg1].op;
		rs->rs1 = regs[reg1].rs;
	}
	if (regs[reg2].op == 0) {
		rs->val2 = regs[reg2].val;
		rs->op2 = 0;
	} else {
		rs->val2 = 0;
		rs->op2 = regs[reg2].op;
		rs->rs2 = regs[reg2].rs;
	}
}

int compute(int op, int val1, int val2) {
	if (op == ADD)
		return val1 + val2;
	if (op == SUB)
		return val1 - val2;
	if (op == MUL)
		return val1 * val2;
	if (op == DIV)
		return val2 == 0 ? val1 : val1 / val2;
}

void update_value(int result, int op, int rs) {
	//printf("update: %d %d %d\n", result, op, rs + 1);
	for (int i = 0; i < ars_cnt; i ++)
		if (ars[i].busy) {
			if (ars[i].op1 && ars[i].op1 == op && ars[i].rs1 == rs) {
				ars[i].val1 = result;
				ars[i].op1 = 0;
			}
			if (ars[i].op2 && ars[i].op2 == op && ars[i].rs2 == rs) {
				ars[i].val2 = result;
				ars[i].op2 = 0;
			}
		}
	for (int i = 0; i < mrs_cnt; i ++)
		if (mrs[i].busy) {
			if (mrs[i].op1 && mrs[i].op1 == op && mrs[i].rs1 == rs) {
				mrs[i].val1 = result;
				mrs[i].op1 = 0;
			}
			if (mrs[i].op2 && mrs[i].op2 == op && mrs[i].rs2 == rs) {
				mrs[i].val2 = result;
				mrs[i].op2 = 0;
			}
		}
}

int compute_cycle(int op, int val1, int val2) {
	if (op == LD)
		return 3;
	if (op == ADD || op == SUB)
		return 3;
	if (op == MUL)
		return 4;
	if (op == DIV && val2 == 0)
		return 1;
	if (op == DIV)
		return 4;
}

void print_status(int cycle) {
	printf("cycle: %d\n", cycle);
	for (int i = 0; i < ars_cnt; i ++) {
		printf("Ars %d: ", i + 1);
		if (ars[i].busy)
			printf("busy ");
		else {
			printf("empty\n");
			continue;
		}
		if (ars[i].run)
			printf("running %d cycles left ", ars[i].cycle_left);
		else
			printf("waiting ");
		if (ars[i].op1)
			printf("rs %d %d ", ars[i].op1, ars[i].rs1 + 1);
		else
			printf("value %d ", ars[i].val1);
		if (ars[i].op2)
			printf("rs %d %d ", ars[i].op2, ars[i].rs2 + 1);
		else
			printf("value %d ", ars[i].val2);
		puts("");
	}
	for (int i = 0; i < mrs_cnt; i ++) {
		printf("Mrs %d: ", i + 1);
		if (mrs[i].busy)
			printf("busy ");
		else {
			printf("empty\n");
			continue;
		}
		if (mrs[i].run)
			printf("running %d cycles left ", mrs[i].cycle_left);
		else
			printf("waiting ");
		if (mrs[i].op1)
			printf("rs %d %d ", mrs[i].op1, mrs[i].rs1 + 1);
		else
			printf("value %d ", mrs[i].val1);
		if (mrs[i].op2)
			printf("rs %d %d ", mrs[i].op2, mrs[i].rs2 + 1);
		else
			printf("value %d ", mrs[i].val2);
		puts("");
	}
	for (int i = 0; i < lb_cnt; i ++) {
		printf("LB %d: ", i + 1);
		if (lb[i].busy)
			printf("busy ");
		else {
			printf("empty\n");
			continue;
		}
		if (lb[i].run)
			printf("running %d cycles left\n", lb[i].cycle_left);
		else
			printf("waiting\n");
	}
}

void tomasulo() {
	int op = 0;
	int cycle = 1;
	while (1) {
		for (int i = 0; i < ars_cnt; i ++)
			if (ars[i].run) {
				ars[i].cycle_left --;
				if (ars[i].cycle_left == 0) {
					ops[ars[i].id].comp = cycle;
				} else if (ars[i].cycle_left == -1) {
					ops[ars[i].id].writeback = cycle;
					if (regs[ars[i].target].op == ars[i].op && regs[ars[i].target].rs == i) {
						regs[ars[i].target].val = compute(ars[i].op, ars[i].val1, ars[i].val2);
						regs[ars[i].target].op = 0;
						regs[ars[i].target].rs = 0;
					}
					update_value(compute(ars[i].op, ars[i].val1, ars[i].val2), ars[i].op, i);
					ars[i].run = false;
					ars[i].busy = false;
				}
			}
		for (int i = 0; i < mrs_cnt; i ++)
			if (mrs[i].run) {
				mrs[i].cycle_left --;
				if (mrs[i].cycle_left == 0) {
					ops[mrs[i].id].comp = cycle;
				} else if (mrs[i].cycle_left == -1) {
					ops[mrs[i].id].writeback = cycle;
					if (regs[mrs[i].target].op == mrs[i].op && regs[mrs[i].target].rs == i) {
						regs[mrs[i].target].val = compute(mrs[i].op, mrs[i].val1, mrs[i].val2);
						regs[mrs[i].target].op = 0;
						regs[mrs[i].target].rs = 0;
					}
					update_value(compute(mrs[i].op, mrs[i].val1, mrs[i].val2), mrs[i].op, i);
					mrs[i].run = false;
					mrs[i].busy = false;
				}
			}
		for (int i = 0; i < lb_cnt; i ++) {
			if (lb[i].run) {
				lb[i].cycle_left --;
				if (lb[i].cycle_left == 0) {
					ops[lb[i].id].comp = cycle;
				} else if (lb[i].cycle_left == -1) {
					ops[lb[i].id].writeback = cycle;
					if (regs[lb[i].target].op == lb[i].op && regs[lb[i].target].rs == i) {
						regs[lb[i].target].val = lb[i].val1;
						regs[lb[i].target].op = 0;
						regs[lb[i].target].rs = 0;
					}
					update_value(lb[i].val1, lb[i].op, i);
					lb[i].run = false;
					lb[i].busy = false;
				}
			}
		}
		
		
		if (op < ops.size()) {
			if (ops[op].op == LD) {
				for (int i = 0; i < lb_cnt; i ++)
					if (lb[i].busy == false) {
						lb[i].busy = true;
						lb[i].run = false;
						lb[i].time_in = cycle;
						lb[i].id = op;
						lb[i].op = LD;
						lb[i].val1 = ops[op].val1;
						lb[i].target = ops[op].reg1;
						regs[lb[i].target].op = ops[op].op;
						regs[lb[i].target].rs = i;
						ops[op].issue = cycle;
						//printf("issue: %d\n", op);
						op ++;
						break;
					}
			} else if (ops[op].op == ADD || ops[op].op == SUB) {
				for (int i = 0; i < ars_cnt; i ++)
					if (ars[i].busy == false) {
						ars[i].busy = true;
						ars[i].run = false;
						ars[i].time_in = cycle;
						ars[i].id = op;
						ars[i].op = ops[op].op;
						set_reg(&ars[i], ops[op].reg2, ops[op].reg3);
						ars[i].target = ops[op].reg1;
						regs[ars[i].target].op = ops[op].op;
						regs[ars[i].target].rs = i;
						ops[op].issue = cycle;
						//printf("issue: %d\n", op);
						op ++;
						break;
					}
			} else if (ops[op].op == MUL || ops[op].op == DIV) {
				for (int i = 0; i < mrs_cnt; i ++)
					if (mrs[i].busy == false) {
						mrs[i].busy = true;
						mrs[i].run = false;
						mrs[i].time_in = cycle;
						mrs[i].id = op;
						mrs[i].op = ops[op].op;
						set_reg(&mrs[i], ops[op].reg2, ops[op].reg3);
						mrs[i].target = ops[op].reg1;
						regs[mrs[i].target].op = ops[op].op;
						regs[mrs[i].target].rs = i;
						ops[op].issue = cycle;
						//printf("issue: %d\n", op);
						op ++;
						break;
					}
			}
		} else {
			bool finish = true;
			for (int i = 0; i < ars_cnt; i ++)
				if (ars[i].busy) {
					finish = false;
					break;
				}
			for (int i = 0; i < mrs_cnt; i ++)
				if (mrs[i].busy) {
					finish = false;
					break;
				}
			for (int i = 0; i < lb_cnt; i ++)
				if (lb[i].busy) {
					finish = false;
					break;
				}
			if (finish) break;
		}
		
		int idle_ars = adder_cnt;
		for (int i = 0; i < ars_cnt; i ++)
			if (ars[i].run) idle_ars --;
		while (idle_ars) {
			int min_op = 10000000, rs_run = -1;
			for (int i = 0; i < ars_cnt; i ++)
				if (ars[i].busy && !ars[i].run && (ars[i].op1 == 0 && ars[i].op2 == 0) && ars[i].id < min_op) {
					min_op = ars[i].id;
					rs_run = i;
				}
			if (rs_run == -1) break;
			idle_ars --;
			ars[rs_run].run = true;
			ars[rs_run].cycle_left = compute_cycle(ars[rs_run].op, ars[rs_run].val1, ars[rs_run].val2);
		}
		int idle_mrs = multer_cnt;
		for (int i = 0; i < mrs_cnt; i ++)
			if (mrs[i].run) idle_mrs --;
		while (idle_mrs) {
			int min_op = 10000000, rs_run = -1;
			for (int i = 0; i < mrs_cnt; i ++)
				if (mrs[i].busy && !mrs[i].run && (mrs[i].op1 == 0 && mrs[i].op2 == 0) && mrs[i].id < min_op) {
					min_op = mrs[i].id;
					rs_run = i;
				}
			if (rs_run == -1) break;
			idle_mrs --;
			mrs[rs_run].run = true;
			mrs[rs_run].cycle_left = compute_cycle(mrs[rs_run].op, mrs[rs_run].val1, mrs[rs_run].val2);
		}
		int idle_load = loader_cnt;
		for (int i = 0; i < lb_cnt; i ++)
			if (lb[i].run) idle_load --;
		while (idle_load) {
			int min_op = 10000000, rs_run = -1;
			for (int i = 0; i < lb_cnt; i ++)
				if (lb[i].busy && !lb[i].run && mrs[i].id < min_op) {
					min_op = lb[i].id;
					rs_run = i;
				}
			if (rs_run == -1) break;
			idle_load --;
			lb[rs_run].run = true;
			lb[rs_run].cycle_left = 3;
		}
		
		//print_status(cycle);
		
		cycle ++;
	}
}

int main() {
	string statement;
	while (cin >> statement) {
		int pos1 = statement.find(",");
		string op = statement.substr(0, pos1);
		if (op == "LD") {
			int pos2 = statement.find(",", pos1 + 1);
			ops.push_back((Operation){LD, statement.substr(pos1 + 1, pos2 - pos1 - 1), "", "", str2int(statement.substr(pos2 + 1)), 0});
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
			ops.push_back((Operation){JUMP, statement.substr(pos2 + 1, pos3 - pos2 - 1), "", "", str2int(statement.substr(pos1 + 1, pos2 - pos1 - 1)), str2int(statement.substr(pos3 + 1))});
		}
	}
	tomasulo();
	for (int i = 0; i < ops.size(); i ++)
		printf("%d %d %d\n", ops[i].issue, ops[i].comp, ops[i].writeback);
	return 0;
}
