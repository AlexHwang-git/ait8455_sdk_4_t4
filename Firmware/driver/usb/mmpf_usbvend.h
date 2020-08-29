#ifndef _MMPF_USBVEND_H
#define _MMPF_USBVEND_H

#include    "includes_fw.h"
#include    "config_fw.h"
/** @addtogroup MMPF_USB
@{
*/

//#define UVC_TEST
#define ENABLE_YUY2     (1)
#define ENABLE_MJPEG    (1)
#define ENABLE_FRAME_BASE_H264 (1)
#define ENABLE_NV12     (0)

#define USING_STILL_METHOD_1 (1)	// patrick test


#define  HIBYTE(v1)     ( (MMP_UBYTE)((v1)>>8) )
#define  LOBYTE(v1)     ( (MMP_UBYTE)((v1)&0xff))
#define   MSSBYTE(v1)    ( (MMP_UBYTE)((v1)>>24)  )
#define   HISBYTE(v1)    ( (MMP_UBYTE)((v1)>>16)  )
#define   LOSBYTE(v1)    ( (MMP_UBYTE)((v1)>>8)   )
#define   LESBYTE(v1)    ( (MMP_UBYTE)((v1)&0xff) )  

#define GET_STATUS              0x00
#define CLEAR_FEATURE           0x01
#define SET_FEATURE             0x03
#define SET_ADDRESS             0x05
#define GET_DESCRIPTOR          0x06
#define SET_DESCRIPTOR          0x07
#define GET_CONFIGURATION       0x08
#define SET_CONFIGURATION       0x09
#define GET_INTERFACE           0x0A
#define SET_INTERFACE           0x0B
#define SYNCH_FRAME             0x0C

#define DEVICE_DESCR            0x01
#define CONFIG_DESCR            0x02
#define STRING_DESCR            0x03
#define INTERFACE_DESCR         0x04
#define ENDPOINT_DESCR          0x05

#define DEVICE_REMOTE_WAKEUP    0x01
#define ENDPOINT_STALL          0x00

//*************************************************************************
//  USB registers
//*************************************************************************
#define USB_BASE_ADDR       0x8000A800

#define USB_BASE_B ((volatile unsigned char *)USB_BASE_ADDR)
#define USB_BASE_W ((volatile unsigned short *)USB_BASE_ADDR)
#define USB_BASE_L ((volatile unsigned long *)USB_BASE_ADDR)
#define FB_BASE_B   ((volatile unsigned char *)0)
#define FB_BASE_W   ((volatile unsigned short *)0)
#define FB_BASE_L   ((volatile unsigned long *)0)




//**********************************************************************************
// descriptor length
//**********************************************************************************
/*
#define NUM_YUY2_VIDEO (19) // 1 for 1920x1088
#define NUM_MJPG_VIDEO (19)
#define NUM_H264_VIDEO (8)
*/

#define USB_UVC_YUY2_STILL_DESC_SIZE	(0x07+NUM_YUY2_STILL*4) * ( (NUM_YUY2_STILL)?1:0 )
#define USB_UVC_MJPEG_STILL_DESC_SIZE	(0x07+NUM_MJPG_STILL*4) * ( (NUM_MJPG_STILL)?1:0 )
#define USB_UVC_COLOR_DESC_SIZE			(0x06)

//input header descriptor length
#define YUY2_DESC_LEN  ((0x01 + 0x1B + 0x1E * NUM_YUY2_VIDEO + USB_UVC_COLOR_DESC_SIZE) * ENABLE_YUY2)
#define NV12_DESC_LEN  ((0x01 + 0x1B + 0x1E * NUM_YUY2_VIDEO + USB_UVC_COLOR_DESC_SIZE) * ENABLE_NV12)
#define MJPEG_DESC_LEN ((0x01 + 0x0B + 0x1E * NUM_MJPG_VIDEO + USB_UVC_MJPEG_STILL_DESC_SIZE + USB_UVC_COLOR_DESC_SIZE) * ENABLE_MJPEG )
#define H264_DESC_LEN  ((0x01 + 0x1C + 0x26 * NUM_H264_VIDEO + USB_UVC_MJPEG_STILL_DESC_SIZE ) * ENABLE_FRAME_BASE_H264 )

