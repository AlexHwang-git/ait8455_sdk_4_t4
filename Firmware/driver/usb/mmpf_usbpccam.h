#ifndef MMPF_USBPCCAM_H
#define MMPF_USBPCCAM_H

#include "pcam_api.h"
#include "mmpf_usb_h264.h"
#include "mmpf_ibc.h"
typedef enum _H264_FORMAT_TYPE {
    INVALID_H264 = 0,
    FRAMEBASE_H264 ,
    UVC_H264,
    SKYPE_H264,
    UVC_YUY2 ,// not h264, but YUY2, for dual stream YUY2+MJPEG
    // For AIT stream viewer tool
    FRAMEBASE_H264_YUY2,
    FRAMEBASE_H264_MJPEG,
    FRAMEBASE_H264_Y
    
} H264_FORMAT_TYPE ;

typedef enum _PIPE_PATH {
    PIPE_0 = 0,
	PIPE_1,
	PIPE_2,
	PIPE_MAX,
	PIPE_3 = 3, // virtual pipe for more h264 stream
	PIPE_4,
	PIPE_5,
	PIPE_6
} PIPE_PATH ;

#define PIPE0_EN    (1<<0)
#define PIPE1_EN    (1<<1)
#define PIPE2_EN    (1<<2)
#define PIPE_EN_MASK (0x7)
#define PIPE_EN01_MASK (0x03)

#define PIPEEN(x)   (1<<x)

/*
extend endpoint information.
*/
#define VIDEP_EN_MASK   (0xF0)

#define VIDEPEN(id )  ( (1 << (4+id) )&VIDEP_EN_MASK) 

#define PIPE_EN(x)  ( x & PIPE_EN_MASK )
#define PIPE_EN01(x)  ( x & PIPE_EN01_MASK )
#define IS_PIPE0_EN(x) ( PIPE_EN(x) == PIPE0_EN )
#define IS_PIPE1_EN(x) ( PIPE_EN(x) == PIPE1_EN )
#define IS_PIPE2_EN(x) ( PIPE_EN(x) == PIPE2_EN )
#define IS_PIPE01_EN(x) ( PIPE_EN01(x)==(PIPE0_EN|PIPE1_EN) )

#define PIPE_EN_SIMULCAST  (1 << 6 )
#define PIPE_EN_LOOP2GRA   (1 << 7 )       
// Add NV12, re-config pipe cfg
#define PIPE_CFG_YUY2   (0)//(1<<0)
#define PIPE_CFG_MJPEG  (1)//(1<<1)
#define PIPE_CFG_H264   (2)//(1<<2)
#define PIPE_CFG_NV12   (3)//(1<<3)
//#define PIPE_CFG_Y      (1<<3) // Same as NV12
// 2 PIPE support
//#define PIPE_CFG_MASK   (0x7)
//#define PIPE_CFG(pipe,cfg)  (  ( (cfg) & PIPE_CFG_MASK ) << ( pipe << 2 )  )
// 3 PIPE support
#define PIPE_CFG_MASK1   (0x3)
#define PIPE_CFG(pipe,cfg)  (  ( (cfg) & PIPE_CFG_MASK1 ) << ( pipe << 1 )  )
#define CUR_PIPE_CFG(pipe,pipe_cfg) ( (pipe_cfg >> (pipe << 1) ) & PIPE_CFG_MASK1 )

#define PIPE_NEXT_FRAME(pipe)   (1<<pipe)

#define PIPE_PH_TYPE_NA  (0)
#define PIPE_PH_TYPE_1  (1) // UVC 1.1 H264 payload header
#define PIPE_PH_TYPE_2  (2) // Skype payload header
#define PIPE_PH_TYPE_3  (3) // StreamViewer payload header ( internal protocol)

typedef struct _STREAM_CTL
{
    MMP_ULONG  rd_index,wr_index ;
    MMP_ULONG64  total_rd,total_wr ;
    MMP_ULONG  buf_addr,slot_size ;
    MMP_USHORT slot_num ;
#if 1//ALIGN_PAYLOAD
    MMP_USHORT off_header ;
#endif
    MMP_USHORT frame_num ; // for Slice based    
} STREAM_CTL ;

