//==============================================================================
//
//  File        : mmp_reg_spi.h
//  Description : INCLUDE File for the Retina register map.
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================



#ifndef _MMP_REG_SPI_H_
#define _MMP_REG_SPI_H_

#include    "mmp_register.h"

/** @addtogroup MMPH_reg
@{
*/


// *******************************
//   SPI structure (0x8000 CC00)
// *******************************
typedef struct _AITS_SPI {
    AIT_REG_W   SPI_CTL;
        /*-DEFINE-----------------------------------------------------*/
        #define SPI_TXFIFO_CLR               0x20
        #define SPI_RXFIFO_CLR	             0x10
        #define SPI_TX_STOP				     0x08
        #define SPI_RX_STOP				     0x04
        #define SPI_TX_DMA_START		     0x02
        #define SPI_RX_DMA_START		     0x01
        /*------------------------------------------------------------*/
    AIT_REG_B   SPI_CLK_DIV;
    AIT_REG_B                           _x03;
    AIT_REG_W   SPI_CFG;
        /*-DEFINE-----------------------------------------------------*/
        #define MASTER_RX_USE_INCLK          0x0000
        #define MASTER_RX_PAD_CLK            0x0200
        #define TX_NON_XCH_MODE			     0x0100
        #define TX_XCH_MODE 		         0x0000
        #define SPI_MASTER_MODE              0x0040
        #define SPI_SLAVE_MODE               0x0000
        #define SPI_TX_EN   	    	     0x0020
        #define SPI_RX_EN	    	         0x0010
		#define	SS_POLAR_HIGH				 0x0008
		#define	SS_POLAR_LOW				 0x0000
		#define SS_BURST_MODE 			 	 0x0000
		#define SS_NON_BURST_MODE		 	 0x0004
		#define SCLK_PHASE0_OP 			 	 0x0000
		#define SCLK_PHASE1_OP 			 	 0x0002
		#define SCLK_IDLE_LOW 			 	 0x0000
		#define SCLK_IDLE_HIGH			 	 0x0001
        /*------------------------------------------------------------*/
    AIT_REG_B   SPI_WORD_LEN;
    AIT_REG_B                           _x07;
    AIT_REG_W   SPI_DLY_CYCLE;
    AIT_REG_W                           _x0A;
    AIT_REG_W   SPI_WAIT_CYCLE;
    AIT_REG_W                           _x0E;
    AIT_REG_B   SPI_XCH_CTL;											// 0x10
        /*-DEFINE-----------------------------------------------------*/
        #define XCH_START                   0x01
        /*------------------------------------------------------------*/
    AIT_REG_B                           _x11[3];
    AIT_REG_B   SPI_TXFIFO_SPC;											// 0x14
    AIT_REG_B   SPI_RXFIFO_SPC;											// 0x15
    AIT_REG_B   SPI_TXFIFO_THD;											// 0x16
    AIT_REG_B   SPI_RXFIFO_THD;											// 0x17
    union _SPI_TXFIFO_DATA {                                            // 0x18
    AIT_REG_B   B[4];
    AIT_REG_W   W[2];
    AIT_REG_D   D[1];
    } SPI_TXFIFO_DATA;
    union _SPI_RXFIFO_DATA {                                            // 0x1C
    AIT_REG_B   B[4];
    AIT_REG_W   W[2];
    AIT_REG_D   D[1];
    } SPI_RXFIFO_DATA;

	AIT_REG_D   SPI_INT_CPU_SR;                                        // 0x20
	AIT_REG_D   SPI_INT_CPU_EN;                                        // 0x24
	AIT_REG_D   SPI_INT_HOST_SR;                                         // 0x28
	AIT_REG_D   SPI_INT_HOST_EN;                                         // 0x2c

		/*-DEFINE-----------------------------------------------------*/
        #define SPI_RXFIFO_LE               0x00000001
        #define SPI_RXFIFO_GE               0x00000002
		#define SPI_RXFIFO_EMPTY            0x00000004
		#define SPI_RXFIFO_FULL             0x00000008
        #define SPI_TXFIFO_LE               0x00000010
        #define SPI_TXFIFO_GE               0x00000020
		#define SPI_TXFIFO_EMPTY            0x00000040
		#define SPI_TXFIFO_FULL             0x00000080
		#define SPI_RXDMA_DONE	            0x00000100
		#define SPI_TXDMA_DONE	            0x00000200
		#define SPI_FIFO_TX_DONE            0x00001000

        #define SPI_SLAVE_ERR               0x00070000
		#define SPI_RXFIFO_OF	            0x00010000
		#define SPI_TXFIFO_UF	            0x00020000
		#define SPI_RX_BIT_OF	            0x00040000
		/*------------------------------------------------------------*/
    AIT_REG_D   SPI_TXDMA_ADDR;											// 0x30
    AIT_REG_D   SPI_RXDMA_ADDR;											// 0x34
    AIT_REG_W   SPI_TXDMA_SIZE;											// 0x38
    AIT_REG_W                           _x3A;
    AIT_REG_W   SPI_RXDMA_SIZE;											// 0x3C
    AIT_REG_W                           _x3E;
    AIT_REG_W                           _x40[96];
} AITS_SPI, *AITPS_SPI;

typedef struct _AITS_SPIB {
	AITS_SPI    SPI[4];                                                
} AITS_SPIB, *AITPS_SPIB;


#if	!defined(BUILD_FW)
// SPI OPR
#define SPI_CTL			            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_CTL         )))
#define SPI_CLK_DIV		            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_CLK_DIV     )))
#define SPI_CFG			            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_CFG	      )))
#define SPI_WORD_LEN	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_WORD_LEN    )))
#define SPI_DLY_CYCLE	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_DLY_CYCLE   )))
#define SPI_WAIT_CYCLE	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_WAIT_CYCLE  )))
#define SPI_XCH_CTL  	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_XCH_CTL  )))

#define SPI_INT_CPU_SR	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_INT_CPU_SR  )))
#define SPI_INT_CPU_EN	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_INT_CPU_EN  )))
#define SPI_INT_HOST_SR	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_INT_HOST_SR )))
#define SPI_INT_HOST_EN	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_INT_HOST_EN )))
#define SPI_TXDMA_ADDR	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_TXDMA_ADDR  )))
#define SPI_RXDMA_ADDR	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_RXDMA_ADDR  )))
#define SPI_TXDMA_SIZE	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_TXDMA_SIZE  )))
#define SPI_RXDMA_SIZE	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_RXDMA_SIZE  )))
#define SPI_RXFIFO_SPC	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_RXFIFO_SPC  )))
#define SPI_TXFIFO_SPC	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_TXFIFO_SPC  )))
#define SPI_TXFIFO_DATA	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_TXFIFO_DATA.D[0] )))
#define SPI_RXFIFO_DATA	            (SPI_BASE +(MMP_ULONG)(&(((AITPS_SPIB )0)->SPI[0].SPI_RXFIFO_DATA.D[0] )))
#endif
/// @}

#endif // _MMPH_REG_SPI_H_