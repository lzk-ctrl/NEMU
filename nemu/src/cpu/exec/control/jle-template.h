#include "cpu/exec/template-start.h"

#define instr jle

static void do_execute() {
    DATA_TYPE_S bias=op_src->val;
    print_asm(str(instr) " %x",cpu.eip+1+DATA_BYTE+bias);
    if(cpu.eflags.ZF==1 || cpu.eflags.SF!=cpu.eflags.OF) {
        cpu.eip+=bias;
        if(DATA_BYTE==2) cpu.eip=cpu.eip&0xffff;
    }
}

make_instr_helper(i)

#include "cpu/exec/template-end.h"