typedef struct _STREAM_CFG
{
    MMP_UBYTE pipe_en  ;
    MMP_UBYTE pipe_cfg ; 
    MMP_UBYTE pipe_ph_type[MMPF_IBC_PIPE_MAX] ;
    MMP_USHORT pipe_w[MMPF_IBC_PIPE_MAX];
    MMP_USHORT pipe_h[MMPF_IBC_PIPE_MAX];
    MMP_USHORT pipe_sync ;
    STREAM_CTL pipe_b[MMPF_IBC_PIPE_MAX];
} STREAM_CFG ;

typedef struct _RES_TYPE_CFG
{
    RES_TYPE_LIST res_type ;
    MMP_USHORT res_w ;
    MMP_USHORT res_h ;
    MMP_USHORT best_ratio;//best_m;    
    MMP_USHORT scaler_input_h ;
} RES_TYPE_CFG ;


#define DMA_CF_DUMMY        (0x01)
#define DMA_CF_EOF          (0x02) // for multi-slice based tx, send EOF at last slice in a frame
#define TX_MODE(tx_flag)    ((tx_flag >> 2) & 0x3 )
#define TX_MODE_BIT(mode)   ((mode & 0x3) << 2)
#define TX_BY_DMA           (0)
#define TX_BY_FIFO          (1)
#define TX_BY_DMA_INT       (2)

//#define TX_BY_FIFO          (0x04) // If set to 1, Data sending by FIFO copy
//#define TX_BY_EP2           (0x08) // If set to 1, Data sending by 2nd endpoint

#define TX_BY_EP2           (0x10) // If set to 1, Data sending by 2nd endpoint
typedef struct _STREAM_DMA_BLK
{
    MMP_USHORT max_dsize;
    MMP_USHORT header_len ;
    MMP_ULONG  blk_addr ;
    MMP_ULONG  blk_size ;
    MMP_ULONG  next_blk ;
    MMP_ULONG  cur_addr   ;
    MMP_ULONG  tx_len ,tx_packets    ;
    MMP_ULONG  dma_buf[2] ;
    MMP_ULONG  ctrl_flag;
} STREAM_DMA_BLK ;

#define SS_TX_MODE(tx_flag) (tx_flag&0x03)

#define SS_TX_BY_DMA                  (0x00)

#define SS_TX_SYNC_STREAMING        (0x0004)
#define SS_TX_NEXT_PACKET           (0x0008) 
#define SS_TX_STREAMING             (0x0010)
#define SS_TX_PRECLOSE_STREAMING    (0x0020)
#define SS_TX_CLOSE_STREAMING       (0x0040)
#define SS_TX_CONFIG_END            (0x0080)

#define SS_TX_PAUSE                 (0x0100)
#define SS_TX_LAST_PACKET           (0x0200)
#define SS_TX_EOS                   (0x0400)
#define SS_TX_COMMITED              (0x0800)
#define SS_TX_BY_ONE_SHOT           (0x1000)
#define SS_TX_1ST_PKT_DONE          (0x2000)
#define SS_TX_1ST_PKT_START         (0x4000)
#define SS_TX_RT_MODE               (0x8000)

typedef struct _STREAM_VSI
{
    MMP_UBYTE vsi_if;
    PCAM_USB_VIDEO_FORMAT vsi_fmtindx;
    PCAM_USB_VIDEO_RES   vsi_frmindx;
} STREAM_VSI ; 

typedef struct _STREAM_SESSION
{
    MMP_ULONG  streamaddr ;
    MMP_ULONG  frameaddr ;
    MMP_ULONG  framelength ;
    MMP_ULONG  packetcount ;
    MMP_ULONG  usbframecount;
    MMP_UBYTE  *cursendptr ;
    MMP_ULONG  cursendsize ;
    MMP_USHORT maxpacketsize;
    MMP_ULONG  maxdmadatasize;
    MMP_USHORT tx_flag ;
    MMP_UBYTE  frametoggle ;
    MMP_UBYTE  ep_id;   // start from 0
    MMP_UBYTE  pipe_id; // start from 0
    STREAM_CFG *stream_cfg;
    STREAM_VSI curvsi ;
    
    MMP_ULONG  STC ;
    MMP_USHORT SOFNUM ; // UVC1.5
    MMP_USHORT dma_timeout ;
    MMP_ULONG  frame_stime ;
    MMP_ULONG  frame_etime ;
    
    MMP_USHORT uvc15_layerid ;
} STREAM_SESSION ;

#define VIDEO_MJPEG_SID         (0x20) //MJPEG(sid=0x20)
#define VIDEO_GRAY_SID          (0x40) //GRAY(sid=0x40)
#define VIDEO_STILL_IMAGE_SID   (0x100)

