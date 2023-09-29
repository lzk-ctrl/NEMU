#include "memory/segment.h"
#include "nemu.h"

uint32_t lnaddr_read(lnaddr_t, size_t);

void load_sregment(uint8_t sreg, uint16_t selec){


	assert(sreg < 6);
    
	cpu.sreg[sreg].val = selec;

	assert(cpu.sreg[sreg].ti == 0);

	uint32_t temp[2];
	lnaddr_t base = cpu.gdtr.base + (selec & ~0x7);
	temp[0] = lnaddr_read(base, 4);
	temp[1] = lnaddr_read(base + 4, 4);

	SegDesc *s = (void *)temp;

	assert(s->present);
	assert(s->operation_size == 0);
	assert(s->granularity == 1);

	cpu.sreg[sreg].base = s->base_15_0 | (s->base_23_16 << 16) | (s-> base_31_24  << 24);
	cpu.sreg[sreg].limit = ((s->limit_15_0 | (s->limit_19_16 << 16)) << 12) | 0xfff;
	cpu.sreg[sreg].type = s->type | (s->segment_type << 4);
	cpu.sreg[sreg].pri_lev = s->privilege_level;
	cpu.sreg[sreg].soft_use = s->soft_use;
}

lnaddr_t segment_translate(swaddr_t addr, uint8_t sreg){
#ifdef DEBUG
	assert(sreg < 6);
#endif
	if (cpu.cr0.PE)
	{
		return cpu.sreg[sreg].base + addr;
	}
    
	return addr;
}
