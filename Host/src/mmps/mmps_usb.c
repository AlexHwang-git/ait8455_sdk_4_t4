//==============================================================================
//
//  File        : mmps_usb.c
//  Description : Usb User Application Interface
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================
/**
 @file mmps_usb.c
 @brief The USB control functions
 @author Hans Liu
 @version 1.0
*/
#ifdef BUILD_FW
#include "config_fw.h"
#endif
#include "mmp_lib.h"
#include "mmpd_usb.h"
//#include "mmps_usb.h"

#include "mmps_dsc.h"
#include "mmps_system.h"
#include "mmpd_system.h"
#include "mmph_hif.h"
#include "ait_utility.h"
/** @addtogroup MMPS_USB
@{
*/

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================

#if (SUPPORT_DPS_FUNC==1)
///DPS Print Information
static MMPS_USB_DPS_PRINT_INFO   m_DpsPrintInfo;
///DPS Buffer Size to Host
static MMP_ULONG        m_ulDPSBuf4host;
#endif // (SUPPORT_DPS_FUNC==1)

/// Usb Memory Allocation Information
static MMPS_USBMODE_BUFFER_INFO m_UsbBufInfo;

#if(SUPPORT_HOST_STORAGE_DEVICE)        
static MMP_ULONG m_DevSize = 0;
static MMP_ULONG m_devID = 0;
static MMP_ULONG m_ulMemDevCmdAddr= 0;
static MMP_ULONG m_ulMemDevAckAddr= 0;
static MMP_ULONG m_ulMaxBufSec= 0;
static void *m_pMemDevBuf;
static MMPS_USB_FUNC *pUSBFunc;
#endif

//please update it after FS_MS_API ready
#define FS_MS_ERROR_NONE    FS_MS_OK
#define FS_MS_ERROE_FAIL    FS_MS_ERROR
//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

//==============================================================================
//  Function    : MMPS_USB_GetConfiguration
//==============================================================================
/** 
    @brief  Get Usb Configuration Structure
    @return m_UsbBufInfo
*/
MMPS_USBMODE_BUFFER_INFO*  MMPS_USB_GetConfiguration(void)
{
    return &m_UsbBufInfo;
}

