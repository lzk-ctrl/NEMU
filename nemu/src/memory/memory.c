#include "common.h"
#include "memory/cache.h"
#include "cpu/reg.h"
#include "memory/tlb.h"
#include "memory/memory.h" 
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



lnaddr_t seg_translate(swaddr_t addr, size_t len, uint8_t sreg_id) {
  if (cpu.cr0.protect_enable == 0) return addr;
  else {
    return cpu.sreg[sreg_id].base + addr;
  }
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

hwaddr_t page_translate(lnaddr_t addr) {
  if (cpu.cr0.protect_enable == 1 && cpu.cr0.paging == 1) {
    uint32_t dir = addr >> 22;
    uint32_t page = (addr >> 12) & 0x3ff;
    uint32_t offset = addr & 0xfff;

    int i = read_tlb(addr);
    if (i != -1) {
      return (tlb[i].page_num << 12) + offset;
    }
    uint32_t dir_start = cpu.cr3.page_directory_base;
    uint32_t dir_position = (dir_start << 12) + (dir << 2);
    Page_Descriptor first_content;
    first_content.val = 0; 
first_content.p = 0;  
    first_content.val = hwaddr_read(dir_position, 4);
    Assert(first_content.p, "pagevalue = %x eip = %x", first_content.val,cpu.eip);
    uint32_t page_start = first_content.addr;
    uint32_t page_pos = (page_start << 12) + (page << 2);
    Page_Descriptor second_content;
    second_content.val = 0;  
second_content.p = 0; 
    second_content.val =  hwaddr_read(page_pos, 4);
    Assert(second_content.p == 1, "Page Cannot Be Used!, %x", cpu.eip);
    uint32_t addr_start = second_content.addr;
    hwaddr_t hwaddr = (addr_start << 12) + offset;
    write_tlb(addr, hwaddr);
    return hwaddr;
  } else {
    return addr;
  }
}
uint32_t lnaddr_read(lnaddr_t addr, size_t len) {
  assert(len == 1 || len == 2 || len == 4);
  uint32_t now_offset = addr & 0xfff;
  if (now_offset + len - 1 > 0xfff) {
    size_t len1 = 0xfff - now_offset + 1;
    size_t len2 = len - len1;
    uint32_t addr_len1 = lnaddr_read(addr, len1);
    uint32_t addr_len2 = lnaddr_read(addr + len1, len2);
    uint32_t value = (addr_len2 << (len1 << 3)) | addr_len1;
    return value;

  } else {
    hwaddr_t hwaddr = page_translate(addr);
    return hwaddr_read(hwaddr, len);
  }
}

void lnaddr_write(lnaddr_t addr, size_t len, uint32_t data) {
  assert(len == 1 || len == 2 || len == 4);
  uint32_t now_offset = addr & 0xfff;
  if (now_offset + len - 1 > 0xfff) {
    size_t len1 = 0xfff - now_offset + 1;
    size_t len2 = len - len1;
    lnaddr_write(addr, len1, data & ((1 << (len1 << 3)) - 1));
    lnaddr_write(addr + len1, len2, data >> (len1 << 3));
  } else {
    hwaddr_t hwaddr = page_translate(addr);
    hwaddr_write(hwaddr, len, data);
  }
}
