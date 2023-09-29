#include "common.h"
#include "burst.h"
#include <stdlib.h>
#include "cache.h"

void ddr3_read(hwaddr_t addr, void *data);
void ddr3_write(hwaddr_t addr, void *data, uint8_t *mask);

void init_cache()
{
	int i;
	for (i = 0; i < STORAGE_SIZE_L1 / BLOCK_SIZE; i++)
	{
		cache[i].valid = false;
		cache[i].tag = 0;
		memset(cache[i].data, 0, BLOCK_SIZE);
	}
	for (i = 0; i < STORAGE_SIZE_L2 / BLOCK_SIZE; i++)
	{
		cache2[i].valid = false;
		cache2[i].dirty = false;
		cache2[i].tag = 0;
		memset(cache2[i].data, 0, BLOCK_SIZE);
	}

}
uint32_t secondarycache_read(hwaddr_t addr)
{
	uint32_t g = (addr >> 6) & ((1 << 12) - 1); 
	uint32_t block = (addr >> 6) << 6;
	int i;
	bool v = false;
	for (i = g * SIXTEEN_WAY; i < (g + 1) * SIXTEEN_WAY; i++)
	{
		if (cache2[i].tag == (addr >> 18) && cache2[i].valid)
		{
			v = true;
			break;
		}
	}
	if (!v)
	{
		int j;
		for (i = g * SIXTEEN_WAY; i < (g + 1) * SIXTEEN_WAY; i++)
		{
			if (!cache2[i].valid)
				break;
		}
		if (i == (g + 1) * SIXTEEN_WAY) 
		{
			srand(0);
			i = g * SIXTEEN_WAY + rand() % SIXTEEN_WAY;
			if (cache2[i].dirty)
			{
				uint8_t mask[BURST_LEN * 2];
				memset(mask, 1, BURST_LEN * 2);
				for (j = 0; j < BLOCK_SIZE / BURST_LEN; j++)
					ddr3_write(block + j * BURST_LEN, cache2[i].data + j * BURST_LEN, mask);
			}
		}
		cache2[i].valid = true;
		cache2[i].tag = addr >> 18;
		cache2[i].dirty = false;
		for (j = 0; j < BURST_LEN; j++)
			ddr3_read(block + j * BURST_LEN, cache2[i].data + j * BURST_LEN);
	}
	return i;
}
uint32_t cache_read(hwaddr_t addr)
{
	uint32_t g = (addr >> 6) & 0x7f; 
	int i;
	bool v = false;
	for (i = g * EIGHT_WAY; i < (g + 1) * EIGHT_WAY; i++)
	{
		if (cache[i].tag == (addr >> 13) && cache[i].valid)
		{
			v = true;
			break;
		}
	}
	if (!v)
	{
		int j = secondarycache_read(addr);
		for (i = g * EIGHT_WAY; i < (g + 1) * EIGHT_WAY; i++)
		{
			if (!cache[i].valid)
				break;
		}
		if (i == (g + 1) * EIGHT_WAY) 
		{
			srand(0);
			i = g * EIGHT_WAY + rand() % EIGHT_WAY;
		}
		cache[i].valid = true;
		cache[i].tag = addr >> 13;
		memcpy(cache[i].data, cache2[j].data, BLOCK_SIZE);
	}
	return i;
}
void secondarycache_write(hwaddr_t addr, size_t len, uint32_t data)
{
	uint32_t g = (addr >> 6) & ((1 << 12) - 1); 
	uint32_t offset = addr & (BLOCK_SIZE - 1);	
	int i;
	bool v = false;
	for (i = g * SIXTEEN_WAY; i < (g + 1) * SIXTEEN_WAY; i++)
	{
		if (cache2[i].tag == (addr >> 13) && cache2[i].valid)
		{
			v = true;
			break;
		}
	}
	if (!v)
		i = secondarycache_read(addr);
	cache2[i].dirty = true;
	memcpy(cache2[i].data + offset, &data, len);
}
void cache_write(hwaddr_t addr, size_t len, uint32_t data)
{
	uint32_t g = (addr >> 6) & 0x7f;		   
	uint32_t offset = addr & (BLOCK_SIZE - 1); 
	int i;
	bool v = false;
	for (i = g * EIGHT_WAY; i < (g + 1) * EIGHT_WAY; i++)
	{
		if (cache[i].tag == (addr >> 13) && cache[i].valid)
		{
			v = true;
			break;
		}
	}
	if (v)
	{
		memcpy(cache[i].data + offset, &data, len);
	}
	secondarycache_write(addr, len, data);
}