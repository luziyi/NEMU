#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
	int len = concat(decode_i_, SUFFIX)(eip + 1);
	reg_l(R_ESP) -= DATA_BYTE; // Decrease ESP by DATA_BYTE (typically 4 bytes for 32-bit)
	MEM_W(reg_l(R_ESP), cpu.eip + len + 1); // Store the return address on the stack
	cpu.eip += (DATA_TYPE_S)op_src->val; // Jump to the target address
	print_asm("call: 0x%x", cpu.eip); // Print the new eip value after the jump
	return len + 1; // Return the total length of the instruction
}

make_helper(concat(call_rm_, SUFFIX)) {
	int len = concat(decode_rm_, SUFFIX)(eip + 1);
	reg_l(R_ESP) -= DATA_BYTE;
	MEM_W(reg_l(R_ESP), cpu.eip + len + 1);
	cpu.eip = (DATA_TYPE_S)op_src->val; // Direct jump to the address in the register/memory
	print_asm("call: %s", op_src->str); // Print the jump target (register or memory address)
	return 0; // No need to add len+1, since eip is directly set
}

#include "cpu/exec/template-end.h"