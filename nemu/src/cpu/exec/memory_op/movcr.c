#include "cpu/exec/helper.h"
void load_sreg(uint8_t, uint16_t);
void init_all_tlb();

make_helper(mov_r2cr) {
	int l = decode_r2rm_l(eip + 1);
	assert(op_dest->type == OP_TYPE_REG);
	
    if(op_src->reg == 0) {
		uint32_t t = cpu.cr0.val;
		cpu.cr0.val = reg_l(op_dest->reg);
		
        if(cpu.cr0.PE) {
			//Log("1 %d",cpu.cr0.val);
			if((t & 0x1) == 0) {
				//Log("2");
				cpu.cs.base = 0;
				cpu.cs.limit = 0xffffffff;
				cpu.cs.type = 0x1a;
				cpu.cs.pri_lev = 0;
				cpu.cs.soft_use = 0;
				//Log("3");
			}
		}
	}
	
	else if(op_src->reg == 3) {
		cpu.cr3.val = reg_l(op_dest->reg);
		init_all_tlb();
	}
	else { assert(0); }
	//Log("4");
	print_asm("movl %%%s, %%cr%d", regsl[op_dest->reg], op_src->reg);
	//Log("5");
	return l + 1;
}

	make_helper(mov_cr2r) {
	int l = decode_r2rm_l(eip + 1);
	assert(op_dest->type == OP_TYPE_REG);
	if(op_dest->reg == 0) { reg_l(op_src->reg) = cpu.cr0.val; }
	else if(op_dest->reg == 3) { reg_l(op_src->reg) = cpu.cr3.val; }
	else { assert(0); }

	print_asm("movl %%cr%d,%%%s", op_src->reg, regsl[op_dest->reg]);
	return l + 1;
}