//==============================================================================
//  Function    : MMPS_USB_SetMode
//==============================================================================
/** 
    @brief  Set usb mode
    @param[in] ubMode : usb mode(MSDC,PCSYNC,...)
    @return MMP_ERR_NONE
*/
#if PCAM_EN==0
MMP_ERR MMPS_USB_SetMode(MMPS_USB_OP_MODE ubMode)
{
    MMP_ULONG buf_start, cur_buf;
    MMP_ULONG name_buf_size, parameter_buf_size, sd_buf_size, sm_buf_size;
    MMP_ULONG ctl_buf_size, msdc_buf_size, pcsync_inbuf_size, pcsync_outbuf_size;

	#if (SUPPORT_PCCAM_FUNC == 1)
    MMP_ULONG pccam_compbuf_size, pccam_linebuf_size;
	#endif

	#if (SUPPORT_MTP_FUNC == 1)
    MMP_ULONG mtp_transferbuf_size;
	#endif

    sd_buf_size =  m_UsbBufInfo.ulSdBufSize;
    sm_buf_size = m_UsbBufInfo.ulSmBufSize;  
    
    
	switch (ubMode) {
	case MMPS_USB_MSDC_MODE:
        /*Step1: disable cachable and allocate SRAM buffer*/
        buf_start = 0x110000;
        
        ctl_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MSDC_MODE].ulCtlBufSize;
        msdc_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MSDC_MODE].ulMSDCBufSize;
        cur_buf = buf_start;
            
        //MMPD_USB_SetMSDCBuf(cur_buf,msdc_buf_size);
        //cur_buf +=msdc_buf_size;

    #if 0
        MMPD_USB_SetCtlBuf(cur_buf,ctl_buf_size);  
    #else
		MMPF_SetUsbCtlBuf(cur_buf);  // for non-OS task  
    #endif        
        cur_buf +=ctl_buf_size;

        /*Step1: Allocate DRAM buffer*/
		//MMPD_System_GetFWEndAddress(&buf_start);
        //cur_buf = ALIGN32(buf_start);

        #if(SUPPORT_HOST_STORAGE_DEVICE)        
        m_ulMemDevCmdAddr= ALIGN32(cur_buf);
        m_ulMemDevAckAddr= cur_buf+MemDevBufferSize1;
        PRINTF("m_ulMemDevCmdAddr=%x, m_ulMemDevAckAddr=%x\r\n",m_ulMemDevCmdAddr,m_ulMemDevAckAddr);
      
        MMPD_USB_SetMemDevBuf(m_ulMemDevCmdAddr, m_ulMemDevAckAddr);
        cur_buf +=( MemDevBufferSize1 +  MemDevBufferSize2);
        #endif

        
		break;
	case MMPS_USB_PCSYNC_MODE:
    #if (SUPPORT_PCSYNC_FUNC==1)	
		MMPD_System_GetFWEndAddress(&buf_start);
        ctl_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCSYNC_MODE].ulCtlBufSize;
        pcsync_inbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCSYNC_MODE].ulPCSYNCInBufSize;
        pcsync_outbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCSYNC_MODE].ulPCSYNCOutBufSize;
        cur_buf = buf_start;
        
        MMPD_USB_SetCtlBuf(cur_buf,ctl_buf_size);
        cur_buf+=ctl_buf_size;
      
        MMPD_USB_SetPCSYNC_InBuf(cur_buf,pcsync_inbuf_size);
        cur_buf+=pcsync_inbuf_size;
      
        MMPD_USB_SetPCSYNC_OutBuf(cur_buf,pcsync_outbuf_size);
        cur_buf+=pcsync_outbuf_size;
      
        MMPD_USB_SetPCSYNC_HandShakeBuf(cur_buf,MMPD_USB_PCSYNC_HANDSHAKE_BUF_SIZE);
        cur_buf+=MMPD_USB_PCSYNC_HANDSHAKE_BUF_SIZE;
        
        MMPD_FS_SetFileNameDmaAddr(cur_buf,name_buf_size,cur_buf+name_buf_size,parameter_buf_size);
        cur_buf+= name_buf_size;  
        cur_buf+= parameter_buf_size;  
    #endif
        break;
    case MMPS_USB_PCCAM_MODE:
	    #if (CHIP == DIAMOND)||(CHIP == D_V1)||(CHIP == PYTHON)
    	#if (SUPPORT_PCCAM_FUNC == 1)
		MMPD_System_GetFWEndAddress(&buf_start);
        ctl_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCCAM_MODE].ulCtlBufSize;
        pccam_compbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCCAM_MODE].ulPCCAMCompBufSize;
        pccam_linebuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_PCCAM_MODE].ulPCCAMLineBufSize;
        cur_buf = buf_start;
        
        MMPD_USB_SetCtlBuf(cur_buf,ctl_buf_size);
        cur_buf+=ctl_buf_size;
        
        MMPD_USB_SetPCCAM_CompressBuf(cur_buf,pccam_compbuf_size);
        cur_buf+=pccam_compbuf_size;
      
        MMPD_USB_SetPCCAM_LineBuf(cur_buf,pccam_linebuf_size);
        cur_buf+=pccam_linebuf_size;

        MMPD_FS_SetFileNameDmaAddr(cur_buf,name_buf_size,cur_buf+name_buf_size,parameter_buf_size);
        cur_buf+= name_buf_size;  
        cur_buf+= parameter_buf_size;  
	    #endif
    	#endif
		break;
	case MMPS_USB_MTP_MODE:
    	#if (CHIP == DIAMOND)||(CHIP == D_V1)||(CHIP == PYTHON)
   	 	#if (SUPPORT_MTP_FUNC == 1)
    	MMPD_System_GetFWEndAddress(&buf_start);
        ctl_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulCtlBufSize;
        pcsync_inbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulPCSYNCInBufSize;
        pcsync_outbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulPCSYNCOutBufSize;
        mtp_transferbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulMtpTranferBufSize;
        cur_buf = buf_start;
        
        MMPD_FS_SetFileNameDmaAddr(cur_buf,name_buf_size,cur_buf+name_buf_size,parameter_buf_size);
        cur_buf+= name_buf_size;  
        cur_buf+= parameter_buf_size;  
            
        MMPD_USB_SetCtlBuf(cur_buf,ctl_buf_size);
        cur_buf+=ctl_buf_size;
      
        MMPD_USB_SetPCSYNC_InBuf(cur_buf,pcsync_inbuf_size);
        cur_buf+=pcsync_inbuf_size;
      
        MMPD_USB_SetPCSYNC_OutBuf(cur_buf,pcsync_outbuf_size);
        cur_buf+=pcsync_outbuf_size;
      
        MMPD_USB_SetPCSYNC_HandShakeBuf(cur_buf,MMPD_USB_PCSYNC_HANDSHAKE_BUF_SIZE);
        cur_buf+=MMPD_USB_PCSYNC_HANDSHAKE_BUF_SIZE;
      
        cur_buf+=0x200;
        MMPD_USB_SetMTP_EPBuf(cur_buf,cur_buf+mtp_transferbuf_size+0x200,mtp_transferbuf_size,cur_buf+2*mtp_transferbuf_size+0x200,m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulMtpMBufSize); //Andy++
        cur_buf+=0x200;
        cur_buf+=(mtp_transferbuf_size<<1);
        cur_buf+=m_UsbBufInfo.UsbMode[MMPS_USB_MTP_MODE].ulMtpMBufSize; //Andy++
	    #endif
    	#endif
		break;
	case MMPS_USB_DPS_MODE:
	    #if (CHIP == DIAMOND)||(CHIP == D_V1)||(CHIP == PYTHON)
    	#if (SUPPORT_DPS_FUNC == 1)
    	MMPD_System_GetFWEndAddress(&buf_start);
        ctl_buf_size = m_UsbBufInfo.UsbMode[MMPS_USB_DPS_MODE].ulCtlBufSize;
        mtp_transferbuf_size = m_UsbBufInfo.UsbMode[MMPS_USB_DPS_MODE].ulMtpTranferBufSize;
        cur_buf = buf_start;
        
        MMPD_FS_SetFileNameDmaAddr(cur_buf,name_buf_size,cur_buf+name_buf_size,parameter_buf_size);
        cur_buf+= name_buf_size;  
        cur_buf+= parameter_buf_size;  
            
	    MMPD_USB_SetCtlBuf(cur_buf,ctl_buf_size);
	    cur_buf+=ctl_buf_size;
	    
	    MMPD_USB_SetMTP_EPBuf(cur_buf,cur_buf+0x500,mtp_transferbuf_size);
	    #endif
    	#endif
		break;
    }
    
    //PRINTF("Usb Alloc Buf Start:0x%X\r\n",buf_start);
    //PRINTF("USb Alloc Buf End:0x%x\r\n",cur_buf);

