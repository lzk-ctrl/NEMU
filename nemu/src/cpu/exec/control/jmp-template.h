#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
	cpu.eip += op_src->val;
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(si)
#if DATA_BYTE == 4
make_helper(jmp_rm_l) {
	int len = decode_rm_l(eip + 1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}

extern Sreg_Descriptor *sreg_desc;
Sreg_Descriptor sreg_tmp;
make_helper(ljmp)
{
	sreg_desc = &sreg_tmp;
	uint32_t op1 = instr_fetch(cpu.eip + 1, 4);
	uint16_t op2 = instr_fetch(cpu.eip + 1 + 4, 2);
	cpu.eip = op1 - 7;
	cpu.cs.selector = op2;
	//get sreg idex
	uint16_t idx = cpu.cs.selector >> 3;
	Assert((idx << 3) <= cpu.gdtr.limit, "Segement Selector Is Out Of The Limit!");
	//chart addr
	uint32_t chart_addr = cpu.gdtr.base + (idx << 3);
	sreg_desc->first_part = lnaddr_read(chart_addr, 4);
	sreg_desc->second_part = lnaddr_read(chart_addr + 4, 4);
	uint32_t base_addr = 0;
	uint32_t seg_limit = 0;
	base_addr |= ((uint32_t)sreg_desc->base1);
	base_addr |= ((uint32_t)sreg_desc->base2) << 16;
	base_addr |= ((uint32_t)sreg_desc->base3) << 24;
	cpu.cs.base = base_addr;
	seg_limit |= ((uint32_t)sreg_desc->limit1);
	seg_limit |= ((uint32_t)sreg_desc->limit2) << 16;
	seg_limit |= ((uint32_t)0xfff) << 24;
	cpu.cs.limit = seg_limit;
	if (sreg_desc->g == 1)
	{
		cpu.cs.limit <<= 12;
	}
	print_asm("ljump");
	return 7;
}

#endif
#include "cpu/exec/template-end.h"
