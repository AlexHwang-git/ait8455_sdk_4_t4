//==============================================================================
//
//  File        : pCam_dfu.c
//  Description : pcam firmware download / update API.
//
//==============================================================================
#include "includes_fw.h"
#include "mmpf_typedef.h"
#include "mmp_reg_gpio.h"
#include "lib_retina.h"
//#include "mmpf_sif.h"
#include "mmpf_sf.h"
//#include "3A_ctl.h"
#include "pcam_dfu.h"
#define VA_MIN(a, b)                        (((a) < (b)) ? (a) : (b))

//==============================================================================
//
//                              Define
//
//==============================================================================
// ----------------------------------------------------

MMP_UBYTE *FirmwareBinPtr;
MMP_ULONG FirmwareBinSize;

MMP_UBYTE *USBFirmwareBinPtr;
MMP_ULONG USBFirmwareBinSize = 0;


//MMP_USHORT AIT_BOOTCODE_FLASH_SIZE ,AIT_FW_FLASH_ADDR ;
MMP_ULONG BOOT_CODE_FLASH_ADDR[] =
{
    AIT_BOOTCODE_FLASH_ADDR , // first 4K
    AIT_BOOTCODE_FLASH_ADDR + AIT_BC1_FLASH_SIZE // next nK in sram
} ;

#define PCCAM_FW_SIZE_OFFSET_IN_FLASH    0x2C
#define PCCAM_FW_CHECKSUM_OFFSET_IN_FLASH    0x30
#define VSN_V3_ID		0x334e5356

//==============================================================================
//
//                              External
//
//==============================================================================

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//==============================================================================
//  Function    : CE_JOB_DISPATCH_Task
//  Description : Main function for CE Job Dispatch
//  Parameters  :
//  Return value:
//==============================================================================
//#undef BUILD_FW

MMP_ERR USB_Read840BootFirmware(MMP_UBYTE *FWImagePtr, MMP_ULONG FWImageSize)
{
    MMP_ULONG section_offset, section_size;
    MMP_UBYTE *fw_ptr;
#if (CHIP==P_V2)
    RTNA_DBG_Str(0, "USB_Read8423BootFirmware : start\r\n");
#endif
#if (CHIP==VSN_V2)||(CHIP==VSN_V3)
    RTNA_DBG_Str(0, "USB_Read845xBootFirmware : start\r\n");
#endif

    FirmwareBinPtr = (MMP_UBYTE *) (FW_DOWNLOAD_BUFFER); 
    fw_ptr = (MMP_UBYTE *) FWImagePtr;

    RTNA_DBG_Str(0, "FW start address = ");
    RTNA_DBG_Long(0, (MMP_ULONG) FWImagePtr);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "FirmwareBinPtr = ");
    RTNA_DBG_Long(0, (MMP_ULONG) FirmwareBinPtr);
    RTNA_DBG_Str(0, "\r\n");

    if(fw_ptr[0] != 'A' || fw_ptr[1] != 'I' || fw_ptr[2] != 'T') {
        RTNA_DBG_Str(0, "840 FW format is incorrect....\r\n");
        return 1;
    }

#if 1
    if( (fw_ptr[9]=='R')&&(fw_ptr[10]=='A') && (fw_ptr[11]=='W') ) {
        dbg_printf(0,"#Raw file\r\n");
        return 2;
    }
#endif

    // search section 1 start address and section size
    section_offset = fw_ptr[0x0A] + (fw_ptr[0x0B] << 8) + (fw_ptr[0x0C] << 16) + (fw_ptr[0x0D] << 24) + 0x0A;
    section_size = fw_ptr[0x0E] + (fw_ptr[0x0F] << 8) + (fw_ptr[0x10] << 16) + (fw_ptr[0x11] << 24);

    if(section_size > FWImageSize) {
        RTNA_DBG_Str(0, "Section size is incorrect....\r\n");
        return 1;
    }


    FirmwareBinSize = section_size;
    #if (CHIP==P_V2)
    if(FirmwareBinSize <= 0x3000 ) {// 12KB boot code size 
        AIT_BOOTCODE_FLASH_SIZE =  0x3000 ;
        AIT_FW_FLASH_ADDR = 0x3000;   
    } else {
        AIT_BOOTCODE_FLASH_SIZE =  0x8000 ;
        AIT_FW_FLASH_ADDR = 0x8000;   
    }
    #endif
    dbg_printf(0,"#[Boot]Sec Off=%x,Sec Size=%x\r\n",section_offset,FirmwareBinSize);
    dbg_printf(0,"#[Boot]FW Off=%x,Boot Size=%x\r\n",AIT_FW_FLASH_ADDR,AIT_BOOTCODE_FLASH_SIZE);

    MEMCPY(FirmwareBinPtr, (MMP_UBYTE *) (fw_ptr + section_offset), FirmwareBinSize);

    RTNA_DBG_Str(0, "USB_Read845x BootFirmware : end\r\n");

    return 0;
}


