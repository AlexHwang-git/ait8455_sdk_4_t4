//==============================================================================
//
//  File        : mmp_reg_icon.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_ICON_H_
#define _MMP_REG_ICON_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/
// *******************************
//   ICO structure (0x8000 6C00)
// *******************************
#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
typedef struct _AITS_ICOB {
    AIT_REG_B   ICO_DLINE_CFG[3];                    //0x00
        /*-DEFINE-----------------------------------------------------*/
        #define ICO_DLINE_BYPASS            0x01
        #define ICO_DLINE_SRC_SEL_MASK      0x06
        #define ICO_DLINE_SRC_SEL(_a)       ((_a << 1) & ICO_DLINE_SRC_SEL_MASK)
        #define ICO_DLINE_DIS               0x08
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x03;
    AIT_REG_W   ICO_DLINE_MAX_USE[3];               //0x04
    AIT_REG_W                           _x0A;
    AIT_REG_B   ICO_DLINE_SRAM_CFG;                 //0x0C
} AITS_ICOB, *AITPS_ICOB;
#endif

#if (CHIP == P_V2)
typedef struct _AITS_ICOD {
    AIT_REG_D   ICO_ADDR_ST;                        //0x00
    AIT_REG_W   ICO_X0;                             //0x04
    AIT_REG_W   ICO_Y0;                             //0x06
    AIT_REG_W   ICO_X1;                             //0x08
    AIT_REG_W   ICO_Y1;                             //0x0A
    AIT_REG_W   ICO_TP_COLR;                        //0x0C
    AIT_REG_B   ICO_TP_COLR_MSB;                    //0x0E
    AIT_REG_B   ICO_SEMITP_WT;                      //0x0F
    AIT_REG_W   ICO_CTL;                            //0x10
        /*-DEFINE-----------------------------------------------------*/
        #define ICO_TP_EN                   0x0001
        #define ICO_SEMITP_EN               0x0002
        #define ICO_FMT_8BPP                0x0004
        #define ICO_FMT_RGB565              0x0000
        #define ICO_FMT_MASK                0x0004
        #define ICO_8BPP_BGR_EN             0x0008
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x32[7];
} AITS_ICOD, *AITPS_ICOD;
typedef struct _AITS_ICO {
    AIT_REG_D   ICO_ADDR_ST;                        //0x00
    AIT_REG_W   ICO_X0;                             //0x04
    AIT_REG_W   ICO_Y0;                             //0x06
    AIT_REG_W   ICO_X1;                             //0x08
    AIT_REG_W   ICO_Y1;                             //0x0A
    AIT_REG_W   ICO_TP_COLR;                        //0x0C
    AIT_REG_B   ICO_SEMITP_WT;                      //0x0E
    AIT_REG_B   ICO_DST_WT;                         //0x0F
    AIT_REG_W   ICO_CTL;                            //0x10
        /*-DEFINE-----------------------------------------------------*/
        #define ICO_TP_EN                   0x0001
        #define ICO_SEMITP_EN               0x0002
        #define ICO_FMT_8BPP                0x0004
        #define ICO_FMT_RGB565              0x0000
        #define ICO_FMT_MASK                0x0004
        #define ICO_8BPP_BGR_EN             0x0008
        /*------------------------------------------------------------*/
    AIT_REG_W                           _x32[7];
} AITS_ICO, *AITPS_ICO;
// *******************************
//   ICO structure (0x8000 7C00)
// *******************************
typedef struct _AITS_ICOB {
    AIT_REG_W   ICO_EN;                                                 // 0x00
    AIT_REG_B   ICO_ALPHA_CTL;																					// 0x02
        /*-DEFINE-----------------------------------------------------*/
		#define ICO_ALPHA_RGBA_FMT            0x10
		#define ICO_ALPHA_ARGB_FMT            0x00
		#define ICO_ALPHA_FMT_MASK            0x10
    	#define ICO_FORMAT_RGB565             0x00
    	#define ICO_FORMAT_ARGB3454           0x04
    	#define ICO_FORMAT_ARGB4444           0x08
    	#define ICO_FORMAT_ARGB8888           0x0C
    	#define ICO_FORMAT_MASK               0x0C
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x03;
    AIT_REG_B   ICO_H_SCALUP_CTL;                                       //[SPEC] By CYKO, although the spec is [3:0],
        /*-DEFINE-----------------------------------------------------*/
        #define ICO_FIFO_1_RST     			0x08
        #define ICO_FIFO_0_RST     			0x04
        #define ICO_JPG_SCALUP_EN			0x02
        #define ICO_DSPY_SCALUP_EN			0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   ICO_SRAM_CTL;																						// 0x05
    AIT_REG_B   ICO_PIPE0_CTL;																					// 0x06
    AIT_REG_B   ICO_PIPE1_CTL;																				  // 0x07    
        /*-DEFINE-----------------------------------------------------*/
        #define ICO_USE_DLINE   				0x00
        #define ICO_BYPASS_DLINE   			0x01
        #define ICO_SCAL_INPUT_DIS 			0x04
        #define ICO0_SEL_SCAL_0    			0x00
        #define ICO0_SEL_SCAL_1    			0x02
        #define ICO1_SEL_SCAL_0    			0x02
        #define ICO1_SEL_SCAL_1    			0x00
        /*------------------------------------------------------------*/ 
    AIT_REG_W   ICO_SR;																									 // 0x08
    AIT_REG_W   ICO_PIP0_DLINE_SR;																			 // 0x0A
    AIT_REG_W   ICO_PIP1_DLINE_SR;																			 // 0x0B
    AIT_REG_W                           _x0E;
    AIT_REG_W                           _x10[8];                        // 0x10
    AITS_ICOD   ICOD[8];
    AITS_ICO    ICO[2];
} AITS_ICOB, *AITPS_ICOB;
#endif

#if	!defined(BUILD_FW)
// ICON OPR
#if (CHIP == P_V2)
#define ICON0_START_ADDR            (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_ADDR_ST     )))
#define ICON0_START_X               (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_X0          )))
#define ICON0_START_Y               (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_Y0          )))
#define ICON0_END_X                 (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_X1          )))
#define ICON0_END_Y                 (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_Y1          )))
#define ICON0_TP_COLOR              (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_TP_COLR     )))
// display
#define ICOND0_TP_COLR_MSB           (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_TP_COLR_MSB )))
#define ICOND0_SEMITP_ICO_WT         (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_SEMITP_WT   )))
#define ICOND0_CTL                   (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICOD[0].ICO_CTL         )))
// jpeg
#define ICONJ0_SEMITP_WT             (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO[0].ICO_SEMITP_WT   )))
#define ICONJ0_DST_WT                (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO[0].ICO_DST_WT      )))
#define ICONJ0_CTL                   (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO[0].ICO_CTL         )))
#define ICON_EN                     (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_EN                 )))
#define ICON_SR                     (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_SR                 )))
#define ICO_H_SCALUP_CTL            (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_H_SCALUP_CTL       )))
#define ICO_PIPE1_CTL               (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_PIPE1_CTL          )))
#define ICO_PIPE0_CTL               (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_PIPE0_CTL          )))
#define ICO_ALPHA_CTL               (ICO_BASE +(MMP_ULONG)(&(((AITPS_ICOB )0)->ICO_ALPHA_CTL          )))
#endif
#endif

/// @}
#endif // _MMPH_REG_ICON_H_
