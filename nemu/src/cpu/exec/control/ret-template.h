#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_, SUFFIX))
{
    cpu.eip = MEM_R(reg_l(R_SP), R_SS) - 1;
    if (DATA_BYTE == 2)
        cpu.eip &= 0xffff;
    reg_l(R_SP) += DATA_BYTE;
    print_asm("ret");

    return 1;
}
#include "cpu/exec/template-end.h"