#if 0     
    MMPD_USB_SetMode(ubMode);
#else
	MMPF_SetUSBChangeMode(ubMode);  // for non-OS task  
#endif    
    
    #if (CHIP == D_V1)||(CHIP == PYTHON)
    #if (SUPPORT_DPS_FUNC == 1)
    if(ubMode == MMPS_USB_DPS_MODE){
    	MMPD_USB_Get_DpsBufAddr(MMPS_USB_DPSHOSTBUF,&m_ulDPSBuf4host);
    }
    #endif //(SUPPORT_DPS_FUNC==1)
    #endif


   // MMPC_USB_PullHighSwitch();
    
    return MMP_ERR_NONE;
}
#endif //PCAM_EN==0
//==============================================================================
//  Function    : MMPS_USB_ReadPCSYNCInData
//==============================================================================
/** 
    @brief  Read PCSYNC In Data
    @param[in] ubMemStart : pcsync in data buffer
    @param[out] *ulReadSize  : return read size
    @return MMP_ERR_NONE
*/
#if SUPPORT_PCSYNC_FUNC==1
MMP_ERR MMPS_USB_ReadPCSYNCInData(MMP_UBYTE *ubMemStart,MMP_ULONG *ulReadSize)
{
    MMP_ULONG in_flag,size;
    MMPD_USB_GetPCSYNCInFlag(&in_flag);
    *ulReadSize = 0;
    if(in_flag==MMPS_USB_PCSYNC_IN_DATA_READY){
        MMPD_USB_GetPCSYNCInSize(&size);
        
        MMPD_USB_ReadPCSYNCInData(ubMemStart,size);
        
        *ulReadSize = size;
        
        MMPD_USB_SetPCSYNCInFlag(MMPS_USB_PCSYNC_IN_EMPTY);
    }    
    return MMP_ERR_NONE;
}
//==============================================================================
//  Function    : MMPS_USB_WritePCSYNCOutData
//==============================================================================
/** 
    @brief  Write PCSYNC Out Data
    @param[in] ubMemStart : pcsync out data buffer
    @param[in] ulWriteSize  : file size to write
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_USB_WritePCSYNCOutData(MMP_UBYTE *ubMemStart,MMP_ULONG ulWriteSize)
{
    MMP_ULONG out_flag;
    MMPD_USB_GetPCSYNCOutFlag(&out_flag);

    if(out_flag==MMPS_USB_PCSYNC_OUT_EMPTY){
        MMPD_USB_SetPCSYNCOutFlag(MMPS_USB_PCSYNC_OUT_DATA_READY);
        
        MMPD_USB_WritePCSYNCOutData(ubMemStart,ulWriteSize);
        
        MMPD_USB_SetPCSYNCOutSize(ulWriteSize);
        
        return MMP_ERR_NONE;
    }    
        return MMP_USB_ERR_PCSYNC_BUSY;
}
#endif

#if (CHIP == D_V1)||(CHIP == PYTHON)
#if (SUPPORT_DPS_FUNC==1)
//==============================================================================
//  Function    : MMPS_DPS_START_JOB
//==============================================================================
/**
    @brief  config the print job setting and send start-print command.
    @return MMPD_USB_SUCCESS
*/
MMP_ERR	MMPS_DPS_START_JOB(MMP_BYTE *bFilePath)
{
    MMP_USHORT printstatus;

    MMPD_DPS_GetPrinterStatus(&printstatus);

    if(printstatus!=(MMPS_USB_NEWJOBOK_TRUE |
                     MMPS_USB_ERRORREASON_NO|
                     MMPS_USB_ERRORSTATUS_NO|
                     MMPS_USB_PRINTSERVICE_IDLE)
	)
    return 1;

    m_DpsPrintInfo.ulFileID=0;///Just set "fileID" initial value
    STRCPY(m_DpsPrintInfo.bFilePath,bFilePath);    
    
    PRINTF("m_DpsPrintInfo.bFilePath=");
    PRINTF(m_DpsPrintInfo.bFilePath);
    PRINTF("\r\n");
    
    
    m_DpsPrintInfo.ulCopies=1;

    m_DpsPrintInfo.ubQuality=0;
    m_DpsPrintInfo.ubPaperSize=0;
    m_DpsPrintInfo.ubPaperType=0;
    m_DpsPrintInfo.ubFileType=0;
    m_DpsPrintInfo.ubDatePrint=0;
    m_DpsPrintInfo.ubFileNamePrint=0;
    m_DpsPrintInfo.ubImageOptimize=0;
    m_DpsPrintInfo.ubLayout=0;
    m_DpsPrintInfo.ubFixedSize=0;

	MMPH_HIF_MemCopyHostToDev(m_ulDPSBuf4host,(MMP_UBYTE *)&m_DpsPrintInfo,sizeof(m_DpsPrintInfo));
    
	return MMPD_DPS_START_JOB();
}

