//==============================================================================
//
//  File        : mmp_reg_scaler.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_SCALER_H_
#define _MMP_REG_SCALER_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
typedef struct _AITS_SCAL_CT {
    AIT_REG_B   IN_COLOR_CTL;                               // 0x00
    AIT_REG_B   IN_Y_GAIN_MINUS1;                           // 0x01
    AIT_REG_B   IN_U_GAIN_MINUS1;                           // 0x02
    AIT_REG_B   IN_V_GAIN_MINUS1;                           // 0x03
    AIT_REG_B   IN_Y_OFST;                                  // 0x04
    AIT_REG_B   IN_U_OFST;                                  // 0x05
    AIT_REG_B   IN_V_OFST;                                  // 0x06
    AIT_REG_B                           _x7;
} AITS_SCAL_CT, *AITPS_SCAL_CT;

typedef struct _AITS_SCAL_COLR_CLIP {
    AIT_REG_B   OUT_Y_MIN;                                  // 0x00
    AIT_REG_B   OUT_Y_MAX;                                  // 0x01
    AIT_REG_B   OUT_U_MIN;                                  // 0x02
    AIT_REG_B   OUT_U_MAX;                                  // 0x03
    AIT_REG_B   OUT_V_MIN;                                  // 0x04
    AIT_REG_B   OUT_V_MAX;                                  // 0x05
    AIT_REG_W                           _x6;
} AITS_SCAL_COLR_CLIP, *AITPS_SCAL_COLR_CLIP;

typedef struct _AITS_SCAL_MTX {
    AIT_REG_B   COLRMTX_CTL;                                // 0x00
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_COLRMTX_EN                     0x01
        #define SCAL_MTX_Y_OFST_MSB                 0x10
        #define SCAL_MTX_U_OFST_MSB                 0x20
        #define SCAL_MTX_V_OFST_MSB                 0x40
        /*------------------------------------------------------------*/
    AIT_REG_B   MTX_Y_OFST;                                 // 0x01
    AIT_REG_B   MTX_U_OFST;                                 // 0x02
    AIT_REG_B   MTX_V_OFST;                                 // 0x03
    AIT_REG_B   MTX_COEFF_ROW1[3];                          // 0x04~0x06
    AIT_REG_B   MTX_COEFF_ROW1_MSB;                         // 0x07
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_MTX_COL_COEFF_MSB_MASK         0x3F
        #define SCAL_MTX_COL_COEFF_MSB(_a, _c)      (((_a >> 8) & 0x03) << (_c << 1))
        #define SIGN_BIT_CONVERT(_a, _nb)           ((_a < 0)? (((MMP_LONG)-_a) | (1 << (_nb-1))): _a)
        /*------------------------------------------------------------*/
    AIT_REG_B   MTX_COEFF_ROW2[3];                          // 0x08~0x0A
    AIT_REG_B   MTX_COEFF_ROW2_MSB;                         // 0x0B
    AIT_REG_B   MTX_COEFF_ROW3[3];                          // 0x0C~0x0E
    AIT_REG_B   MTX_COEFF_ROW3_MSB;                         // 0x0F
} AITS_SCAL_MTX, *AITPS_SCAL_MTX;
#endif

