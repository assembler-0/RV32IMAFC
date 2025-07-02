#ifndef DECODE_TABLE_H
#define DECODE_TABLE_H

#include "cpu.h"

// Decode table entry
typedef struct {
    inst_type_t inst_type;
    uint8_t has_imm_override;  // Special immediate handling needed
} decode_entry_t;

// Decode tables for different opcodes
extern decode_entry_t op_table[128][8];        // OP instructions (funct7, funct3)
extern decode_entry_t op_imm_table[128][8];    // OP-IMM instructions (funct7, funct3)
extern decode_entry_t load_table[8];           // Load instructions (funct3)
extern decode_entry_t store_table[8];          // Store instructions (funct3)
extern decode_entry_t branch_table[8];         // Branch instructions (funct3)
extern decode_entry_t system_table[4096];      // System instructions (funct12)
extern decode_entry_t amo_table[32][8];        // Atomic instructions (funct5, funct3)
extern decode_entry_t op_fp_table[128][8];     // FP instructions (funct7, funct3)

// Initialize decode tables
void init_decode_tables(void);

#endif // DECODE_TABLE_H