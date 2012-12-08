#include "CPU-simulator.h"

#ifndef CACHE_H
#define CACHE_H

#define ASSOCIATIVITY 2

typedef struct {
	int valid;
	int tag;
	int* block;
} DirectMappedCacheEntry;

typedef struct {
	int valid;
	int dirty;
	int lru;
	int tag;
	int* block;
} MultiWayCacheEntry;

typedef struct {
	DirectMappedCacheEntry* cache;
	int cacheLength;
	int blockSize;
} L1Cache;

typedef struct {
	MultiWayCacheEntry** cache;
	int cacheLength;
	int blockSize;
} L2Cache;

void InitCaches(ConfigurationStruct* cs);
//Returns the level of cache needed to hit, 1 for l1, 2 for l2, 3 for Disk
int LoadInstruction(int pc, ConfigurationStruct* cs);

#endif

