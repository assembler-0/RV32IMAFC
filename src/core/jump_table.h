#ifndef JUMP_TABLE_H
#define JUMP_TABLE_H

#include "cpu.h"

// Function pointer type for instruction execution
typedef void (*inst_func_t)(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);

// Jump table for fast instruction dispatch
extern inst_func_t instruction_table[INST_UNKNOWN + 1];

// Initialize the jump table
void init_jump_table(void);

#endif // JUMP_TABLE_H