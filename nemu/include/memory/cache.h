#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"
#include "stdlib.h"
typedef struct
{
    uint8_t *cache_blocks;
    uint32_t *tags;
    bool *valid;
    uint32_t block_size;
    uint32_t size;
    uint32_t associate;
    struct CACHE *next;
    uint8_t state;
} CACHE;

CACHE L1_cache;

void init_cache();

uint32_t read_cache(CACHE *this, uint32_t addr, int len);
void write_cache(CACHE *this, uint32_t addr, int len, uint32_t data);

#endif
