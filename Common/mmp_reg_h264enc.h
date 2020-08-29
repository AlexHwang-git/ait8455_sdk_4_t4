//==============================================================================
//
//  File        : mmp_reg_h264enc.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Hsin-Wei Yang
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_H264ENC_H_
#define _MMP_REG_H264ENC_H_
///@ait_only
#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/
// *******************************
//   Display structure (0x8000 F800)
// *******************************
typedef struct _AITS_H264ENC {
    //GLOBAL
    AIT_REG_B   H264ENC_FRAME_CTL;                 //0x0000
        /*-DEFINE-----------------------------------------------------*/
        #define H264_ENCODE_EN          0x01
        #define H264_SPEED_UP           0x02
        #define H264_DVS_EN             0x04
        #define H264_Y_LINE_DIS         0x08
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define H264_CUR_PP_MODE_EN     0x10
        #endif
        /*------------------------------------------------------------*/

    AIT_REG_B   _x0001;                             //0x0001
    AIT_REG_B   H264ENC_SW_RST;                   //0x0002
        /*-DEFINE-----------------------------------------------------*/
        #define H264_VLC_RESET                      0x01
        #define H264_TRANSFORM_RESET                0x02
        #define H264_INTRA_PRED_RESET               0x04
        #define H264_INTER_PRED_RESET               0x08
        #define H264_FRAME_RECONSTRUCT_RESET        0x10
        #define H264_MODE_DECISION_RESET            0x20
        #define H264_QP_FINETUNE_RESET              0x40
        #define H264_FLOW_CTRL_RESET                0x80
        /*------------------------------------------------------------*/

    AIT_REG_B   _x0003;                             //0x0003
    AIT_REG_W   H264ENC_INT_CPU_EN;                 //0x0004
    AIT_REG_W   H264ENC_INT_CPU_SR;                 //0x0006
    AIT_REG_W   H264ENC_INT_HOST_EN;                //0x0008
    AIT_REG_W   H264ENC_INT_HOST_SR;                //0x000A
        /*-DEFINE-----------------------------------------------------*/
        #define SPS_PPS_DONE                        0x0001
        #define SLICE_DONE                          0x0002
        #define SKIP_FRAME_DONE_CPU_INT_EN          0x0004
        #define EP_BYTE_CPU_INT_EN                  0x0008
        #define TRANS_MB_DONE_CPU_INT_EN            0x0020
        #define TRANS_B_DONE_CPU_INT_EN             0x0040
        #define TRANS_CB_DONE_CPU_INT_EN            0x0080
        #define FRAME_ENC_DONE_CPU_INT_EN           0x0100
        #define INTRA_MB_DONE_CPU_INT_EN            0x0200
        #define ENC_CUR_BUF_OVWR_INT                0x0800
        /*------------------------------------------------------------*/
        
    AIT_REG_W   _0x000C[2];                         //0x000C ~ 0x000F

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_W   H264ENC_SLICE_ARG;                  //0x0010
    AIT_REG_B   H264ENC_SLICE_MODE;                 //0x0012
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_SLICE_MODE_MASK               0x03
        #define H264E_SLICE_MODE_FRM                0x00
        #define H264E_SLICE_MODE_MB                 0x01
        #define H264E_SLICE_MODE_BYTE               0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   _0x0013;                            //0x0013
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   H264ENC_SLICE_SIZE_IN_ROW;          //0x0010
    AIT_REG_W   _0x0012;                            //0x0012
    #endif

    AIT_REG_D   H264ENC_QP_SUM;                     //0x0014

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_D   H264ENC_ROI_QP_SUM;                 //0x0018
    AIT_REG_D   H264ENC_SATD_SUM;                   //0x001C ~ 0x001f
    AIT_REG_W   H264ENC_SLICE_STUFF_BYTES;          //0x0020
    AIT_REG_W   _x0022;                             //0x0022
    AIT_REG_W   H264ENC_CODED_SLICE_NUM;            //0x0024
    AIT_REG_W   H264ENC_ROI_MB_NUM;                 //0x0026
    AIT_REG_D   _x0028[2];                          //0x0028 ~ 0x002f
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   H264ENC_INTRA_MB_COUNT;             //0x0018
    AIT_REG_W   _x001A;                             //0x001A
    AIT_REG_D   _x001C;                             //0x001C
    AIT_REG_D   H264ENC_SATD_SUM;                   //0x0020
    AIT_REG_D   _x0024[3];                          //0x0024 ~ 0x002f
    #endif

    //VLC
    AIT_REG_B   H264ENC_VLC_CTRL1;                  //0x0030
        /*-DEFINE-----------------------------------------------------*/
        #define ENCODE_CURR_FRAME_AS_SKIP          	0x01
        #define ENCODE_SPS_PPS_ONLY                 0x02
        /*------------------------------------------------------------*/

    AIT_REG_B   H264ENC_VLC_CTRL2;                  //0x0031
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_INSERT_EP3_EN                 0x01
        #define H264E_INSERT_SPS_PPS                0x02
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define H264E_COHDR_EN_MASK                 0xFC
        #define H264E_INSERT_SEI_0                  0x04
        #define H264E_INSERT_SEI_1                  0x08
        #define H264E_INSERT_PNALU                  0x10
        #define H264E_SEI_COHDR_EN                  0x20
        #define H264E_PNALU_COHDR_EN                0x40
        #define H264E_SLICE_COHDR_EN                0x80
        #endif
        /*------------------------------------------------------------*/
        
    AIT_REG_W   _x0032;                             //0x0032
    AIT_REG_D   H264ENC_BS_START_ADDR;              //0x0034
    AIT_REG_D   H264ENC_BS_LOWER_BOUND_ADDR;        //0x0038
    AIT_REG_D   H264ENC_BS_UPPER_BOUND_ADDR;        //0x003C
    AIT_REG_D   H264ENC_SLICE_LEN_BUF_ADDR;         //0x0040
    AIT_REG_D   H264ENC_INTRAMB_BITS_COUNT;         //0x0044
    AIT_REG_D   H264ENC_MV_BITS_COUNT;              //0x0048
    AIT_REG_D   H264ENC_HEADER_BITS_COUNT;          //0x004C
    AIT_REG_D   H264ENC_FRAME_LENGTH;               //0x0050
    AIT_REG_W   H264ENC_ROI_SIZE_LS2B;              //0x0054
    AIT_REG_B   H264ENC_ROI_SIZE_MSB;               //0x0056
    AIT_REG_B   H264ENC_VLC_SR;                     //0x0057
    #if (CHIP == VSN_V3)
    AIT_REG_D   H264ENC_BS_TURNAROUND_ADDR;         //0x0058
    AIT_REG_B   _x005C[8];
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_W   _x0058[6];                          //0x0058 ~0x0063
    #endif

    #if (CHIP == VSN_V3)
    AIT_REG_B   _x0064[0x30];                       //0x0064
    #endif
    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_LINE_Y_0;                   //0x0064
    AIT_REG_D   H264ENC_LINE_Y_1;                   //0x0068
    AIT_REG_D   H264ENC_LINE_Y_2;                   //0x006C
    AIT_REG_D   H264ENC_LINE_Y_3;                   //0x0070
    #if (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_LINE_U_0;                   //0x0074
    AIT_REG_D   H264ENC_LINE_U_1;                   //0x0078
    AIT_REG_D   H264ENC_LINE_U_2;                   //0x007C
    AIT_REG_D   H264ENC_LINE_U_3;                   //0x0080
    AIT_REG_D   H264ENC_LINE_V_0;                   //0x0084
    AIT_REG_D   H264ENC_LINE_V_1;                   //0x0088
    AIT_REG_D   H264ENC_LINE_V_2;                   //0x008C
    AIT_REG_D   H264ENC_LINE_V_3;                   //0x0090
    #endif
    #if (CHIP == P_V2)
    AIT_REG_D   H264ENC_LINE_UV_0;                  //0x0074
    AIT_REG_D   H264ENC_LINE_UV_1;                  //0x0078
    AIT_REG_D   H264ENC_LINE_UV_2;                  //0x007C
    AIT_REG_D   H264ENC_LINE_UV_3;                  //0x0080
    AIT_REG_D   H264ENC_LINE_UV_8;                  //0x0084
    AIT_REG_D   H264ENC_LINE_UV_9;                  //0x0088
    AIT_REG_D   H264ENC_LINE_UV_10;                 //0x008C
    AIT_REG_D   H264ENC_LINE_UV_11;                 //0x0090
    #endif
    #endif //(CHIP == P_V2) || (CHIP == VSN_V2)

    AIT_REG_D   H264ENC_CURR_Y_ADDR;                //0x0094
    AIT_REG_D   H264ENC_CURR_U_ADDR;                //0x0098
    AIT_REG_D   H264ENC_CURR_V_ADDR;                //0x009C

    AIT_REG_D   H264ENC_REFBD_Y_LOW;                //0x00A0
    AIT_REG_D   H264ENC_REFBD_Y_HIGH;               //0x00A4
    #if (CHIP == VSN_V3)
    AIT_REG_D   H264ENC_REFBD_UV_LOW;               //0x00A8
    AIT_REG_D   H264ENC_REFBD_UV_HIGH;              //0x00AC
    AIT_REG_B   _x00B0[0x8];                        //0x00B0
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_REFBD_U_LOW;                //0x00A8
    AIT_REG_D   H264ENC_REFBD_U_HIGH;               //0x00AC
    AIT_REG_D   H264ENC_REFBD_V_LOW;                //0x00B0
    AIT_REG_D   H264ENC_REFBD_V_HIGH;               //0x00B4
    #endif
    #if (CHIP == P_V2)
    AIT_REG_D   H264ENC_REFBD_UV_LOW;               //0x00A8
    AIT_REG_D   H264ENC_REFBD_UV_HIGH;              //0x00AC
    AIT_REG_D   H264ENC_REFBD_BW_UV_LOW;            //0x00B0
    AIT_REG_D   H264ENC_REFBD_BW_UV_HIGH;           //0x00B4
    #endif

    AIT_REG_D   H264ENC_REF_Y_ADDR;                 //0x00B8
    #if (CHIP == VSN_V3)
    AIT_REG_D   H264ENC_REF_UV_ADDR;                //0x00BC
    AIT_REG_B   _x00C0[0x4];                        //0x00C0
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_REF_U_ADDR;                 //0x00BC
    AIT_REG_D   H264ENC_REF_V_ADDR;                 //0x00C0
    #endif
    #if (CHIP == P_V2)
    AIT_REG_D   H264ENC_REF_UV_ADDR;                //0x00BC
    AIT_REG_D   H264ENC_REF_BW_UV_ADDR;             //0x00C0
    #endif

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   _x00C4[0x14];                       //0x00C4
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   H264ENC_LINE_MSB_Y_0;               //0x00C4
    AIT_REG_B   H264ENC_LINE_MSB_Y_1;               //0x00C5
    AIT_REG_W   H264ENC_LINE_MSB_UV;                //0x00C6
    AIT_REG_W   H264ENC_LINE_MSB_V;                 //0x00C8
    
    AIT_REG_W   H264ENC_CURR_Y_ADDR_MSB;            //0x00CA
    AIT_REG_W   H264ENC_CURR_U_ADDR_MSB;            //0x00CC
    AIT_REG_W   H264ENC_CURR_V_ADDR_MSB;            //0x00CE

    AIT_REG_B   H264ENC_REF_Y_ADDR_MSB;             //0x00D0
    AIT_REG_B   H264ENC_REF_BW_Y_ADDR_MSB;          //0x00D1
    AIT_REG_B   H264ENC_REF_UV_ADDR_MSB;            //0x00D2
    AIT_REG_B   H264ENC_REF_BW_UV_ADDR_MSB;         //0x00D3
    AIT_REG_B   H264ENC_REF_V_ADDR_MSB;             //0x00D4
    AIT_REG_B   _x00D5;                             //0x00D5
    
    AIT_REG_B   H264ENC_LINE_MSB_Y_2;               //0x00D6
    AIT_REG_B   H264ENC_LINE_MSB_Y_3;               //0x00D7
    #endif //(CHIP == P_V2)

    AIT_REG_W   H264ENC_ME_REFINE_COUNT;            //0x00D8
    AIT_REG_W   H264ENC_ME_PART_LIMIT_COUNT;        //0x00DA
    AIT_REG_W   H264ENC_ME_PART_COST_THRES;         //0x00DC
    AIT_REG_W   H264ENC_ME_INTER_COST_THRES;        //0x00DE
    AIT_REG_B   H264ENC_INTRA_COST_BIAS;            //0x00E0
    AIT_REG_B   H264ENC_DIRECT_COST_ADJUST;         //0x00E1
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_DIS_DIRECT_MODE               0x10
        #define H264E_DIRECT_COST_ADJ_MASK          0x0F
        /*------------------------------------------------------------*/
    AIT_REG_W   H264ENC_INTER_COST_BIAS;            //0x00E2

    #if (CHIP == VSN_V3)
    AIT_REG_B   _x00E4[0x8];                        //0x00E4
    #endif
    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_HALFPEL_UPSAMPLE0_ADDR;     //0x00E4
    AIT_REG_D   H264ENC_HALFPEL_UPSAMPLE1_ADDR;     //0x00E8
    #endif

    AIT_REG_D   _x00EC;                             //0x00EC
    AIT_REG_D   H264ENC_FIRST_P_AFTER_I_IDX;        //0x00F0
    AIT_REG_D   H264ENC_MV_BUFF_START_ADDR;         //0x00F4

    AIT_REG_W   H264ENC_DIST_SCAL_FAC;              //0x00F8
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_MB_TYPE_CTL;                //0x00FA
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_MB_4X4_DIS                0x20
        #if (CHIP == VSN_V3)
        #define H264E_MB_8X8_DIS                0x10
        #endif
        #define H264E_MB_8X16_DIS               0x08
        #define H264E_MB_16X8_DIS               0x04
        /*------------------------------------------------------------*/
    #endif
	#if (CHIP == P_V2)
    AIT_REG_B   _x00FA;                             //0x00FA
    #endif
    AIT_REG_B   _x00FB[5];                          //0x00FB ~ 0x00FF

    //TRANSFORM
    AIT_REG_B   H264ENC_TRANS_CTL;                  //0x0100
        /*-DEFINE-----------------------------------------------------*/
        #define CBP_CTL_EN                      0x01
        #define TRANS_8x8_FLAG                  0x02
        #define H264E_NS_MODE_MASK              0x0C
        #define H264E_NS_MODE_EN                0x04
        /*------------------------------------------------------------*/
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_NS_MAX_QP;                  //0x0101
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   _x0101;                             //0x0101
    #endif

    AIT_REG_B   H264ENC_LUMA_COEFF_COST;            //0x0102
    AIT_REG_B   H264ENC_CHROMA_COEFF_COST;          //0x0103
    AIT_REG_W   H264ENC_COST_LARGER_THAN_ONE;       //0x0104
    AIT_REG_W   _x0106[5];                          //0x0106

    //SLICE HEADER
    AIT_REG_W   H264ENC_FRAME_NUM;                  //0x0110
    AIT_REG_W   H264ENC_IDR_FRAME;                  //0x0112
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_REF_LIST0_REORDER_CTL0;     //0x0114
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_ABS_DIFF_PIC_NUM_MINUS1(_a)       ((_a & 0xF) << 4)
        #define H264E_REORDER_OF_PIC_NUMS_IDC(_a)       (_a & 0xF)
        /*------------------------------------------------------------*/
    AIT_REG_B   _x0115;                             //0x0115
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   _x0114;                             //0x0114
    #endif
    AIT_REG_W   H264ENC_POC;                        //0x0116
    AIT_REG_W   _x0118[4];                          //0x0118
    
    //SPS
    AIT_REG_B   H264ENC_LEVEL_IDC;                  //0x0120
    AIT_REG_B   H264ENC_MAX_FRAMENUM_AND_POC;       //0x0121
        #define SET_LOG2_MAX_FRAMENUM_MINUS_4(X,Y)  X = (X | (Y & 0xF))
        #define SET_LOG2_MAX_POC_MINUS_4(X,Y)       X = (X | ((Y & 0xF) << 4))
        
    AIT_REG_B   H264ENC_GAPS_IN_FRAME_NUM_ALLOWED;  //0x0122
        /*-DEFINE-----------------------------------------------------*/
        #define GAPS_IN_FRAME_NUM_DIS               0x00
        #define GAPS_IN_FRAME_NUM_EN                0x01
        /*------------------------------------------------------------*/

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_PROFILE_IDC;                //0x0123
    #endif
	#if (CHIP == P_V2)
    AIT_REG_B   _x0123;                             //0x0123
    #endif

    AIT_REG_W   H264ENC_FRAME_CROPPING_FLAG;        //0x0124
        /*-DEFINE-----------------------------------------------------*/
        #define FRAME_CROPPING_DIS                  0x00
        #define FRAME_CROPPING_EN                   0x01
        /*------------------------------------------------------------*/
        
    AIT_REG_B   H264ENC_FRAME_CROPPING_LEFT;        //0x0126
    AIT_REG_B   H264ENC_FRAME_CROPPING_RIGHT;       //0x0127
    AIT_REG_B   H264ENC_FRAME_CROPPING_TOP;         //0x0128
    AIT_REG_B   H264ENC_FRAME_CROPPING_BOTTOM;      //0x0129
    AIT_REG_B   _x012A[6];                          //0x012A

    //PPS
    AIT_REG_B   H264ENC_DBLK_CTRL_PRESENT;          //0x0130
        /*-DEFINE-----------------------------------------------------*/
        #define H264ENC_DBLK_DIS                    0x00
        #define H264ENC_DBLK_EN                     0x01
        /*------------------------------------------------------------*/
    
    AIT_REG_B   _x0131;                             //0x0131
    //AIT_REG_B   H264ENC_CONSTRAINED_INTRA_PRED;     //0x0131
    //    /*-DEFINE-----------------------------------------------------*/
    //    #define H264ENC_CONSTRAINED_INTRA_PRED_DIS  0x00
    //    #define H264ENC_CONSTRAINED_INTRA_PRED_EN   0x01
    //    /*------------------------------------------------------------*/

    AIT_REG_W   _x0132[7];                          //0x0132

    //INTRA
    AIT_REG_B   H264ENC_INTRA_PRED_MODE;            //0x0140
        /*-DEFINE-----------------------------------------------------*/
        #define INTRA_PRED_IN_INTER_SLICE_EN        0x01
        #define ONLY_16X16_INTRA_PRED_EN            0x02
        #define INTRA_8x8_EN                        0x04
        #define INTRA_6_MODE_EN                     0x08
        /*------------------------------------------------------------*/
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_INTRA_16x16_COST_ADJ;       //0x0141
    AIT_REG_W   H264ENC_INTRA_REFRESH_PERIOD;       //0x0142
    AIT_REG_W   H264ENC_INTRA_REFRESH_OFFSET;       //0x0144
    AIT_REG_B   H264ENC_INTRA_REFRESH_MODE;         //0x0146
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_INTRA_REFRESH_MB_MODE         0x01
        #define H264E_INTRA_REFRESH_ROW_MODE        0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   _x0147[9];                          //0x0147
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   _x0141;                             //0x0141
    AIT_REG_W   _x0142[7];                          //0x0142
    #endif

    //QP FINE TUNE
    AIT_REG_B   H264ENC_QP_FINE_TUNE_EN;            //0x0150
        /*-DEFINE-----------------------------------------------------*/
        #define QP_FINETUNE_DIS                     0x00
        #define QP_FINETUNE_ROW_MODE_EN             0x01
        #define QP_FINETUNE_SLICE_MODE_EN           0x03
        #define QP_FINETUNE_MB_MODE_EN              0x05
        #define H264E_ROI_MODE_MASK                 0xC0
        #define H264E_ROI_MODE_CQP                  0x40
        #define H264E_ROI_MODE_RC                   0x80
        /*------------------------------------------------------------*/
    AIT_REG_B   _x0151;                             //0x0151
    AIT_REG_B   H264ENC_QP_BASICUNIT_SIZE;          //0x0152
    AIT_REG_B   H264ENC_ROI_BASIC_QP;               //0x0153
    AIT_REG_W   H264ENC_TGT_MB_SIZE_X256_LSB;       //0x0154
    AIT_REG_B   H264ENC_TGT_MB_SIZE_X256_MSB;       //0x0156
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_ROI_TGT_MB_SIZE_X256_MSB;   //0x0157
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   _x0157;                             //0x0157
    #endif
    AIT_REG_B   H264ENC_BASIC_QP;                   //0x0158
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_QP_TUNE_STEP;               //0x0159
    AIT_REG_B   H264ENC_QP_UP_BOUND;                //0x015A
    AIT_REG_B   H264ENC_QP_LOW_BOUND;               //0x015B
    AIT_REG_W   H264ENC_ROI_TGT_MB_SIZE_X256_LSB;   //0x015C
    AIT_REG_B   H264ENC_ROI_QP_UP_BOUND;            //0x015E
    AIT_REG_B   H264ENC_ROI_QP_LOW_BOUND;           //0x015F
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   _x0159;                             //0x0159
    AIT_REG_W   H264ENC_QP_FINE_TUNE_GAP;           //0x015A
    AIT_REG_W   _x015C[2];                          //0x015C
    #endif

    //ME THRES
    AIT_REG_W   H264ENC_ME_EARLY_STOP_THRES;        //0x0160
    AIT_REG_W   H264ENC_ME_STOP_THRES_UPPER_BOUND;  //0x0162
    AIT_REG_W   H264ENC_ME_STOP_THRES_LOWER_BOUND;  //0x0164
    AIT_REG_W   H264ENC_ME_SKIP_THRES_UPPER_BOUND;  //0x0166
    AIT_REG_W   H264ENC_ME_SKIP_THRES_LOWER_BOUND;  //0x0168
    AIT_REG_W   H264ENC_ME_NO_SUBBLOCK;             //0x016A
        /*-DEFINE-----------------------------------------------------*/
        #define REFINE_PART_NO_SUBBLK               0x03
        /*------------------------------------------------------------*/
    AIT_REG_W   H264ENC_ME_COMPLEXITY;              //0x016C
    AIT_REG_W   H264ENC_ME_INTER_CTL;               //0x016E
        /*-DEFINE-----------------------------------------------------*/
        #define INTER_8_PIXL_ONLY                   0x01
        #define INTER_Y_LINE_SPLIT                  0x02
        #define SKIP_CAND_INCR_ME_STEP              0x04
        #define INTER_PIPE_MODE_EN                  0x08
        #define SKIP_CAND_INCR_ME_STEP_IME          0x10
        #define DIS_2PIX_SEARCH_FME                 0x20
        /*------------------------------------------------------------*/
    AIT_REG_B   H264ENC_ME16X16_MAX_MINUS_1;        //0x0170
    AIT_REG_B   H264ENC_IME16X16_MAX_MINUS_1;       //0x0171
    AIT_REG_B   H264ENC_IME_COST_WEIGHT0;           //0x0172
    AIT_REG_B   H264ENC_IME_COST_WEIGHT1;           //0x0173

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   H264ENC_PMV_CTL;                    //0x0174
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_COLOCATE_MV_DIS               0x08
        #define H264E_PMV_SH_CNT_MASK               0x07
        /*------------------------------------------------------------*/
    AIT_REG_B   _x0175[31];                         //0x0175
    AIT_REG_D   H264ENC_CURR_Y_ADDR1;               //0x0194
    AIT_REG_D   H264ENC_CURR_U_ADDR1;               //0x0198
    AIT_REG_D   H264ENC_CURR_V_ADDR1;               //0x019C
    #endif
	#if (CHIP == P_V2)
    AIT_REG_B   _x0174[44];                         //0x0174
    #endif

    #if (CHIP == VSN_V3)
    AIT_REG_B   _x01A0[0x10];                       //0x01A0
    #endif
	#if (CHIP == P_V2) || (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_LINE_Y_4;                   //0x01A0
    AIT_REG_D   H264ENC_LINE_Y_5;                   //0x01A4
    AIT_REG_D   H264ENC_LINE_Y_6;                   //0x01A8
    AIT_REG_D   H264ENC_LINE_Y_7;                   //0x01AC
	#endif

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    #if (CHIP == VSN_V3)
    AIT_REG_B   _x01B0[0x50];                       //0x01B0
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_D   H264ENC_LINE_U_4;                   //0x01B0
    AIT_REG_D   H264ENC_LINE_U_5;                   //0x01B4
    AIT_REG_B   _x01B8[8];                          //0x01B8
    AIT_REG_D   H264ENC_LINE_V_4;                   //0x01C0
    AIT_REG_D   H264ENC_LINE_V_5;                   //0x01C4
    AIT_REG_B   _x01C8[0x38];                       //0x01C8
    #endif

    AIT_REG_B   H264ENC_ROIWIN_ST_X[8];             //0x0200
    AIT_REG_B   H264ENC_ROIWIN_ED_X[8];             //0x0208
    AIT_REG_B   H264ENC_ROIWIN_ST_Y[8];             //0x0210
    AIT_REG_B   H264ENC_ROIWIN_ED_Y[8];             //0x0218
    AIT_REG_W   H264ENC_COHDR_SEI_LEN;              //0x0220
    AIT_REG_B   H264ENC_COHDR_PNALU_LEN;            //0x0222
    AIT_REG_B   H264ENC_COHDR_SLICE_LEN;            //0x0223
    AIT_REG_B   H264ENC_COHDR_SEI_DAT_40B_ST;       //0x0224
    AIT_REG_B   _x0225[39];                         //0x0225
    AIT_REG_B   H264ENC_COHDR_PNALU_DAT_10B_ST;     //0x024C
    AIT_REG_B   _x024D[9];                          //0x024D
    AIT_REG_B   H264ENC_COHDR_SLICE_DAT_10B_ST;     //0x0256
    AIT_REG_B   _x0257[9];                          //0x0257
    AIT_REG_W   H264ENC_TB_ROW_BUDGET;              //0x0260
    AIT_REG_W   H264ENC_TB_FRM_BUDGET;              //0x0262
    AIT_REG_B   H264ENC_TB_ROI_QP_UP;               //0x0264
    AIT_REG_B   H264ENC_TB_ROI_QP_LOW;              //0x0265
    AIT_REG_B   H264ENC_TB_QP_UP;                   //0x0266
    AIT_REG_B   H264ENC_TB_QP_LOW;                  //0x0267
    AIT_REG_B   H264ENC_TB_EN;                      //0x0268
        /*-DEFINE-----------------------------------------------------*/
        #define H264E_TIMING_BUDGET_EN              0x01
        /*------------------------------------------------------------*/
    #endif
	#if (CHIP == P_V2)
    AIT_REG_D   H264ENC_LINE_UV_4;                  //0x01B0
    AIT_REG_D   H264ENC_LINE_UV_5;                  //0x01B4
    AIT_REG_D   H264ENC_LINE_UV_6;                  //0x01B8
    AIT_REG_D   H264ENC_LINE_UV_7;                  //0x01BC
    AIT_REG_D   H264ENC_REFBD_BW_Y_LOW;             //0x01C0
    AIT_REG_D   H264ENC_REFBD_BW_Y_HIGH;            //0x01C4
    AIT_REG_D   H264ENC_REF_BW_Y_ADDR;              //0x01C8
    AIT_REG_B   _x01CC[4];                          //0x01CC
    AIT_REG_D   H264ENC_LINE_Y_8;                   //0x01D0
    AIT_REG_D   H264ENC_LINE_Y_9;                   //0x01D4
    AIT_REG_D   H264ENC_LINE_Y_10;                  //0x01D8
    AIT_REG_D   H264ENC_LINE_Y_11;                  //0x01DC
    AIT_REG_B   _x01E0[64];                         //0x01E0
    AIT_REG_D   H264ENC_HALFPEL_UPSAMPLE2_ADDR;     //0x0220
    AIT_REG_D   H264ENC_HALFPEL_UPSAMPLE3_ADDR;     //0x0224
    #endif
} AITS_H264ENC, *AITPS_H264ENC;


