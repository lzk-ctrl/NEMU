#include "cpu/exec/template-start.h"
#include "memory/tlb.h"

#define instr mov

static void do_execute() {
	OPERAND_W(op_dest, op_src->val);
	print_asm_template2();
}

make_instr_helper(i2r)
make_instr_helper(i2rm)
make_instr_helper(r2rm)
make_instr_helper(rm2r)

make_helper(concat(mov_a2moffs_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	op_src->sreg=R_DS;
	MEM_W(addr, REG(R_EAX));
	print_asm("mov" str(SUFFIX) " %%%s,0x%x", REG_NAME(R_EAX), addr);
	return 5;
}

make_helper(concat(mov_moffs2a_, SUFFIX)) {
	swaddr_t addr = instr_fetch(eip + 1, 4);
	op_src->sreg=R_DS;
	REG(R_EAX) = MEM_R(addr);
	print_asm("mov" str(SUFFIX) " 0x%x,%%%s", addr, REG_NAME(R_EAX));
	return 5;
}

#if DATA_BYTE==2

void sreg_load(uint8_t);

make_helper(mov_srg2rm) {
	uint8_t temp=instr_fetch(eip+1,1);
	uint8_t srg=(temp>>3)&0x7;
	uint8_t reg=temp&0x7;
	cpu.SREG[srg].selector=REG(reg);
	sreg_load(srg);
	print_asm(str(instr) " %s SREG[%u] ",REG_NAME(reg),srg);
	return 2;
}
#endif

#if DATA_BYTE==4
make_helper(mov_cr2r) {
	uint8_t temp=instr_fetch(eip+1,1);
	uint8_t cr=(temp>>3)&0x7;
	uint8_t reg=temp&0x7;
	if(cr==0) {
		REG(reg)=cpu.cr0.val;
		print_asm(str(instr) " CR0 %s",REG_NAME(reg));
	}
	if(cr==3) {
		REG(reg)=cpu.cr3.val;
		print_asm(str(instr) " CR3 %s",REG_NAME(reg));
	}
	return 2;
}

make_helper(mov_r2cr) {
	uint8_t temp=instr_fetch(eip+1,1);
	uint8_t cr=(temp>>3)&0x7;
	uint8_t reg=temp&0x7;
	if(cr==0) {
		cpu.cr0.val=REG(reg);
		print_asm(str(instr) " %s CR0 ",REG_NAME(reg));
	}
	if(cr==3) {
		init_tlb();
		cpu.cr3.val=REG(reg);
		print_asm(str(instr) " %s CR3 ",REG_NAME(reg));
	}
	return 2;
}
#endif

#include "cpu/exec/template-end.h"
