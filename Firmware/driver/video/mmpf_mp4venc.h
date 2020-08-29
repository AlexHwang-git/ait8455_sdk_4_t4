/// @ait_only
//==============================================================================
//
//  File        : mmpf_mp4venc.h
//  Description : Header function of video codec
//  Author      : Will Tseng
//  Revision    : 1.0
//
//==============================================================================

#ifndef _MMPF_MP4VENC_H_
#define _MMPF_MP4VENC_H_

#include "mmpf_3gpmgr.h"
#include "mmpf_ibc.h"

/** @addtogroup MMPF_VIDENC
 *  @{
 */

//==============================================================================
//
//                              COMPILER OPTION
//
//==============================================================================
#define TEST_SIMULCAST      0

#define MGR_DBG_MSG         (0)

#define MAX_NUM_CONT_B_FRAME    (2)

#define RC_JPEG_TARGET_SIZE (400 * 1024 )

#define RATE_CONTROL_EN		1
#define FPS_CTL             1

#define ENCODER_ID_MASK     (0x0F)
#define TEMPORAL_ID_MASK    (0x07)

#if (CHIP == VSN_V2) || (CHIP == VSN_V3)
#define QP_SUM_PATCH        (0)
#endif
#if (CHIP == P_V2)
#define QP_SUM_PATCH        (1)
#endif
/*
if STRICT_AVG_FRAME_SIZE = 1
a. Slice Based Tx will force switch back to Frame Based Tx
b. Because if the frame size is over target size + 10%, may needs to drop.
But some slices were transferred.

If Turn on (Case #2), minimum bitrate should over than 1.4MB(720p)
Otherwise, drop frames will be more easier even QP touched the high bound 
*/
#define STRICT_AVG_FRAME_SIZE       (0)
#if (STRICT_AVG_FRAME_SIZE == 1)
    #if 0 //I quality closer to P. ( Case #1 )
    #define TGT_FRAME_SIZE_I            (300) // Means I-frame size will be 3x size of P frame size,better I frame quality
    #define MAX_FRAME_SIZE_TH_I         (330) // If size is over target size + 10% , drop it.
    #else //Fix I size.             ( Case #2 )
    #define TGT_FRAME_SIZE_I            (100) // Means I-frame size is same as P-frame size
    #define MAX_FRAME_SIZE_TH_I         (110)
    #endif
    #define TGT_FRAME_SIZE_P            (100)
    #define MAX_FRAME_SIZE_TH_P         (110)
    #define MAX_FRAME_SIZE_BASE         (100)
    #define MIN_RC_QP                   (22)
    #define MAX_RC_QP                   (50)
    #define MIN_MB_QP                   (14)
    #define MAX_MB_QP                   (50)
    #define QP_DELTA_UP                 (18)
    #define QP_DELTA_LOW                (6)
#endif
//==============================================================================
//
//                              CONSTANTS
//
//==============================================================================
#if (VID_TIME_SYNC_ST == 1)
#define VID_TIME_TO_PTS(ms, ns)				(ms*10000+ns/100)	// 100ns/unit
#endif

#define MMPF_VIDENC_MAX_QUEUE_SIZE          (4)

#define MMPF_VIDENC_MODULE_H264             (0)
#define MMPF_VIDENC_MODULE_MP4V             (1)
#define MMPF_VIDENC_MODULE_MAX              (2)

// video format
#define MMPF_MP4VENC_FORMAT_OTHERS          0x00
#define	MMPF_MP4VENC_FORMAT_H263	        0x01
#define	MMPF_MP4VENC_FORMAT_MP4V	        0x02
#define MMPF_MP4VENC_FORMAT_H264            0x03
#define MMPF_MP4VENC_FORMAT_MJPEG           0x04
#define MMPF_MP4VENC_FORMAT_YUV422          0x05
#define MMPF_MP4VENC_FORMAT_YUV420          0x06

