#define exUSBDESCR
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbdescr.h"
#include "config_fw.h"
#include "mmpf_usbuvc.h"
/** @addtogroup MMPF_USB
@{
*/

#if defined(USB_FW)||(USB_EN)

#pragma O0

#define VENDOR_ID           0x1532//0x114D
#define PCCAM_ID            0x0E03//0x8455 //8C00

#define MSDC_ID             0x8200
#define PCSYNC_ID           0x8100
#define DPS_ID              0x8300

#define UVC_EU0_ID      (0x05)
#define UVC_EU1_ID      (0x06)
#define UVC_EU2_ID      (0x02)
#define UVC_EU3_ID      (0x08)
#define UVC_OT_ID       (0x04)
#define UVC_PU_ID       (0x03)
#define UVC_IT_ID       (0x01)


//#define AUDIO_PACKET_SIZE(samprate,sampchannel) \
//    ( 512 ) & 0xFF , \
//    ( 512 ) >> 8  

#define UUID(L1,S1,S2,D1,D2) \
LESBYTE(L1),\
LOSBYTE(L1),\
HISBYTE(L1),\
MSSBYTE(L1),\
LOBYTE(S1),\
HIBYTE(S1),\
LOBYTE(S2),\
HIBYTE(S2),\
MSSBYTE(D1),\
HISBYTE(D1),\
LOSBYTE(D1),\
LESBYTE(D1),\
MSSBYTE(D2),\
HISBYTE(D2),\
LOSBYTE(D2),\
LESBYTE(D2)

#define YUY2_FORMAT_DESC(index,num) \
0x01B,\
0x024,\
0x004,\
index,\
num,\
0x059,\
0x055,\
0x059,\
0x032,\
0x000,\
0x000,\
0x010,\
0x000,\
0x080,\
0x000,\
0x000,\
0x0AA,\
0x000,\
0x038,\
0x09B,\
0x071,\
0x010,\
0x001,\
0x000,\
0x000,\
0x000,\
0x000,

#define YUY2_FRAME_DESC(num,w,h,fps) \
0x01E,\
0x024,\
0x005,\
num,\
0x000,\
LOBYTE(w),\
HIBYTE(w),\
LOBYTE(h),\
HIBYTE(h),\
LESBYTE(w*h*2*8*fps),\
LOSBYTE(w*h*2*8*fps),\
HISBYTE(w*h*2*8*fps),\
MSSBYTE(w*h*2*8*fps),\
LESBYTE(w*h*2*8*fps),\
LOSBYTE(w*h*2*8*fps),\
HISBYTE(w*h*2*8*fps),\
MSSBYTE(w*h*2*8*fps),\
LESBYTE(w*h*2),\
LOSBYTE(w*h*2),\
HISBYTE(w*h*2),\
MSSBYTE(w*h*2),\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),\
0x001,\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),

#define YUY2_STILL_DESC(len,num,...) \
len,\
0x024,\
0x003,\
0x000,\
num,\
__VA_ARGS__,\
0x001,\
0x003,

//patrick: GUID = NV12, 12bpp
#define NV12_FORMAT_DESC(index,num) \
0x01B,\
0x024,\
0x004,\
index,\
num,\
0x04E,\
0x056,\
0x031,\
0x032,\
0x000,\
0x000,\
0x010,\
0x000,\
0x080,\
0x000,\
0x000,\
0x0AA,\
0x000,\
0x038,\
0x09B,\
0x071,\
0x00C,\
0x001,\
0x000,\
0x000,\
0x000,\
0x000,

#define NV12_FRAME_DESC(num,w,h,fps) \
0x01E,\
0x024,\
0x005,\
num,\
0x000,\
LOBYTE(w),\
HIBYTE(w),\
LOBYTE(h),\
HIBYTE(h),\
LESBYTE(w*h*3/2*8*fps),\
LOSBYTE(w*h*3/2*8*fps),\
HISBYTE(w*h*3/2*8*fps),\
MSSBYTE(w*h*3/2*8*fps),\
LESBYTE(w*h*3/2*8*fps),\
LOSBYTE(w*h*3/2*8*fps),\
HISBYTE(w*h*3/2*8*fps),\
MSSBYTE(w*h*3/2*8*fps),\
LESBYTE(w*h*3/2),\
LOSBYTE(w*h*3/2),\
HISBYTE(w*h*3/2),\
MSSBYTE(w*h*3/2),\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),\
0x001,\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),

#define MJPEG_FORMAT_DESC(index,num) \
0x00B,\
0x024,\
0x006,\
index,\
num,\
0x000,\
0x001,\
0x000,\
0x000,\
0x000,\
0x000,

#define MJPEG_FRAME_DESC(num,w,h,size,fps) \
0x01E,\
0x024,\
0x007,\
num,\
0x000,\
LOBYTE(w),\
HIBYTE(w),\
LOBYTE(h),\
HIBYTE(h),\
LESBYTE(0x3C00*8*fps),\
LOSBYTE(0x3C00*8*fps),\
HISBYTE(0x3C00*8*fps),\
MSSBYTE(0x3C00*8*fps),\
LESBYTE(0x60000*8*fps),\
LOSBYTE(0x60000*8*fps),\
HISBYTE(0x60000*8*fps),\
MSSBYTE(0x60000*8*fps),\
LESBYTE(size),\
LOSBYTE(size),\
HISBYTE(size),\
MSSBYTE(size),\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),\
0x001,\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),

#define MJPEG_FRAME_DESC1(num,w,h,size,fps1,fps2) \
0x022,\
0x024,\
0x007,\
num,\
0x000,\
LOBYTE(w),\
HIBYTE(w),\
LOBYTE(h),\
HIBYTE(h),\
LESBYTE(0x3C00*8*fps1),\
LOSBYTE(0x3C00*8*fps1),\
HISBYTE(0x3C00*8*fps1),\
MSSBYTE(0x3C00*8*fps1),\
LESBYTE(0x60000*8*fps1),\
LOSBYTE(0x60000*8*fps1),\
HISBYTE(0x60000*8*fps1),\
MSSBYTE(0x60000*8*fps1),\
LESBYTE(size),\
LOSBYTE(size),\
HISBYTE(size),\
MSSBYTE(size),\
LESBYTE(10000000/fps1),\
LOSBYTE(10000000/fps1),\
HISBYTE(10000000/fps1),\
MSSBYTE(10000000/fps1),\
0x002,\
LESBYTE(10000000/fps1),\
LOSBYTE(10000000/fps1),\
HISBYTE(10000000/fps1),\
MSSBYTE(10000000/fps1),\
LESBYTE(10000000/fps2),\
LOSBYTE(10000000/fps2),\
HISBYTE(10000000/fps2),\
MSSBYTE(10000000/fps2),

#define MJPEG_STILL_DESC(len,num,...) \
len,\
0x024,\
0x003,\
0x000,\
num,\
__VA_ARGS__,\
0x001,\
0x003,

