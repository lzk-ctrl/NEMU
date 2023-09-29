#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

typedef struct
{
    bool valid;
    uint32_t tag;
    uint8_t block[64];
} blocks;
typedef struct
{
    blocks set[8];
} sets;
sets cache[1024 / 8];

uint64_t timer;

int32_t readcache(hwaddr_t addr);
void writecache(hwaddr_t addr, size_t len, uint32_t data);

#endif