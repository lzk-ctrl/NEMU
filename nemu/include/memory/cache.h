#ifndef __CACHE_H__
#define __CACHE_H__

#include "common.h"

#define cache_b_bits 6
#define cache_B (1<<cache_b_bits)

#define L1_cache_size 64*1024
#define L1_cache_E (1<<L1_cache_e_bits)
#define L1_cache_S (1<<L1_cache_s_bits)
#define L1_cache_e_bits 3
#define L1_cache_s_bits 7

typedef struct{
	uint8_t data[cache_B];
	uint32_t tag;
	bool valid;
}L1_cache_struct;

L1_cache_struct cache_L1[L1_cache_S*L1_cache_E];


#define L2_cache_size 4*1024*1024
#define L2_cache_E (1<<L2_cache_e_bits)
#define L2_cache_S (1<<L2_cache_s_bits)
#define L2_cache_e_bits 4
#define L2_cache_s_bits 12

typedef struct{
	uint8_t data[cache_B];
	uint32_t tag;
	bool valid;
	bool dirty;
}L2_cache_struct;

L2_cache_struct cache_L2[L2_cache_S*L2_cache_E];

void init_cache();
int read_L1_cache(hwaddr_t);
void write_L1_cache(hwaddr_t,size_t,uint32_t);
int read_L2_cache(hwaddr_t);
void write_L2_cache(hwaddr_t,size_t,uint32_t);

#endif
