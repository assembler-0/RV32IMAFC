#include "decode_table.h"

// Decode tables - initialized at runtime
decode_entry_t op_table[128][8];
decode_entry_t op_imm_table[128][8];
decode_entry_t load_table[8];
decode_entry_t store_table[8];
decode_entry_t branch_table[8];
decode_entry_t system_table[4096];
decode_entry_t amo_table[32][8];
decode_entry_t op_fp_table[128][8];

void init_decode_tables(void) {
    // Initialize all to unknown
    for (int i = 0; i < 128; i++) {
        for (int j = 0; j < 8; j++) {
            op_table[i][j] = (decode_entry_t){INST_UNKNOWN, 0};
            op_imm_table[i][j] = (decode_entry_t){INST_UNKNOWN, 0};
            op_fp_table[i][j] = (decode_entry_t){INST_UNKNOWN, 0};
        }
    }
    
    for (int i = 0; i < 8; i++) {
        load_table[i] = (decode_entry_t){INST_UNKNOWN, 0};
        store_table[i] = (decode_entry_t){INST_UNKNOWN, 0};
        branch_table[i] = (decode_entry_t){INST_UNKNOWN, 0};
    }
    
    for (int i = 0; i < 4096; i++) {
        system_table[i] = (decode_entry_t){INST_UNKNOWN, 0};
    }
    
    for (int i = 0; i < 32; i++) {
        for (int j = 0; j < 8; j++) {
            amo_table[i][j] = (decode_entry_t){INST_UNKNOWN, 0};
        }
    }
    
    // 🚀 POPULATE OP TABLE (R-type)
    op_table[0x00][0] = (decode_entry_t){INST_ADD, 0};
    op_table[0x20][0] = (decode_entry_t){INST_SUB, 0};
    op_table[0x00][1] = (decode_entry_t){INST_SLL, 0};
    op_table[0x00][2] = (decode_entry_t){INST_SLT, 0};
    op_table[0x00][3] = (decode_entry_t){INST_SLTU, 0};
    op_table[0x00][4] = (decode_entry_t){INST_XOR, 0};
    op_table[0x00][5] = (decode_entry_t){INST_SRL, 0};
    op_table[0x20][5] = (decode_entry_t){INST_SRA, 0};
    op_table[0x00][6] = (decode_entry_t){INST_OR, 0};
    op_table[0x00][7] = (decode_entry_t){INST_AND, 0};
    
    // M extension
    op_table[0x01][0] = (decode_entry_t){INST_MUL, 0};
    op_table[0x01][1] = (decode_entry_t){INST_MULH, 0};
    op_table[0x01][2] = (decode_entry_t){INST_MULHSU, 0};
    op_table[0x01][3] = (decode_entry_t){INST_MULHU, 0};
    op_table[0x01][4] = (decode_entry_t){INST_DIV, 0};
    op_table[0x01][5] = (decode_entry_t){INST_DIVU, 0};
    op_table[0x01][6] = (decode_entry_t){INST_REM, 0};
    op_table[0x01][7] = (decode_entry_t){INST_REMU, 0};
#if XLEN == 64
    op_table[0x01][0] = (decode_entry_t){INST_MULW, 0}; // funct3 = 0, funct7 = 0x01
    op_table[0x01][4] = (decode_entry_t){INST_DIVW, 0}; // funct3 = 4, funct7 = 0x01
    op_table[0x01][5] = (decode_entry_t){INST_DIVUW, 0}; // funct3 = 5, funct7 = 0x01
    op_table[0x01][6] = (decode_entry_t){INST_REMW, 0}; // funct3 = 6, funct7 = 0x01
    op_table[0x01][7] = (decode_entry_t){INST_REMUW, 0}; // funct3 = 7, funct7 = 0x01
#endif
    
    // 🚀 POPULATE OP-IMM TABLE (I-type)
    op_imm_table[0x00][0] = (decode_entry_t){INST_ADDI, 0};
    op_imm_table[0x00][2] = (decode_entry_t){INST_SLTI, 0};
    op_imm_table[0x00][3] = (decode_entry_t){INST_SLTIU, 0};
    op_imm_table[0x00][4] = (decode_entry_t){INST_XORI, 0};
    op_imm_table[0x00][6] = (decode_entry_t){INST_ORI, 0};
    op_imm_table[0x00][7] = (decode_entry_t){INST_ANDI, 0};
    op_imm_table[0x00][1] = (decode_entry_t){INST_SLLI, 1}; // Special shamt
    op_imm_table[0x00][5] = (decode_entry_t){INST_SRLI, 1}; // Special shamt
    op_imm_table[0x20][5] = (decode_entry_t){INST_SRAI, 1}; // Special shamt
    
    // 🚀 POPULATE LOAD TABLE
    load_table[0] = (decode_entry_t){INST_LB, 0};
    load_table[1] = (decode_entry_t){INST_LH, 0};
    load_table[2] = (decode_entry_t){INST_LW, 0};
    load_table[4] = (decode_entry_t){INST_LBU, 0};
    load_table[5] = (decode_entry_t){INST_LHU, 0};
#if XLEN == 64
    load_table[3] = (decode_entry_t){INST_LD, 0};
    load_table[6] = (decode_entry_t){INST_LWU, 0};
#endif
    
    // 🚀 POPULATE STORE TABLE
    store_table[0] = (decode_entry_t){INST_SB, 0};
    store_table[1] = (decode_entry_t){INST_SH, 0};
    store_table[2] = (decode_entry_t){INST_SW, 0};
#if XLEN == 64
    store_table[3] = (decode_entry_t){INST_SD, 0};
#endif
    
    // 🚀 POPULATE BRANCH TABLE
    branch_table[0] = (decode_entry_t){INST_BEQ, 0};
    branch_table[1] = (decode_entry_t){INST_BNE, 0};
    branch_table[4] = (decode_entry_t){INST_BLT, 0};
    branch_table[5] = (decode_entry_t){INST_BGE, 0};
    branch_table[6] = (decode_entry_t){INST_BLTU, 0};
    branch_table[7] = (decode_entry_t){INST_BGEU, 0};
    
    // 🚀 POPULATE SYSTEM TABLE
    system_table[0x000] = (decode_entry_t){INST_ECALL, 0};
    system_table[0x001] = (decode_entry_t){INST_EBREAK, 0};
    system_table[0x302] = (decode_entry_t){INST_MRET, 0};
    system_table[0x102] = (decode_entry_t){INST_SRET, 0};
    system_table[0x002] = (decode_entry_t){INST_URET, 0};
    system_table[0x105] = (decode_entry_t){INST_WFI, 0};
    system_table[0x000 | (0x00 << 5)] = (decode_entry_t){INST_FENCE, 0};
    system_table[0x001 | (0x00 << 5)] = (decode_entry_t){INST_FENCE_I, 0};
    system_table[0x000 | (0x09 << 5)] = (decode_entry_t){INST_SFENCE_VMA, 0};
    system_table[0x000 | (0x00 << 5)] = (decode_entry_t){INST_CSRRW, 0};
    system_table[0x000 | (0x01 << 5)] = (decode_entry_t){INST_CSRRS, 0};
    system_table[0x000 | (0x02 << 5)] = (decode_entry_t){INST_CSRRC, 0};
    system_table[0x000 | (0x05 << 5)] = (decode_entry_t){INST_CSRRWI, 0};
    system_table[0x000 | (0x06 << 5)] = (decode_entry_t){INST_CSRRSI, 0};
    system_table[0x000 | (0x07 << 5)] = (decode_entry_t){INST_CSRRCI, 0};
    
    // 🚀 POPULATE AMO TABLE
    amo_table[0x02][2] = (decode_entry_t){INST_LR_W, 0};
    amo_table[0x03][2] = (decode_entry_t){INST_SC_W, 0};
    amo_table[0x01][2] = (decode_entry_t){INST_AMOSWAP_W, 0};
    amo_table[0x00][2] = (decode_entry_t){INST_AMOADD_W, 0};
    amo_table[0x04][2] = (decode_entry_t){INST_AMOXOR_W, 0};
    amo_table[0x0C][2] = (decode_entry_t){INST_AMOAND_W, 0};
    amo_table[0x08][2] = (decode_entry_t){INST_AMOOR_W, 0};
    amo_table[0x10][2] = (decode_entry_t){INST_AMOMIN_W, 0};
    amo_table[0x14][2] = (decode_entry_t){INST_AMOMAX_W, 0};
    amo_table[0x18][2] = (decode_entry_t){INST_AMOMINU_W, 0};
    amo_table[0x1C][2] = (decode_entry_t){INST_AMOMAXU_W, 0};
#if XLEN == 64
    amo_table[0x02][3] = (decode_entry_t){INST_LR_D, 0};
    amo_table[0x03][3] = (decode_entry_t){INST_SC_D, 0};
    amo_table[0x01][3] = (decode_entry_t){INST_AMOSWAP_D, 0};
    amo_table[0x00][3] = (decode_entry_t){INST_AMOADD_D, 0};
    amo_table[0x04][3] = (decode_entry_t){INST_AMOXOR_D, 0};
    amo_table[0x0C][3] = (decode_entry_t){INST_AMOAND_D, 0};
    amo_table[0x08][3] = (decode_entry_t){INST_AMOOR_D, 0};
    amo_table[0x10][3] = (decode_entry_t){INST_AMOMIN_D, 0};
    amo_table[0x14][3] = (decode_entry_t){INST_AMOMAX_D, 0};
    amo_table[0x18][3] = (decode_entry_t){INST_AMOMINU_D, 0};
    amo_table[0x1C][3] = (decode_entry_t){INST_AMOMAXU_D, 0};
#endif
    
    // 🚀 POPULATE FLOATING-POINT TABLE
    op_fp_table[0x00][0] = (decode_entry_t){INST_FADD_S, 0};
    op_fp_table[0x04][0] = (decode_entry_t){INST_FSUB_S, 0};
    op_fp_table[0x08][0] = (decode_entry_t){INST_FMUL_S, 0};
    op_fp_table[0x0C][0] = (decode_entry_t){INST_FDIV_S, 0};
    op_fp_table[0x2C][0] = (decode_entry_t){INST_FSQRT_S, 0}; // rs2 == 0x00
    op_fp_table[0x10][0] = (decode_entry_t){INST_FSGNJ_S, 0};
    op_fp_table[0x10][1] = (decode_entry_t){INST_FSGNJN_S, 0};
    op_fp_table[0x10][2] = (decode_entry_t){INST_FSGNJX_S, 0};
    op_fp_table[0x14][0] = (decode_entry_t){INST_FMIN_S, 0};
    op_fp_table[0x14][1] = (decode_entry_t){INST_FMAX_S, 0};
    op_fp_table[0x60][0] = (decode_entry_t){INST_FCVT_W_S, 0}; // rs2 == 0x00
    op_fp_table[0x60][1] = (decode_entry_t){INST_FCVT_WU_S, 0}; // rs2 == 0x01
    op_fp_table[0x70][0] = (decode_entry_t){INST_FMV_X_W, 0}; // rs2 == 0x00, funct3 == 0x0
    op_fp_table[0x50][2] = (decode_entry_t){INST_FEQ_S, 0};
    op_fp_table[0x50][1] = (decode_entry_t){INST_FLT_S, 0};
    op_fp_table[0x50][0] = (decode_entry_t){INST_FLE_S, 0};
    op_fp_table[0x70][1] = (decode_entry_t){INST_FCLASS_S, 0}; // rs2 == 0x00, funct3 == 0x1
    op_fp_table[0x68][0] = (decode_entry_t){INST_FCVT_S_W, 0}; // rs2 == 0x00
    op_fp_table[0x68][1] = (decode_entry_t){INST_FCVT_S_WU, 0}; // rs2 == 0x01
    op_fp_table[0x78][0] = (decode_entry_t){INST_FMV_W_X, 0}; // rs2 == 0x00, funct3 == 0x0

    // Double precision
    op_fp_table[0x01][0] = (decode_entry_t){INST_FADD_D, 0};
    op_fp_table[0x05][0] = (decode_entry_t){INST_FSUB_D, 0};
    op_fp_table[0x09][0] = (decode_entry_t){INST_FMUL_D, 0};
    op_fp_table[0x0D][0] = (decode_entry_t){INST_FDIV_D, 0};
    op_fp_table[0x2D][0] = (decode_entry_t){INST_FSQRT_D, 0}; // rs2 == 0x00
    op_fp_table[0x11][0] = (decode_entry_t){INST_FSGNJ_D, 0};
    op_fp_table[0x11][1] = (decode_entry_t){INST_FSGNJN_D, 0};
    op_fp_table[0x11][2] = (decode_entry_t){INST_FSGNJX_D, 0};
    op_fp_table[0x15][0] = (decode_entry_t){INST_FMIN_D, 0};
    op_fp_table[0x15][1] = (decode_entry_t){INST_FMAX_D, 0};
    op_fp_table[0x20][1] = (decode_entry_t){INST_FCVT_S_D, 0}; // rs2 == 0x01
    op_fp_table[0x21][0] = (decode_entry_t){INST_FCVT_D_S, 0}; // rs2 == 0x00
    op_fp_table[0x51][2] = (decode_entry_t){INST_FEQ_D, 0};
    op_fp_table[0x51][1] = (decode_entry_t){INST_FLT_D, 0};
    op_fp_table[0x51][0] = (decode_entry_t){INST_FLE_D, 0};
    op_fp_table[0x71][1] = (decode_entry_t){INST_FCLASS_D, 0}; // rs2 == 0x00, funct3 == 0x1
    op_fp_table[0x61][0] = (decode_entry_t){INST_FCVT_W_D, 0}; // rs2 == 0x00
    op_fp_table[0x61][1] = (decode_entry_t){INST_FCVT_WU_D, 0}; // rs2 == 0x01
    op_fp_table[0x69][0] = (decode_entry_t){INST_FCVT_D_W, 0}; // rs2 == 0x00
    op_fp_table[0x69][1] = (decode_entry_t){INST_FCVT_D_WU, 0}; // rs2 == 0x01
#if XLEN == 64
    op_fp_table[0x61][2] = (decode_entry_t){INST_FCVT_L_D, 0}; // rs2 == 0x02
    op_fp_table[0x61][3] = (decode_entry_t){INST_FCVT_LU_D, 0}; // rs2 == 0x03
    op_fp_table[0x71][0] = (decode_entry_t){INST_FMV_X_D, 0}; // rs2 == 0x00, funct3 == 0x0
    op_fp_table[0x69][2] = (decode_entry_t){INST_FCVT_D_L, 0}; // rs2 == 0x02
    op_fp_table[0x69][3] = (decode_entry_t){INST_FCVT_D_LU, 0}; // rs2 == 0x03
    op_fp_table[0x79][0] = (decode_entry_t){INST_FMV_D_X, 0}; // rs2 == 0x00, funct3 == 0x0
#endif
}