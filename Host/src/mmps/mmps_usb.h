//==============================================================================
//
//  File        : mmps_usb.h
//  Description : Usb User Application Interface
//  Author      : Hans Liu
//  Revision    : 1.0
//
//==============================================================================
/**
 @file mmps_usb.h
 @brief Header File for the Host USB API.
 @author Hans Liu
 @version 1.0
*/


#ifndef _MMPS_USB_H_
#define _MMPS_USB_H_

//#include "mmpd_display.h"

/** @addtogroup MMPS_USB
@{
*/


//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================


//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
typedef enum _MMPS_USB_OP_MODE
{
    MMPS_USB_PCCAM_MODE = 0,	///< PCCAM
    MMPS_USB_MSDC_MODE,		    ///< MSDC
    MMPS_USB_PCSYNC_MODE,       ///< PCSYNC
    MMPS_USB_MTP_MODE,          ///< MTP
    MMPS_USB_DPS_MODE           ///< PICTURE BRIDGE
} MMPS_USB_OP_MODE;

#define MMPS_USB_MODE_NUMBER     (5)

typedef struct 
{
  MMP_ULONG     ulFileID;       ///< file id
  MMP_ULONG     ulCopies;       ///< copies
  MMP_UBYTE     ubQuality;      ///< quality
  MMP_UBYTE     ubPaperSize;    ///< paper size
  MMP_UBYTE     ubPaperType;    ///< paper type
  MMP_UBYTE     ubFileType;     ///< file type
  MMP_UBYTE     ubDatePrint;    ///< data print
  MMP_UBYTE     ubFileNamePrint;///< file name print
  MMP_UBYTE     ubImageOptimize;///<image optimize
  MMP_UBYTE     ubLayout;       ///< layout
  MMP_UBYTE     ubFixedSize;    ///<fixed size
  MMP_BYTE	    bFilePath[100]; ///<file path
  
}MMPS_USB_DPS_PRINT_INFO;

#define MMPS_USB_PRINTSERVICE_MASK      0xFFF0 
#define MMPS_USB_PRINTSERVICE_PRINTING  0x0001 
#define MMPS_USB_PRINTSERVICE_IDLE      0x0002 
#define MMPS_USB_PRINTSERVICE_PAUSED    0x0004 

#define MMPS_USB_ERRORSTATUS_MASK       0xFF0F
#define MMPS_USB_ERRORSTATUS_NO         0x0010  
#define MMPS_USB_ERRORSTATUS_WARNING    0x0020 
#define MMPS_USB_ERRORSTATUS_FATAL      0x0040

#define MMPS_USB_ERRORREASON_MASK       0xF0FF
#define MMPS_USB_ERRORREASON_NO         0x0000
#define MMPS_USB_ERRORREASON_PAPER      0x0100
#define MMPS_USB_ERRORREASON_INK        0x0200
#define MMPS_USB_ERRORREASON_HARDWARE   0x0400
#define MMPS_USB_ERRORREASON_FILE       0x0800

#define MMPS_USB_NEWJOBOK_MASK          0x0FFF
#define MMPS_USB_NEWJOBOK_FALSE         0x1000
#define MMPS_USB_NEWJOBOK_TRUE          0x2000


// DPS setting
typedef enum _MMPS_USB_DPS_QUALITY
{
    MMPS_USB_DPS_QUALITY_DEFAULT=0, 
    MMPS_USB_DPS_QUALITY_NORMAL,    
    MMPS_USB_DPS_QUALITY_DRAFT,     
    MMPS_USB_DPS_QUALITY_FINE       
}MMPS_USB_DPS_QUALITY;

