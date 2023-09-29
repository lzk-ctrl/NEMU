#ifndef __TLB_H__
#define __TLB_H__

#include "common.h"

#define tlb_size 64

typedef struct{
	uint32_t page_num;
	uint32_t tag;
	bool valid;
}tlb_struct;

tlb_struct tlb[tlb_size];

void init_tlb();
int read_tlb(lnaddr_t);
void write_tlb(lnaddr_t,hwaddr_t);

#endif