MMP_ERR USB_Read840PCCAMFirmware(MMP_UBYTE *FWImagePtr, MMP_ULONG FWImageSize)
{
    MMP_ULONG section_offset, section_size, offset_tmp;
    MMP_UBYTE *fw_ptr;

    RTNA_DBG_Str(0, "USB_Read8423PCCAMFirmware : start\r\n");

    FirmwareBinPtr = (MMP_UBYTE *) (FW_DOWNLOAD_BUFFER); 
    fw_ptr = (MMP_UBYTE *) FWImagePtr;

    RTNA_DBG_Str(0, "FW start address = ");
    RTNA_DBG_Long(0, (MMP_ULONG) FWImagePtr);
    RTNA_DBG_Str(0, "\r\n");
    RTNA_DBG_Str(0, "FirmwareBinPtr = ");
    RTNA_DBG_Long(0, (MMP_ULONG) FirmwareBinPtr);
    RTNA_DBG_Str(0, "\r\n");

    if(fw_ptr[0] != 'A' || fw_ptr[1] != 'I' || fw_ptr[2] != 'T') {
        RTNA_DBG_Str(0, "84x FW format is incorrect....\r\n");
        return 1;
    }

    // search section 1 start address and section size
    section_offset = fw_ptr[0x0A] + (fw_ptr[0x0B] << 8) + (fw_ptr[0x0C] << 16) + (fw_ptr[0x0D] << 24) + 0x0A;
    section_size = fw_ptr[0x0E] + (fw_ptr[0x0F] << 8) + (fw_ptr[0x10] << 16) + (fw_ptr[0x11] << 24);

    // search section 2 start address and section size
    offset_tmp = (section_offset + section_size);  
    section_offset = (offset_tmp +
                      fw_ptr[offset_tmp] +
                      (fw_ptr[offset_tmp + 1] << 8) +
                      (fw_ptr[offset_tmp + 2] << 16) +
                      (fw_ptr[offset_tmp + 3] << 24));
    section_size = fw_ptr[offset_tmp + 4] +
                   (fw_ptr[offset_tmp + 5] << 8) +
                   (fw_ptr[offset_tmp + 6] << 16) +
                   (fw_ptr[offset_tmp + 7] << 24);

    if(section_size > FWImageSize) {
        RTNA_DBG_Str(0, "Section size is incorrect....\r\n");
        return 1;
    }


    FirmwareBinSize = section_size;


    dbg_printf(0,"#[Fw]Sec Off=%x,Sec Size=%x\r\n",section_offset,FirmwareBinSize);

    MEMCPY(FirmwareBinPtr, (MMP_UBYTE *) (fw_ptr + section_offset), FirmwareBinSize);

    RTNA_DBG_Str(0, "USB_Read840PCCAMFirmware : end\r\n");

    return 0;
}




MMP_ERR USB_DownloadFWInitialize(void)
{
    RTNA_DBG_Str(0, "USB_DownloadFWInitialize Begin...\r\n");

    ISP_IF_3A_Control(ISP_3A_PAUSE);

    // Initialize USB UVC firmware download parameter
    USBFirmwareBinPtr = (MMP_UBYTE *) (FW_DOWNLOAD_BUFFER + AIT_BOOTCODE_FLASH_SIZE * 4); 
    USBFirmwareBinSize = 0;

    USB_LEDDisplay(LED_READY, LED_ON);

    RTNA_DBG_Str(0, "USB_DownloadFWInitialize End...\r\n");

    return 0;
}

MMP_ERR USB_DownloadFWData(MMP_UBYTE *data, MMP_USHORT len)
{
    int n;
    for(n = 0;n < len;++n) {
        USBFirmwareBinPtr[USBFirmwareBinSize + n] = data[n];
    }
    USBFirmwareBinSize += len;

    return 0;
}

