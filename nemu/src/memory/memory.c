#include "common.h"
#include "burst.h"
#include "nemu.h"
#include "memory/cache.h"
#include "memory/tlb.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

lnaddr_t seg_translate(swaddr_t addr,size_t len,uint8_t sreg) {
	if(cpu.cr0.protect_enable==0) return addr;
	return cpu.SREG[sreg].base+addr;
}

hwaddr_t page_translate(lnaddr_t addr) {
	if(cpu.cr0.protect_enable==0 || cpu.cr0.paging==0) return addr;
	uint32_t DIR=addr>>22;
	uint32_t PAGE=(addr>>12)&0x3ff;
	uint32_t OFFSET=addr&0xfff;
	
	int i;
	i=read_tlb(addr);
	if(i!=-1) return (tlb[i].page_num<<12)+OFFSET;

	Page_Descriptor first;
	//printf("cpu.cr3  %08x\n",cpu.cr3.page_directory_base);
	uint32_t dir_start=cpu.cr3.page_directory_base;
	uint32_t dir_entry=(dir_start<<12)+(DIR<<2);
	first.val=hwaddr_read(dir_entry,4);
	Assert(first.p==1,"DIR cannot be used.");
	
	Page_Descriptor second;
	uint32_t page_start=first.addr;
	uint32_t page_entry=(page_start<<12)+(PAGE<<2);
	second.val=hwaddr_read(page_entry,4);
	Assert(second.p==1,"PAGE cannot be used.");

	uint32_t addr_start=second.addr;
	hwaddr_t ans=(addr_start<<12)+OFFSET;
	write_tlb(addr,ans);
	return ans;
}

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	int cache_1_index=read_L1_cache(addr);
	uint32_t bias=addr&(cache_B-1);
	uint8_t temp[BURST_LEN*2];
	if(bias+len<=cache_B) {
		memcpy(temp,cache_L1[cache_1_index].data+bias,len);
	}
	else {
		int cache_2_index=read_L1_cache(addr+cache_B-bias);
		memcpy(temp,cache_L1[cache_1_index].data+bias,cache_B-bias);
		memcpy(temp+cache_B-bias,cache_L1[cache_2_index].data,len-(cache_B-bias));
	}
	int flag=0;
	uint32_t ans=unalign_rw(temp + flag, 4) & (~0u >> ((4 - len) << 3));
	return ans;

       // return dram_read(addr, len) & (~0u >> ((4 - len) << 3));
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	write_L1_cache(addr,len,data);
	//dram_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t bias=addr&0xfff;
	if(bias+len-1>0xfff) {
		size_t valid_len=0xfff-bias+1;
		uint32_t val1=lnaddr_read(addr,valid_len);
		uint32_t val2=lnaddr_read(addr+valid_len,len-valid_len);
		uint32_t ans=(val2<<8) | val1;
		return ans;
	}
	else {
		hwaddr_t hwaddr=page_translate(addr);
		return hwaddr_read(hwaddr,len);
	}
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	uint32_t bias=addr&0xfff;
	if(bias+len-1>0xfff) {
		size_t valid_len=0xfff-bias+1;
		lnaddr_write(addr,valid_len,data&((1<<(valid_len<<3))-1));
		lnaddr_write(addr+valid_len,len-valid_len,data>>(valid_len<<3));
	}
	else {
		hwaddr_t hwaddr=page_translate(addr);
		hwaddr_write(hwaddr,len,data);
	}
}

uint32_t swaddr_read(swaddr_t addr, size_t len, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr=seg_translate(addr,len,sreg);
	return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data, uint8_t sreg) {
#ifdef DEBUG
	assert(len == 1 || len == 2 || len == 4);
#endif
	lnaddr_t lnaddr=seg_translate(addr,len,sreg);
	lnaddr_write(lnaddr, len, data);
}

