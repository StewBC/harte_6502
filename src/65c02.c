#include <stdint.h>
#include <stdlib.h>

#include "6502.h"

typedef enum {
//  0         1          2        3       4          5          6          7       8         9          A         B       C          D          E          F
    cBRK_impl, cORA_X_ind, cUND_02 , cUND_03, cUND_04   , cORA_zpg  , cASL_zpg  , cUND_07, cPHP_impl, cORA_imm  , cASL_A   , cUND_0B, cUND_0C   , cORA_abs  , cASL_abs  , cUND_0F, // 0
    cBPL_rel , cORA_ind_Y, cUND_12 , cUND_13, cUND_14   , cORA_zpg_X, cASL_zpg_X, cUND_17, cCLC_impl, cORA_abs_Y, cUND_1A  , cUND_1B, cUND_1C   , cORA_abs_X, cASL_abs_X, cUND_1F, // 1
    cJSR_abs , cAND_X_ind, cUND_22 , cUND_23, cBIT_zpg  , cAND_zpg  , cROL_zpg  , cUND_27, cPLP_impl, cAND_imm  , cROL_A   , cUND_2B, cBIT_abs  , cAND_abs  , cROL_abs  , cUND_2F, // 2
    cBMI_rel , cAND_ind_Y, cUND_32 , cUND_33, cUND_34   , cAND_zpg_X, cROL_zpg_X, cUND_37, cSEC_impl, cAND_abs_Y, cUND_3A  , cUND_3B, cUND_3C   , cAND_abs_X, cROL_abs_X, cUND_3F, // 3
    cRTI_impl, cEOR_X_ind, cUND_42 , cUND_43, cUND_44   , cEOR_zpg  , cLSR_zpg  , cUND_47, cPHA_impl, cEOR_imm  , cLSR_A   , cUND_4B, cJMP_abs  , cEOR_abs  , cLSR_abs  , cUND_4F, // 4
    cBVC_rel , cEOR_ind_Y, cUND_52 , cUND_53, cUND_54   , cEOR_zpg_X, cLSR_zpg_X, cUND_57, cCLI_impl, cEOR_abs_Y, cUND_5A  , cUND_5B, cUND_5C   , cEOR_abs_X, cLSR_abs_X, cUND_5F, // 5
    cRTS_impl, cADC_X_ind, cUND_62 , cUND_63, cUND_64   , cADC_zpg  , cROR_zpg  , cUND_67, cPLA_impl, cADC_imm  , cROR_A   , cUND_6B, cJMP_ind  , cADC_abs  , cROR_abs  , cUND_6F, // 6
    cBVS_rel , cADC_ind_Y, cUND_72 , cUND_73, cUND_74   , cADC_zpg_X, cROR_zpg_X, cUND_77, cSEI_impl, cADC_abs_Y, cUND_7A  , cUND_7B, cUND_7C   , cADC_abs_X, cROR_abs_X, cUND_7F, // 7
    cUND_80  , cSTA_X_ind, cUND_82 , cUND_83, cSTY_zpg  , cSTA_zpg  , cSTX_zpg  , cUND_87, cDEY_impl, cUND_3D   , cTXA_impl, cUND_8B, cSTY_abs  , cSTA_abs  , cSTX_abs  , cUND_8F, // 8
    cBCC_rel , cSTA_ind_Y, cUND_92 , cUND_93, cSTY_zpg_X, cSTA_zpg_X, cSTX_zpg_Y, cUND_97, cTYA_impl, cSTA_abs_Y, cTXS_impl, cUND_9B, cUND_9C   , cSTA_abs_X, cUND_45   , cUND_9F, // 9
    cLDY_imm , cLDA_X_ind, cLDX_imm, cUND_A3, cLDY_zpg  , cLDA_zpg  , cLDX_zpg  , cUND_A7, cTAY_impl, cLDA_imm  , cTAX_impl, cUND_AB, cLDY_abs  , cLDA_abs  , cLDX_abs  , cUND_AF, // A
    cBCS_rel , cLDA_ind_Y, cUND_B2 , cUND_B3, cLDY_zpg_X, cLDA_zpg_X, cLDX_zpg_Y, cUND_B7, cCLV_impl, cLDA_abs_Y, cTSX_impl, cUND_BB, cLDY_abs_X, cLDA_abs_X, cLDX_abs_Y, cUND_BF, // B
    cCPY_imm , cCMP_X_ind, cUND_C2 , cUND_C3, cCPY_zpg  , cCMP_zpg  , cDEC_zpg  , cUND_C7, cINY_impl, cCMP_imm  , cDEX_impl, cUND_CB, cCPY_abs  , cCMP_abs  , cDEC_abs  , cUND_CF, // C
    cBNE_rel , cCMP_ind_Y, cUND_D2 , cUND_D3, cUND_D4   , cCMP_zpg_X, cDEC_zpg_X, cUND_D7, cCLD_impl, cCMP_abs_Y, cUND_DA  , cUND_DB, cUND_DC   , cCMP_abs_X, cDEC_abs_X, cUND_DF, // D
    cCPX_imm , cSBC_X_ind, cUND_E2 , cUND_E3, cCPX_zpg  , cSBC_zpg  , cINC_zpg  , cUND_E7, cINX_impl, cSBC_imm  , cNOP_impl, cUND_EB, cCPX_abs  , cSBC_abs  , cINC_abs  , cUND_EF, // E
    cBEQ_rel , cSBC_ind_Y, cUND_F2 , cUND_F3, cUND_F4   , cSBC_zpg_X, cINC_zpg_X, cUND_F7, cSED_impl, cSBC_abs_Y, cUND_FA  , cUND_FB, cUND_FC   , cSBC_abs_X, cINC_abs_X, cUND_FF, // F
} OP_65c02;

int machine_run_opcode_65c02(MACHINE *m) { return 0; }