#include "cpu/exec/template-start.h"

#define instr call
static void do_execute(){

    cpu.esp-=4;
    swaddr_write(cpu.esp, 4,cpu.eip+1+DATA_BYTE,R_SS );
    //Log("%x",op_src->val);
    cpu.eip+=op_src->val;
    print_asm(str(instr)str(SUFFIX) " %x", cpu.eip + 1 + DATA_BYTE);
}

make_instr_helper(rel);

make_helper(concat3(instr,_rm_,SUFFIX)){

    int len=1;
    len+=concat(decode_rm_,SUFFIX)(eip+1);
    cpu.esp-=4;
    swaddr_write(cpu.esp, 4,cpu.eip+len,R_SS);
    cpu.eip=op_src->val-len;
    print_asm(str(instr)str(SUFFIX) " %s",op_src->str);
    return len;
}

#include "cpu/exec/template-end.h"

