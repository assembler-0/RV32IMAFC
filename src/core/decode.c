#include "decode.h"
#include "cpu.h"
#include "decode_table.h"
#include <stdio.h>

// Flag to track if decode tables are initialized
static int decode_tables_initialized = 0;

void decode_instruction(uint32_t instruction, instruction_t* decoded_inst) {
    // Initialize decode tables on first use
    if (!decode_tables_initialized) {
        init_decode_tables();
        decode_tables_initialized = 1;
    }
    
    // Extract common fields
    decoded_inst->opcode = instruction & 0x7f;
    decoded_inst->rd = (instruction >> 7) & 0x1f;
    decoded_inst->rs1 = (instruction >> 15) & 0x1f;
    decoded_inst->rs2 = (instruction >> 20) & 0x1f;
    decoded_inst->imm = (int32_t)(instruction >> 20); // Default I-type
    
    // 🚀 BLAZING FAST TABLE LOOKUP DISPATCH!
    uint32_t opcode = decoded_inst->opcode;
    decode_entry_t entry;
    
    // Fast opcode dispatch with minimal branching
    switch (opcode) {
        case OPCODE_LUI:
            decoded_inst->imm = instruction & 0xFFFFF000;
            decoded_inst->inst_type = INST_LUI;
            return;
            
        case OPCODE_AUIPC:
            decoded_inst->imm = instruction & 0xFFFFF000;
            decoded_inst->inst_type = INST_AUIPC;
            return;
            
        case OPCODE_JAL: {
            // J-type immediate - optimized bit extraction
            uint32_t imm20 = (instruction >> 31) & 1;
            uint32_t imm = ((instruction >> 12) & 0xFF000) |    // imm[19:12]
                          ((instruction >> 20) & 0x7FE) |      // imm[10:1]
                          ((instruction >> 9) & 0x800) |       // imm[11]
                          (imm20 ? 0xFFF00000 : 0);            // sign extend
            decoded_inst->imm = (int32_t)imm;
            decoded_inst->inst_type = INST_JAL;
            return;
        }
        
        case OPCODE_JALR:
            decoded_inst->inst_type = INST_JALR;
            return;
            
        case OPCODE_BRANCH: {
            // B-type immediate - optimized extraction
            uint32_t imm12 = (instruction >> 31) & 1;
            uint32_t imm = ((instruction >> 20) & 0x7E0) |      // imm[10:5]
                          ((instruction >> 7) & 0x1E) |        // imm[4:1]
                          ((instruction << 4) & 0x800) |       // imm[11]
                          (imm12 ? 0xFFFFF000 : 0);            // sign extend
            decoded_inst->imm = (int32_t)imm;
            
            uint32_t funct3 = (instruction >> 12) & 0x7;
            entry = branch_table[funct3];
            decoded_inst->inst_type = entry.inst_type;
            return;
        }
        
        case OPCODE_STORE: {
            // S-type immediate - optimized extraction
            uint32_t imm = ((instruction >> 20) & 0xFE0) |      // imm[11:5]
                          ((instruction >> 7) & 0x1F);          // imm[4:0]
            decoded_inst->imm = (int32_t)((imm & 0x800) ? (imm | 0xFFFFF000) : imm);
            
            uint32_t funct3 = (instruction >> 12) & 0x7;
            entry = store_table[funct3];
            decoded_inst->inst_type = entry.inst_type;
            return;
        }
        
        case OPCODE_LOAD: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            entry = load_table[funct3];
            decoded_inst->inst_type = entry.inst_type;
            return;
        }
        
        case OPCODE_OP_IMM: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t imm_field = (instruction >> 20);
            
            uint32_t effective_funct7 = 0x00;
            if (funct3 == 0x1 || funct3 == 0x5) { // SLLI, SRLI, SRAI
                effective_funct7 = (instruction >> 25) & 0x7f;
            }
            
            entry = op_imm_table[effective_funct7][funct3];
            
            if (entry.has_imm_override) {
                // Special shamt handling for shifts
#if XLEN == 64
                decoded_inst->imm = imm_field & 0x3f;
#else
                decoded_inst->imm = imm_field & 0x1f;
#endif
            } else {
                int32_t imm = (instruction >> 20);
                if (imm & 0x800) { // Check if the 11th bit is set (sign bit)
                    imm |= 0xFFFFF000; // Sign extend
                }
                decoded_inst->imm = imm;
            }
            decoded_inst->inst_type = entry.inst_type;
            return;
        }
        
        case OPCODE_OP: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t funct7 = (instruction >> 25) & 0x7f;
            entry = op_table[funct7][funct3];
            decoded_inst->inst_type = entry.inst_type;
            return;
        }
        
        case OPCODE_SYSTEM: {
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
            return;
        }
        
        case OPCODE_AMO: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t funct5 = (instruction >> 27) & 0x1F;
            entry = amo_table[funct5][funct3];
            decoded_inst->inst_type = entry.inst_type;
            return;
        }

        case OPCODE_MADD: {
            uint32_t funct2 = (instruction >> 25) & 0x3;
            if (funct2 == 0x0) {
                decoded_inst->inst_type = INST_FMADD_S;
            } else if (funct2 == 0x1) {
                decoded_inst->inst_type = INST_FMADD_D;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_MSUB: {
            uint32_t funct2 = (instruction >> 25) & 0x3;
            if (funct2 == 0x0) {
                decoded_inst->inst_type = INST_FMSUB_S;
            } else if (funct2 == 0x1) {
                decoded_inst->inst_type = INST_FMSUB_D;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_NMSUB: {
            uint32_t funct2 = (instruction >> 25) & 0x3;
            if (funct2 == 0x0) {
                decoded_inst->inst_type = INST_FNMSUB_S;
            } else if (funct2 == 0x1) {
                decoded_inst->inst_type = INST_FNMSUB_D;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_NMADD: {
            uint32_t funct2 = (instruction >> 25) & 0x3;
            if (funct2 == 0x0) {
                decoded_inst->inst_type = INST_FNMADD_S;
            } else if (funct2 == 0x1) {
                decoded_inst->inst_type = INST_FNMADD_D;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_OP_FP: {
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
            return;
        }
        
        case OPCODE_LOAD_FP: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            if (funct3 == 0x2) {
                decoded_inst->inst_type = INST_FLW;
            } else if (funct3 == 0x3) {
                decoded_inst->inst_type = INST_FLD;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_STORE_FP: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            if (funct3 == 0x2) {
                decoded_inst->inst_type = INST_FSW;
            } else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
#if XLEN == 64
        case OPCODE_OP_IMM_32: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            if (funct3 == 0x0) {
                decoded_inst->inst_type = INST_ADDIW;
            }
            else if (funct3 == 0x1) {
                decoded_inst->imm = (instruction >> 20) & 0x1f; // 5-bit shamt for W operations
                decoded_inst->inst_type = INST_SLLIW;
            }
            else if (funct3 == 0x5) {
                uint32_t funct7 = (instruction >> 25) & 0x7f;
                decoded_inst->imm = (instruction >> 20) & 0x1f;
                if (funct7 == 0x00) {
                    decoded_inst->inst_type = INST_SRLIW;
                }
                else if (funct7 == 0x20) {
                    decoded_inst->inst_type = INST_SRAIW;
                }
                else {
                    decoded_inst->inst_type = INST_UNKNOWN;
                }
            }
            else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
        
        case OPCODE_OP_32: {
            uint32_t funct3 = (instruction >> 12) & 0x7;
            uint32_t funct7 = (instruction >> 25) & 0x7f;
            if (funct7 == 0x00 && funct3 == 0x0) {
                decoded_inst->inst_type = INST_ADDW;
            }
            else if (funct7 == 0x20 && funct3 == 0x0) {
                decoded_inst->inst_type = INST_SUBW;
            }
            else if (funct7 == 0x00 && funct3 == 0x1) {
                decoded_inst->inst_type = INST_SLLW;
            }
            else if (funct7 == 0x00 && funct3 == 0x5) {
                decoded_inst->inst_type = INST_SRLW;
            }
            else if (funct7 == 0x20 && funct3 == 0x5) {
                decoded_inst->inst_type = INST_SRAW;
            }
            else {
                decoded_inst->inst_type = INST_UNKNOWN;
            }
            return;
        }
#endif
        
        default:
            decoded_inst->inst_type = INST_UNKNOWN;
            return;
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
#if XLEN == 64
                case 0x3: { // C.LD
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst << 1) & 0xC0);
                    return (imm << 20) | (rs1 << 15) | (3 << 12) | (rd << 7) | 0x03; // ld rd, imm(rs1)
                }
#else
                case 0x3: { // C.FLW
                    uint32_t rd = ((c_inst >> 2) & 0x7) + 8; // f8-f15
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8; // x8-x15
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return (imm << 20) | (rs1 << 15) | (2 << 12) | (rd << 7) | 0x07; // flw frd, imm(rs1)
                }
#endif
                case 0x5: { // C.FSD
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst << 1) & 0xC0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsd rs2, imm(rs1)
                }
                case 0x6: { // C.SW
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x23; // sw rs2, imm(rs1)
                }
#if XLEN == 64
                case 0x7: { // C.SD
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8;
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8;
                    uint32_t imm = ((c_inst >> 7) & 0x38) | ((c_inst << 1) & 0xC0);
                    return (((imm >> 5) & 0x7F) << 25) | (rs2 << 20) | (rs1 << 15) | (3 << 12) | ((imm & 0x1F) << 7) | 0x23; // sd rs2, imm(rs1)
                }
#else
                case 0x7: { // C.FSW
                    uint32_t rs1 = ((c_inst >> 7) & 0x7) + 8; // x8-x15
                    uint32_t rs2 = ((c_inst >> 2) & 0x7) + 8; // f8-f15
                    uint32_t imm = ((c_inst >> 3) & 0x4) | ((c_inst >> 6) & 0x40) | ((c_inst >> 6) & 0x38);
                    return ((imm >> 5) << 25) | (rs2 << 20) | (rs1 << 15) | (2 << 12) | ((imm & 0x1F) << 7) | 0x27; // fsw frs2, imm(rs1)
                }
#endif
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
#if XLEN == 64
                    uint32_t shamt = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) << 5);
#else
                    uint32_t shamt = ((c_inst >> 2) & 0x1F) | (((c_inst >> 12) & 1) << 5);
#endif
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