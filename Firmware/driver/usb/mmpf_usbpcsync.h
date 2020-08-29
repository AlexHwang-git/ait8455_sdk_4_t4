#ifndef USBPCSYNC_H
#define USBPCSYNC_H

#include "mmpf_typedef.h"
#include "Config_fw.h"
#include "mmpf_usbuvc15h264.h"

#define PCSYNC_CONFIG               0x81
#define PCSYNC_START                0x82
#define PCSYNC_WRITE_FRAME          0x83
#define PCSYNC_STOP                 0x84
#define PCSYNC_READ_FRAME           0x85

#define CONFIG_UVC15_H264           0x01

/*
//Requests 
#define PCSYNC_CTRL_BULK_IN				0x81
#define PCSYNC_CTRL_BULK_OUT			0x82
#define PCSYNC_VENDER_CTRL_IN_TEST 		0x83
#define PCSYNC_VENDER_CTRL_OUT_TEST 	0x84
#define PCSYNC_VENDER_DEBUG_IN_ENABLE   0x85
#define PCSYNC_VENDER_DEBUG_IN_DISABLE  0x86
#define PCSYNC_GET_FD_LIST				0x01

#define PCSYNC_SET_FD_PARAM				0x02
#define PCSYNC_BURN_FIRMWARE			0x03
#define PCSYNC_PARTIAL_FLASH_RW			0x04
#define PCSYNC_BURN_CXNT_FIRMWARE		0x05
#define PCSYNC_BURN_CXNT_ROMFIRMWARE	0x06
*/
#define PCSYNC_DIRECTION_BULK_IN	0x00
#define PCSYNC_DIRECTION_BULK_OUT	0x01

//pcsync vender command
//Requests
#define MMP_REG_READ_B       0x00
#define MMP_REG_WRITE_B      0x01
#define MMP_REG_READ_W       0x02
#define MMP_REG_WRITE_W      0x03
#define ISP_REG_READ_B       0x04
#define ISP_REG_WRITE_B      0x05
#define ISP_REG_READ_W       0x06
#define ISP_REG_WRITE_W      0x07
#define ISP_REG_DUMP_GROUP   0x08
#define ISP_REG_WRITE_GROUP	 0x09
#define UPGRADE_FW			 0x11
#define SET_I2C_SLAVE_ADDR   0x19
#define CCIR_PREVIEW_START   0x1A
#define CCIR_PREVIEW_STOP    0x1B
#define READ_BUF_FROM_ISP	 0x1C
#define WRITE_BUF_TO_ISP	 0x1D
#define SET_MMP_SENSOR_MCLK	 0x1E
#define SET_I2C_SPEED		 0x1F
#define SET_RAW_STORE_RANGE  0x27
#define BULK_SETTING_PCREAD  0x52
#define BULK_SETTING_PCWRITE 0x54
#define GET_FW_VER         	0x58
#define GET_FW_TIME         0x59
#define GET_FW_DATE         0x5A
#define SET_MMP_USB_RW_MEM_ADDR	0x60


#define PCSYNC_EP_FIFO_ADDR_W USB_EP2_FIFO_W

typedef void BulkEndCB(void);

typedef struct PCSyncBulkEp
{
    MMP_UBYTE Id;
	MMP_UBYTE Ep;
	MMP_UBYTE Direction;
	MMP_ULONG ReqBytes;
	MMP_UBYTE *BufAddr;
	BulkEndCB *CallBack;
} PCSyncBulkEp;

typedef  struct
{
	MMP_ULONG	wWidth;
	MMP_ULONG	wHeight;
} VID_RES;

typedef  struct
{
	MMP_ULONG	wProfile;
	MMP_ULONG	wConstrainedToolset;
	MMP_ULONG	bmSetting;
} VID_PROF_TOOLSET;

typedef  struct
{
	MMP_ULONG	wSliceMode;
	MMP_ULONG	wSliceConfigSetting;
} VID_SLICEMODE;

typedef  struct
{
	MMP_ULONG	wQpPrime_I;
	MMP_ULONG	wQpPrime_P;
	MMP_ULONG	wQpPrime_B;
} VID_QP;

typedef  struct
{
	MMP_ULONG	bSyncFrameType;
	MMP_ULONG	wSyncFrameInterval;
	MMP_ULONG	bGradualDecoderRefresh;
} VID_SYNC_FRAME;

