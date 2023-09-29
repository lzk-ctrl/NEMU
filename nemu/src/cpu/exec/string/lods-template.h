#include "cpu/exec/template-start.h"
#define instr lods

static void do_execute () {
    uint32_t val=MEM_R(R_DS,cpu.esi);
    uint32_t IncDec=0;
	OPERAND_W(op_dest,val);
    IncDec = DATA_BYTE;
    if(cpu.eflags.DF)
        IncDec=-IncDec;
    cpu.esi+=IncDec;
    print_asm(str(instr) str(SUFFIX) "%%ds:(%%esi)," str(REG_NAME(0)) );
}

make_helper(concat3(instr,_,SUFFIX))
{
    op_dest->type=OP_TYPE_REG;
    op_dest->size=DATA_BYTE;
    op_dest->reg=0;
    do_execute();
    return 1;
}


#include "cpu/exec/template-end.h"
