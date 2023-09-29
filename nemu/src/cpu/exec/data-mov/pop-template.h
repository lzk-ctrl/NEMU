#include "cpu/exec/template-start.h"

#define instr pop

static void do_execute() {
	if(DATA_BYTE==1||DATA_BYTE==2) {
		reg_l(op_src->reg)=swaddr_read(reg_l(R_ESP),4,op_src->sreg);
		swaddr_write(reg_l(R_ESP),4,0,op_src->sreg);
		reg_l(R_ESP)+=4; 
	}
	else {
    	OPERAND_W(op_src,MEM_R(REG(R_ESP)));
    	MEM_W(REG(R_ESP),0);
  	  	REG(R_ESP)+=DATA_BYTE; 
  	}
    print_asm_template1();
}        

make_instr_helper(r)
make_instr_helper(rm)

#include "cpu/exec/template-end.h"