typedef enum _MMPS_USB_DPS_PAPERSIZE
{
    MMPS_USB_DPS_PAPERSIZE_DEFAILT  = 0x00,
    MMPS_USB_DPS_PAPERSIZE_L        = 0x01,
    MMPS_USB_DPS_PAPERSIZE_2L       = 0x02,
    MMPS_USB_DPS_PAPERSIZE_POSTCARD = 0x03,
    MMPS_USB_DPS_PAPERSIZE_CARDSIZE = 0x04,
    MMPS_USB_DPS_PAPERSIZE_100X150  = 0x05,
    MMPS_USB_DPS_PAPERSIZE_4X6      = 0x06,
    MMPS_USB_DPS_PAPERSIZE_8X10     = 0x07,
    MMPS_USB_DPS_PAPERSIZE_A4       = 0x08,
    MMPS_USB_DPS_PAPERSIZE_LETTER   = 0x09,
    MMPS_USB_DPS_PAPERSIZE_A6       = 0x0A,
    MMPS_USB_DPS_PAPERSIZE_LROLL    = 0x81,
    MMPS_USB_DPS_PAPERSIZE_2LROLL   = 0x82,
    MMPS_USB_DPS_PAPERSIZE_100ROLL  = 0x86,
    MMPS_USB_DPS_PAPERSIZE_A4ROLL   = 0x88
}MMPS_USB_DPS_PAPERSIZE;

typedef enum _MMPS_USB_DPS_PAPERTYPE
{
    MMPS_USB_DPS_PAPERTYPE_DEFAULT   =  0x00,
    MMPS_USB_DPS_PAPERTYPE_PLAIN,            
    MMPS_USB_DPS_PAPERTYPE_PHOTO,            
    MMPS_USB_DPS_PAPERTYPE_FASTPHOTO         
}MMPS_USB_DPS_PAPERTYPE;

typedef enum _MMPS_USB_DPS_FILETYPE
{
    MMPS_USB_DPS_FILETYPE_DEFAULT=0x00,
    MMPS_USB_DPS_FILETYPE_EXIF,          
    MMPS_USB_DPS_FILETYPE_JPEG,          
    MMPS_USB_DPS_FILETYPE_TIFFEP,        
    MMPS_USB_DPS_FILETYPE_FLASHPIX,      
    MMPS_USB_DPS_FILETYPE_BMP,           
    MMPS_USB_DPS_FILETYPE_CIFF,          
    MMPS_USB_DPS_FILETYPE_GIF,           
    MMPS_USB_DPS_FILETYPE_JFIF,          
    MMPS_USB_DPS_FILETYPE_PCD,           
    MMPS_USB_DPS_FILETYPE_PICT,          
    MMPS_USB_DPS_FILETYPE_PNG,           
    MMPS_USB_DPS_FILETYPE_TIFF,          
    MMPS_USB_DPS_FILETYPE_TIFFIT,        
    MMPS_USB_DPS_FILETYPE_JP2,           
    MMPS_USB_DPS_FILETYPE_JPX           
}MMPS_USB_DPS_FILETYPE;

typedef enum _MMPS_USB_DPS_DATAPRINT
{
    MMPS_USB_DPS_DATEPRINT_DEFAULT=0x00,      
    MMPS_USB_DPS_DATEPRINT_OFF,          
    MMPS_USB_DPS_DATEPRINT_ON           
}MMPS_USB_DPS_DATAPRINT;

typedef enum _MMPS_USB_DPS_FILENAMEPRINT
{
    MMPS_USB_DPS_FILENAMEPRINT_DEFAULT = 0x00,
    MMPS_USB_DPS_FILENAMEPRINT_OFF,
    MMPS_USB_DPS_FILENAMEPRINT_ON       
}MMPS_USB_DPS_FILENAMEPRINT;

typedef enum _MMPS_USB_DPS_IMAGEOPTIMIZE
{
    MMPS_USB_DPS_IMAGEOPTIMIZE_DEFAULT= 0x00,
    MMPS_USB_DPS_IMAGEOPTIMIZE_OFF,
    MMPS_USB_DPS_IMAGEOPTIMIZE_ON   
}MMPS_USB_DPS_IMAGEOPTIMIZE;

