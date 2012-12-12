#include "stdafx.h"

L1Cache l1Cache;

L2Cache l2Cache;

ConfigurationStruct* confStruct;

void InitCaches(ConfigurationStruct* cs) 
{
	//L1 init
	int entriesNumber = (cs->l1_cache_size)/(cs->l1_block_size);
	l1Cache.cache = new DirectMappedCacheEntry[entriesNumber];
	memset(l1Cache.cache, 0 , entriesNumber * sizeof(DirectMappedCacheEntry));
	l1Cache.blockSize = cs->l1_block_size;
	l1Cache.cacheLength = entriesNumber;

	for (int i = 0; i < entriesNumber; i++) {
		l1Cache.cache[i].block = new int[l1Cache.blockSize/sizeof(int)];
	}

	//L2 init
	entriesNumber = ((cs->l2_cache_size)/(ASSOCIATIVITY))/(cs->l2_block_size);
	l2Cache.cache = new MultiWayCacheEntry*[entriesNumber];

	l2Cache.blockSize = cs->l2_block_size;
	l2Cache.cacheLength = entriesNumber;

	for (int i = 0; i < l2Cache.cacheLength; i++) {
		l2Cache.cache[i] = new MultiWayCacheEntry[ASSOCIATIVITY];
		memset(l2Cache.cache[i],0,sizeof(MultiWayCacheEntry)*ASSOCIATIVITY);
		for (int j = 0; j < ASSOCIATIVITY; j++) {
			l2Cache.cache[i][j].block = new int[l2Cache.blockSize/sizeof(int)];
		}
	}
	confStruct = cs;
}

int IsWordReadyInBlock(int wordOffset,int blockSize, BlockState* bs)
{
	int wordNum = wordOffset / sizeof(int);
	if ((((bs->wordStartedOn) + (bs->wordsGotten))%(blockSize/4)) >= wordNum) {
		return 1;
	}
	return 0;
}

int GetWordFromBlock(int wordOffset, int* block) 
{
	int wordNum = wordOffset/sizeof(int);
	return block[wordNum];

}

int LoadWord(int address,int* word)
{
	//check for it on L1
	int cyclesSoFar = 1;
	int entryNumL1 = GetCacheEntryNumber(address,l1Cache.blockSize,l1Cache.cacheLength);
	int wordOffsetL1 = GetOffset(address,l1Cache.blockSize,l1Cache.cacheLength);
	if (l1Cache.cache[entryNumL1].valid) {
		if (l1Cache.cache[entryNumL1].tag == GetAddressTag(address,l1Cache.blockSize,l1Cache.cacheLength)) {
			if (IsWordReadyInBlock(wordOffsetL1,l1Cache.blockSize,&(l1Cache.cache[entryNumL1].blockState))) {
				//hit on l1
				l1Cache.hits++;
			} else {
				//miss but block is loading
				l1Cache.misses++;
				//wait for the word we want to load
				while(!IsWordReadyInBlock(wordOffsetL1,l1Cache.blockSize,&(l1Cache.cache[entryNumL1].blockState))) {
					DoWork();
					cyclesSoFar++;
				}
			}
			*word = GetWordFromBlock(wordOffsetL1,l1Cache.cache[entryNumL1].block);
			return cyclesSoFar;
		}
	}
	//l1 missed
	l1Cache.misses++;
	cyclesSoFar = confStruct->l2_access_delay;
	//check for it on L2
	int entryNumL2 = GetCacheEntryNumber(address,l2Cache.blockSize,l2Cache.cacheLength);
	int addressTagL2 = GetAddressTag(address,l2Cache.blockSize,l2Cache.cacheLength);
	int wordOffsetL2 = GetOffset(address,l2Cache.blockSize,l2Cache.cacheLength);
	for (int i = 0 ; i < ASSOCIATIVITY; i++) {
		if (l2Cache.cache[entryNumL2][i].valid && l2Cache.cache[entryNumL2][i].tag == addressTagL2) {
			if (IsWordReadyInBlock(wordOffsetL2,l2Cache.blockSize,&(l2Cache.cache[entryNumL2][i].blockState))) {
				//hit on l2
				l2Cache.hits++;
			} else {
				//miss but block is loading
				l2Cache.misses++;
				//wait for the word we want to load
				while (!IsWordReadyInBlock(wordOffsetL2,l2Cache.blockSize,&(l2Cache.cache[entryNumL2][i].blockState))) {
					DoWork();
					cyclesSoFar++; // TODO: HOW MANY CYCLES??
				}
			}
			*word = GetWordFromBlock(wordOffsetL2,l2Cache.cache[entryNumL2][i].block);
			SetLRU();
			LoadToL1(); //DONT FORGET TO SET VALID BIT
			return cyclesSoFar;
		}
	}
	//L2 Totally missed


	//TODO: CONTINUE FROM HERE


	//get it from Disk
}



int PCtoAddress(int pc)
{
	pc=pc*4;
	return 0x00F00000+pc;
}

int GetCacheEntryNumber(int address,int blockSize, int cacheLength) 
{
	address = address>>((int)log((double)blockSize));
	int mask = 0;
	for (int i = 0 ; i < (int)log((double)cacheLength) ; i++ ) {
		mask = (mask << 1);
		mask += 1;
	}
	return (mask & address) % cacheLength;
}

int GetOffset(int address,int blockSize, int cacheLength)
{
	int mask = 0;
	for (int i = 0; i < (int)log((double)blockSize); i++ ) {
		mask = mask << 1;
		mask = mask + 1;
	}
	return (mask & address);
}

int GetAddressTag(int address,int blockSize, int cacheLength)
{
	return (address >> ((int)log((double)cacheLength) + (int)log((double)blockSize)));
}