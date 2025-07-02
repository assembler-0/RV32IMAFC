#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include "memory.h"

#define NUM_REGISTERS 32

// RISC-V Architecture Configuration
#ifndef XLEN
#define XLEN 32  // Default to RV32, can be overridden with -DXLEN=64
#endif

#if XLEN == 64
typedef uint64_t reg_t;
typedef int64_t sreg_t;
#else
typedef uint32_t reg_t;
typedef int32_t sreg_t;
#endif

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
#define OPCODE_AMO      0x2F  // Atomic operations
#define OPCODE_OP_IMM_32 0x1B // RV64 32-bit immediate operations
#define OPCODE_OP_32    0x3B  // RV64 32-bit operations
#define OPCODE_MADD     0x43
#define OPCODE_MSUB     0x47
#define OPCODE_NMSUB    0x4B
#define OPCODE_NMADD    0x4F
#define OPCODE_OP_FP    0x53
#define OPCODE_LOAD_FP  0x07
#define OPCODE_STORE_FP 0x27

// Privilege Levels
typedef enum {
    USER_MODE = 0,
    SUPERVISOR_MODE = 1,
    MACHINE_MODE = 3
} privilege_level_t;

typedef struct {
    reg_t regs[NUM_REGISTERS];    // General-purpose registers (x0-x31)
    float fregs[NUM_REGISTERS];   // Single precision FP registers
    double dfregs[NUM_REGISTERS]; // Double precision FP registers
    reg_t pc;                     // Program Counter
    uint32_t csrs[4096];          // CSRs (always 32-bit)
    privilege_level_t privilege;  // Current privilege level
    reg_t reserved_address;       // For LR/SC
    int reservation_set;          // For LR/SC
} cpu_t;

typedef struct {
    uint32_t opcode;
    uint32_t rd;
    uint32_t rs1;
    uint32_t rs2;
    uint32_t imm;
    uint32_t inst_type;
} instruction_t;

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
    INST_MULW,
    INST_DIVW,
    INST_DIVUW,
    INST_REMW,
    INST_REMUW,
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
    INST_LR_D,
    INST_SC_D,
    INST_AMOSWAP_D,
    INST_AMOADD_D,
    INST_AMOXOR_D,
    INST_AMOAND_D,
    INST_AMOOR_D,
    INST_AMOMIN_D,
    INST_AMOMAX_D,
    INST_AMOMINU_D,
    INST_AMOMAXU_D,
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
    INST_FCVT_L_D,
    INST_FCVT_LU_D,
    INST_FMV_X_D,
    INST_FCVT_D_L,
    INST_FCVT_D_LU,
    INST_FMV_D_X,
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
    // RV32C Compressed Instructions
    INST_C_ADDI4SPN,
    INST_C_FLD,
    INST_C_LW,
    INST_C_FLW,
    INST_C_FSD,
    INST_C_SW,
    INST_C_FSW,
    INST_C_NOP,
    INST_C_ADDI,
    INST_C_JAL,
    INST_C_LI,
    INST_C_ADDI16SP,
    INST_C_LUI,
    INST_C_SRLI,
    INST_C_SRAI,
    INST_C_ANDI,
    INST_C_SUB,
    INST_C_XOR,
    INST_C_OR,
    INST_C_AND,
    INST_C_J,
    INST_C_BEQZ,
    INST_C_BNEZ,
    INST_C_SLLI,
    INST_C_FLDSP,
    INST_C_LWSP,
    INST_C_FLWSP,
    INST_C_JR,
    INST_C_MV,
    INST_C_EBREAK,
    INST_C_JALR,
    INST_C_ADD,
    INST_C_FSDSP,
    INST_C_SWSP,
    INST_C_FSWSP,
    INST_C_LD,
    INST_C_SD,
    INST_C_ADDIW,
    INST_C_ADDW,
    INST_C_SUBW,
    INST_C_SLLI64,
    INST_C_SRLI64,
    INST_C_SRAI64,
    // RV64I Instructions
    INST_LWU,
    INST_LD,
    INST_SD,
    INST_ADDIW,
    INST_SLLIW,
    INST_SRLIW,
    INST_SRAIW,
    INST_ADDW,
    INST_SUBW,
    INST_SLLW,
    INST_SRLW,
    INST_SRAW,
    //Unknown
    INST_UNKNOWN
} inst_type_t;


void cpu_init(cpu_t* cpu);
void cpu_execute(cpu_t* cpu, memory_t* memory, uint32_t instruction);
void cpu_execute_decoded(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);

#endif // CPU_H
