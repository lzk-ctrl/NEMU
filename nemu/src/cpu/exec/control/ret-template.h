#include "cpu/exec/template-start.h"

#define instr ret

//SUFFIX在template-start.h文件中
//根据DATA_BYTE的值来确定SUFFIX的值（哪个字母）

make_helper(concat(ret_n_, SUFFIX)) {//后缀SUFFIX不一样，则函数不一样，函数不一样->功能不一样（操作的位数）
//make_helper是命名宏定义，个人觉得很没有必要，但是看习惯了就好看了吧
    cpu.eip = MEM_R(reg_l(R_ESP)); //MEM_R函数在template-helper.h中
    reg_l(R_ESP) += DATA_BYTE;
    print_asm("ret");
    return 0;
} 

make_helper(concat(ret_i_,SUFFIX)) {

    int val = instr_fetch(eip + 1, DATA_BYTE);
	cpu.eip = MEM_R(REG(R_ESP));
	if (DATA_BYTE == 2) cpu.eip &= 0xffff;
    REG(R_ESP) += DATA_BYTE;

    int i;
	for (i = 0;i < val; i += DATA_BYTE)
	    MEM_W(REG(R_ESP) + i,0);
	
    REG(R_ESP) += val;
	print_asm("ret\t$0x%x",val);
	return 0;
}
#include "cpu/exec/template-end.h"