MMP_ERR USB_Burning840FW2SFlash(void)
{
    MMP_ERR   status;

    status = USB_Read840BootFirmware(USBFirmwareBinPtr, USBFirmwareBinSize);
    if(status==1) {
        RTNA_DBG_Str(0, "USB_Read8423BootFirmware fail\r\n");
        return 1;
    }

#if (CHIP == VSN_V2)||(CHIP==VSN_V3)
    status = Burning845xFWImage();
    if(status) {
        RTNA_DBG_Str(0, "Burning845xFWImage() fail\r\n");
        return 1;
    }
#endif    

	ISP_IF_3A_Control(ISP_3A_RECOVER);
    USBFirmwareBinSize = 0;

    USB_LEDDisplay(LED_READY, LED_OFF);

    RTNA_DBG_Str(0, "=== Burning845xFWImage() successfully ===\r\n");
    return 0;
}


#if (CHIP==VSN_V2)||(CHIP==VSN_V3)
#define MAX_UPDATE_PARTITION_NUM   2
#if SUPPORT_2FW_MODE==0
#define MAX_FLASH_PARTITION_NUM    2
#else
#define MAX_FLASH_PARTITION_NUM    3
#endif
#define SUPPORT_2FW_MIN_FLASH_SIZE 0x100000

typedef struct _StorageLayout_t
{
    MMP_ULONG tag_id ;
    MMP_ULONG flag ;
    MMP_ULONG blksize ;
    MMP_ULONG blks ;
    MMP_ULONG offset ;
    MMP_USHORT crc ;
} StorageLayout_t ;

StorageLayout_t gsStorageLayout[MAX_UPDATE_PARTITION_NUM] ;
StorageLayout_t gsStorageLayoutFlash[MAX_FLASH_PARTITION_NUM] ;

MMP_USHORT USB_GetStorageLayout(MMP_UBYTE *fwptr,MMP_ULONG fwsize)
{
    MMP_ULONG totalsize = fwsize,offset =0 ;
    MMP_UBYTE *curaddr ;
    MMP_ULONG *ptr,i=0,partsize = 0;
    curaddr = fwptr ;
    gsStorageLayout[i].offset=0;
    while(totalsize) {
        //dbg_printf(3,"Bin file addr[%d]:%x\r\n",i,curaddr);
        ptr = (MMP_ULONG *)curaddr ;
        gsStorageLayout[i].tag_id  = *ptr++ ;
        gsStorageLayout[i].flag    = *ptr++;
        gsStorageLayout[i].blksize = (1 << *ptr++) ;
        gsStorageLayout[i].blks = *ptr ;
        gsStorageLayout[i].crc = *((MMP_USHORT*)(curaddr + gsStorageLayout[i].blksize ));
        gsStorageLayout[i].offset += partsize ;
        
        partsize = gsStorageLayout[i].blks * gsStorageLayout[i].blksize ;
        curaddr += partsize ;
        totalsize-=partsize ;
    	//for(i=0;i<parts;i++) {
    	#if 0
    	    dbg_printf(3,"<offset : %x>\r\n"    ,gsStorageLayout[i].offset );
    	    dbg_printf(3,"<tag : %x>\r\n"    ,gsStorageLayout[i].tag_id );
    	    dbg_printf(3,"<flag : %x>\r\n"   ,gsStorageLayout[i].flag );
    	    dbg_printf(3,"<blksize : %x>\r\n",gsStorageLayout[i].blksize );
    	    dbg_printf(3,"<blks : %x>\r\n"   ,gsStorageLayout[i].blks );
    	    dbg_printf(3,"<crc : %x>\r\n"    ,gsStorageLayout[i].crc);
        #endif    	    
    	//}
        i++ ;
    	if(i>=MAX_UPDATE_PARTITION_NUM) {
    	    break ;
    	}
    }
    
    return i;
}


