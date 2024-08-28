#include "cpu/exec/template-start.h"

#define instr call

make_helper(concat(call_i_, SUFFIX)) {
    int len = concat(decode_i_, SUFFIX)(eip + 1);
    printf("CALL I: len = %d, eip = 0x%x, op_src->val = 0x%x\n", len, cpu.eip, op_src->val);
    reg_l(R_ESP) -= DATA_BYTE; 
    MEM_W(reg_l(R_ESP), cpu.eip + len + 1); 
    printf("Pushed return address 0x%x to stack\n", cpu.eip + len + 1);
    cpu.eip += (DATA_TYPE_S)op_src->val; 
    printf("New eip = 0x%x\n", cpu.eip);
    return len + 1; 
}

make_helper(concat(call_rm_, SUFFIX)) {
    int len = concat(decode_rm_, SUFFIX)(eip + 1);
    printf("CALL RM: len = %d, eip = 0x%x, op_src->val = 0x%x\n", len, cpu.eip, op_src->val);
    reg_l(R_ESP) -= DATA_BYTE;
    MEM_W(reg_l(R_ESP), cpu.eip + len + 1);
    printf("Pushed return address 0x%x to stack\n", cpu.eip + len + 1);
    cpu.eip = (DATA_TYPE_S)op_src->val; 
    printf("New eip = 0x%x\n", cpu.eip);
    return 0; 
}

#include "cpu/exec/template-end.h"