#if !defined(BUILD_FW)
#define H264ENC_FRAME_CTL                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CTL                 ))) // 0x0000
#define H264ENC_SW_RST                      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SW_RST                    ))) // 0x0002
#define H264ENC_INT_CPU_EN                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INT_CPU_EN                ))) // 0x0004
#define H264ENC_INT_CPU_SR                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INT_CPU_SR                ))) // 0x0006
#define H264ENC_INT_HOST_EN                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INT_HOST_EN               ))) // 0x0008
#define H264ENC_INT_HOST_SR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INT_HOST_SR               ))) // 0x000A
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_SLICE_ARG                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SLICE_ARG                 ))) // 0x0010
#define H264ENC_SLICE_MODE                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SLICE_MODE                ))) // 0x0012
#endif
#if (CHIP == P_V2)
#define H264ENC_SLICE_SIZE_IN_ROW           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SLICE_SIZE_IN_ROW         ))) // 0x0010
#endif
#define H264ENC_QP_SUM                      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_SUM                    ))) // 0x0014
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_SLICE_STUFF_BYTES           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SLICE_STUFF_BYTES         ))) // 0x0020
#define H264ENC_CODED_SLICE_NUM             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CODED_SLICE_NUM           ))) // 0x0024
#endif
#if (CHIP == P_V2)
#define H264ENC_INTRA_MB_COUNT              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INTRA_MB_COUNT            ))) // 0x0018
#define H264ENC_SATD_SUM                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SATD_SUM                  ))) // 0x0020
#endif
#define H264ENC_VLC_CTRL1                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_VLC_CTRL1                 ))) // 0x0030
#define H264ENC_VLC_CTRL2                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_VLC_CTRL2                 ))) // 0x0031
#define H264ENC_BS_START_ADDR               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_BS_START_ADDR             ))) // 0x0034
#define H264ENC_BS_LOWER_BOUND_ADDR         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_BS_LOWER_BOUND_ADDR       ))) // 0x0038
#define H264ENC_BS_UPPER_BOUND_ADDR         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_BS_UPPER_BOUND_ADDR       ))) // 0x003C
#define H264ENC_SLICE_LEN_BUF_ADDR          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_SLICE_LEN_BUF_ADDR        ))) // 0x0040
#define H264ENC_INTRAMB_BITS_COUNT          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INTRAMB_BITS_COUNT        ))) // 0x0044
#define H264ENC_MV_BITS_COUNT               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_MV_BITS_COUNT             ))) // 0x0048
#define H264ENC_HEADER_BITS_COUNT           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_HEADER_BITS_COUNT         ))) // 0x004C
#define H264ENC_FRAME_LENGTH                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_LENGTH              ))) // 0x0050
#define H264ENC_VLC_SR                      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_VLC_SR                    ))) // 0x0054
#if (CHIP == VSN_V3)
#define H264ENC_BS_TURNAROUND_ADDR          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_BS_TURNAROUND_ADDR        ))) // 0x0058
#endif
#if (CHIP == P_V2) || (CHIP == VSN_V2)
#define H264ENC_LINE_Y_0                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_0                  ))) // 0x0064
#define H264ENC_LINE_Y_1                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_1                  ))) // 0x0068
#define H264ENC_LINE_Y_2                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_2                  ))) // 0x006C
#define H264ENC_LINE_Y_3                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_3                  ))) // 0x0070
#if (CHIP == VSN_V2)
#define H264ENC_LINE_U_0                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_0                  ))) // 0x0074
#define H264ENC_LINE_U_1                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_1                  ))) // 0x0078
#define H264ENC_LINE_U_2                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_2                  ))) // 0x007C
#define H264ENC_LINE_U_3                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_3                  ))) // 0x0080
#define H264ENC_LINE_V_0                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_0                  ))) // 0x0084
#define H264ENC_LINE_V_1                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_1                  ))) // 0x0088
#define H264ENC_LINE_V_2                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_2                  ))) // 0x008C
#define H264ENC_LINE_V_3                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_3                  ))) // 0x0090
#endif
#if (CHIP == P_V2)
#define H264ENC_LINE_UV_0                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_0                 ))) // 0x0074
#define H264ENC_LINE_UV_1                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_1                 ))) // 0x0078
#define H264ENC_LINE_UV_2                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_2                 ))) // 0x007C
#define H264ENC_LINE_UV_3                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_3                 ))) // 0x0080
#define H264ENC_LINE_UV_8                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_8                 ))) // 0x0084
#define H264ENC_LINE_UV_9                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_9                 ))) // 0x0088
#define H264ENC_LINE_UV_10                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_10                ))) // 0x008C
#define H264ENC_LINE_UV_11                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_11                ))) // 0x0090
#endif
#endif //(CHIP == P_V2) || (CHIP == VSN_V2)

