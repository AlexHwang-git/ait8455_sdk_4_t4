//==============================================================================
//
//  File        : mmp_reg_sif.h
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

// *******************************
//   SIF structure (0x8000 6700)
// *******************************
typedef struct _AITS_SIF {
    AIT_REG_B   SIF_INT_CPU_EN;					//0x00
    AIT_REG_B   _x01[3];
        /*-DEFINE-----------------------------------------------------*/
        // Refer to Offset 0x0C
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_INT_HOST_EN;				//0x04
    AIT_REG_B   _x05[3];
        /*-DEFINE-----------------------------------------------------*/
        // Refer to Offset 0x0C
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_INT_CPU_SR;					//0x08
    AIT_REG_B   _x09[3];
        /*-DEFINE-----------------------------------------------------*/
        // Refer to Offset 0x0C
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_INT_HOST_SR;				//0x0C
    AIT_REG_B	_x0D[3];
        /*-DEFINE-----------------------------------------------------*/
        #define SIF_CMD_DONE				0x01
        #define SIF_AAI_CMD_DONE			0x02
        #define SIF_CLR_CMD_STATUS			0x01
        #define SIF_CLR_AAI_CMD_STATUS		0x02
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_CTL;						//0x10
        /*-DEFINE-----------------------------------------------------*/
        #define SIF_START               	0x80
        #define SIF_FAST_READ         	 	0x40
        #define SIF_R           			0x20
        #define SIF_W           			0x00
        #define SIF_DMA_EN           		0x10
        #define SIF_DATA_EN           		0x08
        #define SIF_ADDR_EN           		0x04
        #define SIF_ADDR_LEN_2        		0x02
        #define SIF_ADDR_LEN_1        		0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_CTL2;					//0x11
        /*-DEFINE-----------------------------------------------------*/
    	#define SIF_AUTO_LOAD_MASK_DIS     	0x00
    	#define SIF_AUTO_LOAD_MASK_EN		0x02
		#define SIF_AAI_MODE_DIS        	0x00
		#define SIF_AAI_MODE_EN         	0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SIF_CLK_DIV;					//0x12
    	// Target clock rate = source clock rate / 2(SIF_CLK_DIV + 1)
    	/*-DEFINE-----------------------------------------------------*/
    	#define SIF_SET_CLK_DIV(_a)     		(_a/2 - 1)
    	/*------------------------------------------------------------*/
    AIT_REG_B   SIF_PD_CPU;						//0x13
    AIT_REG_B   SIF_CMD;						//0x14
    AIT_REG_B   					_x15[3];
	AIT_REG_D   SIF_FLASH_ADDR;					//0x18
	#if (CHIP == VSN_V2)
	AIT_REG_D   					_x1C;		//0x1C~0x1F
	#endif
	#if (CHIP == VSN_V3)
	AIT_REG_B	SIF_BIST_EN;					//0x1C
		/*-DEFINE-----------------------------------------------------*/
    	#define SIF_BIST_ENABLE     		0x01
    	/*------------------------------------------------------------*/
	AIT_REG_B	SIF_DATA_IN_LATCH;				//0x1D
	AIT_REG_W	SIF_CRC_OUT;					//0x1E
	#endif
	AIT_REG_B   SIF_DATA_WR;					//0x20
	#if (CHIP == VSN_V2)
	AIT_REG_B   					_x21[15];
	#endif
	#if (CHIP == VSN_V3)
	AIT_REG_B	SIF_DATA_WR1;					//0x21
	AIT_REG_B   					_x22[14];
	#endif
	AIT_REG_B   SIF_DATA_RD;					//0x30
	AIT_REG_B   					_x31[15];
	AIT_REG_D   SIF_DMA_ST;						//0x40
	AIT_REG_D   SIF_DMA_CNT;					//0x44
	AIT_REG_B   SIF_AAI_INTER_CMD_DELAY;		//0x48
} AITS_SIF, *AITPS_SIF;

#if	!defined(BUILD_FW)
// SIF OPR
#define SIF_INT_CPU_EN			(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_CPU_EN )))
#define SIF_INT_HOST_EN		    (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_HOST_EN )))
#define SIF_INT_CPU_SR			(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_CPU_SR )))
#define SIF_INT_HOST_SR	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_INT_HOST_SR )))
#define SIF_CTL			        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CTL )))
#define SIF_CTL2	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CTL2 )))
#define SIF_CLK_DIV  	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CLK_DIV )))

#define SIF_PD_CPU	           	(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_PD_CPU )))
#define SIF_CMD	            	(SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_CMD )))
#define SIF_FLASH_ADDR	        (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_FLASH_ADDR )))
#define SIF_DATA_WR	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DATA_WR )))
#define SIF_DATA_RD	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DATA_RD )))
#define SIF_DMA_ST	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DMA_ST )))
#define SIF_DMA_CNT	            (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_DMA_CNT )))
#define SIF_AAI_INTER_CMD_DELAY (SIF_BASE +(MMP_ULONG)(&(((AITPS_SIF )0)->SIF_AAI_INTER_CMD_DELAY )))
#endif
/// @}
#endif // _MMPH_REG_SIF_H_