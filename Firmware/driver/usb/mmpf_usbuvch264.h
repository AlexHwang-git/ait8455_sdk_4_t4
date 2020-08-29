#ifndef _MMPF_USBUVC_H264_H
#define _MMPF_USBUVC_H264_H
#include "config_fw.h"
#include "mmpf_typedef.h"
#include "mmpf_usbpccam.h"

#if USB_UVC_H264==1

// 3.3 H.264 UVC Extension Unit definition
#define UVCX_VIDEO_UNDEFINED        (0x00)
#define UVCX_VIDEO_CONFIG_PROBE     (0x01)
#define UVCX_VIDEO_CONFIG_COMMIT    (0x02)
#define UVCX_RATE_CONTROL_MODE      (0x03)
#define UVCX_TEMPORAL_SCALE_MODE    (0x04)
#define UVCX_SPATIAL_SCALE_MODE     (0x05)
#define UVCX_SNR_SCALE_MODE         (0x06)
#define UVCX_LTR_BUFFER_SIZE_CONTROL (0x07)
#define UVCX_LTR_PICTURE_CONTROL    (0x08)
#define UVCX_PICTURE_TYPE_CONTROL   (0x09)
#define UVCX_VERSION                (0x0a)
#define UVCX_ENCODER_RESET          (0x0b)
#define UVCX_FRAMERATE_CONFIG       (0x0c)
#define UVCX_VIDEO_ADVANCE_CONFIG   (0x0d)
#define UVCX_BITRATE_LAYERS         (0x0e)
#define UVCX_QP_STEPS_LAYERS        (0x0f)

#define bMaxLayers      1
#define bMaxStreamId    (MAX_NUM_ENC_SET-1) // simulcast only support stream id 0 & 1

#define TEMPORAL_ID( wLayerID )  (   wLayerID & 0x7 )
#define DEPENDENCY_ID( wLayerID) ( ( wLayerID >> 3 ) & 0xf ) 
#define QUALITY_ID( wLayerID )   ( ( wLayerID >> 7 ) & 0x7 ) 
#define STREAM_ID( wLayerID )    ( ( wLayerID >> 10 ) & 0x7 ) 

#define LAYER_ID( sId,qId,dId,tId )  ( ( ( sId & 0x7) << 10 )| ( (qId & 0x07) << 7) | ( (dId & 0xf) << 3  ) | (tId & 0x07))
 

#define HINT_RES        (0x0001)
#define HINT_PROFILE    (0x0002)
#define HINT_RCMODE     (0x0004)
#define HINT_USAGETYPE  (0x0008)
#define HINT_SLICEMODE  (0x0010)
#define HINT_SLICEUNIT  (0x0020)
#define HINT_MVCVIEW    (0x0040)
#define HINT_TEMPORAL   (0x0080)
#define HINT_SNR        (0x0100)
#define HINT_SPATIAL    (0x0200)
#define HINT_SLRATIO    (0x0400)
#define HINT_FRAMERATE  (0x0800)
#define HINT_LBS        (0x1000)
#define HINT_BITRATE    (0x2000)
#define HINT_CABAC      (0x4000)
#define HINT_IFRAME     (0x8000)

#define CUR_HINT_BITS   (HINT_RES | HINT_PROFILE | HINT_RCMODE | HINT_USAGETYPE | \
                         HINT_SLICEMODE | HINT_SLICEUNIT | HINT_FRAMERATE | HINT_LBS | \
                         HINT_BITRATE | HINT_CABAC | HINT_IFRAME )

#define MUX_AUX_STREAM_DIS  (0x00)
#define MUX_AUX_STREAM_EN   (0x01)
#define MUX_AUX_H264        (0x02)
#define MUX_AUX_YUY2        (0x04)
#define MUX_AUX_NV12        (0x08)
#define MUX_AUX_JPEG        (0x40)

#define MUX_AUX_STREAM(x)      (MUX_AUX_STREAM_EN | MUX_AUX_JPEG | x)

typedef enum {
    SINGLE_SLICE  =0,
    MULTI_SLICE_BITs,
    MULTI_SLICE_MBs ,
    MULTI_SLICE_NUMs
} H264SliceMode ;

