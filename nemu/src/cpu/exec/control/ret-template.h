#include "cpu/exec/template-start.h"

#define instr ret

void static do_execute(){

    cpu.eip=swaddr_read(cpu.esp, 4,R_SS)-1-op_src->size;
    cpu.esp+=4;
    cpu.esp+=op_src->val;
    if(!op_src->size)
        print_asm(str(instr));
    else
        print_asm(str(instr)" $0x%x",op_src->val);
}

make_helper(instr){

    op_src->val=0;
    op_src->size=0;
    do_execute();
    return 1;
}

make_helper(concat(instr,_i_w)){

    int len = 1;
    len+= decode_i_w(eip+1);
    op_src->size=2;
    do_execute();
    return len;
}





#include "cpu/exec/template-end.h"
