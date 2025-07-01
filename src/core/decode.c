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


    else if (decoded_inst->opcode == OPCODE_MADD) {
        uint32_t funct2 = (instruction >> 25) & 0x3;
        if (funct2 == 0x0) {
            decoded_inst->inst_type = INST_FMADD_S;
        } else if (funct2 == 0x1) {
            decoded_inst->inst_type = INST_FMADD_D;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_MSUB) {
        uint32_t funct2 = (instruction >> 25) & 0x3;
        if (funct2 == 0x0) {
            decoded_inst->inst_type = INST_FMSUB_S;
        } else if (funct2 == 0x1) {
            decoded_inst->inst_type = INST_FMSUB_D;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_NMSUB) {
        uint32_t funct2 = (instruction >> 25) & 0x3;
        if (funct2 == 0x0) {
            decoded_inst->inst_type = INST_FNMSUB_S;
        } else if (funct2 == 0x1) {
            decoded_inst->inst_type = INST_FNMSUB_D;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_NMADD) {
        uint32_t funct2 = (instruction >> 25) & 0x3;
        if (funct2 == 0x0) {
            decoded_inst->inst_type = INST_FNMADD_S;
        } else if (funct2 == 0x1) {
            decoded_inst->inst_type = INST_FNMADD_D;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_OP_FP) {
        uint32_t funct7 = (instruction >> 25) & 0x7F;
        uint32_t rs2 = (instruction >> 20) & 0x1F;
        uint32_t funct3 = (instruction >> 12) & 0x7;
        
        if (funct7 == 0x00) {
            decoded_inst->inst_type = INST_FADD_S;
        } else if (funct7 == 0x04) {
            decoded_inst->inst_type = INST_FSUB_S;
        } else if (funct7 == 0x08) {
            decoded_inst->inst_type = INST_FMUL_S;
        } else if (funct7 == 0x0C) {
            decoded_inst->inst_type = INST_FDIV_S;
        } else if (funct7 == 0x2C && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FSQRT_S;
        } else if (funct7 == 0x10 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FSGNJ_S;
        } else if (funct7 == 0x10 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FSGNJN_S;
        } else if (funct7 == 0x10 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_FSGNJX_S;
        } else if (funct7 == 0x14 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FMIN_S;
        } else if (funct7 == 0x14 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FMAX_S;
        } else if (funct7 == 0x60 && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FCVT_W_S;
        } else if (funct7 == 0x60 && rs2 == 0x01) {
            decoded_inst->inst_type = INST_FCVT_WU_S;
        } else if (funct7 == 0x70 && rs2 == 0x00 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FMV_X_W;
        } else if (funct7 == 0x50 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_FEQ_S;
        } else if (funct7 == 0x50 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FLT_S;
        } else if (funct7 == 0x50 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FLE_S;
        } else if (funct7 == 0x70 && rs2 == 0x00 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FCLASS_S;
        } else if (funct7 == 0x68 && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FCVT_S_W;
        } else if (funct7 == 0x68 && rs2 == 0x01) {
            decoded_inst->inst_type = INST_FCVT_S_WU;
        } else if (funct7 == 0x78 && rs2 == 0x00 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FMV_W_X;
        }
        // Double precision
        else if (funct7 == 0x01) {
            decoded_inst->inst_type = INST_FADD_D;
        } else if (funct7 == 0x05) {
            decoded_inst->inst_type = INST_FSUB_D;
        } else if (funct7 == 0x09) {
            decoded_inst->inst_type = INST_FMUL_D;
        } else if (funct7 == 0x0D) {
            decoded_inst->inst_type = INST_FDIV_D;
        } else if (funct7 == 0x2D && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FSQRT_D;
        } else if (funct7 == 0x11 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FSGNJ_D;
        } else if (funct7 == 0x11 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FSGNJN_D;
        } else if (funct7 == 0x11 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_FSGNJX_D;
        } else if (funct7 == 0x15 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FMIN_D;
        } else if (funct7 == 0x15 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FMAX_D;
        } else if (funct7 == 0x20 && rs2 == 0x01) {
            decoded_inst->inst_type = INST_FCVT_S_D;
        } else if (funct7 == 0x21 && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FCVT_D_S;
        } else if (funct7 == 0x51 && funct3 == 0x2) {
            decoded_inst->inst_type = INST_FEQ_D;
        } else if (funct7 == 0x51 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FLT_D;
        } else if (funct7 == 0x51 && funct3 == 0x0) {
            decoded_inst->inst_type = INST_FLE_D;
        } else if (funct7 == 0x71 && rs2 == 0x00 && funct3 == 0x1) {
            decoded_inst->inst_type = INST_FCLASS_D;
        } else if (funct7 == 0x61 && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FCVT_W_D;
        } else if (funct7 == 0x61 && rs2 == 0x01) {
            decoded_inst->inst_type = INST_FCVT_WU_D;
        } else if (funct7 == 0x69 && rs2 == 0x00) {
            decoded_inst->inst_type = INST_FCVT_D_W;
        } else if (funct7 == 0x69 && rs2 == 0x01) {
            decoded_inst->inst_type = INST_FCVT_D_WU;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_LOAD_FP) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_FLW;
        } else if (funct3 == 0x3) {
            decoded_inst->inst_type = INST_FLD;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else if (decoded_inst->opcode == OPCODE_STORE_FP) {
        uint32_t funct3 = (instruction >> 12) & 0x7;
        if (funct3 == 0x2) {
            decoded_inst->inst_type = INST_FSW;
        } else {
            decoded_inst->inst_type = INST_UNKNOWN;
        }
    }
    else {
        decoded_inst->inst_type = INST_UNKNOWN;
    }
}

// Expand 16-bit compressed instruction to 32-bit
uint32_t expand_compressed(uint16_t c_inst) {
    uint32_t opcode = c_inst & 0x3;
    uint32_t funct3 = (c_inst >> 13) & 0x7;
    
    switch (opcode) {
        case 0x0: // C0
            switch (funct3) {
                case 0x0: { // C.ADDI4SPN
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x30) | ((c_inst >> 1) & 0x3C0) | ((c_inst >> 4) & 0x4) | ((c_inst >> 2) & 0x8);
                    if (imm == 0) return 0; // Reserved
                    return (imm << 20) | (2 << 15) | (0 << 12) | (rd << 7) | 0x13; // addi rd, x2, imm
                }
                case 0x1: { // C.FLD
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst << 1) & 0xC0);
                    return (imm << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x07; // fld rd, imm(rs1)
                }
                case 0x2: { // C.LW
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return (imm << 20) | (rs1 << 15) | (2 << 12) | (rd << 7) | 0x03; // lw rd, imm(rs1)
                }
                case 0x3: { // C.FLW
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8; // f8-f15
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8; // x8-x15
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return (imm << 20) | (rs1 << 15) | (2 << 12) | (rd << 7) | 0x07; // flw frd, imm(rs1)
                }
                case 0x6: { // C.SW
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23; // sw rs2, imm(rs1)
                }
                case 0x5: { // C.FSD
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst << 1) & 0xC0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsd rs2, imm(rs1)
                }
                case 0x7: { // C.FSW
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8; // x8-x15
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8; // f8-f15
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsw frs2, imm(rs1)
                }
            }
            break;
        case 0x1: // C1
            switch (funct3) {
                case 0x0: { // C.ADDI
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    int32_t imm = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) ? 0xFFFFFFE0 : 0);
                    return (imm << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x13; // addi rd, rd, imm
                }
                case 0x1: { // C.JAL
                    int32_t imm = ((c_inst >> 1) & 0x800) | ((c_inst << 2) & 0x400) | ((c_inst >> 1) & 0x300) | ((c_inst << 1) & 0x80) | ((c_inst >> 1) & 0x40) | ((c_inst << 3) & 0x20) | ((c_inst >> 7) & 0x10) | ((c_inst >> 2) & 0xE);
                    if (c_inst & 0x1000) imm |= 0xFFFFF000;
                    return (((imm >> 20) & 1) << 31) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 1) << 20) | (((imm >> 12) & 0xFF) << 12) | (1 << 7) | 0x6F; // jal x1, imm
                }
                case 0x2: { // C.LI
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    int32_t imm = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) ? 0xFFFFFFE0 : 0);
                    return (imm << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x13; // addi rd, x0, imm
                }
                case 0x3: { // C.ADDI16SP or C.LUI
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    if (rd == 2) { // C.ADDI16SP
                        int32_t imm = ((c_inst >> 3) & 0x200) | ((c_inst >> 2) & 0x10) | ((c_inst << 1) & 0x40) | ((c_inst << 4) & 0x180) | ((c_inst << 3) & 0x20);
                        if (c_inst & 0x1000) imm |= 0xFFFFFE00;
                        return (imm << 20) | (2 << 15) | (0 << 12) | (2 << 7) | 0x13; // addi x2, x2, imm
                    } else { // C.LUI
                        int32_t imm = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) ? 0xFFFFFFE0 : 0);
                        return (imm << 12) | (rd << 7) | 0x37; // lui rd, imm
                    }
                }
                case 0x4: { // C.SRLI, C.SRAI, C.ANDI, C.SUB, C.XOR, C.OR, C.AND
                    uint32_t rd = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t funct2 = (c_inst >> 10) & 0x3;
                    if (funct2 == 0x0) { // C.SRLI
                        uint32_t shamt = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) << 5);
                        return (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13; // srli rd, rd, shamt
                    } else if (funct2 == 0x1) { // C.SRAI
                        uint32_t shamt = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) << 5);
                        return (0x20 << 25) | (shamt << 20) | (rd << 15) | (5 << 12) | (rd << 7) | 0x13; // srai rd, rd, shamt
                    } else if (funct2 == 0x2) { // C.ANDI
                        int32_t imm = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) ? 0xFFFFFFE0 : 0);
                        return (imm << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x13; // andi rd, rd, imm
                    } else { // C.SUB, C.XOR, C.OR, C.AND
                        uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                        uint32_t funct6 = (c_inst >> 10) & 0x3F;
                        if (funct6 == 0x23) return (0x20 << 25) | (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // sub
                        else if (funct6 == 0x27) return (rs2 << 20) | (rd << 15) | (4 << 12) | (rd << 7) | 0x33; // xor
                        else if (funct6 == 0x2B) return (rs2 << 20) | (rd << 15) | (6 << 12) | (rd << 7) | 0x33; // or
                        else if (funct6 == 0x2F) return (rs2 << 20) | (rd << 15) | (7 << 12) | (rd << 7) | 0x33; // and
                    }
                }
                case 0x5: { // C.J
                    int32_t imm = ((c_inst >> 1) & 0x800) | ((c_inst << 2) & 0x400) | ((c_inst >> 1) & 0x300) | ((c_inst << 1) & 0x80) | ((c_inst >> 1) & 0x40) | ((c_inst << 3) & 0x20) | ((c_inst >> 7) & 0x10) | ((c_inst >> 2) & 0xE);
                    if (c_inst & 0x1000) imm |= 0xFFFFF000;
                    return (((imm >> 20) & 1) << 31) | (((imm >> 1) & 0x3FF) << 21) | (((imm >> 11) & 1) << 20) | (((imm >> 12) & 0xFF) << 12) | (0 << 7) | 0x6F; // jal x0, imm
                }
                case 0x6: { // C.BEQZ
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    int32_t imm = ((c_inst >> 4) & 0x100) | ((c_inst >> 7) & 0x18) | ((c_inst << 1) & 0xC0) | ((c_inst >> 2) & 0x6) | ((c_inst << 3) & 0x20);
                    if (c_inst & 0x1000) imm |= 0xFFFFFE00;
                    return (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3F) << 25) | (0 << 20) | (rs1 << 15) | (0 << 12) | (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 1) << 7) | 0x63; // beq rs1, x0, imm
                }
                case 0x7: { // C.BNEZ
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    int32_t imm = ((c_inst >> 4) & 0x100) | ((c_inst >> 7) & 0x18) | ((c_inst << 1) & 0xC0) | ((c_inst >> 2) & 0x6) | ((c_inst << 3) & 0x20);
                    if (c_inst & 0x1000) imm |= 0xFFFFFE00;
                    return (((imm >> 12) & 1) << 31) | (((imm >> 5) & 0x3F) << 25) | (0 << 20) | (rs1 << 15) | (1 << 12) | (((imm >> 1) & 0xF) << 8) | (((imm >> 11) & 1) << 7) | 0x63; // bne rs1, x0, imm
                }
            }
            break;
        case 0x2: // C2
            switch (funct3) {
                case 0x0: { // C.SLLI
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    uint32_t shamt = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) << 5);
                    if (rd == 0) return 0; // Reserved
                    return (shamt << 20) | (rd << 15) | (1 << 12) | (rd << 7) | 0x13; // slli rd, rd, shamt
                }
                case 0x1: { // C.FLDSP
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    uint32_t imm = ((c_inst >> 2) & 0x1C) | ((c_inst >> 7) & 0x20) | ((c_inst << 3) & 0x1C0);
                    return (imm << 20) | (2 << 15) | (3 << 12) | (rd << 7) | 0x07; // fld rd, imm(x2)
                }
                case 0x2: { // C.LWSP
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    uint32_t imm = ((c_inst >> 2) & 0x1C) | ((c_inst >> 7) & 0x20) | ((c_inst >> 2) & 0xC0);
                    if (rd == 0) return 0; // Reserved
                    return (imm << 20) | (2 << 15) | (2 << 12) | (rd << 7) | 0x03; // lw rd, imm(x2)
                }
                case 0x3: { // C.FLWSP
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    uint32_t imm = ((c_inst >> 2) & 0x1C) | ((c_inst >> 7) & 0x20) | ((c_inst >> 2) & 0xC0);
                    return (imm << 20) | (2 << 15) | (2 << 12) | (rd << 7) | 0x07; // flw rd, imm(x2)
                }
                case 0x4: { // C.JR, C.MV, C.EBREAK, C.JALR, C.ADD
                    uint32_t rd = (c_inst >> 7) & 0x1F;
                    uint32_t rs2 = (c_inst >> 2) & 0x1F;
                    if ((c_inst & 0x1000) == 0) {
                        if (rs2 == 0) return (rd << 15) | (0 << 12) | (0 << 7) | 0x67; // jalr x0, 0(rd)
                        else return (rs2 << 20) | (0 << 15) | (0 << 12) | (rd << 7) | 0x33; // add rd, x0, rs2
                    } else {
                        if (rd == 0 && rs2 == 0) return 0x00100073; // ebreak
                        else if (rs2 == 0) return (rd << 15) | (0 << 12) | (1 << 7) | 0x67; // jalr x1, 0(rd)
                        else return (rs2 << 20) | (rd << 15) | (0 << 12) | (rd << 7) | 0x33; // add rd, rd, rs2
                    }
                }
                case 0x5: { // C.FSDSP
                    uint32_t rs2 = (c_inst >> 2) & 0x1F;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst >> 1) & 0x1C0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsd rs2, imm(x2)
                }
                case 0x6: { // C.SWSP
                    uint32_t rs2 = (c_inst >> 2) & 0x1F;
                    uint32_t imm = ((c_inst >> 7) & 0x3C) | ((c_inst >> 1) & 0xC0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23; // sw rs2, imm(x2)
                }
                case 0x7: { // C.FSWSP
                    uint32_t rs2 = (c_inst >> 2) & 0x1F;
                    uint32_t imm = ((c_inst >> 7) & 0x3C) | ((c_inst >> 1) & 0xC0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (2 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsw rs2, imm(x2)
                }
            }
            break;
    }
    return 0; // Invalid compressed instruction
}
