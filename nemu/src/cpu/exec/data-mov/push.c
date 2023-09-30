#include "cpu/exec/helper.h"

make_helper(push)
{
    uint32_t a = reg_l(instr_fetch(eip, 1) & 7);
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, a, R_SS);
    print_asm("push\t%%%s", regsl[instr_fetch(cpu.eip, 1) & 7]);
    return 1;
}