MMP_USHORT USB_GetStorageLayoutFromFlash(void)
{
    MMP_ULONG i,offset=0,partsize = 0,flash_partition = 0;
    MMP_ULONG *ptr ;//= (MMP_ULONG*)SIF_BUFFER_START;
    MMP_ULONG   sf_id,sf_size,sf_sector_size ;
    //dbg_printf(3,"<flash layout>\r\n");  
    
	MMPF_SF_GetSFInfo(&sf_id,&sf_size,&sf_sector_size);
    if(sf_size >= SUPPORT_2FW_MIN_FLASH_SIZE ) {
        flash_partition = MAX_FLASH_PARTITION_NUM ;
    }
    else {
        flash_partition = 2 ;// Flash is too small, force 1FW mode
    }
     
    for(i=0;i<MAX_FLASH_PARTITION_NUM;i++) {
	    //dbg_printf(3,"<offset %d: %x>\r\n"    ,i,offset );
	    ptr = (MMP_ULONG*)SIF_BUFFER_START;
        MMPF_SF_FastReadData(offset , SIF_BUFFER_START, 16);
        gsStorageLayoutFlash[i].tag_id  = *ptr++ ;
        gsStorageLayoutFlash[i].flag    = *ptr++;
        gsStorageLayoutFlash[i].blksize = (1 << *ptr++) ;
        gsStorageLayoutFlash[i].blks = *ptr ;
       // gsStorageLayoutFlash[i].crc = *((MMP_USHORT*)(curaddr + gsStorageLayout[i].blksize ));
        
        ptr = (MMP_ULONG*)SIF_BUFFER_START;
        //dbg_printf(3,"read crc from :%x\r\n",offset + gsStorageLayoutFlash[i].blksize);
        MMPF_SF_FastReadData(offset + gsStorageLayout[i].blksize, SIF_BUFFER_START, 16);
        gsStorageLayoutFlash[i].crc = *(MMP_USHORT *)ptr ;
        
        partsize = gsStorageLayoutFlash[i].blks * gsStorageLayoutFlash[i].blksize ;
        gsStorageLayoutFlash[i].offset = offset ;
        offset+=partsize;
        
        #if 0
	    dbg_printf(3," <offset : %x>\r\n"    ,gsStorageLayoutFlash[i].offset );
	    dbg_printf(3," <tag : %x>\r\n"    ,gsStorageLayoutFlash[i].tag_id );
	    dbg_printf(3," <flag : %x>\r\n"   ,gsStorageLayoutFlash[i].flag );
	    dbg_printf(3," <blksize : %x>\r\n",gsStorageLayoutFlash[i].blksize );
	    dbg_printf(3," <blks : %x>\r\n"   ,gsStorageLayoutFlash[i].blks );
	    dbg_printf(3," <crc : %x>\r\n"   ,gsStorageLayoutFlash[i].crc );
	    #endif
    }
}
#if 0
MMP_ERR	SaveVideoPro(void)
{
	MMP_BYTE	i;
	MMP_BYTE	*pro_dma_buff;
	pro_dma_buff = (MMP_BYTE *)VideoPreperty_BUF_STA;
	MMPF_SF_ReadData(VideoPreperty_FLASH_ADDR,VideoPreperty_BUF_STA,VideoPreperty_FLASH_SIZE);
	dbg_printf(0,"videopro[0] = %d \r\n", *pro_dma_buff);
	*pro_dma_buff = *pro_dma_buff + 1;
	*(pro_dma_buff+1) = *(pro_dma_buff+1) + 1;
	*(pro_dma_buff+2) = *(pro_dma_buff+2) + 1;
	*(pro_dma_buff+3) = *(pro_dma_buff+3) + 1;
	*(pro_dma_buff+4) = *(pro_dma_buff+4) + 1;
	MoveData2SIF(VideoPreperty_FLASH_ADDR,VideoPreperty_BUF_STA,VideoPreperty_FLASH_SIZE);	
	MMPF_SF_ReadData(VideoPreperty_FLASH_ADDR,VideoPreperty_BUF_STA,VideoPreperty_FLASH_SIZE);
	for(i=0; i<VideoPreperty_FLASH_SIZE;i++)
		dbg_printf(0,"write videopro[%x] = %d \r\n",i, *(pro_dma_buff+i));
}
MMP_ERR	GetVideoPro(void)
{
	MMP_BYTE	i;
	MMP_BYTE	*pro_dma_buff;
	pro_dma_buff = (MMP_BYTE *)VideoPreperty_BUF_STA;
	MMPF_SF_ReadData(VideoPreperty_FLASH_ADDR,VideoPreperty_BUF_STA,VideoPreperty_FLASH_SIZE);
	for(i=0; i<VideoPreperty_FLASH_SIZE;i++)
		dbg_printf(0,"read videopro[%x] = %d \r\n",i, *(pro_dma_buff+i));

}
#endif

