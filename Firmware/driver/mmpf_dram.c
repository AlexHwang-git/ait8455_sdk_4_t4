//==============================================================================
//
//  File        : mmpf_dram.c
//  Description : Firmware DRAM Control Function
//  Author      : Ben Lu
//  Revision    : 1.0
//
//==============================================================================


#include "config_fw.h"
#include "mmpf_dram.h"
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmp_reg_dram.h"
#include "mmp_reg_gbl.h"
#include "mmp_register.h"
#include "mmpf_dma.h"
#include "mmpf_pll.h"
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
#define	DRAM_TEST_CODE			0x0
#define CHIP_WINBOND_64_DDR		0x0A
#define MCI_BASE_ADDR           (0x80007700)

#if (DRAM_TEST_CODE == 0x1)
//#define TEST_DRAM_SIGNAL_CTL	(DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_OUT_INV_EN);
#define TEST_DRAM_SIGNAL_CTL	(DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN);
#endif

MMPF_DRAM_TYPE  	m_dramtype = MMPF_DRAM_TYPE_NONE;
MMPF_DRAM_MODE  	m_dramMode = MMPF_DRAM_MAX_MODE;
extern MMP_UBYTE    m_gbSystemCoreID;
static MMP_BOOL		m_dramScanEn = MMP_FALSE;
extern MMP_ULONG 	gbDramSize;
extern MMP_ULONG	glMemFreq;
volatile MMP_UBYTE 	gbDramID = 0x0;
//==============================================================================
//
//                          MODULE VARIABLES
//
//==============================================================================
MMP_USHORT m_usDramSizeTable[8] = 
{
	2, 4, 8, 16, 32, 64, 128, 256
};
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================
extern MMP_ERR DramDMATest(MMP_ULONG src, MMP_ULONG dst, MMP_ULONG wordSize, MMP_USHORT usLoop);
//==============================================================================
//
//                              FUNCTIONS
//
//==============================================================================
//------------------------------------------------------------------------------
//  Function    : MMPF_System_ReadCoreID
//  Description :
//------------------------------------------------------------------------------
/** @brief Check the chip code ID

This function Get the code ID of the chip
@return It reports the status of the operation.
*/
MMP_UBYTE MMPF_System_ReadCoreID(void)
{
	AITPS_GBL pGBL = AITC_BASE_GBL;
	
	m_gbSystemCoreID = pGBL->GBL_CHIP_VER;
	return m_gbSystemCoreID;
}