//==============================================================================
//  Function    : MMPS_DPS_AbortPrint
//==============================================================================
/**
    @brief  send abort-print command
    @return MMP_ERR_NONE
*/
MMP_ERR	MMPS_DPS_AbortPrint(void)
{
    MMP_USHORT printstatus;

    MMPD_DPS_GetPrinterStatus(&printstatus);

    printstatus&=MMPS_USB_ERRORREASON_MASK;

    if(printstatus!=(	MMPS_USB_NEWJOBOK_FALSE     |
    					MMPS_USB_ERRORREASON_NO     |
                     	MMPS_USB_ERRORSTATUS_WARNING|
                     	MMPS_USB_PRINTSERVICE_PAUSED
					)&&
       printstatus!=(	MMPS_USB_NEWJOBOK_FALSE     |
						MMPS_USB_ERRORREASON_NO     |
						MMPS_USB_ERRORSTATUS_WARNING|
						MMPS_USB_PRINTSERVICE_PRINTING
					)&&
       printstatus!=(	MMPS_USB_NEWJOBOK_FALSE		|
						MMPS_USB_ERRORREASON_NO		|
                     	MMPS_USB_ERRORSTATUS_NO		|
                     	MMPS_USB_PRINTSERVICE_PRINTING
					)&&
       printstatus!=(	MMPS_USB_NEWJOBOK_FALSE     |
       					MMPS_USB_ERRORREASON_NO     |
                     	MMPS_USB_ERRORSTATUS_FATAL  |
                     	MMPS_USB_PRINTSERVICE_PRINTING
					)&&
       printstatus!=(	MMPS_USB_NEWJOBOK_FALSE     |
       					MMPS_USB_ERRORREASON_NO     |
						MMPS_USB_ERRORSTATUS_FATAL  |
						MMPS_USB_PRINTSERVICE_PAUSED
					)
	)
		return 1;
     
	return MMPD_DPS_AbortPrint();
}
//==============================================================================
//  Function    : MMPS_DPS_ContinuePrint
//==============================================================================
/**
    @brief  send continue-print command
    @return MMP_ERR_NONE
*/
MMP_ERR	MMPS_DPS_ContinuePrint(void)
{
    MMP_USHORT printstatus;

    MMPD_DPS_GetPrinterStatus(&printstatus);

    printstatus&=MMPS_USB_ERRORREASON_MASK;

    if(printstatus!=(MMPS_USB_NEWJOBOK_FALSE     |
                     MMPS_USB_ERRORREASON_NO     |
                     MMPS_USB_ERRORSTATUS_WARNING|
                     MMPS_USB_PRINTSERVICE_PAUSED)
	)
       return 1;

	return MMPD_DPS_ContinuePrint();
}

