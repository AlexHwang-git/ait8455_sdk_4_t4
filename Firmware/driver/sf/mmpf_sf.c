#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_typedef.h"
#include "mmpf_sf.h"
#include "mmp_reg_sf.h"
#include "mmp_register.h"
#include "mmpf_pll.h"

static MMP_ULONG m_ulSFDmaAddr;
static MMP_ULONG glDevId;
// Give a default size
static MMP_ULONG glSFTotalSize = 0x100000 ;
static MMP_ULONG glSFSectorSize= 0x1000   ;


MMP_ERR MMPF_SF_CheckReady(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
    
    while((pSIF->SIF_INT_CPU_SR & SIF_CMD_DONE) == 0) ;
	
    return  MMP_ERR_NONE;
}
MMP_ERR MMPF_SF_ReadStatus(MMP_UBYTE *ubStatus)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
   	pSIF->SIF_CMD = READ_STATUS;
    
    pSIF->SIF_CTL = SIF_START | SIF_R | SIF_DATA_EN;

    MMPF_SF_CheckReady();
    
    *ubStatus = pSIF->SIF_DATA_RD;
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_WriteStatus(MMP_UBYTE ubData)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_UBYTE ubStatus;
	
	MMPF_SF_EnableWrite();
	
	pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = WRITE_STATUS;
    
    pSIF->SIF_DATA_WR = ubData;
    
    pSIF->SIF_CTL = SIF_START | SIF_W | SIF_DATA_EN;
    
    MMPF_SF_CheckReady();
   
    do {
        MMPF_SF_ReadStatus(&ubStatus);
    } while (ubStatus & WRITE_IN_PROGRESS);
   
    
    return  MMP_ERR_NONE;
}

//every time you want to write, you must enable write again

