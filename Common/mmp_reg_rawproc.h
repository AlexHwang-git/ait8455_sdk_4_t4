//==============================================================================
//
//  File        : mmp_reg_rawproc.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Rogers Chen
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_RAWPROC_H_
#define _MMP_REG_RAWPROC_H_

#include    "mmp_register.h"


//--------------------------------
// RAWPROC structure (0x8000 C400)
//--------------------------------
typedef struct _AITS_RAWPROC {
    AIT_REG_W   RAWPROC_F_H_BYTE;                                       // 0x00
    AIT_REG_W   RAWPROC_F_V_BYTE;                                       // 0x02
    AIT_REG_D   RAWPROC_F_ADDR;                                         // 0x04
    AIT_REG_D   RAWPROC_F_ST_OFST;                                      // 0x08
    AIT_REG_W   RAWPROC_F_PIX_OFST;                                     // 0x0C
    AIT_REG_W   RAWPROC_F_LINE_OFST;                                    // 0x0E
    AIT_REG_B   RAWPROC_MODE_SEL;                                       // 0x10
    	/*-DEFINE-----------------------------------------------------*/
        #define RAWPROC_FETCH_EN   	0x01
        #define RAWPROC_STORE_EN   	0x02
        #define RAWPROC_1PIXELTWOBYTE_MODE  0x04
        #define RAWPROC_F_BURST_EN  0x08
        #define RAWPROC_10BIT_MODE  0x40
        #define RAWPROC_12BIT_MODE  0x80
        /*------------------------------------------------------------*/
    AIT_REG_B   RAWPROC_F_TIME_CTL;                                     // 0x11
    AIT_REG_W   									_x12;               // 0x12
    AIT_REG_D   RAWPROC_S_ADDR;                                         // 0x14
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_B   RAWPROC_S_DNSAMP_H;                                     // 0x28
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B   RAWPROC_S_SAMP_DN;                                      // 0x18
    #endif
    AIT_REG_B   RAWPROC_SRC_SEL;                                        // 0x19
    		/*-DEFINE-----------------------------------------------------*/
    	    #define RAWPROC_SRC_VIF         0x00
    	    #define RAWPROC_SRC_ISP         0x04
    	    #define RAWPROC_SRC_ISP_SHAD    0x00
    	    #define RAWPROC_SRC_ISP_DPCBNR  0x01
    	    #define RAWPROC_SRC_ISP_WDR     0x02
    	    #define RAWPROC_SRC_ISP_RGB2BAYER 0x03
        	/*------------------------------------------------------------*/

    #if (CHIP == VSN_V3)
    AIT_REG_W                                       _x1A;
    #endif
    #if (CHIP == P_V2) || (CHIP == VSN_V2)
    AIT_REG_W   RAWPROC_F_LINE_TIME;                                    // 0x1A
    #endif

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_D   RAWPROC_S_BASE_END_ADDR;                                // 0x1C
    #endif
    #if (CHIP == P_V2)
    AIT_REG_D   									_x1C;               // 0x1C
    #endif
	__packed union {
		__packed struct {
			AIT_REG_B   RAWPROC_S_IDLE_SR;                  			// 0x20
    		/*-DEFINE-----------------------------------------------------*/
    	    #define BAYER_STORE_IDLE	0x01
        	#define LUMA_STORE_IDLE  	0x02
	        #define RAWPROC_STORE_IDLE  0x03
        	/*------------------------------------------------------------*/
			AIT_REG_B   		                  	_x21[3];
		} V0;
		// ++ P_V1 only
		__packed struct {
			AIT_REG_B   RAWPROC_HOST_INT_EN;                  			// 0x20
			AIT_REG_B   RAWPROC_HOST_INT_SR;                  			// 0x21
			AIT_REG_B   RAWPROC_CPU_INT_EN;                  			// 0x22
			AIT_REG_B   RAWPROC_CPU_INT_SR;                  			// 0x23
    		/*-DEFINE-----------------------------------------------------*/
    	    #define BAYER_STORE_DONE	0x01
        	#define LUMA_STORE_DONE  	0x02
        	/*------------------------------------------------------------*/
		} V1;
        // -- P_V1 only
	} RAWPROC_S_SR;

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    AIT_REG_W   RAWPROC_F_PIXL_CNT;                                     // 0x24
    AIT_REG_W   RAWPROC_F_LINE_CNT;                                     // 0x26
    AIT_REG_B   RAWPROC_S_DNSAMP_V;                                     // 0x28
    AIT_REG_B                                       _x29[0x3];
    AIT_REG_B   RAWPROC_S_GRAB_EN;                                      // 0x2C
    AIT_REG_B                                       _x2D[0x3];
    AIT_REG_W   RAWPROC_S_GRAB_PIXL_ST;                                 // 0x30
    AIT_REG_W   RAWPROC_S_GRAB_PIXL_ED;                                 // 0x32
    AIT_REG_W   RAWPROC_S_GRAB_LINE_ST;                                 // 0x34
    AIT_REG_W   RAWPROC_S_GRAB_LINE_ED;                                 // 0x36
    AIT_REG_B                                       _x38[0x18];
    #if (CHIP == VSN_V3)
    AIT_REG_W   RAWPROC_F_LINE_TIME;                                    // 0x50
    AIT_REG_W                                       _x52;
    AIT_REG_W   RAWPROC_F_FRONT_LINE_TIME;                              // 0x54
    AIT_REG_B   RAWPROC_F_FRONT_LINE_NUM;                               // 0x56
    AIT_REG_B                                       _x57;
    AIT_REG_B   RAWPROC_F_BUSY_MODE_CTL;                                // 0x58
    	/*-DEFINE-----------------------------------------------------*/
        #define RAWPROC_BUSY_MODE_EN    0x01
        /*------------------------------------------------------------*/
    #endif
    #endif
    #if (CHIP == P_V2)
    AIT_REG_B                                       _x24[0x5C];

    AIT_REG_B   RAWPROC_LUMA_MODE;                                      // 0x80
    	/*-DEFINE-----------------------------------------------------*/
        #define RAWPROC_LUMA_EN    0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   RAWPROC_LUMA_CTL;                                       // 0x81
    AIT_REG_B   RAWPROC_LUMA_SAMP_DN;                                   // 0x82
    AIT_REG_B   									_x83;
    AIT_REG_D   RAWPROC_LUMA_ADDR;                                      // 0x84
    #endif

} AITS_RAWPROC, *AITPS_RAWPROC;