typedef enum {
    CONSTRAINED_BASELINE_FLAG1 = 0x4202 ,
    CONSTRAINED_BASELINE_P = 0x4240,
    BASELINE_P = 0x4200 ,
    MAIN_P = 0x4D00,
    CONSTRAINED_HIGH_P_FLAG1 = 0x6430,
    CONSTRAINED_HIGH_P = 0x640C , // New in 1.5e 
    HIGH_P = 0x6400,
    SCALABLE_CONSTRAINED_BASELINE_P = 0x5304 , // New in 1.5e
    SCALABLE_BASELINE_P=0x5300,
    SCALABLE_CONSTRAINED_HIGH__P = 0x5604 , // New in 1.5e
    SCALABALE_HIGH_P = 0x5600,
    MULTIVIEW_HIGH_P = 0x7600,
    STEREO_HIGH_P = 0x8000,
    CONSTRAINED_SET0_FLAG = 0x0080,
    CONSTRAINED_SET1_FLAG = 0x0040,
    CONSTRAINED_SET2_FLAG = 0x0020,
    CONSTRAINED_SET3_FLAG = 0x0010,
    CONSTRAINED_SET4_FLAG = 0x0008,
    CONSTRAINED_SET5_FLAG = 0x0004
   
} H264Profile ;

typedef enum {
    
    REALTIME_H264  = 1 ,
    BROADCAST_H264     ,
    STORAGE_H264       ,
    UCCONFIG_MODE1     ,
    UCCONFIG_MODE2     ,
    UCCONFIG_MODE3     ,
    UCCONFIG_MODE4     ,
    UCCONFIG_MODE5     ,
    UCCONFIG_MODE6     ,
    UCCONFIG_MODE7     ,
    UCCONFIG_MODE8     ,
    UCCONFIG_MODE9     ,
    UCCONFIG_MODE10    , 
    UCCONFIG_MODE11    , 
    UCCONFIG_MODE12     
} H264UsageType ;


typedef enum {
    RESERVED_H264=0,
    CBR_H264 = 1 ,
    VBR_H264 = 2 ,
    CONSTANT_H264_QP = 3,
    NUM_H264_CTLMODE,
    FIXED_FRAME_RATE = 0x10    
} H264RateControlMode ;

typedef enum {
    ONE_LAYER_H264 /*= 1*/,
    TWO_LAYER_H264 ,
    THREE_LAYER_H264 ,
    FOUR_LAYER_H264, 
    FIVE_LAYER_H264,
    SIX_LAYER_H264,
    SEVEN_LAYER_H264
} H264TemporalScaleMode ;

typedef enum {
    ONE_SPATIAL_LAYER /*= 1*/,
    TWO_SPATIAL_LAYER ,
    THREE_SPATIAL_LAYER ,
    FOUR_SPATIAL_LAYER ,
    FIVE_SPATIAL_LAYER ,
    SIX_SPATIAL_LAYER ,
    SEVEN_SPATIAL_LAYER ,
    EIGHT_SPATIAL_LAYER
} H264SpatialScaleMode ;

typedef enum {
    RESERVED_LAYER = 1 ,
    CGS_NONREWRITE_2LAYER,
    CGS_NONREWRITE_3LAYER,
    CGS_REWRITE_2LAYER,
    CGS_REWRITE_3LAYER,
    MGS_2LAYER ,
    ALLMOMODES 
    
} H264SNRScaleMode ;

typedef enum {
    ANNEXB_H264 = 0,
    NAL_H264
} H264StreamFormat ;

typedef enum {
    CAVLC_H264 =0,
    CABAC_H264 
} H264EntroyCABAC ;

// 3.3.3 UVCX_RATE_CONTROL_MODE
typedef __packed struct _UVCX_RateControlMode {
	MMP_USHORT	wLayerID;
	MMP_UBYTE	bRateControlMode;
} UVCX_RateControlMode ;
#define UVCX_RATECONTROLMODE_LEN    sizeof(UVCX_RateControlMode)


