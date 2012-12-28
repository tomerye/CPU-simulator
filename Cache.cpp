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

void SetLRU(int lastUsedBlockNum,MultiWayCacheEntry* line)
{
	for (int i = 0; i < ASSOCIATIVITY ; i++ ) {
		line[i].lru = 0;
	}
	line[lastUsedBlockNum].lru = 1;
}

//Loads a block to L1 cache (used by l2 / ram)
void LoadToL1(int address,int* block)
{
	int offset = GetOffset(address,l1Cache.blockSize,l1Cache.cacheLength);
	int tag = GetAddressTag(address,l1Cache.blockSize,l1Cache.cacheLength);
	int entryNumber = GetCacheEntryNumber(address,l1Cache.blockSize,l1Cache.cacheLength);
	DirectMappedCacheEntry* cacheEntry = &(l1Cache.cache[entryNumber]);
	cacheEntry->tag = tag;
	cacheEntry->valid = 1;
	int l2Offset = GetOffset(address,l2Cache.blockSize,l2Cache.cacheLength);
	int wordsInL2Block = l2Cache.blockSize/sizeof(int);
	int wordsInL1Block = l1Cache.blockSize/sizeof(int);
	memcpy(cacheEntry->block,block+((l2Offset/wordsInL1Block)*(wordsInL2Block/wordsInL1Block)),l1Cache.blockSize);
	cacheEntry->blockState.wordsGotten = 1;
	cacheEntry->blockState.wordStartedOn = l2Offset/wordsInL1Block;
}


void LoadToL2(int address, int* block)
{
	int offset = GetOffset(address,l2Cache.blockSize,l2Cache.cacheLength);
	int tag = GetAddressTag(address,l2Cache.blockSize,l2Cache.cacheLength);
	int lineNumber = GetCacheEntryNumber(address,l2Cache.blockSize,l2Cache.cacheLength);
	MultiWayCacheEntry* line = l2Cache.cache[lineNumber];
	int i;
	for (i = 0; i < ASSOCIATIVITY; i++) {
		if (line[i].valid == 0)
			break;
	}
	if (line[i].valid) {
		for (i = 0; i < ASSOCIATIVITY; i++) {
			if (!line[i].lru)
				break;
		}
	}
	SetLRU(i,line);
	line[i].dirty = 0;
	line[i].valid = 1;
	line[i].tag = tag;
	line[i].blockState.wordsGotten = 1;
	line[i].blockState.wordStartedOn = offset;
	memcpy(line[i].block,block,l2Cache.blockSize);
}

void GetBlockFromMem(int address,int blockSize,int* block)
{
	int wordAddress = address/sizeof(int);
	int blockAddress = wordAddress/(blockSize/4);
	memcpy(block,ram+blockAddress,blockSize);
}

int LoadWord(int address,int* word)
{
	//check for it on L1
	int cyclesSoFar = confStruct->l1_access_delay;
	for (int i = 0; i < confStruct->l1_access_delay; i++)
		DoWork();
	int entryNumL1 = GetCacheEntryNumber(address,l1Cache.blockSize,l1Cache.cacheLength);
	int wordOffsetL1 = GetOffset(address,l1Cache.blockSize,l1Cache.cacheLength);
	if (l1Cache.cache[entryNumL1].valid) {
		if (l1Cache.cache[entryNumL1].tag == GetAddressTag(address,l1Cache.blockSize,l1Cache.cacheLength)) {
			//Block is in cache and loading / ready
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
			// return the word and num of cycles
			*word = GetWordFromBlock(wordOffsetL1,l1Cache.cache[entryNumL1].block);
			return cyclesSoFar;
		}
		
	}
	//l1 missed
	l1Cache.misses++;
	cyclesSoFar += confStruct->l2_access_delay;

	for (int i = 0; i < confStruct->l2_access_delay; i++)
		DoWork();

	//check for it on L2
	int entryNumL2 = GetCacheEntryNumber(address,l2Cache.blockSize,l2Cache.cacheLength);
	int addressTagL2 = GetAddressTag(address,l2Cache.blockSize,l2Cache.cacheLength);
	int wordOffsetL2 = GetOffset(address,l2Cache.blockSize,l2Cache.cacheLength);
	for (int i = 0 ; i < ASSOCIATIVITY; i++) {
		if (l2Cache.cache[entryNumL2][i].valid && l2Cache.cache[entryNumL2][i].tag == addressTagL2) {
			//Block is already in cache and is loading / ready
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
			SetLRU(i,l2Cache.cache[entryNumL2]);
			LoadToL1(address,l2Cache.cache[entryNumL2][i].block); //DONT FORGET TO SET VALID BIT
			return cyclesSoFar;
		}
	}
	//L2 Totally missed
	l2Cache.misses++;
	cyclesSoFar += confStruct->mem_access_delay;
	for (int i = 0; i< confStruct->mem_access_delay; i++)
		DoWork();
	int* block = new int[l2Cache.blockSize];
	GetBlockFromMem(address,l2Cache.blockSize,block);
	LoadToL2(address,block);
	LoadToL1(address,block);
	*word = ram[address/sizeof(int)];
	return cyclesSoFar;
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

//TODO: WHAT HAPPENS WHEN DESTROYING AND CACHES STILL GOT WORK TO DO
void DestroyCaches()
{
	if (confStruct = NULL) {
		return;
	}
	
	//destroying L1 cache
	int l1EntriesNumber = (confStruct->l1_cache_size)/(confStruct->l1_block_size);
	for (int i = 0; i < l1EntriesNumber; i++) {
		delete[] l1Cache.cache[i].block;
	}
	delete[] l1Cache.cache;

	//destroying L2 cache
	int l2EntriesNumber = ((confStruct->l2_cache_size)/(ASSOCIATIVITY))/(confStruct->l2_block_size);
	for (int i = 0; i < l2EntriesNumber; i++) {
		for (int j = 0; j < ASSOCIATIVITY; j++) {
			delete[] l2Cache.cache[i][j].block;
		}
		delete[] l2Cache.cache[i];
	}
	delete[] l2Cache.cache;
}

void DoWork() {
	//Do work in l1 cache
	BlockState* bs;
	for (int i = 0; i < l1Cache.cacheLength; i++) {
		if (!l1Cache.cache[i].valid)
			continue;
		bs = &(l1Cache.cache[i].blockState);
		//The last word is ready in block
		if (!IsWordReadyInBlock((bs->wordStartedOn-1)%l1Cache.blockSize,l1Cache.blockSize,bs)) {
			bs->wordsGotten++;
		}
		break;	
	}
	//Do work in l2 cache
	for (int i = 0; i < l2Cache.cacheLength; i++) {
		for (int j = 0; j < ASSOCIATIVITY) {
			if (!l2Cache.cache[i][j].valid)
				continue;
			bs = &(l2Cache.cache[i][j].blockState);
			if (!IsWordReadyInBlock((bs->wordStartedOn-1)%l2Cache.blockSize,l2Cache.blockSize,bs)) {
				bs->wordsGotten++;
				return;
			}
		}
	}
}