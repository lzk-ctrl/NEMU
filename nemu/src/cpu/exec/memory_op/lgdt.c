#include "cpu/exec/helper.h"
void load_sreg(uint8_t, uint16_t);
void init_all_tlb();

make_helper(lgdt)
{
    int l=decode_rm_l(eip+1);
    cpu.gdtr.limit=lnaddr_read(op_src->addr,2);
    cpu.gdtr.base=lnaddr_read(op_src->addr+2,4);

    print_asm("lgdt %s",op_src->str);
    
    return l+1;
}
