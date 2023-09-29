#include "common.h"
#include "cache.h"
#include "cpu/reg.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len)
{

	uint32_t offset = addr & (BLOCK_SIZE - 1); 
	uint32_t block = cache_read(addr);
	uint8_t temp[4];
	memset(temp, 0, sizeof(temp));

	if (offset + len >= BLOCK_SIZE)
	{
		uint32_t _block = cache_read(addr + len);
		memcpy(temp, cache[block].data + offset, BLOCK_SIZE - offset);
		memcpy(temp + BLOCK_SIZE - offset, cache[_block].data, len - (BLOCK_SIZE - offset));
	}
	else
	{
		memcpy(temp, cache[block].data + offset, len);
	}
	int zero = 0;
	uint32_t tmp = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3));
	return tmp;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data)
{
	cache_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
	return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
	hwaddr_write(addr, len, data);
}

uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	return lnaddr_read(addr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_write(addr, len, data);
}