typedef enum _MMPS_USB_DPS_LAYOUT
{
    MMPS_USB_PS_LAYOUT_DEFAULT     =    0x00,
    MMPS_USB_PS_LAYOUT_1U          =    0x01,
    MMPS_USB_PS_LAYOUT_2U          =    0x02,
    MMPS_USB_PS_LAYOUT_3U          =    0x03,
    MMPS_USB_PS_LAYOUT_4U          =    0x04,
    MMPS_USB_PS_LAYOUT_5U          =    0x05,
    MMPS_USB_PS_LAYOUT_6U          =    0x06,
    MMPS_USB_PS_LAYOUT_7U          =    0x07,
    MMPS_USB_PS_LAYOUT_8U          =    0x08,
    MMPS_USB_PS_LAYOUT_9U          =    0x09,
    MMPS_USB_PS_LAYOUT_10U         =    0x0A,
    MMPS_USB_PS_LAYOUT_11U         =    0x0B,
    MMPS_USB_PS_LAYOUT_12U         =    0x0C,
    MMPS_USB_PS_LAYOUT_13U         =    0x0D,
    MMPS_USB_PS_LAYOUT_14U         =    0x0E,
    MMPS_USB_PS_LAYOUT_15U         =    0x0F,
    MMPS_USB_PS_LAYOUT_16U         =    0x10,
    MMPS_USB_PS_LAYOUT_INDEX       =    0xFE,
    MMPS_USB_PS_LAYOUT_BORDERLESS  =    0xFF
}MMPS_USB_DPS_LAYOUT;

typedef enum _MMPS_USB_DPS_FIXEDSIZE
{
    MMPS_USB_DPS_FIXEDSIZE_DEFAULT = 0x00,
    MMPS_USB_DPS_FIXEDSIZE_2X3,          
    MMPS_USB_DPS_FIXEDSIZE_3X5,          
    MMPS_USB_DPS_FIXEDSIZE_4X6,          
    MMPS_USB_DPS_FIXEDSIZE_5X7,          
    MMPS_USB_DPS_FIXEDSIZE_8X10,         
    MMPS_USB_DPS_FIXEDSIZE_254,          
    MMPS_USB_DPS_FIXEDSIZE_110,          
    MMPS_USB_DPS_FIXEDSIZE_89,           
    MMPS_USB_DPS_FIXEDSIZE_100,          
    MMPS_USB_DPS_FIXEDSIZE_6,            
    MMPS_USB_DPS_FIXEDSIZE_7,            
    MMPS_USB_DPS_FIXEDSIZE_9,            
    MMPS_USB_DPS_FIXEDSIZE_10,           
    MMPS_USB_DPS_FIXEDSIZE_13,           
    MMPS_USB_DPS_FIXEDSIZE_15,           
    MMPS_USB_DPS_FIXEDSIZE_18,           
    MMPS_USB_DPS_FIXEDSIZE_A4,           
    MMPS_USB_DPS_FIXEDSIZE_LETTER       
}MMPS_USB_DPS_FIXEDSIZE;

typedef enum _MMPS_USB_DPS_CROPPING
{
    MMPS_USB_DPS_CROPPING_DEFAULT =0x00,
    MMPS_USB_DPS_CROPPING_OFF,
    MMPS_USB_DPS_CROPPING_ON            
}MMPS_USB_DPS_CROPPING;


#define MMPS_USB_DPSHOSTBUF	0x0001
#define MMPS_USB_DPSBUFEND	0x0002

typedef enum _MMPD_USB_PCSYNC_INSTATUS
{
    MMPS_USB_PCSYNC_IN_EMPTY = 0,	    ///< in empty
    MMPS_USB_PCSYNC_IN_DATA_READY = 2   ///< in data ready
} MMPS_USB_PCSYNC_INSTATUS;

typedef enum _MMPD_USB_PCSYNC_OUTSTATUS
{
    MMPS_USB_PCSYNC_OUT_EMPTY = 0,	    ///< out empty
    MMPS_USB_PCSYNC_OUT_DATA_READY      ///< out data ready
} MMPS_USB_PCSYNC_OUTSTATUS;