// video operation status
#define	MMPF_MP4VENC_FW_STATUS_RECORD       0x0000  ///< status of video encoder
#define	MMPF_MP4VENC_FW_STATUS_START	    0x0001  ///< status of START
#define	MMPF_MP4VENC_FW_STATUS_PAUSE	    0x0002  ///< status of PAUSE
#define	MMPF_MP4VENC_FW_STATUS_RESUME	    0x0003  ///< status of RESUME
#define	MMPF_MP4VENC_FW_STATUS_STOP	        0x0004  ///< status of STOP

// audio format (only used in recoder)
#define	MMPF_AUD_FORMAT_OTHERS		        0x00 
#define	MMPF_AUD_FORMAT_AMR			        0x01
#define	MMPF_AUD_FORMAT_MP4A		        0x02

// vide record speed control
#define MMPF_VIDENC_SPEED_NORMAL            0x00
#define MMPF_VIDENC_SPEED_SLOW              0x01
#define MMPF_VIDENC_SPEED_FAST              0x02
#define MMPF_VIDENC_SPEED_1X                0x00
#define MMPF_VIDENC_SPEED_2X                0x01
#define MMPF_VIDENC_SPEED_3X                0x02
#define MMPF_VIDENC_SPEED_4X                0x03
#define MMPF_VIDENC_SPEED_5X                0x04
#define MMPF_VIDENC_SPEED_MAX               0x05

#define I_FRAME                             (0)
#define P_FRAME                             (1)
#define B_FRAME                             (2)

#define MAX_NUM_ENC_SET                     (2)
#define MAX_NUM_TMP_LAYERS                  (3)
#define MAX_NUM_PARAM_CTL                   (16)

#if (STRICT_AVG_FRAME_SIZE == 1)
#define RC_MAX_WEIGHT_I                     (1050)              ///< 
#define RC_INIT_WEIGHT_I                    (1050)              ///< 
#else
#define RC_MAX_WEIGHT_I                     (3000)//(3500)              ///< 3000, 5000
#define RC_INIT_WEIGHT_I                    (2500)              ///< 2000, 4000
#endif
#define RC_MIN_VBV_FRM_NUM                  ((RC_MAX_WEIGHT_I+2000)/1000)
#define RC_PSEUDO_GOP_SIZE                  (1000)

#define FRAME_NUM_HIGH_BOUND                (0xFFFFFFFF)
#define FRAME_CODED_MAX_SIZE                (0x01000000)

#define FREE_SPACE_TO_RC_TH_ADJ             (1)
#define FREE_SPACE_TO_RC_TH_M_SHIFT         (1)
//==============================================================================
//
//                              STRUCTURES
//
//==============================================================================

typedef void VidEncEndCallBackFunc(void *);

typedef enum _MMPF_VIDENC_ATTRIBUTE {
    MMPF_VIDENC_ATTRIBUTE_PROFILE = 0,
    MMPF_VIDENC_ATTRIBUTE_LEVEL,
    MMPF_VIDENC_ATTRIBUTE_ENTROPY_MODE,
    MMPF_VIDENC_ATTRIBUTE_LAYERS,
    MMPF_VIDENC_ATTRIBUTE_RC_MODE,
    MMPF_VIDENC_ATTRIBUTE_RC_SKIPPABLE,
    MMPF_VIDENC_ATTRIBUTE_FRM_QP,
    MMPF_VIDENC_ATTRIBUTE_BR,
    MMPF_VIDENC_ATTRIBUTE_LB_SIZE,
    MMPF_VIDENC_ATTRIBUTE_PIC_MAX_WEIGHT,
    MMPF_VIDENC_ATTRIBUTE_CROPPING,
    MMPF_VIDENC_ATTRIBUTE_GOP_CTL,
    MMPF_VIDENC_ATTRIBUTE_FORCE_I,
    MMPF_VIDENC_ATTRIBUTE_VIDEO_FULL_RANGE,
    MMPF_VIDENC_ATTRIBUTE_MAX_FPS,
    MMPF_VIDENC_ATTRIBUTE_SLICE_CTL,
    MMPF_VIDENC_ATTRIBUTE_REFGENBUF_MODE,
    MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE,
    MMPF_VIDENC_ATTRIBUTE_SWITCH_CURBUF_MODE,
    MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE,
    MMPF_VIDENC_ATTRIBUTE_RINGBUF_EN,
    MMPF_VIDENC_ATTRIBUTE_POC_TYPE,
    MMPF_VIDENC_ATTRIBUTE_INPUT_COLORMODE,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_START,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_RESTART,
    MMPF_VIDENC_ATTRIBUTE_REG_CALLBACK_ENC_END,
    MMPF_VIDENC_ATTRIBUTE_RESOLUTION,
    MMPF_VIDENC_ATTRIBUTE_CBR_PADDING,
    MMPF_VIDENC_ATTRIBUTE_RESV_MAX_RESOLUTION,
    MMPF_VIDENC_ATTRIBUTE_PREFIX_NALU_EN
} MMPF_VIDENC_ATTRIBUTE;