// 3.3.4 UVCX_TEMPORAL_SCALE_MODE
typedef __packed struct _UVCX_TemporalScaleMode {
    MMP_USHORT wLayerID ;
    MMP_UBYTE  bTemporalScaleMode ;
} UVCX_TemporalScaleMode ;
#define UVCX_TEMPORALSCALEMODE_LEN sizeof(UVCX_TemporalScaleMode)

// 3.3.5 UVCX_SPATIAL_SCALE_MODE
typedef __packed struct _UVCX_SpatialScaleMode {
    MMP_USHORT wLayerID ;
    MMP_UBYTE  bSpatialScaleMode ;
} UVCX_SpatialScaleMode ;
#define UVCX_SPATIALSCALEMODE_LEN sizeof(UVCX_SpatialScaleMode)

// 3.3.6 UVCX_SNR_SCALE_MODE
typedef __packed struct _UVCX_SNRScaleMode {
    MMP_USHORT wLayerID ;
    MMP_UBYTE  bSNRScaleMode ;
    MMP_UBYTE  bMGSSublayerMode ;
} UVCX_SNRScaleMode ;
#define UVCX_SNRSCALEMODE_LEN sizeof(UVCX_SNRScaleMode)

// 3.3.7 UVCX_LTR_BUFFER_SIZE_CONTROL
typedef __packed struct _UVCX_LTRBufferSizeControl {
    MMP_USHORT wLayerID ;
    MMP_UBYTE  bLTRBufferSize ;
    MMP_UBYTE  bLTREncoderControl;
} UVCX_LTRBufferSizeControl;
#define UVCX_LTRBUFFERSIZECONTROL_LEN   sizeof(UVCX_LTRBufferSizeControl)

// 3.3.8 UVCX_LTR_PICTURE_CONTROL
typedef __packed struct _UVCX_LTRPictureControl {
    MMP_USHORT wLayerID ;
    MMP_UBYTE  bPutAtPositionInLTRBuffer ;
    MMP_UBYTE  bEncodeUsingLTR;
} UVCX_LTRPictureControl ;
#define UVCX_LTRPICTURECONTROL_LEN sizeof(UVCX_LTRPictureControl)

// 3.3.9 UVCX_PICTURE_TYPE_CONTROL
typedef __packed struct _UVCX_PictureTypeControl {
    MMP_USHORT wLayerID ;
    MMP_USHORT wPicType ;
} UVCX_PictureTypeControl ;
#define UVCX_PICTYPE_LEN    sizeof(UVCX_PictureTypeControl)

// 3.3.12 UVCX_FRAMERATE_CONFIG
typedef __packed struct _UVCX_FrameRateConfig {
    MMP_USHORT wLayerID ;
    MMP_ULONG  dwFrameInterval;
} UVCX_FrameRateConfig ;
#define UVCX_FRAMERATE_LEN    sizeof(UVCX_FrameRateConfig)


// 3.3.13 UVCX_VIDEO_ADVANCE_CONFIG
typedef __packed struct _UVCX_AdvConfig {
    MMP_USHORT wLayerID ;
    MMP_ULONG dwMb_max;
    MMP_UBYTE bLevel_idc;
    MMP_UBYTE bReserved ;
} UVCX_AdvConfig ;
#define UVCX_ADVCONFIG_LEN  sizeof(UVCX_AdvConfig)

// 3.3.14 UVCX_BITRATE_LAYERS
typedef __packed struct _UVCX_BitRateLayers {
    MMP_USHORT wLayerID ;
    MMP_ULONG dwPeekBitrate ;
    MMP_ULONG dwAverageBitrate;
} UVCX_BitRateLayers ;
#define UVCX_BITRATE_LEN    (12)

// 3.3.15 UVCX_QP_STEPS_LAYERS
typedef __packed struct _UVCX_QPStepsLayers
{
    MMP_USHORT wLayerID ;
    MMP_UBYTE bFrameType ;
    MMP_UBYTE bMinQp;
    MMP_UBYTE bMaxQp ;
} UVCX_QPStepsLayers ;
#define UVCX_QPSTEP_LAYER_LEN sizeof(UVCX_QPStepsLayers) 


typedef enum {
    PIC_I_FRAME = 0,
    PIC_IDR_FRAME ,
    PIC_IDR_WITH_NEW_SPSPPS
} H264PicType ;

