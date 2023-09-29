#include "memory/cache.h"
#include "burst.h"
#include <stdlib.h>
#include <time.h>
#include "cpu/reg.h"

void dram_write(hwaddr_t addr, size_t len, uint32_t data);
void cache_ddr3_read(hwaddr_t addr, void *data);
void cache_ddr3_write(hwaddr_t addr, void *data, uint8_t *mask);

void init_cache()
{
    timer = 0;
    srand(clock());
    int i, j;
    for (i = 0; i < 128; i++)
    {
        for (j = 0; j < 8; j++)
        {
            cache[i].set[j].valid = 0;
        }
    }
}
int32_t readcache(hwaddr_t addr)
{
    int32_t t = (addr >> (7 + 6));
    int32_t s = (addr >> 6) & (128 - 1);
    int i;
    for (i = 0; i < 8; i++)
    {
        if (!cache[s].set[i].valid)
            continue;
        if (cache[s].set[i].valid && cache[s].set[i].tag == t)
        {
            timer += 2;
            return i;
        }
    }
    /* miss */
    for (i = 0; i < 8; i++)
    {
        if (!cache[s].set[i].valid)
            break;
    }
    if (i == 8)
        i = rand() % 8;
    cache[s].set[i].valid = 1;
    cache[s].set[i].tag = t;
    int j;
    for (j = 0; j < 64 / BURST_LEN; j++)
    {
        cache_ddr3_read(((addr >> 6) << 6) + BURST_LEN * j, cache[s].set[i].block + BURST_LEN * j);
    }
    timer += 200;
    return i;
}

void writecache(hwaddr_t addr, size_t len, uint32_t data)
{
    /* write through	&	not write allocate */
    int32_t t = (addr >> (7 + 6));
    int32_t s = (addr >> 6) & (128 - 1);
    int32_t imm = (addr & (64 - 1));
    bool hit = false;
    int i;
    for (i = 0; i < 8; i++)
    {
        if (!cache[s].set[i].valid)
            continue;
        if (cache[s].set[i].tag == t)
        {
            hit = true;
            break;
        }
    }
    if (hit)
    {
        if (imm + len <= 64)
        {
            memcpy(cache[s].set[i].block + imm, &data, len);
            dram_write(addr, len, data);
        }
        else
        {
            memcpy(cache[s].set[i].block + imm, &data, 64 - imm); // 低位低地址
            dram_write(addr, 64 - imm, data);
            writecache(addr + 64 - imm, len - (64 - imm), data >> 8 * (64 - imm)); // 高位高地址
        }
        timer += 2;
    }
    else
    {
        dram_write(addr, len, data);
        timer += 200;
    }
}