typedef enum _MMPF_VIDENC_PROTECTION_MODE {
    MMPF_VIDENC_PROTECTION_TASK_MODE = 0,
    MMPF_VIDENC_PROTECTION_ISR_MODE
} MMPF_VIDENC_PROTECTION_MODE;

typedef enum _MMPF_VIDENC_COLORMODE {
    MMPF_VIDENC_COLORMODE_NV12 = 0,
    MMPF_VIDENC_COLORMODE_I420,
    MMPF_VIDENC_COLORMODE_MAX
} MMPF_VIDENC_COLORMODE;

typedef enum _MMPF_VIDENC_CURBUF_MODE {
    MMPF_VIDENC_CURBUF_FRAME = 0,
    MMPF_VIDENC_CURBUF_RT,
    MMPF_VIDENC_CURBUF_MAX
} MMPF_VIDENC_CURBUF_MODE;

typedef enum _MMPF_VIDENC_RTFCTL_MODE {
    MMPF_VIDENC_RTFCTL_PASSIVE = 0,
    MMPF_VIDENC_RTFCTL_ACTIVE,
    MMPF_VIDENC_RTFCTL_MAX
} MMPF_VIDENC_RTFCTL_MODE;

typedef enum _MMPF_VIDENC_BUFCFG {
    MMPF_VIDENC_BUFCFG_SRAM = 0,
    MMPF_VIDENC_BUFCFG_DRAM,
    MMPF_VIDENC_BUFCFG_INTLV,
    MMPF_VIDENC_BUFCFG_NONE
} MMPF_VIDENC_BUFCFG;

typedef enum _MMPF_VIDENC_REFGENBUF_MODE {
    MMPF_VIDENC_REFGENBUF_INDEPENDENT = 0,
    MMPF_VIDENC_REFGENBUF_OVERWRITE,
    MMPF_VIDENC_REFGENBUF_ROTATE,
    MMPF_VIDENC_REFGENBUF_NONE,
    MMPF_VIDENC_REFGENBUF_MAX
} MMPF_VIDENC_REFGENBUF_MODE;

typedef enum _MMPF_VIDENC_OUTPUT_SYNC_MODE {
    MMPF_VIDENC_OUTPUT_FRAME_SYNC = 0,
    MMPF_VIDENC_OUTPUT_SLICE_SYNC
} MMPF_VIDENC_OUTPUT_SYNC_MODE;

typedef enum _MMPF_VIDENC_RC_MODE {
    MMPF_VIDENC_RC_MODE_CBR = 0,
    MMPF_VIDENC_RC_MODE_VBR = 1,
    MMPF_VIDENC_RC_MODE_CQP = 2,
    MMPF_VIDENC_RC_MODE_MAX = 3
} MMPF_VIDENC_RC_MODE;

typedef enum _MMPF_VIDENC_SYNCFRAME_TYPE {
    MMPF_VIDENC_SYNCFRAME_IDR = 0,
    MMPF_VIDENC_SYNCFRAME_I,
    MMPF_VIDENC_SYNCFRAME_GDR,
    MMPF_VIDENC_SYNCFRAME_LT_IDR,
    MMPF_VIDENC_SYNCFRAME_LT_I,
    MMPF_VIDENC_SYNCFRAME_LT_P,
    MMPF_VIDENC_SYNCFRAME_MAX
} MMPF_VIDENC_SYNCFRAME_TYPE;

