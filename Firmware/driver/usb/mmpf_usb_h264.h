#ifndef _MMPF_USB_H264_H
#define _MMPF_USB_H264_H
#include "mmpf_typedef.h"
// 3.4.1.1 Encoder Video Format Control
#define UT_DEFAULT              (0x00)
#define UT_REAL_TIME_STREAMING  (0x01)
#define UT_REAL_TIME_STREAMING_INTEROP  (0x02)
#define UT_BROADCAST            (0x03)
#define UT_STORAGE              (0x04)

#define DUAL_STREAM_EN          (1<<0)
#define DUAL_MJPEG_H264         (1<<1)
#define DUAL_MJPEG_YUY2         (1<<2)
#define DUAL_YUY2_H264          (1<<3) //sean@2011_01_22, uvc dual yuy2 + h264

//typedef __packed struct _H264_ENCODER_VFC_CFG 
//{
//    MMP_USHORT wWidth ;
//    MMP_USHORT wHeight;
//    MMP_ULONG  dwFramePeriod;
//    MMP_UBYTE  bUsageTarget ;
//    MMP_UBYTE  bStreamMuxOption;
//} H264_ENCODER_VFC_CFG ;

// 3.4.1.2 Encoder Configuration Control
// Slice Format
#define SF_NAL  (0x00) // NAL
#define SF_BYTE (0x01) // Byte Stream
// Profile
#define PROFILE_BASELINE    (66)
#define PROFILE_MAIN        (77)
#define PROFILE_HIGH        (100)
// Rate Control Mode
#define RCM_CBR (0) // CBR
#define RCM_VBR (1) // VBR

#define QM_USER_DEF (0 )
#define QM_LIMIT    (51)

typedef __packed struct _H264_ENCODER_CC_CFG
{
    MMP_ULONG   dwBitRate ;
    MMP_ULONG   dwBitRateMax;
    MMP_ULONG   dwRateControlDelay ;
    MMP_USHORT  wIDRFrequency ;
    MMP_USHORT  wAirrate ;
    MMP_USHORT  wSliceSize ;
    MMP_USHORT  wSliceMode ;
    MMP_UBYTE   bSliceFormat;
    MMP_UBYTE   bProfile ;
    MMP_UBYTE   bRateControlMode ;
    MMP_UBYTE   bQuantizerMin;
    MMP_UBYTE   bQuantizerMax;    
} H264_ENCODER_CC_CFG ;


// 3.4.1.3 Rate Control
typedef __packed struct _H264_ENCODER_RC_CFG
{
    MMP_ULONG   dwFramePeriod ;
    MMP_ULONG   dwBitRate ;
} H264_ENCODER_RC_CFG ;
// 3.4.1.4 Frame Type Control

#define FT_IVIDEO_NA_FRAME  (0)
#define FT_IVIDEO_I_FRAME   (1)
#define FT_IVIDEO_IDR_FRAME (2)
// 3.4.1.5 Camera Delay Control

// 3.4.1.6 Filter Control

typedef __packed struct _H264_ENCODER_FILTER_CFG
{
    MMP_UBYTE   bTemporalFilterStrength ;
    MMP_UBYTE   bSpatialFilterStrength ; 
} H264_ENCODER_FILTER_CFG ;


// 3.5.1 Payload header, 28 bytes for current implementation.
#define ST_UNDEF    (0 )
#define ST_YUY2     (41)
#define ST_NV12     (42)
#define ST_H264     (43)
#define ST_MJPEG    (44) // not in spec.
#define ST_Y        (45) // not in spec.
#define ST_M2TS     (46) // not in spec.
#define ST_H264_2   (53) // For stream viewer 2nd h264 stream
#define ST_JPEG_CAP (60)

// 32 bytes
// move to config_fw.h
//#define ALIGN_PAYLOAD  (1) // align payload offset to 4 bytes at least


typedef __packed union  _FRAME_PAYLOAD_HEADER
{
    __packed struct _UVC_H264_PH {
        MMP_USHORT wVersion ;
        MMP_USHORT wHeaderLen;
        MMP_ULONG  dwStreamType ; // Four CC
        MMP_USHORT wImageWidth ;
        MMP_USHORT wImageHeight;
        MMP_ULONG  dwFrameRate  ; // 100 ns Interval
        MMP_USHORT wDelay ;      // delay by encoder process
        MMP_ULONG  dwTimeStamp ;
#if  ALIGN_PAYLOAD==0       
        /*------Vendor info------*/
        MMP_ULONG  dwFrameSeq ;
        MMP_USHORT wReserved  ;
        /*------Vendor Info-------*/
#endif
        MMP_ULONG  dwPayloadSize;
    } UVC_H264_PH  ;
    
    __packed struct _SKYPE_H264_PH {
        MMP_ULONG64 dwTimeStamp ;
        MMP_UBYTE   bStreamID ;
        MMP_UBYTE   bStreamType     ;
        MMP_USHORT  wFrameSeq  ;
        MMP_ULONG   dwPayloadOffset ;
        MMP_ULONG   dwPayloadSize;
#if ALIGN_PAYLOAD==0        
        /*------Vendor info------*/
        MMP_USHORT wVersion;
        MMP_USHORT wImageWidth ;
        MMP_USHORT wImageHeight;
        MMP_USHORT wDelay ;
        MMP_ULONG  dwFrameRate  ;
        /*------Vendor info------*/
#endif        
    } SKYPE_H264_PH ;
    #if USB_FRAMEBASE_H264_DUAL_STREAM==1
    // for AIT StreamViewer tool
    __packed struct _FRAME_H264_PH {
        MMP_USHORT wVersion ;
        MMP_USHORT wHeaderLen;
        MMP_USHORT wStreamType ; // 41,42,43,44
        MMP_USHORT wImageWidth ;
        MMP_USHORT wImageHeight;
#if ALIGN_PAYLOAD==0
        MMP_USHORT wFrameRate  ;
        /*------Vendor info------*/
        MMP_ULONG  dwFrameSeq ;
#endif
        MMP_ULONG  dwTimeStamp ;
        MMP_ULONG  dwFlag ;
        MMP_ULONG  dwReserved ; 
        /*------Vendor Info-------*/
        MMP_ULONG  dwPayloadSize;
    } FRAME_H264_PH ;
    #endif
} FRAME_PAYLOAD_HEADER ;



//typedef __packed struct _UVC_H264_PH UVC_STREAM_PH ;

#define SKYPE_PAYLOAD_ID    0x000000EE
#define UVC_H264_VER        0x0100 // 0.9F
#define FRAME_PAYLOAD_HEADER_SZ   sizeof(FRAME_PAYLOAD_HEADER)  
#define UVC_H264_PH_LEN (FRAME_PAYLOAD_HEADER_SZ - 4)

#endif