typedef struct _MMPS_USB_BUFFER{
    MMP_ULONG           ulCtlBufSize;   ///< Control Endpoint Buffer Size
    MMP_ULONG           ulMSDCBufSize;  ///< MSDC R/W Buffer Size
    MMP_ULONG           ulPCSYNCInBufSize;  ///< PCSYNC In Data Buffer Size
    MMP_ULONG           ulPCSYNCOutBufSize; ///< PCSYNC Out Data Buffer Size
    MMP_ULONG           ulPCCAMCompBufSize; ///< PCCAM Compress Buffer Size
    MMP_ULONG           ulPCCAMLineBufSize; ///< PCCAM Line Buffer Size
    MMP_ULONG           ulMtpTranferBufSize;///< Mtp Transfer Buffer Size
    MMP_ULONG           ulMtpMBufSize;///< Mtp Malloc Buffer Size Andy++
} MMPS_USB_BUFFER;
    
typedef struct _MMPS_USBMODE_BUFFER_INFO {
    MMP_ULONG           ulNameBufSize;  ///< FS FileName Buffer Size
    MMP_ULONG           ulParameterBufSize; ///< FS Parameter Buffer Size
    MMP_ULONG           ulSdBufSize;    ///< SD DMA Buffer Size
    MMP_ULONG           ulSmBufSize;    ///< SM DMA Buffer Size
    MMPS_USB_BUFFER     UsbMode[MMPS_USB_MODE_NUMBER];
} MMPS_USBMODE_BUFFER_INFO;

typedef struct{
    MMP_ERR (*DevOpen)(MMP_ULONG id);
    MMP_ERR (*DevClose)(MMP_ULONG id); 
    MMP_ERR (*DevGetSize)(MMP_ULONG id, MMP_ULONG *TotalSec, MMP_ULONG *SecSize);
    MMP_ERR (*DevRead)(MMP_ULONG id, MMP_ULONG startSec, MMP_ULONG SecLen, void * buf);
    MMP_ERR (*DevWrite)(MMP_ULONG id, MMP_ULONG startSec, MMP_ULONG SecLen, void * buf);
}MMPS_USB_FUNC;

typedef enum {
    MMPS_USB_MEMDEV_INIT,
    MMPS_USB_MEMDEV_RESET,
    MMPS_USB_MEMDEV_GETINFO,
    MMPS_USB_MEMDEV_READ,
    MMPS_USB_MEMDEV_WRITE,

    MMPS_USB_MEMDEV_ERR = 0xFF
}MMPS_USB_MEMDEV_CMD;

typedef enum {
    ACK_FALSE,
    ACK_TRUE
}ACK_TYPE;

#define MemDevBufferSize1 32
#define MemDevBufferSize2 32

extern MMP_ERR  MMPS_USB_SetMode(MMPS_USB_OP_MODE ubMode);
extern MMP_ERR  MMPS_USB_ReadPCSYNCInData(MMP_UBYTE *ubMemStart,MMP_ULONG *ulReadSize);
extern MMP_ERR  MMPS_USB_WritePCSYNCOutData(MMP_UBYTE *ubMemStart,MMP_ULONG ulWriteSize);
extern MMP_ERR	MMPS_DPS_START_JOB(MMP_BYTE *bFilePath);
extern MMP_ERR	MMPS_DPS_AbortPrint(void);
extern MMP_ERR	MMPS_DPS_ContinuePrint(void);
extern MMP_ERR	MMPS_DPS_GetPrinterStatus(MMP_USHORT *usStatus);
extern void     MMPS_DPS_DSC_Decode(MMP_BYTE *FileName, MMPD_DISPLAY_ROTATE_TYPE rotatetype);
extern MMP_ERR  MMPS_USB_PCCAM_CAPTURE(void);
extern MMPS_USBMODE_BUFFER_INFO*  MMPS_USB_GetConfiguration(void);
extern MMP_ERR MMPS_USB_HandleHostDeviceCommand(void);
extern MMP_ERR MMPS_USB_OpenHostDevice(MMP_ULONG devid, MMPS_USB_FUNC *pfunc, void *pbuf, MMP_ULONG bufsize);
extern MMP_ERR MMPS_USB_CloseHostDevice(void);

/// @}

#endif // _MMPS_USB_H_