//==============================================================================
//  Function    : MMPS_DPS_GetPrinterStatus
//==============================================================================
/**
    @brief  Get printer status
    @param[out] usStatus : printer status
    @return MMP_ERR_NONE
*/
MMP_ERR	MMPS_DPS_GetPrinterStatus(MMP_USHORT *usStatus)
{
	return MMPD_DPS_GetPrinterStatus(usStatus);
}
#endif // (SUPPORT_DPS_FUNC==1)
#endif

#if (CHIP == D_V1)||(CHIP == PYTHON)
#if (SUPPORT_PCCAM_FUNC==1)
//==============================================================================
//  Function    : MMPS_DPS_DSC_Config
//==============================================================================
/**
    @brief Set DSC System Configure.
    This function is copied from DSC_Config() in ex_dsc.c .
    @return N/A
*/
void MMPS_DPS_DSC_Config(void)
{
    MMPC_DSC_InitConfig();

    ////MMPS_DSC_Initialize(gCurrentSensorModule);

    //MMPS_Sensor_SetExposureValue(3);
    //i =  MMPS_Sensor_GetPreviewEffectNum();
    //PRINTF("MMPS_DSC_GetPreviewEffectNum() ==> %d\r\n", i);
    //MMPS_Sensor_SetPreviewEffect(8);

	
    MMPD_Display_SetWinPriority(MMPD_DISPLAY_WIN_PIP, MMPD_DISPLAY_WIN_MAIN, MMPD_DISPLAY_WIN_OVERLAY, MMPD_DISPLAY_WIN_ICON);
    MMPS_DSC_SetSystemMode(0);
    ////MMPD_Sensor_SetLightFreq(DEBAND_60_HZ);
}

//==============================================================================
//  Function    : MMPS_DPS_DSC_Decode
//==============================================================================
/**
  @brief The function decoded the jpeg file from SD card or PC memory.
    This function is copied from DSC_Decode() in ex_dsc.c .
  @param[in] bFileName is the storage path and file name. ex: "SD:\\JPEG\\DSC000.JPG"
  @param[in] rotatetype can set MMPD_DISPLAY_ROTATE_NO_ROTATE, MMPD_DISPLAY_ROTATE_RIGHT_90, MMPD_DISPLAY_ROTATE_RIGHT_180, MMPD_DISPLAY_ROTATE_RIGHT_270.
             When TV displaying, it can not rotate.
  @return N/A
*/

void MMPS_DPS_DSC_Decode(MMP_BYTE *bFileName, MMPD_DISPLAY_ROTATE_TYPE rotatetype)
{
    MMPS_DSC_JPEGINFO jpeginfo;
    MMP_ULONG   size = 0;
//    MMP_USHORT *ptr;

//	MMPD_SCALER_FIT_RANGE	fitrange;
//	MMPD_DISPLAY_TVATTRIBUTE	tvattributes;
/*
#if STD_FS == 1
    FILE        *pfile;
#endif
*/
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_MAIN, MMP_FALSE);
    MMPD_Display_SetWinActive(MMPD_DISPLAY_WIN_PIP, MMP_TRUE);

    MMPS_DPS_DSC_Config();

    //if ( mode == DSC_STORAGE_CARD_MODE ) {
        STRCPY(jpeginfo.bJpegFileName, bFileName);
        jpeginfo.usJpegFileNameLength = STRLEN(bFileName); //+1;

        jpeginfo.usJpegbuf = NULL;
        jpeginfo.ulJpegbufaddr = 0;
        jpeginfo.ulJpegbufsize = size;
		jpeginfo.bDecodeThumbnail = MMP_FALSE;
        jpeginfo.bValid = MMP_FALSE;

    //}

