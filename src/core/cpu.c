#include "cpu.h"
#include "decode.h"
#include "memory.h"
#include <stdio.h>

void cpu_init(cpu_t* cpu) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        cpu->regs[i] = 0;
    }
    for (int i = 0; i < 4096; i++) {
        cpu->csrs[i] = 0;
    }
    cpu->pc = 0;
    cpu->privilege = MACHINE_MODE;
}

void cpu_execute(cpu_t* cpu, memory_t* memory, uint32_t instruction) {
    instruction_t decoded;
    decode_instruction(instruction, &decoded);

    switch (decoded.inst_type) {
        case INST_SLT:
            if (decoded.rd != 0) { // x0 is hardwired to 0
                cpu->regs[decoded.rd] = ((int32_t)cpu->regs[decoded.rs1] < (int32_t) cpu->regs[decoded.rs2]) ? 1 : 0;
            }
            break;
        case INST_SLTU:
            if (decoded.rd != 0) { // x0 is hardwired to 0
                cpu->regs[decoded.rd] = (cpu->regs[decoded.rs1] < cpu->regs[decoded.rs2]) ? 1 : 0;
            }
            break;
        case INST_SLL:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] << (cpu->regs[decoded.rs2] & 0x1F);
            }
            break;
        case INST_SRL:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] >> (cpu->regs[decoded.rs2] & 0x1F);
            }
            break;
        case INST_SRA:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = (int32_t)cpu->regs[decoded.rs1] >> (int32_t)decoded.rs2 & 0x1F;
            }
            break;
        case INST_ADD:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] + cpu->regs[decoded.rs2];
            }
            break;
        case INST_SUB:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] - cpu->regs[decoded.rs2];
            }
            break;
        case INST_MUL:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] * cpu->regs[decoded.rs2];
            }
            break;
        case INST_MULH:
            if (decoded.rd != 0) {
                int64_t result = (int64_t)(int32_t)cpu->regs[decoded.rs1] * (int64_t)(int32_t)cpu->regs[decoded.rs2];
                cpu->regs[decoded.rd] = (uint32_t)(result >> 32);
            }
            break;
        case INST_MULHSU:
            if (decoded.rd != 0) {
                int64_t result = (int64_t)(int32_t)cpu->regs[decoded.rs1] * (uint64_t)cpu->regs[decoded.rs2];
                cpu->regs[decoded.rd] = (uint32_t)(result >> 32);
            }
            break;
        case INST_MULHU:
            if (decoded.rd != 0) {
                uint64_t result = (uint64_t)cpu->regs[decoded.rs1] * (uint64_t)cpu->regs[decoded.rs2];
                cpu->regs[decoded.rd] = (uint32_t)(result >> 32);
            }
            break;
        case INST_DIV:
            if (decoded.rd != 0) {
                int32_t dividend = (int32_t)cpu->regs[decoded.rs1];
                int32_t divisor = (int32_t)cpu->regs[decoded.rs2];
                if (divisor == 0) {
                    cpu->regs[decoded.rd] = -1;
                } else if (dividend == INT32_MIN && divisor == -1) {
                    cpu->regs[decoded.rd] = dividend;
                } else {
                    cpu->regs[decoded.rd] = dividend / divisor;
                }
            }
            break;
        case INST_DIVU:
            if (decoded.rd != 0) {
                uint32_t dividend = cpu->regs[decoded.rs1];
                uint32_t divisor = cpu->regs[decoded.rs2];
                if (divisor == 0) {
                    cpu->regs[decoded.rd] = UINT32_MAX;
                } else {
                    cpu->regs[decoded.rd] = dividend / divisor;
                }
            }
            break;
        case INST_REM:
            if (decoded.rd != 0) {
                int32_t dividend = (int32_t)cpu->regs[decoded.rs1];
                int32_t divisor = (int32_t)cpu->regs[decoded.rs2];
                if (divisor == 0) {
                    cpu->regs[decoded.rd] = dividend;
                } else if (dividend == INT32_MIN && divisor == -1) {
                    cpu->regs[decoded.rd] = 0;
                }
                else {
                    cpu->regs[decoded.rd] = dividend % divisor;
                }
            }
            break;
        case INST_REMU:
            if (decoded.rd != 0) {
                uint32_t dividend = cpu->regs[decoded.rs1];
                uint32_t divisor = cpu->regs[decoded.rs2];
                if (divisor == 0) {
                    cpu->regs[decoded.rd] = dividend;
                } else {
                    cpu->regs[decoded.rd] = dividend % divisor;
                }
            }
            break;
        case INST_XOR:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] ^ cpu->regs[decoded.rs2];
            }
            break;
        case INST_OR:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] | cpu->regs[decoded.rs2];
            }
            break;
        case INST_AND:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] & cpu->regs[decoded.rs2];
            }
            break;
        // IMMEDIATE INSTRUCTION
        case INST_ADDI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] + decoded.imm;
            }
            break;
        case INST_SLTI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = (int32_t)cpu->regs[decoded.rs1] < decoded.imm ? 1 : 0;
            }
            break;
        case INST_SLTIU:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] < (uint32_t)decoded.imm ? 1 : 0;
            }
            break;
        case INST_XORI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] ^ decoded.imm;
            }
            break;
        case INST_ORI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] | decoded.imm;
            }
            break;
        case INST_ANDI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] & decoded.imm;
            }
            break;
        case INST_SLLI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] << decoded.imm;
            }
            break;
        case INST_SRLI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->regs[decoded.rs1] >> decoded.imm;
            }
            break;
        case INST_SRAI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = (int32_t)cpu->regs[decoded.rs1] >> (int32_t)decoded.imm;
            }
            break;
        // Load Instructions
        case INST_LB:
            if (decoded.rd != 0) {
                uint32_t addr = cpu->regs[decoded.rs1] + decoded.imm;
                cpu->regs[decoded.rd] = (int32_t)(int8_t)memory_read_byte(memory, addr);
            }
            break;
        case INST_LH:
            if (decoded.rd != 0) {
                uint32_t addr = cpu->regs[decoded.rs1] + decoded.imm;
                cpu->regs[decoded.rd] = (int32_t)(int16_t)memory_read_halfword(memory, addr);
            }
            break;
        case INST_LW:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = memory_read_word(memory, cpu->regs[decoded.rs1] + decoded.imm);
            }
            break;
        case INST_LBU:
            if (decoded.rd != 0) {
                uint32_t addr = cpu->regs[decoded.rs1] + decoded.imm;
                cpu->regs[decoded.rd] = memory_read_byte(memory, addr);
            }
            break;
        case INST_LHU:
            if (decoded.rd != 0) {
                uint32_t addr = cpu->regs[decoded.rs1] + decoded.imm;
                cpu->regs[decoded.rd] = memory_read_halfword(memory, addr);
            }
            break;

        case INST_SB:
            memory_write_byte(memory, cpu->regs[decoded.rs1] + decoded.imm, cpu->regs[decoded.rs2]);
            break;
        case INST_SH:
            memory_write_halfword(memory, cpu->regs[decoded.rs1] + decoded.imm, cpu->regs[decoded.rs2]);
            break;
        case INST_SW:
            memory_write_word(memory, cpu->regs[decoded.rs1] + decoded.imm, cpu->regs[decoded.rs2]);
            break;
        case INST_BEQ:
            if (cpu->regs[decoded.rs1] == cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm - 4;
            }
            break;
        case INST_BNE:
            if (cpu->regs[decoded.rs1] != cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm - 4;
            }
            break;
        case INST_BLT:
            if ((int32_t)cpu->regs[decoded.rs1] < (int32_t)cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm;
            }
            break;
        case INST_BGE:
            if ((int32_t)cpu->regs[decoded.rs1] >= (int32_t)cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm;
            }
            break;
        case INST_BLTU:
            if (cpu->regs[decoded.rs1] < cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm;
            }
            break;
        case INST_BGEU:
            if (cpu->regs[decoded.rs1] >= cpu->regs[decoded.rs2]) {
                cpu->pc += decoded.imm;
            }
            break;
        case INST_JAL:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->pc + 4;
            }
            cpu->pc += decoded.imm;
            return;
        case INST_JALR:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->pc + 4;
            }
            cpu->pc = (cpu->regs[decoded.rs1] + decoded.imm) & ~1;
            return;
        case INST_LUI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = decoded.imm;
            }
            break;
        case INST_AUIPC:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->pc + decoded.imm;
            }
            break;
        case INST_FENCE:
            // NOP for this simple emulator
            break;
        case INST_FENCE_I:
            // Instruction cache flush - NOP for this emulator
            break;
        case INST_SFENCE_VMA:
            // TLB flush - NOP for this emulator (no virtual memory)
            break;
        case INST_WFI:
            // Wait for interrupt - NOP for this emulator
            break;
        case INST_ECALL:
            // Save current PC to MEPC/SEPC based on privilege
            if (cpu->privilege == USER_MODE) {
                cpu->csrs[CSR_MEPC] = cpu->pc;
                cpu->csrs[CSR_MCAUSE] = 8; // Environment call from U-mode
            } else if (cpu->privilege == SUPERVISOR_MODE) {
                cpu->csrs[CSR_SEPC] = cpu->pc;
                cpu->csrs[CSR_SCAUSE] = 8; // Environment call from S-mode
            } else { // MACHINE_MODE
                cpu->csrs[CSR_MEPC] = cpu->pc;
                cpu->csrs[CSR_MCAUSE] = 11; // Environment call from M-mode
            }
            // All traps enter M-mode
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            return;
        case INST_EBREAK:
            cpu->csrs[CSR_MEPC] = cpu->pc;
            cpu->csrs[CSR_MCAUSE] = 3; // Breakpoint
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            return;
        case INST_MRET: {
            uint32_t mstatus = cpu->csrs[CSR_MSTATUS];
            cpu->pc = cpu->csrs[CSR_MEPC];
            // Restore previous privilege mode from MPP
            cpu->privilege = (mstatus >> 11) & 0x3;
            // Set MPIE to MIE, and MIE to 1
            mstatus = (mstatus & ~(1 << 7)) | (((mstatus >> 7) & 1) << 3); // MPIE to MIE
            mstatus |= (1 << 7); // Set MPIE to 1
            // Set MPP to User Mode (0)
            mstatus &= ~(0x3 << 11);
            cpu->csrs[CSR_MSTATUS] = mstatus;
            return;
        }
        case INST_SRET: {
            uint32_t sstatus = cpu->csrs[CSR_SSTATUS];
            cpu->pc = cpu->csrs[CSR_SEPC];
            // Restore previous privilege mode from SPP
            cpu->privilege = (sstatus >> 8) & 0x1;
            // Set SPIE to SIE, and SIE to 1
            sstatus = (sstatus & ~(1 << 5)) | (((sstatus >> 5) & 1) << 1); // SPIE to SIE
            sstatus |= (1 << 5); // Set SPIE to 1
            // Set SPP to User Mode (0)
            sstatus &= ~(0x1 << 8);
            cpu->csrs[CSR_SSTATUS] = sstatus;
            return;
        }
        case INST_URET:
            // URET is an illegal instruction in most implementations
            cpu->csrs[CSR_MEPC] = cpu->pc;
            cpu->csrs[CSR_MCAUSE] = 2; // Illegal instruction
            cpu->privilege = MACHINE_MODE;
            cpu->pc = cpu->csrs[CSR_MTVEC];
            return;
        case INST_CSRRW:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] = cpu->regs[decoded.rs1];
            break;
        case INST_CSRRS:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] |= cpu->regs[decoded.rs1];
            break;
        case INST_CSRRC:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] &= ~cpu->regs[decoded.rs1];
            break;
        case INST_CSRRWI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] = decoded.rs1;
            break;
        case INST_CSRRSI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] |= decoded.rs1;
            break;
        case INST_CSRRCI:
            if (decoded.rd != 0) {
                cpu->regs[decoded.rd] = cpu->csrs[decoded.imm];
            }
            cpu->csrs[decoded.imm] &= ~decoded.rs1;
            break;
        case INST_UNKNOWN:
        default:
            printf("Unknown instruction: 0x%08x\n", instruction);
            break;
    }
    
    cpu->pc += 4; // Move to next instruction
}
