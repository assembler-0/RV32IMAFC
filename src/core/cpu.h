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
#define OPCODE_SYSTEM   0x73  // System instructions
#define OPCODE_AMO      0x2F
// Privilege Levels
typedef enum {
    USER_MODE = 0,
    SUPERVISOR_MODE = 1,
    MACHINE_MODE = 3
} privilege_level_t;

typedef struct {
    uint32_t regs[NUM_REGISTERS]; // General-purpose registers (x0-x31)
    uint32_t pc;                  // Program Counter
    uint32_t csrs[4096];          // CSRs
    privilege_level_t privilege;  // Current privilege level
    uint32_t reserved_address;    // For LR/SC
    int reservation_set;          // For LR/SC
} cpu_t;
// CSR Addresses
#define CSR_MSTATUS     0x300
#define CSR_MISA        0x301
#define CSR_MIE         0x304
#define CSR_MTVEC       0x305
#define CSR_MEPC        0x341
#define CSR_MCAUSE      0x342
#define CSR_MTVAL       0x343
#define CSR_MIP         0x344

#define CSR_SSTATUS     0x100
#define CSR_SIE         0x104
#define CSR_STVEC       0x105
#define CSR_SEPC        0x141
#define CSR_SCAUSE      0x142
#define CSR_STVAL       0x143
#define CSR_SIP         0x144

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
    // System
    INST_FENCE,
    INST_FENCE_I,
    INST_SFENCE_VMA,
    INST_WFI,
    INST_ECALL,
    INST_EBREAK,
    INST_MRET,
    INST_SRET,
    INST_URET,
    INST_CSRRW,
    INST_CSRRS,
    INST_CSRRC,
    INST_CSRRWI,
    INST_CSRRSI,
    INST_CSRRCI,
    // Atomic Instructions
    INST_LR_W,
    INST_SC_W,
    INST_AMOSWAP_W,
    INST_AMOADD_W,
    INST_AMOXOR_W,
    INST_AMOAND_W,
    INST_AMOOR_W,
    INST_AMOMIN_W,
    INST_AMOMAX_W,
    INST_AMOMINU_W,
    INST_AMOMAXU_W,
    //Floating point
    INST_FMADD_S,
    INST_FMSUB_S,
    INST_FNMSUB_S,
    INST_FNMADD_S,
    INST_FADD_S,
    INST_FSUB_S,
    INST_FMUL_S,
    INST_FDIV_S,
    INST_FSQRT_S,
    INST_FSGNJ_S,
    INST_FSGNJN_S,
    INST_FSGNJX_S,
    INST_FMIN_S,
    INST_FMAX_S,
    INST_FCVT_W_S,
    INST_FCVT_WU_S,
    INST_FMV_X_W,
    INST_FEQ_S,
    INST_FLT_S,
    INST_FLE_S,
    INST_FCLASS_S,
    INST_FCVT_S_W,
    INST_FCVT_S_WU,
    INST_FMV_W_X,
    INST_FMADD_D,
    INST_FMSUB_D,
    INST_FNMSUB_D,
    INST_FNMADD_D,
    INST_FADD_D,
    INST_FSUB_D,
    INST_FMUL_D,
    INST_FDIV_D,
    INST_FSQRT_D,
    INST_FSGNJ_D,
    INST_FSGNJN_D,
    INST_FSGNJX_D,
    INST_FMIN_D,
    INST_FMAX_D,
    INST_FCVT_S_D,
    INST_FCVT_D_S,
    INST_FEQ_D,
    INST_FLT_D,
    INST_FLE_D,
    INST_FCLASS_D,
    INST_FCVT_W_D,
    INST_FCVT_WU_D,
    INST_FCVT_D_W,
    INST_FCVT_D_WU,
    INST_FLW,
    INST_FSW,
    INST_FLD,
    //Unknown
    INST_UNKNOWN
} inst_type_t;


void cpu_init(cpu_t* cpu);
void cpu_execute(cpu_t* cpu, memory_t* memory, uint32_t instruction);

#endif // CPU_H
