#include "cpu/exec/template-start.h"
#define instr pushss
static void do_execute()
{
    if(DATA_BYTE==1)op_src->val&=0xff;
    cpu.esp -= 4;
    swaddr_write(cpu.esp,4,op_src->val,R_SS);
    print_asm_template1();
}
make_instr_helper(si);
#include "cpu/exec/template-end.h"