#include "cpu/exec/helper.h"

make_helper(pop)
{
    reg_l(instr_fetch(cpu.eip, 1) & 7) = swaddr_read(cpu.esp, 4, R_SS);
    cpu.esp += 4;
    print_asm("pop\t%%%s", regsl[instr_fetch(cpu.eip, 1) & 7]);
    return 1;
}