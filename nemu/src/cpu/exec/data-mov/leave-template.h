#include "cpu/exec/template-start.h"
#define instr leave

static void do_execute () {
	
    cpu.esp=cpu.ebp;
    concat(reg_,SUFFIX)(5)=MEM_R(R_SS,cpu.esp);
    cpu.esp+=DATA_BYTE;
	print_asm(str(instr));
}

make_helper(concat3(instr,_,SUFFIX))
{
    do_execute();
    return 1;
}

#include "cpu/exec/template-end.h"
