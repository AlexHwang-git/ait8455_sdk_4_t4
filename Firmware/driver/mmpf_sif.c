#include "config_fw.h"
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_hif.h"
#include "mmpf_sif.h"
#include "mmp_reg_sif.h"
#include "mmp_lib.h"
//#include "mem_map.h"
#include "mmpf_pll.h"
MMP_UBYTE Using_SIF_ID;
#define MXIC_ManufacturerID    0xC2
#define SST_ManufacturerID     0xBF
#define ATMEL_ManufacturerID   0x1F
#define EON_ManufacturerID     0x1C

MMP_USHORT SectorSizeMX25L = 0x1000;

MMP_UBYTE flashReadID = 0x9F; 
MMP_UBYTE eraseChip = 0x60;
MMP_UBYTE eraseSector = 0x20;
MMP_UBYTE eraseBlock = 0xD8; //20110816 minsu, eraseblock cmd is not correct. original value: 0x52

#define VR_MIN(a, b)                        (((a) < (b)) ? (a) : (b))

MMP_ERR MMPF_SIF_CheckReady(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
    
    while((pSIF->SIF_INT_CPU_SR & 0x01) == 0) ;
	
    return  MMP_ERR_NONE;
}
MMP_ERR MMPF_SIF_ReadStatus(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	pSIF->SIF_INT_CPU_SR = 1;
   
   	pSIF->SIF_CMD = MMPF_SIF_STATUS_READ;

   	//pSIF->SIF_DMA_CNT = 0;
    
    pSIF->SIF_CTL = SIF_START | SIF_R | SIF_DATA_EN;
    
    while((pSIF->SIF_INT_CPU_SR & 0x01) == 0) ;
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SIF_WriteStatus(MMP_UBYTE Data)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	pSIF->SIF_INT_CPU_SR = 1;
    
    pSIF->SIF_CMD = MMPF_SIF_STATUS_WRITE;
    
    pSIF->SIF_DATA_WR = Data;
    
    pSIF->SIF_CTL = SIF_START | SIF_W | SIF_DATA_EN;
    
   	while((pSIF->SIF_INT_CPU_SR & 0x01) == 0) ;
    return  MMP_ERR_NONE;
}
MMP_ERR MMPF_SIF_EnableWrite(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = 1;

    pSIF->SIF_CMD = MMPF_SIF_WRITE_ENABLE;
   
   	//pSIF->SIF_DMA_CNT = 0;
    pSIF->SIF_CTL = SIF_START;
    MMPF_SIF_CheckReady();
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SIF_DisableWrite(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
    pSIF->SIF_INT_CPU_SR = 1;
    
    pSIF->SIF_CMD = MMPF_SIF_WRITE_DISABLE;

    pSIF->SIF_CTL = SIF_START;
    MMPF_SIF_CheckReady();

    return  MMP_ERR_NONE;
}

extern MMP_ULONG glGroupFreq[MAX_GROUP_NUM] ;

MMP_ERR MMPF_SIF_Init(void)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	MMP_ULONG frameBuf = SIF_BUFFER_START;
    AITPS_GBL pGBL = AITC_BASE_GBL;
	*(MMP_ULONG *)frameBuf = 0xAAAAAAAA;

	
    pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_BS_SPI_DIS);
    
    pGBL->GBL_IO_CTL1 |= 0x04; //VSN_V2, pad settings
	RTNA_WAIT_MS(5);

    if(glGroupFreq[0] >= 192000) {
   	    pSIF->SIF_CLK_DIV = 3;
   	    RTNA_DBG_Str0("pSIF->SIF_CLK_DIV = 3\r\n");
   	} else {
   	    pSIF->SIF_CLK_DIV = 2;
   	    RTNA_DBG_Str0("pSIF->SIF_CLK_DIV = 2\r\n");
   	}   
    MMPF_SIF_ReadID(flashReadID, frameBuf);
    
   	
    return  MMP_ERR_NONE;
}