#define UVC_INPUT_HEADER_LENGTH (0x0D+(YUY2_DESC_LEN)+(NV12_DESC_LEN)+(MJPEG_DESC_LEN)+(H264_DESC_LEN) )


//format index
#if ENABLE_YUY2
#define YUY2_FORMAT_INDEX      	(2)//(1)	Change defluat to MJPEG 1080p, Modified by James
#else
#define YUY2_FORMAT_INDEX       (0)	//Invalid
#endif

#if ENABLE_FRAME_BASE_H264
    #if USING_EXT_USB_DESC==1
    #define FRAME_BASE_H264_INDEX   (2)//(2)
    #else
    #define FRAME_BASE_H264_INDEX   (3)//(2)
    #endif
#else
    #define FRAME_BASE_H264_INDEX   (0)//(2)
#endif

#if ENABLE_MJPEG
    #if USING_EXT_USB_DESC==1
    #define MJPEG_FORMAT_INDEX      (3)
    #else
    #define MJPEG_FORMAT_INDEX      (1)//(2)	Change defluat to MJPEG 1080p, Modified by James
    #endif
#else
    #define MJPEG_FORMAT_INDEX      (0)
#endif

#if ENABLE_NV12
#define NV12_FORMAT_INDEX       (4) // same as YUY2 for test
#else
#define NV12_FORMAT_INDEX       (0) // same as YUY2 for test
#endif
// total numbers of video formats
#define TOTAL_NUM_VIDEO_FORMAT       (ENABLE_YUY2 + ENABLE_MJPEG + ENABLE_NV12 + ENABLE_FRAME_BASE_H264)

#define PCSYNC_HANDSHAKE_BUF_SIZE           (16)
#define PCSYNC_OUTFLAG_OFFSET_B             (0)
#define PCSYNC_INFLAG_OFFSET_B              (1)
#define PCSYNC_INSIZE_OFFSET_W              (2)
#define PCSYNC_OUTSIZE_OFFSET_W             (4)
#if(CHIP==P_V2)
#define USB_REG_BASE_ADDR           (0x8000A800)
#define USB_DMA_REG_BASE_ADDR       (0x80006000)
#endif
#if(CHIP==VSN_V2)
#define USB_CTL_BASE_ADDR           (0x80001000)
#define USB_DMA_BASE_ADDR           (0x80001400)
#endif

typedef struct _UsbReqType_t
{
#define REQ_DEVICE      0
#define REQ_INTERFACE   1
#define REQ_ENDPOINT    2
#define REQ_OTHER       3
    MMP_UBYTE bReqRecipient:5 ;
#define STD_REQ     0
#define CLASS_REQ   1
#define VENDOR_REQ  2
    MMP_UBYTE bReqType:2;
#define HOST2DEVICE 0
#define DEVICE2HOST 1
    MMP_UBYTE bReqDir:1;
    
} UsbReqType_t;

typedef struct _UsbCtrlReq_t 
{
    union  {
        MMP_UBYTE    bmReqType;
        UsbReqType_t bmReqTypeBits ;
    } bmRequestType ;
    MMP_UBYTE   bRequest;
    MMP_USHORT  wValue;
    MMP_USHORT  wIndex;
    MMP_USHORT  wLength;
    
    // Keep info for request
    MMP_UBYTE   bDevAddr ;
    MMP_USHORT  wInterface    ;
    MMP_USHORT  wAlterSetting ;
    MMP_UBYTE   bConfig ;
    MMP_UBYTE   bEndPoint ;
    MMP_UBYTE   bDevice ;
    MMP_USHORT  wSyncFrame ;
    MMP_USHORT  wTestMode ;
} UsbCtrlReq_t ;

