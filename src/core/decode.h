#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>
#include "cpu.h"
// Decoded instruction format

void decode_instruction(uint32_t instruction, instruction_t* decoded_inst);
uint32_t expand_compressed(uint16_t c_inst);

#endif // DECODE_H
