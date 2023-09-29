#include "cpu/exec/template-start.h"

#define instr push

static void do_execute() {
    if(DATA_BYTE==1||DATA_BYTE==2) {		
    //	DATA_TYPE_S temp=op_src->val;
    	reg_l(R_ESP)-=4;
    	swaddr_write(reg_l(R_ESP),4,op_src->val,op_src->sreg);
    }
    else {
    	REG(R_ESP)-=DATA_BYTE;
    	MEM_W(REG(R_ESP),op_src->val);
    }
    print_asm_template1();
}

make_instr_helper(i)

#if DATA_BYTE==1
make_instr_helper(si)
#endif

#if DATA_BYTE==2 || DATA_BYTE==4
make_instr_helper(r)
make_instr_helper(rm)
#endif

#include "cpu/exec/template-end.h"
