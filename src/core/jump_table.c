#include "jump_table.h"
#include "memory.h"
#include <stdio.h>
#include <math.h>

// Forward declarations - grouped by type
static void exec_alu_reg(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_alu_imm(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_load(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_store(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_branch(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_jump(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_upper(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_system(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_atomic(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_float_single(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_float_double(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
static void exec_float_mem(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
#if XLEN == 64
static void exec_rv64(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);
#endif
static void exec_unknown(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction);

// Jump table - initialized at runtime
inst_func_t instruction_table[INST_UNKNOWN + 1];

void init_jump_table(void) {
    // Initialize all to unknown first
    for (int i = 0; i <= INST_UNKNOWN; i++) {
        instruction_table[i] = exec_unknown;
    }
    
    // ALU Register operations
    instruction_table[INST_ADD] = exec_alu_reg;
    instruction_table[INST_SUB] = exec_alu_reg;
    instruction_table[INST_SLL] = exec_alu_reg;
    instruction_table[INST_SLT] = exec_alu_reg;
    instruction_table[INST_SLTU] = exec_alu_reg;
    instruction_table[INST_XOR] = exec_alu_reg;
    instruction_table[INST_SRL] = exec_alu_reg;
    instruction_table[INST_SRA] = exec_alu_reg;
    instruction_table[INST_OR] = exec_alu_reg;
    instruction_table[INST_AND] = exec_alu_reg;
    instruction_table[INST_MUL] = exec_alu_reg;
    instruction_table[INST_MULH] = exec_alu_reg;
    instruction_table[INST_MULHSU] = exec_alu_reg;
    instruction_table[INST_MULHU] = exec_alu_reg;
    instruction_table[INST_DIV] = exec_alu_reg;
    instruction_table[INST_DIVU] = exec_alu_reg;
    instruction_table[INST_REM] = exec_alu_reg;
    instruction_table[INST_REMU] = exec_alu_reg;
#if XLEN == 64
    instruction_table[INST_MULW] = exec_alu_reg;
    instruction_table[INST_DIVW] = exec_alu_reg;
    instruction_table[INST_DIVUW] = exec_alu_reg;
    instruction_table[INST_REMW] = exec_alu_reg;
    instruction_table[INST_REMUW] = exec_alu_reg;
#endif
    
    // ALU Immediate operations
    instruction_table[INST_ADDI] = exec_alu_imm;
    instruction_table[INST_SLTI] = exec_alu_imm;
    instruction_table[INST_SLTIU] = exec_alu_imm;
    instruction_table[INST_XORI] = exec_alu_imm;
    instruction_table[INST_ORI] = exec_alu_imm;
    instruction_table[INST_ANDI] = exec_alu_imm;
    instruction_table[INST_SLLI] = exec_alu_imm;
    instruction_table[INST_SRLI] = exec_alu_imm;
    instruction_table[INST_SRAI] = exec_alu_imm;
    
    // Load operations
    instruction_table[INST_LB] = exec_load;
    instruction_table[INST_LH] = exec_load;
    instruction_table[INST_LW] = exec_load;
    instruction_table[INST_LBU] = exec_load;
    instruction_table[INST_LHU] = exec_load;
#if XLEN == 64
    instruction_table[INST_LWU] = exec_load;
    instruction_table[INST_LD] = exec_load;
#endif
    
    // Store operations
    instruction_table[INST_SB] = exec_store;
    instruction_table[INST_SH] = exec_store;
    instruction_table[INST_SW] = exec_store;
#if XLEN == 64
    instruction_table[INST_SD] = exec_store;
#endif
    
    // Branch operations
    instruction_table[INST_BEQ] = exec_branch;
    instruction_table[INST_BNE] = exec_branch;
    instruction_table[INST_BLT] = exec_branch;
    instruction_table[INST_BGE] = exec_branch;
    instruction_table[INST_BLTU] = exec_branch;
    instruction_table[INST_BGEU] = exec_branch;
    
    // Jump operations
    instruction_table[INST_JAL] = exec_jump;
    instruction_table[INST_JALR] = exec_jump;
    
    // Upper immediate
    instruction_table[INST_LUI] = exec_upper;
    instruction_table[INST_AUIPC] = exec_upper;
    
    // System operations
    instruction_table[INST_ECALL] = exec_system;
    instruction_table[INST_EBREAK] = exec_system;
    instruction_table[INST_MRET] = exec_system;
    instruction_table[INST_SRET] = exec_system;
    instruction_table[INST_URET] = exec_system;
    instruction_table[INST_FENCE] = exec_system;
    instruction_table[INST_FENCE_I] = exec_system;
    instruction_table[INST_SFENCE_VMA] = exec_system;
    instruction_table[INST_WFI] = exec_system;
    instruction_table[INST_CSRRW] = exec_system;
    instruction_table[INST_CSRRS] = exec_system;
    instruction_table[INST_CSRRC] = exec_system;
    instruction_table[INST_CSRRWI] = exec_system;
    instruction_table[INST_CSRRSI] = exec_system;
    instruction_table[INST_CSRRCI] = exec_system;
    
    // Atomic operations
    instruction_table[INST_LR_W] = exec_atomic;
    instruction_table[INST_SC_W] = exec_atomic;
    instruction_table[INST_AMOSWAP_W] = exec_atomic;
    instruction_table[INST_AMOADD_W] = exec_atomic;
    instruction_table[INST_AMOXOR_W] = exec_atomic;
    instruction_table[INST_AMOAND_W] = exec_atomic;
    instruction_table[INST_AMOOR_W] = exec_atomic;
    instruction_table[INST_AMOMIN_W] = exec_atomic;
    instruction_table[INST_AMOMAX_W] = exec_atomic;
    instruction_table[INST_AMOMINU_W] = exec_atomic;
    instruction_table[INST_AMOMAXU_W] = exec_atomic;
#if XLEN == 64
    instruction_table[INST_LR_D] = exec_atomic;
    instruction_table[INST_SC_D] = exec_atomic;
    instruction_table[INST_AMOSWAP_D] = exec_atomic;
    instruction_table[INST_AMOADD_D] = exec_atomic;
    instruction_table[INST_AMOXOR_D] = exec_atomic;
    instruction_table[INST_AMOAND_D] = exec_atomic;
    instruction_table[INST_AMOOR_D] = exec_atomic;
    instruction_table[INST_AMOMIN_D] = exec_atomic;
    instruction_table[INST_AMOMAX_D] = exec_atomic;
    instruction_table[INST_AMOMINU_D] = exec_atomic;
    instruction_table[INST_AMOMAXU_D] = exec_atomic;
#endif
    
    // Floating-point single precision
    instruction_table[INST_FADD_S] = exec_float_single;
    instruction_table[INST_FSUB_S] = exec_float_single;
    instruction_table[INST_FMUL_S] = exec_float_single;
    instruction_table[INST_FDIV_S] = exec_float_single;
    instruction_table[INST_FSQRT_S] = exec_float_single;
    instruction_table[INST_FMIN_S] = exec_float_single;
    instruction_table[INST_FMAX_S] = exec_float_single;
    instruction_table[INST_FSGNJ_S] = exec_float_single;
    instruction_table[INST_FSGNJN_S] = exec_float_single;
    instruction_table[INST_FSGNJX_S] = exec_float_single;
    instruction_table[INST_FEQ_S] = exec_float_single;
    instruction_table[INST_FLT_S] = exec_float_single;
    instruction_table[INST_FLE_S] = exec_float_single;
    instruction_table[INST_FCLASS_S] = exec_float_single;
    instruction_table[INST_FCVT_W_S] = exec_float_single;
    instruction_table[INST_FCVT_WU_S] = exec_float_single;
    instruction_table[INST_FCVT_S_W] = exec_float_single;
    instruction_table[INST_FCVT_S_WU] = exec_float_single;
    instruction_table[INST_FMV_X_W] = exec_float_single;
    instruction_table[INST_FMV_W_X] = exec_float_single;
    instruction_table[INST_FMADD_S] = exec_float_single;
    instruction_table[INST_FMSUB_S] = exec_float_single;
    instruction_table[INST_FNMSUB_S] = exec_float_single;
    instruction_table[INST_FNMADD_S] = exec_float_single;
    
    // Floating-point double precision
    instruction_table[INST_FADD_D] = exec_float_double;
    instruction_table[INST_FSUB_D] = exec_float_double;
    instruction_table[INST_FMUL_D] = exec_float_double;
    instruction_table[INST_FDIV_D] = exec_float_double;
    instruction_table[INST_FSQRT_D] = exec_float_double;
    instruction_table[INST_FMIN_D] = exec_float_double;
    instruction_table[INST_FMAX_D] = exec_float_double;
    instruction_table[INST_FSGNJ_D] = exec_float_double;
    instruction_table[INST_FSGNJN_D] = exec_float_double;
    instruction_table[INST_FSGNJX_D] = exec_float_double;
    instruction_table[INST_FEQ_D] = exec_float_double;
    instruction_table[INST_FLT_D] = exec_float_double;
    instruction_table[INST_FLE_D] = exec_float_double;
    instruction_table[INST_FCLASS_D] = exec_float_double;
    instruction_table[INST_FCVT_W_D] = exec_float_double;
    instruction_table[INST_FCVT_WU_D] = exec_float_double;
    instruction_table[INST_FCVT_D_W] = exec_float_double;
    instruction_table[INST_FCVT_D_WU] = exec_float_double;
    instruction_table[INST_FCVT_S_D] = exec_float_double;
    instruction_table[INST_FCVT_D_S] = exec_float_double;
    instruction_table[INST_FMADD_D] = exec_float_double;
    instruction_table[INST_FMSUB_D] = exec_float_double;
    instruction_table[INST_FNMSUB_D] = exec_float_double;
    instruction_table[INST_FNMADD_D] = exec_float_double;
    
    // Floating-point memory operations
    instruction_table[INST_FLW] = exec_float_mem;
    instruction_table[INST_FSW] = exec_float_mem;
    instruction_table[INST_FLD] = exec_float_mem;
    
#if XLEN == 64
    // RV64 specific operations
    instruction_table[INST_ADDIW] = exec_rv64;
    instruction_table[INST_SLLIW] = exec_rv64;
    instruction_table[INST_SRLIW] = exec_rv64;
    instruction_table[INST_SRAIW] = exec_rv64;
    instruction_table[INST_ADDW] = exec_rv64;
    instruction_table[INST_SUBW] = exec_rv64;
    instruction_table[INST_SLLW] = exec_rv64;
    instruction_table[INST_SRLW] = exec_rv64;
    instruction_table[INST_SRAW] = exec_rv64;
#endif
}

// 🚀 BLAZING FAST GROUPED INSTRUCTION HANDLERS 🚀

// ALU Register operations - SWITCH OPTIMIZED
static void exec_alu_reg(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    if (decoded->rd == 0) return; // x0 hardwired to 0
    
    switch (decoded->inst_type) {
        case INST_ADD:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] + cpu->regs[decoded->rs2];
            break;
        case INST_SUB:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] - cpu->regs[decoded->rs2];
            break;
        case INST_SLL:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] << (cpu->regs[decoded->rs2] & (XLEN-1));
            break;
        case INST_SLT:
            cpu->regs[decoded->rd] = ((sreg_t)cpu->regs[decoded->rs1] < (sreg_t)cpu->regs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_SLTU:
            cpu->regs[decoded->rd] = (cpu->regs[decoded->rs1] < cpu->regs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_XOR:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] ^ cpu->regs[decoded->rs2];
            break;
        case INST_SRL:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] >> (cpu->regs[decoded->rs2] & (XLEN-1));
            break;
        case INST_SRA:
            cpu->regs[decoded->rd] = (sreg_t)cpu->regs[decoded->rs1] >> (cpu->regs[decoded->rs2] & (XLEN-1));
            break;
        case INST_OR:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] | cpu->regs[decoded->rs2];
            break;
        case INST_AND:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] & cpu->regs[decoded->rs2];
            break;
        case INST_MUL:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] * cpu->regs[decoded->rs2];
            break;
        case INST_MULH: {
            int64_t result = (int64_t)(sreg_t)cpu->regs[decoded->rs1] * (int64_t)(sreg_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = result >> XLEN;
            break;
        }
        case INST_MULHSU: {
            int64_t result = (int64_t)(sreg_t)cpu->regs[decoded->rs1] * (uint64_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = result >> XLEN;
            break;
        }
        case INST_MULHU: {
            uint64_t result = (uint64_t)cpu->regs[decoded->rs1] * (uint64_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = result >> XLEN;
            break;
        }
        case INST_DIV: {
            sreg_t dividend = (sreg_t)cpu->regs[decoded->rs1];
            sreg_t divisor = (sreg_t)cpu->regs[decoded->rs2];
            if (divisor == 0) cpu->regs[decoded->rd] = -1;
            else if (dividend == (sreg_t)(1ULL << (XLEN-1)) && divisor == -1) cpu->regs[decoded->rd] = dividend;
            else cpu->regs[decoded->rd] = dividend / divisor;
            break;
        }
        case INST_DIVU: {
            reg_t dividend = cpu->regs[decoded->rs1];
            reg_t divisor = cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = (divisor == 0) ? ~0ULL : dividend / divisor;
            break;
        }
        case INST_REM: {
            sreg_t dividend = (sreg_t)cpu->regs[decoded->rs1];
            sreg_t divisor = (sreg_t)cpu->regs[decoded->rs2];
            if (divisor == 0) cpu->regs[decoded->rd] = dividend;
            else if (dividend == (sreg_t)(1ULL << (XLEN-1)) && divisor == -1) cpu->regs[decoded->rd] = 0;
            else cpu->regs[decoded->rd] = dividend % divisor;
            break;
        }
        case INST_REMU: {
            reg_t dividend = cpu->regs[decoded->rs1];
            reg_t divisor = cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = (divisor == 0) ? dividend : dividend % divisor;
            break;
        }
#if XLEN == 64
        case INST_MULW: {
            int32_t op1 = (int32_t)cpu->regs[decoded->rs1];
            int32_t op2 = (int32_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = (int64_t)(op1 * op2);
            break;
        }
        case INST_DIVW: {
            int32_t dividend = (int32_t)cpu->regs[decoded->rs1];
            int32_t divisor = (int32_t)cpu->regs[decoded->rs2];
            if (divisor == 0) cpu->regs[decoded->rd] = -1;
            else if (dividend == (int32_t)(1 << 31) && divisor == -1) cpu->regs[decoded->rd] = dividend;
            else cpu->regs[decoded->rd] = (int64_t)(dividend / divisor);
            break;
        }
        case INST_DIVUW: {
            uint32_t dividend = (uint32_t)cpu->regs[decoded->rs1];
            uint32_t divisor = (uint32_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = (divisor == 0) ? ~0ULL : (uint64_t)(dividend / divisor);
            break;
        }
        case INST_REMW: {
            int32_t dividend = (int32_t)cpu->regs[decoded->rs1];
            int32_t divisor = (int32_t)cpu->regs[decoded->rs2];
            if (divisor == 0) cpu->regs[decoded->rd] = (int64_t)dividend;
            else if (dividend == (int32_t)(1 << 31) && divisor == -1) cpu->regs[decoded->rd] = 0;
            else cpu->regs[decoded->rd] = (int64_t)(dividend % divisor);
            break;
        }
        case INST_REMUW: {
            uint32_t dividend = (uint32_t)cpu->regs[decoded->rs1];
            uint32_t divisor = (uint32_t)cpu->regs[decoded->rs2];
            cpu->regs[decoded->rd] = (divisor == 0) ? (uint64_t)dividend : (uint64_t)(dividend % divisor);
            break;
        }
#endif
    }
}

// ALU Immediate operations - SWITCH OPTIMIZED
static void exec_alu_imm(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    if (decoded->rd == 0) return;
    
    switch (decoded->inst_type) {
        case INST_ADDI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] + (sreg_t)decoded->imm;
            break;
        case INST_SLTI:
            cpu->regs[decoded->rd] = ((sreg_t)cpu->regs[decoded->rs1] < (sreg_t)decoded->imm) ? 1 : 0;
            break;
        case INST_SLTIU:
            cpu->regs[decoded->rd] = (cpu->regs[decoded->rs1] < (reg_t)decoded->imm) ? 1 : 0;
            break;
        case INST_XORI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] ^ decoded->imm;
            break;
        case INST_ORI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] | decoded->imm;
            break;
        case INST_ANDI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] & decoded->imm;
            break;
        case INST_SLLI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] << decoded->imm;
            break;
        case INST_SRLI:
            cpu->regs[decoded->rd] = cpu->regs[decoded->rs1] >> decoded->imm;
            break;
        case INST_SRAI:
            cpu->regs[decoded->rd] = (sreg_t)cpu->regs[decoded->rs1] >> decoded->imm;
            break;
    }
}

// Load operations - SWITCH OPTIMIZED
static void exec_load(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    if (decoded->rd == 0) return;
    
    reg_t addr = cpu->regs[decoded->rs1] + (sreg_t)decoded->imm;
    
    switch (decoded->inst_type) {
        case INST_LB:
            cpu->regs[decoded->rd] = (sreg_t)(int8_t)memory_read_byte(memory, addr);
            break;
        case INST_LH:
            cpu->regs[decoded->rd] = (sreg_t)(int16_t)memory_read_halfword(memory, addr);
            break;
        case INST_LW:
            cpu->regs[decoded->rd] = (sreg_t)(int32_t)memory_read_word(memory, addr);
            break;
        case INST_LBU:
            cpu->regs[decoded->rd] = memory_read_byte(memory, addr);
            break;
        case INST_LHU:
            cpu->regs[decoded->rd] = memory_read_halfword(memory, addr);
            break;
#if XLEN == 64
        case INST_LWU:
            cpu->regs[decoded->rd] = memory_read_word(memory, addr);
            break;
        case INST_LD: {
            uint64_t low = memory_read_word(memory, addr);
            uint64_t high = memory_read_word(memory, addr + 4);
            cpu->regs[decoded->rd] = low | (high << 32);
            break;
        }
#endif
    }
}

// Store operations - SWITCH OPTIMIZED
static void exec_store(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    reg_t addr = cpu->regs[decoded->rs1] + (sreg_t)decoded->imm;
    
    switch (decoded->inst_type) {
        case INST_SB:
            memory_write_byte(memory, addr, cpu->regs[decoded->rs2]);
            break;
        case INST_SH:
            memory_write_halfword(memory, addr, cpu->regs[decoded->rs2]);
            break;
        case INST_SW:
            memory_write_word(memory, addr, cpu->regs[decoded->rs2]);
            break;
#if XLEN == 64
        case INST_SD: {
            uint64_t value = cpu->regs[decoded->rs2];
            memory_write_word(memory, addr, value & 0xFFFFFFFF);
            memory_write_word(memory, addr + 4, value >> 32);
            break;
        }
#endif
    }
}

// Branch operations - SWITCH OPTIMIZED
static void exec_branch(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    int taken = 0;
    
    switch (decoded->inst_type) {
        case INST_BEQ:
            taken = (cpu->regs[decoded->rs1] == cpu->regs[decoded->rs2]);
            break;
        case INST_BNE:
            taken = (cpu->regs[decoded->rs1] != cpu->regs[decoded->rs2]);
            break;
        case INST_BLT:
            taken = ((sreg_t)cpu->regs[decoded->rs1] < (sreg_t)cpu->regs[decoded->rs2]);
            break;
        case INST_BGE:
            taken = ((sreg_t)cpu->regs[decoded->rs1] >= (sreg_t)cpu->regs[decoded->rs2]);
            break;
        case INST_BLTU:
            taken = (cpu->regs[decoded->rs1] < cpu->regs[decoded->rs2]);
            break;
        case INST_BGEU:
            taken = (cpu->regs[decoded->rs1] >= cpu->regs[decoded->rs2]);
            break;
    }
    
    if (taken) {
        cpu->pc += (sreg_t)decoded->imm - 4;
    }
}

// Jump operations - SWITCH OPTIMIZED
static void exec_jump(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    switch (decoded->inst_type) {
        case INST_JAL:
            if (decoded->rd != 0) {
                cpu->regs[decoded->rd] = cpu->pc + 4;
            }
            cpu->pc += (sreg_t)decoded->imm;
            break;
        case INST_JALR:
            if (decoded->rd != 0) {
                cpu->regs[decoded->rd] = cpu->pc + 4;
            }
            cpu->pc = (cpu->regs[decoded->rs1] + (sreg_t)decoded->imm) & ~1;
            break;
    }
}

// Upper immediate operations
static void exec_upper(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    if (decoded->rd == 0) return;
    
    switch (decoded->inst_type) {
        case INST_LUI:
            cpu->regs[decoded->rd] = decoded->imm;
            break;
        case INST_AUIPC:
            cpu->regs[decoded->rd] = cpu->pc + decoded->imm;
            break;
    }
}

// System operations - SWITCH OPTIMIZED
static void exec_system(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    switch (decoded->inst_type) {
        case INST_ECALL:
            if (cpu->privilege == USER_MODE) {
                cpu->csrs[CSR_MEPC] = cpu->pc;
                cpu->csrs[CSR_MCAUSE] = 8;
            } else if (cpu->privilege == SUPERVISOR_MODE) {
                cpu->csrs[CSR_SEPC] = cpu->pc;
                cpu->csrs[CSR_SCAUSE] = 8;
            } else {
                cpu->csrs[CSR_MEPC] = cpu->pc;
                cpu->csrs[CSR_MCAUSE] = 11;
            }
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            break;
        case INST_EBREAK:
            cpu->csrs[CSR_MEPC] = cpu->pc;
            cpu->csrs[CSR_MCAUSE] = 3;
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            break;
        case INST_MRET: {
            uint32_t mstatus = cpu->csrs[CSR_MSTATUS];
            cpu->pc = cpu->csrs[CSR_MEPC];
            cpu->privilege = (mstatus >> 11) & 0x3;
            mstatus = (mstatus & ~(1 << 7)) | (((mstatus >> 7) & 1) << 3);
            mstatus |= (1 << 7);
            mstatus &= ~(0x3 << 11);
            cpu->csrs[CSR_MSTATUS] = mstatus;
            break;
        }
        case INST_SRET: {
            uint32_t sstatus = cpu->csrs[CSR_SSTATUS];
            cpu->pc = cpu->csrs[CSR_SEPC];
            cpu->privilege = (sstatus >> 8) & 0x1;
            sstatus = (sstatus & ~(1 << 5)) | (((sstatus >> 5) & 1) << 1);
            sstatus |= (1 << 5);
            sstatus &= ~(0x1 << 8);
            cpu->csrs[CSR_SSTATUS] = sstatus;
            break;
        }
        case INST_URET:
            cpu->csrs[CSR_MEPC] = cpu->pc;
            cpu->csrs[CSR_MCAUSE] = 2;
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            break;
        case INST_CSRRW:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] = cpu->regs[decoded->rs1];
            break;
        case INST_CSRRS:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] |= cpu->regs[decoded->rs1];
            break;
        case INST_CSRRC:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] &= ~cpu->regs[decoded->rs1];
            break;
        case INST_CSRRWI:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] = decoded->rs1;
            break;
        case INST_CSRRSI:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] |= decoded->rs1;
            break;
        case INST_CSRRCI:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = cpu->csrs[decoded->imm];
            cpu->csrs[decoded->imm] &= ~decoded->rs1;
            break;
        case INST_FENCE:
        case INST_FENCE_I:
        case INST_SFENCE_VMA:
        case INST_WFI:
            // NOPs for this emulator
            break;
    }
}

// Atomic operations - SWITCH OPTIMIZED
static void exec_atomic(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    reg_t addr = cpu->regs[decoded->rs1];
    
    switch (decoded->inst_type) {
        case INST_LR_W:
            if (decoded->rd != 0) {
                cpu->regs[decoded->rd] = (sreg_t)(int32_t)memory_read_word(memory, addr);
                cpu->reserved_address = addr;
                cpu->reservation_set = 1;
            }
            break;
        case INST_SC_W:
            if (decoded->rd != 0) {
                if (cpu->reservation_set && cpu->reserved_address == addr) {
                    memory_write_word(memory, addr, cpu->regs[decoded->rs2]);
                    cpu->regs[decoded->rd] = 0;
                } else {
                    cpu->regs[decoded->rd] = 1;
                }
                cpu->reservation_set = 0;
            }
            break;
        case INST_AMOSWAP_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOADD_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, temp + cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOXOR_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, temp ^ cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOAND_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, temp & cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOOR_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, temp | cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOMIN_W: {
            uint32_t temp = memory_read_word(memory, addr);
            int32_t a = (int32_t)temp, b = (int32_t)cpu->regs[decoded->rs2];
            memory_write_word(memory, addr, (a < b) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOMAX_W: {
            uint32_t temp = memory_read_word(memory, addr);
            int32_t a = (int32_t)temp, b = (int32_t)cpu->regs[decoded->rs2];
            memory_write_word(memory, addr, (a > b) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOMINU_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, (temp < cpu->regs[decoded->rs2]) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
        case INST_AMOMAXU_W: {
            uint32_t temp = memory_read_word(memory, addr);
            memory_write_word(memory, addr, (temp > cpu->regs[decoded->rs2]) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)temp;
            break;
        }
#if XLEN == 64
        case INST_LR_D: {
            if (decoded->rd != 0) {
                cpu->regs[decoded->rd] = memory_read_doubleword(memory, addr);
                cpu->reserved_address = addr;
                cpu->reservation_set = 1;
            }
            break;
        }
        case INST_SC_D: {
            if (decoded->rd != 0) {
                if (cpu->reservation_set && cpu->reserved_address == addr) {
                    memory_write_doubleword(memory, addr, cpu->regs[decoded->rs2]);
                    cpu->regs[decoded->rd] = 0;
                } else {
                    cpu->regs[decoded->rd] = 1;
                }
                cpu->reservation_set = 0;
            }
            break;
        }
        case INST_AMOSWAP_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOADD_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, temp + cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOXOR_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, temp ^ cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOAND_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, temp & cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOOR_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, temp | cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOMIN_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            int64_t a = (int64_t)temp, b = (int64_t)cpu->regs[decoded->rs2];
            memory_write_doubleword(memory, addr, (a < b) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOMAX_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            int64_t a = (int64_t)temp, b = (int64_t)cpu->regs[decoded->rs2];
            memory_write_doubleword(memory, addr, (a > b) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOMINU_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, (temp < cpu->regs[decoded->rs2]) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
        case INST_AMOMAXU_D: {
            uint64_t temp = memory_read_doubleword(memory, addr);
            memory_write_doubleword(memory, addr, (temp > cpu->regs[decoded->rs2]) ? temp : cpu->regs[decoded->rs2]);
            if (decoded->rd != 0) cpu->regs[decoded->rd] = temp;
            break;
        }
#endif
    }
}

// Floating-point single precision - SWITCH OPTIMIZED
static void exec_float_single(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    switch (decoded->inst_type) {
        case INST_FADD_S:
            cpu->fregs[decoded->rd] = cpu->fregs[decoded->rs1] + cpu->fregs[decoded->rs2];
            break;
        case INST_FSUB_S:
            cpu->fregs[decoded->rd] = cpu->fregs[decoded->rs1] - cpu->fregs[decoded->rs2];
            break;
        case INST_FMUL_S:
            cpu->fregs[decoded->rd] = cpu->fregs[decoded->rs1] * cpu->fregs[decoded->rs2];
            break;
        case INST_FDIV_S:
            cpu->fregs[decoded->rd] = cpu->fregs[decoded->rs1] / cpu->fregs[decoded->rs2];
            break;
        case INST_FSQRT_S:
            cpu->fregs[decoded->rd] = sqrtf(cpu->fregs[decoded->rs1]);
            break;
        case INST_FMIN_S:
            cpu->fregs[decoded->rd] = fminf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2]);
            break;
        case INST_FMAX_S:
            cpu->fregs[decoded->rd] = fmaxf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2]);
            break;
        case INST_FSGNJ_S:
            cpu->fregs[decoded->rd] = copysignf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2]);
            break;
        case INST_FSGNJN_S:
            cpu->fregs[decoded->rd] = copysignf(cpu->fregs[decoded->rs1], -cpu->fregs[decoded->rs2]);
            break;
        case INST_FSGNJX_S: {
            uint32_t a = *(uint32_t*)&cpu->fregs[decoded->rs1];
            uint32_t b = *(uint32_t*)&cpu->fregs[decoded->rs2];
            uint32_t result = a ^ (b & 0x80000000);
            cpu->fregs[decoded->rd] = *(float*)&result;
            break;
        }
        case INST_FEQ_S:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->fregs[decoded->rs1] == cpu->fregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FLT_S:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->fregs[decoded->rs1] < cpu->fregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FLE_S:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->fregs[decoded->rs1] <= cpu->fregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FCVT_W_S:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)cpu->fregs[decoded->rs1];
            break;
        case INST_FCVT_WU_S:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (reg_t)(uint32_t)cpu->fregs[decoded->rs1];
            break;
        case INST_FCVT_S_W:
            cpu->fregs[decoded->rd] = (float)(sreg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FCVT_S_WU:
            cpu->fregs[decoded->rd] = (float)(reg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FMV_X_W:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)*(uint32_t*)&cpu->fregs[decoded->rs1];
            break;
        case INST_FMV_W_X:
            cpu->fregs[decoded->rd] = *(float*)&cpu->regs[decoded->rs1];
            break;
        case INST_FMADD_S:
            cpu->fregs[decoded->rd] = fmaf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2], cpu->fregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FMSUB_S:
            cpu->fregs[decoded->rd] = fmaf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2], -cpu->fregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FNMSUB_S:
            cpu->fregs[decoded->rd] = -fmaf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2], -cpu->fregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FNMADD_S:
            cpu->fregs[decoded->rd] = -fmaf(cpu->fregs[decoded->rs1], cpu->fregs[decoded->rs2], cpu->fregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FCLASS_S:
            if (decoded->rd != 0) {
                float val = cpu->fregs[decoded->rs1];
                uint32_t result = 0;
                if (isnan(val)) result = (signbit(val)) ? 0x200 : 0x100;
                else if (isinf(val)) result = (signbit(val)) ? 0x001 : 0x080;
                else if (val == 0.0f) result = (signbit(val)) ? 0x008 : 0x010;
                else if (isnormal(val)) result = (signbit(val)) ? 0x002 : 0x040;
                else result = (signbit(val)) ? 0x004 : 0x020;
                cpu->regs[decoded->rd] = result;
            }
            break;
    }
}