// *********************************
//   SCAL  Structure (0x8000 0800)
//   SCAL  Structure (0x8000 6400), VSN_V2
// *********************************
typedef struct _AITS_SCAL {
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_W   SCAL_GRAB_LPF2_H_ST;                                // 0x00, base 0x6400
    AIT_REG_W   SCAL_GRAB_LPF2_H_ED;
    AIT_REG_W   SCAL_GRAB_LPF2_V_ST;
    AIT_REG_W   SCAL_GRAB_LPF2_V_ED;
    AIT_REG_B                           _x008[8];

    AIT_REG_W   SCAL_GRAB_SCAL_2_H_ST;                              // 0x10
    AIT_REG_W   SCAL_GRAB_SCAL_2_H_ED;
    AIT_REG_W   SCAL_GRAB_SCAL_2_V_ST;
    AIT_REG_W   SCAL_GRAB_SCAL_2_V_ED;
    AIT_REG_B                           _x018[8];

    AIT_REG_W   SCAL_GRAB_OUT_2_H_ST;                               // 0x20
    AIT_REG_W   SCAL_GRAB_OUT_2_H_ED;
    AIT_REG_W   SCAL_GRAB_OUT_2_V_ST;
    AIT_REG_W   SCAL_GRAB_OUT_2_V_ED;
    #if 1
    AIT_REG_B                           _x028[0x8];
    AIT_REG_B   SCAL_DNSAMP_LPF2_H;                                      // 0x30
    AIT_REG_B   SCAL_DNSAMP_LPF2_V;
    AIT_REG_B   SCAL_DNSAMP_SCAL_2_H;
    AIT_REG_B   SCAL_DNSAMP_SCAL_2_V;
    AIT_REG_B   SCAL_DNSAMP_OUT_2_H;
    AIT_REG_B   SCAL_DNSAMP_OUT_2_V;
    AIT_REG_B                           _x036[0xA];
    #else
    AIT_REG_B                           _x028[0x18];
    #endif
    AIT_REG_B   SCAL_LPF2_SCAL_CTL;                                 // 0x40
    AIT_REG_B                           _x041;
    AIT_REG_B   SCAL_LPF2_AUTO_CTL;                                 // 0x42
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_LPF2_EN                0x10 
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x043[0xD];
    AIT_REG_B   SCAL_SCAL_2_CTL;                                    // 0x50
        /*-DEFINE-----------------------------------------------------*/
        // ref to SCAL_SCAL_0_CTL
        /*------------------------------------------------------------*/

    AIT_REG_B                           _x051;
    AIT_REG_B   SCAL_SCAL_2_H_N;                                    // 0x52
    AIT_REG_B   SCAL_SCAL_2_H_M;
    AIT_REG_B   SCAL_SCAL_2_V_N;                                    // 0x54
    AIT_REG_B   SCAL_SCAL_2_V_M;
    AIT_REG_B   SCAL_SCAL_2_H_WT;                                   // 0x56
    AIT_REG_B   SCAL_SCAL_2_V_WT;                                   // 0x57
    AIT_REG_B                           _x058[8];

    AIT_REG_B   SCAL_EDGE_2_CTL;                                    // 0x60
        /*-DEFINE-----------------------------------------------------*/
        // ref to SCAL_EDGE_0_CTL
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x061;
    AIT_REG_B   SCAL_EDGE_2_GAIN;                                   // 0x62
    AIT_REG_B   SCAL_EDGE_2_CORE;                                   // 0x63
    AIT_REG_B                           _x064[0xC];
    
    AIT_REG_B   SCAL_OUT_2_CTL;                                     // 0x70
        /*-DEFINE-----------------------------------------------------*/
        // ref to SCAL_OUT_0_CTL
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x071;
    AIT_REG_B   SCAL_PATH_2_CTL;                                    // 0x72
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_GRA_2_SCAL2        0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_P2_GRA_FLOW_CTL_EN;                            // 0x73
    AIT_REG_B   SCAL_P2_PIXL_DELAY;                                 // 0x74
    AIT_REG_B   SCAL_P2_LINE_DELAY;                                 // 0x75

    #if (CHIP == VSN_V3)
    AIT_REG_W                           _x076;
    AIT_REG_B   SCAL_P2_BUSY_MODE_CTL;                              // 0x78
        /*-DEFINE-----------------------------------------------------*/
        // ref to SCAL_P0_BUSY_MODE_CTL
        /*------------------------------------------------------------*/
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_B                           _x076[3];
    #endif
    AIT_REG_B                           _x079;
    AIT_REG_B                           _x07A[0x2A];

    AIT_REG_B   SCAL_P2_CPU_INT_EN;                                 // 0xA4
    AIT_REG_B   SCAL_P2_HOST_INT_EN;                                // 0xA5
    AIT_REG_B   SCAL_P2_CPU_INT_SR;                                 // 0xA6
    AIT_REG_B   SCAL_P2_HOST_INT_SR;                                // 0xA7
        /*-DEFINE-----------------------------------------------------*/
		#define SCAL2_INT_FRM_END       0x01
		#define SCAL2_INT_INPUT_END     0x02
		#define SCAL2_INT_UP_OVWR       0x04
		#define SCAL2_INT_DBL_FRM_ST    0x08
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x0A8[8];
    AITS_SCAL_CT            SCAL_P2_COLR_TRANS;                     // 0xB0~0xB7
    AIT_REG_B                           _x0B8[8];
    AITS_SCAL_COLR_CLIP     SCAL_P2_EE_CLIP;                        // 0xC0~0xC7
    AIT_REG_B                           _x0C8[8];
    AITS_SCAL_MTX           SCAL_P2_CT_MTX;                         // 0xD0~0xDF
    AIT_REG_B                           _x0E0[0x10];
    AITS_SCAL_COLR_CLIP     SCAL_P2_MTX_CLIP;                       // 0xF0~0xF7
    AIT_REG_B                           _x0F8[8];
    AIT_REG_B                           _x100[0xA00];
    #endif

    AIT_REG_W   SCAL_GRAB_LPF_H_ST;                                     // 0x00, base 0x6F00
    AIT_REG_W   SCAL_GRAB_LPF_H_ED;
    AIT_REG_W   SCAL_GRAB_LPF_V_ST;
    AIT_REG_W   SCAL_GRAB_LPF_V_ED;

    AIT_REG_W   SCAL_GRAB_LPF1_H_ST;                                     // 0x08
    AIT_REG_W   SCAL_GRAB_LPF1_H_ED;
    AIT_REG_W   SCAL_GRAB_LPF1_V_ST;
    AIT_REG_W   SCAL_GRAB_LPF1_V_ED;

    AIT_REG_W   SCAL_GRAB_SCAL_0_H_ST;                                  // 0x10
    AIT_REG_W   SCAL_GRAB_SCAL_0_H_ED;
    AIT_REG_W   SCAL_GRAB_SCAL_0_V_ST;
    AIT_REG_W   SCAL_GRAB_SCAL_0_V_ED;
    AIT_REG_W   SCAL_GRAB_SCAL_1_H_ST;
    AIT_REG_W   SCAL_GRAB_SCAL_1_H_ED;
    AIT_REG_W   SCAL_GRAB_SCAL_1_V_ST;
    AIT_REG_W   SCAL_GRAB_SCAL_1_V_ED;

    AIT_REG_W   SCAL_GRAB_OUT_0_H_ST;                                   // 0x20
    AIT_REG_W   SCAL_GRAB_OUT_0_H_ED;
    AIT_REG_W   SCAL_GRAB_OUT_0_V_ST;
    AIT_REG_W   SCAL_GRAB_OUT_0_V_ED;
    AIT_REG_W   SCAL_GRAB_OUT_1_H_ST;
    AIT_REG_W   SCAL_GRAB_OUT_1_H_ED;
    AIT_REG_W   SCAL_GRAB_OUT_1_V_ST;
    AIT_REG_W   SCAL_GRAB_OUT_1_V_ED;

    AIT_REG_B   SCAL_DNSAMP_LPF_H;                                      // 0x30
    AIT_REG_B   SCAL_DNSAMP_LPF_V;
    AIT_REG_B   SCAL_DNSAMP_SCAL_0_H;
    AIT_REG_B   SCAL_DNSAMP_SCAL_0_V;
    AIT_REG_B   SCAL_DNSAMP_SCAL_1_H;
    AIT_REG_B   SCAL_DNSAMP_SCAL_1_V;
    AIT_REG_B   SCAL_DNSAMP_OUT_0_H;
    AIT_REG_B   SCAL_DNSAMP_OUT_0_V;
    AIT_REG_B   SCAL_DNSAMP_OUT_1_H;
    AIT_REG_B   SCAL_DNSAMP_OUT_1_V;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_DNSAMP_NONE            0x00
        #define SCAL_DNSAMP_1_2             0x01
        #define SCAL_DNSAMP_1_3             0x02
        #define SCAL_DNSAMP_1_4             0x03
        #define SCAL_DNSAMP_MASK            0x03
        /*------------------------------------------------------------*/

    AIT_REG_B   SCAL_DNSAMP_LPF1_H;                                     // 0x3A
    AIT_REG_B   SCAL_DNSAMP_LPF1_V;
    AIT_REG_B                           _x3A[4];

    AIT_REG_B   SCAL_LPF_SCAL_CTL;                                      // 0x40
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_LPF_DNSAMP_NONE        0x00
        #define SCAL_LPF_DNSAMP_1_2         0x02
        #define SCAL_LPF_DNSAMP_1_4         0x03
        #define SCAL_LPF_DNSAMP_MASK        0x03
        #define SCAL_LPF_Y_L1_EN            0x10
        #define SCAL_LPF_Y_L2_EN            0x20
        #define SCAL_LPF_UV_EN              0x40
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_LPF_CTL;                                           // 0x41
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_LPF_BYPASS             0x00
        #define SCAL_LPF_4X4                0x02
        #define SCAL_LPF_7X4                0x03
        #define SCAL_LPF_MASK				0x03
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_LPF_AUTO_CTL;                                      // 0x42
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_LPF0_EN                0x10 
        #define SCAL_LPF1_EN                0x20 

        #define SCAL_LPF_BY_SCAL_0_NM       0x10 // LPF 1:1 mode check
        #define SCAL_LPF_BY_SCAL_1_NM       0x20
        /*------------------------------------------------------------*/

    AIT_REG_B                           _x43;
    AIT_REG_B   SCAL_LPF1_SCAL_CTL;                                     // 0x44
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _x45;
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   SCAL_LPF1_CTL;								            // 0x45
    #endif
    AIT_REG_B                           _x46[10];

    AIT_REG_B   SCAL_SCAL_0_CTL;                                        // 0x50
    AIT_REG_B   SCAL_SCAL_1_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_SCAL_EN                0x00		
        #define SCAL_SCAL_BYPASS            0x01
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define SCAL_UP_EARLY_ST            0x04
        #endif
        #if (CHIP == P_V2)
        #define SCAL_SCAL_UP_CONT           0x02
        #define SCAL_SCAL_DBL_FIFO          0x04
        #endif
        #define SCAL_SCAL_PATH_EN           0x80
        /*------------------------------------------------------------*/
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   SCAL_SCAL_0_H_N;                                         // 0x52
    AIT_REG_B   SCAL_SCAL_0_H_M;
    AIT_REG_B   SCAL_SCAL_1_H_N;                                         // 0x54
    AIT_REG_B   SCAL_SCAL_1_H_M;
    AIT_REG_B   SCAL_SCAL_0_H_WT;                                        // 0x56
    AIT_REG_B   SCAL_SCAL_1_H_WT;
    AIT_REG_B   SCAL_SCAL_0_V_N;                                         // 0x58
    AIT_REG_B   SCAL_SCAL_0_V_M;
    AIT_REG_B   SCAL_SCAL_1_V_N;                                         // 0x5A
    AIT_REG_B   SCAL_SCAL_1_V_M;
    AIT_REG_B   SCAL_SCAL_0_V_WT;                                        // 0x5C
    AIT_REG_B   SCAL_SCAL_1_V_WT;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_NM_MAX                 0x7F
        #define SCAL_SCAL_WT_AVG            0x01        // default
        #define SCAL_SCAL_WT_MASK           0xFE
        #define SCAL_SCAL_WT_SHFT           1
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x5E;
    #endif
    #if (CHIP == P_V2)
    AIT_REG_W   SCAL_SCAL_0_H_M;                                         // 0x52
    AIT_REG_W   SCAL_SCAL_0_H_N;
    AIT_REG_W   SCAL_SCAL_0_H_WT;
    AIT_REG_W   SCAL_SCAL_1_H_M;                                         // 0x58
    AIT_REG_W   SCAL_SCAL_1_H_N;
    AIT_REG_W   SCAL_SCAL_1_H_WT;
    AIT_REG_W                           _x5E;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_NM_MAX                 0x3FF
        #define SCAL_SCAL_WT_AVG            0x001        // default
        #define SCAL_SCAL_WT_MASK           0x3FE
        #define SCAL_SCAL_WT_SHFT           1
        /*------------------------------------------------------------*/
    #endif
    AIT_REG_B   SCAL_EDGE_0_CTL;                                    // 0x60
    AIT_REG_B   SCAL_EDGE_1_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_EDGE_EN                0x00
        #define SCAL_EDGE_BYPASS            0x01
        #define SCAL_EDGE_YUV_AVG_DIS       0x00
        #define SCAL_EDGE_YUV_AVG_EN        0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_EDGE_0_GAIN;                                   // 0x62
    AIT_REG_B   SCAL_EDGE_0_CORE;
    AIT_REG_B   SCAL_EDGE_1_GAIN;
    AIT_REG_B   SCAL_EDGE_1_CORE;
    AIT_REG_B                           _x66[10];

    AIT_REG_B   SCAL_OUT_0_CTL;                                     // 0x70
    AIT_REG_B   SCAL_OUT_1_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_OUT_YUV_EN             0x01
        #define SCAL_YUV422_LPF_EN          0x04
        #define SCAL_OUT_FMT_YUV422         0x08
        #if (CHIP == P_V2)
        #define SCAL_OUT_FMT_YUV444         0x00
        #define SCAL_OUT_RGB_EN             0x02
        #define SCAL_OUT_FMT_RGB888         0x00
        #define SCAL_OUT_FMT_RGB565         0x10
        #define SCAL_OUT_FMT_RGB555         0x20
        #define SCAL_OUT_DITHER_EN          0x40
        #define SCAL_OUT_RGB                0x00
        #define SCAL_OUT_BGR                0x80
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_PATH_CTL;      						        // 0x72
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define SCAL_GRA_2_SCAL0            0x01	///< from graphic engine
        #define SCAL_GRA_2_SCAL1            0x02	///< from graphic engine
        #endif
        #if (CHIP == P_V2)
        #define SCAL_ISP_2_LPF              0x00	///< from ISP
        #define SCAL_GRA_2_LPF              0x01	///< from graphic engine
        #define SCAL_JPG_2_LPF              0x08	///< from JPEG decode engine
        #define SCAL_LPF_PATH_SEL_MASK      0x09	///< the input mask of the LPF of pipe0

        #define SCAL_ISP_2_LPF1             0x00	///< from ISP
        #define SCAL_GRA_2_LPF1             0x40	///< from graphic engine
        #define SCAL_JPG_2_LPF1             0x80	///< from JPEG decode engine
        #define SCAL_LPF1_PATH_SEL_MASK     0xC0	///< the input mask of the LPF of pipe1

        #define SCAL_LPF_2_SCAL0            0x00	///< from LPF0
        #define SCAL_GRA_2_SCAL0            0x02	///< from graphic engine
        #define SCAL_JPG_2_SCAL0            0x10	///< from JPEG decode engine
        #define SCAL_SCAL0_PATH_SEL_MASK    0x12	///< the input mask of the scaler of pipe0

        #define SCAL_LPF_2_SCAL1            0x00	///< from LPF1
        #define SCAL_GRA_2_SCAL1            0x04	///< from graphic engine
        #define SCAL_JPG_2_SCAL1            0x20	///< from JPEG decode engine
        #define SCAL_SCAL1_PATH_SEL_MASK    0x24	///< the input mask of the scaler of pipe1
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_B   SCAL_GRA_FLOW_CTL_EN;                               // 0x73
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   SCAL_P0_PIXL_DELAY;                                 // 0x74
    AIT_REG_B   SCAL_P0_LINE_DELAY;                                 // 0x75
    AIT_REG_B   SCAL_P1_PIXL_DELAY;                                 // 0x76
    AIT_REG_B   SCAL_P1_LINE_DELAY;                                 // 0x77
    #if (CHIP == VSN_V3)
    AIT_REG_B   SCAL_P0_BUSY_MODE_CTL;                              // 0x78
    AIT_REG_B   SCAL_P1_BUSY_MODE_CTL;                              // 0x79
        /*-DEFINE-----------------------------------------------------*/
        #define SCAL_BUSY_MODE_EN           0x01
        /*------------------------------------------------------------*/
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_B                           _x78[0x2];
    #endif
    AIT_REG_B                           _x7A[0x26];
    AIT_REG_B   SCAL_DBG_MODE_EN;                                   // 0xA0
    AIT_REG_B   SCAL_DBG_MODE_DATA;                                 // 0xA1
    AIT_REG_W                           _xA2;
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B                           _x74[14];
    AIT_REG_W   SCAL_SCAL_0_V_M;                                    // 0x82
    AIT_REG_W   SCAL_SCAL_0_V_N;
    AIT_REG_W   SCAL_SCAL_0_V_WT;
    AIT_REG_W   SCAL_SCAL_1_V_M;                                    // 0x88
    AIT_REG_W   SCAL_SCAL_1_V_N;
    AIT_REG_W   SCAL_SCAL_1_V_WT;
    AIT_REG_W                           _x8E;
    AIT_REG_B   SCAL_CONV_0_UV_11;                                  // 0x90
    AIT_REG_B   SCAL_CONV_0_UV_12;
    AIT_REG_B   SCAL_CONV_0_UV_21;
    AIT_REG_B   SCAL_CONV_0_UV_22;
    AIT_REG_B   SCAL_CONV_0_RGB_GAIN;
    AIT_REG_B   SCAL_CONV_0_RGB_OFST;
    AIT_REG_B                           _x96[2];
    AIT_REG_B   SCAL_CONV_1_UV_11;
    AIT_REG_B   SCAL_CONV_1_UV_12;
    AIT_REG_B   SCAL_CONV_1_UV_21;
    AIT_REG_B   SCAL_CONV_1_UV_22;
    AIT_REG_B   SCAL_CONV_1_RGB_GAIN;
    AIT_REG_B   SCAL_CONV_1_RGB_OFST;
    AIT_REG_B                           _x9E[6];
    #endif
    AIT_REG_B   SCAL_CPU_INT_EN;                                    // 0xA4
    AIT_REG_B   SCAL_HOST_INT_EN;                                   // 0xA5
    AIT_REG_B   SCAL_CPU_INT_SR;                                    // 0xA6
    AIT_REG_B   SCAL_HOST_INT_SR;                                   // 0xA7
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
		#define SCAL0_INT_FRM_END       0x01
		#define SCAL0_INT_INPUT_END     0x02
		#define SCAL0_INT_UP_OVWR       0x04
		#define SCAL0_INT_DBL_FRM_ST    0x08

		#define SCAL1_INT_FRM_END       0x10
		#define SCAL1_INT_INPUT_END     0x20
		#define SCAL1_INT_UP_OVWR       0x40
		#define SCAL1_INT_DBL_FRM_ST    0x80
        #endif
        #if (CHIP == P_V2)
		#define SCAL0_INT_FRM_END	0x01
		#define SCAL1_INT_FRM_END	0x02
		#endif
        /*------------------------------------------------------------*/

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B                           _xA8[8];
    AITS_SCAL_CT            SCAL_P0_COLR_TRANS;                     // 0xB0~0xB7
    AITS_SCAL_CT            SCAL_P1_COLR_TRANS;                     // 0xB8~0xBF
    AITS_SCAL_COLR_CLIP     SCAL_P0_EE_CLIP;                        // 0xC0~0xC7
    AITS_SCAL_COLR_CLIP     SCAL_P1_EE_CLIP;                        // 0xC8~0xCF
    AITS_SCAL_MTX           SCAL_P0_CT_MTX;                         // 0xD0~0xDF
    AITS_SCAL_MTX           SCAL_P1_CT_MTX;                         // 0xE0~0xEF
    AITS_SCAL_COLR_CLIP     SCAL_P0_MTX_CLIP;                       // 0xF0~0xF7
    AITS_SCAL_COLR_CLIP     SCAL_P1_MTX_CLIP;                       // 0xF8~0xFF
    #endif
} AITS_SCAL, *AITPS_SCAL;


