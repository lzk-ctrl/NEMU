#include "cpu/exec/template-start.h"
#define instr pushl
static void do_execute()
{
    cpu.esp -= 4;
    swaddr_write(cpu.esp, 4, op_src->val, R_SS);

    print_asm_template1();
}
make_instr_helper(rm)
#include "cpu/exec/template-end.h"