#define USBREQ_T(req)   req.bmRequestType
#define USBREQ_R(req)   req.bRequest
#define USBREQ_V(req)   req.wValue 
#define USBREQ_I(req)   req.wIndex  
#define USBREQ_L(req)   req.wLength
#define USB_GBL_CLK_USBPHY_DIV		(0x15)
#if (CHIP==P_V2)
//Base addr 0x80006000
#define USB_DMA_CTL_B               (0x80)
#define USB_DMA_INTENABLE_B         (0x82)
#define USB_DMA_INTSTS_B            (0x83)
#define USB_DMA_FBADDR_L            (0x84>>2)
#define USB_DMA_FIFOADDR_W          (0x88>>1)
#define USB_DMA_CMDADDR_W           (0x8A>>1)
#define USB_DMA_PKTBYTE_W           (0x8C>>1)
#define USB_DMA_LASTPKTBYTE_W       (0x8E>>1)
#define USB_DMA_PKTSUM_L            (0x90>>2)
#define USB_DMA_ANDVALUE_L          (0x94>>2)
#define USB_DMA_ORVALUE_L           (0x98>>2)
#endif

//Base addr 0x8000A800
#define USB_FADDR_B                 (0x00)
#define USB_POWER_B                 (0x01)
#define USB_INTRTX_W                (0x02>>1)
#define USB_INTRRX_W                (0x04>>1)
#define USB_INTRTXE_W               (0x06>>1)
#define USB_INTRRXE_W               (0x08>>1)
#define USB_INTRUSB_B               (0x0A)
#define USB_INTRUSBE_B              (0x0B)
#define USB_FRAME_W                 (0x0C>>1)
#define USB_INDEX_B                 (0x0E)
#define USB_TESTMODE_B              (0x0F)
#define USB_SOFT_CON_B				(0xBF)

#define USB_TXMAXP_W                ((0x10+ 0xF0 + (endpoint << 4))>>1)
#define USB_TXCSR_W                 ((0x12+ 0xF0 + (endpoint << 4))>>1)
#define USB_CSR0_W  USB_TXCSR_W
#define USB_RXMAXP_W                ((0x14+ 0xF0 + (endpoint << 4))>>1)
#define USB_RXCSR_W                 ((0x16+ 0xF0 + (endpoint << 4))>>1)
#define USB_RXCOUNT_W               ((0x18+ 0xF0 + (endpoint << 4))>>1)
#define USB_COUNT0_W    USB_RXCOUNT_W
#define USB_FIFOSIZE_B              (0x1F+ 0xF0 + (endpoint << 4))
#define USB_CONFIGDATA_B    USB_FIFOSIZE_B

typedef enum _MMPF_USB_EP_ID
{
    MMPF_USB_EP0 = 0,
    MMPF_USB_EP1,
    MMPF_USB_EP2,
    MMPF_USB_EP3,
    MMPF_USB_EP4,
    MMPF_USB_EP5
} MMPF_USB_EP_ID;

#define USB_DEVCTL_B                (0x60)

//  USB_INTRTX_W
#define EP0_INT_BIT                 (0x0001)
#define EP1_TX_INT_BIT              (0x0002)
#define EP2_TX_INT_BIT              (0x0004)
#define EP3_TX_INT_BIT              (0x0008)
#define EP4_TX_INT_BIT              (0x0010)
#define EP5_TX_INT_BIT              (0x0020)

//  USB_POWER_W
#define HS_MODE_BIT                 (0x10)


//  USB_INTRRX_W
#define EP1_RX_INT_BIT              (0x0002)
#define EP2_RX_INT_BIT              (0x0004)
#define EP3_RX_INT_BIT              (0x0008)
#define EP4_RX_INT_BIT              (0x0010)
#define EP5_RX_INT_BIT              (0x0020)
//  USB_INTRUSB_B
#define SUSPEND_INT_BIT             (0x0001)
#define RESUME_INT_BIT              (0x0002)
#define RESET_INT_BIT               (0x0004)
#define SOF_INT_BIT                 (0x0008)

