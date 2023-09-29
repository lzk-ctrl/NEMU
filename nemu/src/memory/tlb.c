#include "common.h"
#include "memory/tlb.h"
#include "burst.h"
#include <time.h>
#include <stdlib.h>

void init_tlb() {
	int i;
	for(i=0;i<tlb_size;i++) {
		tlb[i].valid=false;
	}
	return;
}

int read_tlb(lnaddr_t aim) {
	uint32_t tag=aim>>12;
	int i;
	for(i=0;i<tlb_size;i++) {
		if(tlb[i].valid==0) continue;
		if(tlb[i].tag==tag) 
			return i;
	}
	return -1;
}

void write_tlb(lnaddr_t lnaddr,hwaddr_t hwaddr) {
	uint32_t tag=lnaddr>>12;
	uint32_t page=hwaddr>>12;
	int i;
	for(i=0;i<tlb_size;i++) {
		if(tlb[i].valid==1) continue;
		tlb[i].valid=true;
		tlb[i].tag=tag;
		tlb[i].page_num=page;
		return;
	}
	srand(time(0));
	i=rand()%tlb_size;
	tlb[i].valid=true;
	tlb[i].tag=tag;
	tlb[i].page_num=page;
	return;
}