MMP_ERR Burning845xFWImage(void)
{
#define BIT_WRITE_EN	(1<<1)
//	MMP_UBYTE	ubManufacturerID;
//  MMP_USHORT	usDeviceID;

    StorageLayout_t newlayout[MAX_FLASH_PARTITION_NUM] ;

    MMP_UBYTE	*curAddr;
    MMP_USHORT	CRC,parts,i;
	MMP_ULONG   *ulPtr;
	MMP_ULONG   ulTotalSize, ulPartSize, ulBlockSize, ulFlag, ulSIFAddr, ulSIFOfst;
    MMP_ERR     err ;
    MMP_ULONG   sf_id,sf_size,sf_sector_size ;
    
	// no 18bytes FW_INFO section now
	curAddr = (MMP_UBYTE *)(FirmwareBinPtr);
	ulSIFAddr = 0;

	ulTotalSize = FirmwareBinSize;
	RTNA_DBG_Str(0, "FirmwareSize = ");
	RTNA_DBG_Long(0, ulTotalSize);
	RTNA_DBG_Str(0, "\r\n");
	
	parts = USB_GetStorageLayout(FirmwareBinPtr,FirmwareBinSize);
 	
 	
 	
	MMPF_SF_SetTmpAddr((MMP_ULONG)SIF_BUFFER_START);
	MMPF_SF_InitialInterface();
    err = MMPF_SF_Reset() ;
	if (err ) {
		RTNA_DBG_Str(0, "\r\nSIF init error !!\r\n");
		return err ;
	}
	MMPF_SF_GetSFInfo(&sf_id,&sf_size,&sf_sector_size);
	dbg_printf(3,"Flash Id :%x,size:%x\r\n",sf_id,sf_size);
	USB_GetStorageLayoutFromFlash();
	
	
 	if(gsStorageLayout[0].tag_id==VSN_V3_ID ) {
 	    if(gsStorageLayout[0].flag & BIT_WRITE_EN) {
 	    // Write BootCode
 	        dbg_printf(3,"BC will be updated\r\n");
 	        newlayout[0] = gsStorageLayout[0]; // bootcode
 	        newlayout[1] = gsStorageLayout[1]; // FW 1
 	        newlayout[1].flag |= BIT_WRITE_EN ;
 	        #if SUPPORT_2FW_MODE==1
 	        if(sf_size >= SUPPORT_2FW_MIN_FLASH_SIZE) { // protect if flash size is not enough
 	            newlayout[2] = gsStorageLayout[1]; // FW 2
 	            newlayout[2].offset = gsStorageLayout[1].offset + gsStorageLayout[1].blks * gsStorageLayout[1].blksize ;
 	            newlayout[2].flag |= BIT_WRITE_EN ;
 	        }
 	        #endif
 	    }
 	    else {
 	        newlayout[0] = gsStorageLayoutFlash[0]; // Keep BC
 	        newlayout[0].flag &= ~BIT_WRITE_EN ;
 	        newlayout[0].offset = 0;
 	        
 	    #if SUPPORT_2FW_MODE==1
 	        if(sf_size >= SUPPORT_2FW_MIN_FLASH_SIZE) { // protect if flash size is not enough
     	        if(gsStorageLayout[1].tag_id==VSN_V3_ID) { // FW 1 is valid
     	            newlayout[1] = gsStorageLayoutFlash[1] ;
     	            newlayout[1].flag &= ~BIT_WRITE_EN ;   // Keep FW 1
     	            newlayout[1].offset = newlayout[0].offset + newlayout[0].blks * newlayout[0].blksize ;
     	            
     	            newlayout[2] = gsStorageLayout[1] ;    // Update FW 2
     	            newlayout[2].offset = newlayout[1].offset + newlayout[1].blks * newlayout[1].blksize ;
     	        } else {    // FW 1 is invalid
     	            newlayout[1] = gsStorageLayout[1]; // Update FW 1->1
     	            newlayout[1].offset = newlayout[0].offset + newlayout[0].blks * newlayout[0].blksize ;
      	            newlayout[2] = gsStorageLayout[1]; // Update FW 1->2
     	            newlayout[2].offset = newlayout[1].offset + newlayout[1].blks * newlayout[1].blksize ;
    	            
     	        }
 	        }
 	        else {
     	        newlayout[1] = gsStorageLayout[1] ;
     	        newlayout[1].offset = newlayout[0].offset + newlayout[0].blks * newlayout[0].blksize ;
 	        }
 	    #else
 	        newlayout[1] = gsStorageLayout[1] ;
 	        newlayout[1].offset = newlayout[0].offset + newlayout[0].blks * newlayout[0].blksize ;
 	    #endif
 	    }
 	}
	for(i=0;i<MAX_FLASH_PARTITION_NUM;i++) {
	    #if 0
 	    dbg_printf(3,"New flash layout %d:%x->%x\r\n",i,curAddr,newlayout[i].offset);
	    dbg_printf(3," <tag : %x>\r\n"    ,newlayout[i].tag_id );
	    dbg_printf(3," <flag : %x>\r\n"   ,newlayout[i].flag );
	    dbg_printf(3," <blksize : %x>\r\n",newlayout[i].blksize );
	    dbg_printf(3," <blks : %x>\r\n"   ,newlayout[i].blks );
	    dbg_printf(3," <crc : %x>\r\n"    ,newlayout[i].crc );
	    #endif
	    if( ( newlayout[i].flag & BIT_WRITE_EN) && (newlayout[i].tag_id==VSN_V3_ID) ) {
			MMPF_SF_EraseSector(newlayout[i].offset);
			MMPF_SF_EraseSector(newlayout[i].offset + newlayout[i].blksize);
			MoveData2SIF(newlayout[i].offset + 2 * newlayout[i].blksize , (MMP_ULONG)(curAddr+2 * newlayout[i].blksize), ((newlayout[i].blks-2) * newlayout[i].blksize));
			MoveData2SIF(newlayout[i].offset, (MMP_ULONG)(curAddr), 2 * newlayout[i].blksize );
			dbg_printf(3,"CRC[%x,%x]\r\n",newlayout[i].crc,*(MMP_ULONG *)( curAddr + newlayout[i].blksize ) );
			dbg_printf(3,"Write [%d] : Addr :%x->%x,Size : %x ,ID : %x\r\n",i,curAddr,newlayout[i].offset,newlayout[i].blks * newlayout[i].blksize ,*(MMP_ULONG *)curAddr);
	    }
	    if(i==0) {
            curAddr += newlayout[i].blks * newlayout[i].blksize ; 
        }
	}
	
	
	return MMP_ERR_NONE;
}

