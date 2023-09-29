#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_,SUFFIX)) {
	int len = concat(decode_i_,SUFFIX) (eip+1);
    REG(R_ESP)-=DATA_BYTE;
    MEM_W(REG(R_ESP),cpu.eip+len);
    DATA_TYPE_S bias=op_src->val;
    cpu.eip += bias;
    if(DATA_BYTE==2) cpu.eip=cpu.eip&0xffff;
	print_asm(str(instr) " %x", cpu.eip+len+1);
	return len+1;
}
make_helper(concat(call_rm_,SUFFIX)) {
	int len = concat(decode_rm_,SUFFIX) (eip+1);
    REG(R_ESP)-=DATA_BYTE;
    MEM_W(REG(R_ESP), cpu.eip+len);
    DATA_TYPE_S bias=op_src->val;
    cpu.eip = bias-len-1;
    if(DATA_BYTE==2) cpu.eip=cpu.eip&0xffff;
	print_asm(str(instr) " %x", bias);
	return len+1;
}

#include "cpu/exec/template-end.h"
