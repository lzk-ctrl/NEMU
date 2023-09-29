#include "burst.h"
#include "common.h"
#include "cpu/reg.h"
#include "memory/cache.h"
#include "memory/tlb.h"
uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len) {
	uint32_t offset = addr & (CACHE_BLOCK - 1);
	uint32_t block = cache_read(addr);
	uint8_t temp[4];
	memset(temp, 0, sizeof(temp));
	if (offset + len >= CACHE_BLOCK) {
		uint32_t second_block = cache_read(addr + len);
		memcpy(temp, cache[block].byte + offset, CACHE_BLOCK - offset);
		memcpy(temp + CACHE_BLOCK - offset, cache[second_block].byte, len - (CACHE_BLOCK - offset));
	} else {
		memcpy(temp, cache[block].byte + offset, len);
	}
	int zero = 0;
	uint32_t result = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3));
	//printf("time: %ld\n", cnt);
	return result;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data) {
	cache_write(addr, len, data);
}
lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg) {
  return addr + cpu.segReg[sreg].invisiblePart.base_15_0 +
	  (cpu.segReg[sreg].invisiblePart.base_23_16 << 16) +
	  (cpu.segReg[sreg].invisiblePart.base_31_24 << 24);
}
hwaddr_t page_translate(lnaddr_t addr, size_t len) {
  uint32_t offset = (addr & 0xfff);
  uint32_t pageIndex = (addr >> 12) & 0x3ff;
  uint32_t dirIndex = (addr >> 22) & 0x3ff;
  assert(offset + len - 1 <= 0xfff);
  /*read tlb*/
  uint32_t tlb_ans = read_tlb((addr>>12)&0xFFFFF);
  if(tlb_ans!=-1){
	return (tlb_ans<<12)+offset;
  }
  /* find Page table address*/
  union PageDirectoryEntry pageTable;
  pageTable.val = hwaddr_read((cpu.cr3.page_directory_base << 12) + dirIndex * sizeof(union PageDirectoryEntry),
							  sizeof(union PageDirectoryEntry));
  assert(pageTable.present == 1);

  /* find Page frame address*/
  union PageTableEntry pageFrame;
  pageFrame.val = hwaddr_read((pageTable.page_frame << 12) + pageIndex * sizeof(union PageTableEntry),
							  sizeof(union PageTableEntry));
  assert(pageFrame.present == 1);
  /*update tlb*/
  update_tlb((addr >> 12)&0xFFFFF,pageFrame.page_frame);

  return (pageFrame.page_frame << 12) + offset;
}
uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
  if (cpu.cr0.paging && cpu.cr0.protect_enable) {
	uint32_t offset = addr & 0xfff;
	if (offset + len - 1 > 0xfff) {
	  size_t l = 0xfff - offset + 1;
	  uint32_t addr_r = lnaddr_read(addr, l);
	  uint32_t addr_l = lnaddr_read(addr + l, len - l);
	  uint32_t val = (addr_l << (l << 3)) | addr_r;
	  return val;
	}
	hwaddr_t hwaddr = page_translate(addr, len);
	return hwaddr_read(hwaddr, len);
  }
  return hwaddr_read(addr, len);
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
  if (cpu.cr0.protect_enable && cpu.cr0.paging) {
	uint32_t offset = addr & 0xfff;
	if (offset + len - 1 > 0xfff) {
	  size_t l = 0xfff - offset + 1;
	  lnaddr_write(addr, l, data & ((1 << (l << 3)) - 1));
	  lnaddr_write(addr + l, len - l, data >> (l << 3));
	  return;
	} else {
	  hwaddr_t hwaddr = page_translate(addr, len);
	  return hwaddr_write(hwaddr, len, data);
	}
  }
  hwaddr_write(addr, len, data);
}


uint32_t swaddr_read(swaddr_t addr, size_t len) {
#ifdef DEBUG
  assert(len == 1 || len == 2 || len == 4);
#endif
  lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
  return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data) {
#ifdef DEBUG
  assert(len == 1 || len == 2 || len == 4);
#endif
  lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
  lnaddr_write(lnaddr, len, data);
}
