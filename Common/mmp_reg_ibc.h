//==============================================================================
//
//  File        : mmp_register_ibc.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_IBC_H_
#define _MMP_REG_IBC_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

// ********************************
//   IBCP structure (0x8000 9020)
// ********************************
typedef struct _AITS_IBCP {
    //for IBC pipe-0 and pipe-1   
    AIT_REG_D   IBC_ADDR_Y_ST;                                          // 0x20
    AIT_REG_D   IBC_ADDR_U_ST;                                          // 0x24
    AIT_REG_D   IBC_ADDR_V_ST;                                          // 0x28

    #if (CHIP == VSN_V3)
    AIT_REG_W   IBC_FRM_WIDTH;                                          // 0x2C
    AIT_REG_W                           _x2E;
    AIT_REG_B   IBC_BUF_COLR_FMT_CTL;                                   // 0x30
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_420_FMT_NV21_EN     0x01
       /*------------------------------------------------------------*/
    AIT_REG_B                           _x31[0xF];
    #endif
    #if (CHIP == VSN_V2)
    AIT_REG_W   IBC_FRM_WIDTH;                                          // 0x2C
    AIT_REG_B                           _x2E[0x12];
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B                           _x2C[4];    
    AIT_REG_B   IBC_422_OUTPUT_SEQ;    
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_422_YUYV     0x00
        #define IBC_422_YVYU     0x01
        #define IBC_422_UYVY     0x10
        #define IBC_422_VYUY     0x11
       /*------------------------------------------------------------*/
    AIT_REG_B                           _x31[15];    
    #endif

    AIT_REG_B   IBC_SRC_SEL;                                            // 0x40
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_SRC_YUV420          0x01
        #define IBC_SRC_SEL_MASK        0x06
        #define IBC_SRC_SEL_ICO(_a)     ((_a << 1) & IBC_SRC_SEL_MASK)
        #define IBC_420_STORE_CBR       0x08
        #define IBC_LINEOFFSET_EN       0x10
        #define IBC_NV12_EN             0x20
        #define IBC_422_SEQ_MASK        0x40
        #define IBC_422_SEQ_UYVY        0x00
        #define IBC_422_SEQ_YUYV        0x40
        #define IBC_RING_ADDR_RST_DIS   0x80
        /*------------------------------------------------------------*/
        #endif
        #if (CHIP == P_V2)
        #define IBC_SRC_RGB565           0x00
        #define IBC_SRC_YUV422           0x01
        #define IBC_SRC_RGB888           0x02
        #define IBC_SRC_YUV420           0x04
        #define IBC_420_STORE_LUMI_ONLY  0x00        
        #define IBC_420_STORE_ALL        0x10        
        #define IBC_FROM_ICON0           0x00
        #define IBC_FROM_ICON1           0x08

        #define IBC_CONNECT_ICON(_m)         (_m<<3)

        #define IBC_LINEOFFSET_EN        0x20        
        #define IBC_INTERLACE_MODE_EN    0x40        
        #define IBC_SRC_MASK             0x7F
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x41;     
    AIT_REG_W                           _x42;
    AIT_REG_B   IBC_BUF_CFG;										    // 0x44
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_STORE_SING_FRAME    0x01
        #define IBC_STORE_PIX_CONT      0x02
        #define IBC_ICON_PAUSE_EN       0x04
        #define IBC_STORE_EN            0x08
        #define IBC_INT_STOP_STORE_EN   0x10
        #define IBC_RING_BUF_EN         0x20
        #define IBC_BT601_EN            0x40
        #define IBC_MIRROR_EN           0x80
        /*------------------------------------------------------------*/
        #endif
        #if (CHIP == P_V2)
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_STORE_SING_FRAME    0x01
        #define IBC_STORE_PIX_CONT      0x02
        #define IBC_STORE_PIX_NCONT     0x00
        #define IBC_ICON_PAUSE_EN       0x04
        #define IBC_STORE_EN            0x08
        #define IBC_INT_STOP_STORE_EN   0x10
        #define IBC_YUV_LP121_EN        0x20
        #define IBC_YUV_LP11_EN         0x40
        // ++ P_65C only
        #define IBC_RING_BUF_EN         0x80
        // -- P_65C only
        /*------------------------------------------------------------*/
        #endif
    AIT_REG_B                           _x45;     
    AIT_REG_W                           _x46;
    AIT_REG_W   IBC_INT_LINE_CNT;                                       // 0x48
    AIT_REG_B                           _x4A[2];
    AIT_REG_D   IBC_LINE_OFST;                                          // 0x4C
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_D   IBC_CBR_LINE_OFST;                                      // 0x50
    AIT_REG_D                           _x54;
    AIT_REG_D   IBC_WR_PTR;                                             // 0x58
    AIT_REG_D                           _x5C;                           // 0x5C~0x5F
    // total : 0x60 - 0x20 = 0x40
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B                           _x50[0xD0];
    #endif
} AITS_IBCP, *AITPS_IBCP;

// ********************************
//   IBC structure (0x8000 9000)
// ********************************

