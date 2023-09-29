#include "cpu/exec/template-start.h"

#define instr sub

static void do_execute() {
	DATA_TYPE result = op_dest->val - op_src->val;
	OPERAND_W(op_dest, result);
	if(op_dest->val < op_src->val) cpu.eflags.CF = 1;
    else cpu.eflags.CF = 0;
    cpu.eflags.SF = result >> (DATA_BYTE*8 -1);
    int a = op_dest->val >> (DATA_BYTE*8 - 1);
    int b = op_src->val >> (DATA_BYTE*8 - 1);
	
    cpu.eflags.OF = (a != b) && (b == cpu.eflags.SF);
    cpu.eflags.ZF = !result;
    int qwq = 0;
    int i;
    for(i = 0; i < 8; i++) qwq += (result >> i) & 1;
    if(qwq % 2) cpu.eflags.PF = 0;
    else cpu.eflags.PF = 1;
    if(( (op_dest->val & 0xf) - (op_src->val & 0xf) ) >> 4) cpu.eflags.AF = 1;
    else cpu.eflags.AF = 0;
	print_asm_template2();
}

make_instr_helper(i2rm)
#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif
make_instr_helper(i2a)
make_instr_helper(r2rm)
make_instr_helper(rm2r)


#include "cpu/exec/template-end.h"