#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute(){

	uint32_t byte = DATA_BYTE==2?2:4;
	uint32_t val=MEM_R(R_SS,cpu.esp);
	OPERAND_W(op_src,val);
	cpu.esp+=byte;
	print_asm(str(instr) " %s",op_src->str);
}


#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(r);
make_instr_helper(rm);

#endif

#include "cpu/exec/template-end.h"