typedef enum _MMPF_VIDENC_PICCTL {
    MMPF_VIDENC_PICCTL_NONE,
    MMPF_VIDENC_PICCTL_IDR_RESYNC,
    MMPF_VIDENC_PICCTL_IDR,
    MMPF_VIDENC_PICCTL_I_RESYNC,
    MMPF_VIDENC_PICCTL_I,
    MMPF_VIDENC_PICCTL_MAX
} MMPF_VIDENC_PICCTL;

typedef enum _MMPF_VIDENC_BUF_TYPE {
    MMPF_VIDENC_BUF_TYPE_SINGLE,
    MMPF_VIDENC_BUF_TYPE_RING
} MMPF_VIDENC_BUF_TYPE;

typedef enum _MMPF_VIDENC_SLICE_MODE {
    MMPF_VIDENC_SLICE_MODE_FRM,
    MMPF_VIDENC_SLICE_MODE_MB,
    MMPF_VIDENC_SLICE_MODE_BYTE,
    MMPF_VIDENC_SLICE_MODE_ROW
} MMPF_VIDENC_SLICE_MODE;

typedef struct _MMPF_VIDENC_RESOLUTION {
	MMP_USHORT	usWidth;
	MMP_USHORT	usHeight;
} MMPF_VIDENC_RESOLUTION;

typedef struct _MMPF_VIDENC_CURBUF_MODE_CTL {
    MMPF_VIDENC_CURBUF_MODE InitCurBufMode;
    MMPF_VIDENC_BUFCFG      CurBufFrameConfig;
    MMPF_VIDENC_BUFCFG      CurBufRTConfig;
    MMP_UBYTE               ubRTSrcPipe;
} MMPF_VIDENC_CURBUF_MODE_CTL;

typedef struct _MMPF_VIDENC_RC_MODE_CTL {
    MMPF_VIDENC_RC_MODE RcMode;
    MMP_BOOL            bLayerGlobalRc;
} MMPF_VIDENC_RC_MODE_CTL;

typedef struct _MMPF_VIDENC_GOP_CTL {
    MMP_USHORT  usGopSize;
    MMP_USHORT  usMaxContBFrameNum;
    MMPF_VIDENC_SYNCFRAME_TYPE SyncFrameType;
} MMPF_VIDENC_GOP_CTL;

typedef struct _MMPF_VIDENC_CROPPING {
    MMP_USHORT  usTop;
    MMP_USHORT  usBottom;
    MMP_USHORT  usLeft;
    MMP_USHORT  usRight;
} MMPF_VIDENC_CROPPING;

typedef struct _MMPF_VIDENC_QUEUE {
    MMP_UBYTE   buffers[MMPF_VIDENC_MAX_QUEUE_SIZE];  ///< queue for buffer ready to encode, in display order
    MMP_UBYTE   head;
    MMP_UBYTE   size;
} MMPF_VIDENC_QUEUE;

typedef struct _MMPF_VIDENC_FRAME {
    MMP_ULONG   ulYAddr;
    MMP_ULONG   ulUAddr;
    MMP_ULONG   ulVAddr;
} MMPF_VIDENC_FRAME;

#define	VIDEO_INPUT_FB_MAX_CNT	4
typedef struct _MMPF_VIDENC_FRAME_LIST {
    MMP_ULONG           ulFrameCnt;
    MMPF_VIDENC_FRAME   FrameList[VIDEO_INPUT_FB_MAX_CNT];
} MMPF_VIDENC_FRAME_LIST;

typedef struct _MMPF_VIDENC_FRAMEBUF_BD {
    MMPF_VIDENC_FRAME   LowBound;
    MMPF_VIDENC_FRAME   HighBound;
} MMPF_VIDENC_FRAMEBUF_BD;

typedef struct _MMPF_VIDENC_BITRATE_CTL {   ///< bitrate param control
    MMP_UBYTE ubLayerBitMap;                ///< 0'b111 means all temporal layers
    MMP_ULONG ulBitrate[MAX_NUM_TMP_LAYERS];///< bitrate, bits
} MMPF_VIDENC_BITRATE_CTL;

