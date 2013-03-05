#include "StdAfx.h"
#include "Tomasulo.h"


Tomasulo::Tomasulo(
	int addsub_delay,
	int mul_delay,
	int div_delay,
	int instruction_q_depth,
	int addsub_rs,
	int muldiv_rs,
	int load_q_depth,
	int store_q_depth)
{
	this->pc = 0;
	this->mul_delay = mul_delay;
	this->div_delay = div_delay;
	this->instruction_q_depth = instruction_q_depth;
	this->addsub_rs = addsub_rs;
	this->muldiv_rs = muldiv_rs;
	this->load_q_depth = load_q_depth;
	this->store_q_depth = store_q_depth;

	addsubRS = new reservationEntry[addsub_rs];
	muldivRS = new reservationEntry[muldiv_rs];
	loadRS = new reservationEntry[load_q_depth];
	storeRS = new reservationEntry[store_q_depth];

	for (int i=0; i < addsub_rs; i++) {
		addsubRS[i].busy = false;
	}
	for (int i=0; i < muldiv_rs; i++) {
		muldivRS[i].busy = false;
	}
	for (int i=0; i < load_q_depth; i++) {
		loadRS[i].busy = false;
	}
	for (int i=0; i < store_q_depth; i++) {
		storeRS[i].busy = false;
	}

	memset(this->registersStatus,0,sizeof(this->registersStatus));

}


Tomasulo::~Tomasulo(void)
{
	delete[] addsubRS;
	delete[] muldivRS;
	delete[] loadRS;
	delete[] storeRS;
}

bool Tomasulo::addToQueue(vector<std::string> newInstruction) 
{
	if (this->isInstQueueFull()) {
		return false;
	}
	else
	{
		instructionsQueue.push(newInstruction);
		return true;
	}

}

bool Tomasulo::isInstQueueFull()
{
	return (instructionsQueue.size() == instruction_q_depth);		
}

int Tomasulo::findOpenSpotInRS(reservationEntry* rs,int rsSize) 
{
	for (int i = 0; i<rsSize; i++) {
		if (rs[i].busy == false)
			return i;
	}
	return -1;
}

void Tomasulo::doWork() 
{
	if (!instructionsQueue.empty()) {		
		vector<string> currentInst = instructionsQueue.front();
		if (issue(currentInst)) {
			instructionsQueue.pop();
		}
	}

	for (int i = 0; i < addsub_rs; i++) {
		if (addsubRS[i].state == addsub_delay) {
			addsubRS[i].busy = false;
			//Here to trace instruction finished!
		}
		addsubRS[i].state += 1;
	}
	pc++;
}

bool Tomasulo::issue(vector<string> curInst) 
{
	if (isRSFull(addsubRS,addsub_rs)) {
		return false;
	} else {
		reservationEntry* rse = addsubRS + findOpenSpotInRS(addsubRS,addsub_rs);
		rse->op = curInst[0];
		rse->busy = true;
		rse->state = 0;
	}

	return true;
}

void doWorkForRS(reservationEntry rs, int rsSize) 
{

}

bool Tomasulo::isRSFull(reservationEntry* rs ,int size) 
{
	for (int i = 0; i < size ; i++) {
		if (rs[i].busy == false)
			return false;
	}
	return true;
}