#if 0
    //Decoded data and store to host buffer
    ptrOut = (MMP_USHORT *)HOST_RAM_ADDRESS + 0x20000;
    MMPS_DSC_DecodeJpeg(&jpeginfo, ptrOut, 176, 220, NULL);
#else
    //Decoded data and store to internal SRAM buffer
		MMPS_Display_SetOutputPanel(MMPS_DISPLAY_PRM_CTL, MMPS_Display_GetConfiguration()->OutputPanel);
        MMPS_DSC_SetPlaybackMode(DSC_NORMAL_DECODE_MODE);//Ok
        MMPS_DSC_PlaybackJpeg(&jpeginfo, 0, 0);//Ok

#endif
}
//==============================================================================
//  Function    : MMPS_USB_PCCAM_CAPTURE
//==============================================================================
/**
    @brief  PCCAM Capture
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_USB_PCCAM_CAPTURE(void)
{
    MMPD_USB_PCCAM_CAPTURE();

    return MMP_ERR_NONE;
}
#endif // (SUPPORT_PCCAM_FUNC == 1)
#endif


#if (SUPPORT_HOST_STORAGE_DEVICE)
//==============================================================================
//  Function    : MMPS_USB_OpenHostDevice
//==============================================================================
/**
    @brief  Open host storage device
    @param[in] devid : select the device ID 
    @param[in] pfunc : device Read/Write function pointer
    @param[in] pbuf   : Read/Write temporary buffer
    @param[in] bufsize : Read/Write temporary buffer size
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_USB_OpenHostDevice(MMP_ULONG devid, MMPS_USB_FUNC *pfunc, void *pbuf, MMP_ULONG bufsize)
{
    //MMPC_System_AllocZeroMem(MAX_USB_BUF_SIZE, m_pMemDevBuf);
    m_pMemDevBuf = pbuf;
    pUSBFunc = pfunc;
    m_devID = devid;
    m_ulMaxBufSec = bufsize>>9;
    
    pUSBFunc->DevOpen(m_devID);
}

//==============================================================================
//  Function    : MMPS_USB_CloseHostDevice
//==============================================================================
/**
    @brief  Close host storage device
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_USB_CloseHostDevice()
{
    //MMPC_System_FreeMem(m_pMemDevBuf);
    pUSBFunc->DevClose(m_devID);
}


//==============================================================================
//  Function    : MMPS_USB_SendHostDeviceAck
//==============================================================================
/**
    @brief  Send ACK to AIT 
    @param[in] cmd : select command type
    @param[in] AckType : ACK or NAK
    @return MMP_ERR_NONE
*/
void MMPS_USB_SendHostDeviceAck(MMPS_USB_MEMDEV_CMD cmd, ACK_TYPE AckType)
{   
    MMPH_HIF_MemSetL(m_ulMemDevAckAddr, AckType);
    switch(cmd){
        case MMPS_USB_MEMDEV_INIT:
        case MMPS_USB_MEMDEV_RESET:
        case MMPS_USB_MEMDEV_GETINFO:    
            MMPH_HIF_MemSetL(m_ulMemDevAckAddr+4, m_DevSize);
            MMPH_HIF_CmdSend(HIF_CMD_MSDC_ACK);
            #ifdef MEM_USB_DEBUG
            PRINTF("Cm ack\r\n");
            #endif
            break;

        case MMPS_USB_MEMDEV_READ:
            MMPH_HIF_CmdSend(HIF_CMD_MSDC_ACK);
            #ifdef MEM_USB_DEBUG
            PRINTF("Rd ack\r\n");
            #endif
            break;
            
        case MMPS_USB_MEMDEV_WRITE:
            MMPH_HIF_CmdSend(HIF_CMD_MSDC_ACK);
            #ifdef MEM_USB_DEBUG
            PRINTF("Wr ack\r\n");
            #endif
            break;
    }
}

