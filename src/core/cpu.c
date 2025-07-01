#include "cpu.h"
#include "decode.h"
#include "memory.h"
#include <stdio.h>

void cpu_init(cpu_t* cpu) {
    for (int i = 0; i < NUM_REGISTERS; i++) {
        cpu->regs[i] = 0;
    }
    cpu->pc = 0;
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
                cpu->regs[decoded.rd] = (int32_t)cpu->regs[decoded.rs1] >> (cpu->regs[decoded.rs2] & 0x1F);
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
        case INST_UNKNOWN:
        default:
            printf("Unknown instruction: 0x%08x\n", instruction);
            break;
    }
    
    cpu->pc += 4; // Move to next instruction
}