//  USB_CSR0_W
#define EP0_RXPKTRDY_BIT            (0x0001)
#define EP0_TXPKTRDY_BIT            (0x0002)
#define EP0_SENTSTALL_BIT           (0x0004)
#define EP0_DATAEND_BIT             (0x0008)
#define EP0_SETUPEND_BIT            (0x0010)
#define EP0_SENDSTALL_BIT           (0x0020)
#define CLEAR_EP0_SENTSTALL         (0x0000)
#define SET_EP0_SERVICED_SETUPEND   (0x0084)
#define SET_EP0_SERVICED_RXPKTRDY   (0x0044)
#define SET_EP0_TXPKTRDY            (0x0006)
#define SET_EP0_SENDSTALL           (0x0024)
#define EP0_FLUSHFIFO_BIT           (0x0100)
//  USB_TXCSR_W
#define TX_TXPKTRDY_BIT             (0x0001)
#define TX_FIFO_NOTEMPTY_BIT        (0x0002)
#define TX_UNDERRUN_BIT             (0x0004)
#define TX_FLUSHFIFO_BIT            (0x0008)
#define TX_SENDSTALL_BIT            (0x0010)
#define TX_SENTSTALL_BIT            (0x0020)
#define TX_INCOMPTX_BIT             (0x0080)
#define SET_TX_CLRDATATOG           (0x00E6)
#define SET_TX_TXPKTRDY             (0x00A7)
#define SET_TX_SENDSTALL            (0x00B1)
#define CLEAR_TX_SENTSTALL          (0x0086)
#define SET_TX_FLUSHFIFO            (0x00AE)
#define TXCSR_RW_MASK               (0xFF10)
#define TX_DMAREQENAB		(0x0100)

#define SET_TX_MODE                 (0x2000)        //except EP1, EP2 and EP3 should set TX or RX mode
#define SET_TX_ISO                  (0x4000)
#define SET_RX_MODE_J               (~0x2000)

//  USB_RXCSR_W
#define RX_RXPKTRDY_BIT             (0x0001)
#define RX_SENDSTALL_BIT            (0x0020)
#define RX_SENTSTALL_BIT            (0x0040)
#define SET_RX_CLRDATATOG           (0x01C5)
#define SET_RX_SENDSTALL            (0x0165)
#define CLEAR_RX_SENTSTALL          (0x0105)
#define CLEAR_RX_RXPKTRDY           (0x0144)
#define RXCSR_RW_MASK               (0xF820)
#define RX_FLUSHFIFO_BIT            (0x0010)
#define RX_SET_ISO					(0x4000)
#define RX_DMA_ENABLE				(0x2000)


#define USB_CONTROL_IDLE            (0x0)
#define USB_CONTROL_TX              (0x1)
#define USB_CONTROL_RX              (0x2)

#define USB_REQUEST_DIRECTION_BIT   (0x80)

#define USB_REQUEST_TYPE_MASK       (0x60)
#define USB_REQUEST_RECEIVER_MASK   (0x1F)
#define USB_ENDPOINT_ADDRESS_MASK   (0x7F)

#define USB_STANDARD_REQUEST        (0x00)
#define USB_CLASS_REQUEST           (0x20)
#define USB_VENDOR_REQUEST          (0x40)


//***************************************************************
#define  STD_CMD                   0x60
#define  GET_STATUS_CMD            0x60
#define  CLEAR_FEATURE_CMD         0x61
#define  SET_FEATURE_CMD           0x62
#define  SET_ADDRESS_CMD           0x63
#define  GET_DESCRIPTOR_CMD        0x64
#define  SET_DESCRIPTOR_CMD        0x65
#define  GET_CONFIG_CMD            0x66
#define  SET_CONFIG_CMD            0x67
#define  GET_INTERFACE_CMD         0x68
#define  SET_INTERFACE_CMD         0x69
#define  SYNCH_FRAME_CMD           0x6A

