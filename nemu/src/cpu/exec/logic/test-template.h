#include "cpu/exec/template-start.h"

#define instr test

static void do_execute()
{
    DATA_TYPE r=op_dest->val&op_src->val;
	int len=(DATA_BYTE<<3)-1;
 	cpu.eflags.OF=0;
	cpu.eflags.CF=0;
	cpu.eflags.SF=r>>len;
	cpu.eflags.ZF=!r;
	r^=r>>4;
	r^=r>>2;
	r^=r>>1;
	cpu.eflags.PF=!(r&1);
	print_asm_template2();
    /**/
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(si2rm)
#endif
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"