#define H264_FORMAT_DESC(index,num) \
0x01C,\
0x024,\
0x10,\
index,\
num,\
0x048,\
0x032,\
0x036,\
0x034,\
0x000,\
0x000,\
0x010,\
0x000,\
0x080,\
0x000,\
0x000,\
0x0AA,\
0x000,\
0x038,\
0x09B,\
0x071,\
0x010,\
0x001,\
0x000,\
0x000,\
0x000,\
0x000,\
0x001,

#define H264_FRAME_DESC(num,w,h,fps) \
0x026,\
0x024,\
0x011,\
num,\
0x000,\
LOBYTE(w),\
HIBYTE(w),\
LOBYTE(h),\
HIBYTE(h),\
LESBYTE(0x80000*8*fps),\
LOSBYTE(0x80000*8*fps),\
HISBYTE(0x80000*8*fps),\
MSSBYTE(0x80000*8*fps),\
LESBYTE(0x100000*8*fps),\
LOSBYTE(0x100000*8*fps),\
HISBYTE(0x100000*8*fps),\
MSSBYTE(0x100000*8*fps),\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),\
0x003,\
0x000,\
0x000,\
0x000,\
0x000,\
LESBYTE(10000000/fps),\
LOSBYTE(10000000/fps),\
HISBYTE(10000000/fps),\
MSSBYTE(10000000/fps),\
LESBYTE(12500000/fps),\
LOSBYTE(12500000/fps),\
HISBYTE(12500000/fps),\
MSSBYTE(12500000/fps),\
0x020,\
0x0A1,\
0x007,\
0x000,

#define H264_STILL_DESC(len,num,...) \
len,\
0x024,\
0x003,\
0x000,\
num,\
__VA_ARGS__,\
0x001,\
0x003,

#define COLOR_DESC(color,gamma,matrix) \
0x006,\
0x024,\
0x00D,\
color,\
gamma,\
matrix,

#define VC_OT_DESC(id,srcid) \
0x009,\
0x024,\
0x003,\
id,\
0x001,\
0x001,\
0x000,\
srcid,\
0x000,

#define VC_PU_DESC(id,srcid,bitmap) \
0x00B,\
0x024,\
0x005,\
id,\
srcid,\
0x000,\
0x000,\
0x002,\
LOBYTE(bitmap),\
HIBYTE(bitmap),\
0x000,

#define VC_IT_DESC(id,bitmap) \
0x012,\
0x024,\
0x002,\
id,\
0x001,\
0x002,\
0x000,\
0x000,\
0x000,\
0x000,\
0x000,\
0x000,\
0x000,\
0x000,\
0x003,\
LESBYTE(bitmap),\
LOSBYTE(bitmap),\
HISBYTE(bitmap),

#define VC_XU_DESC(id,srcid,uuid,cn,bitmap) \
0x01B,\
0x024,\
0x006,\
id,\
uuid,\
cn,\
0x001,\
srcid,\
0x002,\
LOBYTE(bitmap),\
HIBYTE(bitmap),\
0x000,

#define UVC_EU2_GUID    UUID(0x2C49D16A,0x32B8,0x4485,0x3EA8643A,0x152362F2)

#define UVC_EU1_GUID    UUID(0x23E49ED0,0x1178,0x4f31,0xAE52D2FB,0x8A8D3B48)

#if USB_UVC_SKYPE
// {B42153BD-35D6-45CA-B203-4E0149B301BC} for Skype API
#define UVC_EU0_GUID    UUID(0xB42153BD,0x35D6,0x45CA,0xB2034E01,0x49B301BC)
#endif

#if SKYPE_UVC_H264
// {A29E7641-DE04-47E3-8B2B-F4341AFF003B} for USBIF UVC 1.1 for H.264 Payload
#define UVC_EU3_GUID    UUID(0xA29E7641,0xDE04,0x47E3,0x8B2BF434,0x1AFF003B)
#endif

#define UAC_PCM_FORMAT_DESC(len,ch,w,bit,num,...) \
len,\
0x024,\
0x002,\
0x001,\
ch,\
w,\
bit,\
num,\
__VA_ARGS__,


//#if (SUPPORT_UVC_FUNC==1)
MMP_UBYTE   DEVICE_QUALIFIER_DESCRIPTOR_DATA1[0x0A] =
{
// device qualifier descriptor
0x00a,
0x006,
0x000,
0x002,
0x0EF,
0x002,
0x001,
0x040,
0x001,
0x000,
};                    

MMP_UBYTE   DEVICE_DESCRIPTOR_DATA1[0x12] =
{
// device descriptor
0x012,
0x001,
0x000,
0x002,
0x0EF,
0x002,
0x001,
0x040,

LOBYTE(VENDOR_ID),      
HIBYTE(VENDOR_ID),      
LOBYTE(PCCAM_ID),     
HIBYTE(PCCAM_ID),     

LOBYTE(FW_VER_BCD), //0x000,
HIBYTE(FW_VER_BCD), //0x001,
0x001,
0x002,
#if (CUSTOMER == LIT)
0x003,
#else
0x000,
#endif
0x001
};                    