#define  GET_DEV_DESCR_CMD          0x6B
#define  GET_CONFIG_DESCR_CMD       0x6C
#define  GET_STRING_DESCR_CMD       0x6D
#define  GET_DEVQUA_DESCR_CMD       0x6E
#define  GET_OTHERSP_DESCR_CMD      0x6F

#define  GET_HIDREPORT_DESCR_CMD    0x70


#define  GET_STATUS              0x00
#define  CLEAR_FEATURE           0x01
#define  SET_FEATURE             0x03
#define  SET_ADDRESS             0x05
#define  GET_DESCRIPTOR          0x06         
#define  SET_DESCRIPTOR          0x07
#define  GET_CONFIGURATION       0x08
#define  SET_CONFIGURATION       0x09
#define  GET_INTERFACE           0x0A
#define  SET_INTERFACE           0x0B
#define  SYNCH_FRAME             0x0C

#define  DEVICE_DESCR            0x01
#define  CONFIG_DESCR            0x02
#define  STRING_DESCR            0x03
#define  INTERFACE_DESCR         0x04
#define  ENDPOINT_DESCR          0x05
#define  DEVICE_QUALIFIER_DESCR  0x06
#define  OTHER_SPEED_CONFIG_DESC 0x07
#define  INTERFACE_POWER_DESC    0x08

#define  DEVICE_REMOTE_WAKEUP    0x01
#define  ENDPOINT_STALL          0x00

#define  CLASS_CMD    0xF0
#define  BOT_RESET_CMD    0xFF
#define  BOT_GET_MAX_LUN_CMD      0xFE


// for VC, AC
#define  RC_UNDEFINED            0x00
#define  SET_CUR_CMD             0x01
#define  GET_CUR_CMD             0x81
#define  GET_MIN_CMD             0x82              
#define  GET_MAX_CMD             0x83
#define  GET_RES_CMD             0x84
#define  GET_LEN_CMD             0x85
#define  GET_MEM_CMD             0x85
#define  GET_INFO_CMD            0x86
#define  GET_DEF_CMD             0x87
// New for UVC1.5
#define  SET_CUR_ALL             0x11 // New for UVC1.5
#define  GET_CUR_ALL             0x91
#define  GET_MIN_ALL             0x92
#define  GET_MAX_ALL             0x93
#define  GET_RES_ALL             0x94
#define  GET_DEF_ALL             0x97


#define  CAP_SET_CUR_CMD            (1 << 0)
#define  CAP_GET_CUR_CMD            (1 << 1)
#define  CAP_GET_MIN_CMD            (1 << 2)
#define  CAP_GET_MAX_CMD            (1 << 3)
#define  CAP_GET_RES_CMD            (1 << 4)
#define  CAP_GET_LEN_CMD            (1 << 5)
#define  CAP_GET_MEM_CMD            (1 << 5)
#define  CAP_GET_INFO_CMD           (1 << 6)
#define  CAP_GET_DEF_CMD            (1 << 7)
#define  CAP_SET_CUR_ALL_CMD        (1 << 8)
#define  CAP_GET_MIN_ALL_CMD        (1 << 9)
#define  CAP_GET_MAX_ALL_CMD        (1 << 10)
#define  CAP_GET_RES_ALL_CMD        (1 << 11)
#define  CAP_GET_DEF_ALL_CMD        (1 << 12)


// ERROR_CODE
#define CONTROL_NO_ERROR         0x00
#define CONTROL_NOT_READY        0x01
#define CONTROL_WRONG_STATE      0x02
#define CONTROL_POWER            0x03
#define CONTROL_OUT_OF_RANGE     0x04
#define CONTROL_INVALID_UNIT     0x05
#define CONTROL_INVALID_CONTROL  0x06
#define CONTROL_INVALID_REQUEST  0x07
#define CONTROL_BUFFER_OVERFLOW  0x08       // UVC1.5
#define CONTROL_INVALID_VAL_IN_RANGE 0x09   // UVC1.5
#define CONTROL_UNKNOWN          0xFF


