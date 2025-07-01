#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

// Decoded instruction format
typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm;
    uint32_t inst_type;
} instruction_t;

void decode_instruction(uint32_t instruction, instruction_t* decoded_inst);

#endif // DECODE_H
