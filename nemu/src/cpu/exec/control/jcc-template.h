#include "cpu/exec/template-start.h"
#define instr concat(j,CC)

static void do_execute(){
	if(CONDITION)
		cpu.eip+=op_src->val;
	if(DATA_BYTE==2)
		cpu.eip&=0xffff;

	print_asm(str(instr)" %x",cpu.eip+1+DATA_BYTE+(DATA_BYTE==2||DATA_BYTE==4));
}

make_instr_helper(rel);

#include "cpu/exec/template-end.h"