#define STD_GET_STATUS_CMD          (0x0)
#define STD_CLEAR_FEATURE_CMD       (0x1)
#define STD_SET_FEATURE_CMD         (0x3)
#define STD_SET_ADDRESS_CMD         (0x5)
#define STD_GET_DESCRIPTOR_CMD      (0x6)
#define STD_SET_DESCRIPTOR_CMD      (0x7)
#define STD_GET_CONFIGURATION_CMD   (0x8)
#define STD_SET_CONFIGURATION_CMD   (0x9)
#define STD_GET_INTERFACE_CMD       (0xA)
#define STD_SET_INTERFACE_CMD       (0xB)
#define STD_SYNCH_FRAME_CMD         (0xC)

#define REQUEST_RECIVER_DEVICE      (0x0)
#define REQUEST_RECIVER_INTERFACE   (0x1)
#define REQUEST_RECIVER_ENDPOINT    (0x2)

#define MAX_INTERFACE_NUM           (0x1)
#define MAX_RX_ENDPOINT_NUM         (0x2)
#define MAX_TX_ENDPOINT_NUM         (0x2)

#define CLEAR_WAKEUP_FEATURE        (0xFD)
#define SET_WAKEUP_FEATURE          (0x02)
#define CLEAR_HALT_FEATURE          (0xFE)
#define SET_HALT_FEATURE            (0x01)

#define ENDPOINT_HALT_FEATURE           (0x0)
#define DEVICE_REMOTE_WAKEUP_FEATURE    (0x1)
#define TEST_MODE_FEATURE               (0x2)

#define TEST_J                          (0x1)
#define TEST_K                          (0x2)
#define TEST_SE0_NAK                    (0x3)
#define TEST_PACKET                     (0x4)
#define TEST_FORCE_ENABLE               (0x5)

#define CLASS_CANCEL_REQUEST_CMD                (0x64)
#define CLASS_RESET_DEVICE_CMD                  (0x66)
#define CLASS_GET_DEVICE_STATUS_CMD             (0x67)
#define CLASS_MASS_STORAGE_RESET_CMD            (0xFF)
#define CLASS_GET_MAXLUN_CMD                    (0xFE)

#define USB_HIGH_SPEED                          (0x0)
#define USB_FULL_SPEED                          (0x1)

// Video endpoint 1
#define PCCAM_TX_EP1_ADDR                       (0x01)
// Video endpint 2
#define PCCAM_TX_EP2_ADDR                       (0x02)
// audio endpoint 1
#define PCCAM_AU_EP_ADDR                        (0x03)
// status endpoint
#define PCCAM_EX_EP_ADDR                        (0x04)
// hid endpoint (VSN_V3)
#if (CHIP==VSN_V3)

#if SUPPORT_PCSYNC
#define PCSYNC_EP_ADDR                          (0x05) // 256 x 2 bytes buffer
#define PCSYNC_DBG_EP_ADDR                      (0x06)
#endif

#define PCCAM_HID_EP_ADDR                       (0x07)


#endif
#if (CHIP==VSN_V2)
#define PCCAM_HID_EP_ADDR                       (0x00)
#endif


// endpoint 5, 6 in VSN_V3 unused now
#define AU_EP_MAX_PK_SIZE                    (512)
#define AU_EP_MAX_PK_SIZE_HS                (512)
#define AU_EP_MAX_PK_SIZE_FS                (68)
     
#if USB_UVC_BULK_EP
#define HS_TX_MAX_PK_SIZE                   (512)
#define FS_TX_MAX_PK_SIZE                   (64)
#else   

#if  YUY2_848_480_30FPS==1
#define HS_TX_MAX_PK_SIZE                   (1024)
#else
#define HS_TX_MAX_PK_SIZE                   (1020)
#endif

#define FS_TX_MAX_PK_SIZE                   (944)
#endif

#define TX_PER_FRAME                        (3)

#define TX_ADD_FRAME_BITS(x)                ( (x) << 11)

#define EX_EP_MAX_PK_SIZE                   (8)
 