#define H264ENC_CURR_Y_ADDR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_Y_ADDR               ))) // 0x0094
#define H264ENC_CURR_U_ADDR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_U_ADDR               ))) // 0x0098
#define H264ENC_CURR_V_ADDR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_V_ADDR               ))) // 0x009C

#define H264ENC_REFBD_Y_LOW                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_Y_LOW               ))) // 0x00A0
#define H264ENC_REFBD_Y_HIGH                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_Y_HIGH              ))) // 0x00A4
#if (CHIP == VSN_V3)
#define H264ENC_REFBD_UV_LOW                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_UV_LOW              ))) // 0x00A8
#define H264ENC_REFBD_UV_HIGH               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_UV_HIGH             ))) // 0x00AC
#endif
#if (CHIP == VSN_V2)
#define H264ENC_REFBD_U_LOW                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_U_LOW               ))) // 0x00A8
#define H264ENC_REFBD_U_HIGH                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_U_HIGH              ))) // 0x00AC
#define H264ENC_REFBD_V_LOW                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_V_LOW               ))) // 0x00B0
#define H264ENC_REFBD_V_HIGH                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_V_HIGH              ))) // 0x00B4
#endif
#if (CHIP == P_V2)
#define H264ENC_REFBD_UV_LOW                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_UV_LOW              ))) // 0x00A8
#define H264ENC_REFBD_UV_HIGH               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_UV_HIGH             ))) // 0x00AC
#define H264ENC_REFBD_BW_UV_LOW             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_BW_UV_LOW           ))) // 0x00B0
#define H264ENC_REFBD_BW_UV_HIGH            (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_BW_UV_HIGH          ))) // 0x00B4
#endif

