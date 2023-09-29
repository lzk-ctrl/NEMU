#include "cpu/exec/template-start.h"

#define instr test

static void do_execute() {
    DATA_TYPE temp=op_dest->val&op_src->val;
    if(DATA_BYTE==1 || DATA_BYTE==2) {
        cpu.eflags.SF=MSB(temp);
        cpu.eflags.ZF=(temp==0);
        int t=temp&0xff;
        t^=t>>4;
        t^=t>>2;
        t^=t>>1;
        cpu.eflags.PF=!(t&1);
    }
    else{
        update_eflags_pf_zf_sf(temp);
    }
    cpu.eflags.OF=0;
    cpu.eflags.CF=0;
    print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(r2rm)

#include "cpu/exec/template-end.h"
