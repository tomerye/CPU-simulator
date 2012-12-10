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
	int entryNum = GetCacheEntryNumber(address,l1Cache.blockSize,l1Cache.cacheLength);
	if (l1Cache.cache[entryNum].valid) {
		if (l1Cache.cache[entryNum].tag==GetAddressTag(address,l1Cache.blockSize,l1Cache.cacheLength)) {
			while(!IsWordReadyInBlock()) {
				DoWork();
			}
			GetWordFromBlock(GetOffset(address,l1Cache.blockSize,l1Cache.cacheLength),
		}


	}
	//check for it on L2 if not present

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