typedef __packed struct _UVCX_CropConfig
{
    MMP_USHORT wCropLeftOffset ;
    MMP_USHORT wCropRightOffset ;
    MMP_USHORT wCropTopOffset ;
    MMP_USHORT wCropBottomOffset ;
    
} UVCX_CropConfig;
#define UVCX_CROP_CONFIG_LEN sizeof(UVCX_CropConfig)


// 3.3.1 UVCX_VIDEO_CONFIG_PROBE & UVCX_VIDEO_CONFIG_COMMIT
// Appendix-B H264 Video Config
typedef __packed struct _UVCX_VIDEO_CONFIG {
    MMP_ULONG dwFrameInterval;
    MMP_ULONG dwBitRate;
    MMP_USHORT bmHints;
    MMP_USHORT wConfigurationIndex;
    MMP_USHORT wWidth;
    MMP_USHORT wHeight;
    MMP_USHORT wSliceUnits;
    MMP_USHORT wSliceMode;
    MMP_USHORT wProfile;
    // V1.0 : unit : milliseconds
    // < V1.0 : unit : 100 milliseconds
    MMP_USHORT wIFramePeriod; 
    MMP_USHORT wEstimatedVideoDelay;
    MMP_USHORT wEstimatedMaxConfigDelay;
    MMP_UBYTE bUsageType;
    MMP_UBYTE bRateControlMode;
    MMP_UBYTE bTemporalScaleMode;
    MMP_UBYTE bSpatialScaleMode;
    MMP_UBYTE bSNRScaleMode;
    MMP_UBYTE bStreamMuxOption;
    MMP_UBYTE bStreamFormat;
    MMP_UBYTE bEntropyCABAC;
    MMP_UBYTE bTimestamp;
    MMP_UBYTE bNumOfReorderFrames;
    MMP_UBYTE bPreviewFlipped;
    MMP_UBYTE bView;
    MMP_UBYTE bReserved1;
    MMP_UBYTE bReserved2;
    MMP_UBYTE bStreamID ;
    MMP_UBYTE bSpatialLayerRatio;
    MMP_USHORT wLeakyBucketSize;
} UVCX_VIDEO_CONFIG;

typedef struct _SLICE_BITS_MODE_CFG {
    MMP_UBYTE  minQP ;
    MMP_USHORT inaccuracyBytes ;
} SLICE_BITS_MODE_CFG ;

#if 1//H264_SIMULCAST_EN
typedef enum _ENCODE_BUF_MODE {
    REALTIME_MODE = 0,
    FRAME_MODE 
} ENCODE_BUF_MODE ;
#endif

#if H264_SIMULCAST_EN
#define MAX_STREAMING_NUM 		(3)
#define MAX_STREAMING_FRM_BUF_NUM 	(2)
#define STREAMING0_FRM_BUF_NUM 	(2)
#define STREAMING1_FRM_BUF_NUM 	(2)
#define STREAMING2_FRM_BUF_NUM 	(2)

typedef enum _MMPF_H264ENC_STREAMING {
    GRA_STREAMING0     = 0,
    GRA_STREAMING1,
    GRA_STREAMING2
} GRA_STREAMING_ID;

#define GRA_LOOP_STOP   (0)
#define GRA_LOOP_RUN    (1)

typedef struct _GRA_STREAMING_CFG {
	MMP_USHORT 	streaming_num;
	MMP_USHORT 	work_streaming;
	MMP_USHORT  frm_buf_format[MAX_STREAMING_NUM]; 
    MMP_USHORT  frm_buf_index[MAX_STREAMING_NUM];
	MMP_USHORT  frm_buf_w[MAX_STREAMING_NUM];
	MMP_USHORT  frm_buf_h[MAX_STREAMING_NUM];
	#if 0
	MMP_ULONG   frm_buf_addr_y[MAX_STREAMING_NUM][MAX_STREAMING_FRM_BUF_NUM];
	MMP_ULONG   frm_buf_addr_u[MAX_STREAMING_NUM][MAX_STREAMING_FRM_BUF_NUM];
	MMP_ULONG   frm_buf_addr_v[MAX_STREAMING_NUM][MAX_STREAMING_FRM_BUF_NUM];
    MMP_UBYTE   frm_enc_bind[MAX_STREAMING_NUM]; // binds enc id, 0xFF for none
    #endif
    MMP_UBYTE   gra_loop_state ;
    MMP_BOOL    gra_loop_stop ;
} GRA_STREAMING_CFG;

