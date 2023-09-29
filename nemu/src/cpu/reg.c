#include "nemu.h"
#include <stdlib.h>
#include <time.h>

CPU_state cpu;

const char *regsl[] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
const char *regsw[] = {"ax", "cx", "dx", "bx", "sp", "bp", "si", "di"};
const char *regsb[] = {"al", "cl", "dl", "bl", "ah", "ch", "dh", "bh"};

void reg_test() {
	srand(time(0));
	uint32_t sample[8];
	uint32_t eip_sample = rand();
	cpu.eip = eip_sample;

	int i;
	for(i = R_EAX; i <= R_EDI; i ++) {
		sample[i] = rand();
		reg_l(i) = sample[i];
		assert(reg_w(i) == (sample[i] & 0xffff));
	}

	assert(reg_b(R_AL) == (sample[R_EAX] & 0xff));
	assert(reg_b(R_AH) == ((sample[R_EAX] >> 8) & 0xff));
	assert(reg_b(R_BL) == (sample[R_EBX] & 0xff));
	assert(reg_b(R_BH) == ((sample[R_EBX] >> 8) & 0xff));
	assert(reg_b(R_CL) == (sample[R_ECX] & 0xff));
	assert(reg_b(R_CH) == ((sample[R_ECX] >> 8) & 0xff));
	assert(reg_b(R_DL) == (sample[R_EDX] & 0xff));
	assert(reg_b(R_DH) == ((sample[R_EDX] >> 8) & 0xff));

	assert(sample[R_EAX] == cpu.eax);
	assert(sample[R_ECX] == cpu.ecx);
	assert(sample[R_EDX] == cpu.edx);
	assert(sample[R_EBX] == cpu.ebx);
	assert(sample[R_ESP] == cpu.esp);
	assert(sample[R_EBP] == cpu.ebp);
	assert(sample[R_ESI] == cpu.esi);
	assert(sample[R_EDI] == cpu.edi);

	assert(eip_sample == cpu.eip);
}

Sreg_Descriptor* sreg_desc;

void sreg_load(uint8_t sreg_num) {
	//Sreg_Descriptor* sreg_desc;
	Assert(cpu.cr0.protect_enable, "not in protect mode.");
	uint16_t index=cpu.SREG[sreg_num].selector>>3;
	Assert((index<<3)<=cpu.GDTR.size,"segement selector is out of the limit.");
	lnaddr_t chart_addr=cpu.GDTR.base+(index<<3);
	sreg_desc->first_part=lnaddr_read(chart_addr,4);
	sreg_desc->second_part=lnaddr_read(chart_addr+4,4);
	uint32_t base_addr=0;
	uint32_t seg_limit=0;
	base_addr+=((uint32_t)sreg_desc->base1);
	base_addr+=((uint32_t)sreg_desc->base2)<<16;
	base_addr+=((uint32_t)sreg_desc->base3)<<24;
	cpu.SREG[sreg_num].base=base_addr;
	seg_limit+=((uint32_t)sreg_desc->limit1);
	seg_limit+=((uint32_t)sreg_desc->limit2)<<16;
	seg_limit+=((uint32_t)0xfff)<<24;
	cpu.SREG[sreg_num].limit=seg_limit;
	if(sreg_desc->g==1) cpu.SREG[sreg_num].limit<<=12;
}