//Note: ulSifAddr should be 4KB alignmented address
void MoveData2SIF(MMP_ULONG ulSifAddr, MMP_ULONG ulSourceAddr, MMP_ULONG ulDataSize)
{
	MMP_ULONG ulNumOf4kbAlign = (ulDataSize >> 12) + 1;
	MMP_ULONG i = 0x0, j = 0x0;
	const MMP_USHORT usSifAlignmentSize = 0x1000;
	const MMP_USHORT usSifWriteUnit = 0x100;
	const MMP_UBYTE	 ubSifWriteCycle = (usSifAlignmentSize/usSifWriteUnit);
	
	#if 0 //Debug only
	MMPF_SF_ReadData(0x1000, (MMP_ULONG)&data1, 4);
	RTNA_DBG_PrintLong(0, data1);	
	#endif
	for(i = 0x0; i < ulNumOf4kbAlign; i++) {
		MMPF_SF_EraseSector(ulSifAddr);
		for(j = 0x0; j < ubSifWriteCycle; j++) {	
			if(ulDataSize <= usSifWriteUnit)  {
				MMPF_SF_WriteData(ulSifAddr, ulSourceAddr, ulDataSize);
				return;
			}
			else {
				MMPF_SF_WriteData(ulSifAddr, ulSourceAddr, usSifWriteUnit);
				ulSifAddr += usSifWriteUnit;
				ulSourceAddr += usSifWriteUnit;
				ulDataSize -= usSifWriteUnit;
			}
		}
	}	
}

