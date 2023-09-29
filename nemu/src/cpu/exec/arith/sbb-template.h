#include "cpu/exec/template-start.h"

#define instr sbb

static void do_execute () {
	DATA_TYPE result = op_dest->val - (op_src->val + cpu.eflags.CF);
	OPERAND_W(op_dest, result);

	/* TODO: Update EFLAGS. */
  update_eflags_pf_zf_sf(result);
	int len = (DATA_BYTE << 3) - 1;
	cpu.eflags.OF = ((op_dest->val >> len) != (result >> len) && (result >> len) == cpu.eflags.SF);

	cpu.eflags.CF = result > op_dest->val;

	print_asm_template2();
}

make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)
make_instr_helper(i2a)

#if DATA_BYTE == 2 || DATA_BYTE == 4
make_instr_helper(si2rm)
#endif


#include "cpu/exec/template-end.h"
