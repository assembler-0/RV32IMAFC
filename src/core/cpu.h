#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h"

#define NUM_REGISTERS 32

// RISC-V Opcodes
#define OPCODE_OP       0x33  // R-type arithmetic
#define OPCODE_OP_IMM   0x13  // I-type arithmetic
#define OPCODE_LOAD     0x03  // Load instructions
#define OPCODE_LUI      0x37  // Load Upper Immediate
#define OPCODE_AUIPC    0x17  // Add Upper Immediate to P
#define OPCODE_BRANCH   0x63  // Branch instructions
#define OPCODE_STORE    0x23  // Store instructions
#define OPCODE_JAL      0x6F
#define OPCODE_JALR     0x67
// Instruction types
typedef enum {
    //R-Type
    INST_ADD,
    INST_SUB,
    INST_MUL,
    INST_MULH,
    INST_MULHSU,
    INST_MULHU,
    INST_DIV,
    INST_DIVU,
    INST_REM,
    INST_REMU,
    INST_SLL,
    INST_SRL,
    INST_SRA,
    INST_SLT,
    INST_SLTU,
    INST_XOR,
    INST_OR,
    INST_AND,
    //I-Type
    INST_ADDI,
    INST_SLTI,
    INST_SLTIU,
    INST_XORI,
    INST_ORI,
    INST_ANDI,
    INST_SLLI,
    INST_SRLI,
    INST_SRAI,
    //Loading
    INST_LB,
    INST_LH,
    INST_LW,
    INST_LBU,
    INST_LHU,
    //Storing
    INST_SB,
    INST_SH,
    INST_SW,
    //Control Transfer
    INST_BEQ,
    INST_BNE,
    INST_BLT,
    INST_BGE,
    INST_BLTU,
    INST_BGEU,
    INST_JAL,
    INST_JALR,
    INST_LUI,
    INST_AUIPC,
    //Unknown
    INST_UNKNOWN
} inst_type_t;

// RISC-V CPU state
typedef struct {
    uint32_t regs[NUM_REGISTERS]; // General-purpose registers (x0-x31)
    uint32_t pc;                  // Program Counter
} cpu_t;

void cpu_init(cpu_t* cpu);
void cpu_execute(cpu_t* cpu, memory_t* memory, uint32_t instruction);

#endif // CPU_H