#define H264ENC_REF_Y_ADDR                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_Y_ADDR                ))) // 0x00B8
#if (CHIP == VSN_V3)
#define H264ENC_REF_UV_ADDR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_UV_ADDR               ))) // 0x00BC
#endif
#if (CHIP == VSN_V2)
#define H264ENC_REF_U_ADDR                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_U_ADDR                ))) // 0x00BC
#define H264ENC_REF_V_ADDR                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_V_ADDR                ))) // 0x00C0
#endif
#if (CHIP == P_V2)
#define H264ENC_REF_UV_ADDR                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_UV_ADDR               ))) // 0x00BC
#define H264ENC_REF_BW_UV_ADDR              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_BW_UV_ADDR            ))) // 0x00C0
#endif

#if (CHIP == P_V2)
#define H264ENC_LINE_MSB_Y_0                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_Y_0              ))) // 0x00C4
#define H264ENC_LINE_MSB_Y_1                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_Y_1              ))) // 0x00C5
#define H264ENC_LINE_MSB_UV                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_UV               ))) // 0x00C6
#define H264ENC_LINE_MSB_V                  (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_V                ))) // 0x00C8
#define H264ENC_CURR_Y_ADDR_MSB             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_Y_ADDR_MSB           ))) // 0x00CA
#define H264ENC_CURR_U_ADDR_MSB             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_U_ADDR_MSB           ))) // 0x00CC
#define H264ENC_CURR_V_ADDR_MSB             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_V_ADDR_MSB           ))) // 0x00CE
#define H264ENC_REF_Y_ADDR_MSB              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_Y_ADDR_MSB            ))) // 0x00D0
#define H264ENC_REF_BW_Y_ADDR_MSB           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_BW_Y_ADDR_MSB         ))) // 0x00D1
#define H264ENC_REF_UV_ADDR_MSB             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_UV_ADDR_MSB           ))) // 0x00D2
#define H264ENC_REF_BW_UV_ADDR_MSB          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_BW_UV_ADDR_MSB        ))) // 0x00D3
#define H264ENC_REF_V_ADDR_MSB              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_V_ADDR_MSB            ))) // 0x00D4
#define H264ENC_LINE_MSB_Y_2                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_Y_2              ))) // 0x00D6
#define H264ENC_LINE_MSB_Y_3                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_MSB_Y_3              ))) // 0x00D7
#endif

