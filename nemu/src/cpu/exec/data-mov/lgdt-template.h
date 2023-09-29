#include "cpu/exec/template-start.h"

#define instr lgdt

static void do_execute() {
	cpu.GDTR.size=swaddr_read(op_src->addr,2,op_src->sreg);
	if(DATA_BYTE==2) {
		cpu.GDTR.base=swaddr_read(op_src->addr+2,3,op_src->sreg);
	}
	else {
		cpu.GDTR.base=swaddr_read(op_src->addr+2,4,op_src->sreg);
	}
	print_asm_template1();
}

make_instr_helper(rm);

#include "cpu/exec/template-end.h"
