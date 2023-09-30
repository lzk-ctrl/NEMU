#include "cpu/exec/template-start.h"

#define instr leave

make_helper (concat(leave_n_,SUFFIX)) 
{
	swaddr_t i;
	for (i = cpu.esp;i < cpu.ebp; i+=DATA_BYTE)
		MEM_W (i,0);
	cpu.esp = cpu.ebp;
	cpu.ebp = MEM_R (cpu.esp);
	cpu.esp+=4;
	print_asm("leave");
	return 1;
}

#include "cpu/exec/template-end.h"