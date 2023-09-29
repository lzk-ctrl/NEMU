#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define E 8
#define CACHE_BLOCK 64
#define CACHE_SIZE 64*1024

struct cache_l1 {
    bool valid;
    int tag;
    uint8_t byte[CACHE_BLOCK];
} cache[CACHE_SIZE / CACHE_BLOCK];

uint64_t cnt;

void init_cache();
uint32_t cache_read(hwaddr_t addr);
void cache_write(hwaddr_t addr,size_t len,uint32_t data);

#endif