#if !defined(BUILD_FW)
// SCAL OPR
#define SCAL_PATH_CTL               (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_PATH_CTL        )))
#define SCAL_GRA_FLOW_CTL_EN        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRA_FLOW_CTL_EN )))
#define SCAL_OUT_0_CTL              (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_OUT_0_CTL       )))
#define SCAL_OUT_1_CTL              (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_OUT_1_CTL       )))
#define SCAL_DNSAMP_LPF_H           (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_LPF_H    )))
#define SCAL_DNSAMP_LPF_V           (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_LPF_V    )))

// ++ P_V1 only
#if (CHIP == P_V2)
#define SCAL_DNSAMP_LPF1_H          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_LPF1_H   )))
#define SCAL_DNSAMP_LPF1_V          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_LPF1_V   )))
#endif
// -- P_V1 only

#define SCAL_DNSAMP_SCAL_0_H        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_SCAL_0_H )))
#define SCAL_DNSAMP_SCAL_0_V        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_SCAL_0_V )))
#define SCAL_DNSAMP_SCAL_1_H        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_SCAL_1_H )))
#define SCAL_DNSAMP_SCAL_1_V        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_SCAL_1_V )))
#define SCAL_GRAB_LPF_H_ST          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF_H_ST)))
#define SCAL_GRAB_LPF_H_ED          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF_H_ED)))
#define SCAL_GRAB_LPF_V_ST          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF_V_ST)))
#define SCAL_GRAB_LPF_V_ED          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF_V_ED)))

