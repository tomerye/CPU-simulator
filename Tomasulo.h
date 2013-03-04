#pragma once
#include <queue>

#define TRACEFILE "trace1.txt"

using namespace std;

typedef struct reservationEntry_ {
	bool busy;
	string op;
	int vj;
	int vk;
	int qj;
	int qk;
	int state;
} reservationEntry;

class Tomasulo
{
public:
	Tomasulo(int addsub_delay,
		int mul_delay,
		int div_delay,
		int instruction_q_depth,
		int addsub_rs,
		int muldiv_rs,
		int load_q_depth);
	~Tomasulo(void);

	bool addToQueue(vector<std::string>);
	bool isFull();
	void doWork();

private:
	int pc;
	queue<vector<std::string>> instructionsQueue;
	struct registerStatus_ {
		int vi;
		int qi;
		int rs;
	} registersStatus[32];

	//Reservation stations
	reservationEntry* addsubRS;
	reservationEntry* muldivRS;
	reservationEntry* loadRS;
	reservationEntry* storeRS;

	int addsub_delay;
	int mul_delay;
	int div_delay;
	int instruction_q_depth;
	int addsub_rs;
	int muldiv_rs;
	int load_q_depth;
	int store_q_depth;
};

