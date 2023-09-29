#include "common.h"
#include "memory/cache.h"

uint32_t dram_read(hwaddr_t, size_t);
void dram_write(hwaddr_t, size_t, uint32_t);

int is_mmio(hwaddr_t);
uint32_t mmio_read(hwaddr_t, size_t, int);
void mmio_write(hwaddr_t, size_t, uint32_t, int);
lnaddr_t seg_translate(swaddr_t, size_t, uint8_t);
hwaddr_t page_translate(lnaddr_t addr);
extern uint8_t current_sreg;

/* Memory accessing interfaces */

uint32_t hwaddr_read(hwaddr_t addr, size_t len)
{
    int index = is_mmio(addr);
    if (index >= 0)
    {
        return mmio_read(addr, len, index);
    }
    uint32_t offset = addr & (CACHE_BLOCK_SIZE - 1);
    uint32_t block = cache_read(addr);
    uint8_t temp[4];
    memset(temp, 0, sizeof(temp));
    if (offset + len >= CACHE_BLOCK_SIZE) //addr too long && cache_read again
    {
        uint32_t _block = cache_read(addr + len);
        memcpy(temp, cache[block].data + offset, CACHE_BLOCK_SIZE - offset);
        memcpy(temp + CACHE_BLOCK_SIZE - offset, cache[_block].data, len - (CACHE_BLOCK_SIZE - offset));
    }
    else
    {
        memcpy(temp, cache[block].data + offset, len);
    }
    int zero = 0;
    uint32_t cnt = unalign_rw(temp + zero, 4) & (~0u >> ((4 - len) << 3));
    //printf("time: %d\n", tol_time);
    return cnt;
}

void hwaddr_write(hwaddr_t addr, size_t len, uint32_t data)
{
    int index = is_mmio(addr);
    if (index >= 0)
    {
        mmio_write(addr, len, data, index);
        return;
    }
    cache_write(addr, len, data);
}

uint32_t lnaddr_read(lnaddr_t addr, size_t len)
{
    assert(len == 1 || len == 2 || len == 4);
    uint32_t offset = addr & 0xfff;
    if ((int64_t)(offset + len) > 0x1000)
    {
        //assert(0);
        size_t l = 0xfff - offset + 1;                    // 低位最多几个字节同页
        uint32_t down_val = lnaddr_read(addr, l);         // 低位
        uint32_t up_val = lnaddr_read(addr + l, len - l); //高位
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
        //assert(0);
        size_t l = 0xfff - offset + 1;                      // 低位最多几个字节同页
        lnaddr_write(addr, l, data & ((1 << (l * 8)) - 1)); // 低位
        lnaddr_write(addr + l, len - l, data >> (l * 8));   //高位
    }
    else
    {
        hwaddr_t hwaddr = page_translate(addr);
        hwaddr_write(hwaddr, len, data);
    }
}

uint32_t swaddr_read(swaddr_t addr, size_t len)
{
#ifdef DEBUG
    assert(len == 1 || len == 2 || len == 4);
#endif
    lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
    return lnaddr_read(lnaddr, len);
}

void swaddr_write(swaddr_t addr, size_t len, uint32_t data)
{
#ifdef DEBUG
    assert(len == 1 || len == 2 || len == 4);
#endif
    lnaddr_t lnaddr = seg_translate(addr, len, current_sreg);
    lnaddr_write(lnaddr, len, data);
}