MMP_ERR MMPF_SIF_ReadID(MMP_BYTE chipIDCMD, MMP_ULONG fbAddr)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

	pSIF->SIF_INT_CPU_SR = 1;

    pSIF->SIF_CMD = chipIDCMD;
      
    pSIF->SIF_DMA_ST =  fbAddr;

	pSIF->SIF_DMA_CNT = 2;

	pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_R | SIF_DMA_EN);

	while((pSIF->SIF_INT_CPU_SR & 0x01) == 0) ;
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SIF_ReadData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_ULONG Count)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

    pSIF->SIF_INT_CPU_SR = 1;
    
    pSIF->SIF_CMD = MMPF_SIF_DATA_READ;
    
    pSIF->SIF_FRESH_ADDR = destAddr;
    
    pSIF->SIF_DMA_ST = fbAddr;

    pSIF->SIF_DMA_CNT = Count-1;
    
    pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_R | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    MMPF_SIF_CheckReady();
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_MX_SIF_WriteData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_USHORT Count)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

    pSIF->SIF_INT_CPU_SR = 1;

    pSIF->SIF_CMD = MMPF_SIF_PAGE_PROGRAM;
    
    pSIF->SIF_FRESH_ADDR = destAddr;
    
    pSIF->SIF_DMA_CNT = Count;
      
    pSIF->SIF_DMA_ST = fbAddr;
    
    pSIF->SIF_CTL =  (SIF_START | SIF_DATA_EN | SIF_W | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    MMPF_SIF_CheckReady();
    do {
        MMPF_SIF_ReadStatus();
    } while (pSIF->SIF_DATA_RD & 0x01);
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SIF_EraseSector(MMP_ULONG destAddr)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;

    pSIF->SIF_INT_CPU_SR = 1;
    
    pSIF->SIF_CMD = eraseSector;
    
    pSIF->SIF_FRESH_ADDR = destAddr;

	//pSIF->SIF_DMA_CNT = 0;
      
    pSIF->SIF_CTL =  (SIF_START | SIF_W | SIF_ADDR_EN | SIF_ADDR_LEN_2);
    
    MMPF_SIF_CheckReady();
    do {
        MMPF_SIF_ReadStatus();
	} while (pSIF->SIF_DATA_RD & 0x01);
	
	return MMP_ERR_NONE;	
}

