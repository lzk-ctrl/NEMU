#ifndef __TLB_H__
#define __TLB_H__

#include "common.h"

#define TLB_SIZE 64

typedef struct
{
    bool valid;
    uint32_t tag, data;
} TLB;

TLB tlb[TLB_SIZE];
void init_TLB();

int read_TLB(lnaddr_t addr);
void write_TLB(lnaddr_t addr, hwaddr_t haaddr);

#endif