#if (CHIP == P_V2)
#define SCAL_GRAB_LPF1_H_ST         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF1_H_ST)))
#define SCAL_GRAB_LPF1_H_ED         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF1_H_ED)))
#define SCAL_GRAB_LPF1_V_ST         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF1_V_ST)))
#define SCAL_GRAB_LPF1_V_ED         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_LPF1_V_ED)))
#endif

#define SCAL_GRAB_SCAL_0_H_ST       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_0_H_ST)))
#define SCAL_GRAB_SCAL_0_H_ED       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_0_H_ED)))
#define SCAL_GRAB_SCAL_0_V_ST       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_0_V_ST)))
#define SCAL_GRAB_SCAL_0_V_ED       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_0_V_ED)))
#define SCAL_GRAB_OUT_0_H_ST        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_0_H_ST )))
#define SCAL_GRAB_OUT_0_H_ED        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_0_H_ED )))
#define SCAL_GRAB_OUT_0_V_ST        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_0_V_ST )))
#define SCAL_GRAB_OUT_0_V_ED        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_0_V_ED )))
#define SCAL_SCAL_0_CTL		        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_CTL      )))
#define SCAL_SCAL_1_CTL     		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_CTL 	   )))

#if (CHIP == P_V2)    
#define SCAL_SCAL_0_H_M	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_H_M     )))
#define SCAL_SCAL_0_H_N    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_H_N 	   )))
#define SCAL_SCAL_0_H_WT	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_H_WT     )))
#define SCAL_SCAL_1_H_M    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_H_M 	   )))
#define SCAL_SCAL_1_H_N	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_H_N     )))
#define SCAL_SCAL_1_H_WT    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_H_WT 	   )))
#define SCAL_SCAL_0_V_M	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_V_M     )))
#define SCAL_SCAL_0_V_N    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_V_N 	   )))
#define SCAL_SCAL_0_V_WT	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_0_V_WT     )))
#define SCAL_SCAL_1_V_M    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_V_M 	   )))
#define SCAL_SCAL_1_V_N	        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_V_N     )))
#define SCAL_SCAL_1_V_WT    		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_SCAL_1_V_WT 	   )))
#endif