// Floating-point double precision - SWITCH OPTIMIZED
static void exec_float_double(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    switch (decoded->inst_type) {
        case INST_FADD_D:
            cpu->dfregs[decoded->rd] = cpu->dfregs[decoded->rs1] + cpu->dfregs[decoded->rs2];
            break;
        case INST_FSUB_D:
            cpu->dfregs[decoded->rd] = cpu->dfregs[decoded->rs1] - cpu->dfregs[decoded->rs2];
            break;
        case INST_FMUL_D:
            cpu->dfregs[decoded->rd] = cpu->dfregs[decoded->rs1] * cpu->dfregs[decoded->rs2];
            break;
        case INST_FDIV_D:
            cpu->dfregs[decoded->rd] = cpu->dfregs[decoded->rs1] / cpu->dfregs[decoded->rs2];
            break;
        case INST_FSQRT_D:
            cpu->dfregs[decoded->rd] = sqrt(cpu->dfregs[decoded->rs1]);
            break;
        case INST_FMIN_D:
            cpu->dfregs[decoded->rd] = fmin(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2]);
            break;
        case INST_FMAX_D:
            cpu->dfregs[decoded->rd] = fmax(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2]);
            break;
        case INST_FSGNJ_D:
            cpu->dfregs[decoded->rd] = copysign(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2]);
            break;
        case INST_FSGNJN_D:
            cpu->dfregs[decoded->rd] = copysign(cpu->dfregs[decoded->rs1], -cpu->dfregs[decoded->rs2]);
            break;
        case INST_FSGNJX_D: {
            uint64_t a = *(uint64_t*)&cpu->dfregs[decoded->rs1];
            uint64_t b = *(uint64_t*)&cpu->dfregs[decoded->rs2];
            uint64_t result = a ^ (b & 0x8000000000000000ULL);
            cpu->dfregs[decoded->rd] = *(double*)&result;
            break;
        }
        case INST_FEQ_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->dfregs[decoded->rs1] == cpu->dfregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FLT_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->dfregs[decoded->rs1] < cpu->dfregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FLE_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (cpu->dfregs[decoded->rs1] <= cpu->dfregs[decoded->rs2]) ? 1 : 0;
            break;
        case INST_FCVT_W_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)(int32_t)cpu->dfregs[decoded->rs1];
            break;
        case INST_FCVT_WU_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (reg_t)(uint32_t)cpu->dfregs[decoded->rs1];
            break;
        case INST_FCVT_D_W:
            cpu->dfregs[decoded->rd] = (double)(sreg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FCVT_D_WU:
            cpu->dfregs[decoded->rd] = (double)(reg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FCVT_L_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (sreg_t)cpu->dfregs[decoded->rs1];
            break;
        case INST_FCVT_LU_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = (reg_t)cpu->dfregs[decoded->rs1];
            break;
        case INST_FMV_X_D:
            if (decoded->rd != 0) cpu->regs[decoded->rd] = *(uint64_t*)&cpu->dfregs[decoded->rs1];
            break;
        case INST_FCVT_D_L:
            cpu->dfregs[decoded->rd] = (double)(sreg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FCVT_D_LU:
            cpu->dfregs[decoded->rd] = (double)(reg_t)cpu->regs[decoded->rs1];
            break;
        case INST_FMV_D_X:
            cpu->dfregs[decoded->rd] = *(double*)&cpu->regs[decoded->rs1];
            break;
        case INST_FCVT_S_D:
            cpu->fregs[decoded->rd] = (float)cpu->dfregs[decoded->rs1];
            break;
        case INST_FCVT_D_S:
            cpu->dfregs[decoded->rd] = (double)cpu->fregs[decoded->rs1];
            break;
        case INST_FMADD_D:
            cpu->dfregs[decoded->rd] = fma(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2], cpu->dfregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FMSUB_D:
            cpu->dfregs[decoded->rd] = fma(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2], -cpu->dfregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FNMSUB_D:
            cpu->dfregs[decoded->rd] = -fma(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2], -cpu->dfregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FNMADD_D:
            cpu->dfregs[decoded->rd] = -fma(cpu->dfregs[decoded->rs1], cpu->dfregs[decoded->rs2], cpu->dfregs[(instruction >> 27) & 0x1F]);
            break;
        case INST_FCLASS_D:
            if (decoded->rd != 0) {
                double val = cpu->dfregs[decoded->rs1];
                uint32_t result = 0;
                if (isnan(val)) result = (signbit(val)) ? 0x200 : 0x100;
                else if (isinf(val)) result = (signbit(val)) ? 0x001 : 0x080;
                else if (val == 0.0) result = (signbit(val)) ? 0x008 : 0x010;
                else if (isnormal(val)) result = (signbit(val)) ? 0x002 : 0x040;
                else result = (signbit(val)) ? 0x004 : 0x020;
                cpu->regs[decoded->rd] = result;
            }
            break;
    }
}

// Floating-point memory operations
static void exec_float_mem(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    reg_t addr = cpu->regs[decoded->rs1] + (sreg_t)decoded->imm;
    
    switch (decoded->inst_type) {
        case INST_FLW:
            if (decoded->rd != 0) {
                uint32_t raw = memory_read_word(memory, addr);
                cpu->fregs[decoded->rd] = *(float*)&raw;
            }
            break;
        case INST_FSW: {
            uint32_t raw = *(uint32_t*)&cpu->fregs[decoded->rs2];
            memory_write_word(memory, addr, raw);
            break;
        }
        case INST_FLD:
            if (decoded->rd != 0) {
                uint64_t low = memory_read_word(memory, addr);
                uint64_t high = memory_read_word(memory, addr + 4);
                uint64_t raw = low | (high << 32);
                cpu->dfregs[decoded->rd] = *(double*)&raw;
            }
            break;
    }
}

#if XLEN == 64
// RV64 specific operations - SWITCH OPTIMIZED
static void exec_rv64(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    switch (decoded->inst_type) {
        case INST_ADDIW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] + decoded->imm;
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_SLLIW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] << decoded->imm;
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_SRLIW:
            if (decoded->rd != 0) {
                uint32_t result = (uint32_t)cpu->regs[decoded->rs1] >> decoded->imm;
                cpu->regs[decoded->rd] = (int64_t)(int32_t)result;
            }
            break;
        case INST_SRAIW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] >> decoded->imm;
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_ADDW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] + (int32_t)cpu->regs[decoded->rs2];
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_SUBW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] - (int32_t)cpu->regs[decoded->rs2];
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_SLLW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] << (cpu->regs[decoded->rs2] & 0x1F);
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
        case INST_SRLW:
            if (decoded->rd != 0) {
                uint32_t result = (uint32_t)cpu->regs[decoded->rs1] >> (cpu->regs[decoded->rs2] & 0x1F);
                cpu->regs[decoded->rd] = (int64_t)(int32_t)result;
            }
            break;
        case INST_SRAW:
            if (decoded->rd != 0) {
                int32_t result = (int32_t)cpu->regs[decoded->rs1] >> (cpu->regs[decoded->rs2] & 0x1F);
                cpu->regs[decoded->rd] = (int64_t)result;
            }
            break;
    }
}
#endif

static void exec_unknown(cpu_t* cpu, memory_t* memory, instruction_t* decoded, uint32_t instruction) {
    printf("Unknown instruction: 0x%08x\n", instruction);
}