typedef struct _MMPF_VIDENC_LEAKYBUCKET_CTL { ///< leacky bucket param control
    MMP_UBYTE ubLayerBitMap;                ///< 0'b111 means all temporal layers
    MMP_ULONG ulLeakyBucket[MAX_NUM_TMP_LAYERS];///< in ms
} MMPF_VIDENC_LEAKYBUCKET_CTL;

typedef struct _MMPF_VIDENC_QP_CTL {        ///< QP control, for initail QP and CQP
    MMP_UBYTE ubTID;                        ///< 0'b111 means all temporal layers
    MMP_UBYTE ubTypeBitMap;                 ///< 0: I, 1: P, 2: B
    MMP_UBYTE ubQP[3];
    MMP_LONG  CbrQpIdxOffset[3];            ///< Chroma QP index offset
    MMP_LONG  CrQpIdxOffset[3];             ///< 2nd chroma QP index offset
} MMPF_VIDENC_QP_CTL;

typedef struct _MMPF_VIDENC_PIC_WEIGHT_CTL {///< RC picture weighting control
    MMP_UBYTE ubLayerID;                    ///< layer id
    MMP_UBYTE ubTypeBitMap;                 ///< 0: I, 1: P, 2: B
    MMP_ULONG ulIWeight;                    ///< P weight awalys 1000
    MMP_ULONG ulBWeight;
} MMPF_VIDENC_PIC_WEIGHT_CTL;

typedef struct _MMPF_VIDNC_MAX_FPS_CTL {
    MMP_ULONG   ulMaxFpsResolution;
    MMP_ULONG   ulMaxFpsIncreament;
} MMPF_VIDENC_MAX_FPS_CTL;

typedef struct _MMPF_VIDENC_SLICE_CTL {
    MMPF_VIDENC_SLICE_MODE  SliceMode;
    MMP_ULONG               ulSliceSize;
} MMPF_VIDENC_SLICE_CTL;

typedef struct _MMPF_VIDENC_PARAM_CTL {
    MMPF_VIDENC_ATTRIBUTE Attrib;
    void (*CallBack)(MMP_ERR);
    union {
        MMPF_VIDENC_RC_MODE_CTL     RcMode;
        MMPF_VIDENC_BITRATE_CTL     Bitrate;
        MMPF_VIDENC_LEAKYBUCKET_CTL CpbSize;
        MMPF_VIDENC_QP_CTL          Qp;
        MMPF_VIDENC_GOP_CTL         Gop;
    } Ctl;
} MMPF_VIDENC_PARAM_CTL;

#if (RATE_CONTROL_EN == 1)

#define WINDOW_SIZE                         (20)
#define MAX_SUPPORT_LAYER                   (MAX_NUM_TMP_LAYERS)
#define MAX_SUPPORT_ENC_NUM                 (MAX_NUM_ENC_SET)
#define PEAK_WINDOW_SIZE                    (30)
#define SUPPORT_PEAK_BR_CONTROL             (0)