#define H264ENC_ME_REFINE_COUNT             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_REFINE_COUNT           ))) // 0x00D8
#define H264ENC_ME_PART_LIMIT_COUNT         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_PART_LIMIT_COUNT       ))) // 0x00DA
#define H264ENC_ME_PART_COST_THRES          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_PART_COST_THRES        ))) // 0x00DC
#define H264ENC_ME_INTER_COST_THRES         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_INTER_COST_THRES       ))) // 0x00DE
#define H264ENC_INTRA_COST_BIAS             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INTRA_COST_BIAS           ))) // 0x00E0
#define H264ENC_DIRECT_COST_ADJUST          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_DIRECT_COST_ADJUST        ))) // 0x00E1
#define H264ENC_INTER_COST_BIAS             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INTER_COST_BIAS           ))) // 0x00E2
#define H264ENC_HALFPEL_UPSAMPLE0_ADDR      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_HALFPEL_UPSAMPLE0_ADDR    ))) // 0x00E4
#define H264ENC_HALFPEL_UPSAMPLE1_ADDR      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_HALFPEL_UPSAMPLE1_ADDR    ))) // 0x00E8
#define H264ENC_FIRST_P_AFTER_I_IDX         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FIRST_P_AFTER_I_IDX       ))) // 0x00F0
#define H264ENC_MV_BUFF_START_ADDR          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_MV_BUFF_START_ADDR        ))) // 0x00F4
#define H264ENC_MB_TYPE_CTL                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_MB_TYPE_CTL               ))) // 0x00FA
#define H264ENC_TRANS_CTL                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TRANS_CTL                 ))) // 0x0100
#define H264ENC_LUMA_COEFF_COST             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LUMA_COEFF_COST           ))) // 0x0102
#define H264ENC_CHROMA_COEFF_COST           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CHROMA_COEFF_COST         ))) // 0x0103
#define H264ENC_COST_LARGER_THAN_ONE        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COST_LARGER_THAN_ONE      ))) // 0x0104
#define H264ENC_FRAME_NUM                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_NUM                 ))) // 0x0110
#define H264ENC_IDR_FRAME                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_IDR_FRAME                 ))) // 0x0112
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_REF_LIST0_REORDER_CTL0      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_LIST0_REORDER_CTL0    ))) // 0x0114
#endif
#define H264ENC_POC                         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_POC                       ))) // 0x0116
#define H264ENC_LEVEL_IDC                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LEVEL_IDC                 ))) // 0x0120
#define H264ENC_MAX_FRAMENUM_AND_POC        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_MAX_FRAMENUM_AND_POC      ))) // 0x0121
#define H264ENC_GAPS_IN_FRAME_NUM_ALLOWED   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_GAPS_IN_FRAME_NUM_ALLOWED ))) // 0x0122
#if (CHIP == VSN_V2)
#define H264ENC_PROFILE_IDC                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_PROFILE_IDC               ))) // 0x0123
#endif
#define H264ENC_FRAME_CROPPING_FLAG         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CROPPING_FLAG       ))) // 0x0124
#define H264ENC_FRAME_CROPPING_LEFT         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CROPPING_LEFT       ))) // 0x0126
#define H264ENC_FRAME_CROPPING_RIGHT        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CROPPING_RIGHT      ))) // 0x0127
#define H264ENC_FRAME_CROPPING_TOP          (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CROPPING_TOP        ))) // 0x0128
#define H264ENC_FRAME_CROPPING_BOTTOM       (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_FRAME_CROPPING_BOTTOM     ))) // 0x0129
#define H264ENC_DBLK_CTRL_PRESENT           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_DBLK_CTRL_PRESENT         ))) // 0x0130
#define H264ENC_INTRA_PRED_MODE             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_INTRA_PRED_MODE           ))) // 0x0140
#define H264ENC_QP_FINE_TUNE_EN             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_FINE_TUNE_EN           ))) // 0x0150
#define H264ENC_QP_BASICUNIT_SIZE           (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_BASICUNIT_SIZE         ))) // 0x0152
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_ROI_BASIC_QP                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROI_BASIC_QP              ))) // 0x0153
#endif
#define H264ENC_TGT_MB_SIZE_X256_LSB        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TGT_MB_SIZE_X256_LSB      ))) // 0x0154
#define H264ENC_TGT_MB_SIZE_X256_MSB        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TGT_MB_SIZE_X256_MSB      ))) // 0x0155
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_ROI_TGT_MB_SIZE_X256_MSB    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROI_TGT_MB_SIZE_X256_MSB  ))) // 0x0157
#endif
#define H264ENC_BASIC_QP                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_BASIC_QP                  ))) // 0x0158
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_QP_TUNE_STEP                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_TUNE_STEP              ))) // 0x0159
#define H264ENC_QP_LOW_BOUND                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_LOW_BOUND              ))) // 0x015A
#define H264ENC_QP_UP_BOUND                 (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_UP_BOUND               ))) // 0x015B
#define H264ENC_ROI_TGT_MB_SIZE_X256_LSB    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROI_TGT_MB_SIZE_X256_LSB  ))) // 0x015C
#define H264ENC_ROI_QP_LOW_BOUND            (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROI_QP_LOW_BOUND          ))) // 0x015E
#define H264ENC_ROI_QP_UP_BOUND             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROI_QP_UP_BOUND           ))) // 0x015F
#endif
#if (CHIP == P_V2)
#define H264ENC_QP_FINE_TUNE_GAP            (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_QP_FINE_TUNE_GAP          ))) // 0x015A
#endif
#define H264ENC_ME_EARLY_STOP_THRES         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_EARLY_STOP_THRES       ))) // 0x0160
#define H264ENC_ME_STOP_THRES_UPPER_BOUND   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_STOP_THRES_UPPER_BOUND ))) // 0x0162
#define H264ENC_ME_STOP_THRES_LOWER_BOUND   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_STOP_THRES_LOWER_BOUND ))) // 0x0164
#define H264ENC_ME_SKIP_THRES_UPPER_BOUND   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_SKIP_THRES_UPPER_BOUND ))) // 0x0166
#define H264ENC_ME_SKIP_THRES_LOWER_BOUND   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_SKIP_THRES_LOWER_BOUND ))) // 0x0167
#define H264ENC_ME_NO_SUBBLOCK              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_NO_SUBBLOCK            ))) // 0x016A
#define H264ENC_ME_COMPLEXITY               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_COMPLEXITY             ))) // 0x016C
#define H264ENC_ME_INTER_CTL                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_INTER_CTL              ))) // 0x016E
#define H264ENC_ME16X16_MAX_MINUS_1         (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME16X16_MAX_MINUS_1       ))) // 0x0170
#define H264ENC_IME16X16_MAX_MINUS_1        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_IME16X16_MAX_MINUS_1      ))) // 0x0171
#define H264ENC_IME_COST_WEIGHT0            (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_IME_COST_WEIGHT0          ))) // 0x0172
#define H264ENC_IME_COST_WEIGHT1            (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_IME_COST_WEIGHT1          ))) // 0x0173
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define H264ENC_PMV_CTL                     (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_PMV_CTL                   ))) // 0x0174
#define H264ENC_CURR_Y_ADDR1                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_Y_ADDR1              ))) // 0x0194
#define H264ENC_CURR_U_ADDR1                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_U_ADDR1              ))) // 0x0194
#define H264ENC_CURR_V_ADDR1                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_CURR_V_ADDR1              ))) // 0x0194
#endif
#define H264ENC_LINE_Y_4                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_4                  ))) // 0x01A0
#define H264ENC_LINE_Y_5                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_5                  ))) // 0x01A4
#define H264ENC_LINE_Y_6                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_6                  ))) // 0x01A8
#define H264ENC_LINE_Y_7                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_7                  ))) // 0x01AC
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#if (CHIP == VSN_V2)
#define H264ENC_LINE_U_4                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_4                  ))) // 0x01B0
#define H264ENC_LINE_U_5                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_U_5                  ))) // 0x01B4
#define H264ENC_LINE_V_4                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_4                  ))) // 0x01C0
#define H264ENC_LINE_V_5                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_V_5                  ))) // 0x01C4
#endif
#define H264ENC_ROIWIN_ST_X_0               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROIWIN_ST_X[0]            ))) // 0x0200
#define H264ENC_ROIWIN_ED_X_0               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROIWIN_ED_X[0]            ))) // 0x0208
#define H264ENC_ROIWIN_ST_Y_0               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROIWIN_ST_Y[0]            ))) // 0x0210
#define H264ENC_ROIWIN_ED_Y_0               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ROIWIN_ED_Y[0]            ))) // 0x0208
#define H264ENC_COHDR_SEI_LEN               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_SEI_LEN             ))) // 0x0220
#define H264ENC_COHDR_PNALU_LEN             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_PNALU_LEN           ))) // 0x0222
#define H264ENC_COHDR_SLICE_LEN             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_SLICE_LEN           ))) // 0x0223
#define H264ENC_COHDR_SEI_DAT_40B_ST        (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_SEI_DAT_40B_ST      ))) // 0x0224
#define H264ENC_COHDR_PNALU_DAT_10B_ST      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_PNALU_DAT_10B_ST    ))) // 0x024C
#define H264ENC_COHDR_SLICE_DAT_10B_ST      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_COHDR_SLICE_DAT_10B_ST    ))) // 0x0256