typedef enum _JPEG_CAPTURE_MODE
{
    CONTI_PREVIEW_MODE = 0,
    STILL_CAPTURE_MODE  
} JPEG_CAPTURE_MODE;

extern MMP_UBYTE STREAM_EP_H264 ;
extern MMP_UBYTE STREAM_EP_YUY2 ;
extern MMP_UBYTE STREAM_EP_MJPEG ;
extern MMP_UBYTE STREAM_EP_NV12  ;

extern MMP_UBYTE gbChangeH264Res,gbChangeH264BufMode ,gbRestartPreview;
RES_TYPE_CFG *GetResCfg(MMP_UBYTE resolution);
PCAM_USB_VIDEO_RES GetResIndexBySize(MMP_USHORT w,MMP_USHORT h);
PCAM_USB_VIDEO_RES GetResIndexByEp(MMP_UBYTE ep_id);
PCAM_USB_VIDEO_FORMAT GetFmtIndexByEp(MMP_UBYTE ep_id) ;

//MMP_USHORT MMPF_PollingPCCam(void);
MMP_ULONG GetYUY2FrameSize(MMP_UBYTE resolution);
MMP_ULONG GetMJPEGFrameSize(MMP_UBYTE resolution);
MMP_ULONG GetYUV420FrameSize(MMP_UBYTE resolution);
//MMP_USHORT GetMaxFrameRate(MMP_UBYTE resolution) ;

//void MMPF_InitPCCam(void);
void MMPF_InitPCCam(STREAM_CFG *stream_cfg,MMP_ULONG res,MMP_BOOL skipsetscaler);
MMP_USHORT GetZoomResolution(PIPE_PATH pipe,MMP_USHORT w,MMP_USHORT h,MMP_USHORT user_ratio);

void InitScale(STREAM_CFG *stream_cfg,MMP_ULONG res,MMP_BOOL skipsetscaler);
void InitIBC(MMPF_IBC_PIPEID cur_pipe,MMP_UBYTE pipe_cfg,MMP_USHORT w,MMP_USHORT h);
STREAM_CFG *usb_get_cur_image_pipe(PIPE_PATH pipe_id);
STREAM_CFG *usb_get_cur_fdtc_pipe(void);
STREAM_CFG *usb_get_cur_image_pipe_by_epid(MMP_UBYTE ep_id);
MMP_UBYTE usb_get_cur_image_pipe_id(STREAM_CFG *cur_stream,MMP_UBYTE format);
MMP_UBYTE usb_get_cur_image_ep(PIPE_PATH pipe_id);
void usb_set_cur_image_pipe(MMP_UBYTE ep_id,STREAM_CFG *cur_pipe);
void usb_get_cur_image_size(MMP_UBYTE pipe,MMP_ULONG *pw, MMP_ULONG *ph);
STREAM_CFG *usb_get_cur_image_pipe_by_format(MMP_UBYTE ep_id,MMP_UBYTE format,PIPE_PATH *pipe) ;
MMP_UBYTE usb_get_cur_image_format_by_pipe(MMP_UBYTE ep_id,PIPE_PATH pipe_id);
MMP_USHORT MMPF_GetJfifTag(MMP_USHORT *pTagID, MMP_USHORT *pTagLength, MMP_UBYTE **ppImgAddr, MMP_ULONG length);
void MMPF_Video_SignalFrameDone(MMP_UBYTE eid,PIPE_PATH UvcPipePath);
void MMPF_Video_GetBufBound(PIPE_PATH pipe,MMP_ULONG *lowbound,MMP_ULONG *highbound);
void MMPF_Video_GetSlotInfo(MMP_BOOL ring_buf_mode,PIPE_PATH pipe,MMP_ULONG *addr,MMP_ULONG *slot_size,MMP_USHORT *slot_num);

