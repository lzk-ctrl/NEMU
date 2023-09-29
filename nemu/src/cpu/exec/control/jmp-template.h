#include "cpu/exec/template-start.h"

#define instr jmp

static void do_execute() {
        DATA_TYPE_S bias=op_src->val;
	cpu.eip += bias;
	print_asm(str(instr) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(i)
#if DATA_BYTE == 4 ||  DATA_BYTE == 2
make_helper(concat(jmp_rm_,SUFFIX)) {
	int len = concat(decode_rm_,SUFFIX) (eip+1);
	cpu.eip = op_src->val - (len + 1);
	print_asm(str(instr) " *%s", op_src->str);
	return len + 1;
}
#endif
#include "cpu/exec/template-end.h"
