#include "cpu/exec/template-start.h"

#define instr test

static void do_execute()
{
    DATA_TYPE result = op_dest->val & op_src->val;
    cpu.eflags.OF = 0;
    cpu.eflags.CF = 0;
    update_eflags_pf_zf_sf(result);
    cpu.eflags.SF = result >> (DATA_BYTE * 8 - 1);
    cpu.eflags.ZF = !result;
    int qwq = 0;
    int i;
    for (i = 0; i < 8; i++)
        qwq += (result >> i) & 1;
    if (qwq % 2)
        cpu.eflags.PF = 0;
    else
        cpu.eflags.PF = 1;
    // update_eflags_pf_zf_sf(result);
    print_asm_template2();
}

make_instr_helper(i2a)
    make_instr_helper(i2rm)
        make_instr_helper(r2rm)
#include "cpu/exec/template-end.h"