#if SUPPORT_PCSYNC
#define PCSYNC_EP_MAX_PK_SIZE               (256)
#endif
 
#if (CHIP==VSN_V2) 

#if (USING_EXT_USB_DESC==1)
#define PCCAM_EP_DISABLE_MAP                 ( ( 1 << 5) ) // disable endpoint 4,5   
#else
#define PCCAM_EP_DISABLE_MAP                 ((1<<2) | ( 1 << 5) ) // disable endpoint 4,5   
#endif

#endif

#if (CHIP==VSN_V3) 
    #if (USING_EXT_USB_DESC==1)
        #define PCCAM_EP_DISABLE_MAP                 ( (1<<5) | (1<<6) | (1<<7))  // disable endpoint 5,6,7    
    #else
        #if SUPPORT_PCSYNC
        #define PCCAM_EP_DISABLE_MAP                 ((1<<2)  | (1<<6) | (1<<7))  // disable endpoint 5,6,7    
        #else
        #define PCCAM_EP_DISABLE_MAP                 ((1<<2)  | (1<<5) | (1<<6) | (1<<7))  // disable endpoint 5,6,7    
        #endif
    #endif
#endif

#if SUPPORT_PCSYNC
#define VENDOR_REQUEST      0x10
#define USB_DUMMY       0x00

//**********************************************************************************
//
//**********************************************************************************
#define VENDOR_CMD              0x30
#define VENDOR_IN               0x31


//**********************************************************************************
//         USB Vendor Command for PC Sync
//**********************************************************************************
#define VCMD_READ_REGISTER       0x00
#define VCMD_WRITE_REGISTER      0x01
#define VCMD_GET_MODE            0x10
#define VCMD_SET_BULK_IN          0x20
#define VCMD_SET_BULK_OUT         0x22
#define VCMD_SIZE_BULK_IN         0x24
#define VCMD_SIZE_BULK_OUT        0x26
#define VCMD_NOTIFY_BULK_IN_OK     0x30
#define VCMD_NOTIFY_BULK_OUT_OK    0x32


////////////////////////////////////////////////////////////////////////////////////
// for pcsync directly
#define VCMD_READ_PCSYNC         0x52
#define VCMD_WRITE_PCSYNC        0x54
#endif


 
typedef __packed struct
{
    MMP_ULONG dwNextDescAddr;
    MMP_ULONG dwPLDAddr;
    MMP_USHORT wPLDSize;
    MMP_USHORT wPara;
}USB_DMA_DESC;
#define USBDMA_LIST_LEN            (0x0C)

#define DMALIST_INVALID (0)
#define DMALIST_BUSY    (1)
typedef __packed struct
{
    MMP_ULONG   dmalist_addr;
    MMP_ULONG   data_header_len ;
    MMP_ULONG   data_addr ;
    MMP_ULONG   max_dsize;
    MMP_ULONG   checksum ;
    MMP_USHORT  dmalist_status ;
    #if H264_CBR_PADDING_EN==1
    MMP_USHORT  dma_flag ;
    #endif
} USB_DMA_DATA_INFO ;

#define EP1_DMA_LIST_BUF_CNT        (2)
#define EP2_DMA_LIST_BUF_CNT        (2)
#define USB_DMA_DATA_INFO_OFFSET    (64 )
#define USB_DMA_DATA_LIST_OFFSET    (128)
#define USB_DMA_BUF_SIZE            (USB_DMA_DATA_LIST_OFFSET + 8192)
// UVC & UAC end