#define SCAL_EDGE_0_CTL		        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_EDGE_0_CTL      )))
#define SCAL_EDGE_1_CTL	     		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_EDGE_1_CTL 	   )))
#define SCAL_DNSAMP_OUT_0_H         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_OUT_0_H  )))
#define SCAL_DNSAMP_OUT_0_V         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_OUT_0_V  )))
#define SCAL_LPF_SCAL_CTL           (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_LPF_SCAL_CTL    )))
#define SCAL_LPF_CTL                (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_LPF_CTL         )))

#if (CHIP == P_V2)
#define SCAL_LPF1_SCAL_CTL          (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_LPF1_SCAL_CTL    )))
#define SCAL_LPF1_CTL               (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_LPF1_CTL         )))
#endif

#define SCAL_LPF_AUTO_CTL           (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_LPF_AUTO_CTL    )))
#define SCAL_GRAB_SCAL_1_H_ST       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_1_H_ST)))
#define SCAL_GRAB_SCAL_1_H_ED       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_1_H_ED)))
#define SCAL_GRAB_SCAL_1_V_ST       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_1_V_ST)))
#define SCAL_GRAB_SCAL_1_V_ED       (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_SCAL_1_V_ED)))
#define SCAL_GRAB_OUT_1_H_ST        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_1_H_ST )))
#define SCAL_GRAB_OUT_1_H_ED        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_1_H_ED )))
#define SCAL_GRAB_OUT_1_V_ST        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_1_V_ST )))
#define SCAL_GRAB_OUT_1_V_ED        (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_GRAB_OUT_1_V_ED )))
#define SCAL_DNSAMP_OUT_1_H         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_OUT_1_H  )))
#define SCAL_DNSAMP_OUT_1_V         (SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_DNSAMP_OUT_1_V  )))

#define SCAL_CPU_INT_EN        		(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_CPU_INT_EN )))
#define SCAL_HOST_INT_EN        	(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_HOST_INT_EN )))
#define SCAL_CPU_INT_SR         	(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_CPU_INT_SR  )))
#define SCAL_HOST_INT_SR         	(SCAL_BASE+(MMP_ULONG)(&(((AITPS_SCAL)0)->SCAL_HOST_INT_SR  )))

#endif

/// @}

#endif // _MMPH_REG_SCALER_H_