typedef  struct
{
    MMP_ULONG minQP ;
    MMP_ULONG maxQP ;
} VID_QP_RANGE ;


typedef   struct _PCSYNC_H264_CONFIG
{
    // The Encoding Unit control sets
    MMP_ULONG       wLayerOrViewID ;
    VID_RES         res ;
    VID_PROF_TOOLSET toolSet;
    MMP_ULONG       dwFrameInterval ;
    VID_SLICEMODE    sliceMode ;
    MMP_ULONG       bRateControlMode;
    MMP_ULONG       dwAverageBitRate[MAX_NUM_TMP_LAYERS]; // can controlled by layer
    //MMP_ULONG       dwCPBsize[MAX_NUM_TMP_LAYERS] ;
    MMP_ULONG       dwLeakyBucket[MAX_NUM_TMP_LAYERS] ;
    MMP_ULONG       dwPeakBitRate ;
    VID_QP          QPCtrl[MAX_NUM_TMP_LAYERS];
    VID_SYNC_FRAME  syncRefFrameCtrl;
    MMP_ULONG       bPriority[MAX_NUM_TMP_LAYERS] ;
    VID_QP_RANGE    qpRange[MAX_NUM_TMP_LAYERS];
    MMP_ULONG       bLevelIDC ;
    
    //
    // additional information
    //
    MMP_ULONG       bSvcLayers ;

} PCSYNC_H264_CONFIG ;

typedef __packed struct  _PCSYNC_VIDEO_FRAME
{
        MMP_USHORT wVersion ;
        MMP_USHORT wHeaderLen;
        MMP_USHORT wStreamType ; // 41,42,43,44
        MMP_USHORT wImageWidth ;
        MMP_USHORT wImageHeight;
        MMP_USHORT wFrameRate  ;
        /*------Vendor info------*/
        MMP_ULONG  dwFrameSeq ;
        MMP_ULONG  dwTimeStamp ;
        MMP_ULONG  dwFlag ;
        MMP_ULONG  dwReserved ; 
        /*------Vendor Info-------*/
        MMP_ULONG  dwPayloadSize;
} PCSYNC_VIDEO_FRAME;

typedef __packed struct _PCSYNC_VIDEO_STATUS
{
    MMP_ULONG error;
    MMP_ULONG length;
} PCSYNC_VIDEO_STATUS ;

extern PCSyncBulkEp gPCSyncBulkEp;
void pcsync_init_bulk_transfer(PCSyncBulkEp *pcSyncEp,
								  MMP_UBYTE ep,
								  MMP_UBYTE *buf,
								  MMP_LONG length,
								  MMP_UBYTE direction,
								  BulkEndCB *cb);

void pcsync_bulk_ep_interrupt(PCSyncBulkEp *pcSyncEp);
void pcsync_set_ep_direction(MMP_UBYTE ep,MMP_UBYTE direction);
MMP_ULONG pcsync_ep_bulk_rw(MMP_UBYTE *srcBuf,MMP_ULONG length,MMP_UBYTE BulkIn,MMP_UBYTE Ep);
MMP_ULONG pcsync_usb_fifo_push(MMP_BYTE Ep,MMP_UBYTE *buf,MMP_USHORT length);


void pcsync_vendor_request_setup(void);
void pcsync_vendor_request_data_out(void);
void pcsync_vendor_request_data_in(void);
void pcsync_ep0_rx_init(MMP_USHORT rx_len);
MMP_BOOL pcsync_ep0_rx(MMP_UBYTE *ptr);
void pcsync_start_preview(void);
void pcsync_stop_preview(void);
MMP_BOOL pcsync_is_preview(void);
MMP_UBYTE *pcsync_get_cur_ibc_buffer(MMP_UBYTE sId);
void pcsync_encode_frame(MMP_UBYTE sId);
MMP_ULONG pcsync_get_frame(MMP_ULONG *frame_size,MMP_USHORT *sId);
void pcsync_dump_h264_config(MMP_USHORT streamid,PCSYNC_H264_CONFIG *config_in);
void pcsync_dump_frame_desc(PCSYNC_VIDEO_FRAME *desc);
MMP_BOOL pcsync_h264_to_uvc15_h264(UVC15_VIDEO_CONFIG *uvc15_h264,PCSYNC_H264_CONFIG *pcsync_h264) ;
#endif
