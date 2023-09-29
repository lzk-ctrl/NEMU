#include "cpu/exec/template-start.h"

#define instr lods

make_helper(concat(lods_n_, SUFFIX))
{
    cpu.eax = swaddr_read(cpu.esi, DATA_BYTE, 2);
    if (cpu.eflags.DF == 0)
    {
        cpu.esi += DATA_BYTE;
    }
    else
        cpu.esi -= DATA_BYTE;
    return 1;
}
#include "cpu/exec/template-end.h"