MMP_UBYTE   HS_CONFIG_DESCRIPTOR_DATA1[] =//+0x6B
{
// configuration descriptor
0x009,
0x002,
LOBYTE(0),      
HIBYTE(0), 

#if SUPPORT_PCSYNC==0     
    #if SUPPORT_UAC
    0x004,// num of interface
    #else
    0x002,// num of interface
    #endif
#else
    #if SUPPORT_UAC
    0x005,// num of interface
    #else
    0x003,// num of interface
    #endif
#endif

0x001,
0x000,
0x080,
0x0FA,                    

// *******************************************************
// Interface Association Descriptor
0x008,
0x00B,
0x000,// first interface
0x002,// num of interface
0x00E,// class:video class
0x003,// sub-class:video interface collection 
0x000,
0x000,
// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x000,// interface
0x000,// alt.
0x001,
0x00E,// class:video class
SC_VIDEOCONTROL, //0x001,// sub-class:video control
0x000,
0x000,

// VC Interface Descriptor
0x00D,
CS_INTERFACE,//0x024,
VC_HEADER,//0x001,// Header
0x000,// UVC Version 1.0 ( xp not support ver 1.1 )
0x001,
#if USB_UVC_SKYPE
#if SKYPE_UVC_H264
0x09F,
0x000,
#else
0x084,
0x000,
#endif
#else
0x069,//0x068,// total size
0x000,
#endif
LESBYTE(USB_STC_CLK),// sys clock - 300MHZ
LOSBYTE(USB_STC_CLK),
HISBYTE(USB_STC_CLK),
MSSBYTE(USB_STC_CLK),
0x001,// num of video stream
0x001,// interface of video stream



#if SKYPE_UVC_H264
// Interface Output Terminal Descriptor
VC_OT_DESC(UVC_OT_ID, UVC_EU3_ID)  // TID = 0x04, src ID = 0x02

// {A29E7641-DE04-47E3-8B2B-F4341AFF003B} for USBIF UVC 1.1 for H.264 Payload
VC_XU_DESC(UVC_EU3_ID, UVC_EU2_ID, UVC_EU3_GUID, 0x0B, 0x7F07)
#else
// Interface Output Terminal Descriptor
VC_OT_DESC(UVC_OT_ID, UVC_EU2_ID)  // TID = 0x04, src ID = 0x02
#endif

// Interface Extension Unit Descriptor
// {2C49D16A-32B8-4485-3EA8-643A152362F2} for LGT
VC_XU_DESC(UVC_EU2_ID, UVC_EU1_ID, UVC_EU2_GUID, 0x06, 0x003F)



#if USB_UVC_SKYPE
// Interface Extension Unit Descriptor
// {B42153BD-35D6-45CA-B203-4E0149B301BC} for Skype API
VC_XU_DESC(UVC_EU0_ID, UVC_PU_ID, UVC_EU0_GUID, 0x10, 0x7E7F)
// Interface Extension Unit Descriptor
// {23E49ED0-1178-4f31-AE52-D2FB8A8D3B48} for AIT XU
VC_XU_DESC(UVC_EU1_ID, UVC_EU0_ID, UVC_EU1_GUID, 0x05, 0x6FFF)
#else
VC_XU_DESC(UVC_EU1_ID, UVC_PU_ID, UVC_EU1_GUID, 0x05, 0x6FFF)
#endif


// Interface Input(Camera) Terminal Descriptor
#if SUPPORT_AUTO_FOCUS
VC_IT_DESC(UVC_IT_ID,0x020A2E)
#else
VC_IT_DESC(UVC_IT_ID,0x000A0E)
#endif

// Interface Processing Unit Descriptor
//VC_PU_DESC(UVC_PU_ID,UVC_IT_ID,0x177F)
VC_PU_DESC(UVC_PU_ID,UVC_IT_ID,0x175B)


// Endpoint Descriptor
0x007,
0x005,
(0x080|PCCAM_EX_EP_ADDR),// EP3
0x003,// INT
0x040,//0x008,// size
0x000,
0x008,//0x004,// interval

// Endpoint Interrupt Descriptor
0x005,
0x025,
0x003,// interrupt
0x040,//0x008,// size
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x001,// interface
0x000,// alt.
#if USB_UVC_BULK_EP
0x001,// eps
#else
0x000,// eps
#endif
0x00E,// class:video class
SC_VIDEOSTREAMING,// sub-class:video streaming
0x000,
0x000,

// Interface Input Header Descriptor
0x00D +ENABLE_YUY2 + ENABLE_NV12 + ENABLE_MJPEG + ENABLE_FRAME_BASE_H264 ,
CS_INTERFACE,//0x024,
VS_INPUT_HEADER,//0x001,// Input Header
ENABLE_YUY2 + ENABLE_NV12 + ENABLE_MJPEG + ENABLE_FRAME_BASE_H264 ,
LOBYTE(UVC_INPUT_HEADER_LENGTH),      
HIBYTE(UVC_INPUT_HEADER_LENGTH),      
(0x080 |PCCAM_TX_EP1_ADDR),// video endpoint
0x000,
UVC_OT_ID, //0x004,//0x003,// linkID
#if USING_STILL_METHOD_1
0x000,//0x002, //0x001,//still method 1 //0x002,// still method 2
#else
0x002,//0x002, //0x001,//still method 1 //0x002,// still method 2
#endif
0x000,// Hardware still trigger support, 0: Not supported, 1: Supported
0x000,// Trigger usage, 0:Initiate still image capture, 1: General purpose button event
0x001,// bControlSize, size of each bmaControls(x) field, in bytes
#if ENABLE_YUY2==1
0x000,
#endif
#if ENABLE_NV12==1
0x000,
#endif
#if ENABLE_MJPEG==1
0x000,
#endif
#if ENABLE_FRAME_BASE_H264==1
0x000,
#endif



#if ENABLE_NV12==1

NV12_FORMAT_DESC(NV12_FORMAT_INDEX, NUM_NV12_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
NV12_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
NV12_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
NV12_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
NV12_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
NV12_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
NV12_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
NV12_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
NV12_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
NV12_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
NV12_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
NV12_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
NV12_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
NV12_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
NV12_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
NV12_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
NV12_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
NV12_FRAME_DESC(0x11,1920,1080, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
NV12_FRAME_DESC(0x12,2304,1296, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
NV12_FRAME_DESC(0x13,2304,1536, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
NV12_FRAME_DESC(0x14, 272, 144, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
NV12_FRAME_DESC(0x15, 368, 208, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
NV12_FRAME_DESC(0x16, 384, 216, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
NV12_FRAME_DESC(0x17, 480, 272, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
NV12_FRAME_DESC(0x18, 624, 352, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
NV12_FRAME_DESC(0x19, 912, 512, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
NV12_FRAME_DESC(0x1a, 424, 240, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
NV12_FRAME_DESC(0x1b, 480, 270, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
NV12_FRAME_DESC(0x1c, 848, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
NV12_FRAME_DESC(0x1d, 960, 540, 30)


// Interface Color Format
COLOR_DESC(1,1,4)

#endif // NV12

// Interface Format MJPEG Descriptor
#if ENABLE_MJPEG==1

MJPEG_FORMAT_DESC(MJPEG_FORMAT_INDEX, NUM_MJPG_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
MJPEG_FRAME_DESC(0x01, 640, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
MJPEG_FRAME_DESC(0x02,1920,1080, 0x100000,  30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
MJPEG_FRAME_DESC1(0x03,1280, 720, 0x100000, 60, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
MJPEG_FRAME_DESC(0x04, 640, 360, 0x100000,  30)

/*
// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
MJPEG_FRAME_DESC(0x01, 640, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
MJPEG_FRAME_DESC(0x02, 160,  90, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
MJPEG_FRAME_DESC(0x03, 160, 120, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
MJPEG_FRAME_DESC(0x04, 176, 144, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
MJPEG_FRAME_DESC(0x05, 320, 180, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
MJPEG_FRAME_DESC(0x06, 320, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
MJPEG_FRAME_DESC(0x07, 352, 288, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
MJPEG_FRAME_DESC(0x08, 432, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
MJPEG_FRAME_DESC(0x09, 640, 360, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
MJPEG_FRAME_DESC(0x0A, 800, 448, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
MJPEG_FRAME_DESC(0x0B, 800, 600, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
MJPEG_FRAME_DESC(0x0C, 864, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
MJPEG_FRAME_DESC(0x0D, 960, 720, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
MJPEG_FRAME_DESC(0x0E,1024, 576, 0x100000, 30)

#if (CUSTOMER == RAZ)
// Interface Frame Uncompressed Descriptor ( 1280x720 60fps )
MJPEG_FRAME_DESC(0x0F,1280, 720, 0x100000, 60)
#else
// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
MJPEG_FRAME_DESC(0x0F,1280, 720, 0x100000, 30)
#endif

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
MJPEG_FRAME_DESC(0x10,1600, 896, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
MJPEG_FRAME_DESC(0x11,1920,1080, 0x100000, 30)
*/

/*
// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
MJPEG_FRAME_DESC(0x12,2304,1296, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
MJPEG_FRAME_DESC(0x13,2304,1536, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
MJPEG_FRAME_DESC(0x14, 272, 144, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
MJPEG_FRAME_DESC(0x15, 368, 208, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
MJPEG_FRAME_DESC(0x16, 384, 216, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
MJPEG_FRAME_DESC(0x17, 480, 272, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
MJPEG_FRAME_DESC(0x18, 624, 352, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
MJPEG_FRAME_DESC(0x19, 912, 512, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
MJPEG_FRAME_DESC(0x1a, 424, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
MJPEG_FRAME_DESC(0x1b, 480, 270, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
MJPEG_FRAME_DESC(0x1c, 848, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
MJPEG_FRAME_DESC(0x1d, 960, 540, 0x100000, 30)
*/


#if (USING_STILL_METHOD_1 == 0) && (NUM_MJPG_STILL!=0)
//Interface Still Image Frame Descriptor
MJPEG_STILL_DESC(USB_UVC_MJPEG_STILL_DESC_SIZE, NUM_MJPG_STILL, \
                 (MMP_UBYTE)1280,(MMP_UBYTE)(1280>>8),(MMP_UBYTE)720,(MMP_UBYTE)(720>>8),\
                 (MMP_UBYTE)800,(MMP_UBYTE)(800>>8),(MMP_UBYTE)600,(MMP_UBYTE)(600>>8),\
                 (MMP_UBYTE)640,(MMP_UBYTE)(640>>8),(MMP_UBYTE)480,(MMP_UBYTE)(480>>8),\
                 (MMP_UBYTE)320,(MMP_UBYTE)(320>>8),(MMP_UBYTE)240,(MMP_UBYTE)(240>>8),\
                 (MMP_UBYTE)160,(MMP_UBYTE)(160>>8),(MMP_UBYTE)120,(MMP_UBYTE)(120>>8))
#endif

// Interface Color Format
COLOR_DESC(1,1,4)

#endif //ENABLE_MJPEG


#if ENABLE_YUY2==1//
// Interface Format Uncompressed Descriptor
// 32595559-0000-0010-8000-00AA00389B71  'YUY2' == MEDIASUBTYPE_YUY2
YUY2_FORMAT_DESC(YUY2_FORMAT_INDEX, NUM_YUY2_VIDEO)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
//YUY2_FRAME_DESC(0x01,1920,1080, 6)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
//YUY2_FRAME_DESC(0x02,1280, 720, 13)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
YUY2_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
YUY2_FRAME_DESC(0x04, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
//YUY2_FRAME_DESC(0x04, 864, 480, 30)

/*
// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
YUY2_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
YUY2_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
YUY2_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
YUY2_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
YUY2_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
YUY2_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
YUY2_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
YUY2_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
YUY2_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
YUY2_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
YUY2_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
YUY2_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
YUY2_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
YUY2_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
YUY2_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
YUY2_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
YUY2_FRAME_DESC(0x11,1920,1080, 30)
*/


/*
// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
YUY2_FRAME_DESC(0x12,2304,1296, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
YUY2_FRAME_DESC(0x13,2304,1536, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
YUY2_FRAME_DESC(0x14, 272, 144, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
YUY2_FRAME_DESC(0x15, 368, 208, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
YUY2_FRAME_DESC(0x16, 384, 216, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
YUY2_FRAME_DESC(0x17, 480, 272, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
YUY2_FRAME_DESC(0x18, 624, 352, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
YUY2_FRAME_DESC(0x19, 912, 512, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
YUY2_FRAME_DESC(0x1a, 424, 240, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
YUY2_FRAME_DESC(0x1b, 480, 270, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
YUY2_FRAME_DESC(0x1c, 848, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
YUY2_FRAME_DESC(0x1d, 960, 540, 30)
*/


#if (USING_STILL_METHOD_1 == 0) &&(NUM_YUY2_STILL!=0)
//Interface Still Image Frame Descriptor
YUY2_STILL_DESC(USB_UVC_YUY2_STILL_DESC_SIZE, NUM_YUY2_STILL, 640, 640>>8, 480, 480>>8, 320, 320>>8, 240, 240>>8, 160, 160>>8, 120, 120>>8)
#endif

// Interface Color Format
COLOR_DESC(1,1,4)

#endif	//ENABLE_YUY2

#if ENABLE_FRAME_BASE_H264==1
// Interface Format Uncompressed Descriptor
// 31544941-0000-0010-8000-00AA00389B71  'AIT1' == MEDIASUBTYPE_AIT1
// 0x34363248,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71 //FOURCC H264
H264_FORMAT_DESC(FRAME_BASE_H264_INDEX, NUM_H264_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
H264_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
H264_FRAME_DESC(0x02,1920,1080, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
H264_FRAME_DESC(0x03,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
H264_FRAME_DESC(0x04, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
//H264_FRAME_DESC(0x04, 864, 480, 30)

/*
H264_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
H264_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
H264_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
H264_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
H264_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
H264_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
H264_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
H264_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
H264_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
H264_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
H264_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
H264_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
H264_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
H264_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
H264_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
H264_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
H264_FRAME_DESC(0x11,1920,1080, 30)
*/


/*
// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
H264_FRAME_DESC(0x12,2304,1296, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
H264_FRAME_DESC(0x13,2304,1536, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
H264_FRAME_DESC(0x14, 272, 144, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
H264_FRAME_DESC(0x15, 368, 208, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
H264_FRAME_DESC(0x16, 384, 216, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
H264_FRAME_DESC(0x17, 480, 272, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
H264_FRAME_DESC(0x18, 624, 352, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
H264_FRAME_DESC(0x19, 912, 512, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
H264_FRAME_DESC(0x1a, 424, 240, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
H264_FRAME_DESC(0x1b, 480, 270, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
H264_FRAME_DESC(0x1c, 848, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
H264_FRAME_DESC(0x1d, 960, 540, 30)
*/

// Interface Color Format
COLOR_DESC(1,1,4)

#endif //ENABLE_FRAME_BASE_H264

#if USB_UVC_BULK_EP
#else
// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x001,// interface
0x001,// alt.
0x001,// eps
0x00E,// class:video class
SC_VIDEOSTREAMING,// sub-class:video streaming
0x000,
0x000,
#endif

// Endpoint Descriptor
0x007,// 
0x005,// endpoint descriptor type
//0x082,// EP2
(0x80 | PCCAM_TX_EP1_ADDR),
// Tx Type
#if USB_UVC_BULK_EP
0x002, // Bulk
// Max Packet Size
LOBYTE(HS_TX_MAX_PK_SIZE ),
HIBYTE(HS_TX_MAX_PK_SIZE ),
#else
0x005, // ISO
// Max Packet Size
LOBYTE(HS_TX_MAX_PK_SIZE | TX_ADD_FRAME_BITS(TX_PER_FRAME-1)),
HIBYTE(HS_TX_MAX_PK_SIZE | TX_ADD_FRAME_BITS(TX_PER_FRAME-1)),
#endif

// Interval
#if USB_UVC_BULK_EP
0x000,
#else
0x001,
#endif

#if SUPPORT_UAC

// *******************************************************
// Interface Association Descriptor
0x008,
0x00B,
0x002,// first interface
0x002,// num of interface
0x001,// class:audio class
0x002,// sub-class:audio stream
0x000,
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x002,// interface
0x000,// alt.
0x000,
0x001,// class:audio class
0x001,// sub-class:audio control
0x000,
0x000,

// Interface Header Descriptor
0x009,
0x024,
0x001,// Header
0x000,// UAC Version 1.0 ( xp not support ver 2.0 )
0x001,
//0x026,// total size
//0x000,
LOBYTE(USB_UAC_IF_CTL_TOTAL_SIZE),      
HIBYTE(USB_UAC_IF_CTL_TOTAL_SIZE),      
0x001,// num of audio stream
0x003,// interface of audio stream

// Interface Input Terminal Descriptor
0x00C,
0x024,
0x002,// Input Terminal
0x001,// TID
0x001,// input type Microphone
0x002,
0x000,
AUDIN_CHANNEL,
#if AUDIN_CHANNEL==2
0x003,
#endif
#if AUDIN_CHANNEL==1
0x000,
#endif
0x000,
0x000,
0x000,

// Interface Output Terminal Descriptor
0x009,
0x024,
0x003,// Output Terminal
0x003,// TID
0x001,// output type streaming
0x001,
0x000,// assoc TID ( 0 or 1? )
0x005,// src ID
0x000,

// Interface Feature Unit Descriptor
0x008,
0x024,
0x006,// Feature Unit
0x005,// UID
0x001,// src ID
0x001,// num of control byte
0x003,// mute, volume
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x003,// interface
0x000,// alt.
0x000,
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x003,// interface
0x001,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

// Interface General Descriptor
0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(16000),LOSBYTE(16000),HISBYTE(16000)\
/*,LESBYTE(24000),LOSBYTE(24000),HISBYTE(24000)\*/
)
#if 0
// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR ),
0x005,// ISO

(0x44 & 0xFF),
(0x44 >> 8),

0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,


// *******************************************************
// Interface Descriptor , alternate setting 2
0x009,
0x004,
0x003,// interface
0x002,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(24000),LOSBYTE(24000),HISBYTE(24000)\
)

// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0x64 & 0xFF),
(0x64 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,

// *******************************************************
// Interface Descriptor , alternate setting 3
0x009,
0x004,
0x003,// interface
0x003,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(32000),LOSBYTE(32000),HISBYTE(32000)\
)

// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0x84 & 0xFF),
(0x84 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,


// *******************************************************
// Interface Descriptor , alternate setting 4
0x009,
0x004,
0x003,// interface
0x004,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(48000),LOSBYTE(48000),HISBYTE(48000)\
)
#endif
// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0xc4 & 0xFF),
(0xc4 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,
#endif  // end of #if SUPPORT_UAC

#if SUPPORT_PCSYNC
    // Interface Descriptor
    0x009, //Length
    INTERFACE_DESCR, //descriptor type
    #if SUPPORT_UAC
    0x004,
    #else
    0x003,
    #endif
    0x000,	//alt.
    0x002,	//num endpoint
    0x0FF,	//class
    0x0FF,	//sub-class
    0x000,	//protocol
    0x000,	//

    // Endpint Descriptor
    0x007,
    ENDPOINT_DESCR,
    0x080|PCSYNC_EP_ADDR,// ep2 - in
    0x002,// bulk
    //0x000,0x002,//max packet size
    LOBYTE(PCSYNC_EP_MAX_PK_SIZE),
    HIBYTE(PCSYNC_EP_MAX_PK_SIZE),
    0x000,
    // Endpint Descriptor
    0x007,
    ENDPOINT_DESCR,
    PCSYNC_EP_ADDR,// ep2 - out
    0x002,// bulk
    //0x000,0x002,//max packet size
    LOBYTE(PCSYNC_EP_MAX_PK_SIZE),
    HIBYTE(PCSYNC_EP_MAX_PK_SIZE),
    0x000,

#endif


};

MMP_UBYTE   CONFIG_DESCRIPTOR_DATA1[] = 
{
// configuration descriptor
0x009,
0x002,
LOBYTE(0),      
HIBYTE(0),      
#if SUPPORT_UAC
0x004,// num of interface
#else
0x002,// num of interface
#endif
0x001,
0x000,
0x080,
0x0FA,                    

// *******************************************************
// Interface Association Descriptor
0x008,
0x00B,
0x000,// first interface
0x002,// num of interface
0x00E,// class:video class
0x003,// sub-class:video interface collection 
0x000,
0x000,
// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x000,// interface
0x000,// alt.
0x001,
0x00E,// class:video class
SC_VIDEOCONTROL, //0x001,// sub-class:video control
0x000,
0x000,

// VC Interface Descriptor
0x00D,
CS_INTERFACE,//0x024,
VC_HEADER,//0x001,// Header
0x000,// UVC Version 1.0 ( xp not support ver 1.1 )
0x001,
#if USB_UVC_SKYPE
#if SKYPE_UVC_H264
0x09F,
0x000,
#else
0x084,
0x000,
#endif
#else
0x069,//0x068,// total size
0x000,
#endif
LESBYTE(USB_STC_CLK),// sys clock - 300MHZ
LOSBYTE(USB_STC_CLK),
HISBYTE(USB_STC_CLK),
MSSBYTE(USB_STC_CLK),
0x001,// num of video stream
0x001,// interface of video stream



#if SKYPE_UVC_H264
// Interface Output Terminal Descriptor
VC_OT_DESC(UVC_OT_ID, UVC_EU3_ID)  // TID = 0x04, src ID = 0x02

// {A29E7641-DE04-47E3-8B2B-F4341AFF003B} for USBIF UVC 1.1 for H.264 Payload
VC_XU_DESC(UVC_EU3_ID, UVC_EU2_ID, UVC_EU3_GUID, 0x0B, 0x7F07)
#else
// Interface Output Terminal Descriptor
VC_OT_DESC(UVC_OT_ID, UVC_EU2_ID)  // TID = 0x04, src ID = 0x02
#endif

// Interface Extension Unit Descriptor
// {2C49D16A-32B8-4485-3EA8-643A152362F2} for LGT
VC_XU_DESC(UVC_EU2_ID, UVC_EU1_ID, UVC_EU2_GUID, 0x06, 0x003F)



#if USB_UVC_SKYPE
// Interface Extension Unit Descriptor
// {B42153BD-35D6-45CA-B203-4E0149B301BC} for Skype API
VC_XU_DESC(UVC_EU0_ID, UVC_PU_ID, UVC_EU0_GUID, 0x10, 0x7E7F)
// Interface Extension Unit Descriptor
// {23E49ED0-1178-4f31-AE52-D2FB8A8D3B48} for AIT XU
VC_XU_DESC(UVC_EU1_ID, UVC_EU0_ID, UVC_EU1_GUID, 0x05, 0x6FFF)
#else
VC_XU_DESC(UVC_EU1_ID, UVC_PU_ID, UVC_EU1_GUID, 0x05, 0x6FFF)
#endif


// Interface Input(Camera) Terminal Descriptor
#if SUPPORT_AUTO_FOCUS
VC_IT_DESC(UVC_IT_ID,0x020A2E)
#else
VC_IT_DESC(UVC_IT_ID,0x000A0E)
#endif

// Interface Processing Unit Descriptor
//VC_PU_DESC(UVC_PU_ID,UVC_IT_ID,0x177F)
VC_PU_DESC(UVC_PU_ID,UVC_IT_ID,0x175B)


// Endpoint Descriptor
0x007,
0x005,
(0x080|PCCAM_EX_EP_ADDR),// EP3
0x003,// INT
0x040,//0x008,// size
0x000,
0x008,//0x004,// interval

// Endpoint Interrupt Descriptor
0x005,
0x025,
0x003,// interrupt
0x040,//0x008,// size
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x001,// interface
0x000,// alt.
#if USB_UVC_BULK_EP
0x001,// eps
#else
0x000,// eps
#endif
0x00E,// class:video class
SC_VIDEOSTREAMING,// sub-class:video streaming
0x000,
0x000,

// Interface Input Header Descriptor
0x00D+ENABLE_YUY2 + ENABLE_NV12 + ENABLE_MJPEG + ENABLE_FRAME_BASE_H264,
CS_INTERFACE,//0x024,
VS_INPUT_HEADER,//0x001,// Input Header
ENABLE_YUY2 + ENABLE_NV12 + ENABLE_MJPEG + ENABLE_FRAME_BASE_H264 ,
LOBYTE(UVC_INPUT_HEADER_LENGTH),      
HIBYTE(UVC_INPUT_HEADER_LENGTH),      
(0x080 |PCCAM_TX_EP1_ADDR),// video endpoint
0x000,
UVC_OT_ID, //0x004,//0x003,// linkID
#if USING_STILL_METHOD_1
0x000,//0x002, //0x001,//still method 1 //0x002,// still method 2
#else
0x002,//0x002, //0x001,//still method 1 //0x002,// still method 2
#endif
0x000,// Hardware still trigger support, 0: Not supported, 1: Supported
0x000,// Trigger usage, 0:Initiate still image capture, 1: General purpose button event
0x001,// bControlSize, size of each bmaControls(x) field, in bytes
#if ENABLE_YUY2==1
0x000,
#endif
#if ENABLE_NV12==1
0x000,
#endif
#if ENABLE_MJPEG==1
0x000,
#endif
#if ENABLE_FRAME_BASE_H264==1
0x000,
#endif

#if ENABLE_NV12==1

NV12_FORMAT_DESC(NV12_FORMAT_INDEX, NUM_NV12_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
NV12_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
NV12_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
NV12_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
NV12_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
NV12_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
NV12_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
NV12_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
NV12_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
NV12_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
NV12_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
NV12_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
NV12_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
NV12_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
NV12_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
NV12_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
NV12_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
NV12_FRAME_DESC(0x11,1920,1080, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
NV12_FRAME_DESC(0x12,2304,1296, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
NV12_FRAME_DESC(0x13,2304,1536, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
NV12_FRAME_DESC(0x14, 272, 144, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
NV12_FRAME_DESC(0x15, 368, 208, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
NV12_FRAME_DESC(0x16, 384, 216, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
NV12_FRAME_DESC(0x17, 480, 272, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
NV12_FRAME_DESC(0x18, 624, 352, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
NV12_FRAME_DESC(0x19, 912, 512, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
NV12_FRAME_DESC(0x1a, 424, 240, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
NV12_FRAME_DESC(0x1b, 480, 270, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
NV12_FRAME_DESC(0x1c, 848, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
NV12_FRAME_DESC(0x1d, 960, 540, 30)

// Interface Color Format
COLOR_DESC(1,1,4)

#endif // NV12

// Interface Format MJPEG Descriptor
#if ENABLE_MJPEG==1

MJPEG_FORMAT_DESC(MJPEG_FORMAT_INDEX, NUM_MJPG_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
MJPEG_FRAME_DESC(0x01, 640, 480, 0x100000,  30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
MJPEG_FRAME_DESC(0x02,1920,1080, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
MJPEG_FRAME_DESC1(0x03,1280, 720, 0x100000, 60, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
MJPEG_FRAME_DESC(0x04, 640, 360, 0x100000,  30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
//MJPEG_FRAME_DESC(0x04, 864, 480, 0x100000,  30)


#if 0
// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
MJPEG_FRAME_DESC(0x01, 640, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
MJPEG_FRAME_DESC(0x02, 160,  90, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
MJPEG_FRAME_DESC(0x03, 160, 120, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
MJPEG_FRAME_DESC(0x04, 176, 144, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
MJPEG_FRAME_DESC(0x05, 320, 180, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
MJPEG_FRAME_DESC(0x06, 320, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
MJPEG_FRAME_DESC(0x07, 352, 288, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
MJPEG_FRAME_DESC(0x08, 432, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
MJPEG_FRAME_DESC(0x09, 640, 360, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
MJPEG_FRAME_DESC(0x0A, 800, 448, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
MJPEG_FRAME_DESC(0x0B, 800, 600, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
MJPEG_FRAME_DESC(0x0C, 864, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
MJPEG_FRAME_DESC(0x0D, 960, 720, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
MJPEG_FRAME_DESC(0x0E,1024, 576, 0x100000, 30)

#if (CUSTOMER == RAZ)
// Interface Frame Uncompressed Descriptor ( 1280x720 60fps )
MJPEG_FRAME_DESC(0x0F,1280, 720, 0x100000, 60)
#else // old
// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
MJPEG_FRAME_DESC(0x0F,1280, 720, 0x100000, 30)
#endif

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
MJPEG_FRAME_DESC(0x10,1600, 896, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
MJPEG_FRAME_DESC(0x11,1920,1080, 0x100000, 30)

/*
// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
MJPEG_FRAME_DESC(0x12,2304,1296, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
MJPEG_FRAME_DESC(0x13,2304,1536, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
MJPEG_FRAME_DESC(0x14, 272, 144, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
MJPEG_FRAME_DESC(0x15, 368, 208, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
MJPEG_FRAME_DESC(0x16, 384, 216, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
MJPEG_FRAME_DESC(0x17, 480, 272, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
MJPEG_FRAME_DESC(0x18, 624, 352, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
MJPEG_FRAME_DESC(0x19, 912, 512, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
MJPEG_FRAME_DESC(0x1a, 424, 240, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
MJPEG_FRAME_DESC(0x1b, 480, 270, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
MJPEG_FRAME_DESC(0x1c, 848, 480, 0x100000, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
MJPEG_FRAME_DESC(0x1d, 960, 540, 0x100000, 30)
*/
#endif
#if (USING_STILL_METHOD_1 == 0) && (NUM_MJPG_STILL!=0)
//Interface Still Image Frame Descriptor
MJPEG_STILL_DESC(USB_UVC_MJPEG_STILL_DESC_SIZE, NUM_MJPG_STILL, \
                 (MMP_UBYTE)1280,(MMP_UBYTE)(1280>>8),(MMP_UBYTE)720,(MMP_UBYTE)(720>>8),\
                 (MMP_UBYTE)800,(MMP_UBYTE)(800>>8),(MMP_UBYTE)600,(MMP_UBYTE)(600>>8),\
                 (MMP_UBYTE)640,(MMP_UBYTE)(640>>8),(MMP_UBYTE)480,(MMP_UBYTE)(480>>8),\
                 (MMP_UBYTE)320,(MMP_UBYTE)(320>>8),(MMP_UBYTE)240,(MMP_UBYTE)(240>>8),\
                 (MMP_UBYTE)160,(MMP_UBYTE)(160>>8),(MMP_UBYTE)120,(MMP_UBYTE)(120>>8))
#endif

// Interface Color Format
COLOR_DESC(1,1,4)

#endif //ENABLE_MJPEG


#if ENABLE_YUY2==1
// Interface Format Uncompressed Descriptor
// 32595559-0000-0010-8000-00AA00389B71  'YUY2' == MEDIASUBTYPE_YUY2
YUY2_FORMAT_DESC(YUY2_FORMAT_INDEX, NUM_YUY2_VIDEO)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
//YUY2_FRAME_DESC(0x01,1920,1080, 6)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
//YUY2_FRAME_DESC(0x02,1280, 720,13)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
YUY2_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
YUY2_FRAME_DESC(0x04, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
//YUY2_FRAME_DESC(0x04, 864, 480, 30)

/*
// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
YUY2_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
YUY2_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
YUY2_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
YUY2_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
YUY2_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
YUY2_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
YUY2_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
YUY2_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
YUY2_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
YUY2_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
YUY2_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
YUY2_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
YUY2_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
YUY2_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
YUY2_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
YUY2_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
YUY2_FRAME_DESC(0x11,1920,1080, 30)


// Interface Frame Uncompressed Descriptor ( 2304x1296 30fps )
YUY2_FRAME_DESC(0x12,2304,1296, 30)

// Interface Frame Uncompressed Descriptor ( 2304x1536 30fps )
YUY2_FRAME_DESC(0x13,2304,1536, 30)

// Interface Frame Uncompressed Descriptor ( 272x144 30fps )
YUY2_FRAME_DESC(0x14, 272, 144, 30)

// Interface Frame Uncompressed Descriptor ( 368x208 30fps )
YUY2_FRAME_DESC(0x15, 368, 208, 30)

// Interface Frame Uncompressed Descriptor ( 384x216 30fps )
YUY2_FRAME_DESC(0x16, 384, 216, 30)

// Interface Frame Uncompressed Descriptor ( 480x272 30fps )
YUY2_FRAME_DESC(0x17, 480, 272, 30)

// Interface Frame Uncompressed Descriptor ( 624x352 30fps )
YUY2_FRAME_DESC(0x18, 624, 352, 30)

// Interface Frame Uncompressed Descriptor ( 912x512 30fps )
YUY2_FRAME_DESC(0x19, 912, 512, 30)

// Interface Frame Uncompressed Descriptor ( 424x240 30fps )
YUY2_FRAME_DESC(0x1a, 424, 240, 30)

// Interface Frame Uncompressed Descriptor ( 480x270 30fps )
YUY2_FRAME_DESC(0x1b, 480, 270, 30)

// Interface Frame Uncompressed Descriptor ( 848x480 30fps )
YUY2_FRAME_DESC(0x1c, 848, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x540 30fps )
YUY2_FRAME_DESC(0x1d, 960, 540, 30)
*/

#if (USING_STILL_METHOD_1 == 0)&&(NUM_YUY2_STILL!=0)
//Interface Still Image Frame Descriptor
YUY2_STILL_DESC(USB_UVC_YUY2_STILL_DESC_SIZE, NUM_YUY2_STILL, 640, 640>>8, 480, 480>>8, 320, 320>>8, 240, 240>>8, 160, 160>>8, 120, 120>>8)
#endif

// Interface Color Format
COLOR_DESC(1,1,4)

#endif	//ENABLE_YUY2

#if ENABLE_FRAME_BASE_H264==1
// Interface Format Uncompressed Descriptor
// 31544941-0000-0010-8000-00AA00389B71  'AIT1' == MEDIASUBTYPE_AIT1
// 0x34363248,0x0000,0x0010,0x80,0x00,0x00,0xAA,0x00,0x38,0x9B,0x71 //FOURCC H264
H264_FORMAT_DESC(FRAME_BASE_H264_INDEX, NUM_H264_VIDEO)

// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
H264_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
H264_FRAME_DESC(0x02,1920,1080, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
H264_FRAME_DESC(0x03,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
H264_FRAME_DESC(0x04, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
//H264_FRAME_DESC(0x04, 864, 480, 30)

#if 0
// Interface Frame Uncompressed Descriptor ( 640x480 30fps )
H264_FRAME_DESC(0x01, 640, 480, 30)

// Interface Frame Uncompressed Descriptor ( 160x90 30fps )
H264_FRAME_DESC(0x02, 160,  90, 30)

// Interface Frame Uncompressed Descriptor ( 160x120 30fps )
H264_FRAME_DESC(0x03, 160, 120, 30)

// Interface Frame Uncompressed Descriptor ( 176x144 30fps )
H264_FRAME_DESC(0x04, 176, 144, 30)

// Interface Frame Uncompressed Descriptor ( 320x180 30fps )
H264_FRAME_DESC(0x05, 320, 180, 30)

// Interface Frame Uncompressed Descriptor ( 320x240 30fps )
H264_FRAME_DESC(0x06, 320, 240, 30)

// Interface Frame Uncompressed Descriptor ( 352x288 30fps )
H264_FRAME_DESC(0x07, 352, 288, 30)

// Interface Frame Uncompressed Descriptor ( 432x240 30fps )
H264_FRAME_DESC(0x08, 432, 240, 30)

// Interface Frame Uncompressed Descriptor ( 640x360 30fps )
H264_FRAME_DESC(0x09, 640, 360, 30)

// Interface Frame Uncompressed Descriptor ( 800x448 30fps )
H264_FRAME_DESC(0x0A, 800, 448, 30)

// Interface Frame Uncompressed Descriptor ( 800x600 30fps )
H264_FRAME_DESC(0x0B, 800, 600, 30)

// Interface Frame Uncompressed Descriptor ( 864x480 30fps )
H264_FRAME_DESC(0x0C, 864, 480, 30)

// Interface Frame Uncompressed Descriptor ( 960x720 30fps )
H264_FRAME_DESC(0x0D, 960, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1024x576 30fps )
H264_FRAME_DESC(0x0E,1024, 576, 30)

// Interface Frame Uncompressed Descriptor ( 1280x720 30fps )
H264_FRAME_DESC(0x0F,1280, 720, 30)

// Interface Frame Uncompressed Descriptor ( 1600x896 30fps )
H264_FRAME_DESC(0x10,1600, 896, 30)

// Interface Frame Uncompressed Descriptor ( 1920x1080 30fps )
H264_FRAME_DESC(0x11,1920,1080, 30)
#endif
// Interface Color Format
COLOR_DESC(1,1,4)

#endif //ENABLE_FRAME_BASE_H264

#if USB_UVC_BULK_EP
#else
// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x001,// interface
0x001,// alt.
0x001,// eps
0x00E,// class:video class
SC_VIDEOSTREAMING,// sub-class:video streaming
0x000,
0x000,
#endif

// Endpoint Descriptor
0x007,// 
0x005,// endpoint descriptor type
//0x082,// EP2
(0x80 | PCCAM_TX_EP1_ADDR ),
#if USB_UVC_BULK_EP
0x002,// BULK
#else
0x005,// ISO
#endif
// MaxPkSize
LOBYTE(FS_TX_MAX_PK_SIZE),
HIBYTE(FS_TX_MAX_PK_SIZE),
// Interval
#if USB_UVC_BULK_EP
0x000,
#else
0x001,
#endif

#if SUPPORT_UAC

// *******************************************************
// Interface Association Descriptor
0x008,
0x00B,
0x002,// first interface
0x002,// num of interface
0x001,// class:audio class
0x002,// sub-class:audio stream
0x000,
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x002,// interface
0x000,// alt.
0x000,
0x001,// class:audio class
0x001,// sub-class:audio control
0x000,
0x000,

// Interface Header Descriptor
0x009,
0x024,
0x001,// Header
0x000,// UAC Version 1.0 ( xp not support ver 2.0 )
0x001,
//0x026,// total size
//0x000,
LOBYTE(USB_UAC_IF_CTL_TOTAL_SIZE),      
HIBYTE(USB_UAC_IF_CTL_TOTAL_SIZE),      
0x001,// num of audio stream
0x003,// interface of audio stream

// Interface Input Terminal Descriptor
0x00C,
0x024,
0x002,// Input Terminal
0x001,// TID
0x001,// input type Microphone
0x002,
0x000,
AUDIN_CHANNEL,
#if AUDIN_CHANNEL==2
0x003,
#endif
#if AUDIN_CHANNEL==1
0x000,
#endif
0x000,
0x000,
0x000,

// Interface Output Terminal Descriptor
0x009,
0x024,
0x003,// Output Terminal
0x003,// TID
0x001,// output type streaming
0x001,
0x000,// assoc TID ( 0 or 1? )
0x005,// src ID
0x000,

// Interface Feature Unit Descriptor
0x008,
0x024,
0x006,// Feature Unit
0x005,// UID
0x001,// src ID
0x001,// num of control byte
0x003,// mute, volume
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x003,// interface
0x000,// alt.
0x000,
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

// *******************************************************
// Interface Descriptor
0x009,
0x004,
0x003,// interface
0x001,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

// Interface General Descriptor
0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(16000),LOSBYTE(16000),HISBYTE(16000)\
)
#if 0

// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0x44 & 0xFF),
(0x44 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,

// *******************************************************
// Interface Descriptor , alternate setting 2
0x009,
0x004,
0x003,// interface
0x002,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(24000),LOSBYTE(24000),HISBYTE(24000)\
)

// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0x64 & 0xFF),
(0x64 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,

// *******************************************************
// Interface Descriptor , alternate setting 3
0x009,
0x004,
0x003,// interface
0x003,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(32000),LOSBYTE(32000),HISBYTE(32000)\
)

// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0x84 & 0xFF),
(0x84 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,


// *******************************************************
// Interface Descriptor , alternate setting 4
0x009,
0x004,
0x003,// interface
0x004,// alt.
0x001,// eps
0x001,// class:audio class
0x002,// sub-class:audio streaming
0x000,
0x000,

0x007,
0x024,
0x001,// General
0x003,// endpoint link TID
AS_INTERNAL_DELAY,//0x001,// delay ms ( num of frames )
0x001,// FormatTag, 1: PCM data format
0x000,

UAC_PCM_FORMAT_DESC(USB_UAC_IF_STREAM_FORMAT_PCM_DESC_SIZE,AUDIN_CHANNEL,2,16,NUM_OF_UAC_SAMPLING_RATE\
,LESBYTE(48000),LOSBYTE(48000),HISBYTE(48000)\
)
#endif
// Endpoint Descriptor
0x009,
0x005,
//0x081,// EP1
(0x80 | PCCAM_AU_EP_ADDR),
0x005,// ISO
(0xc4 & 0xFF),
(0xc4 >> 8),
0x004,// 4 packets per frame
0x000,
0x000,

// Endpoint General Descriptor
0x007,
0x025,
0x001,// General
0x001,// pitch control
0x000,
0x000,
0x000,

#endif  // end of #if SUPPORT_UAC

};

MMP_UBYTE   LANGUAGE_ID_DATA[0x04] =
{
0x004, 0x003, 0x009, 0x004
};  


MMP_UBYTE   MANUFACTURER_STRING_DATA[] =
{
0x034,//0x01E,
0x003,
'A',0x00,
'l',0x00,
'p',0x00,
'h',0x00,
'a',0x00,
' ',0x00,
'I',0x00,
'm',0x00,
'a',0x00,
'g',0x00,
'i',0x00,
'n',0x00,
'g',0x00,
' ',0x00,
'T',0x00,
'e',0x00,
'c',0x00,
'h',0x00,
'.',0x00,
' ',0x00,
'C',0x00,
'o',0x00,
'r',0x00,
'p',0x00,
'.',0x00
};

MMP_UBYTE   PRODUCT_STRING_DATA[] =
{
0x16,//0x02A,//0x01E,
0x003,
'R',0x00,
'a',0x00,
'z',0x00,
'e',0x00,
'r',0x00,
' ',0x00,
'K',0x00,
'i',0x00,
'y',0x00,
'o',0x00
};         

MMP_UBYTE   SERIALNUMBER_STRING_DATA[] =
{
0x1a,
0x03,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
'0',
0x00,
};         

MMP_UBYTE *get_configuration_len(MMP_BOOL highspeed,MMP_USHORT *len)
{
    MMP_UBYTE *ptr ;
    if(highspeed) {
        *len =  sizeof(HS_CONFIG_DESCRIPTOR_DATA1) ;
        ptr = HS_CONFIG_DESCRIPTOR_DATA1 ;
    } else {
        *len =  sizeof(CONFIG_DESCRIPTOR_DATA1) ;
        ptr = CONFIG_DESCRIPTOR_DATA1 ;
    }
    return ptr ;
}


#endif  //#if   defined(USB_FW)

#undef exUSBDESCR

/// @}
