#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "memory/page.h"

hwaddr_t page_translate(lnaddr_t addr);




uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);
lnaddr_t segment_translate(swaddr_t addr, uint8_t sreg);
/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	if(L1_cache.state==1){

		return read_cache(&L1_cache,addr,len) & (~0u >> ((4 - len) << 3));
	}
	else return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	 if(L1_cache.state==1)write_cache(&L1_cache,addr,len,data);
	else dram_write(addr, len, data);
}





uint32_t lnaddr_read(lnaddr_t addr, size_t len){

	uint32_t val;
	if(cpu.cr0.PG) {
		hwaddr_t hwaddr = page_translate(addr);
		uint32_t remain_byte = PAGE_SIZE - (addr & PAGE_MASK);
		if(remain_byte < len) {
			val = hwaddr_read(hwaddr, remain_byte);

			hwaddr = page_translate(addr + remain_byte);
			val |= hwaddr_read(hwaddr, len - remain_byte) << (remain_byte << 3);
		}
		else {
			val = hwaddr_read(hwaddr, len);
		}
	}
	else {
		
		val = hwaddr_read(addr, len);
	}

	return val;
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data){

	if(cpu.cr0.PG) {

		hwaddr_t hwaddr = page_translate(addr);
		uint32_t remain_byte = PAGE_SIZE - (addr & PAGE_MASK);
		
		if(remain_byte < len) {
			uint32_t cut = PAGE_SIZE - (addr & PAGE_MASK);
			assert(cut < 4);
			hwaddr_write(hwaddr, cut, data);

			hwaddr = page_translate(addr + cut);
			hwaddr_write(hwaddr, len - cut, data >> (cut << 3));
		}
		else {
			hwaddr_write(hwaddr, len, data);
		}
	}
	else {
		hwaddr_write(addr, len, data);
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg){

#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = segment_translate(addr, sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg){

#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr = segment_translate(addr, sreg);
	lnaddr_write(lnaddr, len, data);
}