#endif

void UVCX_CommitH264Config(MMP_BOOL streamid,void *config_commit);
UVCX_VIDEO_CONFIG *UVCX_GetH264StartConfig(MMP_USHORT streamid);
MMP_USHORT UVCX_SetH264StartMode(UVCX_VIDEO_CONFIG *config);
// 3.3.3
MMP_USHORT UVCX_SetH264ControlMode(UVCX_RateControlMode *ctlMode) ;
// 3.3.4
MMP_USHORT UVCX_SetH264TemporalScaleMode(UVCX_TemporalScaleMode *tempScaleMode);
// 3.3.5 
MMP_USHORT UVCX_SetH264SpatialScaleMode(UVCX_SpatialScaleMode *spatialScaleMode);
// 3.3.6 
MMP_USHORT UVCX_SetH264SNRScaleMode(UVCX_SNRScaleMode *SNRScaleMode);
// 3.3.7 
MMP_USHORT UVCX_SetH264LTRBufferSize(UVCX_LTRBufferSizeControl *ltrBufSize);
// 3.3.8
MMP_USHORT UVCX_SetH264LTRPicture( UVCX_LTRPictureControl *ltrPict);
// 3.3.9
MMP_USHORT UVCX_SetH264PictureTypeCtl(UVCX_PictureTypeControl *picType);
// 3.3.11.1
MMP_USHORT UVCX_SetH264EncoderReset( MMP_USHORT layerId);
// 3.3.12
MMP_USHORT UVCX_SetH264FrameRate(UVCX_FrameRateConfig *frameRate);
// 3.3.13
MMP_USHORT UVCX_SetH264AdvConfig(UVCX_AdvConfig *advConfig);
// 3.3.14
MMP_USHORT UVCX_SetH264BitRate(UVCX_BitRateLayers *bRc);
// 3.3.15
MMP_USHORT UVCX_SetH264QPStepsLayers(UVCX_QPStepsLayers *qStep);


MMP_USHORT UVCX_SetH264CropConfig(UVCX_CropConfig *crop);
MMP_USHORT UVCX_SetH264FlipMode(MMP_USHORT flip_id);
void UVCX_EnableSliceBasedTx(MMP_BOOL en);
MMP_BOOL UVCX_IsSliceBasedTx(PIPE_PATH pipe);
MMP_BOOL UVCX_IsSliceBasedTxStream(STREAM_CFG *cur_stream);
MMP_USHORT UVCX_SetH264Resolution(MMP_USHORT stream0_res_indx,MMP_USHORT stream1_res_indx,MMP_USHORT resnum ) ;
MMP_BOOL   UVCX_IsSimulcastH264(void);
void UVCX_SetSimulcastH264(MMP_BOOL en) ;

MMP_USHORT UVCX_GetH264CurQP(MMP_USHORT wLayerId,MMP_UBYTE typeBitMap, MMP_UBYTE *I_QP,MMP_UBYTE *P_QP,MMP_UBYTE *B_QP);

MMP_USHORT UVCX_GetInaccurayBytesByMinQP(MMP_UBYTE minQP);

void UVCX_ConfigH264BufferBySliceMode(UVCX_VIDEO_CONFIG *config);
//
// New RC XU for CBR bitstuffing
//
MMP_USHORT UVCX_SetH264ControlModeFixCBR(UVCX_RateControlMode *ctlMode);
//
// For simulcast new control API
//
MMP_USHORT UVCX_SetH264EncodeBufferMode(ENCODE_BUF_MODE mode);
ENCODE_BUF_MODE UVCX_GetH264EncodeBufferMode(void);
MMP_USHORT UVCX_SwitchH264EncodeBufferMode(ENCODE_BUF_MODE mode);
MMP_USHORT UVCX_RestartPreview(MMP_UBYTE ep);

#endif

#endif



