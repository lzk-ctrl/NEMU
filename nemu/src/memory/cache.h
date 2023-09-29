#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"
#define BLOCK_SIZE 64
#define STORAGE_SIZE_L1 64*1024
#define STORAGE_SIZE_L2 4*1024*1024
#define EIGHT_WAY 8
#define SIXTEEN_WAY 16

struct Cache
{
	bool valid;
	int tag;
	uint8_t data[BLOCK_SIZE];
}cache[STORAGE_SIZE_L1/BLOCK_SIZE];
struct SecondaryCache
{
	bool valid,dirty;
	int tag;
	uint8_t data[BLOCK_SIZE];
}cache2[STORAGE_SIZE_L2/BLOCK_SIZE];

void init_cache();
uint32_t cache_read(hwaddr_t addr);
uint32_t secind_cache_read(hwaddr_t addr);
void cache_write(hwaddr_t addr,size_t len, uint32_t data);
void second_cache_write(hwaddr_t addr,size_t len, uint32_t data);


#endif