#if !defined(BUILD_FW)
// RAWPROC OPR
#define RAWPROC_F_H_BYTE        (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_H_BYTE    )))
#define RAWPROC_F_V_BYTE        (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_V_BYTE    )))
#define RAWPROC_F_ADDR          (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_ADDR      )))
#define RAWPROC_F_ST_OFST       (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_ST_OFST   )))
#define RAWPROC_F_PIX_OFST      (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_PIX_OFST  )))
#define RAWPROC_F_LINE_OFST     (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_LINE_OFST )))
#define RAWPROC_MODE_SEL        (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_MODE_SEL    )))
#define RAWPROC_F_TIME_CTL      (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_TIME_CTL  )))
#define RAWPROC_S_ADDR          (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_S_ADDR      )))
#define RAWPROC_S_SAMP_DN       (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_S_SAMP_DN   )))
#define RAWPROC_SRC_SEL         (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_SRC_SEL     )))
#define RAWPROC_F_LINE_TIME     (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_F_LINE_TIME )))
#define RAWPROC_S_IDLE_SR       (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_S_SR.V0.RAWPROC_S_IDLE_SR   )))
#define RAWPROC_LUMA_MODE       (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_LUMA_MODE   )))
#define RAWPROC_LUMA_CTL        (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_LUMA_CTL    )))
#define RAWPROC_LUMA_SAMP_DN    (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_LUMA_SAMP_DN)))
#define RAWPROC_LUMA_ADDR       (RAWPROC_BASE+(MMP_ULONG)(&(((AITPS_RAWPROC)0)->RAWPROC_LUMA_ADDR   )))
#endif


#endif //_MMPH_REG_RAWPROC_H_

