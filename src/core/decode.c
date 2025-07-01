#include "decode.h"
#include "cpu.h"
#include <stdio.h>

void decode_instruction(uint32_t instruction, instruction_t* decoded_inst) {
    decoded_inst->opcode = instruction & 0x7f;
    decoded_inst->rd = (instruction >> 7) & 0x1f;
    decoded_inst->rs1 = (instruction >> 15) & 0x1f;
    decoded_inst->rs2 = (instruction >> 20) & 0x1f;
    // Default I-type immediate
    decoded_inst->imm = (int32_t)(instruction >> 20);
    
    if (decoded_inst->opcode == OPCODE_AUIPC) {
        decoded_inst->imm = instruction & 0xFFFFF000; // U-type
        decoded_inst->inst_type = INST_AUIPC;
    }
    else if (decoded_inst->opcode == OPCODE_LUI) {
        decoded_inst->imm = instruction & 0xFFFFF000;
        decoded_inst->inst_type = INST_LUI;
    }
    else if (decoded_inst->opcode == OPCODE_JAL) {
        // J-type immediate
        uint32_t imm20 = (instruction >> 31) & 1;
        uint32_t imm10_1 = (instruction >> 21) & 0x3FF;
        uint32_t imm11 = (instruction >> 20) & 1;
        uint32_t imm19_12 = (instruction >> 12) & 0xFF;
        decoded_inst->imm = (int32_t)((imm20 << 20) | (imm19_12 << 12) | (imm11 << 11) | (imm10_1 << 1));
        if (imm20) decoded_inst->imm |= 0xFFE00000; // Sign extend
        decoded_inst->inst_type = INST_JAL;
    }
    else if (decoded_inst->opcode == OPCODE_JALR) {
        decoded_inst->inst_type = INST_JALR;
    }
    else if (decoded_inst->opcode == OPCODE_BRANCH) {
        // B-type immediate
        uint32_t imm12 = (instruction >> 31) & 1;
        uint32_t imm10_5 = (instruction >> 25) & 0x3F;
        uint32_t imm4_1 = (instruction >> 8) & 0xF;
        uint32_t imm11 = (instruction >> 7) & 1;
        decoded_inst->imm = (int32_t)((imm12 << 12) | (imm11 << 11) | (imm10_5 << 5) | (imm4_1 << 1));
        if (imm12) decoded_inst->imm |= 0xFFFFE000; // Sign extend
        
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x0) {
            decoded_inst->inst_type = INST_BEQ;
        }
        else if (funct3 == 0x1) {
            decoded_inst->inst_type = INST_BNE;
        }
        else if (funct3 == 0x4) {
            decoded_inst->inst_type = INST_BLT;
        }
        else if (funct3 == 0x5) {
            decoded_inst->inst_type = INST_BGE;
        }
        else if (funct3 == 0x6) {
            decoded_inst->inst_type = INST_BLTU;
        }
        else if (funct3 == 0x7) {
            decoded_inst->inst_type = INST_BGEU;
        }
        else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_STORE) {
        // S-type immediate
        uint32_t imm11_5 = (instruction >> 25) & 0x7F;
        uint32_t imm4_0 = (instruction >> 7) & 0x1F;
        decoded_inst->imm = (int32_t)((imm11_5 << 5) | imm4_0);
        if (imm11_5 & 0x40) decoded_inst->imm |= 0xFFFFF000; // Sign extend
        
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x0) {
            decoded_inst->inst_type = INST_SB;
        }
        else if (funct3 == 0x1) {
            decoded_inst->inst_type = INST_SH;
        }
        else if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_SW;
        }
        else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_LOAD) { // Load instructions
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x0) {
            decoded_inst->inst_type = INST_LB;
        }
        else if (funct3 == 0x1) {
            decoded_inst->inst_type = INST_LH;
        }
        else if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_LW;
        }
        else if (funct3 == 0x4) {
            decoded_inst->inst_type = INST_LBU;
        }
        else if (funct3 == 0x5) {
            decoded_inst->inst_type = INST_LHU;
        }
        else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_OP_IMM) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x0) {
            decoded_inst->inst_type = INST_ADDI;
        }
        else if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_SLTI;
        }
        else if (funct3 == 0x3) {
            decoded_inst->inst_type = INST_SLTIU;
        }
        else if (funct3 == 0x4) {
            decoded_inst->inst_type = INST_XORI;
        }
        else if (funct3 == 0x6) {
            decoded_inst->inst_type = INST_ORI;
        }
        else if (funct3 == 0x7) {
            decoded_inst->inst_type = INST_ANDI;
        }
        else if (funct3 == 0x1) {
            decoded_inst->imm = (instruction >> 20) & 0x1f;
            decoded_inst->inst_type = INST_SLLI;
        }
        else if (funct3 == 0x5) {
            uint32_t funct7 = (instruction >> 25) & 0x7f;
            if (funct7 == 0x00) {
                decoded_inst->imm = (instruction >> 20) & 0x1f;
                decoded_inst->inst_type = INST_SRLI;
            }
            else if (funct7 == 0x20) {
                decoded_inst->imm = (instruction >> 20) & 0x1f;
                decoded_inst->inst_type = INST_SRAI;
            }
            else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
        }
        else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_OP) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        uint32_t funct7 = (instruction >> 25) & 0x7f;
        if (funct7 == 0x00 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_SLT;
        }
        else if (funct7 == 0x00 && funct3 == 0x3) {
            decoded_inst->inst_type = INST_SLTU;
        }
        else if (funct7 == 0x00 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_SLL;
        }
        else if (funct7 == 0x00 && funct3 == 0x5) {
            decoded_inst->inst_type = INST_SRL;
        }
        else if (funct7 == 0x20 && funct3 == 0x5) {
            decoded_inst->inst_type = INST_SRA;
        }
        else if (funct7 == 0x20 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_SUB;
        }
        else if (funct7 == 0x00 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_ADD;
        }
        else if (funct7 == 0x01 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_MUL;
        }
        else if (funct7 == 0x01 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_MULH;
        }
        else if (funct7 == 0x01 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_MULHSU;
        }
        else if (funct7 == 0x01 && funct3 == 0x3) {
            decoded_inst->inst_type = INST_MULHU;
        }
        else if (funct7 == 0x01 && funct3 == 0x4) {
            decoded_inst->inst_type = INST_DIV;
        }
        else if (funct7 == 0x01 && funct3 == 0x5) {
            decoded_inst->inst_type = INST_DIVU;
        }
        else if (funct7 == 0x01 && funct3 == 0x6) {
            decoded_inst->inst_type = INST_REM;
        }
        else if (funct7 == 0x01 && funct3 == 0x7) {
            decoded_inst->inst_type = INST_REMU;
        }
        else if (funct7 == 0x00 && funct3 == 0x4) {
            decoded_inst->inst_type = INST_XOR;
        }
        else if (funct7 == 0x00 && funct3 == 0x6) {
            decoded_inst->inst_type = INST_OR;
        }
        else if (funct7 == 0x00 && funct3 == 0x7) {
            decoded_inst->inst_type = INST_AND;
        }
        else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_SYSTEM) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        uint32_t funct12 = (instruction >> 20) & 0xFFF;
        decoded_inst->imm = funct12;

        if (funct3 == 0) { // FENCE, ECALL, EBREAK, MRET, WFI, SFENCE.VMA
            if (funct12 == 0x000) {
                decoded_inst->inst_type = INST_ECALL;
            } else if (funct12 == 0x001) {
                decoded_inst->inst_type = INST_EBREAK;
            } else if (funct12 == 0x302) {
                decoded_inst->inst_type = INST_MRET;
            } else if (funct12 == 0x102) {
                decoded_inst->inst_type = INST_SRET;
            } else if (funct12 == 0x002) {
                decoded_inst->inst_type = INST_URET;
            } else if (funct12 == 0x105) {
                decoded_inst->inst_type = INST_WFI;
            } else if ((funct12 >> 5) == 0x09) { // SFENCE.VMA (funct7=0x09)
                decoded_inst->inst_type = INST_SFENCE_VMA;
            } else {
                decoded_inst->inst_type = INST_FENCE;
            }
        } else if (funct3 == 0x1) {
            if (funct12 == 0x001) {
                decoded_inst->inst_type = INST_FENCE_I;
            } else {
                decoded_inst->inst_type = INST_CSRRW;
            }
        } else if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_CSRRS;
        } else if (funct3 == 0x3) {
            decoded_inst->inst_type = INST_CSRRC;
        } else if (funct3 == 0x5) {
            decoded_inst->inst_type = INST_CSRRWI;
        } else if (funct3 == 0x6) {
            decoded_inst->inst_type = INST_CSRRSI;
        } else if (funct3 == 0x7) {
            decoded_inst->inst_type = INST_CSRRCI;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_AMO) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        uint32_t funct5 = (instruction >> 27) & 0x1F;

        if (funct3 == 0x2) { // All AMO.W instructions have funct3 = 0x2
            if (funct5 == 0x02) {
                decoded_inst->inst_type = INST_LR_W;
            } else if (funct5 == 0x03) {
                decoded_inst->inst_type = INST_SC_W;
            } else if (funct5 == 0x01) {
                decoded_inst->inst_type = INST_AMOSWAP_W;
            } else if (funct5 == 0x00) {
                decoded_inst->inst_type = INST_AMOADD_W;
            } else if (funct5 == 0x04) {
                decoded_inst->inst_type = INST_AMOXOR_W;
            } else if (funct5 == 0x0C) {
                decoded_inst->inst_type = INST_AMOAND_W;
            } else if (funct5 == 0x08) {
                decoded_inst->inst_type = INST_AMOOR_W;
            } else if (funct5 == 0x10) {
                decoded_inst->inst_type = INST_AMOMIN_W;
            } else if (funct5 == 0x14) {
                decoded_inst->inst_type = INST_AMOMAX_W;
            } else if (funct5 == 0x18) {
                decoded_inst->inst_type = INST_AMOMINU_W;
            } else if (funct5 == 0x1C) {
                decoded_inst->inst_type = INST_AMOMAXU_W;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else {
        decoded_inst->inst_type = INST_UNKNOWN;
    }
}