//==============================================================================
//  Function    : MMPS_USB_HandleHostDeviceCommand
//==============================================================================
/**
    @brief  Handle the AIT request command. Ex. Read, Write, Init....and so on.
          The function should be called after host receive AIT interrupt.
    @return MMP_ERR_NONE
*/
MMP_ERR MMPS_USB_HandleHostDeviceCommand(void)
{
    MMPS_USB_MEMDEV_CMD cmd;
    MMP_ERR err = MMP_ERR_NONE;
    //FS_MS_ERROR_TYPE err;
    MMP_ULONG dmastartaddr, startsect, sectcount;    
    MMP_ULONG sector_size, temp_len, tem_count;
    
    cmd = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr);
    switch(cmd){
        case MMPS_USB_MEMDEV_INIT:
            //PRINTF("Get One Cmd %x\r\n", cmd);

            err = pUSBFunc->DevGetSize(m_devID, &m_DevSize, &sector_size);

            MMPS_USB_SendDualDevAck(cmd, (err == MMP_ERR_NONE) ? ACK_TRUE : ACK_FALSE);            
            break;        
        case MMPS_USB_MEMDEV_RESET:
            //PRINTF("Get One Cmd %x\r\n", cmd);

            err = pUSBFunc->DevGetSize(m_devID, &m_DevSize, &sector_size);

            MMPS_USB_SendDualDevAck(cmd, (err == MMP_ERR_NONE) ? ACK_TRUE : ACK_FALSE);            
            break;        
        case MMPS_USB_MEMDEV_GETINFO:
            //PRINTF("Get One Cmd %x\r\n", cmd);

            err = pUSBFunc->DevGetSize(m_devID, &m_DevSize, &sector_size);

            MMPS_USB_SendDualDevAck(cmd, (err == MMP_ERR_NONE) ? ACK_TRUE : ACK_FALSE);            
            break;        
        case MMPS_USB_MEMDEV_WRITE:
            /* get the WRITE cmd parameter*/
            dmastartaddr = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+4);
            startsect = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+8);
            sectcount = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+12);

            #ifdef MEM_USB_DEBUG
            PRINTF("====WR %x, size %x\r\n", startsect, sectcount);
            #endif    

            tem_count = 0;
            while(1){
                if(sectcount > m_ulMaxBufSec)
                    temp_len = m_ulMaxBufSec;
                else
                    temp_len = sectcount;

                /* copy data from AIT to m_pMemDevBuf*/
                MMPH_HIF_MemCopyDevToHost(m_pMemDevBuf, dmastartaddr + (tem_count<<9), temp_len<<9);

                /* write data to B/B opened device*/
                err = pUSBFunc->DevWrite(m_devID, startsect + tem_count, temp_len, (void *)m_pMemDevBuf);

                sectcount -= temp_len;
                tem_count += temp_len;
                if((err != MMP_ERR_NONE) || (sectcount == 0))
                    break;
            }
            /* send ack to AIT*/
            MMPS_USB_SendDualDevAck(cmd, (err == MMP_ERR_NONE) ? ACK_TRUE : ACK_FALSE);
            break;
            
        case MMPS_USB_MEMDEV_READ:
            /* get the Read cmd parameter*/
            dmastartaddr = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+4);
            startsect = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+8);
            sectcount = MMPH_HIF_MemGetL(m_ulMemDevCmdAddr+12);
            
            #ifdef MEM_USB_DEBUG
            PRINTF("====RD %x, size %x\r\n",  startsect, sectcount);
            #endif

            tem_count = 0;
            while(1){
                if(sectcount > m_ulMaxBufSec)
                    temp_len = m_ulMaxBufSec;
                else
                    temp_len = sectcount;

                /* read data from B/B opened device*/
                err = pUSBFunc->DevRead(m_devID, startsect + tem_count, temp_len, (void *)m_pMemDevBuf);

                /* copy data to AIT from m_pMemDevBuf*/
                if(err == MMP_ERR_NONE) 
                    MMPH_HIF_MemCopyHostToDev(dmastartaddr + (tem_count<<9), m_pMemDevBuf, temp_len<<9);

                sectcount -= temp_len;
                tem_count += temp_len;
                if((err != MMP_ERR_NONE) || (sectcount == 0))
                    break;
            }

            /* send ack to AIT*/
            MMPS_USB_SendDualDevAck(cmd, (err == MMP_ERR_NONE) ? ACK_TRUE : ACK_FALSE);
            break;
            
        default:
            err = MMP_INT_ERR_PARAMETER;
            break;
    }


    return err;
}
#endif
/// @}