typedef struct _AITS_IBC {
    AIT_REG_B   IBC_IMG_PIPE_CTL;                                       // 0x00
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define IBC_H264_SRC_SEL_MASK   0x03
        #define IBC_H264_SRC_SEL(_a)    (_a & IBC_H264_SRC_SEL_MASK)
        #endif
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x01;     
    AIT_REG_W                           _x02;     
    AIT_REG_B   IBC_JPEG_PIPE_SEL;                                      // 0x04
        /*-DEFINE-----------------------------------------------------*/
        #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
        #define IBC_JPEG_SRC_SEL_MASK   0x03
        #define IBC_JPEG_SRC_SEL(_a)    (_a & IBC_JPEG_SRC_SEL_MASK)
        #define IBC_THUMB_SRC_SEL_MASK  0x0C
        #define IBC_THUMB_SRC_SEL(_a)   ((_a << 2) & IBC_THUMB_SRC_SEL_MASK)
        #define IBC_THUMB_ENC_EN        0x10
        #endif
        #if (CHIP == P_V2)
        #define IBC_JPEG_P0_ENC          0x00
        #define IBC_JPEG_P1_ENC          0x01
        #endif
        /*------------------------------------------------------------*/  
    AIT_REG_B                           _x05;     
    AIT_REG_W                           _x06;
    AIT_REG_D                           _x08[2];   
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   IBC_P0_INT_CPU_EN;                                      // 0x10
    AIT_REG_B   IBC_P1_INT_CPU_EN;                                      // 0x11
    AIT_REG_B   IBC_P2_INT_CPU_EN;                                      // 0x12
    AIT_REG_B                           _x13;
    AIT_REG_B   IBC_P0_INT_HOST_EN;                                     // 0x14
    AIT_REG_B   IBC_P1_INT_HOST_EN;                                     // 0x15
    AIT_REG_B   IBC_P2_INT_HOST_EN;                                     // 0x16
    AIT_REG_B                           _x17;
    AIT_REG_B   IBC_P0_INT_CPU_SR;                                      // 0x18
    AIT_REG_B   IBC_P1_INT_CPU_SR;                                      // 0x19
    AIT_REG_B   IBC_P2_INT_CPU_SR;                                      // 0x1A
    AIT_REG_B                           _x1B;
    AIT_REG_B   IBC_P0_INT_HOST_SR;                                     // 0x1C
    AIT_REG_B   IBC_P1_INT_HOST_SR;                                     // 0x1D
    AIT_REG_B   IBC_P2_INT_HOST_SR;                                     // 0x1E
    AIT_REG_B                           _x1F;
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_INT_FRM_RDY         0x01
        #define IBC_INT_FRM_ST          0x02
        #define IBC_INT_FRM_END         0x04
        #define IBC_INT_PRE_FRM_RDY     0x08
        #define IBC_INT_ALL             0x0F
        /*------------------------------------------------------------*/
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   IBC_P0_INT_CPU_EN;                                      // 0x10
    AIT_REG_B   IBC_P1_INT_CPU_EN;                                      // 0x11
    AIT_REG_B   IBC_P0_INT_HOST_EN;                                     // 0x12
    AIT_REG_B   IBC_P1_INT_HOST_EN;                                     // 0x13
    AIT_REG_B   IBC_P0_INT_CPU_SR;                                      // 0x14
    AIT_REG_B   IBC_P1_INT_CPU_SR;                                      // 0x15
    AIT_REG_B   IBC_P0_INT_HOST_SR;                                     // 0x16
    AIT_REG_B   IBC_P1_INT_HOST_SR;                                     // 0x17
        /*-DEFINE-----------------------------------------------------*/
        #define IBC_INT_FRM_RDY         0x01
        #define IBC_INT_FRM_ST          0x02
        #define IBC_INT_FRM_END         0x04
        #define IBC_INT_PRE_FRM_RDY     0x08
        #define IBC_INT_ALL             0x0F
        /*------------------------------------------------------------*/
    AIT_REG_D                           _x18[2];     
    #endif

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AITS_IBCP   IBCP_0;                                                 // 0x20~0x60
    AIT_REG_B                           _x60[0x20];
    AITS_IBCP   IBCP_1;                                                 // 0x80~0xC0
    AITS_IBCP   IBCP_2;                                                 // 0xC0~
    #endif
    #if (CHIP == P_V2)
    AITS_IBCP   IBCP[2];
    #endif
} AITS_IBC, *AITPS_IBC;

#if !defined(BUILD_FW)
// IBC OPR
#define IBC_IMG_PIPE_CTL        (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_IMG_PIPE_CTL  )))
#define IBC_JPEG_PIPE_SEL       (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_JPEG_PIPE_SEL )))
#define IBC_P0_INT_CPU_EN       (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P0_INT_CPU_EN )))
#define IBC_P1_INT_CPU_EN       (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P1_INT_CPU_EN )))
#define IBC_P0_INT_HOST_EN      (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P0_INT_HOST_EN)))
#define IBC_P1_INT_HOST_EN      (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P1_INT_HOST_EN)))
#define IBC_P0_INT_CPU_SR       (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P0_INT_CPU_SR )))
#define IBC_P1_INT_CPU_SR       (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P1_INT_CPU_SR )))
#define IBC_P0_INT_HOST_SR      (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P0_INT_HOST_SR)))
#define IBC_P1_INT_HOST_SR      (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBC_P1_INT_HOST_SR)))

#if (CHIP == P_V2)
#define IBCP0_ADDR_Y_ST         (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_ADDR_Y_ST     )))
#define IBCP0_ADDR_U_ST         (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_ADDR_U_ST     )))
#define IBCP0_ADDR_V_ST         (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_ADDR_V_ST     )))
#define IBCP0_SRC_SEL           (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_SRC_SEL       )))
#define IBCP0_BUF_CFG           (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_BUF_CFG       )))
#define IBCP0_INT_LINE_CNT      (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_INT_LINE_CNT)))
#define IBCP0_ADDR_Y_END        (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_ADDR_U_ST     )))
#define IBCP0_422_OUTPUT_SEQ    (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_422_OUTPUT_SEQ     )))
#define IBCP0_LINE_OFST         (IBC_BASE +(MMP_ULONG)(&(((AITPS_IBC )0)->IBCP[0].IBC_LINE_OFST     )))
#endif

#endif

/// @}
#endif // _MMPH_REG_IBC_H_