MMP_BOOL MMPF_Video_IsEmpty(PIPE_PATH pipe);
MMP_ULONG MMPF_Video_GetEmptySlotNum(PIPE_PATH pipe);
MMP_ULONG MMPF_Video_GetEmptyFrameNum(PIPE_PATH pipe);
MMP_ULONG MMPF_Video_GetFreeBufSize( PIPE_PATH pipe) ;
MMP_BOOL MMPF_Video_IsFull(PIPE_PATH pipe);
void MMPF_Video_UpdateWrPtr(PIPE_PATH pipe);
void MMPF_Video_UpdateRdPtr(PIPE_PATH pipe);
MMP_UBYTE *MMPF_Video_CurRdPtr(PIPE_PATH pipe);
MMP_UBYTE *MMPF_Video_CurWrPtr(PIPE_PATH pipe);
void MMPF_Video_Init_Buffer(MMP_UBYTE ep_id,MMP_USHORT off_header);
STREAM_CTL *MMPF_Video_GetStreamCtlInfo(PIPE_PATH pipe);
void MMPF_Video_UpdateRdPtrByPayloadLength(MMP_UBYTE ep_id);
int MMPF_Video_DataCount(PIPE_PATH pipe);
MMP_UBYTE *MMPF_Video_NextRdPtr(PIPE_PATH pipe,MMP_USHORT next_n);
void MMPF_Video_InitDMABlk(MMP_USHORT ep_id,MMP_USHORT uvc_payload_size,MMP_ULONG dmabuf1,MMP_ULONG dmabuf2) ;
MMP_BOOL MMPF_Video_AddDMABlk(MMP_USHORT ep_id,MMP_ULONG header_len,MMP_ULONG blk_addr,MMP_ULONG blk_size,MMP_USHORT ctl_flag);
MMP_BOOL MMPF_Video_AddDMABlkH264SliceMode(MMP_USHORT ep_id,MMP_ULONG header_len,MMP_ULONG blk_addr,MMP_UBYTE sid, MMP_BOOL sof);
STREAM_DMA_BLK *MMPF_Video_CurBlk(MMP_USHORT ep_id);
MMP_BOOL MMPF_Video_NextBlk(MMP_USHORT ep_id,MMP_BOOL tx_by_fifo);
MMP_UBYTE *MMPF_Video_GetBlkAppHeader(STREAM_DMA_BLK *dma_blk);//sean@2011_01_22, change api name
void MMPF_USB_ReleaseDm(MMP_UBYTE ep_id);
void MMPF_Video_InitStreamSession(MMP_UBYTE ep_id,MMP_UBYTE tx_flag,MMP_USHORT max_packet_size);
void MMPF_Video_CloseStreamSession(MMP_UBYTE ep_id);
STREAM_SESSION *MMPF_Video_GetStreamSessionByEp(MMP_UBYTE ep_id);
void MMPF_Video_AllocStreamPipe(MMP_UBYTE ep_id,STREAM_CFG *stream_cfg );
int MMPF_Video_GetWorkingStreamSession(void);
MMP_UBYTE MMPF_Video_GetFreeStreamPipe(void);
//void MMPF_Video_SetStreamSessionVsi(MMP_UBYTE ep_id);
void MMPF_Video_SetStreamSessionVsi(MMP_UBYTE ep_id);
MMP_UBYTE MMPF_Video_GetInteralVsiFrameIndex(MMP_UBYTE frameindex);
MMP_BOOL MMPF_Video_IsSyncStreamMode(PIPE_PATH pipe);
MMP_UBYTE MMPF_Video_IsLoop2GRA(STREAM_CFG *stream_cfg) ;
MMP_BOOL MMPF_Video_SetLoop2GRA(STREAM_CFG *stream_cfg,MMP_BOOL en);
MMP_USHORT GetScalerRefInHeight(void);
MMP_USHORT GetScalerInHeight(MMP_USHORT w,MMP_USHORT h) ;
MMP_USHORT GetScalerUserRatio(MMP_USHORT w,MMP_USHORT h) ;
void SetScalerRefInOutHeight(PIPE_PATH pathsel,MMP_USHORT real_w,MMP_USHORT real_h) ;
void MMPF_Video_EnableJpeg(void);
void MMPF_Video_EnableJpegISR(void);
void MMPF_Video_SetJpegCaptureMode(JPEG_CAPTURE_MODE mode);
JPEG_CAPTURE_MODE MMPF_Video_GetJpegCaptureMode(void);
void MMPF_Video_SetJpegResolution(MMP_USHORT w,MMP_USHORT h);
void MMPF_Video_SetJpegCompressBuf(MMP_ULONG buf_addr, MMP_ULONG buf_size);
MMP_ULONG MMPF_Video_GetJpegCompressBuf(MMP_ULONG *buf_size) ;
MMP_BOOL MMPF_Video_WaitJpegEncDone(MMP_ULONG timeout_ms);
MMP_BOOL MMPF_Video_SetJpegEngine(PIPE_PATH jpeg_pipe,MMP_USHORT jpeg_w,MMP_USHORT jpeg_h);
#endif
