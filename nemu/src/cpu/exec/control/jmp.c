#include "cpu/exec/helper.h"

#define DATA_BYTE 1
#include "jmp-template.h"
#undef DATA_BYTE

#define DATA_BYTE 4
#include "jmp-template.h"
#undef DATA_BYTE


void load_sregment(uint8_t, uint16_t);

make_helper(ljmp) {
	swaddr_t ad = instr_fetch(eip + 1, 4);
	uint16_t cs = instr_fetch(eip + 5, 2);
	cpu.eip = ad - 7;//(1 + 4 + 2)
	load_sregment(R_CS, cs);

	print_asm("ljmp $0x%x,$0x%x", cs, ad);

	return 7;
}
