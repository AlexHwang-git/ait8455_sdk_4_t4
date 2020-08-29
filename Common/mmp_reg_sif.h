//==============================================================================
//
//  File        : mmp_reg_spi.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_SIF_H_
#define _MMP_REG_SIF_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/

#if	(CHIP==P_V2)||(CHIP == VSN_V2)||(CHIP == VSN_V3)
// *******************************
//   SPI structure (0x8000 CC00)
// *******************************
typedef struct _AITS_SIF {
    AIT_REG_B   SIF_INT_CPU_EN;
    AIT_REG_B   _x01[3];
    AIT_REG_B   SIF_INT_HOST_EN;
    AIT_REG_B   _x05[3];
    AIT_REG_B   SIF_INT_CPU_SR;
    AIT_REG_B   _x09[3];
    AIT_REG_B   SIF_INT_HOST_SR;
    AIT_REG_B   					_x0D[3];
        /*-DEFINE-----------------------------------------------------*/
        #define SIF_EN               0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define SIF_START               0x80
        #define SIF_PROGRAM_EN          0x40
        #define SIF_R           		0x20
        #define SIF_W           		0x00
        #define SIF_DMA_EN           	0x10
        #define SIF_DATA_EN           	0x08
        #define SIF_ADDR_EN           	0x04
        #define SIF_ADDR_LEN_2        	0x02
        #define SIF_ADDR_LEN_1        	0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_POL_PHA;
        /*-DEFINE-----------------------------------------------------*/
    	#define SIF_POL_HIGH            0x0
    	#define SIF_POL_LOW             0x2
		#define SIF_PHA_0               0x0
		#define SIF_PHA_1               0x1
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_CLK_DIV;
    AIT_REG_B   SIF_PD_CPU;
    AIT_REG_B   SIF_CMD;
    AIT_REG_B   					_x15[3];
	AIT_REG_D   SIF_FRESH_ADDR;
	AIT_REG_D   					_x1C;
	AIT_REG_B   SIF_DATA_WR;
	AIT_REG_B   					_x21[15];
	AIT_REG_B   SIF_DATA_RD;
	AIT_REG_B   					_x31[15];
	AIT_REG_D   SIF_DMA_ST;
	AIT_REG_W   SIF_DMA_CNT;
	AIT_REG_W   					_x46[5];
	AIT_REG_B   SIF_TSC_EN;
	AIT_REG_B   					_x51[3];
	AIT_REG_B   SIF_TSC_RW;
	AIT_REG_B   					_x55[3];
	AIT_REG_B   SIF_TSC_SEP;
} AITS_SIF, *AITPS_SIF;

#if	!defined(BUILD_FW)
// SIF OPR
#define SIF_INT_CPU_EN			(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_CPU_EN      )))
#define SIF_INT_HOST_EN		    (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_HOST_EN     )))
#define SIF_INT_CPU_SR			(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_CPU_SR	      )))
#define SIF_INT_HOST_SR	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_HOST_SR    )))
#define SIF_CTL			        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CTL   )))
#define SIF_POL_PHA	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_POL_PHA  )))
#define SIF_CLK_DIV  	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CLK_DIV  )))

#define SIF_PD_CPU	           	(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_PD_CPU  )))
#define SIF_CMD	            	(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CMD  )))
#define SIF_FRESH_ADDR	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_FRESH_ADDR )))
#define SIF_DATA_WR	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DATA_WR )))
#define SIF_DATA_RD	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DATA_RD  )))
#define SIF_DMA_ST	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DMA_ST  )))
#define SIF_DMA_CNT	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DMA_CNT  )))
#define SIF_TSC_EN	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_TSC_EN  )))
#define SIF_TSC_RW	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_TSC_RW  )))
#define SIF_TSC_SEP	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_TSC_SEP  )))
#endif
/// @}
#endif
#endif // _MMPH_REG_SIF_H_