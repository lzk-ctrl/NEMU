#include "cpu/exec/template-start.h"

#define instr ret

make_helper(concat(ret_n_,SUFFIX)) {
    cpu.eip=MEM_R(REG(R_ESP));
    MEM_W(REG(R_ESP),0);
    REG(R_ESP)+=DATA_BYTE; 
    if(DATA_BYTE==2) cpu.eip=cpu.eip&0xffff;
	print_asm(str(instr));
	return 1;
}

make_helper(concat(ret_i_,SUFFIX)) {
    int value=instr_fetch(cpu.eip+1,2);
    int temp;
    cpu.eip=MEM_R(REG(R_ESP)); 
    if(DATA_BYTE==2) cpu.eip=cpu.eip&0xffff;
    for(temp=0;temp<=value;temp+=DATA_BYTE) {
        MEM_W(REG(R_ESP),0);
        REG(R_ESP)+=DATA_BYTE;
    }
	print_asm(str(instr) " %x",temp);
	return 1;
}

#include "cpu/exec/template-end.h"