//*----------------------------------------------------------------------------
// function : MMPF_DRAM_RangeScan
// input    : none
// output   : none
// descript : The dram initialize function
//*----------------------------------------------------------------------------
MMP_ERR MMPF_DRAM_AnaRangeScan(MMP_UBYTE ulClkOutDlyRange, MMP_UBYTE *ubMacroRangeResult)
{
	#if (DRAM_TEST_CODE == 0x1)
	MMP_USHORT  j = 0x0;
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	MMP_BOOL ubDmaPass = MMP_TRUE;
	MMP_UBYTE ubMacroRange = 0xFF;
	
	
	for(j = 0x0; j < 0x100; j=j+4) {
			if(glMemFreq == 48000) {
				j = j - 3;
			}
	
			pGBL->GBL_RST_REG_EN |= (GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
			pGBL->GBL_RST_CTL01 |= (GBL_DRAM_RST | GBL_DMA_RST);
			    
			 MMPF_PLL_WaitCount(50);
			    
			 	pGBL->GBL_RST_REG_EN &= ~(GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
				pGBL->GBL_RST_CTL01 &= ~(GBL_DRAM_RST | GBL_DMA_RST);
			    
				ubDmaPass = MMP_TRUE;
				
				pDRAM->DRAM_SIGNAL_CTL = TEST_DRAM_SIGNAL_CTL;
				pDRAM->DRAM_CLK_OUT_DLY = ulClkOutDlyRange;
				
				pDRAM->DRAM_DDR_CLK_MACRO_DLY = j;
				MMPF_DRAM_Initialize(&gbDramSize, (glMemFreq/1000), MMPF_DRAM_MODE_DDR);
				while(1) {
					if (DramDMATest(0x00108000, 0x01100000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram Test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    //RTNA_DBG_Str(0, "DRAM->SRAM DMA Test start\r\n");
				    
				    if (DramDMATest(0x01200000, 0x00108000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    
				    //RTNA_DBG_Str(0, "DRAM->DRAM DMA Test start\r\n");
				    if (DramDMATest(0x01300000, 0x01000000, 512*1024, 3)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    break;
			    }
			    
			    if(ubDmaPass != MMP_TRUE) {  //Pass DMA Test
			    	break;
			    }
			    else {
			    	if(glMemFreq == 48000) {
			    		RTNA_DBG_PrintByte(0, ubMacroRange);
			    	}
			    	ubMacroRange = j;
			    }
		   	}
	
	*ubMacroRangeResult = ubMacroRange;
	#endif
	return MMP_ERR_NONE;
}
#if (DRAM_TEST_CODE == 0x1)
#include "mmpf_pll.h"
#include "mmpf_uart.h"

MMP_UBYTE strToNum(MMP_BYTE* str, MMP_ULONG ulNumOfByte, MMP_LONG* value)
{
	MMP_UBYTE str_index = 0x0, i = 0x0;
	if(((str[0] < 0x30)||(str[0] > 0x39)) && ((str[0] < 'A')||(str[0] > 'F'))&& ((str[0] < 'a')||(str[0] > 'f'))) {
		return MMP_FALSE;
	}
	
	if(str[0] == '0') {
		if((str[1] == 'x') || (str[1] == 'X')) {
			str_index = 2;
		}
		else {
			str_index = 0;
		}	
	}
	
	*value = 0;
	for(i = str_index; i < ulNumOfByte; i ++) {
		if((str[i] >= '0')&&(str[i] <= '9')) {
			*value = (*value) * 16 + (str[i] - '0');
		}
		else if((str[i] >= 'A')&&(str[i] <= 'F')) {
			*value = (MMP_ULONG)((*value) * 16 + (str[i] - 'A') + 10);
			
		}
		else if((str[i] >= 'a')&&(str[i] <= 'f')) {
			*value = (MMP_ULONG)((*value) * 16 + (str[i] - 'a') + 10);
		}
	}
	
	return MMP_TRUE;
}


MMP_ERR MMPF_DRAM_RangeScanV3(void)
{
	#if (DRAM_TEST_CODE == 0x1)
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	MMP_BYTE str[20] = {0};
	MMP_ULONG ulNumOfByte = 0x0, ulReg = 0x0, ulValue = 0x0;
	MMP_LONG ulRangeStr = 0x0, ulRangeEnd = 0x0, ulRangeStep = 0x0, ubAnalogMacroRange = 0x0;
	MMP_ULONG ulPassRangeStr = 0x0, ulPassRangeEnd = 0x0;
	MMP_BOOL ubInvTurnOn = MMP_TRUE;
	MMP_LONG  i = 0x0, j = 0x0;
	MMPF_PLL_MODE PLLMode = MMPF_PLL_96CPU_96G0134_X;
	MMP_BOOL ubDmaPass = MMP_TRUE;
	
	MMPF_OS_Sleep(500);
	while((str[0]!='x')&&(str[0]!='X')) {
		RTNA_DBG_Str(0, "\r\n==========================================\r\n");
		//RTNA_DBG_Str(0, "0 : Set Analog Delay Scan Start------\r\n");
		//RTNA_DBG_Str(0, "1 : Set Analog Delay Scan End--------\r\n");
		//RTNA_DBG_Str(0, "2 : Set Analog Delay Scan Step Range-\r\n");
		RTNA_DBG_Str(0, "0/1 : Turn ON/OFF DDR Inverse Settings-\r\n");
		//RTNA_DBG_Str(0, "a : Set PLL(DDR48): MMPF_PLL_48CPU_48G0134_X\r\n");
		RTNA_DBG_Str(0, "b : Set PLL(DDR96): MMPF_PLL_96CPU_96G0134_X\r\n");
		//RTNA_DBG_Str(0, "c : Set PLL(DDR132): MMPF_PLL_132CPU_132G0134_X\r\n");
		RTNA_DBG_Str(0, "d : Set PLL(DDR144): MMPF_PLL_144CPU_144G0134_X\r\n");
		RTNA_DBG_Str(0, "e : Set PLL(DDR166): MMPF_PLL_332CPU_192G034_166G1_X\r\n");
		RTNA_DBG_Str(0, "f : Set PLL(DDR200): MMPF_PLL_400CPU_192G034_200G1_X\r\n");
		RTNA_DBG_Str(0, "z : Set (delay start, end, range)\r\n");
		RTNA_DBG_Str(0, "X : Use Above Settings To Test-------\r\n");
		RTNA_DBG_Str(0, "==========================================\r\n\r\n");
		MMPF_Uart_GetDebugString(MMPF_UART_ID_0, str, &ulNumOfByte);
		switch (str[ulNumOfByte - 1]) {
			case '0':
				ubInvTurnOn = MMP_TRUE;
				break;
			case '1':
				ubInvTurnOn = MMP_FALSE;
				break;
			case '2':
				RTNA_DBG_Str(0, "\r\nEnter the Range value(Hex value): \r\n");
				MMPF_Uart_GetDebugString(MMPF_UART_ID_0, str, &ulNumOfByte);
				if(strToNum(str, ulNumOfByte, &ulRangeStep) == MMP_FALSE) {
					RTNA_DBG_Str(0, "Input Error !!!\r\n");
					return 1;
				}
				break;
			case 'a':
				PLLMode = MMPF_PLL_48CPU_48G0134_X;
				break;
			case 'b':
				PLLMode = MMPF_PLL_96CPU_96G0134_X;
				break;
			case 'c':
				PLLMode = MMPF_PLL_132CPU_132G0134_X;
				break;
			case 'd':
				PLLMode = MMPF_PLL_144CPU_144G0134_X;
				break;
			case 'e':
				PLLMode = MMPF_PLL_332CPU_192G034_166G1_X;
				break;
			case 'f':
				PLLMode = MMPF_PLL_400CPU_192G034_200G1_X;
				break;
			case 'z':
			case 'Z':
				RTNA_DBG_Str(0, "\r\nDelay Start(Hex value): \r\n");
				MMPF_Uart_GetDebugString(MMPF_UART_ID_0, str, &ulNumOfByte);
				if(strToNum(str, ulNumOfByte, &ulRangeStr) == MMP_FALSE) {
					RTNA_DBG_Str(0, "Input Error !!!\r\n");
					return 1;
				}
				RTNA_DBG_Str(0, "\r\nDelay End(Hex value): \r\n");
				MMPF_Uart_GetDebugString(MMPF_UART_ID_0, str, &ulNumOfByte);
				if(strToNum(str, ulNumOfByte, &ulRangeEnd) == MMP_FALSE) {
					RTNA_DBG_Str(0, "Input Error !!!\r\n");
					return 1;
				}
				RTNA_DBG_Str(0, "\r\nDelay Step(Hex value): \r\n");
				MMPF_Uart_GetDebugString(MMPF_UART_ID_0, str, &ulNumOfByte);
				if(strToNum(str, ulNumOfByte, &ulRangeStep) == MMP_FALSE) {
					RTNA_DBG_Str(0, "Input Error !!!\r\n");
					return 1;
				}
				break;
		}
	
	}
	
	
	m_dramScanEn = MMP_TRUE;
	
	if (PLLMode != MMPF_PLL_96CPU_96G0134_X) {
		MMPF_PLL_Setting(PLLMode, MMP_FALSE);
	}
	
	if(ulRangeStr <= ulRangeEnd) {
		for(i = ulRangeStr; i <= ulRangeEnd; i = i + ulRangeStep) {
		
		pGBL->GBL_RST_REG_EN |= (GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
		pGBL->GBL_RST_CTL01 |= (GBL_DRAM_RST | GBL_DMA_RST);
			    
		MMPF_PLL_WaitCount(50);
			    
		pGBL->GBL_RST_REG_EN &= ~(GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
		pGBL->GBL_RST_CTL01 &= ~(GBL_DRAM_RST | GBL_DMA_RST);
		
		ubDmaPass = MMP_TRUE;
	
		pDRAM->DRAM_DDR_CLK_MACRO_DLY = i;
		
		//RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		RTNA_DBG_Str(0, ".");
		
		if(ubInvTurnOn == MMP_TRUE) {
			RTNA_DBG_Str(0, "Inverse On !!\r\n");
			pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_OUT_INV_EN);
		}
		else {
			pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN);
		}
		MMPF_DRAM_Initialize(&gbDramSize, (glMemFreq/1000), MMPF_DRAM_MODE_DDR);
		
		if(i == ulRangeStr) {
			RTNA_DBG_Str(0, "\r\n+++++ Lock Code (R/W) +++++\r\n");
			#if (CHIP == VSN_V3)
			RTNA_DBG_PrintLong(0, pDRAM->DRAM_DQBLK0_RD_CTL);
			RTNA_DBG_PrintLong(0, pDRAM->DRAM_DQBLK0_WR_CTL);
			#endif
		}
		
		RTNA_DBG_Str(0, "\r\n Current Scan Range = ");
		RTNA_DBG_Long(0, i);
		RTNA_DBG_Str(0, "  ");
		MMPF_PLL_WaitCount(0xFF);
		//RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		MMPF_DMA_Initialize();
				while(1) {
					if (DramDMATest(0x00120000, 0x01100000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram Test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    //RTNA_DBG_Str(0, "DRAM->SRAM DMA Test start\r\n");
				    
				    if (DramDMATest(0x01200000, 0x00120000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    
				    //RTNA_DBG_Str(0, "DRAM->DRAM DMA Test start\r\n");
				    if (DramDMATest(0x01300000, 0x01000000, 256*1024, 3)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    break;
			    }
			    
			    if(ubDmaPass != MMP_TRUE) {  //Pass DMA Test
			    	break;
			    }
			    else {
			    	ubAnalogMacroRange = i;
			    	
				    if(ulPassRangeStr == 0x0) {
				    	ulPassRangeStr = ubAnalogMacroRange;
				    }
				    else {
				    	ulPassRangeEnd = ubAnalogMacroRange;
				    }
			    }
			    
			    if(ulRangeStep == 0x0) {
			    	break;
			    }
		}
		
	}
	else {
		for(i = ulRangeStr; i >= ulRangeEnd; i = i - ulRangeStep) {
		
		pGBL->GBL_RST_REG_EN |= (GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
		pGBL->GBL_RST_CTL01 |= (GBL_DRAM_RST | GBL_DMA_RST);
			    
		MMPF_PLL_WaitCount(50);
			    
		pGBL->GBL_RST_REG_EN &= ~(GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
		pGBL->GBL_RST_CTL01 &= ~(GBL_DRAM_RST | GBL_DMA_RST);
		
		ubDmaPass = MMP_TRUE;
	
		pDRAM->DRAM_DDR_CLK_MACRO_DLY = i;
		
		//RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		RTNA_DBG_Str(0, ".");
		MMPF_DRAM_Initialize(&gbDramSize, (glMemFreq/1000), MMPF_DRAM_MODE_DDR);
		
		if(i == ulRangeStr) {
			RTNA_DBG_Str(0, "\r\n+++++ Lock Code (R/W) +++++\r\n");
			#if (CHIP == VSN_V3)
			RTNA_DBG_PrintLong(0, pDRAM->DRAM_DQBLK0_RD_CTL);
			RTNA_DBG_PrintLong(0, pDRAM->DRAM_DQBLK0_WR_CTL);
			#endif
		}
		
		RTNA_DBG_Str(0, "\r\nCurrent Scan Range = ");
		RTNA_DBG_Long(0, i);
		RTNA_DBG_Str(0, "  ");
		MMPF_PLL_WaitCount(0xFF);
		//RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		MMPF_DMA_Initialize();
				while(1) {
					if (DramDMATest(0x00120000, 0x01100000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram Test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    //RTNA_DBG_Str(0, "DRAM->SRAM DMA Test start\r\n");
				    
				    if (DramDMATest(0x01200000, 0x00120000, 64*1024, 2)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    
				    //RTNA_DBG_Str(0, "DRAM->DRAM DMA Test start\r\n");
				    if (DramDMATest(0x01300000, 0x01000000, 256*1024, 3)) {
				        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
				        ubDmaPass = MMP_FALSE;
				        break;
				    }
				    break;
			    }
			    
			    if(ubDmaPass != MMP_TRUE) {  //Pass DMA Test
			    	break;
			    }
			    else {
			    	ubAnalogMacroRange = i;
			    	
			    	if(ulPassRangeEnd == 0x0) {
				    	ulPassRangeEnd = ubAnalogMacroRange;
				    }
				    else {
				    	ulPassRangeStr = ubAnalogMacroRange;
				    }	
			    }
			 
			 if(ulRangeStep == 0x0) {
			    	break;
			 }
		}
		
	}
	
	
	
		RTNA_DBG_Str(0, "\r\nDDR clock = ");
	    RTNA_DBG_Long(0, glMemFreq);
	    RTNA_DBG_Str(0, "  , ("); 
		RTNA_DBG_Str(0, "MinDly = ");
		RTNA_DBG_Byte(0, ulPassRangeStr);
		RTNA_DBG_Str(0, ", MaxDly = ");
		RTNA_DBG_Byte(0, ulPassRangeEnd);
		RTNA_DBG_Str(0, ")\r\n");
		MMPF_PLL_WaitCount(50);	
	
	return 1;
	#endif //#if (DRAM_TEST_CODE == 0x1)

}
#endif

MMP_ERR MMPF_DRAM_RangeScan(void)
{
	#if (DRAM_TEST_CODE == 0x1)
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	MMP_BOOL ubDmaPass = MMP_TRUE;
	MMP_UBYTE ubOutDlyMin, ubOutDlyMax, ubOutDlyCurrent, ubRangeMax = 0x10;
	MMP_UBYTE ubRangCount = 0x0;
	MMP_UBYTE ubCurrentPllMode = 0x0 /*MMPF_PLL_48CPU_48G0134_X*/, ubMaxPllMode = 0x6;
	MMP_UBYTE ubMacroRangeMin = 0x0, ubMacroRangeMax = 0x0;
	MMP_UBYTE ubPllRangeStart[0x10] = {2, 0, 0, 0, 0, 0};
	
	
	m_dramScanEn = MMP_TRUE;
	for(ubCurrentPllMode = 0x2; ubCurrentPllMode < 3/*ubMaxPllMode*/; ubCurrentPllMode ++) {
		ubOutDlyMin = ubRangeMax;
		ubOutDlyMax = ubRangeMax;
		ubOutDlyCurrent = 0x0;
		#if (CHIP == VSN_V2)
		MMPF_PLL_SetVSNV2PLL((MMPF_PLL_MODE)ubCurrentPllMode, MMP_FALSE);
		#endif
		
		
		RTNA_DBG_Str(0, "--------------------------------------\r\n");
		RTNA_DBG_PrintLong(0, glMemFreq);
		for(ubOutDlyCurrent = 2/*ubPllRangeStart[ubCurrentPllMode]*/; ubOutDlyCurrent < ubRangeMax; ubOutDlyCurrent++) {
			RTNA_DBG_Str(0,"*");
			
			if(glMemFreq == 48000) {
				if(ubOutDlyCurrent == 0x9) {
					break;
				}
			}
			pGBL->GBL_RST_REG_EN |= (GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
			pGBL->GBL_RST_CTL01 |= (GBL_DRAM_RST | GBL_DMA_RST);
		    
		    MMPF_PLL_WaitCount(50);
		    
		    pGBL->GBL_RST_REG_EN &= ~(GBL_REG_DRAM_RST | GBL_REG_DMA_RST);
			pGBL->GBL_RST_CTL01 &= ~(GBL_DRAM_RST | GBL_DMA_RST);
		 
			ubDmaPass = MMP_TRUE;
			pDRAM->DRAM_SIGNAL_CTL = TEST_DRAM_SIGNAL_CTL;
			pDRAM->DRAM_CLK_OUT_DLY = ubOutDlyCurrent;
			MMPF_DRAM_Initialize(&gbDramSize, (glMemFreq/1000), MMPF_DRAM_MODE_DDR);
			while(1) {
				if (DramDMATest(0x00108000, 0x01100000, 64*1024, 2)) {
			        RTNA_DBG_Str(0, "Dram Test failed\r\n");
			        ubDmaPass = MMP_FALSE;
			        break;
			    }
			    //RTNA_DBG_Str(0, "DRAM->SRAM DMA Test start\r\n");
			    
			    if (DramDMATest(0x01200000, 0x00108000, 64*1024, 2)) {
			        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
			        ubDmaPass = MMP_FALSE;
			        break;
			    }
			    
			    //RTNA_DBG_Str(0, "DRAM->DRAM DMA Test start\r\n");
			    if (DramDMATest(0x01300000, 0x01000000, 512*1024, 3)) {
			        RTNA_DBG_Str(0, "Dram DMA test failed\r\n");
			        ubDmaPass = MMP_FALSE;
			        break;
			    }
			    break;
		    }
		    
		    if(ubDmaPass == MMP_TRUE) {  //Pass DMA Test
		    	if(ubOutDlyMin == ubRangeMax) {
		    		ubOutDlyMin = ubOutDlyCurrent;
		    	}
		    	else{
		    		ubOutDlyMax = ubOutDlyCurrent;
		    	}
		    }
		   	else {
		   		RTNA_DBG_Str(0, "!!! bad settings\r\n");
		   		RTNA_DBG_PrintByte(0, ubOutDlyCurrent);
		   		
		   		if(ubOutDlyMax != ubRangeMax){
		   			break;
		   		}
		   		else if(ubOutDlyMin != ubRangeMax){
		   			ubOutDlyMax = ubOutDlyMin;
		   		}
		   	}
		   
	    }
	    
	    ubMacroRangeMin = 0x0;
	    ubMacroRangeMax = 0x0;
		
		RTNA_DBG_Str(0, "===TEST MACRO===\r\n");
		MMPF_DRAM_AnaRangeScan(ubOutDlyMin, &ubMacroRangeMin);
		MMPF_DRAM_AnaRangeScan(ubOutDlyMax, &ubMacroRangeMax);
		RTNA_DBG_Str(0, "\r\nDRAM clock = ");
	    RTNA_DBG_Long(0, glMemFreq);
	    RTNA_DBG_Str(0, "  , ("); 
		RTNA_DBG_Str(0, "MinDly = ");
		RTNA_DBG_Byte(0, ubOutDlyMin);
		RTNA_DBG_Str(0, ", MinMacroDly = ");
		RTNA_DBG_Byte(0, ubMacroRangeMin);
		RTNA_DBG_Str(0, ", MaxDly = ");
		RTNA_DBG_Byte(0, ubOutDlyMax);
		RTNA_DBG_Str(0, ", MaxMacroDly = ");
		RTNA_DBG_Byte(0, ubMacroRangeMax);
		RTNA_DBG_Str(0, ")\r\n");
		MMPF_PLL_WaitCount(50);	
    }
	
	
	RTNA_DBG_Str(0, "\r\n\r\n\r\n\r\n====================Rnage Scan Done !!!====================\r\n");
	#endif
	return MMP_ERR_NONE;
}


#pragma arm section code = "EnterSelfSleepMode", rwdata = "EnterSelfSleepMode",  zidata = "EnterSelfSleepMode"
//*----------------------------------------------------------------------------
//* Function Name       : MMPF_DRAM_SetSelfRefresh
//* Input Parameters    : none
//* Output Parameters   : none
//* Functions called    : Initial Dram
//*----------------------------------------------------------------------------
MMP_ERR MMPF_DRAM_SetSelfRefresh(MMP_BOOL bEnterSelfRefresh)
{
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	if(bEnterSelfRefresh) {
		pDRAM->DRAM_FUNC &= ~(DRAM_CLK_GATE_EN);
		pDRAM->DRAM_CMD_CTL0 = DRAM_SRF_ST;
		while((pDRAM->DRAM_INT_CPU_SR & DRAM_SRF_DONE) == 0);
		pDRAM->DRAM_INT_CPU_SR = DRAM_SRF_DONE;
		pDRAM->DRAM_FUNC |= (DRAM_CLK_GATE_EN);
	}
	else {
		pDRAM->DRAM_FUNC &= ~(DRAM_CLK_GATE_EN);
		pDRAM->DRAM_CMD_CTL0 = DRAM_EXIT_SRF_ST;
		while((pDRAM->DRAM_INT_CPU_SR & DRAM_EXIT_SRF_ST) == 0);
		pDRAM->DRAM_INT_CPU_SR = DRAM_EXIT_SRF_ST;
		pDRAM->DRAM_FUNC |= (DRAM_CLK_GATE_EN);
	
	}
	return	MMP_ERR_NONE;	
}
#pragma arm section code, rwdata,  zidata

//*----------------------------------------------------------------------------
// function : MMPF_DRAM_Initialize
// input    : none
// output   : none
// descript : The dram initialize function
//*----------------------------------------------------------------------------
#if (CHIP == VSN_V2)
MMP_ERR MMPF_DRAM_Initialize(MMP_ULONG *ulSize, MMP_ULONG ulClock, MMPF_DRAM_MODE drammode)
{
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	MMP_UBYTE i;
	volatile MMP_UBYTE *MCI_REG_BASE_B = (volatile MMP_UBYTE *) MCI_BASE_ADDR;
	//volatile MMP_UBYTE *DRAM_REG_BASE_B = (volatile MMP_UBYTE *) 0x80006E00;  //for Debug
	
	gbDramID = MMPF_System_ReadCoreID();
	
	#if defined(ALL_FW)
		if(gbDramID == MMPF_DRAMID_WINBOND_SDR16)  {
			*ulSize = 16*1024*1024; //Mbit
		}
		else if((gbDramID == MMPF_DRAMID_WINBOND_DDR64) || (gbDramID == MMPF_DRAMID_WINBOND_SDR64)){
			*ulSize = 64*1024*1024; //Mbit
		}
		return MMP_ERR_NONE;
	#endif
	
	for(i = 0x0;i < 0x20;i++) {  //6940~695F
		pGBL->GBL_IO_CFG_PSTM_T[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
    for(i = 0x0;i < 0x18;i++) {  //69d8~68EF
        pGBL->GBL_IO_CFG_PSTM_B[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
	for(i = 0x0;i < 0x8;i++) {  //6998~689F
        pGBL->GBL_IO_CFG_PSTM_B24[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
	
	if((gbDramID == MMPF_DRAMID_WINBOND_DDR64) || (gbDramID == MMPF_DRAMID_WINBOND_SDR64) || (gbDramID == MMPF_DRAMID_WINBOND_SDR16)) {
		*ulSize = 64*1024*1024;
		
		if(gbDramID == MMPF_DRAMID_WINBOND_SDR16) {
			*ulSize = 16*1024*1024;
		}
		pDRAM->DRAM_DDR_LOCK_JUDGE_CTL = DDR_LOCK_REF_CLK_CNT(0x10);
		pDRAM->DRAM_DDR_CLK_OPT = DQCLK_DUTY_BYPASS;	
		pDRAM->DRAM_DDR_PHY_DQB0_CTL_OP = DQ_BLOCK_CTL(0x0);	
		pDRAM->DRAM_DDR_PHY_DQB1_CTL_OP = DQ_BLOCK_CTL(0x0);
			
		pDRAM->DRAM_DDR_LOCK_LOOP_CTL = DDR_HW_CTL_BY_SW;
		
		pDRAM->DRAM_DDR_PHY_CTL = (DRAM_DDR_DLL_RST | DRAM_DDR_PWR_EN);
		
		pDRAM->DRAM_DDR_LOCK_LOOP_CTL = DDR_STOP_DLYBIT_CTL;
		pDRAM->DRAM_DDR_PHY_CTL = DDR_PHY_CTL_CLEAN;
			
		if(gbDramID == MMPF_DRAMID_WINBOND_SDR16) {
			pDRAM->DRAM_CTL_1 = (DRAM_BUS_16 | DRAM_BANK_INTLV);
			pDRAM->DRAM_CTL_2 = (DRAM_COL_256 | DRAM_ROW_2048);
			
			//MCI bypass dram queue
			MCI_REG_BASE_B[0x10]=0x02;
			//MCI_REG_BASE_B[0x11]=0x04;
		
		}
		else if((gbDramID == MMPF_DRAMID_WINBOND_SDR64) || (gbDramID == MMPF_DRAMID_WINBOND_DDR64)) {
			pDRAM->DRAM_CTL_1 = (DRAM_BUS_32 | DRAM_BANK_INTLV);
			pDRAM->DRAM_CTL_2 = (DRAM_COL_256 | DRAM_ROW_2048);
			
			//MCI bypass dram queue
			MCI_REG_BASE_B[0x10]=0x02;
			MCI_REG_BASE_B[0x11]=0x04;
		}
		
		if(gbDramID == MMPF_DRAMID_WINBOND_DDR64) { 
			pGBL->GBL_STACK_MEM_TYPE = MEM_WINBO_64Mb_DDR;
				
			pDRAM->DRAM_CTL_0 = (DDR_MODE | BANK_SIZE_4M | DRAM_CAS_LATENCY_3);
			pDRAM->DRAM_DDR_OPR_CTL = DRAM_DDR_CLK_PAD_EN;
				
			pDRAM->DRAM_FUNC = /*DRAM_EH_SPRE_EN |*/ DRAM_NA_EN | DRAM_CLK_GATE_EN |
								DRAM_ARF_EN | DRAM_APD_EN | DRAM_PMP_EN /*| DRAM_SPRE_EN*/;
			if(!m_dramScanEn) {
			   	pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN /*| DRAM_CLK_OUT_INV_EN*/);		
			} 
		  	
		   	switch(ulClock) {
		   		case 48:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 48\r\n");
		   				pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x2DE;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(2) |	
										DRAM_TRC_CYC(2)  | 	
										DRAM_TRCD_CYC(0) |
										DRAM_TRFC_CYC(3) |
										DRAM_TRP_CYC(0)  |
										DRAM_TRRD_CYC(0) |
										DRAM_TXSR_CYC(5) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(0);
		   			break;
		   		case 96:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 96\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x05CC;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(4) |	
										DRAM_TRC_CYC(5)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(6) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(11) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 120:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 120\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 2;
		   				pDRAM->DRAM_CLK_OUT_DLY = 9; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0743;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(8) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(8) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
		   			break;
		   		case 132:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 132\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x7FF;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(9) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 133:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 133\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x080F;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(9) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 144:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 144\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x910;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(11) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(17) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 166:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 166\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0A12;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(9)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(11) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(19) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
		   			break;
		   		default:
		   			RTNA_DBG_Str(0, "Un-supported DRAM ulClock !\r\n");
		   			return 1;
		   			break;
		   	
		   	};
		}//if(gbDramID == MMPF_DRAMID_WINBOND_DDR64)
	 	else if(gbDramID == MMPF_DRAMID_WINBOND_SDR64) {
	 		pGBL->GBL_STACK_MEM_TYPE = MEM_WINBO_64Mb_SDR;
			pDRAM->DRAM_CTL_0 = (BANK_SIZE_4M | DRAM_CAS_LATENCY_3);
			pDRAM->DRAM_DDR_OPR_CTL = (DRAM_SDR_MODE | DRAM_DDR_CLK_PAD_EN);
				
			pDRAM->DRAM_FUNC = DRAM_EH_SPRE_EN | DRAM_NA_EN | DRAM_CLK_GATE_EN |
							DRAM_ARF_EN | DRAM_APD_EN | DRAM_PMP_EN | DRAM_SPRE_EN;
							   	
		   	switch(ulClock) {
		   		case 48:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 48\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x2DE;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(2) |	
										DRAM_TRC_CYC(2)  | 	
										DRAM_TRCD_CYC(0) |
										DRAM_TRFC_CYC(3) |
										DRAM_TRP_CYC(0)  |
										DRAM_TRRD_CYC(0) |
										DRAM_TXSR_CYC(5) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(0);
		   			break;
		   		case 96:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 96\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x05CC;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(4) |	
										DRAM_TRC_CYC(5)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(6) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(11) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 120:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 120\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0743;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(8) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(8) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
		   			break;
		   		case 132:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 132\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x7FF;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(9) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 133:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 133\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x080F;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(9) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 144:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 144\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x910;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(11) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(17) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 166:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR 166\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN | DRAM_CLK_IN_INV_EN);
		   				pDRAM->DRAM_CLK_OUT_DLY = 0; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 8;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0A12;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(9)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(11) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(19) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
		   			break;
		   		default:
		   			RTNA_DBG_Str(0, "Un-supported DRAM ulClock !\r\n");
		   			return 1;
		   			break;
		   	
		   	};
	 	}
		else if(gbDramID == MMPF_DRAMID_WINBOND_SDR16) {
	 		pGBL->GBL_STACK_MEM_TYPE = MEM_WINBO_16Mb_SDR;
			pDRAM->DRAM_CTL_0 = (BANK_SIZE_2M | DRAM_CAS_LATENCY_3);
			pDRAM->DRAM_DDR_OPR_CTL = (DRAM_SDR_MODE);
				
			pDRAM->DRAM_FUNC = DRAM_EH_SPRE_EN | DRAM_NA_EN | DRAM_CLK_GATE_EN |
							DRAM_ARF_EN | DRAM_APD_EN | DRAM_PMP_EN | DRAM_SPRE_EN;
							   	
		   	switch(ulClock) {
		   		case 48:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 48\r\n");
		   				pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x2DE;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(1) |	
										DRAM_TRC_CYC(2)  | 	
										DRAM_TRCD_CYC(0) |
										DRAM_TRFC_CYC(3) |
										DRAM_TRP_CYC(0)  |
										DRAM_TRRD_CYC(0) |
										DRAM_TXSR_CYC(3) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 96:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 96\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_CLK_OUT_DLY = 6; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x05CC;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(3) |	
										DRAM_TRC_CYC(5)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(7) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(6) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(3);
		   			break;
		   		case 120:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 120\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 2;
		   				pDRAM->DRAM_CLK_OUT_DLY = 9; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0743;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(4) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(9) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(8) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(4);
		   			break;
		   		case 132:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 132\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x7FF;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(10) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(9) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(4);
		   			break;
		   		case 133:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 133\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x080F;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(10) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(9) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(4);
		   			break;
		   		case 144:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 144\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x8BA;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(11) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(10) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(5);
		   			break;
		   		case 166:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "SDR16 166\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_IN_DLY_EN /*| DRAM_CLK_IN_INV_EN*/);
		   				pDRAM->DRAM_CLK_OUT_DLY = 0x0;
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0x0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0A12;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(9)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(13) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(11) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(5);
		   			break;
		   		default:
		   			RTNA_DBG_Str(0, "Un-supported DRAM ulClock !\r\n");
		   			return 1;
		   			break;
		   	
		   		};
		   }
		pDRAM->DRAM_CTL_4 = DRAM_INIT_CLEAN;
		
		pDRAM->DRAM_INT_HOST_SR |= (DRAM_ARF_DONE | DRAM_INIT_DONE);
		pDRAM->DRAM_CTL_4 = DRAM_INIT_ST;
		while(!(pDRAM->DRAM_INT_HOST_SR & DRAM_INIT_DONE));
	
	}
	if(!m_dramScanEn) {
		RTNA_DBG_Str(0, "Init DRAM done !\r\n");
	}
	return MMP_ERR_NONE;
}
#endif //#if (CHIP == VSN_V2)


#if (CHIP == VSN_V3)
MMP_ERR MMPF_DRAM_Initialize(MMP_ULONG *ulSize, MMP_ULONG ulClock, MMPF_DRAM_MODE drammode)
{
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_DRAM pDRAM = AITC_BASE_DRAM;
	MMP_UBYTE i;
	volatile MMP_UBYTE *MCI_REG_BASE_B = (volatile MMP_UBYTE *) MCI_BASE_ADDR;
	//volatile MMP_UBYTE *DRAM_REG_BASE_B = (volatile MMP_UBYTE *) 0x80006E00;  //for Debug
	volatile MMP_UBYTE *ptr = (volatile MMP_UBYTE *)0x8000690A;

	//gbDramID = MMPF_System_ReadCoreID();
	
	#if defined(ALL_FW)
		*ulSize = 256*1024*1024;
		return MMP_ERR_NONE;
	#endif 

	ptr[0x0] |= 0x80;
	//pDRAM->DRAM_DDR_DQPLK_PD_CTL = 0x3F;
	MMPF_PLL_WaitCount(0x1FF);
	//pDRAM->DRAM_DDR_DQPLK_PD_CTL = 0x0;
	ptr[0x0] &= (~0x80);
	
	for(i = 0x0;i < 0x20;i++) {  //6940~695F
		pGBL->GBL_IO_CFG_PSTM_T[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
    for(i = 0x0;i < 0xB;i++) {  //5D40~5D4A
		pGBL->GBL_IO_CFG_PSTM_T32[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
    
    for(i = 0x0;i < 0x18;i++) {  //69d8~68EF
        pGBL->GBL_IO_CFG_PSTM_B[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
	for(i = 0x0;i < 0x8;i++) {  //6998~689F
        pGBL->GBL_IO_CFG_PSTM_B24[i] = (GBL_IO_OUT_DRIVING(0x2) | GBL_IO_SLOW_SLEW_EN | GBL_IO_PULL_DOWN_RESIS_EN | GBL_IO_SCHMITT_TRIG_EN);
    }
	
	
	//if((gbDramID == MMPF_DRAMID_WINBOND_DDR64) || (gbDramID == MMPF_DRAMID_WINBOND_SDR64)) {
	if(1) {
		*ulSize = 256*1024*1024;
		//0x65 & 0x69 set as 0x10
		pDRAM->DRAM_DDR_DLL0_LOCK_CTL = DDR_LOCK_REF_CLK_CNT(0x10);
		pDRAM->DRAM_DDR_DLL1_LOCK_CTL = DDR_LOCK_REF_CLK_CNT(0x10);
		
		//0x6B as 0x08
		pDRAM->DRAM_DDR_CLOCK_OPT = DQCLK_DUTY_BYPASS;
		
		//0x63 & 0x67 set as 0x40
		pDRAM->DRAM_DDR_DLL0_CTL = DDR_HW_CTL_BY_SW;
		pDRAM->DRAM_DDR_DLL1_CTL = DDR_HW_CTL_BY_SW;
		
		//0x60 power-down & reset DLL0 & DLL1
		pDRAM->DRAM_DDR_DLL_CTL = (DRAM_DDR_DLL0_RST | DRAM_DDR_DLL1_RST | DRAM_DDR_DLL0_PWR_DOWN | DRAM_DDR_DLL1_PWR_DOWN);
		
		//0x63 & 0x67 set as 0x40
		pDRAM->DRAM_DDR_DLL0_CTL = DDR_STOP_DLYBIT_CTL;
		pDRAM->DRAM_DDR_DLL1_CTL = DDR_STOP_DLYBIT_CTL;
		
		pDRAM->DRAM_DDR_DLL_CTL = DDR_PHY_CTL_CLEAN;
		
		//pDRAM->DRAM_CTL_1 = (DRAM_BUS_32 | DRAM_BANK_SEQ);
		pDRAM->DRAM_CTL_1 = (DRAM_BUS_32 | DRAM_BANK_INTLV);
		pDRAM->DRAM_CTL_2 = (DRAM_COL_512 | DRAM_ROW_4096);
		
		//MCI bypass dram queue
		MCI_REG_BASE_B[0x10]=0x02;
		MCI_REG_BASE_B[0x11]=0x04;
		//------------------------------------------------
		
		
		if(1) {//(gbDramID == MMPF_DRAMID_WINBOND_DDR256) { 
			pGBL->GBL_STACK_MEM_TYPE = MEM_WINBO_256Mb_DDR;
				
			pDRAM->DRAM_CTL_0 = (DDR_MODE | BANK_SIZE_4M | DRAM_CAS_LATENCY_3);
			pDRAM->DRAM_DDR_OPR_CTL = (DRAM_DDR_CLK_PAD_EN | DRAM_DQBLK1_SDR_EN | DRAM_DQBLK3_SDR_EN); //6 DQBLK only use 4
				
			pDRAM->DRAM_FUNC = DRAM_EH_SPRE_EN | DRAM_NA_EN | DRAM_CLK_GATE_EN |
								DRAM_ARF_EN | DRAM_APD_EN | DRAM_PMP_EN | DRAM_SPRE_EN;
			if(!m_dramScanEn) {
			   	pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN | DRAM_CLK_OUT_INV_EN);		
			} 
		  	
		   	switch(ulClock) {
		   		case 48:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 48\r\n");
		   				pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x2DE;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(1) |	
										DRAM_TRC_CYC(2)  | 	
										DRAM_TRCD_CYC(0) |
										DRAM_TRFC_CYC(3) |
										DRAM_TRP_CYC(0)  |
										DRAM_TRRD_CYC(0) |
										DRAM_TXSR_CYC(3) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(0);
		   			break;
		   		case 96:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 96\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 4;
		   				pDRAM->DRAM_CLK_OUT_DLY = 0x0; //DRAM_CLK_OUT_INV_EN off
		   				//pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x05CC;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(3) |	
										DRAM_TRC_CYC(5)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(7) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(0) |
										DRAM_TXSR_CYC(7) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 120:
		   			
		   			break;
		   		case 132:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 132\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x7FF;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(10) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(10) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 133:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 133\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x080F;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(7)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(10) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(10) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 144:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 144\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				pDRAM->DRAM_CLK_OUT_DLY = 8; //DRAM_CLK_OUT_INV_EN off
		   				pDRAM->DRAM_RDDATA_CLK_DLY = 0;
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x910;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(5) |	
										DRAM_TRC_CYC(8)  | 	
										DRAM_TRCD_CYC(1) |
										DRAM_TRFC_CYC(12) |
										DRAM_TRP_CYC(1)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(12) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(1);
		   			break;
		   		case 166:
		   			if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 166\r\n");
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0x6; //DRAM_CLK_OUT_INV_EN off
		   				//pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN);
		   				pDRAM->DRAM_DDR_CLK_MACRO_DLY = 0x52;
		   				RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		   				
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0A12;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(6) |	
										DRAM_TRC_CYC(9)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(13) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(13) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
					break;
			    case 192:
			        if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 192\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN);
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0x0; //DRAM_CLK_OUT_INV_EN off
		   				//pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   				pDRAM->DRAM_DDR_CLK_MACRO_DLY = 0x38;
		   				RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		   				RTNA_DBG_PrintLong(0, pDRAM->DRAM_SIGNAL_CTL);
		   				
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0BA8;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(7) |	
										DRAM_TRC_CYC(10)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(15) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);
		   			break;
			    case 195:
			    case 198:
				case 200:
				case 204:
					pDRAM->DRAM_FUNC = DRAM_NA_EN | DRAM_CLK_GATE_EN | DRAM_ARF_EN | DRAM_APD_EN | DRAM_PMP_EN | DRAM_SPRE_EN;
					if(!m_dramScanEn) {
		   				RTNA_DBG_Str(0, "DDR 200\r\n");
		   				pDRAM->DRAM_SIGNAL_CTL = (DRAM_CYC_DLY_EN | DRAM_CLK_OUT_DLY_EN);
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0;
		   				//pDRAM->DRAM_CLK_OUT_DLY = 0x0; //DRAM_CLK_OUT_INV_EN off
		   				//pDRAM->DRAM_RDDATA_CLK_DLY = 2;
		   				pDRAM->DRAM_DDR_CLK_MACRO_DLY = 0x32;
		   				RTNA_DBG_PrintLong(0, pDRAM->DRAM_DDR_CLK_MACRO_DLY);
		   				RTNA_DBG_PrintLong(0, pDRAM->DRAM_SIGNAL_CTL);
		   				
		   			}
		   			pDRAM->DRAM_ARF_CYC = 0x0C25;
			   		pDRAM->DRAM_CTL_CYC_0 = (DRAM_TRAS_CYC(7) |	
										DRAM_TRC_CYC(10)  | 	
										DRAM_TRCD_CYC(2) |
										DRAM_TRFC_CYC(15) |
										DRAM_TRP_CYC(2)  |
										DRAM_TRRD_CYC(1) |
										DRAM_TXSR_CYC(15) |
										DRAM_TMRD_CYC(1));
										
					pDRAM->DRAM_CTL_CYC_1 = DRAM_TWR_CYC(2);	
					pDRAM->DRAM_CTL_CYC_EXT = DRAM_TRFC_EXT_CYC(15) | DRAM_TXSR_EXT_CYC(15);
		   			break;
		   		default:
		   			RTNA_DBG_Str(0, "Un-supported DRAM ulClock !\r\n");
		   			return 1;
		   			break;
		   	
		   	};
		   	
		   	
		   	pDRAM->DRAM_CTL_4 = DRAM_INIT_CLEAN;
		
			pDRAM->DRAM_INT_HOST_SR |= (DRAM_ARF_DONE | DRAM_INIT_DONE);
			pDRAM->DRAM_CTL_4 = DRAM_INIT_ST;
			while(!(pDRAM->DRAM_INT_HOST_SR & DRAM_INIT_DONE));
			
			
			#if 0  //DRAM function test only!!!
			pDRAM->DRAM_DDR_DQBLK0_CTL = 0x1;
			//pDRAM->DRAM_DDR_DQBLK1_CTL = 0x4;
			pDRAM->DRAM_DDR_DQBLK2_CTL = 0x1;
			//pDRAM->DRAM_DDR_DQBLK3_CTL |= 0x4;
			pDRAM->DRAM_DDR_DQBLK4_CTL = 0x1;
			pDRAM->DRAM_DDR_DQBLK5_CTL = 0x1;
			pDRAM->DRAM_DDR_DQBLK0_CTL_OPT = 0x08;
			//pDRAM->DRAM_DDR_DQBLK1_CTL_OPT |= 0x4;
			pDRAM->DRAM_DDR_DQBLK2_CTL_OPT = 0x08;
			//pDRAM->DRAM_DDR_DQBLK3_CTL_OPT |= 0x4;
			pDRAM->DRAM_DDR_DQBLK4_CTL_OPT = 0x8;
			pDRAM->DRAM_DDR_DQBLK5_CTL_OPT = 0x8;
			
			pGBL->GBL_PROBE_CTL |= 0x80;
			pGBL->GBL_PROBE_CTL |= 0x6;
			
			
			pDRAM->DRAM_DFI_CTL = 0x09;
			#endif
		}//if(gbDramID == MMPF_DRAMID_WINBOND_DDR64)
		
	}
	 	
	if(!m_dramScanEn) {
		RTNA_DBG_Str(0, "Init DRAM done !\r\n");
	}
	return MMP_ERR_NONE;
}

#endif //#if (CHIP == VSN_V3)