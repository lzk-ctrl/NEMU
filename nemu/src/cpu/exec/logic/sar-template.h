#include "cpu/exec/template-start.h"

#define instr sar

static void do_execute () {
	DATA_TYPE src = op_src->val;
	DATA_TYPE_S dest = op_dest->val;
	DATA_TYPE_S result = op_dest->val;
	uint8_t count = src & 0x1f;
	dest >>= count;
	result = dest;
	int len = (DATA_BYTE << 3) - 1;
	cpu.eflags.CF=0;
	cpu.eflags.OF=0;
	cpu.eflags.SF=result >> len;
    	cpu.eflags.ZF=!result;
    	OPERAND_W(op_dest, dest);
	result ^= result >>4;
	result ^= result >>2;
	result ^= result >>1;
	cpu.eflags.PF=!(result & 1);
	/* There is no need to update EFLAGS, since no other instructions 
	 * in PA will test the flags updated by this instruction.
	 */

	print_asm_template2();
}

make_instr_helper(rm_1)
make_instr_helper(rm_cl)
make_instr_helper(rm_imm)

#include "cpu/exec/template-end.h"