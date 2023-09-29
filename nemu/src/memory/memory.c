#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "burst.h"

#include "macro.h"
#include <stdlib.h>
#include "memory/tlb.h"
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len)
{
	// return dram_read(addr, len) & (~0u >> ((4 - len) << 3));

	int32_t s = (addr >> 6) & (128 - 1);
	int32_t i = readcache(addr);
	int32_t imm_l1 = (addr & (64 - 1));
	int8_t t[64 * 2];
	if (imm_l1 + len > 64)
	{
		memcpy(t, cache[s].set[i].block + imm_l1, 64 - imm_l1);
		int32_t ilast = readcache(addr + 64 - imm_l1);
		int32_t slast = ((addr + 64 - imm_l1) >> 6) & (128 - 1);
		memcpy(t + 64 - imm_l1, cache[slast].set[ilast].block, len - (64 - imm_l1));
	}

	else
		memcpy(t, cache[s].set[i].block + imm_l1, len);
	int qwq = 0;
	uint32_t ret = unalign_rw(t + qwq, 4) & (~0u >> ((4 - len) << 3));
	return ret;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data)
{
	// dram_write(addr, len, data);
	writecache(addr, len, data);
}

// uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
// 	return hwaddr_read(addr, len);
// }

// void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
// 	hwaddr_write(addr, len, data);
// }
int read_tlb(lnaddr_t addr);
void write_tlb(lnaddr_t addr, hwaddr_t haaddr);

hwaddr_t page_translate(lnaddr_t addr)
{
	if (!cpu.cr0.protect_enable || !cpu.cr0.paging)
		return addr;
	/* addr = 10 dictionary + 10 page + 12 offset */
	uint32_t dictionary = addr >> 22, page = (addr >> 12) & 0x3ff, offset = addr & 0xfff;
	int index = read_tlb(addr);
	if (index != -1)
		return (tlb[index].data << 12) + offset;
	uint32_t tmp = (cpu.cr3.page_directory_base << 12) + dictionary * 4;
	Page_info dictionary_, page_;
	dictionary_.val = hwaddr_read(tmp, 4);
	tmp = (dictionary_.addr << 12) + page * 4;
	page_.val = hwaddr_read(tmp, 4);
#ifdef DEBUG_page_p
	printf("eip:0x%x\taddr 0x%x\n", cpu.eip, addr);
#endif
	Assert(dictionary_.p == 1, "dirctionary present != 1");
	Assert(page_.p == 1, "second page table present != 1");
#ifdef DEBUG_page
	printf("0x%x\n", (page_.addr << 12) + offset);
#endif
	hwaddr_t addr_ = (page_.addr << 12) + offset;
	write_tlb(addr, addr_);
	return addr_;
}
uint32_t lnaddr_read(lnaddr_t addr, size_t len)
{
	assert(len == 1 || len == 2 || len == 4);
	uint32_t offset = addr & 0xfff;
	if ((int64_t)(offset + len) > 0x1000)
	{
		// assert(0);
		size_t l = 0xfff - offset + 1;
		uint32_t down_val = lnaddr_read(addr, l);
		uint32_t up_val = lnaddr_read(addr + l, len - l);
		return (up_val << (l * 8)) | down_val;
	}
	else
	{
		hwaddr_t hwaddr = page_translate(addr);
		return hwaddr_read(hwaddr, len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data)
{
	assert(len == 1 || len == 2 || len == 4);
	uint32_t offset = addr & 0xfff;
	if ((int64_t)(offset + len) > 0x1000)
	{
		size_t l = 0xfff - offset + 1;
		lnaddr_write(addr, l, data & ((1 << (l * 8)) - 1));
		lnaddr_write(addr + l, len - l, data >> (l * 8));
		// assert(0);
	}
	else
	{
		hwaddr_t hwaddr = page_translate(addr);
		hwaddr_write(hwaddr, len, data);
	}
}

lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg)
{
	if (cpu.cr0.protect_enable == 0)
		return addr;
	return cpu.sreg[sreg].base + addr;
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg)
{
	assert(len == 1 || len == 2 || len == 4);
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg)
{
	assert(len == 1 || len == 2 || len == 4);
	lnaddr_t lnaddr = seg_translate(addr, len, sreg);
	lnaddr_write(lnaddr, len, data);
}
// uint32_t swaddr_read(swaddr_t addr, size_t len) {
// #ifdef DEBUG
// 	assert(len == 1 || len == 2 || len == 4);
// #endif
// 	return lnaddr_read(addr, len);
// }

// void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
// #ifdef DEBUG
// 	assert(len == 1 || len == 2 || len == 4);
// #endif
// 	lnaddr_write(addr, len, data);
// }