#define H264ENC_TB_ROW_BUDGET               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_ROW_BUDGET             ))) // 0x0260
#define H264ENC_TB_FRM_BUDGET               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_FRM_BUDGET             ))) // 0x0262
#define H264ENC_TB_ROI_QP_UP                (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_ROI_QP_UP              ))) // 0x0264
#define H264ENC_TB_ROI_QP_LOW               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_ROI_QP_LOW             ))) // 0x0265
#define H264ENC_TB_QP_UP                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_QP_UP                  ))) // 0x0266
#define H264ENC_TB_QP_LOW                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_QP_LOW                 ))) // 0x0267
#define H264ENC_TB_EN                       (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_TB_EN                     ))) // 0x0268
#endif
#if (CHIP == P_V2)
#define H264ENC_LINE_UV_4                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_4                 ))) // 0x01B0
#define H264ENC_LINE_UV_5                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_5                 ))) // 0x01B4
#define H264ENC_LINE_UV_6                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_6                 ))) // 0x01B8
#define H264ENC_LINE_UV_7                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_UV_7                 ))) // 0x01BC
#define H264ENC_REFBD_BW_Y_LOW              (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_BW_Y_LOW            ))) // 0x01C0
#define H264ENC_REFBD_BW_Y_HIGH             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REFBD_BW_Y_HIGH           ))) // 0x01C4
#define H264ENC_REF_BW_Y_ADDR               (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_REF_BW_Y_ADDR             ))) // 0x01C8
#define H264ENC_ME_BUF_V_ADDR_7             (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_ME_BUF_V_ADDR_7           ))) // 0x01CC
#define H264ENC_LINE_Y_8                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_8                  ))) // 0x01D0
#define H264ENC_LINE_Y_9                    (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_9                  ))) // 0x01D4
#define H264ENC_LINE_Y_10                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_10                 ))) // 0x01D8
#define H264ENC_LINE_Y_11                   (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_LINE_Y_11                 ))) // 0x01DC
#define H264ENC_HALFPEL_UPSAMPLE2_ADDR      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_HALFPEL_UPSAMPLE2_ADDR    ))) // 0x0220
#define H264ENC_HALFPEL_UPSAMPLE3_ADDR      (H264ENC_BASE + (MMP_ULONG)(&(((AITPS_H264ENC)0)->H264ENC_HALFPEL_UPSAMPLE3_ADDR    ))) // 0x0224
#endif


#endif //!define(BUILD_FW)

/// @}

#endif// _MMPH_REG_H264ENC_H_
///@end_ait_only