MMP_ERR MMPF_SF_EnableWrite(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
    pSIF->SIF_CMD = WRITE_ENABLE;
   
    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_DisableWrite(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = WRITE_DISABLE;

    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();

    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_SetTmpAddr(MMP_ULONG ulStartAddr)
{
    m_ulSFDmaAddr = ulStartAddr;
    
    RTNA_DBG_Str(3, "sf dma addr: ");
    RTNA_DBG_Long(3, ulStartAddr);
    RTNA_DBG_Str(3, " \r\n");
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_GetTmpAddr(MMP_ULONG *ulStartAddr)
{
    *ulStartAddr = m_ulSFDmaAddr;
    
    return  MMP_ERR_NONE;
}

/** @brief Initial SD module and interafce

This function initial SIF module 
@param[in] padid Pad ID of SIF controller
@retval MMP_ERR_NONE Now it always return success.
*/
MMP_ERR MMPF_SF_InitialInterface(void)
{
	AITPS_GBL pGBL = AITC_BASE_GBL;
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_ULONG ulG0Freq = 0x0;
	
    pGBL->GBL_CLK_DIS1 &= ~GBL_CLK_BS_SPI_DIS;
    
    pGBL->GBL_IO_CTL1 |= GBL_SPI_PAD_EN;
    
    MMPF_PLL_GetGroupFreq(0x0, &ulG0Freq);
    //Note: SIF should divid more than 2
    RTNA_DBG_PrintLong(0, ulG0Freq);
    if(ulG0Freq <= 192000){
   		pSIF->SIF_CLK_DIV = SIF_SET_CLK_DIV(6);
	}else if((ulG0Freq > 192000) && (ulG0Freq <= 256000)){
		pSIF->SIF_CLK_DIV = SIF_SET_CLK_DIV(8);
	}else{
		pSIF->SIF_CLK_DIV = SIF_SET_CLK_DIV(10);
	}
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_Reset(void)
{
	MMP_ULONG	id;

	MMPF_SF_ReadDevId(&id);
	if ((id == 0) || (id == 0xFFFFFF))
		return MMP_SD_ERR_READ_ID;
	
	
	glDevId = id;

	switch(glDevId) {
		case EON_EN25F10:
			glSFTotalSize = 0x20000; //128KB
			glSFSectorSize = 0x1000;
			break;
		case EON_EN25F20:
			glSFTotalSize = 0x40000; //256KB
			glSFSectorSize = 0x1000;
			break;
		case EON_EN25F40:
			glSFTotalSize = 0x80000; //512KB
			glSFSectorSize = 0x1000;
			break;
		case EON_EN25F80:
			glSFTotalSize = 0x100000; //1MB
			glSFSectorSize = 0x1000;
			break;
		case EON_EN25Q16A:
			glSFTotalSize = 0x200000; //2MB
			glSFSectorSize = 0x1000;
			break;
		case EON_EN25Q32A:
			glSFTotalSize = 0x400000; //4MB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25X10:
			glSFTotalSize = 0x20000; //128KB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25X20:
			glSFTotalSize = 0x40000; //256KB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25X40:
			glSFTotalSize = 0x80000; //512KB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25X80:
			glSFTotalSize = 0x100000; //1MB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25Q80:
			glSFTotalSize = 0x100000; //1MB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25Q16:
			glSFTotalSize = 0x200000; //2MB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25Q32:
			glSFTotalSize = 0x400000; //4MB
			glSFSectorSize = 0x1000;
			break;
		case WINBOND_25Q64:
			glSFTotalSize = 0x800000; //8MB
			glSFSectorSize = 0x1000;
			break;
			
		case MXIC_25L512:
			glSFTotalSize = 0x10000;  //64KB
			glSFSectorSize = 0x1000;
			break;
		case MXIC_25L1005:
			glSFTotalSize = 0x20000; //128KB
			glSFSectorSize = 0x1000;
			break;
		case MXIC_25L2005:
			glSFTotalSize = 0x40000; //256KB
			glSFSectorSize = 0x1000;
			break;
		case MXIC_25L4005:
			glSFTotalSize = 0x80000; //512KB
			glSFSectorSize = 0x1000;
			break;
		case MXIC_25L8005:
			glSFTotalSize = 0x100000; //1MB
			glSFSectorSize = 0x1000;
			break;
		case SST_25VF080B:
			glSFTotalSize = 0x100000; //1MB
			glSFSectorSize = 0x1000;
			break;
		default:
			RTNA_DBG_Str(0, "Unknow SF id !!\r\n");
			RTNA_DBG_PrintLong(0, id);
			break;
	}

	return MMPF_SF_WriteStatus(0);
}

MMP_ERR MMPF_SF_GetSFInfo(MMP_ULONG *ulDevId, MMP_ULONG *ulSFTotalSize, MMP_ULONG *ulSFSectorSize)
{
	*ulDevId = glDevId;
	*ulSFTotalSize = glSFTotalSize;
	*ulSFSectorSize = glSFSectorSize;
	
	return MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_ReadDevId(MMP_ULONG *id)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    pSIF->SIF_CMD = READ_DEVICE_ID;
    pSIF->SIF_DMA_ST = m_ulSFDmaAddr;
	pSIF->SIF_DMA_CNT = 3 - 1; // real read 3 byte 
	pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_R | SIF_DMA_EN);

    MMPF_SF_CheckReady();

    *id = (((MMP_ULONG)(*(MMP_UBYTE *)(m_ulSFDmaAddr + 0))) << 16) 
    			+ ((MMP_ULONG)((*(MMP_UBYTE *)(m_ulSFDmaAddr + 1))) << 8)
    			+ (MMP_ULONG)(*(MMP_UBYTE *)(m_ulSFDmaAddr + 2));

	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_ReadData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = READ_DATA;
    
    pSIF->SIF_FLASH_ADDR = ulSFAddr;
    
    pSIF->SIF_DMA_ST = ulDmaAddr;

    pSIF->SIF_DMA_CNT = ulByteCount - 1;
    
    pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_R | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    
    MMPF_SF_CheckReady();
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_FastReadData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = FAST_READ_DATA;
    
    pSIF->SIF_FLASH_ADDR = ulSFAddr;
    
    pSIF->SIF_DMA_ST = ulDmaAddr;

    pSIF->SIF_DMA_CNT = ulByteCount - 1;
    
    pSIF->SIF_CTL =  (SIF_START | SIF_FAST_READ | SIF_DATA_EN | SIF_R | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    
    MMPF_SF_CheckReady();
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_WriteData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_UBYTE ubStatus;
	
	MMPF_SF_EnableWrite();

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;

    pSIF->SIF_CMD = PAGE_PROGRAM;
    
    pSIF->SIF_FLASH_ADDR = ulSFAddr;
    
    pSIF->SIF_DMA_ST = ulDmaAddr;
    
    pSIF->SIF_DMA_CNT = ulByteCount - 1;
    
    pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_W | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    MMPF_SF_CheckReady();
    do {
        MMPF_SF_ReadStatus(&ubStatus);
    } while (ubStatus & WRITE_IN_PROGRESS);
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_EraseSector(MMP_ULONG ulSFAddr)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_UBYTE ubStatus;
	
	MMPF_SF_EnableWrite();

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = SECTOR_ERASE;
    
    pSIF->SIF_FLASH_ADDR = ulSFAddr;
      
    pSIF->SIF_CTL =  (SIF_START | SIF_R | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    MMPF_SF_CheckReady();
    do {
        MMPF_SF_ReadStatus(&ubStatus);
	} while (ubStatus & WRITE_IN_PROGRESS);
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_EraseBlock(MMP_ULONG ulSFAddr)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_UBYTE ubStatus;
	
	MMPF_SF_EnableWrite();

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = BLOCK_ERASE;
    
    pSIF->SIF_FLASH_ADDR = ulSFAddr;
      
    pSIF->SIF_CTL =  (SIF_START | SIF_R | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    MMPF_SF_CheckReady();
    do {
        MMPF_SF_ReadStatus(&ubStatus);
	} while (ubStatus & WRITE_IN_PROGRESS);
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_EraseChip()
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_UBYTE ubStatus;
	
	MMPF_SF_EnableWrite();

    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
    
    pSIF->SIF_CMD = CHIP_ERASE;
      
    pSIF->SIF_CTL = SIF_START;
    MMPF_SF_CheckReady();
    do {
        MMPF_SF_ReadStatus(&ubStatus);
	} while (ubStatus & WRITE_IN_PROGRESS);
	
	return MMP_ERR_NONE;	
}

//AAI Write CMD must write in word
MMP_ERR MMPF_SF_AaiWriteData(MMP_ULONG ulSFAddr, MMP_ULONG ulDmaAddr, MMP_ULONG ulByteCount)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	MMPF_SF_EnableWrite();
	
	MMPF_SF_EBSY();
	
	pSIF->SIF_INT_CPU_SR = SIF_CLR_AAI_CMD_STATUS|SIF_CLR_CMD_STATUS;

    pSIF->SIF_CMD = ADDR_AUTO_INC_WRITE;

    pSIF->SIF_FLASH_ADDR = ulSFAddr;

    pSIF->SIF_DMA_CNT = ulByteCount - 1;
        
    pSIF->SIF_DMA_ST = ulDmaAddr;
    
    pSIF->SIF_CTL2 |= SIF_AAI_MODE_EN;
    
    pSIF->SIF_CTL = SIF_START | SIF_DMA_EN | SIF_DATA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2;
    
    while((pSIF->SIF_INT_CPU_SR & SIF_AAI_CMD_DONE) == 0) ;
    
	MMPF_SF_DBSY();

	MMPF_SF_DisableWrite();
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SF_EBSY(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
    pSIF->SIF_CMD = EBSY;
   
    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_DBSY(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
    pSIF->SIF_CMD = DBSY;
   
    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_ReleasePowerDown(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
    pSIF->SIF_CMD = RELEASE_DEEP_POWER_DOWN;
   
    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();
    
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SF_PowerDown(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = SIF_CLR_CMD_STATUS;
   
    pSIF->SIF_CMD = DEEP_POWER_DOWN;
   
    pSIF->SIF_CTL = SIF_START;
    
    MMPF_SF_CheckReady();
    
    return  MMP_ERR_NONE;
}


#if (CHIP == VSN_V3)
MMP_ERR MMPF_SF_EnableCrcCheck(MMP_BOOL bEnable)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	if(bEnable) {
		pSIF->SIF_BIST_EN |= SIF_BIST_ENABLE;
	}
	else {
		pSIF->SIF_BIST_EN &= (~SIF_BIST_ENABLE);
	}
	
	#if 0//Debug only
	RTNA_DBG_PrintByte(0, pSIF->SIF_BIST_EN);
	#endif
	
	return MMP_ERR_NONE;
}
MMP_ERR MMPF_SF_GetCrcValue(MMP_USHORT* usCrcValue)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	*usCrcValue = pSIF->SIF_CRC_OUT;
	
	return MMP_ERR_NONE;
}
#endif