/*************************************************************/
/*                                                           */
/*                  Function Prototype                       */
/*                                                           */
/*************************************************************/
extern MMP_ULONG usb_cur_dma_addr(MMP_UBYTE ep_id);
extern void UsbWriteTxMaxP(MMP_ULONG endpoint, MMP_ULONG size);
extern void UsbWriteRxMaxP(MMP_ULONG endpoint, MMP_ULONG size);
extern void UsbEp1RxIntHandler(void);
extern void UsbEp1TxIntHandler(void);
extern void UsbEp0IntHandler(void);
extern void UsbEp2TxIntHandler(void);
extern void UsbEp3TxIntHandler(void);
#if SUPPORT_PCSYNC
extern void UsbEp5TxIntHandler(void);
extern void UsbEp5RxIntHandler(void);
#endif
extern void UsbRestIntHandler(void);
extern void UsbWriteRxEpCSR(MMP_ULONG endpoint, MMP_USHORT csr);
extern void UsbWriteTxEpCSR(MMP_ULONG endpoint, MMP_USHORT csr);
extern void UsbWriteEp0CSR(MMP_USHORT csr);
extern MMP_USHORT UsbReadRxEpCSR(MMP_ULONG endpoint);
extern MMP_USHORT UsbReadTxEpCSR(MMP_ULONG endpoint);
extern MMP_USHORT UsbReadEp0CSR(void);
extern MMP_USHORT UsbReadRxEpCOUNT(MMP_ULONG endpoint);
extern MMP_USHORT UsbReadEp0COUNT(void);
extern void UsbISODmaTxConfig(MMP_ULONG endpoint, MMP_ULONG fb_addr, MMP_USHORT pkt_byte, MMP_USHORT last_pkt_byte,
                       MMP_ULONG pkt_sum, MMP_ULONG zero_end);
extern void UsbDmaTxConfig(MMP_ULONG endpoint, MMP_ULONG fb_addr, MMP_USHORT pkt_byte, MMP_USHORT last_pkt_byte,
                           MMP_ULONG pkt_sum, MMP_ULONG zero_end);
extern void UsbDmaRxConfig(MMP_ULONG endpoint, MMP_ULONG fb_addr, MMP_USHORT pkt_byte, MMP_USHORT last_pkt_byte,
                           MMP_ULONG pkt_sum);
extern MMP_BOOL UsbISODmaListTxConfig(MMP_UBYTE ep_id,MMP_ULONG fb_addr,MMP_ULONG size,MMP_UBYTE flag);

extern void UsbDmaStop(MMP_UBYTE);
extern void usb_dma_setlist(MMP_USHORT usSize, MMP_ULONG ulAddr, MMP_ULONG ulFbAddr, MMP_USHORT usPara);
extern void usb_dma_en(MMP_ULONG ulAddr,MMP_UBYTE ubEnableInt, MMP_UBYTE ubEndpoint,MMP_USHORT des_cnt);
extern MMP_BOOL usb_dma_busy(MMP_UBYTE ep_id);
extern void usb_dma_wait_start(MMP_UBYTE ep_id);
extern MMP_UBYTE usb_epid_link_dma(MMP_UBYTE ep_id) ;
extern MMP_UBYTE usb_epid_link_dmaid(MMP_UBYTE ep_id);
extern MMP_UBYTE usb_epaddr_link_epid(MMP_UBYTE ep_addr);


extern void usb_audio_set_max_pktsize(MMP_ULONG sr,MMP_BOOL sync_mode);

#define USB_FIFO_MODE		(1)
#define USB_DMA_MODE		(2)
#define USB_DMA_LIST_MODE	(3)
#define USB_WORKING_MODE	USB_DMA_LIST_MODE
#define EP1_DMA_LIST_MAX_SIZE   (48*1024) // 2 ms in HSHB
#define EP2_DMA_LIST_MAX_SIZE   (16*1024)

#define LIST_TXPKTRDY       (0x8000)
#define LIST_LAST           (0x4000)
#define LIST_INDEX          (0x2000)
#define LIST_FIXED_DATA     (0x1000)
#define UVC_HEADER_SIZE (0x0C) // always is 0x0c
#define USB_DESC_START_ADDR    (0x1700000)//(0x128000)
#define USB_DMA_DONE        (0x01)
#define USB_DMA_TXPKTRDY    (0x02)

#define USB_UVC_EOF         (0x01)
#define USB_DMA_EN          (0x02)
/// @}
#endif