MMP_ERR MMPF_SST_SIF_WriteData(MMP_ULONG destAddr, MMP_ULONG fbAddr, MMP_SHORT Count)
{
	AITPS_SIF pSIF = AITC_BASE_SIF;
	
	if(!(Count&1))	//program write in SST25VF080B must in word, not in byte
		return MMP_SYSTEM_ERR_PARAMETER;
	
	pSIF->SIF_INT_CPU_SR = 1;

    pSIF->SIF_CMD = 0xAD;  // Auto Address Increment Programming command

    pSIF->SIF_FRESH_ADDR = destAddr;

    pSIF->SIF_DMA_CNT = 1;
        
    pSIF->SIF_DMA_ST = fbAddr;
    
    pSIF->SIF_CTL = SIF_START | SIF_DATA_EN | SIF_W | SIF_DMA_EN | SIF_ADDR_EN | SIF_ADDR_LEN_2;
    MMPF_SIF_CheckReady();
    
    do {
        MMPF_SIF_ReadStatus();
    } while (pSIF->SIF_DATA_RD & 0x01);  
	Count--;    
    
    for(;Count>0;Count-=2)
    {
    	fbAddr+=2;
		
		pSIF->SIF_INT_CPU_SR = 1;
	    
	    pSIF->SIF_CMD = 0xAD;
	    
	    //pSIF->SIF_CTL = SIF_START;
	    
	    pSIF->SIF_FRESH_ADDR = *(MMP_UBYTE *)(fbAddr+1) + ((*(MMP_UBYTE *)fbAddr)<<8);
	    
	    pSIF->SIF_DMA_CNT = 1;
	           
	    pSIF->SIF_DMA_ST = fbAddr;
	   
	    pSIF->SIF_CTL = (SIF_START|SIF_ADDR_EN|SIF_ADDR_LEN_1);
	    MMPF_SIF_CheckReady();    

        do {
        	MMPF_SIF_ReadStatus();
    	} while (pSIF->SIF_DATA_RD & 0x01);
    }
    
    
    MMPF_SIF_DisableWrite();	
    
    do {
        MMPF_SIF_ReadStatus();
    } while (pSIF->SIF_DATA_RD & 0x01);
  
	
	return MMP_ERR_NONE;	
}
//------------------------------------------------------------------------------
//  Function    : MMPF_SIF_WriteData
//  Description :
//------------------------------------------------------------------------------
/** @brief The function write data to flash.

The function write data to flash.

  @param[in] flashAddress is the flash destination address. size is data size
  @return It reports the status of the operation.
*/
MMP_ERR	MMPF_SIF_WriteData(MMP_ULONG flashAddress, MMP_ULONG fbAddr, MMP_ULONG size)
{
#if 0

	MMP_ULONG  StartAddrOfCurSector, wrsize = 0;
	MMP_USHORT j; 
	MMP_USHORT WrSectorNum;
	MMP_ULONG  i;
	
	i = ((MMP_USHORT)(flashAddress % SectorSizeMX25L) + size);
	WrSectorNum = CEIL(i, SectorSizeMX25L);
	
	StartAddrOfCurSector = (flashAddress & 0xFFFFF000);
	
	for(i = 0 ; i < WrSectorNum ; i++){
		
		MMPF_SIF_ReadData(StartAddrOfCurSector, SIF_BUFFER_START, SectorSizeMX25L);

		for(j = 0 ; flashAddress % SectorSizeMX25L && (wrsize != size) || !j ; flashAddress++ , j++, wrsize++){
			*(MMP_UBYTE *)(SIF_BUFFER_START + (flashAddress % SectorSizeMX25L)) = *(MMP_UBYTE *)(fbAddr + wrsize);
		}

		MMPF_SIF_EnableWrite();
		MMPF_SIF_EraseSector(StartAddrOfCurSector);
		
		for(j = 0 ; j < SectorSizeMX25L ; j += 0x100){
		
			MMPF_SIF_EnableWrite();
			
			if(Using_SIF_ID == SST_ManufacturerID)
				MMPF_SST_SIF_WriteData(StartAddrOfCurSector + j, SIF_BUFFER_START + j, 0xFF);	
			else  //if((Using_SIF_ID == MXIC_ManufacturerID) || (Using_SIF_ID == ATMEL_ManufacturerID) || (Using_SIF_ID == EON_ManufacturerID))
				MMPF_MX_SIF_WriteData(StartAddrOfCurSector + j, SIF_BUFFER_START + j, 0xFF);
					
		}
		
		StartAddrOfCurSector += SectorSizeMX25L;
	}

#else

	MMP_ULONG  i;

	for (i = 0; size > 0; i++) {

		if (i % 16 == 0) {
			MMPF_SIF_EnableWrite();
			MMPF_SIF_EraseSector(flashAddress + i * 256);
		}

		MMPF_SIF_EnableWrite();
		if(Using_SIF_ID == SST_ManufacturerID)
			MMPF_SST_SIF_WriteData(flashAddress + i * 256, fbAddr + i * 256, 0xFF);	
		else  //if((Using_SIF_ID == MXIC_ManufacturerID) || (Using_SIF_ID == ATMEL_ManufacturerID) || (Using_SIF_ID == EON_ManufacturerID))
		    MMPF_MX_SIF_WriteData(flashAddress + i * 256, fbAddr + i * 256, 0xFF);	

		size -= VR_MIN(256, size);
	}


#endif

	return MMP_ERR_NONE;	

}
