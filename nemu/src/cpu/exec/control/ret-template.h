#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat3(instr,_p_,SUFFIX)){
    cpu.eip = MEM_R(REG(R_ESP));
    MEM_W(REG(R_ESP),0);
    REG(R_ESP) += DATA_BYTE;
    print_asm("ret");

    return 0;
}

make_helper(concat3(instr,_si_,SUFFIX)){
    cpu.eip = MEM_R(REG(R_ESP));
    if(DATA_BYTE == 2) cpu.eip &= 0xffff;

    MEM_W(REG(R_ESP),0);
    REG(R_ESP) += DATA_BYTE;
    REG(R_ESP) += op_src->val;
    print_asm("ret imm");

    return 0;
}

#include "cpu/exec/template-end.h"
