#include "cpu/exec/template-start.h"

#define instr push

static void do_execute(){

    uint32_t byte = DATA_BYTE == 2 ? 2 : 4;
    cpu.esp -= byte;
    
    swaddr_write(cpu.esp, byte, op_src->val,R_SS);
    print_asm(str(instr) str(SUFFIX) " %s", op_src->str);
}

#if DATA_BYTE != 1
make_instr_helper(r);
make_instr_helper(rm);
#endif
make_instr_helper(rel);
make_instr_helper(i);

#include "cpu/exec/template-end.h"
