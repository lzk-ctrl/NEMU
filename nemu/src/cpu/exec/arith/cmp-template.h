#include "cpu/exec/template-start.h"

#define instr cmp

static void do_execute() {
    DATA_TYPE temp=op_dest->val-op_src->val;
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
    int temp1=MSB(op_src->val);
    int temp2=MSB(op_dest->val);
    cpu.eflags.OF=(temp2!=temp1 && cpu.eflags.SF!=temp2);
    cpu.eflags.CF=(op_dest->val<op_src->val);
    print_asm_template2();
}

make_instr_helper(i2a)
make_instr_helper(i2rm)
make_instr_helper(rm2r)
make_instr_helper(r2rm)

#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(si2rm)
#endif

#include "cpu/exec/template-end.h"
