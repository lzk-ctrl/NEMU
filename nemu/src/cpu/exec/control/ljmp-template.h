#include "cpu/exec/template-start.h"

#define instr ljmp

#if DATA_BYTE == 4
extern Sreg_Descriptor *sreg_desc;
Sreg_Descriptor sreg_temp;
make_helper(ljmp) {
	sreg_desc=&sreg_temp;
	uint32_t temp1=instr_fetch(cpu.eip+1,4);
	uint16_t temp2=instr_fetch(cpu.eip+5,2);
	cpu.eip=temp1-7;
	cpu.cs.selector=temp2;
	uint16_t index=temp2>>3;
	Assert((index<<3)<=cpu.GDTR.size,"segement selector is out of the limit.");

	uint32_t chart_addr=cpu.GDTR.base+(index<<3);
	sreg_desc->first_part=lnaddr_read(chart_addr,4);
	sreg_desc->second_part=lnaddr_read(chart_addr+4,4);
	uint32_t base_addr=0;
	uint32_t seg_limit=0;
	base_addr |=((uint32_t)sreg_desc->base1);
	base_addr |=((uint32_t)sreg_desc->base2)<<16;
	base_addr |=((uint32_t)sreg_desc->base3)<<24;
	cpu.cs.base=base_addr;
	seg_limit |=((uint32_t)sreg_desc->limit1);
	seg_limit |=((uint32_t)sreg_desc->limit2)<<16;
	seg_limit |=((uint32_t)0xfff)<<24;
	cpu.cs.limit=seg_limit;
	if(sreg_desc->g==1) cpu.cs.limit<<=12;
	print_asm(str(instr) " %x %x",temp2,temp1);
	return 7;
}
#endif

#include "cpu/exec/template-end.h"
