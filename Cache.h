#include "CPU-simulator.h"

#ifndef CACHE_H
#define CACHE_H

#define ASSOCIATIVITY 2

typedef struct {
	int wordStartedOn;
	int wordsGotten;
} BlockState;

typedef struct {
	int valid;
	int tag;
	int isLoading;
	int* block;
	BlockState blockState;
} DirectMappedCacheEntry;

typedef struct {
	int valid;
	int dirty;
	int lru;
	int tag;
	int isLoading;
	int* block;
	BlockState blockState;
} MultiWayCacheEntry;

typedef struct {
	DirectMappedCacheEntry* cache;
	int cacheLength;
	int blockSize;
	int hits;
	int misses;
} L1Cache;

typedef struct {
	MultiWayCacheEntry** cache;
	int cacheLength;
	int blockSize;
	int hits;
	int misses;
} L2Cache;

void InitCaches(ConfigurationStruct* cs);

void DestroyCaches();

double GetL1HitRate();

double GetL2HitRate();

int LoadWord();

//note: should take care of isLoading field and BlockStatus Struct
int DoWork();

int PCtoAddress(int pc);

int GetCacheEntryNumber(int address,int blockSize, int cacheLength);

int GetOffset(int address,int blockSize, int cacheLength);

int GetAddressTag(int address,int blockSize, int cacheLength);


#endif