typedef struct {
    MMP_ULONG   LayerBitRate[MAX_SUPPORT_LAYER];
    MMP_ULONG   BitRate[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBV_LayerSize[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBV_size[MAX_SUPPORT_LAYER];
    MMP_LONG    VBV_fullness[MAX_SUPPORT_LAYER];
    MMP_ULONG   TargetVBVLevel[MAX_SUPPORT_LAYER];
    MMP_ULONG   TargetVBV[MAX_SUPPORT_LAYER];
    MMP_ULONG   VBVRatio[MAX_SUPPORT_LAYER];
} VBV_PARAM;

typedef struct {
    MMP_LONG    bitrate;
    MMP_LONG    intra_period;
    MMP_LONG    VBV_fullness;
    MMP_LONG    VBV_size;
    MMP_LONG    target_framesize;

    MMP_LONG    bits_budget;
    MMP_LONG    total_frames;
    MMP_LONG    left_frames[3];
    MMP_LONG    Iframe_num;
    MMP_LONG    Pframe_num;
    MMP_LONG    total_framesize[3];
    //MMP_LONG    total_I_size;
    //MMP_LONG    total_P_size;
    //MMP_LONG    total_B_size;
    //MMP_LONG    last_qp;
    //MMP_LONG    lastXP;

    MMP_LONG    last_X[3];
    MMP_LONG    last_X2[3];
    unsigned long long    X[3];
    MMP_LONG    X_Count[3];
    unsigned long long    X2[3][WINDOW_SIZE];
    MMP_LONG    X_Idx[3];
    MMP_LONG    count[3];
    MMP_LONG    frame_count;
    MMP_LONG    target_P;
    MMP_LONG    last_bits;
    //MMP_LONG    last_IQP;
    //MMP_LONG    last_Bqp;
    MMP_ULONG   lastQP[3];
    //MMP_LONG    clip_qp;

    MMP_LONG    prev_window_size[3];
    MMP_LONG    window_size[3];

    //MMP_LONG    AlphaI;
    //MMP_LONG    AlphaB;
    MMP_LONG    Alpha[3];
    //MMP_LONG    avg_xp;
    MMP_LONG    avg_qp[3];

    //MMP_LONG    is_vbr_mode;
    MMP_LONG    rc_mode;
    MMP_LONG    enable_vbr_mode;
    MMP_LONG    GOP_frame_count;
    MMP_LONG    GOP_count;
    MMP_LONG    GOP_totalbits;
    MMP_LONG    QP_sum;
    MMP_LONG    GOP_QP[3];
    MMP_LONG    GOP_left_frames[3];

    MMP_LONG    GOP_num_per_I_period;
    MMP_LONG    GOP_count_inside_I_period;
    
    MMP_LONG    last_headerbits[3];
    MMP_ULONG   nP;
    MMP_ULONG   nB;
    
    MMP_ULONG   header_bits[3][WINDOW_SIZE];
    MMP_ULONG   header_index[3];
    MMP_ULONG   header_count[3];
    MMP_ULONG   avg_header[3];
    
    MMP_ULONG   avgXP[3];
    //test
    MMP_LONG    budget;
    MMP_ULONG   frametype_count[3];
    MMP_ULONG   targetPSize;
    
    MMP_LONG    vbr_budget;
    MMP_LONG    vbr_total_frames;
	MMP_ULONG	framerate;
	MMP_BOOL	SkipFrame;
	
	MMP_LONG	GOPLeftFrames;
	MMP_BOOL	bResetRC;
	MMP_ULONG   light_condition;
	
	//MMP_LONG	TargetLowerBound;
	//MMP_LONG	TargetUpperBound;
	
	MMP_ULONG   MaxQPDelta[3];
	MMP_ULONG	MaxWeight[3];		//1.5 * 1000
	MMP_ULONG	MinWeight[3];		//1.0 * 1000
	MMP_ULONG	VBV_Delay;			//500 ms
	MMP_ULONG	TargetVBVLevel;		//250 ms
	MMP_ULONG   FrameCount;
	MMP_BOOL    SkipPrevFrame;
	MMP_ULONG   SkipFrameThreshold;
	
	MMP_ULONG   m_LowerQP[3];
    MMP_ULONG   m_UpperQP[3];
    MMP_ULONG   m_VideoFormat;
    MMP_ULONG   m_ResetCount;
    MMP_ULONG   m_GOP;
    MMP_ULONG   m_Budget;
    MMP_ULONG64 m_AvgHeaderSize;
    MMP_ULONG   m_lastQP;
    MMP_UBYTE   m_ubFormatIdx;
    MMP_ULONG   MBWidth;
	MMP_ULONG   MBHeight;
	MMP_ULONG   TargetVBV;
	MMP_BOOL    bPreSkipFrame;
	
	MMP_ULONG	VBVRatio;
	MMP_ULONG	bUseInitQP;
	
	MMP_ULONG   m_LastType;
	
	#if SUPPORT_PEAK_BR_CONTROL
	MMP_ULONG   ulPeakByteCnt;
	MMP_ULONG   ulWatchFrame;
	MMP_ULONG   ulPeakBufFrameCnt;
	MMP_ULONG   ulFrameSize[PEAK_WINDOW_SIZE];
	MMP_UBYTE   ubPeakWIdx;
	MMP_ULONG   ulTotalByte;
	#endif
	
	//++Will RC
	void*       pGlobalVBV;
	MMP_ULONG   LayerRelated;
	MMP_ULONG   Layer;
	//--Will RC
} RC;

typedef struct {
	MMP_ULONG	MaxIWeight;			//1.5 * 1000
	MMP_ULONG	MinIWeight;			//1.0 * 1000
	MMP_ULONG	MaxBWeight;			//1.0 * 1000
	MMP_ULONG	MinBWeight;			//0.5 * 1000
	MMP_ULONG	VBV_Delay;			//500 ms
	MMP_ULONG	TargetVBVLevel;		//250 ms
	MMP_ULONG	InitWeight[3];
	MMP_ULONG	MaxQPDelta[3];
	MMP_ULONG   SkipFrameThreshold;
	MMP_ULONG   MBWidth;
	MMP_ULONG   MBHeight;
	MMP_ULONG   InitQP[3];
	MMP_ULONG   rc_mode;
	MMP_ULONG   bPreSkipFrame;
	
	#if SUPPORT_PEAK_BR_CONTROL
	MMP_ULONG   ulPeakBR;
	#endif
	//++Will RC
	MMP_ULONG   LayerRelated;
	MMP_ULONG   Layer;
	MMP_ULONG   EncID;
	//--Will RC
} RC_CONFIG_PARAM;
#endif

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================



//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

#define MAX(a,b)                                (((a) > (b)) ? (a) : (b))
#define MIN(a,b)                                (((a) < (b)) ? (a) : (b))
#define CLIP(a,i,s)                             (((a) > (s)) ? (s) : MAX(a,i))

void MMPF_VIDENC_SoftStop(void);
MMP_ERR MMPF_VIDENC_CheckWorkingBufSet (void *EncHandle);
MMP_ERR MMPF_VIDENC_UpdateWorkingBufWrIdx (void *EncHandle);
MMP_ERR MMPF_VIDENC_UpdateWorkingBufRdIdx (void *EncHandle);
MMPF_VIDENC_OUTPUT_SYNC_MODE MMPF_VIDENC_GetOutputSyncMode (void);
MMP_ERR MMPF_VIDENC_SetOutputSyncMode (MMPF_VIDENC_OUTPUT_SYNC_MODE SyncMode);
MMP_ERR MMPF_VIDENC_InitCodedDataDesc(MMPF_VIDMGR_CODED_DESC *pDesc,
                                        MMP_UBYTE   (*XhdrBuf)[MAX_XHDR_SIZE],
                                        MMP_ULONG   *XhdrSize,
                                        MMP_ULONG   *DataBuf,
                                        MMP_ULONG   *DataSize,
                                        void        *EncHandle);
MMPF_3GPMGR_FRAME_TYPE MMPF_VIDENC_GetFrameType (void *EncHandle, MMP_ULONG ulEncFrameNum);
MMP_ERR MMPF_VIDENC_SetFrameReady(MMP_UBYTE ubEncID);
void MMPF_VIDENC_UpdateOperation (void *EncHandle);
MMP_ERR MMPF_VIDENC_SetParameter (MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *pInfo);
MMP_ERR MMPF_VIDENC_GetParameter(MMP_UBYTE ubEncId, MMPF_VIDENC_ATTRIBUTE attrib, void *ulValue);
MMP_ERR MMPF_VIDENC_InitRCConfig (void *pEncHandle);
MMP_ERR MMPF_VIDENC_SetQPBound (MMP_UBYTE ubEncId, MMP_ULONG ulLayer,
                                MMPF_3GPMGR_FRAME_TYPE type,
                                MMP_LONG lMinQp, MMP_LONG lMaxQp);
MMP_ERR MMPF_VIDENC_GetQPBound (MMP_UBYTE ubEncId, MMP_ULONG ulLayer,
                                MMPF_3GPMGR_FRAME_TYPE type,
                                MMP_LONG *lMinQp, MMP_LONG *lMaxQp);
MMP_ERR     MMPF_VIDENC_SetTotalPipeNum(MMP_UBYTE ubPipeNum);
MMP_UBYTE   MMPF_VIDENC_GetTotalPipeNum(void);
MMP_ERR MMPF_VIDENC_TriggerFrameDone(MMP_UBYTE ubEncID, MMP_UBYTE *pbCurBuf, MMP_UBYTE *pbIBCBuf, MMP_UBYTE ubBufCount, 
                                    MMP_ULONG ulCurYBufAddr[], MMP_ULONG ulCurUBufAddr[], MMP_ULONG ulCurVBufAddr[]);
MMP_ERR MMPF_VIDENC_SetEncodeDisable(void);

// functions of video encoder
void MMPF_VIDENC_SetStatus(MMP_USHORT status);
MMP_USHORT MMPF_VIDENC_GetStatus(void);
MMP_ERR MMPF_VIDENC_Stop(void);
MMP_ERR MMPF_VIDENC_Pause(void);

// functions of H264 encoder
MMP_ERR MMPF_H264ENC_Start(void);
MMP_ERR MMPF_H264ENC_Close (void);
MMP_ERR MMPF_H264ENC_Resume(void);
MMP_ERR MMPF_H264ENC_TriggerFrameDone(MMP_UBYTE ubEncID, MMPF_VIDENC_PROTECTION_MODE ProtectionMode);

#if (RATE_CONTROL_EN == 1)
MMP_ERR MMPF_VIDENC_UpdateRCQuality(MMP_UBYTE ubEncId, MMP_UBYTE ubLayer, MMPF_3GPMGR_FRAME_TYPE type, MMP_UBYTE ubQuality);
MMP_LONG MMPF_VidRateCtl_Get_VOP_QP(void* RCHandle,MMP_LONG vop_type,MMP_ULONG *target_size, MMP_ULONG *qp_delta, MMP_BOOL *bSkipFrame, MMP_ULONG ulMaxFrameSize);
MMP_ERR MMPF_VidRateCtl_ForceQP(void* RCHandle,MMP_LONG vop_type, MMP_ULONG QP);
MMP_ULONG MMPF_VidRateCtl_UpdateModel(void* handle,MMP_LONG vop_type, MMP_ULONG CurSize,MMP_ULONG HeaderSize,MMP_ULONG last_QP, MMP_BOOL bForceSkip, MMP_BOOL *bSkipFrame,MMP_ULONG *pending_bytes);
void MMPF_VidRateCtl_Init(void* *handle,MMP_ULONG idx,MMP_USHORT gsVidRecdFormat, MMP_LONG targetsize, MMP_LONG framerate, MMP_ULONG nP, MMP_ULONG nB, /*MMP_ULONG InitQP,*/ MMP_BOOL PreventBufOverflow, /*MMP_LONG is_vbr_mode,*/ RC_CONFIG_PARAM 	RcConfig);
void MMPF_VidRateCtl_ResetBitrate(void* handle,MMP_LONG bit_rate, MMP_ULONG framerate,MMP_BOOL ResetParams, MMP_ULONG ulVBVSize, MMP_BOOL bResetBufUsage);
void MMPF_VidRateCtl_SetQPBoundary(void* handle,MMP_ULONG frame_type,MMP_LONG QP_LowerBound,MMP_LONG QP_UpperBound);
void MMPF_VidRateCtl_GetQPBoundary(void* handle,MMP_ULONG frame_type,MMP_LONG *QP_LowerBound,MMP_LONG *QP_UpperBound);
void MMPF_VidRateCtl_ResetBufSize(void* RCHandle, MMP_LONG BufSize);
#endif
void MMPF_VIDENC_RegisterStartInstanceCallBack(MMP_BOOL enable_cb) ;

#endif	// _MMPF_MP4VENC_H_
/// @}
/// @end_ait_only