MMP_ERR GetSIFCRCValue(MMP_USHORT* crc)
{
	MMP_ULONG   *ulPtr;
	MMP_ULONG	ulSIFAddr = 0, ulPartSize = 0;
	MMP_UBYTE	i;
	
	MMPF_SF_InitialInterface();
	ulPtr = (MMP_ULONG*)SIF_BUFFER_START;

#if 0
	MMPF_SF_SetTmpAddr((MMP_ULONG)SIF_BUFFER_START);
	if (MMPF_SF_Reset()) {
		RTNA_DBG_Str(0, "\r\nSIF init error !!\r\n");
	}
#endif
	
	for (i = 0; i < 3; i++){
		// Read header and parse header info to get next section addr	
		MMPF_SF_FastReadData(ulSIFAddr, (MMP_ULONG)SIF_BUFFER_START, 16);	
		
		if (*ulPtr == VSN_V3_ID)	{
			ulPartSize = *(ulPtr+3);
			
			// Read CRC
			MMPF_SF_FastReadData((ulSIFAddr+0x1000), (MMP_ULONG)SIF_BUFFER_START, 2);
			crc[i] = *((MMP_USHORT*)ulPtr);
						
			ulSIFAddr += (ulPartSize)*0x1000;
			dbg_printf(3, "Addr = x%x, CRC[%d] = x%X\r\n", ulSIFAddr, i, crc[i]);		
		} else {	
			dbg_printf(3, "Invalid section.\r\n");
			crc[i] = 0;
		}
	}
	
	return MMP_ERR_NONE;
}

#endif

extern MMP_UBYTE IQID;		//AlexH @ 2020/08/25


void SetSerialNumber2Flash(MMP_ULONG ulSifAddr, MMP_ULONG ulSourceAddr)
{
	MMP_BYTE	buff_index;
	MMP_USHORT	checksum = 0;
	MMP_BYTE	*serial_number_dma_buff;
	serial_number_dma_buff = (MMP_BYTE *)SERIAL_NUMBER_WRITE_BUF_STA;
	
	#if 0		//AlexH @ 2020/08/25
	
	for(buff_index = 0; buff_index <= 19; buff_index++)
	{
		checksum += *(serial_number_dma_buff + buff_index);
	}
	*(serial_number_dma_buff + 20) = checksum&0x00FF;
	*(serial_number_dma_buff + 21) = (checksum >> 8)&0xFF;
	MoveData2SIF(ulSifAddr+1, ulSourceAddr, 23);			// shift 1 byte
	
	#else
	
	for(buff_index = 0; buff_index <= 20; buff_index++)
	{
		checksum += *(serial_number_dma_buff + buff_index);
	}
	*(serial_number_dma_buff + 21) = checksum&0x00FF;
	*(serial_number_dma_buff + 22) = (checksum >> 8)&0xFF;
	*(serial_number_dma_buff + 23) = IQID;
	
	MoveData2SIF(ulSifAddr+1, ulSourceAddr, 24);			// shift 1 byte

	#endif
}

void GetSerialNumberFromFlash(MMP_ULONG ulSifAddr, MMP_ULONG ulDumpBuffAddr)
{
	MMP_BYTE	buff_index;
	MMP_USHORT	checksum = 0;
	//MMP_USHORT checksum1 = 0;
	MMP_BYTE	*serial_number_dma_buff;
	serial_number_dma_buff = (MMP_BYTE *)SERIAL_NUMBER_READ_BUF_STA;

	MMPF_SF_ReadData(ulSifAddr, ulDumpBuffAddr, 23);
	
	//*(MMP_BYTE *)(ulDumpBuffAddr+23) = 0x11;
	
	for(buff_index = 1; buff_index <= 20; buff_index++)
	{
		checksum += *(serial_number_dma_buff + buff_index);
	}
	//checksum1 = (*(serial_number_dma_buff + 22) << 8) + *(serial_number_dma_buff + 21);
	//dbg_printf(3, "checkH %x  checkL %x  checksum = %x \r\n", *(serial_number_dma_buff + 22), *(serial_number_dma_buff + 21),checksum);
	if(checksum != (*(serial_number_dma_buff + 22) << 8) + *(serial_number_dma_buff + 21))
	{
		*serial_number_dma_buff = 1;
	}else *serial_number_dma_buff = 0;
	
	#if 1	//AlexH @ 2020/08/25
	if(	IQID != *(serial_number_dma_buff + 23))
		*(serial_number_dma_buff + 23) = IQID;
	#endif
}

//#define BUILD_FW
