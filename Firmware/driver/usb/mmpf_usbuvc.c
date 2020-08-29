#define exUSBMSDC
#include "includes_fw.h"
#include "lib_retina.h"
#include "config_fw.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbuvc_hw.h"
#include "mmpf_usbcolor.h"
#include "mmpf_usbpccam.h"
#include "mmp_err.h"
#include "mmp_reg_jpeg.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_gbl.h"

#include "mmp_reg_dma.h"
#include "mmpf_dma.h"

//#include "sensor_ctl.h"
//#include "3A_ctl.h"
#include "mmps_3gprecd.h"
#include "mmpd_sensor.h"
#include "mmpf_system.h"
//#include "preview_ctl.h"
#include "text_ctl.h"
#include "isp_if.h"
#include "hdm_ctl.h"
#include "mmpf_sensor.h"
#include "mmpf_3gpmgr.h"
#include "mmpf_dma.h"
#include "mmp_reg_h264enc.h"
#include "mmpf_mp4venc.h"
#include "mmpf_h264enc.h"
#include "mmpf_scaler.h"
#include "mmp_reg_usb.h"

#if (SUPPORT_UVC_FUNC==1)
#include "pcam_usb.h"
#include "pcam_msg.h"
#if USB_UVC_H264==1
#include "mmpf_usb_h264.h"
#include "mmpf_usbuvch264.h"
#endif
#if USB_UVC_SKYPE==1
#include "mmpf_usbskypeh264.h"
#endif
#include "mmpf_pwm.h"
#include "mmpf_boot.h"
#if ENCODING_UNIT==1
#include "mmpf_usbuvc15h264.h"
#endif

#if (SUPPORT_LED_DRIVER)
#include "Led_ctl.h"
#endif

#include "mmpf_sensor.h"
extern MMPF_SENSOR_FUNCTION *gsSensorFunction;

void write_cpu_exception(MMP_ULONG lr,MMP_ULONG sp);

void write_boot_count(MMP_ULONG boot_ok_count,MMP_ULONG enum_ok_count);
void cali_time_loop(void);
void power_on_off_test(int count);
void itcm_write_test(void) ;

void usb_vc_cfg_video_probe(MMP_USHORT indx,MMP_ULONG max_frame_size,MMP_ULONG max_payload_size,MMP_ULONG frameInterval) ;
void usb_vc_cmd_cfg(MMP_UBYTE req,VC_CMD_CFG *cfg,MMP_ULONG cur_val);
void usb_vc_dump_vsi_info(MMP_UBYTE if_id);


extern MMP_USHORT USBEP0TxCnt;
extern MMP_UBYTE  USBOutData;
extern MMP_USHORT  UVCTxFIFOSize[] ;// EP2TxFIFOSize;
#if USING_EXT_USB_DESC==1
MMP_USHORT gsCurUVCVersion = bcdVCD15;
#else
MMP_USHORT gsCurUVCVersion = bcdVCD10;
#endif
MMP_UBYTE gbVCITID = 0x01;
MMP_UBYTE gbVCOTID = 0x04;
MMP_UBYTE gbVCPUID = 0x03;

#if USB_UVC_SKYPE // ID1->ID3->ID5->ID6->ID2->ID4                                   
MMP_UBYTE gbVCEU0ID = 0x05;
#else            //  ID1->ID3->ID6->ID2->ID4
#endif

MMP_UBYTE gbVCEU2ID = 0x02; // for amb cmd
MMP_UBYTE gbVCEU1ID = 0x06; // for ait tool

#if SKYPE_UVC_H264
MMP_UBYTE gbVCEU3ID = 0x08; // for USB-IF UVC H.264 payload
#endif

MMP_UBYTE gbVCVCIF = 0x00;// same with descriptor
MMP_UBYTE gbVCVSIF1 = 0x01;// same with descriptor
MMP_UBYTE gbVCVSIF2 = 0x02;// same with descriptor

UVC_VSI_INFO    gCurVsiInfo[2];

MMP_UBYTE  gbVCERRCode;
//MMP_UBYTE  gbCurFormatIndex = 1, gbCurFrameIndex = 1;
MMP_UBYTE  gbCurStillFormatIndex = 1, gbCurStillFrameIndex = 1;//,framemode
//MMP_UBYTE  gbCurFrameRate[2] = {30,30}; 
MMP_USHORT gsCurFrameRatex10[] = { 30*10,30*10 };
MMP_UBYTE  gbEnableH264Mode = 0;
MMP_BOOL    gbAutoFocus;
MMP_UBYTE   gbBandFilter;
MMP_UBYTE   gbTemperInfo;
MMP_UBYTE   gbEVInfo;
MMP_UBYTE   gbFocusInfo ;

MMP_UBYTE   gbAWB = AWB_DEF;
MMP_UBYTE   gblastAWB;
MMP_UBYTE   gbAE,gbAEPriority;
MMP_UBYTE   gblastAE;
MMP_UBYTE   gblastAF;
MMP_UBYTE   gbUpdAE;
MMP_UBYTE   gbUpdAWB;
MMP_UBYTE   gbUpdAF ;
MMP_UBYTE   gbTriOP;

MMP_USHORT  gsAFocus;
MMP_USHORT  gsZoom;
MMP_USHORT  gsBacklight;
MMP_USHORT  gsTemper;
MMP_USHORT  gsSaturation;
MMP_USHORT  gsContrast;
MMP_USHORT  gsBrightness;
MMP_SHORT   gsHue;
MMP_SHORT   gsGamma;
MMP_SHORT   gsSharpness;
MMP_SHORT   gsGain ;

MMP_ULONG   glEV; // 100 us unit.

MMP_LONG    glPan,glTilt  ;
//MMP_ULONG   glExposureTime ;
// New in UVC1.5
MMP_UBYTE   gbSimpleFocus ;


VIDEO_PROBE       vp = 
{
    0x0001, // bmHint
    (YUY2_FORMAT_INDEX << 8) | (0x0001), // wIndex 
    0x1615, // wIntervalLo
    0x0005, // wIntervalHi
    0x0000, // wKeyFrameRate
    0x0000, // wPFrameRate
    0x0000, // wCompQuality
    0x0000, // wCompWinSize
    0x0000, // wDelay
    0x0000, // wMaxFrameSizeLo
    0x0000, // wMaxFrameSizeHi
    0x0000, // wMaxPayloadSizeLo
    0x0000, // wMaxPayloadSizeHi
} ;

VIDEO_PROBE       vpin;
STILL_PROBE       sp; 	//still probe
STILL_PROBE		  spin;	//still probe in
STILL_PROBE		  sc;	//still commit
STILL_PROBE		  scin;	//still commit in
#if 0
//STREAM_SESSION glStreamSession[2];
#endif
//MMP_UBYTE   gbvcstart;
//MMP_ULONG packetcount;
//MMP_UBYTE frametoggle;
volatile MMP_ULONG SOFNUM;
volatile MMP_ULONG uSOFNUM ;

//MMP_ULONG usbframecount;
//MMP_ULONG glFrameLength  ; // Payload length.

//volatile MMP_UBYTE usb_close_Tx_flag;
//MMP_UBYTE usb_preclose_Tx_flag;

MMP_UBYTE gbUVCDSCCommand = 0;
MMP_UBYTE gbUVCPara[8];


extern MMP_ULONG    glPCCamCnt0,glPCCamCnt1;
extern MMP_ULONG    glPCCamCompressBufAddr;
extern MMP_ULONG    glPCCamCompressBufSize;
extern MMP_BOOL     m_bVidRecdPreviewStatus[];

extern MMP_UBYTE  gbUsbHighSpeed;
extern UsbCtrlReq_t gsUsbCtrlReq ;

extern MMP_RELEASE_VERSION gbFwVersion;
extern MMP_SYSTEM_BUILD_VERSION mmpf_buildVersion;


#if USB_UVC_SKYPE
MMP_UBYTE gbSkypeEncRes = 0x0a;
//MMP_UBYTE gbSkypeFrameRate = 0x1E;
//MMP_UBYTE gbSkypeEnforceKey = 0x00;
MMP_UBYTE gbSkypeMinQP = 0x1E;
MMP_UBYTE gbSkypeMaxQP = 0x32;
MMP_UBYTE gbSkypeCABAC = 0x00;
MMP_USHORT gsSkypeFWDays = 0x0E5A;
MMP_USHORT gsSkypeProfile = 0x0000;
MMP_UBYTE gbSkypeLevel = 31;  // level 3.1 is 31
MMP_USHORT gsSkypeSuppProfile = 0x02;  // baseline profile = 0x02, main profile = 0x04
//MMP_UBYTE gbSkypeUVCVersion = 0x12;  // valid value = 0x12(version 1.2)
MMP_UBYTE gbSkypeMode = 0x01; // default is ModeB
MMP_LONG  glSkypePreYuvOffset;
MMP_ULONG glSkypeYUVAddr;
MMP_ULONG yuv_data_size;
MMP_ULONG yuv_preview_size;
#endif
MMP_UBYTE gbSnapKeyFlag = 0;
MMP_UBYTE gbAEStatus ;
MMP_UBYTE gbAWBStatus ;
//still image test
MMP_UBYTE gbStillCaptureEvent = 0;
MMP_UBYTE gbCaptureBayerRawFlag = 0;
////////////////

extern MMP_USHORT USB_ForceH264IDRFrame(void);

extern MMP_UBYTE gsUSBXU_I2C_ID;
#if USB_UVC_BULK_EP
MMP_UBYTE gbCheckSystemInitFlag = 0;
#endif

STREAM_SESSION glStreamSession[2];

#if 0 // My Log

 640,  480
 160,  120
 176,  144
 320,  180 * O
 320,  240
 352,  288
 424,  240
 480,  270 *
 640,  360 O
 800,  448
 800,  600
 848,  480
 960,  540 *
1024,  576
1280,  720 O
1600,  896
1920, 1080

#endif
RES_TYPE_CFG gsCurResList[] = 
{
	{  PCCAM_640_480  , 640,  480  ,480  ,SENSOR_IN_H  },
	{  PCCAM_1920_1080, 1920, 1080 ,972,SENSOR_16_9_H  },
	{  PCCAM_1280_720 , 1280, 720  ,648,SENSOR_16_9_H  }, //15
	{  PCCAM_640_360  , 640,  360  ,324,SENSOR_16_9_H  },
	//{  PCCAM_864_480  , 864,  480  ,437  ,SENSOR_16_9_H},	
/*
	{  PCCAM_640_480  , 640,  480  ,0  ,SENSOR_16_9_H},    // 1
	{  PCCAM_160_90   , 160,   90  ,0  ,SENSOR_16_9_H},
	{  PCCAM_160_120  , 160,  120  ,0  ,SENSOR_16_9_H},
	{  PCCAM_176_144  , 176,  144  ,0  ,SENSOR_16_9_H},
	{  PCCAM_320_180  , 320,  180  ,180,SENSOR_IN_H  },  // 5
	{  PCCAM_320_240  , 320,  240  ,0  ,SENSOR_16_9_H},
	{  PCCAM_352_288  , 352,  288  ,0  ,SENSOR_16_9_H},
	{  PCCAM_432_240  , 432,  240  ,0  ,SENSOR_16_9_H},

	{  PCCAM_800_448  , 800,  448  ,0  ,SENSOR_16_9_H}, //10
	{  PCCAM_800_600  , 800,  600  ,0  ,SENSOR_16_9_H},

	{  PCCAM_960_720  , 960,  720  ,0  ,SENSOR_16_9_H},
	{  PCCAM_1024_576 , 1024, 576  ,576,SENSOR_IN_H  },
	
	{  PCCAM_1600_896 , 1600, 896  ,0  ,SENSOR_16_9_H},
	
*/
#if 0
	{  PCCAM_2304_1296, 2304, 1296 ,0  ,SENSOR_16_9_H},
	{  PCCAM_2304_1536, 2304, 1536 ,0  ,SENSOR_16_9_H},
	
	{  PCCAM_272_144  , 272,  144  ,0  ,SENSOR_16_9_H}, //20
	{  PCCAM_368_208  , 368,  208  ,0  ,SENSOR_16_9_H},
	{  PCCAM_384_216  , 384,  216  ,216,SENSOR_IN_H  },
	{  PCCAM_480_272  , 480,  272  ,270,SENSOR_IN_H  }, // 272 -> 270
	{  PCCAM_624_352  , 624,  352  ,0  ,SENSOR_16_9_H},
	{  PCCAM_912_512  , 912,  512  ,0  ,SENSOR_16_9_H}, //25
	#if 1//USING_EXT_USB_DESC==0
	// New 4 resoltions add
	{  PCCAM_424_240  , 424,  240  ,0  ,SENSOR_16_9_H/*SENSOR_IN_H*/  }, // 26
	{  PCCAM_480_270  , 480,  270  ,270,SENSOR_IN_H  }, // 27 // 272 -> 270
	{  PCCAM_848_480  , 848,  480  ,480,SENSOR_IN_H  }, // 28
	{  PCCAM_960_540  , 960,  540  ,540,SENSOR_IN_H  }, // 29
	#endif
#endif

	{ PCCAM_RES_NUM  ,0    ,0      }
};

char gsCurResListStr[PCCAM_RES_NUM][20] = 
{
{	"PCCAM_640_480" },
{	"PCCAM_1920_1080"},
{   "PCCAM_1280_720"},
{   "PCCAM_640_360" },
//{   "PCCAM_864_480" }
#if 0
{   "PCCAM_640_480" },

{   "PCCAM_160_90"  },
{   "PCCAM_160_120" },
{   "PCCAM_176_144" },
{   "PCCAM_320_180" },
{	"PCCAM_320_240" },
{   "PCCAM_352_288" },
{   "PCCAM_432_240" },
{   "PCCAM_640_360" },
{   "PCCAM_800_448" },
{   "PCCAM_800_600" },    
{   "PCCAM_864_480" },
{   "PCCAM_960_720" },
{   "PCCAM_1024_576"},
{   "PCCAM_1280_720"},
{   "PCCAM_1600_896"},
{   "PCCAM_1920_1080"},

/*
{   "PCCAM_2304_1296"}, 
{   "PCCAM_2304_1536"},
{   "PCCAM_272_144"},
{   "PCCAM_368x208"},
{   "PCCAM_384_216"},
{   "PCCAM_480x272"},
{   "PCCAM_624x352"},
{   "PCCAM_912x512"}
*/
#endif
};

H264_FORMAT_TYPE gbCurH264Type = INVALID_H264; //SKYPE_H264;//UVC_H264 ;
//
// Local preview streaming type
// Only for UVC H264.
// sean@2011_01_22, this global variable should be change 
// to correct format type in XU command to support local stream is yuy2 or mjpeg
//
MMP_UBYTE        gbCurLocalStreamType = ST_MJPEG ;


//  H264 XU structure.
#if USB_UVC_H264==1
// UVC H264 Spec. Reversion 1.0
//extern MMP_UBYTE            gbUVCXCtlMode ;
extern UVCX_RateControlMode     gsUVCXCtlModeCur;
extern UVCX_TemporalScaleMode   gsUVCXTemporalScaleMode ;
extern UVCX_SpatialScaleMode    gsUVCXSpatialScaleMode  ;
extern UVCX_SNRScaleMode        gsUVCXSNRScaleMode ;
extern MMP_USHORT               gsUVCXVersion ;
extern UVCX_FrameRateConfig     gsUVCXFrameRate ;
extern UVCX_BitRateLayers       gsUVCXBitRate ;
extern UVCX_QPStepsLayers       gsUVCXQPStepSize ;
extern UVCX_PictureTypeControl  gsUVCXPicTypeCtl ;
extern UVCX_CropConfig          gsUVCXCropConfig ;
extern UVCX_AdvConfig           gsUVCXAdvConfig ;
extern UVCX_LTRBufferSizeControl gsUVCXLtrBufferSizeCtl ;
extern UVCX_LTRPictureControl   gsUVCXLtrPictCtl ;
extern MMP_USHORT               gsUVCXEncoderResetId ;

extern VC_CMD_CFG UVCX_RATE_CONTROL_MODE_CFG ;
extern VC_CMD_CFG UVCX_TEMPORAL_SCALE_MODE_CFG ;
extern VC_CMD_CFG UVCX_SPATIAL_SCALE_MODE_CFG ;
extern VC_CMD_CFG UVCX_SNR_SCALE_MODE_CFG ;
extern VC_CMD_CFG UVCX_LTR_BUFFER_SIZE_CONTROL_CFG;
extern VC_CMD_CFG UVCX_LTR_PICTURE_CONTROL_CFG ;
extern VC_CMD_CFG UVCX_PICTURE_TYPE_CONTROL_CFG ;
extern VC_CMD_CFG UVCX_VERSION_CFG ;
extern VC_CMD_CFG UVCX_ENCODER_RESET_CFG ;
extern VC_CMD_CFG UVCX_FRAMERATE_CONFIG_CFG ;
extern VC_CMD_CFG UVCX_VIDEO_ADVANCE_CONFIG_CFG ;
extern VC_CMD_CFG UVCX_BITRATE_CFG ;
extern VC_CMD_CFG UVCX_QP_STEPS_LAYERS_CFG ;
extern VC_CMD_CFG UVCX_VIDEO_ADVANCE_CONFIG_CFG ;
extern VC_CMD_CFG UVCX_CROP_CONFIG_CFG ;

//H264_ENCODER_VFC_CFG gsH264EncoderVFC_Cfg_Cur =
//{
//1280, // Width
//720, // Height
//0, // FramePeriod
//UT_DEFAULT, // UsageTarget
//DUAL_STREAM_EN | DUAL_MJPEG_H264,
//} ;

//H264_ENCODER_VFC_CFG *usb_get_uvc_h264_cfg(void)
//{
//    return (H264_ENCODER_VFC_CFG *)&gsH264EncoderVFC_Cfg_Cur;
//}


#endif

#if (ENCODING_UNIT == 1)
extern VC_CMD_CFG  	EU_SELECT_LAYER_CFG ;
extern VC_CMD_CFG   EU_PROFILE_TOOLSET_CFG ;
extern VC_CMD_CFG  	EU_VIDEO_RESOLUTION_CFG ;
extern VC_CMD_CFG  	EU_FRAME_INTERVAL_CFG;
extern VC_CMD_CFG  	EU_SLICE_MODE_CFG ;
extern VC_CMD_CFG  	EU_RATE_CTRL_MODE_CFG;
extern VC_CMD_CFG  	EU_AVG_BIT_RATE_CFG;
extern VC_CMD_CFG  	EU_CPB_SIZE_CFG;
extern VC_CMD_CFG  	EU_PEAK_BIT_RATE_CFG; 
extern VC_CMD_CFG  	EU_QUANTIZATION_PARAMS_CFG ;
extern VC_CMD_CFG  	EU_SYNC_MODE_CFG ;
extern VC_CMD_CFG  	EU_PRIORITY_ID_CTRL_CFG  ;
extern VC_CMD_CFG  	EU_START_STOP_LAYER_CTRL_CFG ;
extern VC_CMD_CFG  	EU_LEVEL_IDC_LIMIT_CTRL_CFG ;
extern VC_CMD_CFG  	EU_QP_RANGE_CTRL_CFG;
extern VC_CMD_CFG  	EU_SEI_PAYLOAD_TYPE_CTRL_CFG;
extern MMP_USHORT		gw_EU_SelectLayer ;
extern EU_PROF_TOOLSET  gs_EU_ProfToolsetCur;
extern MMP_ULONG		gs_EU_VideoRes;
extern MMP_ULONG		gl_EU_minFrmIntval;
extern EU_SLICEMODE     gs_EU_SliceMode ;
extern MMP_UBYTE		gb_EU_RateCtrl;
extern MMP_ULONG		gl_EU_AvgBitRate;
extern MMP_ULONG		gl_EU_CPBSize;
extern MMP_ULONG		gl_EU_PeakBitRate ;
extern EU_QP			gs_EU_QuantParam ;
extern MMP_ULONG		gs_EU_Sync_RefFrm  ;
extern MMP_UBYTE		gb_EU_PriorityId;
extern MMP_UBYTE		gb_EU_StartLayerCtrl ;
extern MMP_UBYTE		gb_EU_LevelIDCCtrl ;
extern MMP_USHORT       gs_EU_QpRangeCtrl;
extern MMP_UBYTE        gb_EU_SEIPayloadTypeCtrl[8];
#endif

//
// VCI control unit
//
VC_CMD_CFG VCI_ERROR_CFG = {
    ( CAP_GET_INFO_CMD | CAP_GET_CUR_CMD ) ,
    ( INFO_GET_SUPPORT ),
    1,1,
    0,0,0,0,0    
} ;

//
// SU input control unit
//
VC_CMD_CFG  SU_INPUT_SELECT_CFG =
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    1,1,
    0x01,0x01,0x01,0x01,0x01
} ;
    
    
//
// PU control unit
//
VC_CMD_CFG  PU_BACKLIGHT_CFG = // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,BLC_DEF,BLC_MIN,BLC_MAX,BLC_DEF
} ;

VC_CMD_CFG PU_BRIGHTNESS_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,BRI_DEF,BRI_MIN,BRI_MAX,BRI_DEF
} ;


VC_CMD_CFG PU_CONTRAST_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,CON_DEF,CON_MIN,CON_MAX,CON_DEF
};

VC_CMD_CFG PU_POWERLINEREQ_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    1,1,
    0x01,BAND_DEF,BAND_50HZ,BAND_60HZ,BAND_DEF
};

VC_CMD_CFG PU_HUE_CFG =  // (Disabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,HUE_DEF,HUE_MIN,HUE_MAX,HUE_DEF
};

VC_CMD_CFG PU_SATURATION_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,SAT_DEF,SAT_MIN,SAT_MAX,SAT_DEF
};

VC_CMD_CFG PU_SHARPNESS_CFG =   // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,SHA_DEF,SHA_MIN,SHA_MAX,SHA_DEF
};

VC_CMD_CFG PU_GAMMA_CFG =  // (Disabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,GAM_DEF,GAM_MIN,GAM_MAX,GAM_DEF
};

VC_CMD_CFG PU_WB_TEMP_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x000A,WBT_DEF,WBT_MIN,WBT_MAX,WBT_DEF
};

VC_CMD_CFG PU_WB_TEMP_AUTO_CFG =  // (Enabled) 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD  ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    1,1,
    0x01,AWB_DEF,AWB_MIN,AWB_MAX,AWB_DEF
};
// Still need to add the following
// 1. Gain
VC_CMD_CFG PU_GAIN_CFG = 
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT|INFO_SET_SUPPORT),
    2,1,
    0x0001,GAIN_DEF,GAIN_MIN,GAIN_MAX,GAIN_DEF
};


//
// CT control unit 
//
VC_CMD_CFG CT_FOCUS_ABS_CFG =  // (Disabled)
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT | INFO_AUTO_MODE_SUPPORT | INFO_AUTOUPDATE_CONTROL_SUPPORT ),
    2,1,
    AAF_RES,AAF_DEF,AAF_MIN,AAF_MAX,AAF_DEF
};

VC_CMD_CFG CT_FOCUS_AUTO_CFG = // (Enabled)
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT  ),
    1,1,
    0x01,AUTOAF_DEF,AUTOAF_MIN,AUTOAF_MAX,AUTOAF_DEF
};

VC_CMD_CFG CT_ZOOM_ABS_CFG =  // (Enabled)
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT /*| INFO_ASYNC_CONTROL_SUPPORT*/),
    2,1,
    //0x0001,ZOM_DEF,ZOM_MIN,ZOM_MAX,ZOM_DEF
    ZOM_RES,ZOM_DEF,ZOM_MIN,ZOM_MAX,ZOM_DEF
};


const MMP_ULONG glPanTiltDef[2] = {PAN_DEF,TILT_DEF } ;
const MMP_ULONG glPanTiltMax[2] = {PAN_MAX,TILT_MAX } ;
const MMP_ULONG glPanTiltMin[2] = {PAN_MIN,TILT_MIN } ;
const MMP_ULONG glPanTiltRes[2] = {PANTILT_RES,PANTILT_RES } ;

VC_CMD_CFG CT_PANTILT_ABS_CFG =   // (Enabled)
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT /*| INFO_ASYNC_CONTROL_SUPPORT*/ ),
    8,1,
    (MMP_ULONG)glPanTiltRes,(MMP_ULONG)glPanTiltDef,(MMP_ULONG)glPanTiltMin,(MMP_ULONG)glPanTiltMax,(MMP_ULONG)glPanTiltDef
};

// Still need to add the following
// 1. Auto Exposure mode
VC_CMD_CFG CT_AEMODE_CFG =
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD  | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT  ),
    1,1,
    (AEMODE_MANUAL|AEMODE_APERTURE),AEMODE_DEF,AEMODE_DEF,AEMODE_DEF,AEMODE_DEF
};

// 2. Auto Exposure priority
VC_CMD_CFG CT_AEPRIORITY_CFG =
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT  ),
    1,1,
    1,AEPRIORITY_DEF,AEPRIORITY_MIN,AEPRIORITY_MAX,AEPRIORITY_DEF
};


// 3. Exposure time (absolute)
VC_CMD_CFG CT_EXPOSURE_ABS_CFG =
{
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT  ),
    4,1,
    1,EXPOSURE_DEF,EXPOSURE_MIN,EXPOSURE_MAX,EXPOSURE_DEF
};


VC_CMD_CFG CT_FOCUS_SIMPLE_CFG = 
{
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD ),
    ( INFO_GET_SUPPORT | INFO_SET_SUPPORT),
    1,1,
    1,SIMPLE_FOCUS_DEF,SIMPLE_FOCUS_MIN,SIMPLE_FOCUS_MAX,SIMPLE_FOCUS_DEF
} ;

MMP_BYTE gbSignalType = (MMP_UBYTE)-1 ; 

MMP_USHORT UVC_VCD(void)
{
    return gsCurUVCVersion ;
}

void  SET_UVC_VCD(MMP_USHORT ver)
{
    gsCurUVCVersion = ver ;
    dbg_printf(3,"gsCurUVCVersion : %x\r\n",gsCurUVCVersion);
    // re-init endpoint configuration
    usb_ep_init();
}

MMP_USHORT VIDEO_EP_END(void) 
{
    if(gsCurUVCVersion==bcdVCD10) {
        return 1 ;
    }
    return 2 ;
}






//extern VC_CMD_CFG FU_VOL_CFG ;

void usb_vc_cmd_cfg(MMP_UBYTE req,VC_CMD_CFG *cfg,MMP_ULONG cur_val)
{
    int print = 0 ;
    int ret_stall = 0 ;
    /*if((cfg==&FU_VOL_CFG)) {
        RTNA_DBG_Str3("FU_VOL_CFG\r\n");
        print = 1 ;
    }
    */
    /*
    if((cfg==&CT_FOCUS_ABS_CFG)) {
        RTNA_DBG_Str3("CT_FOCUS_ABS_CFG\r\n");
        print = 1 ;
    }
    */
    if(print) {
       // RTNA_DBG_Str3("vc.req : "); RTNA_DBG_Byte3(req); RTNA_DBG_Str3("\r\n");
       // RTNA_DBG_Str3("vc.val : "); RTNA_DBG_Long3(cur_val); RTNA_DBG_Str3("\r\n");
       // dbg_printf(3,"cmd cap : %x,info cap : %x\r\n",cfg->bCmdCap,cfg->bInfoCap );
    }
    
    switch(req) {
    case SET_CUR_CMD:
        if(cfg->bCmdCap & CAP_SET_CUR_CMD) {
            if(cfg->bInfoCap & INFO_AUTO_MODE_SUPPORT) {
            
            }
            if(ret_stall==0) {
                USBOutPhase = req;
                USBOutData = 1 ;
            }
        } else {
            goto invalid_req ;
        }
        break;
    case GET_CUR_CMD:
        if(cfg->bCmdCap & CAP_GET_CUR_CMD) {
            if(cfg->bCmdLen > 4) {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)cur_val,1);   
            } else {
                
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)&cur_val,1);                
            }    
        } else {
            goto invalid_req;
        }
        break;    
    case GET_INFO_CMD:
        if(cfg->bCmdCap & CAP_GET_INFO_CMD) {
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bInfoLen,(MMP_UBYTE *)&cfg->bInfoCap,1) ;    
        } else {
            goto invalid_req;
        } 
        break ;
    
    case GET_DEF_CMD:
        if(cfg->bCmdCap & CAP_GET_DEF_CMD) {

   if(print) {
        RTNA_DBG_Str3("vc.def : "); RTNA_DBG_Long3(cfg->dwDefVal); RTNA_DBG_Str3("\r\n");
   }
        
            if(cfg->bCmdLen > 4) {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)cfg->dwDefVal,1);   
            } else {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)&cfg->dwDefVal,1);                
            }    
        } else {
            goto invalid_req;
        }
        break;    
    case GET_MAX_CMD:
        if(cfg->bCmdCap & CAP_GET_MAX_CMD) {
   if(print) {
        RTNA_DBG_Str3("vc.max : "); RTNA_DBG_Long3(cfg->dwMaxVal); RTNA_DBG_Str3("\r\n");
   }
            if(cfg->bCmdLen > 4) {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)cfg->dwMaxVal,1);   
            } else {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)&cfg->dwMaxVal,1);                
            }    
        } else {
            goto invalid_req;
        }
        break;    
    case GET_MIN_CMD:
        if(cfg->bCmdCap & CAP_GET_MIN_CMD) {
   if(print) {
        RTNA_DBG_Str3("vc.mix : "); RTNA_DBG_Long3(cfg->dwMinVal); RTNA_DBG_Str3("\r\n");
   }
            if(cfg->bCmdLen > 4) {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)cfg->dwMinVal,1);   
            } else {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)&cfg->dwMinVal,1);
            }    
        } else {
            goto invalid_req;
        }
        break;    
    case GET_RES_CMD:
        if(cfg->bCmdCap & CAP_GET_RES_CMD) {
   if(print) {
        RTNA_DBG_Str3("vc.res : "); RTNA_DBG_Long3(cfg->dwResVal); RTNA_DBG_Str3("\r\n");
   }
            if(cfg->bCmdLen > 4) {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)cfg->dwResVal,1);
            } else {
                 USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,cfg->bCmdLen,(MMP_UBYTE *)&cfg->dwResVal,1);
            }    
        } else {
            goto invalid_req;
        }
        break;
    case GET_LEN_CMD:
        if(cfg->bCmdCap & CAP_GET_LEN_CMD) {
            MMP_USHORT cmdLen = cfg->bCmdLen ;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,2,(MMP_UBYTE *)&cmdLen,1);  
        } else {
            goto invalid_req;
        }
        break;    
    default:
invalid_req: 
        if(print) {
            RTNA_DBG_Str3("Cfg.Err\r\n");
        }   
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_REQUEST ;  
        break ;          
                        
    }

    //if(print) {
    //    RTNA_DBG_Str3("Cfg.End\r\n");
    //}   

}

//
// sean@2011_01_21, revised usb_vc_update_automode()
// use usb_vc_update_async_mode() to reply status
// 
void usb_vc_update_automode(void)
{
    /*
    RTNA_DBG_Str(0, "  ** usb_vc_update_automode() **\r\n");
    VAR_B(0,gbUpdAE);
    VAR_B(0,gbUpdAWB);
    VAR_B(0,gbUpdAF);
    */
    
    MMP_UBYTE originator,selector = 0,value = 0 ;//,attr,
    //dbg_printf(0,"#auto mode(AE,AWB,AF):(%d,%d,%d)\r\n",gbUpdAE,gbUpdAWB,gbUpdAF);
    if(gbUpdAE) {
        gbUpdAE = 0;
        originator = gbVCITID ;
        selector = CT_EXPOSURE_TIME_ABSOLUTE_CONTROL ;
        value = gbEVInfo ;
    } else if(gbUpdAWB) {
        gbUpdAWB = 0;
        originator = gbVCPUID ;
        selector = PU_WHITE_BALANCE_TEMPERATURE_CONTROL ;
        value = gbTemperInfo ;
    } else if (gbUpdAF) {
        gbUpdAF = 0 ;
        originator = gbVCITID ;
        selector = CT_FOCUS_ABSOLUTE_CONTROL ;
        value = gbFocusInfo ;
    }
    if(selector) {
        usb_vc_update_async_mode(originator,selector,VC_STS_ATTR_UPDATE_INFO,&value,1);
    }
}

// Async. Mode for VC control
void usb_vc_update_async_mode(MMP_UBYTE bOriginator,MMP_UBYTE bSelector,MMP_UBYTE bAttribute,void *bValue,MMP_UBYTE bValUnit)
{
    MMP_UBYTE ep = PCCAM_EX_EP_ADDR ;
    
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    #if 0
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0x01;//VideoControl interface
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = bOriginator;
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0x00;//Control Change
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = bSelector;// Control Selector
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = bAttribute;//0:value,1:info,2:failure
    #else
    MMP_UBYTE cmd[5] ;
    cmd[0] = 0x01;//VideoControl interface
    cmd[1] = bOriginator;
    cmd[2] = 0x00;//Control Change
    cmd[3] = bSelector;// Control Selector
    cmd[4] = bAttribute;//0:value,1:info,2:failure
    USB_PUT_FIFO_DATA(0,ep,5,(MMP_UBYTE *)cmd,1) ;
    #endif
    USB_PUT_FIFO_DATA(0,ep,bValUnit,(MMP_UBYTE *)bValue,1) ;
    EnableEx(PCCAM_EX_EP_ADDR);
}


void usb_vc_capture_trigger(MMP_UBYTE pressed)
{
    MMP_UBYTE ep = PCCAM_EX_EP_ADDR ;
    
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    MMP_UBYTE cmd[4] ;
    
    RTNA_DBG_Str(0, "  ** usb_vc_capture_trigger() **\r\n");
    RTNA_DBG_Short(0, pressed);     
    RTNA_DBG_Str(0, "-pressed\r\n"); 

    if(gbSnapKeyFlag == pressed)  return;
    
    gbSnapKeyFlag = pressed;
    #if 0
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0x02;//VideoStream interface
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = gbVCVSIF1;
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = 0x00;//Button Press
    pUSB_CTL->USB_FIFO_EP[ep].FIFO_B = pressed;//0:released,1:pressed
    #else
    cmd[0] = 0x02 ;
    cmd[1] = gbVCVSIF1 ;
    cmd[2] = 0x00 ;
    cmd[3] = pressed ;
    USB_PUT_FIFO_DATA(0,ep,4,(MMP_UBYTE *)cmd,1) ;    
    #endif
    EnableEx(PCCAM_EX_EP_ADDR);
}
/*
void gpio_c_irq_handler()
{
    usb_vc_capture_trigger(0);
}
*/

void uvc_init(MMP_BOOL reset_val,MMP_UBYTE ep_id)
{
    if(!reset_val) {
        STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id);
        ss->packetcount = 0 ;
        ss->usbframecount = 0;
        //ss->tx_flag&=~SS_TX_CLOSE_STREAMING ;
        ss->tx_flag |= SS_TX_CLOSE_STREAMING ;
        ss->tx_flag&=~SS_TX_PRECLOSE_STREAMING;
     //   ss->pipe_id = 0xFF ;
     
        //dbg_printf(3,"*ep_id : %d,tx_flag : %x\r\n",ep_id,ss->tx_flag);
    } else {
        STREAM_SESSION *ss;
        ss = MMPF_Video_GetStreamSessionByEp(0);
        if(ss) {
            ss->packetcount = 0 ;
            ss->usbframecount = 0;
            ss->tx_flag = SS_TX_CLOSE_STREAMING ;
            ss->pipe_id = 0xFF ;
        }
        ss = MMPF_Video_GetStreamSessionByEp(1);
        if(ss) {
            ss->packetcount = 0 ;
            ss->usbframecount = 0;
            ss->tx_flag = 0;
            ss->pipe_id = 0xFF ;
        }
    }
      
    //packetcount = 0;
    //usbframecount = 0;
    //usb_close_Tx_flag = 0;
    //usb_preclose_Tx_flag = 0;
	//glPan = PAN_DEF;
	//glTilt = TILT_DEF ;
    //gsZoom = ZOM_DEF;
    if(reset_val) {
        /*glPan = PAN_DEF;
        glTilt = TILT_DEF ;
        gsZoom = ZOM_DEF;
        gsBacklight = BLC_DEF;
        gsTemper = WBT_DEF;
        gsSaturation = SAT_DEF;
        gsContrast = CON_DEF;
        gsBrightness = BRI_DEF;
        gsHue = HUE_DEF;
        gsGamma = GAM_DEF;
        gsSharpness = SHA_DEF;
        gsGain = GAIN_DEF ;
        gbBandFilter = BAND_60HZ;
        glEV = EXPOSURE_DEF ;
        gsAFocus = AAF_DEF;*/
        
        // New in UVC1.5
        gbSimpleFocus = SIMPLE_FOCUS_DEF;
        // sean@2011_03_15, reset 3A, reinit again after reset
        MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_RESET);
    }
 
    if(reset_val) {
        /*gbAutoFocus = AUTOAF_DEF;
        gbAE = AEMODE_DEF ;//2;
        gbAWB = AWB_DEF ;*/
    }
    gblastAF =  gbAutoFocus ;
    gblastAE = gbAE ;//2;
    gblastAWB = gbAWB;//1; //sean@2011_01_21, 1-> gbAWB
    gbAEPriority = AEPRIORITY_DEF ;
    
    
    gbTemperInfo = INFO_AUTOUPDATE_CONTROL_SUPPORT | INFO_AUTO_MODE_SUPPORT | INFO_SET_SUPPORT | INFO_GET_SUPPORT;
    gbEVInfo = INFO_AUTOUPDATE_CONTROL_SUPPORT | INFO_AUTO_MODE_SUPPORT | INFO_GET_SUPPORT | INFO_SET_SUPPORT;
    gbFocusInfo  = INFO_AUTOUPDATE_CONTROL_SUPPORT | INFO_AUTO_MODE_SUPPORT | INFO_GET_SUPPORT | INFO_SET_SUPPORT;  
}



void usb_vc_cfg_video_probe(MMP_USHORT indx,MMP_ULONG max_frame_size,MMP_ULONG max_payload_size,MMP_ULONG frameInterval)
{

   // VAR_L(0,indx);
   // VAR_L(0,max_frame_size);
   // VAR_L(0,max_payload_size);
   // VAR_L(0,frameInterval);
    //dbg_printf(3,"#cfg.probe : idex:%x,framesize:%d,interval:%d\r\n",indx,max_frame_size,frameInterval);
    vp.wIndex = indx ;
    vp.wMaxFrameSizeLo   =  max_frame_size & 0xFFFF ;
    vp.wMaxFrameSizeHi   = (max_frame_size >> 16 ) & 0xFFFF ;
    vp.wMaxPayloadSizeLo = max_payload_size & 0xFFFF ;       
    vp.wMaxPayloadSizeHi = (max_payload_size >> 16 ) & 0xFFFF ;
    vp.wIntervalLo = frameInterval & 0xFFFF ;       
    vp.wIntervalHi = (frameInterval >> 16 ) & 0xFFFF ;
        
}

void usb_vc_get_video_probe(MMP_UBYTE IfIndex,MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex,MMP_USHORT fpsx10,MMP_BOOL upd)
{
// enlarge h264 frame size for probe / commit 
#define H264_REPORT_FRAME_SIZE  (H264E_RINGBUF_SIZE)

#if 1
    MMP_USHORT index = 0;
    MMP_ULONG  maxFrame = 0,maxPayload = 0, frameInterval = 0;
    PCAM_USB_VIDEO_FORMAT   pCamVidFmt;
    MMP_ULONG               pCamRes = 0;
    //MMP_UBYTE               lmode;
    
   // VAR_B(3,fps);
    //lmode = FrameIndex;//mode;
#if USB_UVC_BULK_EP
    maxPayload = UVC_DMA_SIZE;//0x2000;        
#else     
    if(IfIndex==gbVCVSIF1) {
        maxPayload = UVCTxFIFOSize[0] ; //EP2TxFIFOSize;        
    } else {
        maxPayload = UVCTxFIFOSize[1] ;
    }    
#endif

    index = (FrameIndex<<8) + FormatIndex ;
    if(FormatIndex==NV12_FORMAT_INDEX) {
        pCamVidFmt = PCAM_USB_VIDEO_FORMAT_YUV420 ;
        pCamRes = FrameIndex - 1 ;
        maxFrame = GetYUV420FrameSize((MMP_UBYTE)pCamRes); 
    }
    else if(FormatIndex==YUY2_FORMAT_INDEX) {
        pCamVidFmt = PCAM_USB_VIDEO_FORMAT_YUV422 ;
        pCamRes = FrameIndex - 1 ;
        maxFrame = GetYUY2FrameSize((MMP_UBYTE)pCamRes); 
    }
    else  if(FormatIndex==(MJPEG_FORMAT_INDEX)) {
        pCamRes = FrameIndex -1;	        
        maxFrame = GetMJPEGFrameSize((MMP_UBYTE)pCamRes);
    #if USB_MJPEGH264_STREAM==1
        gbCurH264Type = UVC_H264 ; 
        if( (gbCurH264Type==UVC_H264) && (gbCurLocalStreamType==ST_MJPEG)) {
            //dbg_printf(3,"MJPEG+H264 Frame size:%d\r\n",maxFrame);
        }
        if( (gbCurH264Type==UVC_H264) && (gbCurLocalStreamType==ST_YUY2)) {
            // Fixed Local Preview Size
            maxFrame += GetYUY2FrameSize(PCCAM_640_360/*(MMP_UBYTE)pCamRes*/);
            //dbg_printf(3,"YUY2+H264 Frame size:%d\r\n",maxFrame);
        } 
        
        if( (gbCurH264Type==UVC_YUY2) && (gbCurLocalStreamType==ST_MJPEG) ) {
            maxFrame += GetYUY2FrameSize(pCamRes); 
            dbg_printf(3,"MJPEG+YUY2 Frame size:%d\r\n",maxFrame);
        } 
        maxFrame += H264_REPORT_FRAME_SIZE;// GetH264FrameSize((MMP_UBYTE)pCamRes);
    #endif        
        pCamVidFmt = PCAM_USB_VIDEO_FORMAT_MJPEG ;
    
    }
    else if(FormatIndex==(FRAME_BASE_H264_INDEX)) {
        pCamVidFmt = PCAM_USB_VIDEO_FORMAT_H264 ;
        gbCurH264Type = FRAMEBASE_H264 ;
        pCamRes = FrameIndex -1;	
        maxFrame = H264_REPORT_FRAME_SIZE;//GetH264FrameSize((MMP_UBYTE)pCamRes);
    #if USB_FRAMEBASE_H264_DUAL_STREAM //  AIT stream viewer
        if(gbSignalType==0) {
            gbCurH264Type = FRAMEBASE_H264_YUY2 ;
            maxFrame += GetYUY2FrameSize(PCCAM_640_480/*(MMP_UBYTE)pCamRes*/);
            dbg_printf(3,"YUY2+H264 %d Frame size:%d,gbCurH264Type:%d\r\n",FrameIndex,maxFrame,gbCurH264Type);
        }
        else if(gbSignalType==1) {
            gbCurH264Type = FRAMEBASE_H264_MJPEG ;
            maxFrame += GetMJPEGFrameSize(PCCAM_1920_1080/*(MMP_UBYTE)pCamRes*/);
            dbg_printf(3,"MJPEG+H264 %d Frame size:%d,gbCurH264Type:%d\r\n",FrameIndex,maxFrame,gbCurH264Type);
        }
        else { 
            dbg_printf(3,"H264 only\r\n");
        }
    #endif        
    }

    //frameInterval =(MMP_ULONG)( 10000000 / fps );
    frameInterval =(MMP_ULONG)( ( 10000000*10) / fpsx10 );
    
    usb_vc_cfg_video_probe(index,maxFrame,maxPayload,frameInterval);
	
    if(upd) {
        usb_vc_dump_vsi_info(IfIndex);
        if(pCamVidFmt == PCAM_USB_VIDEO_FORMAT_H264 ) {
            USB_SetH264Resolution(PCAM_API/*PCAM_NONBLOCKING*/,pCamRes );
            dbg_printf(3,"commit : H264\r\n");
        } else {
            USB_SetVideoFormat(PCAM_API/*PCAM_NONBLOCKING*/,pCamVidFmt) ;
            //glPccamResolution = pCamRes ;
           // dbg_printf(3,"commit : Non-H264\r\n");
        }
        //glPccamResolution = pCamRes ;
    }

#endif

}


void usb_vc_set_still_commit(STILL_PROBE* pSCIN)
{
	//RTNA_DBG_Str(0, "usb_vc_set_still_commit \r\n");
    sc = *pSCIN ;
}

void usb_vc_get_still_probe(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex ,STILL_PROBE* pSP)
{
#if 0
	MMP_ULONG maxFrame ;
	MMP_ULONG maxPayload ;
	//RTNA_DBG_Str(0, "usb_vc_get_still_probe : ");

#if ENABLE_MJPEG==1	
	if (FormatIndex==(MJPEG_FORMAT_INDEX) ) {
    #if CAPTURE_BAYER_RAW_ENABLE
        maxFrame = 0x00280000 ;
    #else
        maxFrame = 0x00140000 ;
    #endif	
    #if USB_UVC_BULK_EP
        maxPayload = UVC_DMA_SIZE ;
    #else
        maxPayload = UVCTxFIFOSize[0] ;//EP2TxFIFOSize ;
    #endif
       // RTNA_DBG_Str(0, "[MJPEG Order]\r\n");
    }
#endif

#if ENABLE_YUY2==1
    if(FormatIndex==YUY2_FORMAT_INDEX) {
        RTNA_DBG_Str(0, "[YUY2 Order]\r\n");
    }
#endif
#if ENABLE_NV12==1
    if(FormatIndex==NV12_FORMAT_INDEX) {
        RTNA_DBG_Str(0, "[YUY2 Order]\r\n");
    }
#endif

	
	switch(FormatIndex)
	{
	#if ENABLE_MJPEG==1
	case MJPEG_FORMAT_INDEX:
		switch(FrameIndex)
		{
#if (CAPTURE_FULL_SCALE)
			case 1: // JPEG 1600x1200
				RTNA_DBG_Str(0, "  ** JPEG 1600x1200 \r\n");
			    break;
#else
			case 1: // JPEG 1280x720
				RTNA_DBG_Str(0, "  ** JPEG 1280x720 \r\n");
			    break;
#endif
#if (CUSTOMER == SAL) || (CUSTOMER == LGT) || (CUSTOMER == BIS_5M)
			    
			case 2: // JPEG 2560x1920
				RTNA_DBG_Str(0, "  ** JPEG 2560x1920 \r\n");
			    break;
			
			case 3: // JPEG 2048x1536
				RTNA_DBG_Str(0, "  ** JPEG 2048x1536 \r\n");
			    break;
			    
			case 4: // JPEG 1920x1080
				RTNA_DBG_Str(0, "  ** JPEG 1920x1080 \r\n");
			    break;
			    
			case 5: // JPEG 1280x1024
				RTNA_DBG_Str(0, "  ** JPEG 1280x1024 \r\n");
			    break;
#else

			case 2: // JPEG 800x600
				RTNA_DBG_Str(0, "  ** JPEG 800x600 \r\n");
			break;
			
			case 3: // JPEG 640x480
				RTNA_DBG_Str(0, "  ** JPEG 640x480 \r\n");
			    break;
			    
			case 4: // JPEG 320x240
				RTNA_DBG_Str(0, "  ** JPEG 320x240 \r\n");
			    break;
			    
			case 5: // JPEG 160x120
				RTNA_DBG_Str(0, "  ** JPEG 160x120 \r\n");
			    break;

#endif			    
			default:
			break;
		}
		break;
	#endif
	
	#if  ENABLE_YUY2==1
	
	case YUY2_FORMAT_INDEX:
		switch(FrameIndex)
		{
			case 1: // YUY2 640x480
				RTNA_DBG_Str(0, "  ** YUY2 640x480 \r\n");
				maxFrame = 640*480*2 ;
			    break;		
			
			case 2: // YUY2 320x240
				RTNA_DBG_Str(0, "  ** YUY2 320x240 \r\n");
				maxFrame = 320*240*2 ;
			    break;
			
			case 3: // YUY2 160x120
				RTNA_DBG_Str(0, "  ** YUY2 160x120 \r\n");
				maxFrame = 160*120*2 ;
			    break;
			
			default:
			break;
		}
		break;
	#endif
	default:
		break;
	}
	pSP->bFormatIndex = FormatIndex;
	pSP->bFrameIndex  = FrameIndex;
    pSP->bMaxFrameSize[0] = (  maxFrame & 0xFF);
    pSP->bMaxFrameSize[1] = ( (maxFrame >> 8)  & 0xFF );
    pSP->bMaxFrameSize[2] = ( (maxFrame >> 16) & 0xFF );
    pSP->bMaxFrameSize[3] = ( (maxFrame >> 24) & 0xFF );
    
    pSP->bMaxPayloadSize[0] = (  maxPayload & 0xFF);
    pSP->bMaxPayloadSize[1] = ( (maxPayload >> 8)  & 0xFF );
    pSP->bMaxPayloadSize[2] = ( (maxPayload >> 16) & 0xFF );
    pSP->bMaxPayloadSize[3] = ( (maxPayload >> 24) & 0xFF );	
#endif	
}

//void usb_vc_set_still_probe(STILL_PROBE* pSPIN)
void usb_vc_set_still_probe(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex)
{

	usb_vc_get_still_probe(FormatIndex,FrameIndex,&sp);
}

void usb_vc_set_still_resolution(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex)
{
#if 0
   // PCAM_USB_VIDEO_FORMAT pCamVidFmt;
    
   	switch(FormatIndex)
   	{
   	#if ENABLE_YUY2==1
   		case YUY2_FORMAT_INDEX:
   		    //m_VideoFmt = MMPS_3GPRECD_VIDEO_FORMAT_YUV422;
   		    //pCamVidFmt = PCAM_USB_VIDEO_FORMAT_YUV422 ;
   		    USB_SetVideoFormat(PCAM_NONBLOCKING,PCAM_USB_VIDEO_FORMAT_YUV422 );
            RTNA_DBG_Str(0, " ** Still Format is YUV422\r\n"); 
			glPccamResolution = FrameIndex - 1;   				
   			break;
   	#endif
   	

   	#if ENABLE_MJPEG==1
   		case MJPEG_FORMAT_INDEX:
   		    //m_VideoFmt = MMPS_3GPRECD_VIDEO_FORMAT_MJPEG;
   		    //pCamVidFmt = PCAM_USB_VIDEO_FORMAT_MJPEG ;
   		    USB_SetVideoFormat(PCAM_NONBLOCKING,PCAM_USB_VIDEO_FORMAT_MJPEG );
            RTNA_DBG_Str(0, " ** Still Format is JPEG\r\n"); 
			glPccamResolution = FrameIndex - 1;   			
   			break;
   	#endif
   	}
#endif   	
   	//pcam_usb_set_attributes(PCAM_USB_SETTING_VIDEO_FORMAT, &pCamVidFmt);
}

//extern PCAM_USB_VIDEO_FORMAT gsVidFmt;
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_UBYTE gbDevicePowerSavingStatus;
extern MMPF_PWM_ATTRIBUTE pulseA ;
extern MMP_BOOL gbSimulcastH264 ;

MMP_UBYTE STREAM_EP_H264 ;
MMP_UBYTE STREAM_EP_YUY2 ;
MMP_UBYTE STREAM_EP_MJPEG ;
MMP_UBYTE STREAM_EP_NV12  ;

MMP_UBYTE gbChangeH264Res ;
#if H264_SIMULCAST_EN
MMP_UBYTE gbChangeH264BufMode ,gbRestartPreview;
#endif
void usb_vc_vci_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
//    volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
    case VC_VIDEO_POWER_MODE_CONTROL:
        // under coding
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_UNKNOWN;
        break;
    case VC_REQUEST_ERROR_CODE_CONTROL:
        usb_vc_cmd_cfg(req,&VCI_ERROR_CFG,gbVCERRCode);    
        break;
    case VC_CONTROL_UNDEFINED:
    case VC_RESERVED:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}

void usb_vc_su_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
 //   volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
    case SU_INPUT_SELECT_CONTROL:
        usb_vc_cmd_cfg(req,&SU_INPUT_SELECT_CFG,0x01);
        break;
    case SU_CONTROL_UNDEFINED:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}

void usb_vc_ct_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
//    volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    //dbg_printf(3,"ct : %d, req : %x\r\n",cs,req );
    
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    switch(cs) {
#if CT_CS_VAL & (1 << CT_FOCUS_SIMPLE_CONTROL )
        case CT_FOCUS_SIMPLE_CONTROL:
            usb_vc_cmd_cfg(req,&CT_FOCUS_SIMPLE_CFG,gbSimpleFocus);
            break;
#endif    
#if CT_CS_VAL & (1 << CT_FOCUS_ABSOLUTE_CONTROL)         
        case CT_FOCUS_ABSOLUTE_CONTROL:
        #if SUPPORT_AUTO_FOCUS
            CT_FOCUS_ABS_CFG.bInfoCap = gbFocusInfo ;
            usb_vc_cmd_cfg(req,&CT_FOCUS_ABS_CFG,gsAFocus);  
            break;       
        #else
            goto invalid_ct ; // CH9 FOCUS FAILED 
        #endif
#endif            
        
#if CT_CS_VAL & (1 << CT_FOCUS_AUTO_CONTROL)         
        case CT_FOCUS_AUTO_CONTROL:
        #if SUPPORT_AUTO_FOCUS 
            usb_vc_cmd_cfg(req,&CT_FOCUS_AUTO_CFG,gbAutoFocus);  
            break;       
        #else
            goto invalid_ct ;
        #endif
#endif
#if CT_CS_VAL & (1 << CT_ZOOM_ABSOLUTE_CONTROL)         
        case CT_ZOOM_ABSOLUTE_CONTROL:
        #if SUPPORT_DIGITAL_ZOOM   
            if(USB_IsPreviewActive()) {
                //CT_ZOOM_ABS_CFG.bInfoCap |=  INFO_ASYNC_CONTROL_SUPPORT ;
            } else {
                //CT_ZOOM_ABS_CFG.bInfoCap &= ~INFO_ASYNC_CONTROL_SUPPORT ;
            }
            usb_vc_cmd_cfg(req,&CT_ZOOM_ABS_CFG,gsZoom);  
            break;       
        #else
            goto invalid_ct ;
        #endif 
#endif
#if CT_CS_VAL & (1 << CT_PANTILT_ABSOLUTE_CONTROL)         
          
        case CT_PANTILT_ABSOLUTE_CONTROL:
        #if SUPPORT_DIGITAL_PAN  
        {
            MMP_ULONG curVal[2] ;
            curVal[0] = glPan ;
            curVal[1] = glTilt ; 
            if(USB_IsPreviewActive()) {
                //CT_PANTILT_ABS_CFG.bInfoCap |=  INFO_ASYNC_CONTROL_SUPPORT ;
            } else {
                //CT_PANTILT_ABS_CFG.bInfoCap &= ~INFO_ASYNC_CONTROL_SUPPORT ;
            }
            usb_vc_cmd_cfg(req,&CT_PANTILT_ABS_CFG,(MMP_ULONG)curVal);  
            break;       
        }
        #else
            goto invalid_ct ;
        #endif 
#endif
#if CT_CS_VAL & (1 << CT_AE_MODE_CONTROL)         
        case CT_AE_MODE_CONTROL:
            usb_vc_cmd_cfg(req,&CT_AEMODE_CFG,gbAE);  
            break ;
#endif         

#if CT_CS_VAL & (1 << CT_AE_PRIORITY_CONTROL)         
        case CT_AE_PRIORITY_CONTROL:
            usb_vc_cmd_cfg(req,&CT_AEPRIORITY_CFG,gbAEPriority);
            break ;
#endif         
#if CT_CS_VAL & (1 << CT_EXPOSURE_TIME_ABSOLUTE_CONTROL)         
        case CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            CT_EXPOSURE_ABS_CFG.bInfoCap = gbEVInfo;
            //VAR_L(0,glEV);
            // Can't pass UVC if do this
            //glEV = USB_GetExposureTime();
            usb_vc_cmd_cfg(req,&CT_EXPOSURE_ABS_CFG,glEV);
            break ;
#endif         

 
        case CT_CONTROL_UNDEFINED:
        case CT_SCANNING_MODE_CONTROL:
        case CT_EXPOSURE_TIME_RELATIVE_CONTROL:
        case CT_FOCUS_RELATIVE_CONTROL:
        case CT_IRIS_ABSOLUTE_CONTROL:
        case CT_IRIS_RELATIVE_CONTROL:
        case CT_ZOOM_RELATIVE_CONTROL:
        case CT_PANTILT_RELATIVE_CONTROL:
        case CT_ROLL_ABSOLUTE_CONTROL:
        case CT_ROLL_RELATIVE_CONTROL:
        case CT_PRIVACY_CONTROL:
invalid_ct:        
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_CONTROL;
            break;
    }
}


void usb_vc_pu_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
//    volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   
    cs = (/*UsbRequestPayload_wValue */gsUsbCtrlReq.wValue>> 8);
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    switch(cs) {
#if PU_CS_VAL & (1 << PU_BACKLIGHT_COMPENSATION_CONTROL)            
        case PU_BACKLIGHT_COMPENSATION_CONTROL:
            usb_vc_cmd_cfg(req,&PU_BACKLIGHT_CFG,gsBacklight);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_BRIGHTNESS_CONTROL)          
        case PU_BRIGHTNESS_CONTROL:
            usb_vc_cmd_cfg(req,&PU_BRIGHTNESS_CFG,gsBrightness);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_CONTRAST_CONTROL)      
        case PU_CONTRAST_CONTROL:
            usb_vc_cmd_cfg(req,&PU_CONTRAST_CFG,gsContrast);
            break;
#endif 
#if PU_CS_VAL & (1 << PU_GAIN_CONTROL)         
        case PU_GAIN_CONTROL:
           //  PU_GAIN_CFG.bInfoCap = gbEVInfo ;
            PU_GAIN_CFG.bInfoCap = gbEVInfo;
            usb_vc_cmd_cfg(req,&PU_GAIN_CFG,gsGain);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_POWER_LINE_FREQUENCY_CONTROL)         
        case PU_POWER_LINE_FREQUENCY_CONTROL:
            usb_vc_cmd_cfg(req,&PU_POWERLINEREQ_CFG,gbBandFilter); 
            break;
#endif            
#if PU_CS_VAL & (1 << PU_HUE_CONTROL)         
        case PU_HUE_CONTROL:
            usb_vc_cmd_cfg(req,&PU_HUE_CFG,gsHue);
            break;
#endif
#if PU_CS_VAL & (1 << PU_SATURATION_CONTROL)         
        case PU_SATURATION_CONTROL:
            usb_vc_cmd_cfg(req,&PU_SATURATION_CFG,gsSaturation);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_SHARPNESS_CONTROL)         
        case PU_SHARPNESS_CONTROL:
            usb_vc_cmd_cfg(req,&PU_SHARPNESS_CFG,gsSharpness);
            break;
#endif
#if PU_CS_VAL & (1 << PU_GAMMA_CONTROL)         
        case PU_GAMMA_CONTROL:
            usb_vc_cmd_cfg(req,&PU_GAMMA_CFG,gsGamma);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_WHITE_BALANCE_TEMPERATURE_CONTROL)         
        case PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            PU_WB_TEMP_CFG.bInfoCap = gbTemperInfo;
            usb_vc_cmd_cfg(req,&PU_WB_TEMP_CFG,gsTemper);
            break;
#endif            
#if PU_CS_VAL & (1 << PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL)         
        case PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            usb_vc_cmd_cfg(req,&PU_WB_TEMP_AUTO_CFG,gbAWB);
            break;
#endif
unsupport_pu:        
        case PU_CONTROL_UNDEFINED:
        case PU_WHITE_BALANCE_COMPONENT_CONTROL:
        case PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
        case PU_DIGITAL_MULTIPLIER_CONTROL:
        case PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
        case PU_HUE_AUTO_CONTROL:
        case PU_ANALOG_VIDEO_STANDARD_CONTROL:
        case PU_ANALOG_LOCK_STATUS_CONTROL:
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_CONTROL;
            break;
    }
}

#if (ENCODING_UNIT == 1)
//-----------------------------------------------------------------------------------//
//-	Function 	: usb_vc_eu_cs(MMP_UBYTE req)										-//
//-	Description	: Encoding Unit defined in UVC1.5									-//
//-																					-//
//-	Parameters	: req: SET_CUR, GET_XXX.....										-//
//- Return		: None.																-//
//-----------------------------------------------------------------------------------//
void usb_vc_eu_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs,size_override;

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
#if EU_CS_VAL & (1 << EU_SELECT_LAYER_CONTROL)            
		//-------------------------------------------//
		//-		CS1:	Select Layer Control		-//
		//-------------------------------------------//
        case EU_SELECT_LAYER_CONTROL:
            usb_vc_cmd_cfg(req, &EU_SELECT_LAYER_CFG, gw_EU_SelectLayer);
            break;
#endif            

#if EU_CS_VAL & (1 << EU_PROFILE_TOOLSET_CONTROL)          
		//-------------------------------------------//
		//-		CS2: Profile & Toolset Control		-//
		//-------------------------------------------//
        case EU_PROFILE_TOOLSET_CONTROL:
            usb_vc_cmd_cfg(req, &EU_PROFILE_TOOLSET_CFG, (MMP_ULONG)&gs_EU_ProfToolsetCur);
            break;
#endif

#if EU_CS_VAL & (1 << EU_VIDEO_RESOLUTION_CONTROL)      
		//-------------------------------------------//
		//-		CS3: Video Resolution Control		-//
		//-------------------------------------------//
        case EU_VIDEO_RESOLUTION_CONTROL:
        {
            MMP_ULONG res = 0;
            res = gs_EU_VideoRes;//EU_S2L(gs_EU_VideoRes.wWidth,gs_EU_VideoRes.wHeight);
			size_override = 1;
            usb_vc_cmd_cfg(req, &EU_VIDEO_RESOLUTION_CFG, (MMP_ULONG)res);
			size_override = 0;
            break;
        }    
#endif 

#if EU_CS_VAL & (1 << EU_MIN_FRAME_INTERVAL_CONTROL)         
	//-------------------------------------------//
	//-		CS4: min. frame interval Control	-//
	//-------------------------------------------//
        case EU_MIN_FRAME_INTERVAL_CONTROL:
            usb_vc_cmd_cfg(req, &EU_FRAME_INTERVAL_CFG, gl_EU_minFrmIntval);
            break;
#endif            

#if EU_CS_VAL & (1 << EU_SLICE_MODE_CONTROL)         
	//-------------------------------------------//
	//-		CS5: 	Slice Mode Control			-//
	//-------------------------------------------//
        case EU_SLICE_MODE_CONTROL:
        {
            MMP_ULONG slicemode ;
            slicemode = EU_S2L(gs_EU_SliceMode.wSliceMode,gs_EU_SliceMode.wSliceConfigSetting) ;
			size_override = 1;
            usb_vc_cmd_cfg(req,&EU_SLICE_MODE_CFG, (MMP_ULONG)slicemode ); 
			size_override = 0;
            break;
        }    
#endif            

#if EU_CS_VAL & (1 << EU_RATE_CONTROL_MODE_CONTROL)         
		//-------------------------------------------//
		//-		CS6: Rate control Mode 				-//
		//-------------------------------------------//
        case EU_RATE_CONTROL_MODE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_RATE_CTRL_MODE_CFG, gb_EU_RateCtrl);
            break;
#endif

#if EU_CS_VAL & (1 << EU_AVERAGE_BITRATE_CONTROL)         
		//-------------------------------------------//
		//-		CS7: Average Bit Rate Control		-//
		//-------------------------------------------//
        case EU_AVERAGE_BITRATE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_AVG_BIT_RATE_CFG, gl_EU_AvgBitRate);
            break;
#endif            

#if EU_CS_VAL & (1 << EU_CPB_SIZE_CONTROL)         
		//-------------------------------------------//
		//-		CS8:  	CPB Size Control			-//
		//-------------------------------------------//
        case EU_CPB_SIZE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_CPB_SIZE_CFG, gl_EU_CPBSize);
            break;
#endif

#if EU_CS_VAL & (1 << EU_PEAK_BIT_RATE_CONTROL)         
		//-------------------------------------------//
		//-		CS9: 	Peak bit rate Control		-//
		//-------------------------------------------//
        case EU_PEAK_BIT_RATE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_PEAK_BIT_RATE_CFG, gl_EU_PeakBitRate);
            break;
#endif            

#if EU_CS_VAL & (1 << EU_QUANTIZATION_PARAMS_CONTROL)         
		//-------------------------------------------//
		//-		CS10: Quantization Parameter CTRL	-//
		//-------------------------------------------//
        case EU_QUANTIZATION_PARAMS_CONTROL:
            usb_vc_cmd_cfg(req, &EU_QUANTIZATION_PARAMS_CFG, (MMP_ULONG)&gs_EU_QuantParam);
            break;
#endif            


#if EU_CS_VAL & (1 << EU_PRIORITY_ID_CONTROL)         
        case EU_PRIORITY_ID_CONTROL:
            usb_vc_cmd_cfg(req, &EU_PRIORITY_ID_CTRL_CFG, gb_EU_PriorityId);
            break;
#endif
#if EU_CS_VAL & (1 << EU_SYNC_REF_FRAME_CONTROL)         
        case EU_SYNC_REF_FRAME_CONTROL:
			size_override = 1;
            usb_vc_cmd_cfg(req, &EU_SYNC_MODE_CFG, (MMP_ULONG)&gs_EU_Sync_RefFrm);
			size_override = 0;
            break;
#endif

#if EU_CS_VAL & (1 << EU_START_OR_STOP_LAYER_CONTROL)         
		//-------------------------------------------//
		//-		CS16: Start/Stop Layer Control		-//
		//-------------------------------------------//
        case EU_START_OR_STOP_LAYER_CONTROL:
            usb_vc_cmd_cfg(req, &EU_START_STOP_LAYER_CTRL_CFG, gb_EU_StartLayerCtrl);
            break;
#endif
#if EU_CS_VAL & (1 << EU_LEVEL_IDC_LIMIT_CONTROL )
        case EU_LEVEL_IDC_LIMIT_CONTROL:
            usb_vc_cmd_cfg(req, &EU_LEVEL_IDC_LIMIT_CTRL_CFG, gb_EU_LevelIDCCtrl);
            break;
#endif
#if EU_CS_VAL & (1 << EU_QP_RANGE_CONTROL )
        case EU_QP_RANGE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_QP_RANGE_CTRL_CFG, gs_EU_QpRangeCtrl);
            break;
#endif
#if EU_CS_VAL & (1 << EU_SEI_PAYLOADTYPE_CONTROL )
        case EU_SEI_PAYLOADTYPE_CONTROL:
            usb_vc_cmd_cfg(req, &EU_SEI_PAYLOAD_TYPE_CTRL_CFG, (MMP_ULONG)gb_EU_SEIPayloadTypeCtrl);
            break;
#endif

unsupport_eu:        
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_CONTROL;
            break;
    }
}
#endif			//#if (ENCODING_UNIT == 1)

#define EU_ISP_CMD_OK      0x00
#define EU_ISP_CMD_NOT_SUPPORT 0x80
/*
#define EU1_CMD1_MIN (0x0)
#define EU1_CMD1_MAX (0xFFFFFF)
#define EU1_CMD1_DEF (0x341234)

#define EU1_CMD2_MIN (0x0)
#define EU1_CMD2_MAX (0xFFFF)
#define EU1_CMD2_DEF (0x2030)

#define EU1_CMD3_LEN 0x10   //16bytes
MMP_ULONG eu1_cmd1_value;
MMP_USHORT eu1_cmd2_value;
*/
#define EU0_CMD0_SKYPE_LASTERR     (0x00)
#define     EU0_CMD0_LEN                  (0x0002)           
#define     EU0_CMD0_MIN                  (0x0000)           
#define     EU0_CMD0_MAX                  (0x0003)           
#define     EU0_CMD0_DEF                  (0x0000)           
#define     EU0_CMD0_RES                  (0x0001)           

#define EU0_CMD1_SKYPE_ENCRES      (0x01)
#define     EU0_CMD1_LEN                  (0x0001)           
#define     EU0_CMD1_MIN                  (0x01)           
#define     EU0_CMD1_MAX                  (0x0a)           
#define     EU0_CMD1_DEF                  (0x0a)           
#define     EU0_CMD1_RES                  (0x01)           

#define EU0_CMD2_SKYPE_FRAMERATE   (0x02)
#define     EU0_CMD2_LEN                  (0x0001)           
#define     EU0_CMD2_MIN                  (0x01)           
#define     EU0_CMD2_MAX                  (0x1e)           
#define     EU0_CMD2_DEF                  (0x1e)           
#define     EU0_CMD2_RES                  (0x01)           

#define EU0_CMD3_SKYPE_BITRATE     (0x03)
#define     EU0_CMD3_LEN                  (0x0002)           
#define     EU0_CMD3_MIN                  (0x0032)           
#define     EU0_CMD3_MAX                  (0x0bb8)           
#define     EU0_CMD3_DEF                  (0x01f4)           
#define     EU0_CMD3_RES                  (0x0032)           

#define EU0_CMD4_SKYPE_ENFORCEKEY  (0x04)
#define     EU0_CMD4_LEN                  (0x0001)           
#define     EU0_CMD4_MIN                  (0x00)           
#define     EU0_CMD4_MAX                  (0x01)           
#define     EU0_CMD4_DEF                  (0x00)           
#define     EU0_CMD4_RES                  (0x01)           

#define EU0_CMD5_SKYPE_MINQP       (0x05)
#define     EU0_CMD5_LEN                  (0x0001)           
#define     EU0_CMD5_MIN                  (0x00)           
#define     EU0_CMD5_MAX                  (0x33)           
#define     EU0_CMD5_DEF                  (0x1e)           
#define     EU0_CMD5_RES                  (0x01)           

#define EU0_CMD6_SKYPE_MAXQP       (0x06)
#define     EU0_CMD6_LEN                  (0x0001)           
#define     EU0_CMD6_MIN                  (0x00)           
#define     EU0_CMD6_MAX                  (0x33)           
#define     EU0_CMD6_DEF                  (0x32)           
#define     EU0_CMD6_RES                  (0x01)           

#define EU0_CMD7_SKYPE_CABAC       (0x07)
#define     EU0_CMD7_LEN                  (0x0001)           
#define     EU0_CMD7_MIN                  (0x00)           
#define     EU0_CMD7_MAX                  (0x01)           
#define     EU0_CMD7_DEF                  (0x00)           
#define     EU0_CMD7_RES                  (0x01)           

#define EU0_CMDa_SKYPE_FWDAYS      (0x0a)
#define     EU0_CMDa_LEN                  (0x0002)           
#define     EU0_CMDa_MIN                  (0x0F00)           
#define     EU0_CMDa_MAX                  (0xFFFF)           
#define     EU0_CMDa_DEF                  (0x0F00)           
#define     EU0_CMDa_RES                  (0x0001)           

#define EU0_CMDb_SKYPE_PROFILE     (0x0b)
#define     EU0_CMDb_LEN                  (0x0002)           
#define     EU0_CMDb_MIN                  (0x0000)           
#define     EU0_CMDb_MAX                  (0xFFFF)           
#define     EU0_CMDb_DEF                  (0x0000)           
#define     EU0_CMDb_RES                  (0x0001)           

#define EU0_CMDc_SKYPE_LEVEL       (0x0c)
#define     EU0_CMDc_LEN                  (0x0001)           
#define     EU0_CMDc_MIN                  (0x0A)           
#define     EU0_CMDc_MAX                  (0x33)           
#define     EU0_CMDc_DEF                  (0x00)           
#define     EU0_CMDc_RES                  (0x01)           

#define EU0_CMDd_SKYPE_SUPPROFILE  (0x0d)
#define     EU0_CMDd_LEN                  (0x0002)           
#define     EU0_CMDd_MIN                  (0x0000)           
#define     EU0_CMDd_MAX                  (0xFFFF)           
#define     EU0_CMDd_DEF                  (0x0000)           
#define     EU0_CMDd_RES                  (0x0001)           

#define EU0_CMDe_SKYPE_UVCVER      (0x0e)
#define     EU0_CMDe_LEN                  (0x0001)           
#define     EU0_CMDe_MIN                  (0x10)           
#define     EU0_CMDe_MAX                  (0x12)           
#define     EU0_CMDe_DEF                  (0xFF)           
#define     EU0_CMDe_RES                  (0x01)           

#define EU0_CMDf_SKYPE_MODE        (0x0f)
#define     EU0_CMDf_LEN                  (0x0001)           
#define     EU0_CMDf_MIN                  (0x01)           
#define     EU0_CMDf_MAX                  (0x04)           
#define     EU0_CMDf_DEF                  (0x01)           
#define     EU0_CMDf_RES                  (0x01)           


//20091225
#define EU1_SET_ISP             (0x1)   //command number
#define EU1_SET_ISP_LEN         (0x08)
#define EU1_SET_ISP_MIN         (0x0)
#define EU1_SET_ISP_MAX         (0xFFFFFFFFFFFFFFFF)
#define EU1_SET_ISP_DEF         (0x0000000000000000)

#define EU1_GET_ISP_RESULT      (0x2)
#define EU1_GET_ISP_RESULT_LEN  (0x08)
#define EU1_GET_ISP_RESULT_MIN  (0x0)
#define EU1_GET_ISP_RESULT_MAX  (0xFFFFFFFFFFFFFFFF)
#define EU1_GET_ISP_RESULT_DEF  (0x0000000000000000)

#define EU1_SET_FW_DATA         (0x03)
#define EU1_SET_FW_DATA_LEN     (0x20)      //32bytes
//#define EU1_SET_FW_DATA_MIN   (0x00)
//#define EU1_SET_FW_DATA_MAX   (0xFF)
//#define EU1_SET_FW_DATA_DEF   (0x00)

#define EU1_SET_MMP             (0x04)  //command number
#define EU1_SET_MMP_LEN         (0x08)
#define EU1_SET_MMP_MIN         (0x0)
#define EU1_SET_MMP_MAX         (0xFFFFFFFFFFFFFFFF)
#define EU1_SET_MMP_DEF         (0x0000000000000000)

#define EU1_GET_MMP_RESULT      (0x5)
#define EU1_GET_MMP_RESULT_LEN  (0x08)
#define EU1_GET_MMP_RESULT_MIN  (0x0)
#define EU1_GET_MMP_RESULT_MAX  (0xFFFFFFFFFFFFFFFF)
#define EU1_GET_MMP_RESULT_DEF  (0x0000000000000000)

#define EU1_SET_ISP_EX			(0x6)
#define EU1_SET_ISP_EX_LEN      (0x10)
//#define EU1_SET_ISP_EX_MIN         (0x0)
//#define EU1_SET_ISP_EX_MAX         (0xFFFFFFFFFFFFFFFF)
//#define EU1_SET_ISP_EX_DEF         (0x0000000000000000)

#define EU1_GET_ISP_EX_RESULT      (0x7)
#define EU1_GET_ISP_EX_RESULT_LEN  (0x10)
//#define EU1_GET_ISP_EX_RESULT_MIN  (0x0)
//#define EU1_GET_ISP_EX_RESULT_MAX  (0xFFFFFFFFFFFFFFFF)
//#define EU1_GET_ISP_EX_RESULT_DEF  (0x0000000000000000)

#define EU1_READ_MMP_MEM			(0x08)
#define EU1_READ_MMP_MEM_LEN		(0x10)
//#define EU1_READ_MMP_MEM_MIN         (0x0)
//#define EU1_READ_MMP_MEM_MAX         (0xFFFFFFFFFFFFFFFF)
//#define EU1_READ_MMP_MEM_DEF         (0x0000000000000000)

#define EU1_WRITE_MMP_MEM			(0x09)
#define EU1_WRITE_MMP_MEM_LEN		(0x10)
//#define EU1_WRITE_MMP_MEM_MIN         (0x0)
//#define EU1_WRITE_MMP_MEM_MAX         (0xFFFFFFFFFFFFFFFF)
//#define EU1_WRITE_MMP_MEM_DEF         (0x0000000000000000)

#define EU1_GET_CHIP_INFO      (0xA)
#define EU1_GET_CHIP_INFO_LEN  (0x10)
#define EU1_GET_CHIP_INFO_MIN  (0x0)
#define EU1_GET_CHIP_INFO_MAX  (0xFFFFFFFFFFFFFFFF)
#define EU1_GET_CHIP_INFO_DEF  (0x0000000000000000)

#define EU1_GET_DATA_32			(0x0B)
#define EU1_GET_DATA_32_LEN		(0x20)

#define EU1_SET_DATA_32			(0x0C)
#define EU1_SET_DATA_32_LEN		(0x20)

#define EU1_SET_MMP_CMD16      (0xE)
#define EU1_SET_MMP_CMD16_LEN  (0x10)
#define EU1_SET_MMP_CMD16_MIN  (0x0)
#define EU1_SET_MMP_CMD16_MAX  (0xFFFFFFFFFFFFFFFF)
#define EU1_SET_MMP_CMD16_DEF  (0x0000000000000000)

#define EU1_GET_MMP_CMD16_RESULT      (0xF)
#define EU1_GET_MMP_CMD16_RESULT_LEN  (0x10)
#define EU1_GET_MMP_CMD16_RESULT_MIN  (0x0)
#define EU1_GET_MMP_CMD16_RESULT_MAX  (0xFFFFFFFFFFFFFFFF)
#define EU1_GET_MMP_CMD16_RESULT_DEF  (0x0000000000000000)

#define EU1_ACCESS_CUSTOMER_DATA (0x10)
#define EU1_ACCESS_CUSTOMER_DATA_LEN    (0x20)
//#define EU1_ACCESS_CUSTOMER_DATA_MIN    
//#define EU1_ACCESS_CUSTOMER_DATA_MAX    
//#define EU1_ACCESS_CUSTOMER_DATA_DEF    
#define EU1_ACCESS_CUSTOMER_DATA_RES    (0x01)


MMP_UBYTE eu1_set_isp_val[EU1_SET_MMP_LEN];
MMP_UBYTE eu1_get_isp_result_val[EU1_GET_ISP_RESULT_LEN];
MMP_UBYTE eu1_set_mmp_val[EU1_SET_MMP_LEN];
MMP_UBYTE eu1_get_mmp_result_val[EU1_GET_MMP_RESULT_LEN];
MMP_UBYTE eu1_set_isp_ex_val[EU1_SET_ISP_EX_LEN];
MMP_UBYTE eu1_get_isp_ex_result_val[EU1_GET_ISP_EX_RESULT_LEN];
MMP_UBYTE eu1_get_chip_info_val[EU1_GET_CHIP_INFO_LEN];
MMP_UBYTE eu1_set_mmp_cmd16_val[EU1_SET_MMP_CMD16_LEN];
MMP_UBYTE eu1_get_mmp_cmd16_result_val[EU1_GET_MMP_CMD16_RESULT_LEN];

//ISP IQTOOL
#define ISPIQ_GET_PREVIEW_GAIN_ID	0x00
#define ISPIQ_GET_CAPTURE_GAIN_ID	0x01
#define ISPIQ_GET_PREVIEW_ENGERY_ID	0x02
#define ISPIQ_GET_CAPTURE_ENGERY_ID	0x03
#define ISPIQ_SET_AWB_MODE			0x04
#define ISPIQ_GET_PREVIEW_TEMP_ID	0x05
#define ISPIQ_GET_CAPTURE_TEMP_ID	0x06
#define ISPIQ_SELECT_DBG_TABLE		0x07
#define ISPIQ_SET_DBG_TABLE_BY_INDEX  0x08
#define ISPIQ_GET_DBG_TBL_ROWS		0x09
#define ISPIQ_GET_DBG_TBL_COLS		0x0A
#define ISPIQ_GET_DBG_TBL_TYPE		0x0B
#define ISPIQ_GET_DBG_TBL_MODE		0x0C
#define ISPIQ_GET_DBG_TBL_VALUE		0x0D
#define ISPIQ_GET_DBL_TBL_START_ADDR  0x0E
#define ISPIQ_GET_DBG_TBL_APTOOL_ADDR 0x0F
#define ISPIQ_GET_DBL_TBL_TOTAL_SIZE  0x10
#define ISPIQ_WRITE_MEM_PARAM	  	0x11
#define ISPIQ_WRITE_MEM				0x12
#define ISPIQ_READ_MEM_PARAM	  	0x13
#define ISPIQ_READ_MEM				0x14
#define ISPIQ_MOVE_MEM 				0x15
#define ISPIQ_SET_AE_ISO			0x16
#define ISPIQ_GET_AWB_GAIN			0x17
#define ISPIQ_GET_LIGHT_COND_VAL	0x18
#define ISPIQ_GET_AE_ENERGY			0x19
#define ISPIQ_SET_IQ_SWITCH			0x20
#define ISPIQ_GET_AE_BASE_SHUTTER	0x21

//add by casio
#define ISPIQ_SET_COLOR_TRANSFORM	0x23

MMP_ULONG WriteMemAddr = 0;
MMP_ULONG WriteMemCount = 0;
MMP_ULONG ReadMemAddr = 0;
MMP_ULONG ReadMemCount = 0;



#define EU2_CMD1_MIN (0x08)
#define EU2_CMD1_MAX (0x80)
#define EU2_CMD1_DEF (0x5a)
MMP_UBYTE  eu2_cmd1_value;

#if USB_UVC_SKYPE
//
// Skype XU 0
//

extern VC_CMD_CFG SKYPE_VERSION_CFG ;
extern VC_CMD_CFG SKYPE_LASTERROR_CFG ;
extern VC_CMD_CFG SKYPE_FWDAYS_CFG  ;
extern VC_CMD_CFG SKYPE_STREAMID_CFG;
extern VC_CMD_CFG SKYPE_ENDPOINT_CFG;
extern VC_CMD_CFG SKYPE_VIDEO_CFG ;
extern VC_CMD_CFG SKYPE_BITRATE_CFG ;
extern VC_CMD_CFG SKYPE_FRAMERATE_CFG;
extern VC_CMD_CFG SKYPE_KEYFRAME_CFG ;


extern SKYPE_VIDEO_CONFIG gsSkypeProbeCfgCur[] , gsSkypeProbeCfgMin[],gsSkypeProbeCfgMax[] ;
extern SkypeH264Error gbSkypeLastError;
extern MMP_ULONG glSkypeFrameInterval;
extern SkypeH264StreamID gbSkypeStreamID ;
extern SkypeH264EndPointType gbSkypeEndpoint;
extern MMP_ULONG glSkypeBitrate ;
extern MMP_UBYTE gbSkypeKeyFrame ;
extern MMP_USHORT gsSkypeFwDays ;

void usb_vc_eu0_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs;
//    MMP_USHORT i;
//    volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   

    cs = (gsUsbCtrlReq.wValue >> 8);

    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
    case XU_SKYPE_VERSION:
        usb_vc_cmd_cfg(req,&SKYPE_VERSION_CFG,SKYPE_VERSION);   
       break; 
    case XU_SKYPE_LASTERROR:
        usb_vc_cmd_cfg(req,&SKYPE_LASTERROR_CFG, gbSkypeLastError );   
        break;
    case XU_SKYPE_FWDAYS:
        usb_vc_cmd_cfg(req,&SKYPE_FWDAYS_CFG, gsSkypeFwDays);   
        break;
    case XU_SKYPE_STREAMID:
        usb_vc_cmd_cfg(req,&SKYPE_STREAMID_CFG, gbSkypeStreamID);   
        break;
    case XU_SKYPE_ENDPOINT:
        usb_vc_cmd_cfg(req,&SKYPE_ENDPOINT_CFG, gbSkypeEndpoint);   
        break;
    case XU_SKYPE_SF_PROBE:
        usb_vc_cmd_cfg(req,&SKYPE_VIDEO_CFG, (MMP_ULONG)&gsSkypeProbeCfgCur[gbSkypeStreamID]);   
        break;
    case XU_SKYPE_SF_COMMIT:
        usb_vc_cmd_cfg(req,&SKYPE_VIDEO_CFG, (MMP_ULONG)&gsSkypeProbeCfgCur[gbSkypeStreamID]);   
        break ;
    case XU_SKYPE_BITRATE:
        usb_vc_cmd_cfg(req,&SKYPE_BITRATE_CFG, glSkypeBitrate );   
        break ;
    case XU_SKYPE_FRAMERATE:
        usb_vc_cmd_cfg(req,&SKYPE_FRAMERATE_CFG, glSkypeFrameInterval);    
        break;
    case XU_SKYPE_KEYFRAME:
        usb_vc_cmd_cfg(req,&SKYPE_KEYFRAME_CFG, gbSkypeKeyFrame); 
        break;
    default:
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;          
        break;
    }
}
 
#endif

//
// XU1 : AIT internal ISP / MMP control
//
VC_CMD_CFG VC_XU_SET_ISP_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_ISP_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_GET_ISP_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_GET_SUPPORT),
    EU1_GET_ISP_RESULT_LEN,1,
    0,0,0,0,0
} ;


VC_CMD_CFG VC_XU_SET_FW_DATA_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_FW_DATA_LEN,1,
    0,0,0,0,0
} ;


VC_CMD_CFG VC_XU_SET_MMP_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_MMP_LEN,1,
    0,0,0,0,0
} ;


VC_CMD_CFG VC_XU_GET_MMP_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_CUR_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_GET_SUPPORT),
    EU1_GET_MMP_RESULT_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_SET_ISP_EX_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_ISP_EX_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_GET_ISP_EX_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_GET_SUPPORT),
    EU1_GET_ISP_EX_RESULT_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_READ_MMP_MEM_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_CUR_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_GET_SUPPORT),
    EU1_READ_MMP_MEM_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_SET_MMP_MEM_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_WRITE_MMP_MEM_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_ACCESS_CUSTOMER_DATA_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_ACCESS_CUSTOMER_DATA_LEN,1,
    0,0,0,0,0
} ;
//#define CUSTOMER_INIT_FLASH_ADDR   IQ_SETTING_ADDR_IN_FLASH //(0x1e000)
//#define CUSTOMER_INIT_FLASH_SIZE   (0x1000)
//#define SIF_SECTOR_SIZE 1024*4	//4K 
//MMP_BYTE  gbNVMCmd = 0;
MMP_USHORT gsNVMAddr = 0;
MMP_BYTE CustomDataRwBuf[EU1_ACCESS_CUSTOMER_DATA_LEN];


VC_CMD_CFG VC_XU_GET_CHIP_INFO_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_CUR_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_GET_SUPPORT),
    EU1_GET_CHIP_INFO_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_GET_DATA_32_CFG = {
    ( CAP_GET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_GET_DATA_32_LEN,1,
    0,0,0,0,0
} ;

VC_CMD_CFG VC_XU_SET_DATA_32_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD | CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_DATA_32_LEN,1,
    0,0,0,0,0
};

VC_CMD_CFG VC_XU_SET_MMP_CMD16_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT),
    EU1_SET_MMP_CMD16_LEN,1,
    0,0,0,0,0
} ;

DataExchangeParam gDEParam = {0,0,0,0};

VC_CMD_CFG VC_XU_GET_MMP_CMD16_RESULT_CFG = {
    ( CAP_SET_CUR_CMD | CAP_GET_INFO_CMD |CAP_GET_CUR_CMD |CAP_GET_DEF_CMD | CAP_GET_MIN_CMD | CAP_GET_MAX_CMD | CAP_GET_RES_CMD | CAP_GET_LEN_CMD),
    ( INFO_SET_SUPPORT|INFO_GET_SUPPORT ),
    EU1_GET_MMP_CMD16_RESULT_LEN,1,
    0,0,0,0,0
} ;

extern MMP_UBYTE IQID;		// AlexH @ 2020/08/25


void usb_vc_eu1_cs(MMP_UBYTE req)
{
    MMP_UBYTE cs,cap;
//    MMP_USHORT i;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;   

    cs = (/*UsbRequestPayload_wValue */gsUsbCtrlReq.wValue>> 8);


    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);

    switch(cs) {
    case EU1_SET_ISP:
    {
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_SET_ISP\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_SET_ISP_CFG, 0);
        break; 
    }
    case EU1_GET_ISP_RESULT:
    {
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_GET_ISP\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_GET_ISP_CFG, (MMP_ULONG)eu1_get_isp_result_val);
        break; 
    }
       //Get ISP Api result 
    case EU1_SET_FW_DATA:
    {
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        VC_XU_SET_ISP_CFG.dwCurVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_SET_FW_DATA\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_SET_FW_DATA_CFG, 0);
        break; 
    }
    case EU1_SET_MMP:
    {
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwMinVal = 0;
        VC_XU_SET_ISP_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        
        //RTNA_DBG_Str(0,"EU1_SET_MMP\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_SET_MMP_CFG, 0);
        break; 
    }
    case EU1_GET_MMP_RESULT:
    {
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_GET_MMP_RESULT\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_GET_MMP_CFG, (MMP_ULONG)eu1_get_mmp_result_val);
        break; 
    }
    case EU1_SET_ISP_EX:
    {
        VC_XU_SET_ISP_EX_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_EX_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal
        VC_XU_SET_ISP_EX_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_SET_ISP_EX\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_SET_ISP_EX_CFG, 0);
        break; 
    }
    case EU1_GET_ISP_EX_RESULT:
    {
        VC_XU_SET_ISP_EX_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_EX_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_ISP_EX_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        //RTNA_DBG_Str(0,"EU1_GET_ISP_EX\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_GET_ISP_EX_CFG, (MMP_ULONG)eu1_get_isp_ex_result_val);
        break; 
    }

   	case EU1_READ_MMP_MEM:
   	{
   	#if 0
        MMP_USHORT i;
        
        //get isp command 16 BYTES
        switch(req) {
        //case SET_CUR_CMD:
        //    USBOutPhase = req;
        //    USBOutData = 1;           
        //    break;  
        case GET_CUR_CMD:
        {
        	MMP_UBYTE n;
			MMP_UBYTE rlen;
            USBInPhase = req;
            USBEP0TxCnt = EU1_READ_MMP_MEM_LEN;
            
			if(ReadMemCount>EU1_READ_MMP_MEM_LEN)
				rlen = EU1_READ_MMP_MEM_LEN;
			else
				rlen = ReadMemCount;
			
			ReadMemCount -= rlen;
			
			for(n=0;n<16;++n)
			{
				  //if(n>=0 && n<8)
				  //	pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = n;//+0xA0;
				  //else
				  //	pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = ((MMP_BYTE*)ReadMemAddr)[n];
				  
				  if(n<rlen)
				        pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = ((MMP_BYTE*)ReadMemAddr)[n];
				  else
				  	pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0;
				  
			}
			ReadMemAddr += rlen;			
            //RTNA_DBG_Str(0,"Read MMP MEM\r\n");
            usb_ep0_in();
            break;
        }
        case GET_INFO_CMD:
            USBInPhase = req;
            cap = INFO_GET_SUPPORT;//INFO_SET_SUPPORT;//INFO_GET_SUPPORT;//|INFO_SET_SUPPORT;
            USBEP0TxCnt = 0x01;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = cap;
            usb_ep0_in();
            break;
        case GET_DEF_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_READ_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_READ_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;
            usb_ep0_in();
            break;
        case GET_MIN_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_READ_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_READ_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;
            usb_ep0_in();
            break;
        case GET_MAX_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_READ_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_READ_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0xFF;
            usb_ep0_in();
            break;
        case GET_RES_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_READ_MMP_MEM_LEN;//0x2;//0x03;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x01;
            for(i = 0;i < EU1_GET_ISP_EX_RESULT_LEN - 1;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;             
            usb_ep0_in();
            break;
        case GET_LEN_CMD:
            USBInPhase = req;
            USBEP0TxCnt = 0x02;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = EU1_READ_MMP_MEM_LEN & 0xFF;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = (EU1_READ_MMP_MEM_LEN >> 8) & 0xFF;
            usb_ep0_in();             
            break;
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;     
            break;
        }      
	#else
		MMP_UBYTE buf[EU1_READ_MMP_MEM_LEN];
		if(req == GET_CUR_CMD)
		{
        	MMP_UBYTE n;
			MMP_UBYTE rlen;
            USBInPhase = req;
            
			if(ReadMemCount>EU1_READ_MMP_MEM_LEN)
				rlen = EU1_READ_MMP_MEM_LEN;
			else
				rlen = ReadMemCount;
			
			ReadMemCount -= rlen;
			for(n=0;n<16;++n)
			{
				  if(n<rlen)
				        buf[n] = ((MMP_BYTE*)ReadMemAddr)[n];
				  else
				  	buf[n] = 0;
				  
			}
			ReadMemAddr += rlen;					
		}
		VC_XU_READ_MMP_MEM_CFG.dwMaxVal = 0;
		VC_XU_READ_MMP_MEM_CFG.dwMinVal = 0;
		VC_XU_READ_MMP_MEM_CFG.dwResVal = 0;
        usb_vc_cmd_cfg(req,&VC_XU_READ_MMP_MEM_CFG, (MMP_ULONG)buf);				
	#endif
        break;
    }    
    case EU1_WRITE_MMP_MEM:
    {
    #if 0
        MMP_USHORT i;
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;           
            break;  
        case GET_INFO_CMD:
            USBInPhase = req;
            cap = INFO_SET_SUPPORT;//INFO_GET_SUPPORT;//|INFO_SET_SUPPORT;
            USBEP0TxCnt = 0x01;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = cap;
            usb_ep0_in();
            break;
        case GET_DEF_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_WRITE_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_WRITE_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;
            usb_ep0_in();
            break;
        case GET_MIN_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_WRITE_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_WRITE_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;
            usb_ep0_in();
            break;
        case GET_MAX_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_WRITE_MMP_MEM_LEN;//0x2;//0x03;
            for(i = 0;i < EU1_WRITE_MMP_MEM_LEN;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0xFF;
            usb_ep0_in();
            break;
        case GET_RES_CMD:
            USBInPhase = req;
            USBEP0TxCnt = EU1_WRITE_MMP_MEM_LEN;//0x2;//0x03;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x01;
            for(i = 0;i < EU1_WRITE_MMP_MEM_LEN - 1;++i)
                pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = 0x00;             
            usb_ep0_in();
            break;
        case GET_LEN_CMD:
            USBInPhase = req;
            USBEP0TxCnt = 0x02;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = EU1_WRITE_MMP_MEM_LEN & 0xFF;
            pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B = (EU1_WRITE_MMP_MEM_LEN >> 8) & 0xFF;
            usb_ep0_in();             
            break;
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;     
            break;
        }      
	#else
		VC_XU_SET_MMP_MEM_CFG.dwMaxVal = 0;
		VC_XU_SET_MMP_MEM_CFG.dwMinVal = 0;
		VC_XU_SET_MMP_MEM_CFG.dwResVal = 0;
        usb_vc_cmd_cfg(req,&VC_XU_READ_MMP_MEM_CFG, 0);		
	#endif      
        break;
    }

    case EU1_GET_CHIP_INFO:
    {
        MMP_UBYTE i ;
        
        VC_XU_GET_CHIP_INFO_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_GET_CHIP_INFO_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_GET_CHIP_INFO_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        
        
        //RTNA_DBG_Str(0,"EU1_GET_CHIP_INFO\r\n");
        eu1_get_chip_info_val[0] = '8';
        eu1_get_chip_info_val[1] = '4';
        eu1_get_chip_info_val[2] = '5';
        eu1_get_chip_info_val[3] = '5';
        eu1_get_chip_info_val[4] = 0;
        eu1_get_chip_info_val[5] = 0;;
        eu1_get_chip_info_val[6] = 0;
        eu1_get_chip_info_val[7] = 0;
        eu1_get_chip_info_val[8] = 0;
        eu1_get_chip_info_val[9] = 0;
        eu1_get_chip_info_val[10] = 0;
        eu1_get_chip_info_val[11] = 0;
        eu1_get_chip_info_val[12] = 0;
        eu1_get_chip_info_val[13] = 0;
        eu1_get_chip_info_val[14] = 0;
        eu1_get_chip_info_val[15] = 0;
        usb_vc_cmd_cfg(req,&VC_XU_GET_CHIP_INFO_CFG, (MMP_ULONG)&eu1_get_chip_info_val[0]);
        break; 
    }

   	case EU1_GET_DATA_32:
    {
        
        MMP_UBYTE curVal[EU1_GET_DATA_32_LEN];		
        
        VC_XU_GET_DATA_32_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_GET_DATA_32_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_GET_DATA_32_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
		VC_XU_GET_DATA_32_CFG.dwCurVal = 0;//(MMP_ULONG)curVal ;
		
       
        if(req == GET_CUR_CMD)
        {
			#if 0
        	if(gDEParam.cur_offset < gDEParam.data_len)
        	{
        		MMP_UBYTE i,len;
        		len = (gDEParam.data_len-gDEParam.cur_offset>EU1_GET_DATA_32_LEN)?EU1_GET_DATA_32_LEN:(gDEParam.data_len-gDEParam.cur_offset);
        		for(i=0;i<len;++i)
        		{
        			curVal[i] = gDEParam.ptr[gDEParam.cur_offset+i];
        		}     		
        		gDEParam.cur_offset += len;
        	}
        	#endif
        	
        	//VC_XU_GET_DATA_32_CFG.dwCurVal = (MMP_ULONG)curVal;
        	*(MMP_BYTE *)(SERIAL_NUMBER_READ_BUF_STA+23) = IQID;			// 0x11;   AlexH @ 2020/08/25
			VC_XU_GET_DATA_32_CFG.dwCurVal = SERIAL_NUMBER_READ_BUF_STA;		
		}
		
        usb_vc_cmd_cfg(req,&VC_XU_GET_DATA_32_CFG, VC_XU_GET_DATA_32_CFG.dwCurVal);
        break; 
    }
   	case EU1_SET_DATA_32:
    {
        VC_XU_SET_DATA_32_CFG.dwMaxVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_DATA_32_CFG.dwMinVal = 0;//(MMP_ULONG)maxVal ;
        VC_XU_SET_DATA_32_CFG.dwResVal = 0;//(MMP_ULONG)resVal ;
        VC_XU_SET_DATA_32_CFG.dwCurVal = 0;//(MMP_ULONG)maxVal ;        

        //RTNA_DBG_Str(0,"EU1_SET_FW_DATA\r\n");
        usb_vc_cmd_cfg(req,&VC_XU_SET_DATA_32_CFG, 0);
        break; 
    }

    case EU1_SET_MMP_CMD16:
    {
    #if 0
        MMP_UBYTE maxVal[EU1_SET_MMP_CMD16_LEN] ;
        MMP_UBYTE resVal[EU1_SET_MMP_CMD16_LEN] ;
        MMP_UBYTE i ;
        
        for(i=0;i<EU1_SET_MMP_CMD16_LEN;i++) {
            maxVal[i] = 0xFF ;
            resVal[i] = 0x00 ;
        }
        resVal[0] = 0x01 ;
        VC_XU_SET_MMP_CMD16_CFG.dwMaxVal = (MMP_ULONG)maxVal ;
        VC_XU_SET_MMP_CMD16_CFG.dwResVal = (MMP_ULONG)resVal ;
        usb_vc_cmd_cfg(req,&VC_XU_SET_MMP_CMD16_CFG, 0);
    #else
        VC_XU_SET_MMP_CMD16_CFG.dwMinVal = 0 ;
        VC_XU_SET_MMP_CMD16_CFG.dwMaxVal = 0 ;
        VC_XU_SET_MMP_CMD16_CFG.dwResVal = 0 ;
        usb_vc_cmd_cfg(req,&VC_XU_SET_MMP_CMD16_CFG, 0);    	
    #endif
        break; 
    }
    case EU1_GET_MMP_CMD16_RESULT:
    {
    #if 0
        MMP_UBYTE maxVal[EU1_GET_MMP_CMD16_RESULT_LEN] ;
        MMP_UBYTE resVal[EU1_GET_MMP_CMD16_RESULT_LEN] ;
        MMP_UBYTE i ;
        
        for(i=0;i<EU1_GET_MMP_CMD16_RESULT_LEN;i++) {
            maxVal[i] = 0xFF ;
            resVal[i] = 0x00 ;
        }
        resVal[0] = 0x01 ;
        VC_XU_GET_MMP_CMD16_RESULT_CFG.dwMaxVal = (MMP_ULONG)maxVal ;
        VC_XU_GET_MMP_CMD16_RESULT_CFG.dwResVal = (MMP_ULONG)resVal ;
        usb_vc_cmd_cfg(req,&VC_XU_GET_MMP_CMD16_RESULT_CFG, (MMP_ULONG)eu1_get_mmp_cmd16_result_val);
 	#else
 	   	VC_XU_GET_MMP_CMD16_RESULT_CFG.dwMinVal = 0 ;
        VC_XU_GET_MMP_CMD16_RESULT_CFG.dwMaxVal = 0 ;
        VC_XU_GET_MMP_CMD16_RESULT_CFG.dwResVal = 0 ; 		
        usb_vc_cmd_cfg(req,&VC_XU_GET_MMP_CMD16_RESULT_CFG, (MMP_ULONG)eu1_get_mmp_cmd16_result_val);        
 	#endif
        break; 
    }


    case XU_CONTROL_UNDEFINED:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}    



//
// XU2 : TBD for customer XU
//

void usb_vc_eu2_cs(MMP_UBYTE req)
{

    MMP_UBYTE cs;
//    volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   
//    MMP_UBYTE i;
    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
    switch(cs) {
    #if USB_UVC_H264==1
    case UVCX_RATE_CONTROL_MODE:
        usb_vc_cmd_cfg(req,&UVCX_RATE_CONTROL_MODE_CFG,(MMP_ULONG)&gsUVCXCtlModeCur );
        break ;
    case UVCX_TEMPORAL_SCALE_MODE:
        usb_vc_cmd_cfg(req,&UVCX_TEMPORAL_SCALE_MODE_CFG,(MMP_ULONG)&gsUVCXTemporalScaleMode);
        break ;
    case UVCX_SPATIAL_SCALE_MODE:
        usb_vc_cmd_cfg(req,&UVCX_SPATIAL_SCALE_MODE_CFG,(MMP_ULONG)&gsUVCXSpatialScaleMode);
        break;
    case UVCX_SNR_SCALE_MODE:
        usb_vc_cmd_cfg(req,&UVCX_SNR_SCALE_MODE_CFG,(MMP_ULONG)&gsUVCXSNRScaleMode);
        break;
    case UVCX_LTR_BUFFER_SIZE_CONTROL:
        usb_vc_cmd_cfg(req,&UVCX_LTR_BUFFER_SIZE_CONTROL_CFG,(MMP_ULONG)&gsUVCXLtrBufferSizeCtl);
        break ;     
    case UVCX_LTR_PICTURE_CONTROL:
        usb_vc_cmd_cfg(req,&UVCX_LTR_PICTURE_CONTROL_CFG,(MMP_ULONG)&gsUVCXLtrPictCtl);
        break ;     
    case UVCX_PICTURE_TYPE_CONTROL:
        usb_vc_cmd_cfg(req,&UVCX_PICTURE_TYPE_CONTROL_CFG,(MMP_ULONG)&gsUVCXPicTypeCtl);
        break;
    case UVCX_VERSION:
        usb_vc_cmd_cfg(req,&UVCX_VERSION_CFG,gsUVCXVersion);
        break;
    case UVCX_ENCODER_RESET:
        usb_vc_cmd_cfg(req,&UVCX_VERSION_CFG,gsUVCXEncoderResetId);
        break;
    case UVCX_FRAMERATE_CONFIG:
        usb_vc_cmd_cfg(req,&UVCX_FRAMERATE_CONFIG_CFG,(MMP_ULONG)&gsUVCXFrameRate);
        break;
    case UVCX_VIDEO_ADVANCE_CONFIG: 
        usb_vc_cmd_cfg(req,&UVCX_VIDEO_ADVANCE_CONFIG_CFG,(MMP_ULONG)&gsUVCXAdvConfig);
        break;
    case UVCX_BITRATE_LAYERS:
        usb_vc_cmd_cfg(req,&UVCX_BITRATE_CFG,(MMP_ULONG)&gsUVCXBitRate);
        break;
    case UVCX_QP_STEPS_LAYERS:
        usb_vc_cmd_cfg(req,&UVCX_PICTURE_TYPE_CONTROL_CFG,(MMP_ULONG)&gsUVCXQPStepSize);
        break;
    //case UVCX_CROP_CONFIG:
    //    usb_vc_cmd_cfg(req,&UVCX_CROP_CONFIG_CFG,(MMP_ULONG)&gsUVCXCropConfig);
    //    break;
           
    #endif        
    case XU_CONTROL_UNDEFINED:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}

void usb_vc_vsi_cs(MMP_UBYTE req)//interface control request
{
    MMP_UBYTE cs,cap,if_id;
    MMP_USHORT i;
    MMP_USHORT *tmpptr;
    MMP_UBYTE *tmpptrb;
    MMP_ULONG frameInterval ;
    
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;      

    cs = ( /*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);//control selector
    if_id = ( /*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) ;
    
    UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
#if 0
    dbg_printf(3,"vsi.cs - req : %d,cs :%d, if_id :%x\r\n",req,cs,if_id );
    
#endif
    switch(cs) {
    case VS_PROBE_CONTROL:
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;        
            break;
        case GET_CUR_CMD:
            //USBInPhase = req;
            tmpptr = (MMP_USHORT *) &vp;
            //dbg_printf(3,"VS_PROBE_CONTROL : ** GET_CUR_CMD: gbCurFormatIndex = %d\r\n",gCurVsiInfo[if_id-1].bFormatIndex);
            
            /*--------------------------*/
            //sean@2010_01_07, Get frame from probe set
            /*--------------------------*/
            frameInterval = ( vpin.wIntervalHi << 16 ) | vpin.wIntervalLo ;
            if(frameInterval  > 0) {
                //gbCurFrameRate[if_id-1]    = ( 10000000   / frameInterval); 
                gsCurFrameRatex10[if_id-1] = ( (10000000*10)  / frameInterval); 
            } else {
                //gbCurFrameRate[if_id-1] = 30;
                gsCurFrameRatex10[if_id-1] = 30*10;
            }
            // ooxx test
            //gbCurFrameRate =2 ;
            
            if ( (gCurVsiInfo[if_id-1].bFormatIndex==YUY2_FORMAT_INDEX ) || 
                 (gCurVsiInfo[if_id-1].bFormatIndex==NV12_FORMAT_INDEX)) {
                if(gbUsbHighSpeed) {
                    usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,gsCurFrameRatex10[if_id-1],1); //sean@2010_12_31 , get cur upd = 1
                } else {
                    //if(gbCurFrameRate[if_id-1]>15) {
                    //    gbCurFrameRate[if_id-1] = 15 ;
                    //}
                    if(gsCurFrameRatex10[if_id-1]>150) {
                        gsCurFrameRatex10[if_id-1] = 150 ;
                    }
                    usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,gsCurFrameRatex10[if_id-1],1);
                }
            } else {
                usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,gsCurFrameRatex10[if_id-1],1);
            }
#if USB_UVC_BULK_EP  // tomy@2010_09_01, to fix Skype preview issue via BULK transfer on some linux OS
            gbCheckSystemInitFlag++;
            if(gbCheckSystemInitFlag >= 254)  gbCheckSystemInitFlag = 254;
#endif    
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,2);
            break;
        case GET_MIN_CMD:
            //USBInPhase = req;
            tmpptr = (MMP_USHORT *) &vp;
            
            //dbg_printf(3," ** GET_MIN_CMD: gbCurFormatIndex = %d\r\n",gCurVsiInfo[if_id-1].bFormatIndex);
            
            //usb_vc_get_video_probe(1);
            usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,1,50,0);
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,2);
            break;
        case GET_MAX_CMD:
            //USBInPhase = req;
            tmpptr = (MMP_USHORT *) &vp;
           
            //dbg_printf(3," ** GET_MAX_CMD: gbCurFormatIndex == %d\r\n",gCurVsiInfo[if_id-1].bFormatIndex);
          
            //usb_vc_get_video_probe(2);

         	usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex, PCCAM_RES_NUM,300,0);
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,2);
            break;
        case GET_RES_CMD:
            // under coding
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_UNKNOWN;
            break;
        case GET_LEN_CMD:
            {
                MMP_UBYTE cmd[2] ;
                cmd[0] = 0x1A;
                cmd[1] = 0x00; 
                USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,2,cmd,1);
            }    
            break;
        case GET_INFO_CMD:
            //USBInPhase = req;
            cap = INFO_GET_SUPPORT | INFO_SET_SUPPORT;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&cap,1);
            break;
        case GET_DEF_CMD:
            //USBInPhase = req;
            tmpptr = (MMP_USHORT *) &vp;
           // dbg_printf(3," ** GET_DEF_CMD: gbCurFormatIndex = %d\r\n",gCurVsiInfo[if_id-1].bFormatIndex);
            usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex, PCCAM_640_480+1,300,1);
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,2);
            break;
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;          
            break;
        }        
        break;
    case VS_COMMIT_CONTROL:
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;     
            break;
        case GET_CUR_CMD:
            //USBInPhase = req;
            tmpptr = (MMP_USHORT *) &vp;
            
            //dbg_printf(3," ** GET_DEF_CMD: VS_COMMIT_CONTROL = %d\r\n",gbCurFormatIndex);
            if ((gCurVsiInfo[if_id-1].bFormatIndex==YUY2_FORMAT_INDEX) || 
                (gCurVsiInfo[if_id-1].bFormatIndex==NV12_FORMAT_INDEX)) {
                if(gbUsbHighSpeed) {
                    usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,300,1); //sean@2010_12_31 , get cur upd = 1
                } else {
                    usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,150,1);
                }
            } else {
                usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex,300,1);
            }
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,2);
            break;
        case GET_LEN_CMD:
            {
                MMP_UBYTE cmd[2] ;
                cmd[0] = 0x1A;
                cmd[1] = 0x00; 
                USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,2,cmd,1);
            }    
            
            break;
        case GET_INFO_CMD:
            cap = INFO_GET_SUPPORT | INFO_SET_SUPPORT;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&cap,1);
            break;
        case GET_MIN_CMD:
        case GET_MAX_CMD:
        case GET_RES_CMD:
        case GET_DEF_CMD:
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;
            break;
        }        
        break;
    case VS_STILL_PROBE_CONTROL:
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;        
            break;
        case GET_CUR_CMD:
            tmpptrb = (MMP_UBYTE *) &sp;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptrb,1);
            break;
        case GET_MIN_CMD:
        	{
        		STILL_PROBE temp_sp;
        		usb_vc_get_still_probe(gbCurStillFormatIndex,1,&temp_sp);
        		tmpptrb = (MMP_UBYTE *) &temp_sp;
            	USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptrb,1);
            }           
            break;
        case GET_MAX_CMD:
        /*
            USBInPhase = req;
            tmpptrb = (MMP_UBYTE *) &sp;
            usb_vc_get_still_probe(3);
            USBEP0TxCnt = UsbRequestPayload_wLength;
            for(i = 0;i < USBEP0TxCnt;i++) {
                USB_REG_BASE_B[USB_EP0_FIFO_B] = *(tmpptr + i);
            }
            usb_ep0_in();           
        */
        	{
        		STILL_PROBE temp_sp;        		

                switch(gbCurStillFormatIndex)
                {
            #if ENABLE_YUY2==1
            	    case YUY2_FORMAT_INDEX://YVY2
	                    usb_vc_get_still_probe(gbCurStillFormatIndex,NUM_YUY2_STILL,&temp_sp);
            	    break;
            #endif
            #if ENABLE_MJPEG==1
            	    case MJPEG_FORMAT_INDEX://MJPEG
	                    usb_vc_get_still_probe(gbCurStillFormatIndex,NUM_MJPG_STILL,&temp_sp);
            	    break;
            #endif
                }
	            tmpptrb = (MMP_UBYTE *) &temp_sp;
	            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptrb,1);      		
        	}    
            break;
        case GET_RES_CMD:
            // under coding
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_UNKNOWN;
            break;
        case GET_LEN_CMD:
            {
                MMP_UBYTE cmd[2] ;
                cmd[0] = 0x0B;
                cmd[1] = 0x00; 
                USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,2,cmd,1);
            }    
            break;
        case GET_INFO_CMD:
            cap = INFO_GET_SUPPORT | INFO_SET_SUPPORT;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&cap,1);
            break;
        case GET_DEF_CMD:
        	{
        		STILL_PROBE temp_sp;
        		usb_vc_get_still_probe(1,1,&temp_sp);
	            tmpptrb = (MMP_UBYTE *) &temp_sp;//&vp;
	            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptrb,1);
            }
            break;
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;          
            break;
        }        
        break;
    case VS_STILL_COMMIT_CONTROL:
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;           
            break;
        case GET_CUR_CMD:
            tmpptrb = (MMP_UBYTE *) &sc;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptrb,1);
            break;
        case GET_LEN_CMD:
            {
                MMP_UBYTE cmd[2] ;
                cmd[0] = 0x0B;
                cmd[1] = 0x00; 
                USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,2,cmd,1);
            }    
            break;
        case GET_INFO_CMD:
            cap = INFO_GET_SUPPORT | INFO_SET_SUPPORT;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&cap,1);
            break;
        case GET_MIN_CMD:
        case GET_MAX_CMD:
        case GET_RES_CMD:
        case GET_DEF_CMD:
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;
            break;
        }        
        break;
    case VS_STILL_IMAGE_TRIGGER_CONTROL:
        switch(req) {
        case SET_CUR_CMD:
            USBOutPhase = req;
            USBOutData = 1;          
            break;
        case GET_CUR_CMD:
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&gbTriOP,1);
            break;
        case GET_INFO_CMD:
            cap = INFO_GET_SUPPORT | INFO_SET_SUPPORT;
            USB_PUT_FIFO_DATA(req,MMPF_USB_EP0,1,&cap,1);
            break;
        case GET_MIN_CMD:
        case GET_MAX_CMD:
        case GET_RES_CMD:
        case GET_LEN_CMD:
        case GET_DEF_CMD:
        default:
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL);
            gbVCERRCode = CONTROL_INVALID_REQUEST;
            break;
        }        
        break;
    case VS_CONTROL_UNDEFINED:
    case VS_STREAM_ERROR_CODE_CONTROL:
    case VS_GENERATE_KEY_FRAME_CONTROL:
    case VS_UPDATE_FRAME_SEGMENT_CONTROL:
    case VS_SYNCH_DELAY_CONTROL:
    default:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL);
        gbVCERRCode = CONTROL_INVALID_CONTROL;
        break;
    }
}

void usb_vc_req_proc(MMP_UBYTE req)
{
    MMP_UBYTE id;

    id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex >> 8);
#if 0
    RTNA_DBG_Str3("VC.Req:");
    RTNA_DBG_Byte3(req);
    RTNA_DBG_Str3(",");
 
    RTNA_DBG_Str3("VC.Id:");
    RTNA_DBG_Byte3(id);
    RTNA_DBG_Str3("\r\n");    
#endif
    if(id == 0x00) {//if Hbyte of wIndex is 0 -> interface control request otherwise are Terminal ID or Unit ID
        MMP_UBYTE if_id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) ;
        
        if(if_id == gbVCVSIF1) {
            usb_vc_vsi_cs(req);
        }
        else if(if_id==gbVCVSIF2) {
            usb_vc_vsi_cs(req);
        } else if(if_id == gbVCVCIF) {
            usb_vc_vci_cs(req);
        } else {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_INVALID_UNIT;
        }
    } else if(id == gbVCITID) {
        usb_vc_ct_cs(req);
    } 
#if 0    
    else if(id == gbVCOTID) {
    }
    else if(id == gbVCSUID) {
        usb_vc_su_cs(req);
    }
#endif          
    else if(id == gbVCPUID) {
        usb_vc_pu_cs(req);
    }
#if USB_UVC_SKYPE
    else if(id == gbVCEU0ID) {
        usb_vc_eu0_cs(req);
    } 
#endif
    else if(id == gbVCEU1ID) {
        usb_vc_eu1_cs(req);
    } 
    else if(id == gbVCEU2ID) {
        usb_vc_eu2_cs(req);
    }
    else {
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbVCERRCode = CONTROL_INVALID_UNIT;
    }
}

void usb_vc_vci_cs_out(void)
{
    MMP_UBYTE cs;

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    switch(cs) {
    case VC_VIDEO_POWER_MODE_CONTROL:
        // under coding
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbVCERRCode = CONTROL_UNKNOWN;
        break;
    case VC_REQUEST_ERROR_CODE_CONTROL:
        // un-support
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbVCERRCode = CONTROL_INVALID_REQUEST;
        break;
    case VC_CONTROL_UNDEFINED:
    case VC_RESERVED:
    default:
        // impossible flow      
        break;
    }
}

void usb_vc_su_cs_out(void)
{
    MMP_UBYTE cs,tmp;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);

    switch(cs) {
    case SU_INPUT_SELECT_CONTROL:
        USBInPhase = SET_CUR_CMD;
        USBOutData = 0;           
        tmp = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        if(tmp != 1) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_OUT_OF_RANGE;
        } else {
            //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
            usb_ep0_null_in();
        }
        break;
    case SU_CONTROL_UNDEFINED:
    default:
        // impossible flow
        break;
    }
}

void usb_vc_ct_cs_out(void)
{
static MMP_BOOL gbAEVal ;
    MMP_UBYTE cs,len,tmp;
    MMP_LONG pan,tilt;
    MMP_USHORT tmp2;
    MMP_ULONG tmp4;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;

    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;

    //VAR_B(0,cs);
    
    switch(cs) {
#if CT_CS_VAL & (1 << CT_FOCUS_SIMPLE_CONTROL )
        case CT_FOCUS_SIMPLE_CONTROL:
            USBOutData = 0 ;
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,&tmp,sizeof(tmp)) ;
            if( tmp > SIMPLE_FOCUS_MAX) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE; 
                break;    
            } else {
                gbSimpleFocus = tmp ;
                // TBD
                USB_SetSimpleFocus(PCAM_NONBLOCKING,gbSimpleFocus) ;
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in(); 
        break;
#endif      
#if CT_CS_VAL & (1 << CT_AE_MODE_CONTROL)         
        case CT_AE_MODE_CONTROL:
            USBOutData = 0;  
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,&tmp, sizeof(tmp) );
            //VAR_B(0,tmp);
            // only one bit set
            if( (tmp != AEMODE_MANUAL) && ( tmp!=AEMODE_APERTURE )) {
           //if( (tmp==0) || ( tmp & ~CT_AEMODE_CFG.dwResVal ) != 0 ) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE;  
                break;
                                    
            } else {
                gbAE = tmp ;
                //sean@2011_01_24, fixed bug.
                if(gbAE==AEMODE_MANUAL) {
                    gbAEVal = MMP_FALSE ;
                } else {
                    gbAEVal = MMP_TRUE ;
                }
                //sean@2011_03_25, using overwrite
                USB_EnableAE(PCAM_NONBLOCKING,gbAEVal);
                
            }
            if(gbAE==AEMODE_APERTURE) { // Auto Exposure
                gbEVInfo=INFO_AUTOUPDATE_CONTROL_SUPPORT|INFO_AUTO_MODE_SUPPORT|INFO_GET_SUPPORT|INFO_SET_SUPPORT;
            }          
            else {
                gbEVInfo=INFO_AUTOUPDATE_CONTROL_SUPPORT|INFO_GET_SUPPORT|INFO_SET_SUPPORT;
            }
           // VAR_B(0,gbEVInfo);

               
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            
            if(gbAE!=gblastAE)
                gbUpdAE=1;
            gblastAE=gbAE;
            break;
#endif  

#if CT_CS_VAL & (1 << CT_AE_PRIORITY_CONTROL)         
        case CT_AE_PRIORITY_CONTROL:
            USBOutData = 0;  
            //tmp = USB_REG_BASE_B[USB_EP0_FIFO_B];
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,&tmp, sizeof(tmp) );
            if(tmp > AEPRIORITY_MAX) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE;  
                break;
            } else {
                gbAEPriority = tmp ;
                if(gbAE==AEMODE_APERTURE) { // Auto Exposure
                    USB_SetAEPriority(PCAM_NONBLOCKING,gbAEPriority);    
                } 
            }  
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif
          
#if CT_CS_VAL & (1 << CT_EXPOSURE_TIME_ABSOLUTE_CONTROL)         
        case CT_EXPOSURE_TIME_ABSOLUTE_CONTROL:
            // under coding
            USBOutData = 0;  
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len, (MMP_UBYTE *)&tmp4, sizeof(tmp4) );
            if(gbAE==AEMODE_APERTURE) {
                RTNA_DBG_Str0("Set EXP time in AE,stall\r\n");
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_WRONG_STATE;
                break;
            }
            
            if(tmp4>EXPOSURE_MAX||tmp4<EXPOSURE_MIN) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE;                      
                break;
            }
            else
            {
                glEV = tmp4;
                VAR_L(3,glEV);
                // sean@2011_01_24
                USB_SetExposureTime(PCAM_NONBLOCKING,glEV);
                
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
          break;
#endif
#if CT_CS_VAL & (1 << CT_FOCUS_ABSOLUTE_CONTROL)         
          
        case CT_FOCUS_ABSOLUTE_CONTROL:
            USBOutData = 0;
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len, (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            //dbg_printf(3,"focus_abs.out :%d,%d\r\n",gbAutoFocus,tmp2);
            // If AF is on, stall
            if(gbAutoFocus==1) {
                RTNA_DBG_Str0("Set Focus in AF,stall\r\n");
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_WRONG_STATE;
                break;
            }
                
            if(tmp2<AAF_MIN || tmp2>AAF_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                gsAFocus = tmp2;
                //VAR_W(0,gsAFocus);
                USB_SetLensPosition(PCAM_NONBLOCKING,gsAFocus);
            }             
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif
#if CT_CS_VAL & (1 << CT_FOCUS_AUTO_CONTROL)         
        case CT_FOCUS_AUTO_CONTROL:
             {
                MMP_BOOL tmp;
                USBOutData = 0;           
                USB_GET_FIFO_DATA(MMPF_USB_EP0,len,&tmp, sizeof(tmp) );

                if(tmp<AUTOAF_MIN || tmp>AUTOAF_MAX) {
                    // un-support
                    UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                    gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                    break;
                } else {
                    gbAutoFocus = tmp;
                    dbg_printf(3,"AF : %d\r\n",gbAutoFocus);
                    if(1/*gbAutoFocus*/) { // enable or disable
                        USB_EnableAF(PCAM_NONBLOCKING,gbAutoFocus);
                    }
                }  
                if(gbAutoFocus) { // Auto Focus on
                    gbFocusInfo=INFO_AUTOUPDATE_CONTROL_SUPPORT|INFO_AUTO_MODE_SUPPORT|INFO_GET_SUPPORT|INFO_SET_SUPPORT;
                }          
                else {           // Auto Focus off
                    gbFocusInfo=INFO_AUTOUPDATE_CONTROL_SUPPORT|INFO_GET_SUPPORT|INFO_SET_SUPPORT;
                }   
                //dbg_printf(3,"<Switch Focus Info : %x,%x>\r\n", gbAutoFocus,gbFocusInfo );         
                USBInPhase = SET_CUR_CMD;
                usb_ep0_null_in();
                
                if(gbAutoFocus != gblastAF ) {
                    gbUpdAF = 1 ;
                }
                gblastAF = gbAutoFocus ;
            }
            break; 
#endif
#if CT_CS_VAL & (1 << CT_ZOOM_ABSOLUTE_CONTROL)         
        case CT_ZOOM_ABSOLUTE_CONTROL:
#if SUPPORT_DIGITAL_ZOOM    
            USBOutData = 0;
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,  (MMP_UBYTE *)&tmp2, sizeof(tmp2) );

            if(tmp2<ZOM_MIN || tmp2>ZOM_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                // Enable Async Mode
                if(CT_ZOOM_ABS_CFG.bInfoCap & INFO_ASYNC_CONTROL_SUPPORT ) {
                    USB_SetAsyncVideoControl(gbVCITID,CT_ZOOM_ABSOLUTE_CONTROL,0,2);
                }
                USB_SetDigitalZoom(PCAM_OVERWR /*PCAM_NONBLOCKING*/,2,gsZoom,tmp2,ZOM_MAX); // 2->Auto
                gsZoom = tmp2 ;
            }      
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
       // while(1);
#endif        
        break;
#endif
        
#if CT_CS_VAL & (1 << CT_PANTILT_ABSOLUTE_CONTROL)         
    case CT_PANTILT_ABSOLUTE_CONTROL:
#if SUPPORT_DIGITAL_PAN    
        USBOutData = 0;
        if(len < 4 ) {
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len , (MMP_UBYTE *)&pan, sizeof(pan) );
            tilt = 0 ;
        } else {
            USB_GET_FIFO_DATA(MMPF_USB_EP0, 4 , (MMP_UBYTE *)&pan, sizeof(pan) );
            len = len - 4 ;
            USB_GET_FIFO_DATA(MMPF_USB_EP0 ,len , (MMP_UBYTE *)&tilt, sizeof(tilt) );
        }
        //pan = 0;
        //
        // NOTES:
        //   
        // It looks like Windows-XP has a bug, when adjust Tilt value.
        // Pan value will be garbage.
        //
        if(pan < PAN_MIN || pan > PAN_MAX) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            break;
        } 
        
        if(tilt < TILT_MIN || tilt > TILT_MAX) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_OUT_OF_RANGE;    
            break;
        }
        
        if(1) 
        {
            // Enable Async Mode
            if(CT_PANTILT_ABS_CFG.bInfoCap & INFO_ASYNC_CONTROL_SUPPORT ) {
                USB_SetAsyncVideoControl(gbVCITID,CT_PANTILT_ABSOLUTE_CONTROL,0,8);
            }    
            USB_SetDigitalPanTilt( PCAM_OVERWR,/*PCAM_NONBLOCKING*/glPan,pan ,glTilt,tilt,(PAN_MAX-PAN_MIN)/PANTILT_RES);
            glPan = pan ; 
            glTilt = tilt;
        }             
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
#endif        
    
        break;
#endif
        
    case CT_CONTROL_UNDEFINED:
    case CT_SCANNING_MODE_CONTROL:
    //case CT_AE_PRIORITY_CONTROL:
    //case CT_EXPOSURE_TIME_RELATIVE_CONTROL:
    case CT_FOCUS_RELATIVE_CONTROL:
        // case CT_FOCUS_AUTO_CONTROL:
    case CT_IRIS_ABSOLUTE_CONTROL:
    case CT_IRIS_RELATIVE_CONTROL:
    case CT_ZOOM_RELATIVE_CONTROL:
    case CT_PANTILT_RELATIVE_CONTROL:
    case CT_ROLL_ABSOLUTE_CONTROL:
    case CT_ROLL_RELATIVE_CONTROL:
    case CT_PRIVACY_CONTROL:
    default:
        // impossible flow       
        break;
    }
}



void usb_vc_pu_cs_out(void)
{
    MMP_UBYTE cs,tmp,len;
    MMP_USHORT tmp2;
    MMP_SHORT  sTmp;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;  


    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;////UsbRequestPayload_wLength;

   // VAR_B(0,cs);
    switch(cs) {
#if PU_CS_VAL & (1 << PU_BACKLIGHT_COMPENSATION_CONTROL)            
        case PU_BACKLIGHT_COMPENSATION_CONTROL:
            // under coding
            USBOutData = 0;           
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len , (MMP_UBYTE *)&tmp2, sizeof(tmp2) );

            if(tmp2 > BLC_MAX)
            {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE;                    
                break;
            }
            else
            {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsBacklight = tmp2;
                // to do here
                USB_SetBacklight(PCAM_NONBLOCKING,gsBacklight);         
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_BRIGHTNESS_CONTROL)          
        case PU_BRIGHTNESS_CONTROL:
            // under coding
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len , (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            
            if( tmp2 > BRI_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsBrightness = tmp2;
                // to do here
                USB_SetBrightness(PCAM_NONBLOCKING,gsBrightness);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_CONTRAST_CONTROL)      
        case PU_CONTRAST_CONTROL:
            // under coding
            USBOutData = 0;           
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len , (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            
            if(tmp2 > CON_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE; 
                //VAR_W(0,tmp2);                   
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsContrast = tmp2;
                //VAR_W(0,gsContrast); 
                // to do here
                USB_SetContrast(PCAM_NONBLOCKING,gsContrast);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_POWER_LINE_FREQUENCY_CONTROL)         
        case PU_POWER_LINE_FREQUENCY_CONTROL:
            USBOutData = 0;           
            //tmp = USB_REG_BASE_B[USB_EP0_FIFO_B];
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,&tmp, sizeof(tmp) );
            
            if(tmp > BAND_MAX ) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gbBandFilter = tmp;
                // to do here
                USB_SetPowerLineFreq(PCAM_NONBLOCKING,gbBandFilter);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_HUE_CONTROL)         
        case PU_HUE_CONTROL:
            // under coding
            USBOutData = 0;     
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,(MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            
            if(tmp2 > HUE_MAX ) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
            gsHue = sTmp;
                // to do here
                USB_SetHue(PCAM_NONBLOCKING,gsHue);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_SATURATION_CONTROL)         
        case PU_SATURATION_CONTROL:
            // under coding
            USBOutData = 0;   
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,  (MMP_UBYTE *)&tmp2, sizeof(tmp2) );

            if(tmp2 > SAT_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsSaturation = tmp2;
                // to do here
                USB_SetSaturation(PCAM_NONBLOCKING,gsSaturation);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_SHARPNESS_CONTROL)         
        case PU_SHARPNESS_CONTROL:
            // under coding
            USBOutData = 0;       
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,  (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            if(tmp2 > SHA_MAX) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsSharpness = tmp2;
                // to do here
                USB_SetSharpness(PCAM_NONBLOCKING,gsSharpness);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif            
#if PU_CS_VAL & (1 << PU_GAMMA_CONTROL)         
        case PU_GAMMA_CONTROL:
            // under coding
            USBOutData = 0;   
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,&tmp2, sizeof(tmp2) );
            if(tmp2 > (MMP_SHORT)GAM_MAX)
            {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_OUT_OF_RANGE;                     
                break;
            }
            else
            {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsGamma = sTmp;
                // to do here
                USB_SetGamma(PCAM_NONBLOCKING,gsGamma);         
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif
#if PU_CS_VAL & (1 << PU_WHITE_BALANCE_TEMPERATURE_CONTROL)         

        case PU_WHITE_BALANCE_TEMPERATURE_CONTROL:
            // under coding
            USBOutData = 0;      
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len , (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
           // VAR_B(0,gbAWB);
           // VAR_W(0,tmp2) ;
            if(gbAWB) {
                RTNA_DBG_Str0("Set Temper in AWB mode, Stall\r\n");
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;
                break;
            }// Gason@ ooxx
            if(tmp2 > WBT_MAX || tmp2 < WBT_MIN) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsTemper = tmp2;
                USB_SetAWBTemp(PCAM_NONBLOCKING,gsTemper);
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif
#if PU_CS_VAL & (1 << PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL)         

        case PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL:
            USBOutData = 0;           
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,&tmp, sizeof(tmp) );
            //VAR_W(0,len);
            //VAR_W(0,tmp);
            //dbg_printf(0,"AWB:%d\r\n",tmp);
            if(tmp > AWB_MAX || tmp < AWB_MIN) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;                    
                break;
            } else {
                gbAWB = tmp;
                //VAR_W(0,gbAWB);
                
                USB_SetAWBMode(PCAM_NONBLOCKING,gbAWB); // ooxx
            }

            if(gbAWB) {
                gbTemperInfo = INFO_AUTOUPDATE_CONTROL_SUPPORT |INFO_AUTO_MODE_SUPPORT |INFO_SET_SUPPORT |INFO_GET_SUPPORT;
            }
            else {
                gbTemperInfo = INFO_AUTOUPDATE_CONTROL_SUPPORT | INFO_SET_SUPPORT | INFO_GET_SUPPORT;
            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();

            if(gbAWB != gblastAWB) {
                //RTNA_DBG_Str0("Update Info\r\n");
                gbUpdAWB = 1;
            }
            gblastAWB = gbAWB;
            break;
#endif
#if PU_CS_VAL & (1 << PU_GAIN_CONTROL)         
        case PU_GAIN_CONTROL:
            USBOutData = 0;
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len , (MMP_UBYTE *)&tmp2, sizeof(tmp2) );
            /*
            UVC test can't pass
            if(gbAE==AEMODE_APERTURE) {
                
                RTNA_DBG_Str0("Set Gain in AE mode, Stall\r\n");
                UsbWriteEp0CSR(SET_EP0_SENDSTALL|SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode=CONTROL_WRONG_STATE;
                break;
            }
            */
            if( (tmp2 > GAIN_MAX) || (tmp2 < GAIN_MIN)) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
                //VAR_W(0,tmp2);                 
                break;
            } else {
                //              UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                gsGain = tmp2;
                if(gbAE!=AEMODE_APERTURE) {
                    USB_SetGain(PCAM_NONBLOCKING,gsGain);
                }
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
#endif
            
        case PU_CONTROL_UNDEFINED:
        case PU_WHITE_BALANCE_COMPONENT_CONTROL:
        case PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL:
        case PU_DIGITAL_MULTIPLIER_CONTROL:
        case PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL:
        case PU_HUE_AUTO_CONTROL:
        case PU_ANALOG_VIDEO_STANDARD_CONTROL:
        case PU_ANALOG_LOCK_STATUS_CONTROL:
        default:
            RTNA_DBG_Str0("Bad Flow\r\n");
        // impossible flow       
        break;
    }
}

#if (ENCODING_UNIT == 1)
//-----------------------------------------------------------------------------------//
//-	Function 	: usb_vc_eu_cs_out ( )												-//
//-	Description	: Encoding Unit defined in UVC1.5.									-//
//-																					-//
//-	Parameters	: None.																-//
//- Return		: None.																-//
//-----------------------------------------------------------------------------------//
void usb_vc_eu_cs_out(void)
{
    MMP_UBYTE cs, len;
    MMP_USHORT ret ;	
    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;
   
    switch(cs) {
	#if EU_CS_VAL & (1 << EU_SELECT_LAYER_CONTROL)
		//-------------------------------------------//
		//-		CS1:	Select Layer Control		-//
		//-------------------------------------------//
        case EU_SELECT_LAYER_CONTROL:
        {
            USBOutData = 0;           
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gw_EU_SelectLayer, sizeof(gw_EU_SelectLayer));
			
			// To do here
			// ...
			ret = UVC15_SetH264LayerId(gw_EU_SelectLayer);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	    }
	#endif            
	
	#if EU_CS_VAL & (1 << EU_PROFILE_TOOLSET_CONTROL)          
		//-------------------------------------------//
		//-		CS2: Profile & Toolset Control		-//
		//-------------------------------------------//
        case EU_PROFILE_TOOLSET_CONTROL:
        {
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_ProfToolsetCur, sizeof(gs_EU_ProfToolsetCur) );
            
			// To do here
			// ...
            ret = UVC15_SetH264ProfileToolSet(&gs_EU_ProfToolsetCur);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
          
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	    }
	#endif          

	#if EU_CS_VAL & (1 << EU_VIDEO_RESOLUTION_CONTROL)      
		//-------------------------------------------//
		//-		CS3: Video Resolution Control		-//
		//-------------------------------------------//
        case EU_VIDEO_RESOLUTION_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_VideoRes, sizeof(gs_EU_VideoRes) );
            
			// To do here
			// ...
            ret = UVC15_SetH264Resolution( (EU_VDO_RES *) &gs_EU_VideoRes );
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_MIN_FRAME_INTERVAL_CONTROL)         
		//-------------------------------------------//
		//-		CS4: min. frame interval Control	-//
		//-------------------------------------------//
        case EU_MIN_FRAME_INTERVAL_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gl_EU_minFrmIntval, sizeof(gl_EU_minFrmIntval) );
            
			// To do here
			// ...
            ret = UVC15_SetH264FrameRate( gl_EU_minFrmIntval );
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_SLICE_MODE_CONTROL)         
		//-------------------------------------------//
		//-		CS5: 	Slice Mode Control			-//
		//-------------------------------------------//
        case EU_SLICE_MODE_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_SliceMode, sizeof(gs_EU_SliceMode) );
            ret = UVC15_SetH264SliceMode(&gs_EU_SliceMode );
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_RATE_CONTROL_MODE_CONTROL)         
		//-------------------------------------------//
		//-		CS6: Rate control Mode 				-//
		//-------------------------------------------//
        case EU_RATE_CONTROL_MODE_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gb_EU_RateCtrl, sizeof(gb_EU_RateCtrl) );
            
			// To do here
			// ...
            ret = UVC15_SetH264RateControlMode( gb_EU_RateCtrl );
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;   
            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_AVERAGE_BITRATE_CONTROL)         
		//-------------------------------------------//
		//-		CS7: Average Bit Rate Control		-//
		//-------------------------------------------//
        case EU_AVERAGE_BITRATE_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gl_EU_AvgBitRate, sizeof(gl_EU_AvgBitRate) );
            
			// To do here
			// ...
            ret = UVC15_SetH264BitRate(gl_EU_AvgBitRate) ;
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   
           
            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_CPB_SIZE_CONTROL)         
		//-------------------------------------------//
		//-		CS8:  	CPB Size Control			-//
		//-------------------------------------------//
        case EU_CPB_SIZE_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gl_EU_CPBSize, sizeof(gl_EU_CPBSize) );
            
			// To do here
			// ...
            ret = UVC15_SetH264CPBSize(gl_EU_CPBSize) ;
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif          

	#if EU_CS_VAL & (1 << EU_PEAK_BIT_RATE_CONTROL)         
		//-------------------------------------------//
		//-		CS9: 	Peak bit rate Control		-//
		//-------------------------------------------//
        case EU_PEAK_BIT_RATE_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gl_EU_PeakBitRate, sizeof(gl_EU_PeakBitRate) );
            
			// To do here
			// ...
            ret = UVC15_SetH264PeakBitRate(gl_EU_PeakBitRate) ;
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }

            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif	// EU_PEAK_BIT_RATE_CONTROL

	#if EU_CS_VAL & (1 << EU_QUANTIZATION_PARAMS_CONTROL)         
		//-------------------------------------------//
		//-		CS10: Quantization Parameter CTRL	-//
		//-------------------------------------------//
        case EU_QUANTIZATION_PARAMS_CONTROL:

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_QuantParam, sizeof(gs_EU_QuantParam) );
            
			// To do here
			// ...
            ret = UVC15_SetH264QP(&gs_EU_QuantParam) ;
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }

            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif	// EU_PEAK_BIT_RATE_CONTROL
		//-------------------------------------------//
		//-		CS11: Sync Ref frame control    	-//
		//-------------------------------------------//

	#if EU_CS_VAL & (1 << EU_SYNC_REF_FRAME_CONTROL)     
	    case EU_SYNC_REF_FRAME_CONTROL:
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_QuantParam, sizeof(gs_EU_QuantParam) );
            ret = UVC15_SetH264SyncRefFrame((EU_SYNC_REF *)&gs_EU_QuantParam) ;
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }

            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
    #endif
 		//-------------------------------------------//
		//-		CS12: Priority ID               	-//
		//-------------------------------------------//
	#if EU_CS_VAL & (1 << EU_PRIORITY_ID_CONTROL)     
	    case EU_PRIORITY_ID_CONTROL:
	    
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gb_EU_PriorityId, sizeof(gb_EU_PriorityId) );
            
			// To do here
			// ...
            ret =  UVC15_SetH264PriorityID(gb_EU_PriorityId);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
    #endif



	#if EU_CS_VAL & (1 << EU_START_OR_STOP_LAYER_CONTROL)         
        case EU_START_OR_STOP_LAYER_CONTROL:
		//-------------------------------------------//
		//-		CS13: Start/Stop Layer Control		-//
		//-------------------------------------------//

            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gb_EU_StartLayerCtrl, sizeof(gb_EU_StartLayerCtrl) );
            
            ret = UVC15_SetH264StartStopLayer(gb_EU_StartLayerCtrl);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
            break;
	#endif	// EU_PEAK_BIT_RATE_CONTROL
	
	#if EU_CS_VAL & (1 << EU_LEVEL_IDC_LIMIT_CONTROL )
	    case EU_LEVEL_IDC_LIMIT_CONTROL:
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gb_EU_LevelIDCCtrl, sizeof(gb_EU_LevelIDCCtrl) );
            
            ret = UVC15_SetH264LevelIDC(gb_EU_LevelIDCCtrl);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
	        break;
    #endif
    
 	#if EU_CS_VAL & (1 << EU_QP_RANGE_CONTROL )
	    case EU_QP_RANGE_CONTROL:
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)&gs_EU_QpRangeCtrl, sizeof(gs_EU_QpRangeCtrl) );
            
            ret = UVC15_SetH264QPRange( (EU_QP_RANGE *)&gs_EU_QpRangeCtrl);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
	        break;
    #endif
 	#if EU_CS_VAL & (1 << EU_SEI_PAYLOADTYPE_CONTROL )
	    case EU_SEI_PAYLOADTYPE_CONTROL:
            USBOutData = 0;    
            USB_GET_FIFO_DATA(MMPF_USB_EP0, len, (MMP_UBYTE *)gb_EU_SEIPayloadTypeCtrl, sizeof(gb_EU_SEIPayloadTypeCtrl) );
            
            ret = UVC15_SetH264SEIMessage( (MMP_UBYTE *)&gb_EU_SEIPayloadTypeCtrl);
            if(ret) {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   

            }
            
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
	        break;
    #endif
   
    
	}
}
#endif		//#if (ENCODING_UNIT == 1)

#if USB_UVC_SKYPE
void usb_vc_eu0_cs_out(void)
{
//
// sean@2010_09_13, bug fixed for remove UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);  
//
    MMP_UBYTE cs,len;
//    MMP_USHORT i;
    MMP_BOOL IsPreviewEnable ;
   // volatile MMP_UBYTE *USB_REG_BASE_B = (volatile MMP_UBYTE *) USB_REG_BASE_ADDR;   
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;  
    
    cs = (gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;

    switch(cs) {
    case XU_SKYPE_STREAMID:
    {
        SkypeH264Error err ;
        USBOutData = 0;
        //err = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B ;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,(MMP_UBYTE *)&err,sizeof(err)) ;
        
        if( err >= SkypeStreamEnd ) {
            SKYPE_SetLastError(InvalidArgument);
        } else {
            gbSkypeLastError = err ;
        }
        
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
    }
    case XU_SKYPE_SF_PROBE:
        USBOutData = 0;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break ;
    case XU_SKYPE_SF_COMMIT:
        USBOutData = 0;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break ;
    case XU_SKYPE_BITRATE:
    {
        MMP_ULONG bitrate ;
        MMP_USHORT err ;
        USBOutData = 0 ;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,  (MMP_UBYTE *)&bitrate, sizeof(bitrate) );
        // TBD : Check the range.
        err = SKYPE_SetH264BitRate(bitrate);
        if(err) {
            SKYPE_SetLastError( NotSupported ); 
        } else {
            SKYPE_SetLastError( NoError );
        }
        USBInPhase = SET_CUR_CMD ;
        usb_ep0_null_in();
        break;
    }    
    case XU_SKYPE_FRAMERATE:
    {
        MMP_ULONG frameinterval ;
        MMP_USHORT err ;
        USBOutData = 0 ;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,  (MMP_UBYTE *)&frameinterval, sizeof(frameinterval) );
        
        err = SKYPE_SetH264FrameRate( frameinterval) ;
        if(err) {
            SKYPE_SetLastError( NotSupported ); 
        } else {
            SKYPE_SetLastError( NoError );
        }
        USBInPhase = SET_CUR_CMD ;
        usb_ep0_null_in();
        break;        
    }    
    case XU_SKYPE_KEYFRAME:
    {
        MMP_UBYTE keyframe;
        MMP_USHORT err ;
        USBOutData = 0; 
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len ,  (MMP_UBYTE *)&keyframe, sizeof(keyframe) );
        err = SKYPE_SetH264KeyFrame(keyframe );
        if(err) {
            SKYPE_SetLastError( NotAllowed ); 
        } else {
            SKYPE_SetLastError( NoError );
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break; 
        
    }
    case XU_CONTROL_UNDEFINED:
    default:
        // impossible flow       
        break;
    }
}
#endif

#define ISP_CMD_DO_AF 1
#define ISP_CMD_SET_FPS 2
MMP_BOOL gbISPCtlNrEn=1,gbISPCtlCurNr = 1 ;
#if SUPPORT_GRA_ZOOM
extern MMPF_SCALER_SOURCE gScalSrc;
extern MMP_USHORT	gsZoomParam;

#endif

void usb_vc_eu1_isp_cmd(MMP_UBYTE *cmd)//cmd -> 8byte
{

    UVCX_PictureTypeControl pictTypeCtl = { 0,0 } ;
    PCAM_USB_VIDEO_QUALITY vidQuality ;
    PCAM_USB_DEBAND        deband ;
    MMP_BOOL IsPreviewEnable;
    MMP_ULONG bitrate;  // Kbps
#if 0
    RTNA_DBG_Str(0, "isp cmd ");
    RTNA_DBG_Byte(0, cmd[0]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[1]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[2]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[3]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[4]);
    RTNA_DBG_Str(0, "\r\n");
#endif
    //eu1_cmd2_value = EU_ISP_CMD_OK;
    eu1_get_isp_result_val[0] = EU_ISP_CMD_OK;

    switch(cmd[0]) {
    case 0x15:
        {
            switch (cmd[1]) { //julian test code
            case 0x01:
                pictTypeCtl.wPicType = PIC_IDR_WITH_NEW_SPSPPS ;
                UVCX_SetH264PictureTypeCtl( &pictTypeCtl );
                break;
            case 0x02:
                pictTypeCtl.wPicType = PIC_IDR_FRAME ;
                UVCX_SetH264PictureTypeCtl( &pictTypeCtl );
                break;
            case 0x03:
                pictTypeCtl.wPicType = PIC_I_FRAME ;
                UVCX_SetH264PictureTypeCtl(&pictTypeCtl );
                break;
            case 0x04:
            {
                gsUVCXCtlModeCur.bRateControlMode = cmd[2] ;
                //dbg_printf(3, "change rc method %d\r\n", cmd[2]);
                UVCX_SetH264ControlMode(&gsUVCXCtlModeCur);
                break;
            }
            case 0x05:
                {
                MMPF_VIDENC_LEAKYBUCKET_CTL lb;
                lb.ubLayerBitMap = 0x03;
                lb.ulLeakyBucket[0] = cmd[2];
                MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_LB_SIZE, (void*)&lb);
                }
                break;
            case 0x06: 
            {
            #if H264_CBR_PADDING_EN
                UVCX_RateControlMode rc_mode = {0};
                if(cmd[2]==0) {
                    rc_mode.bRateControlMode = CBR_H264 ; 
                }
                else {
                    rc_mode.bRateControlMode = VBR_H264 ; 
                }
                UVCX_SetH264ControlModeFixCBR(&rc_mode);
                break;
            #endif
                
            }
            case 0x07:
                MMPF_VIDENC_UpdateRCQuality(0, 0, cmd[2], cmd[3]);
                break;
            case 0x08:
            {
                UVCX_QPStepsLayers qs = {0} ;
                qs.bFrameType = cmd[2];
                qs.bMinQp = cmd[3];
                qs.bMaxQp = cmd[4];
                UVCX_SetH264QPStepsLayers(&qs);
            }
                break;
            case 0x09:
            {
                MMP_LONG min,max;
                MMPF_VIDENC_GetQPBound(0, 0, MMPF_3GPMGR_FRAME_TYPE_P,&min,&max);
            }
                break;
            }
        }
        break;
    case 0:
#if USING_UVCX_VIDEO_CONFIG==1
    if(UVC_VCD()==bcdVCD10){

        UVCX_PictureTypeControl picTypeCtl = {0, PIC_IDR_FRAME} ;
// Force -IDR
        UVCX_SetH264PictureTypeCtl(&picTypeCtl);

    } else {
        EU_SYNC_REF sync_ref = { SYNC_IDR_WITH_SPSPPS , 0,0 } ;
        UVC15_SetH264SyncRefFrame(&sync_ref);
    }
#endif      
        break;    

    case 1:
        //AF

        ISP_IF_AF_Control(ISP_AF_START);
        break;
    case 2:
        //set encoder fps
         if(1){
#if (FPS_CTL == 1)   

        PCAM_USB_VIDEO_FORMAT vidFmt = pcam_get_info()->pCamVideoFormat;
        if(vidFmt == PCAM_USB_VIDEO_FORMAT_H264) {
            MMP_USHORT fps = cmd[1] ;
            MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
#if USING_UVCX_VIDEO_CONFIG==1
            if(UVC_VCD()==bcdVCD10){
                if(IsPreviewEnable) {
                    UVCX_FrameRateConfig frameRate = {0,0 } ;
                    
                    if(fps > 30) 
                        fps = 30 ;
                    else if (fps < 5) 
                        fps = 5;
                    frameRate.dwFrameInterval = 10000000 / fps ;         
                    UVCX_SetH264FrameRate(&frameRate);
                }
            }
#endif
#if ENCODING_UNIT==1
            if(UVC_VCD()==bcdVCD15){
                if(IsPreviewEnable) {
                    UVC15_SetH264FrameRate(EU_FRAME_INTERVAL(fps)) ;
                }
            }
#endif
        } else {
			//DBG_S(3, "Set FPS (Non-H264) =");
            MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
            if(IsPreviewEnable){
                PCAM_USB_Set_FPSx10(gbAEPriority,0,cmd[1]*10);
            }
        }   
#else
			DBG_S(3, "Set FPS =");
            MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
            if(IsPreviewEnable){
                ISP_IF_AE_SetFPS(cmd[1] );
            }

#endif            
			DBG_S(3, "\r\n");
        }           
        break;
    case 4:
        //Set IDR frame

        USB_ForceH264IDRFrame();

        break;
    case 5:
    {
    #if SUPPORT_PCSYNC
        if(cmd[1]==0) {
            pcsync_stop_preview();
        }
        else {
            pcsync_start_preview();
        }
        break ;
    #endif
        
    }
    case 6:
    {
    #if SUPPORT_PCSYNC
        pcsync_encode_frame(cmd[1]);
        break;
    #endif    
    }    
    case 7:
    {
    #if SUPPORT_PCSYNC   
        MMP_USHORT sId;
        MMP_ULONG frameptr,framesize ;
        frameptr = pcsync_get_frame(&framesize,&sId);
        dbg_printf(3,"pcsync.get.frame,ptr:%x,size:%d\r\n",frameptr,framesize);
        break;
    #endif    
    }
    case 8:
        // change quality
        vidQuality = cmd[1] ;
        //m_usVideoQuality = cmd[1];
        switch(vidQuality/*m_usVideoQuality*/) {
        case 0xff:  // custom video bit rate
 
            bitrate = (MMP_ULONG)(cmd[2] + ((MMP_USHORT)cmd[3] << 8));
 
#if (FPS_CTL == 1)   // bit rate control API test

            RTNA_DBG_Str(0, "  ** bitrate = ");       
            MMPF_DBG_Int(bitrate, -5);
            RTNA_DBG_Str(0, "\r\n");

            MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
            if(IsPreviewEnable){
#if USING_UVCX_VIDEO_CONFIG==1
                if(UVC_VCD()==bcdVCD10){
                    UVCX_BitRateLayers br_ctl ;
                   // br_ctl.wLayerID |= (1 << LAYER_ID_BIT_SHIFT_TEMPORAL); //layer 0
                   
                    br_ctl.wLayerID = LAYER_ID( cmd[4],0,0,0) ;
                    br_ctl.dwAverageBitrate = br_ctl.dwPeekBitrate = bitrate * 1000;
                    UVCX_SetH264BitRate(&br_ctl);
                }
#endif 
#if ENCODING_UNIT
                if(UVC_VCD()==bcdVCD15){
                    UVC15_SetH264BitRate(bitrate * 1000);
                }
#endif
               
            }
#endif
            
           //USB_SetVideoQuality(PCAM_NONBLOCKING,vidQuality);
           break;
        default:
           ISP_IF_AE_SetFPS(30); // ooxx842
            break;
        }

        break;
    case 9:
        // set flicker mode, 0: auto, 1: 60Hz, 2: 50Hz
        if(cmd[1] == 0x01)
            gbBandFilter = BAND_60HZ;
        else if(cmd[1] == 0x02)
            gbBandFilter = BAND_50HZ;
        else
            gbBandFilter = BAND_60HZ;
            
        RTNA_DBG_Str(3, "  ** USB Light Freq Set : ");  
        RTNA_DBG_Short(3, gbBandFilter);
        RTNA_DBG_Str(3, "\r\n");
        USB_SetPowerLineFreq(PCAM_NONBLOCKING,gbBandFilter);
        
        break;
    case 10:
    {
		MMP_USHORT Addr;
		MMP_USHORT Val;
		volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) (0x80000000);
        switch(cmd[1])
        {
        	case 0://MMP register 1 byte read
                Addr = cmd[2] + cmd[3] * 0x100;
                    eu1_get_isp_result_val[2] = REG_BASE_B[Addr];
                break;
        	case 1://MMP register 1 byte write
				Addr = cmd[2] + cmd[3] * 0x100;
            	    REG_BASE_B[Addr] 	= cmd[4];//Value&0xFF;   
                break;
        	case 2://sensor register 1 byte read
				Addr = cmd[2] + cmd[3] * 0x100;
            	gsSensorFunction->MMPF_Sensor_GetReg(Addr, &Val);         
            	eu1_get_isp_result_val[2] = (MMP_UBYTE)(Val & 0xFF);  // Low byte
            	eu1_get_isp_result_val[3] = (MMP_UBYTE)(Val >> 8);  // High byte
        RTNA_DBG_Str(0, "  ** Read Sensor Reg[ ");       
        RTNA_DBG_Short(0, Addr);
        RTNA_DBG_Str(0, "] = ");
        RTNA_DBG_Short(0, Val);
        RTNA_DBG_Str(0, "\r\n");
                break;
        	case 3://sensor register 1 byte write
				Addr = cmd[2] + cmd[3] * 0x100;        	
            	Val = cmd[4] + cmd[5] * 0x100;
            	gsSensorFunction->MMPF_Sensor_SetReg(Addr, Val);  
        RTNA_DBG_Str(0, "  ** Write Sensor Reg[ ");       
        RTNA_DBG_Short(0, Addr);
        RTNA_DBG_Str(0, "] = ");
        RTNA_DBG_Short(0, Val);
        RTNA_DBG_Str(0, "\r\n");
                break;
        	default:
        		eu1_get_isp_result_val[1] = EU_ISP_CMD_NOT_SUPPORT;
        	break;
        }          
    }
        break;
    case 11: 
        //get FW ver
         {
            EU_VDO_RES h264_res = {  640, 360 };
            #if 0
            
            eu1_get_isp_result_val[2] = (gbFwVersion.major >> 8) & 0xFF;
            eu1_get_isp_result_val[3] = gbFwVersion.major & 0xFF;
            eu1_get_isp_result_val[4] = (gbFwVersion.minor >> 8) & 0xFF;
            eu1_get_isp_result_val[5] = gbFwVersion.minor & 0xFF;               
            eu1_get_isp_result_val[6] = (gbFwVersion.build >> 8) & 0xFF;
            eu1_get_isp_result_val[7] = gbFwVersion.build & 0xFF;
            
            #else
            
            eu1_get_isp_result_val[2] = gbFwVersion.main & 0xFF;
            eu1_get_isp_result_val[3] = gbFwVersion.major & 0xFF;
            eu1_get_isp_result_val[4] = (gbFwVersion.minor >> 8) & 0xFF;
            eu1_get_isp_result_val[5] = gbFwVersion.minor & 0xFF;               
            eu1_get_isp_result_val[6] = (gbFwVersion.build >> 8) & 0xFF;
            eu1_get_isp_result_val[7] = gbFwVersion.build & 0xFF;

            #endif
            //UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,0,0) );
            //UVC15_SetH264Resolution( &h264_res);
           // while(1);
         }
        break;
    case 12://get FW build date
         {
            eu1_get_isp_result_val[1] = mmpf_buildVersion.szDate[9];                
            eu1_get_isp_result_val[2] = mmpf_buildVersion.szDate[10];
            eu1_get_isp_result_val[3] = mmpf_buildVersion.szDate[0];
            eu1_get_isp_result_val[4] = mmpf_buildVersion.szDate[1];
            eu1_get_isp_result_val[5] = mmpf_buildVersion.szDate[2];                
            eu1_get_isp_result_val[6] = mmpf_buildVersion.szDate[4];
            eu1_get_isp_result_val[7] = mmpf_buildVersion.szDate[5];
        }
        break;
    case 13:
    	{
    		#if (USB_UVC_SKYPE==0)&&(USB_UVC15==0)
    		//SetH264Mode(cmd[1]);
    		#endif
    	}
    	break;
    case 14:
    	{
            if(cmd[1]){
				ISP_IF_F_SetWDR(255); // 0 ~ 255
                RTNA_DBG_Str(0, "  ** WDR ON  **\r\n");
            }
            else{
				ISP_IF_F_SetWDR(0);
                RTNA_DBG_Str(0, "  ** WDR OFF  **\r\n");
            }
    	}
    	break;
    	
    case 0x30:
    {
        UVCX_SetSimulcastH264(cmd[1]);
    }
    break;
    case 0x29:
    {
            if(cmd[1]==0) {                          
                UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,0,0) );
            }
            else {
                UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,1,0) );
            }
            UVC15_SetH264PriorityID( cmd[2] );
            break ;
    }	
    case 0x28:
    {
        //ENCODE_BUF_MODE buf_mode = (cmd[1]==0)?REALTIME_MODE :FRAME_MODE;
        static ENCODE_BUF_MODE buf_mode = REALTIME_MODE ;
        do {
            
            //UVCX_SwitchH264EncodeBufferMode(buf_mode);
            if(buf_mode==REALTIME_MODE) {
                USB_SetH264EncodeBufferMode(MMP_FALSE);
                buf_mode = FRAME_MODE ;
            }
            else {
                USB_SetH264EncodeBufferMode(MMP_TRUE);
                buf_mode = REALTIME_MODE ;
            }
            //MMPF_OS_Sleep(5*1000);
        } while( 0 );
        
        break;
    }    	
    case 0x27:
    {
        MMP_USHORT ratioN,ratioM;
        MMP_UBYTE  edge_core,edge_val ;
        MMPF_Scaler_GetCurRatio(MMPF_SCALER_PATH_0,&ratioN,&ratioM) ;
        
        if(ratioN != 0) {
            if(ratioN > ratioM) {
                if(cmd[1]==0) {
                    MMPF_Scaler_GetEdge(MMPF_SCALER_PATH_0,&edge_core,&edge_val) ;
                    dbg_printf(3,"--Edge Core ,Val :%d,%d\r\n",edge_core,edge_val);
                }
                else {
                    MMPF_Scaler_SetEdge(MMPF_SCALER_PATH_0,cmd[2],cmd[3]) ;
                }
            }
        }
        
    }
    break;    	
    case 0x25:
    {
extern MMP_USHORT GetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB);
extern MMP_USHORT SetH264QPRange(MMP_UBYTE sid,MMP_UBYTE tid,EU_QP_RANGE *qpI,EU_QP_RANGE *qpP,EU_QP_RANGE *qpB);
        EU_QP_RANGE qpI,qpP,qpB;
        if(cmd[1]==0) {
            GetH264QPRange(0,0,&qpI,&qpP,&qpB);
            GetH264QPRange(0,1,&qpI,&qpP,&qpB);
            GetH264QPRange(0,2,&qpI,&qpP,&qpB);
        } else {
            qpI.minQP = cmd[1] ;
            qpI.maxQP = cmd[2] ;
            UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,0,0) );
            UVC15_SetH264QPRange(&qpI);
        /*
            qpI.minQP = cmd[1] ;
            qpI.maxQP = cmd[2] ;
            SetH264QPRange(0,0,&qpI);
            SetH264QPRange(0,1,&qpI);
            SetH264QPRange(0,2,&qpI);
        */    
        }
        break;
    }	
    case 0x26:
    {
        extern MMP_ULONG m_OffScrScalingYBufAddr[2],m_OffScrScalingUBufAddr[2];
        extern MMP_ULONG m_OffScrScalingW[],m_OffScrScalingH[];
        if(cmd[1]==0) {
            USB_ScalingOSD(PCAM_OVERWR,0,cmd[2],cmd[3]);    
        }
        else {
            pcam_Osd_SetColor(cmd[2],cmd[3],cmd[4]);
            pcam_Osd_DrawColor(m_OffScrScalingYBufAddr[0],m_OffScrScalingUBufAddr[0],m_OffScrScalingW[0],m_OffScrScalingH[0]);
                
        }
        break ; 
    
    }
    case 0x24: 
         {
            EU_VDO_RES h264_res[] = {
                                      { 1920,1080 },
                                      { 1280,720  },
                                      {  640, 360 },
                                      {  320, 180 } 
                                    } ;
            if(cmd[1]==0) {                          
                UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,0,0) );
            }
            else {
                UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,1,0) );
            }
            dbg_printf(0,"sId : %d,res -> %d,%d\r\n", cmd[1],h264_res[cmd[2] ].wWidth ,h264_res[cmd[2] ].wHeight );
            
            UVC15_SetH264Resolution( &h264_res[ cmd[2] ]);
         }
    	 break;
    case 0x23:
    {    
        MMP_USHORT layerid ;
        dbg_printf(3,"cmd[1] : %d , cmd[2] :%d\r\n",cmd[1],cmd[2]);
        
        layerid = SVC_LAYER_ID(0 , 0, cmd[2] , SIM_WILDCARD_MASK , 0 );    
        UVC15_SetH264LayerId(layerid);
        UVC15_SetH264StartStopLayer(cmd[1]);
        break;	
    }
    case 0x22: // change h264 profile
    {
        EU_PROF_TOOLSET toolset ;
        if(cmd[1]==0) {                          
            UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,0,0) );
        }
        else {
            UVC15_SetH264LayerId( SVC_LAYER_ID(0,0,7,1,0) );
        }
        toolset.bmSetting = 0x28;// DEVICE_CHOOSE; 
        toolset.wProfile = (cmd[2]==0)?CONSTRAINED_BASELINE_P : CONSTRAINED_HIGH_P ;
        toolset.wConstrainedToolset =0 ;
        UVC15_SetH264ProfileToolSet(&toolset);
        break;
    }
#if SUPPORT_GRA_ZOOM 
    case 0x21:
    {
   // extern MMP_USHORT  gsPixlDly , gsLineDly  ;
   // gsPixlDly = cmd[1] ;
   // gsLineDly = cmd[2] ;
   // dbg_printf(3,"gsLineDly : %x,gsPixlDly : %x\r\n",gsLineDly,gsPixlDly);
        MMP_LONG pan,tilt ;
        MMPF_SCALER_GRABCONTROL tmpgrabCtl;
        pan = (MMP_ULONG)( (MMP_UBYTE)cmd[1] - 10) ;
        tilt = (MMP_ULONG)( (MMP_UBYTE)cmd[2] - 10) ;
        
        dbg_printf(3,"###pan : %d\r\n",pan);
        tilt *= PANTILT_RES ; 
        pan *= PANTILT_RES ;
        
       
        dbg_printf(3,"Set[Pan,Tilt]=[%d,%d]\r\n",pan,tilt);
        USB_SetDigitalPanTilt( PCAM_OVERWR,/*PCAM_NONBLOCKING*/glPan,pan ,glTilt,tilt,(PAN_MAX-PAN_MIN)/PANTILT_RES);
        glPan = pan ; 
        glTilt = tilt;
        
        break;
    }  
     	
    case 0x20:
        if(1) {
            int c = 0,x = 1;
            MMP_USHORT tmp2 = 100;
            MMP_USHORT ratio[]={100,150,200,250,300,350,400,450,500,480,420,380,320,280,220,180,120,500,180,150 };
            MMP_ULONG  usof;
            MMP_UBYTE  *ptr =(MMP_UBYTE  *) &usof;
            switch(cmd[1]) {
            case 0x09:
            do {
               
                //dbg_printf(3,"step:%d\r\n",tmp2);
                USB_SetDigitalZoom(PCAM_OVERWR /*PCAM_NONBLOCKING*/,2,gsZoom,tmp2,ZOM_MAX); // 2->Auto
                gsZoom = tmp2 ;
                MMPF_OS_Sleep(500) ;
                 tmp2++;
            } while(tmp2 <= 210);
            break;
            case 0x10:
            do {
                usof = usb_vc_cur_usof();
                
                tmp2 = ptr[0] + ptr[1] + ptr[2] +ptr[3] ;
                if(tmp2 > 500) tmp2 = tmp2 % 500 ;
                if(tmp2 < 100) tmp2 = 100 ;
                
                dbg_printf(3,"Zoom> Step : %d\r\n",tmp2);
                USB_SetDigitalZoom(PCAM_OVERWR /*PCAM_NONBLOCKING*/,2,gsZoom,tmp2,ZOM_MAX); // 2->Auto
                gsZoom = tmp2 ;
                c++;
                if(c >= 20) {
                    c = 0;
                }
                MMPF_OS_Sleep(40) ;
            } while(1);
            break;
            case 0x11:
                c = 500;
                x = -1 ;
                do {
                tmp2 = c ;
                dbg_printf(3,"Zoom> Step : %d\r\n",tmp2);
                USB_SetDigitalZoom(PCAM_OVERWR /*PCAM_NONBLOCKING*/,2,gsZoom,tmp2,ZOM_MAX); // 2->Auto
                gsZoom = tmp2 ;
                c+=x;
                if(c <=100 ) {
                    c = 100;
                    x = 1 ;
                } 
                else if(c >= 500){
                    c = 500;
                    x = -1;
                }
                MMPF_OS_Sleep(40) ;
                } while(1);
                break;
            
            default:
                tmp2 = (cmd[1] << 8) | cmd[2] ;
                dbg_printf(3,"Zoom> Step : %d\r\n",tmp2);
                USB_SetDigitalZoom(PCAM_OVERWR /*PCAM_NONBLOCKING*/,2,gsZoom,tmp2,ZOM_MAX); // 2->Auto
                gsZoom = tmp2 ;
            }
        }
    	break;
#endif   		
    case 0x50:
        {
            if(cmd[1]==0) {
                USB_ForceBlackEnable(PCAM_NONBLOCKING,MMP_FALSE) ;
            }
            else {
                USB_ForceBlackEnable(PCAM_NONBLOCKING,MMP_TRUE) ;
            }
        }
        break;    	
    case 0xFF:  // ISP set command
    	{
            #if SUPPORT_LED_DRIVER
            MMP_USHORT usMSB, usLSB, code, read_value;;
            #endif

    		switch(cmd[1])
    		{
    		case 0:
    			if(cmd[2]==0)
    			{
    				//disable lens shading
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_LS, 0);
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_CS, 0);
    			}else{
    				//enable lens shading
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_LS, 1);
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_CS, 1);
    			}
    			break;
    		case 1:
    			if(cmd[2]==0)
    			{
    				//disable denoise
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_NR, 0);
    			}else{
    				//enable denoise
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_NR, 1);
    			}
    			break;
    		case 2:
    			if(cmd[2]==0)
    			{
    				//disable gamma
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_GC8, 0);
    				//gbGammaStatus = 0;
    			}else{
    				//enable gamma
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_GC8, 1);
    				//gbGammaStatus = 1;
    			}
    			break;    			
    		case 3:
    			if(cmd[2]==0)
    			{
    				//disable color matrix
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_CC, 0);
    				//gbColorMatrixStatus = 0;
    			}else{
    				//enable color matrix
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_CC, 1);
    				//gbColorMatrixStatus = 1;
    			}
    			break;    			
    		case 4:
    			if(cmd[2]==0)
    			{
    				//disable edge enhancement
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_EE, 0);
    				//gbEdgeEnhancementStatus = 0;
    			}else{
    				//enable edge enhancement
    				ISP_IF_IQ_SetSwitch(ISP_IQ_MODULE_EE, 1);
    				//gbEdgeEnhancementStatus = 1;
    			}
    			break;    			
    		case 5:
    			if(cmd[2]==0)
    			{
    				//disable AE
    				//Set_AEEnable(0);
    				gbAEStatus = 0;
    			}else{
    				//enable AE
    				//Set_AEEnable(1);
    				gbAEStatus = 1;
    			}
    			USB_EnableAE(PCAM_NONBLOCKING,gbAEStatus);
    			break;    			
    		case 6:
    			if(cmd[2]==0)
    			{
    				//disable AWB
    				//Set_AWBEnable(0);
    				gbAWBStatus = 0;
    			}else{
    				//enable AWB
    				//Set_AWBEnable(1);
    				gbAWBStatus = 1;
    			}
    			USB_SetAWBMode(PCAM_NONBLOCKING,gbAWBStatus);
    			break;    			
    		case 7:
    			if(cmd[2] < 9)
    			{
    			    gsSharpness = cmd[2];
   				    USB_SetSharpness(PCAM_NONBLOCKING,(MMP_BYTE)(gsSharpness-4));
    			}
    			break;    			

            #if SUPPORT_LED_DRIVER
            #if BIND_LED_LM36922
            // Set LM36922 Register
            case 0x08:
                dbg_printf(3,"[XU] Set LED Reg 0x%x = x%x \r\n",cmd[2],cmd[3]);
                MMPF_LED_WriteI2C(cmd[2],cmd[3]);
                break;

            // Get LM36922 Register
            case 0x09:
                MMPF_LED_ReadI2C(cmd[2],&read_value);
                dbg_printf(3,"[XU] Get LED Reg 0x%x = x%x \r\n",cmd[2],read_value);
                break;

            // Enable or Disable LED Driver
            case 0x0A:
				if(cmd[2]==0) // LED Driver Off
				{
					 LED_Driver_Enable(0);
					 RTNA_DBG_Str(0, "[XU] Disable LED Driver\r\n");
				}
				else if(cmd[2]==1) // LED Driver On
				{
					 LED_Driver_Enable(1);
					 RTNA_DBG_Str(0, "[XU] Enable LED Driver\r\n");
				}
                break;

            // Set LED Driver Brightness
            case 0x0B:
                 usMSB = cmd[2];
		if(usMSB >= LM36922_BRIGHT_STEP)	
			usMSB = LM36922_BRIGHT_STEP;	
		   //LED_Driver_SetBrightnessCode(usMSB*1000);
		   	  //if(usMSB == 0)
			  //	code = 0;
			  //else
                        //    code = ((usMSB -1)*LM36922_BRIGHT_PERSTEP) + LM36922_Min_BRIGHT;
			  //LED_Driver_SetBrightnessCode(code);
			  //usMSB = usMSB*Ext_CHA_INT_DIV;
			   LED_Driver_SetBrightnessStep(usMSB);
			 usMSB = LED_Driver_GetBrightnessStep();  
		   dbg_printf(0,"[XU] Set Bright code MSB: %d \r\n",usMSB);
		#if 0
                 usLSB = cmd[3]&0x7;

                 code = (usMSB<<3)|usLSB;
                 dbg_printf(0,"[XU] Set Bright code MSB: x%03x  %04d\r\n",usMSB,usMSB);
                 dbg_printf(0,"[XU] Set Bright code LSB: x%03x  %04d\r\n",usLSB,usLSB);
                 dbg_printf(0,"[XU] Brightness code    : x%03x  %04d\r\n",code,code);

                 LED_Driver_SetBrightnessCode(code);
		 #endif
                 //LED_Driver_GetBrightnessCode();
                 //LED_Driver_GetBrightnessStep();
                break;
            case 0x0C:
				code = LED_Driver_GetBrightnessCode();
		 usMSB = LED_Driver_GetBrightnessStep();
		 dbg_printf(3,"[XU] Get Brightness code    : x%d %d \r\n",usMSB , code);
		 //usMSB = usMSB/Ext_CHA_INT_DIV;
		 dbg_printf(3,"[XU] Get Brightness %d \r\n",usMSB);
		 eu1_get_isp_result_val[2] = (MMP_UBYTE)usMSB ; 
		 
		#if 0
                 code = LED_Driver_GetBrightnessCode();
			dbg_printf(0,"[XU] Get Brightness code    : x%03x  %04d\r\n",code,code);
			eu1_get_isp_result_val[2] = (code >> 8) & 0xFF; 
			eu1_get_isp_result_val[3] = code & 0xFF; 
		#endif
                 //LED_Driver_GetBrightnessStep();
                break;
            //Clean Video Properties in flash
            case 0x0D:
				MMPF_SF_EraseSector(CUSTOMER_INIT_FLASH_ADDR_VP);
           	 	break;   
            case 0x0E:
                USB_EnableBayerRawPreview(cmd[2]);                   
                dbg_printf(3, "USB_EnableBayerRawPreview(%d)\r\n", cmd[2]);
                break;
           	//Write data to flash
           	case 0x10:
            	 SetSerialNumber2Flash(SERIAL_NUMBER_FLASH_ADDR, SERIAL_NUMBER_WRITE_BUF_STA);
           	 	break;
           	//Read data from flash 
           	case 0x11:
            	 GetSerialNumberFromFlash(SERIAL_NUMBER_FLASH_ADDR, SERIAL_NUMBER_READ_BUF_STA);
           	break;                 		
            #endif
            #endif

			#if 0
            // Sensor Mirror Flip
            case 0x11:
                /*
                 * cmd[2]:
                 * 0: MMPF_SENSOR_ROTATE_NO_ROTATE
                 * 1: MMPF_SENSOR_ROTATE_RIGHT_90
                 * 2: MMPF_SENSOR_ROTATE_RIGHT_180
                 * 3: MMPF_SENSOR_ROTATE_RIGHT_270
                 * 4: MMPF_SENSOR_ROTATE_H_MIRROR
                 * 5: MMPF_SENSOR_ROTATE_V_FLIP
                 */
                dbg_printf(0,"[XU] MMPF_Sensor_SetColorID: %d\r\n",cmd[2]);
                gsSensorFunction->MMPF_Sensor_SetColorID(cmd[2]);
                break;
                
            #endif               
    		}
    	}
    	break;
    case 0x7F:  // ISP get command
    	{
    		switch(cmd[1])
    		{
    		case 0:
    		
                eu1_get_isp_result_val[2] = ISP_IF_IQ_GetSwitch(ISP_IQ_MODULE_LS);
    		
    			break;
    		case 1:
    		
                eu1_get_isp_result_val[2] = ISP_IF_IQ_GetSwitch(ISP_IQ_MODULE_NR);
    		
    			break;
    		case 2:
    		
                eu1_get_isp_result_val[2] = ISP_IF_IQ_GetSwitch(ISP_IQ_MODULE_GC8);;
    		
    			break;    			
    		case 3:
    		
                eu1_get_isp_result_val[2] = ISP_IF_IQ_GetSwitch(ISP_IQ_MODULE_CC);
    		
    			break;    			
    		case 4:
    		
                eu1_get_isp_result_val[2] = ISP_IF_IQ_GetSwitch(ISP_IQ_MODULE_EE);
    		
    			break;    			
    		case 5:
    		
                eu1_get_isp_result_val[2] = gbAEStatus;
    		
    			break;    			
    		case 6:
    		
                eu1_get_isp_result_val[2] = gbAWBStatus;
    		
    			break;    			
    		case 7:
    		
                eu1_get_isp_result_val[2] = gsSharpness;
    		
    			break;    			
    		case 8://get FPS
    		    {
    			MMPF_VIDENC_MAX_FPS_CTL FpsCtl;

                MMPF_VIDENC_GetParameter(0, MMPF_VIDENC_ATTRIBUTE_MAX_FPS, &FpsCtl);

                eu1_get_isp_result_val[2] = (FpsCtl.ulMaxFpsIncreament)? 
                                            FpsCtl.ulMaxFpsResolution/FpsCtl.ulMaxFpsIncreament:
                                            0;
                }
    			break;    			
    		case 9://get FICKER setting
    			
                eu1_get_isp_result_val[2] = gbBandFilter;    			
    			
    			break;    			
    		}
    	}
    	break;
    default:
        //eu1_cmd2_value = EU_ISP_CMD_NOT_SUPPORT;
        eu1_get_isp_result_val[0] = EU_ISP_CMD_NOT_SUPPORT;     
        break;
    }
}


MMP_ERR USB_DownloadFWInitialize(void);
MMP_ERR USB_DownloadFWData(MMP_UBYTE *data, MMP_USHORT len);
MMP_ERR USB_Burning840FW2SFlash(void);


#define DOWNLOAD_MMP_FW_CMD    1
#define GET_DRAM_SIZE          2
#define GET_FLASH_ID           3
#define GET_FLASH_SIZE         4
#define ERASE_FLASH            5
#define STREAM_VIEWER_CMD      12
#define ROMBOOT_RESET		   22	



#define I2C_MODE_1A1D 1
#define I2C_MODE_2A1D 0
void usb_vc_eu1_mmp_cmd(MMP_UBYTE *cmd)//cmd -> 8byte
{
    RTNA_DBG_Str(0, "mmp cmd ");
    RTNA_DBG_Byte(0, cmd[0]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[1]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[2]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[3]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[4]);
    RTNA_DBG_Str(0, "\r\n");

    eu1_get_mmp_result_val[0] = EU_ISP_CMD_OK;

    switch(cmd[0]) {
    case STREAM_VIEWER_CMD:
    {
        gbSignalType = cmd[1] ;
        dbg_printf(3,"stream viewer signal type : %d\r\n",gbSignalType);
    }
    break;
    case DOWNLOAD_MMP_FW_CMD:
        //FW burning,burn the firmware in task instead of ISR
        switch(cmd[1]) {
        case 0:
            gbUVCDSCCommand = 0x80;
			USB_UpdateFirmware(PCAM_BLOCKING,0);
            break;
        case 1:
            gbUVCDSCCommand = 0x81;
            gbUVCPara[0] = cmd[3];  // 0: old FW Package, 1: new ROM Package
			eu1_get_mmp_result_val[0] = 0x81;
			USB_UpdateFirmware(PCAM_NONBLOCKING,1);    
            break;
        default:
            break;
        }
        break;
     case GET_DRAM_SIZE:
            eu1_get_mmp_result_val[1] = 0x00;  // LSB                
            eu1_get_mmp_result_val[2] = 0x00;
            eu1_get_mmp_result_val[3] = 0x00;
            eu1_get_mmp_result_val[4] = 0x02;  // MSB

        break;
    case GET_FLASH_ID:
            eu1_get_mmp_result_val[1] = 0x01;  // 1: serial flash, 2: NAND flash     

         	//MMPF_SIF_Init();

            eu1_get_mmp_result_val[2] = *(MMP_UBYTE *)(SIF_BUFFER_START);  // LSB
            eu1_get_mmp_result_val[3] = *(MMP_UBYTE *)(SIF_BUFFER_START + 1);
            eu1_get_mmp_result_val[4] = *(MMP_UBYTE *)(SIF_BUFFER_START + 2);  // 
            eu1_get_mmp_result_val[5] = *(MMP_UBYTE *)(SIF_BUFFER_START + 3);  // MSB

        break;
    case GET_FLASH_SIZE:
            eu1_get_mmp_result_val[1] = 0x00;  // LSB                
            eu1_get_mmp_result_val[2] = 0x00;
            eu1_get_mmp_result_val[3] = 0x04;
            eu1_get_mmp_result_val[4] = 0x00;  // MSB

        break;
    case ERASE_FLASH:
        switch(cmd[1]) {
        case 1:  // Chip Erase
            //gbUVCDSCCommand = 0x90;
            //MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_USBOP, MMPF_OS_FLAG_SET); 
            PCAM_USB_EraseFlash(1, 0);      
            break;
        case 2:  // Sector Erase
        {
            MMP_ULONG addr;
           // gbUVCDSCCommand = 0x91;
            gbUVCPara[0] = cmd[2];  // LSB
            gbUVCPara[1] = cmd[3];
            gbUVCPara[2] = cmd[4];
            gbUVCPara[3] = cmd[5];  // MSB
            addr = (MMP_ULONG)(gbUVCPara[0] + gbUVCPara[1] << 8 + gbUVCPara[2] << 16 + gbUVCPara[3] << 24);
            PCAM_USB_EraseFlash(2, addr);    
            
            //MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_USBOP, MMPF_OS_FLAG_SET); 
            break;
         }   
        default:
            break;
        }
        break;
    case ROMBOOT_RESET:
    	usb_ep0_null_in();
    	{
                AITPS_GBL pGBL = AITC_BASE_GBL;
                AITPS_VIF pVIF = AITC_BASE_VIF;
                 
                //Make sure that VI Clock is ON
                pGBL->GBL_CLK_DIS0 &= (MMP_UBYTE)(~(GBL_CLK_VI_DIS));
                
                // pull high this pin to force ROM boot download(MSDC) mode, 
                // if the pin is high => MSDC mode, if the pin is low => normal boot mode.
        #if (VIF_CTL == VIF_CTL_VIF0) 
                MMPF_VIF_SetPIODir(VIF_SIF_RST, 0x01);  
                MMPF_VIF_SetPIOOutput(VIF_SIF_RST, 0x01); 
        #else
        		pVIF->VIF_0_SENSR_SIF_EN |= VIF_SIF_RST;
        		pVIF->VIF_0_SENSR_SIF_DATA |= VIF_SIF_RST;        		
        #endif        
                // 1: re-boot FW to MSDC mode, 0: re-boot ROM & Flash code normally
                MMPF_SYS_ResetSystem(0x1);  
      	}    
        break;
    case 0xFF:  // USB XU external I2C command
    	{
    		switch(cmd[1])
    		{
    			case 0:  //set I2C slave address
    				gsUSBXU_I2C_ID = cmd[2];  //cmd[2] = slave address
                    RTNA_DBG_Str(0, " gsUSBXU_I2C_ID = ");       
                    RTNA_DBG_Byte(0, gsUSBXU_I2C_ID);
                    RTNA_DBG_Str(0, "\r\n");
    			    break;

    			case 1:  //write I2C 2A1D
    				//cmd[2] = addr HB
    				//cmd[3] = addr LB
    				//cmd[4] = value
    				USB_WriteI2C((MMP_USHORT)(cmd[2]*256 + cmd[3]), (MMP_USHORT)cmd[4],I2C_MODE_2A1D);

                    RTNA_DBG_Str(0, " 2A1D Write Reg[");       
                    RTNA_DBG_Short(0, (cmd[2]*256 + cmd[3]));
                    RTNA_DBG_Str(0, " ] = ");       
                    RTNA_DBG_Byte(0, cmd[4]);
                    RTNA_DBG_Str(0, "\r\n");

    			    break;
    			    
    			case 2:  //read I2C 2A1D
    				//cmd[2] = addr HB
    				//cmd[3] = addr LB
    				//cmd[4] = value
    				eu1_get_mmp_result_val[2] = (MMP_UBYTE)USB_ReadI2C((MMP_USHORT)(cmd[2]*256 + cmd[3]),I2C_MODE_2A1D);

                    RTNA_DBG_Str(0, " 2A1D Read Reg[");       
                    RTNA_DBG_Short(0, (cmd[2]*256 + cmd[3]));
                    RTNA_DBG_Str(0, " ] = ");       
                    RTNA_DBG_Byte(0, eu1_get_mmp_result_val[2]);
                    RTNA_DBG_Str(0, "\r\n");
    				
    			    break;
    			
    			case 3:  //write I2C 1A1D
    				//cmd[2] = addr HB
    				//cmd[3] = addr LB
    				//cmd[4] = value
    				USB_WriteI2C((MMP_USHORT)(cmd[2]), (MMP_USHORT)cmd[4],I2C_MODE_1A1D);

                    RTNA_DBG_Str(0, " 1A1D Write Reg[");       
                    RTNA_DBG_Short(0, cmd[2]);
                    RTNA_DBG_Str(0, " ] = ");       
                    RTNA_DBG_Byte(0, cmd[4]);
                    RTNA_DBG_Str(0, "\r\n");

    			    break;
    			    
    			case 4:  //read I2C
    				//cmd[2] = addr HB
    				//cmd[3] = addr LB
    				//cmd[4] = value
    				eu1_get_mmp_result_val[2] = (MMP_UBYTE)USB_ReadI2C((MMP_USHORT)cmd[2],I2C_MODE_1A1D);

                    RTNA_DBG_Str(0, " 1A1D Read Reg[");       
                    RTNA_DBG_Short(0, cmd[2]);
                    RTNA_DBG_Str(0, " ] = ");       
                    RTNA_DBG_Byte(0, eu1_get_mmp_result_val[2]);
                    RTNA_DBG_Str(0, "\r\n");
    				
    			    break;    			
    		}
    	}
    	break;
    default:
        eu1_get_mmp_result_val[0] = EU_ISP_CMD_NOT_SUPPORT;     
        break;
    }
}

void usb_vc_eu1_mmp_cmd16(MMP_UBYTE *cmd)//cmd -> 8byte
{
 #if 0
    RTNA_DBG_Str(0, "mmp cmd 16");
    RTNA_DBG_Byte(0, cmd[0]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[1]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[2]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[3]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[4]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[5]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[6]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[7]);
    RTNA_DBG_Str(0, " ");      
    RTNA_DBG_Byte(0, cmd[8]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Str(0, "\r\n");
#endif
    switch(cmd[0]) {
		case 0x01:  // 
		{
		}
        break;
        case 0x02:	//
		{	
		}
		break;
	case 0x03:
		break;
	case 0x04:				
		{
		}
		break;
	case 0x05:	//Audio Mode
		{
			// CXNT_Initial_Success = 0;
			//USB_SetAudioMode(PCAM_API, cmd[1]);
		}
		break;
	case 0x06:	//M2TS option
		{
		}
		break;
	case 0x08:	//JPEG Quality ,
		break;
    default:
        eu1_get_mmp_result_val[0] = EU_ISP_CMD_NOT_SUPPORT;     
        break;
    }
}
#if 0
void usb_vc_eu1_isp_ex_cmd(MMP_UBYTE *cmd)//cmd -> 8byte
{
	MMP_ULONG Ret=0;
    RTNA_DBG_Str(0, "isp cmd ex");
    RTNA_DBG_Byte(0, cmd[0]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[1]);
    RTNA_DBG_Str(0, " ");       
    RTNA_DBG_Byte(0, cmd[2]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[3]);
    RTNA_DBG_Str(0, " ");
    RTNA_DBG_Byte(0, cmd[4]);
    RTNA_DBG_Str(0, "\r\n");
    
	eu1_get_isp_ex_result_val[0] = 0;
	switch(cmd[0])
	{
	case ISPIQ_GET_PREVIEW_GAIN_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_PREVIEW, ISP_IQ_CHECK_CLASS_GAIN);
		//eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetPreviewGID();
		break;
	case ISPIQ_GET_CAPTURE_GAIN_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_CAPTURE, ISP_IQ_CHECK_CLASS_GAIN);
		//eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetCaptureGID();
		break;
	case ISPIQ_GET_PREVIEW_ENGERY_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_PREVIEW, ISP_IQ_CHECK_CLASS_ENERGY);
		//eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetPreviewEID();		
		break;
	case ISPIQ_GET_CAPTURE_ENGERY_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_CAPTURE, ISP_IQ_CHECK_CLASS_ENERGY);
		//eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetCaptureEID();
		break;
	case ISPIQ_SET_AWB_MODE:
		//ISP_IF_AWB_SetMode(cmd[1]);
		switch (cmd[1]) {
		case 0: // bypass
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_BYPASS);
			break;
		case 1: // auto
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO);
			break;
		case 2: // cloudy
			ISP_IF_AWB_SetColorTemp(7500);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 3: // daylight
			ISP_IF_AWB_SetColorTemp(6500);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 4: // coolwhite
			ISP_IF_AWB_SetColorTemp(3400);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 5: // alight
			ISP_IF_AWB_SetColorTemp(2800);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 6: // fluorescent
			ISP_IF_AWB_SetColorTemp(3400);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 7: // effect
			ISP_IF_AWB_SetColorTemp(6500);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 8: // dawn
			ISP_IF_AWB_SetColorTemp(6500);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		case 9: // sunset
			ISP_IF_AWB_SetColorTemp(6500);
			ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL);
			break;
		}
		break;
	case ISPIQ_GET_PREVIEW_TEMP_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_PREVIEW, ISP_IQ_CHECK_CLASS_COLORTEMP);
		break;
	case ISPIQ_GET_CAPTURE_TEMP_ID:
		eu1_get_isp_ex_result_val[1] = ISP_IF_IQ_GetID(ISP_IQ_CONDITION_CAPTURE, ISP_IQ_CHECK_CLASS_COLORTEMP);
		break;
	case ISPIQ_SELECT_DBG_TABLE:
		{
		
			#if DRAW_TEXT_FEATURE_EN
			MMP_USHORT sheet_no;
			MMP_USHORT table_no;
			sheet_no = cmd[1]+cmd[2]*0x100;
			table_no = cmd[3]+cmd[4]*0x100;
			_sprintf(gDrawTextBuf,"Select Table sheet=%X table=%X\r\n",sheet_no,table_no);
			RTNA_DBG_Str(0,gDrawTextBuf);
			#endif
			
			ISP_IF_DBG_GetTable(cmd[1]+cmd[2]*0x100,cmd[3]+cmd[4]*0x100);
			break;
		}
	case ISPIQ_SET_DBG_TABLE_BY_INDEX:
		{
			ISP_IF_DBG_SetTableByIndex(cmd[1]+cmd[2]*0x100);		
			break;
		}
	case ISPIQ_GET_DBG_TBL_ROWS:
		{
			Ret = ISP_IF_DBG_GetTableRows();
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8)&0xFF;
			break;
		}
	case ISPIQ_GET_DBG_TBL_COLS:
		{
			Ret = ISP_IF_DBG_GetTableCols();
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8)&0xFF;
			break;
		}
	case ISPIQ_GET_DBG_TBL_TYPE:
		{
			eu1_get_isp_ex_result_val[1] = ISP_IF_DBG_GetTableType();
			break;
		}
	case ISPIQ_GET_DBG_TBL_MODE:
		{
			eu1_get_isp_ex_result_val[1] = ISP_IF_DBG_GetTableMode();		
			break;
		}
	case ISPIQ_GET_DBG_TBL_VALUE:
		{
			Ret = ISP_IF_DBG_GetTableValue();
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8)&0xFF;
			eu1_get_isp_ex_result_val[3] = (Ret>>16)&0xFF;
			eu1_get_isp_ex_result_val[4] = (Ret>>24)&0xFF;					
			break;
		}
	case ISPIQ_GET_DBL_TBL_START_ADDR:
		{
			//Ret = ISP_IF_DBG_GetTableStartAddr();
			Ret = ISP_HDM_IF_GetTableDstAddr();
			
			#if DRAW_TEXT_FEATURE_EN
			_sprintf(gDrawTextBuf,"Get DestAddr %X \r\n",Ret);
			RTNA_DBG_Str(0,gDrawTextBuf);
			#endif
			
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = Ret>>8&0xFF;
			eu1_get_isp_ex_result_val[3] = Ret>>16&0xFF;
			eu1_get_isp_ex_result_val[4] = Ret>>24&0xFF;				
			break;
		}
	case ISPIQ_GET_DBG_TBL_APTOOL_ADDR:
		{
			//Ret = ISP_HDM_IF_GetTableDstAddr();
			Ret = ISP_HDM_IF_GetTableApToolBufAddr();
			
			#if DRAW_TEXT_FEATURE_EN
			_sprintf(gDrawTextBuf,"Get ApToolAddr %X \r\n",Ret);
			RTNA_DBG_Str(0,gDrawTextBuf);
			#endif
			
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = Ret>>8&0xFF;
			eu1_get_isp_ex_result_val[3] = Ret>>16&0xFF;
			eu1_get_isp_ex_result_val[4] = Ret>>24&0xFF;			
			break;
		}
	case ISPIQ_GET_DBL_TBL_TOTAL_SIZE:
		{
			Ret = ISP_IF_DBG_GetTableTotalSize();
			eu1_get_isp_ex_result_val[1] = Ret&0xFF;
			eu1_get_isp_ex_result_val[2] = Ret>>8&0xFF;
			eu1_get_isp_ex_result_val[3] = Ret>>16&0xFF;
			eu1_get_isp_ex_result_val[4] = Ret>>24&0xFF;	
			break;
		}
	case ISPIQ_WRITE_MEM_PARAM:
		{
			WriteMemAddr = eu1_set_isp_ex_val[1]
						 + eu1_set_isp_ex_val[2]*0x100
						 + eu1_set_isp_ex_val[3]*0x10000
						 + eu1_set_isp_ex_val[4]*0x1000000;
			WriteMemCount = eu1_set_isp_ex_val[5]
						 + eu1_set_isp_ex_val[6]*0x100
						 + eu1_set_isp_ex_val[7]*0x10000
						 + eu1_set_isp_ex_val[8]*0x1000000;						 						 		
			break;
		}
	case ISPIQ_WRITE_MEM:
		{	
			MMP_UBYTE n;
			MMP_UBYTE wlen;
			if(WriteMemCount>15)
				wlen = 15;
			else
				wlen = WriteMemCount;
			
			WriteMemCount -= wlen;
			
			for(n=0;n<wlen;++n)
			{
				((MMP_BYTE*)WriteMemAddr)[n] = eu1_set_isp_ex_val[n+1];
			}
			WriteMemAddr += wlen;
			break;
		}
	case ISPIQ_READ_MEM_PARAM:
		{
			ReadMemAddr = eu1_set_isp_ex_val[1]
						 + eu1_set_isp_ex_val[2]*0x100
						 + eu1_set_isp_ex_val[3]*0x10000
						 + eu1_set_isp_ex_val[4]*0x1000000;
			ReadMemCount = eu1_set_isp_ex_val[5]
						 + eu1_set_isp_ex_val[6]*0x100
						 + eu1_set_isp_ex_val[7]*0x10000
						 + eu1_set_isp_ex_val[8]*0x1000000;				
			break;
		}
	case ISPIQ_READ_MEM:
		{
			MMP_UBYTE n;
			MMP_UBYTE rlen;
			if(ReadMemCount>15)
				rlen = 15;
			else
				rlen = ReadMemCount;
			
			ReadMemCount -= rlen;
			
			for(n=0;n<rlen;++n)
			{
				 eu1_get_isp_ex_result_val[n+1] = ((MMP_BYTE*)ReadMemAddr)[n];
			}
			ReadMemAddr += rlen;   			
			break;
		}
	case ISPIQ_MOVE_MEM:
		{
			int n;
			MMP_UBYTE *src;
			MMP_UBYTE *dest;
			MMP_ULONG Size;
			
			((MMP_ULONG)src) 	= eu1_set_isp_ex_val[1]
						 	 	+ eu1_set_isp_ex_val[2]*0x100
						 		+ eu1_set_isp_ex_val[3]*0x10000
						 		+ eu1_set_isp_ex_val[4]*0x1000000;
			((MMP_ULONG)dest)  	= eu1_set_isp_ex_val[5]
						 		+ eu1_set_isp_ex_val[6]*0x100
						 		+ eu1_set_isp_ex_val[7]*0x10000
						 		+ eu1_set_isp_ex_val[8]*0x1000000;
			Size = eu1_set_isp_ex_val[9]
				 + eu1_set_isp_ex_val[10]*0x100
				 + eu1_set_isp_ex_val[11]*0x10000
				 + eu1_set_isp_ex_val[12]*0x1000000;
				 
#if DRAW_TEXT_FEATURE_EN
			_sprintf(gDrawTextBuf,"CopyMemory: Src=%x Dst=%x Size=%x \r\n",(MMP_ULONG)src,(MMP_ULONG)dest,(MMP_ULONG)Size);
			RTNA_DBG_Str(0,gDrawTextBuf);
#endif
			
			for(n=0;n<Size;++n)
			{
				dest[n] = src[n];
			}
			
#if DRAW_TEXT_FEATURE_EN
			_sprintf(gDrawTextBuf,"src[0-4]=%X,%X,%X,%X\r\n",src[0],src[1],src[2],src[3]);
			RTNA_DBG_Str(0,gDrawTextBuf);
			_sprintf(gDrawTextBuf,"dest[0-4]=%X,%X,%X,%X\r\n",dest[0],dest[1],dest[2],dest[3]);
			RTNA_DBG_Str(0,gDrawTextBuf);
#endif			
							
			break;
		}
	case ISPIQ_SET_AE_ISO:
		{
			ISP_IF_AE_SetISO(cmd[1]);
			break;
		}
		
	case ISPIQ_GET_AWB_GAIN:
		{
			Ret = ISP_IF_AWB_GetGainR();
			eu1_get_isp_ex_result_val[1] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8) & 0xFF;
			Ret = ISP_IF_AWB_GetGainG();
			eu1_get_isp_ex_result_val[3] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[4] = (Ret>>8) & 0xFF;
			Ret = ISP_IF_AWB_GetGainB();
			eu1_get_isp_ex_result_val[5] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[6] = (Ret>>8) & 0xFF;
			break;
		}
	
	case ISPIQ_GET_LIGHT_COND_VAL:
		{
			Ret = ISP_IF_AE_GetLightCond();
			eu1_get_isp_ex_result_val[1] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8) & 0xFF;
			eu1_get_isp_ex_result_val[3] = (Ret>>16) & 0xFF;
			eu1_get_isp_ex_result_val[4] = (Ret>>24) & 0xFF;
			break;
		}
	
	case ISPIQ_GET_AE_ENERGY:
		{
			Ret = ISP_IF_AE_GetPreEnergy();
			eu1_get_isp_ex_result_val[1] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8) & 0xFF;
			eu1_get_isp_ex_result_val[3] = (Ret>>16) & 0xFF;
			eu1_get_isp_ex_result_val[4] = (Ret>>24) & 0xFF;
			break;
		}
		
	case ISPIQ_SET_IQ_SWITCH:
		{
			ISP_IF_IQ_SetSwitch(cmd[1], cmd[2]);
			break;
		}
		
	case ISPIQ_GET_AE_BASE_SHUTTER:
		{
 			Ret = ISP_IF_AE_GetBaseShutter();
 			eu1_get_isp_ex_result_val[1] = Ret & 0xFF;
			eu1_get_isp_ex_result_val[2] = (Ret>>8) & 0xFF;
			eu1_get_isp_ex_result_val[3] = (Ret>>16) & 0xFF;
			eu1_get_isp_ex_result_val[4] = (Ret>>24) & 0xFF;
			break;
		}
		
	case ISPIQ_SET_SCENE_MODE:
		{
			ISP_IF_F_SetScene(cmd[1]);
			break;
		}
	//add by casio
	case ISPIQ_SET_COLOR_TRANSFORM:
		 
		{
			//ISP_IF_IQ_SetColorTransform(1);
			ISP_IF_IQ_SetColorTransform(cmd[1]);
			Ret = 2;
			eu1_get_isp_ex_result_val[1] = cmd[1];
			break;
		}
		
	}
}
#endif

void usb_vc_eu1_cs_out(void)
{
    MMP_UBYTE cs,len;
    MMP_USHORT i;
    //MMP_ULONG  tmp2 = 0;
    //MMP_ULONG  tmp4 = 0;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;  


    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len =gsUsbCtrlReq.wLength;// UsbRequestPayload_wLength;
    switch(cs) {
    case EU1_SET_ISP:
        USBOutData = 0; 
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_set_isp_val,sizeof(eu1_set_isp_val)) ;
        usb_vc_eu1_isp_cmd(eu1_set_isp_val);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;

    case EU1_GET_ISP_RESULT:
        USBOutData = 0;    
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_get_isp_result_val,sizeof(eu1_get_isp_result_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    case EU1_SET_FW_DATA:
        //set fw data
         {
            MMP_UBYTE FWDataBuf[EU1_SET_FW_DATA_LEN];
            USBOutData = 0;           
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,FWDataBuf,sizeof(FWDataBuf)) ;
            USB_DownloadFWData(FWDataBuf, EU1_SET_FW_DATA_LEN);
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
        }break;
    case EU1_SET_MMP:
        //
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_set_mmp_val,sizeof(eu1_set_mmp_val)) ;
        usb_vc_eu1_mmp_cmd(eu1_set_mmp_val);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
    case EU1_GET_MMP_RESULT:
        USBOutData = 0;    
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_get_mmp_result_val,sizeof(eu1_get_mmp_result_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();         
        break;    

   case EU1_SET_MMP_CMD16:
        //
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_set_mmp_cmd16_val,sizeof(eu1_set_mmp_cmd16_val)) ;
        usb_vc_eu1_mmp_cmd16(eu1_set_mmp_cmd16_val);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;
        
    case EU1_GET_MMP_CMD16_RESULT:
        //
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_get_mmp_cmd16_result_val,sizeof(eu1_get_mmp_cmd16_result_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;        
        
    case EU1_SET_ISP_EX:
        USBOutData = 0; 
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_set_isp_ex_val,sizeof(eu1_set_isp_ex_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;

    case EU1_GET_ISP_EX_RESULT:
        USBOutData = 0;    
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_get_isp_ex_result_val,sizeof(eu1_get_isp_ex_result_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    case EU1_WRITE_MMP_MEM:
    	{	
			MMP_UBYTE n;
			MMP_UBYTE wlen;
			MMP_UBYTE Buf[EU1_WRITE_MMP_MEM_LEN];
			
            USB_GET_FIFO_DATA(MMPF_USB_EP0,len,Buf,sizeof(Buf)) ;
			
			if(WriteMemCount>EU1_WRITE_MMP_MEM_LEN)
				wlen = EU1_WRITE_MMP_MEM_LEN;
			else
				wlen = WriteMemCount;
			
			WriteMemCount -= wlen;
			
			for(n=0;n<wlen;++n)
			{
				((MMP_BYTE*)WriteMemAddr)[n] = Buf[n];
			}
			WriteMemAddr += wlen;
			
			USBOutData = 0;
			USBInPhase = SET_CUR_CMD;
			usb_ep0_null_in();
			break;
		}
    case EU1_GET_CHIP_INFO:
        USBOutData = 0;    
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,eu1_get_chip_info_val,sizeof(eu1_get_chip_info_val)) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();         
        break;    
	case EU1_SET_DATA_32:
		{
		//MMP_UBYTE tmp[EU1_SET_DATA_32_LEN];
        USBOutData = 0;    
        //USB_GET_FIFO_DATA(MMPF_USB_EP0,len,tmp,sizeof(tmp)) ;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,(MMP_UBYTE *)SERIAL_NUMBER_WRITE_BUF_STA,32) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
		} 

    case XU_CONTROL_UNDEFINED:
    default:
        // impossible flow       
        break;
    }
}


void usb_vc_eu2_cs_out(void)
{
    MMP_UBYTE cs,len, i,*ptr;
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;  
   // MMP_UBYTE tmp_buf[9];   
    MMP_UBYTE tmp;
    
    cs = (/*UsbRequestPayload_wValue*/gsUsbCtrlReq.wValue >> 8);
    len = gsUsbCtrlReq.wLength;//UsbRequestPayload_wLength;
    
    if(len==0) {
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbVCERRCode=CONTROL_UNKNOWN;     
        return ;
    }
    
#if USB_UVC_H264==1
    switch(cs) {
    case UVCX_RATE_CONTROL_MODE:  // 3.4.1.1 Encoder Video Format Control
    {
        MMP_UBYTE ratectl_byte[UVCX_RATECONTROLMODE_LEN] ;
        UVCX_RateControlMode *ratectl;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,ratectl_byte, UVCX_RATECONTROLMODE_LEN );
        ratectl = (UVCX_RateControlMode *)ratectl_byte ;
        
        if( (ratectl->bRateControlMode <= RESERVED_H264) || (ratectl->bRateControlMode >= NUM_H264_CTLMODE) ) {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
             gbVCERRCode=CONTROL_OUT_OF_RANGE ;
             break ;    
        } else {
            UVCX_SetH264ControlMode((UVCX_RateControlMode *)ratectl);
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    }
    case UVCX_TEMPORAL_SCALE_MODE: // 3.4.1.2 Encoder Configuration Control
    {
        MMP_UBYTE tempscale_byte[UVCX_TEMPORALSCALEMODE_LEN] ;
        UVCX_TemporalScaleMode *tempScaleMode ;
        USBOutData = 0;
        // Do something here
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,tempscale_byte, UVCX_TEMPORALSCALEMODE_LEN );
        
        tempScaleMode = (UVCX_TemporalScaleMode *)tempscale_byte;
        
        if( (tempScaleMode->bTemporalScaleMode < ONE_LAYER_H264) || (tempScaleMode->bTemporalScaleMode > SEVEN_LAYER_H264) ) {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
             gbVCERRCode=CONTROL_OUT_OF_RANGE;
             break ;    
        } else {
            UVCX_SetH264TemporalScaleMode(tempScaleMode ) ;
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    }    
    case UVCX_SPATIAL_SCALE_MODE:                  // 3.4.1.3 Rate Control
    {
        MMP_UBYTE spatialscale_byte[UVCX_SPATIALSCALEMODE_LEN] ;
        UVCX_SpatialScaleMode *spatialScaleMode ;
         
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,spatialscale_byte, UVCX_SPATIALSCALEMODE_LEN);
        spatialScaleMode =  (UVCX_SpatialScaleMode *)spatialscale_byte;
        if( (spatialScaleMode->bSpatialScaleMode < ONE_SPATIAL_LAYER) || (spatialScaleMode->bSpatialScaleMode  > EIGHT_SPATIAL_LAYER) ) {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
             gbVCERRCode=CONTROL_OUT_OF_RANGE;
             break ;    
        } else {
            UVCX_SetH264SpatialScaleMode(spatialScaleMode) ;
        }
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    }
    case UVCX_SNR_SCALE_MODE:            // 3.4.1.4 Frame Type Control
    {
        MMP_UBYTE snrscale_byte[UVCX_SNRSCALEMODE_LEN] ;
        UVCX_SNRScaleMode *snrScaleMode ;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,snrscale_byte, UVCX_SNRSCALEMODE_LEN );
        snrScaleMode =  (UVCX_SNRScaleMode *)snrscale_byte ;
        if( snrScaleMode->bSNRScaleMode != 0 ) {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
             gbVCERRCode=CONTROL_OUT_OF_RANGE;
             break ;    
        } else {
            UVCX_SetH264SNRScaleMode( snrScaleMode ) ;
        }
        usb_ep0_null_in();      
        break;
    }
    case UVCX_BITRATE_LAYERS:          // 3.4.1.5 Camera Delay Control
    {
        MMP_UBYTE bitrate[UVCX_BITRATE_LEN] ;
        
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,bitrate, UVCX_BITRATE_LEN);
        UVCX_SetH264BitRate( (UVCX_BitRateLayers *)bitrate );
        // Do something here
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break;
    }    
    case UVCX_QP_STEPS_LAYERS:                 // Filter Delay Control
    {
        MMP_UBYTE qstep[UVCX_QPSTEP_LAYER_LEN] ;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,qstep, UVCX_QPSTEP_LAYER_LEN);
        UVCX_SetH264QPStepsLayers( (UVCX_QPStepsLayers *)qstep );
        // Do something here
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break ;                    
    }
    case UVCX_PICTURE_TYPE_CONTROL:
    {
        //MMP_USHORT pict_type ;
        UVCX_PictureTypeControl pict_type;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,(MMP_UBYTE *)&pict_type, UVCX_PICTYPE_LEN );
        
        if( (pict_type.wPicType != PIC_IDR_FRAME) && (pict_type.wPicType != PIC_IDR_WITH_NEW_SPSPPS) ) {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode=CONTROL_OUT_OF_RANGE;
            break ;    
        } else {
            UVCX_SetH264PictureTypeCtl(&pict_type);
        }        
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break ;                    
    }
    /*
    case UVCX_CROP_CONFIG:
    {
        MMP_UBYTE crop[UVCX_CROP_CONFIG_LEN] ;
        USBOutData = 0;
        USB_GET_FIFO_DATA(USB_EP0_FIFO_B,len,crop, UVCX_CROP_CONFIG_LEN);
        UVCX_SetH264CropConfig( (UVCX_CropConfig *)crop);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break ;                    
    }
    */
    case UVCX_FRAMERATE_CONFIG:
    {
        //MMP_ULONG fr ;
        UVCX_FrameRateConfig fr;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,(MMP_UBYTE *)&fr, UVCX_FRAMERATE_LEN );
        UVCX_SetH264FrameRate(&fr);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break ;                    
        
    }   
    case UVCX_VERSION:
    {
        MMP_USHORT ver;
        USBOutData = 0;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,(MMP_UBYTE *)&ver, 2);
        gsUVCXVersion = ver ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();      
        break ;                    
    }  
    case UVCX_VIDEO_ADVANCE_CONFIG:
    {
        MMP_UBYTE advcfg[UVCX_ADVCONFIG_LEN];
        USBOutData = 0 ;
        USB_GET_FIFO_DATA(MMPF_USB_EP0,len,advcfg, UVCX_ADVCONFIG_LEN);
        UVCX_SetH264AdvConfig( (UVCX_AdvConfig *)advcfg ) ;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();  
        break; 
    }
    case XU_CONTROL_UNDEFINED:
    default:
        // impossible flow       
        break;
    }
#endif
    
}


void usb_vc_dump_vsi_info(MMP_UBYTE if_id)
{
    // dbg_printf(3,"vs(if,fmt,frame)index : (%d,%d,%d)\r\n",if_id,gCurVsiInfo[if_id-1].bFormatIndex,gCurVsiInfo[if_id-1].bFrameIndex);
}

void usb_vc_vsi_cs_out(void)
{
#if 1
    MMP_UBYTE cs,if_id,maxframeindex = 0;
    MMP_USHORT i;
    MMP_USHORT *tmpptr;
    MMP_UBYTE *tmpptrb;
    MMP_ULONG VcPayloadSize;
    MMP_ULONG frameInterval = 0;
    STREAM_SESSION *ss ;
    
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;    

    cs = (/*UsbRequestPayload_wValue */gsUsbCtrlReq.wValue>> 8);
    if_id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) ;
    
    gCurVsiInfo[if_id-1].bInterface = if_id ;
#if 1
    dbg_printf(3,"vsi.cs_out - cs :%d, if_id :%d\r\n",cs,if_id );
    
#endif
    
    
    switch(cs) {
    case VS_PROBE_CONTROL:
        tmpptr = (MMP_USHORT *) &vpin;
        USBOutData = 0;           
        #if 0
        for(i = 0;i < /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength / 2;i++) {
            *(tmpptr + i)  = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
            *(tmpptr + i) |= pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B << 8;
        }
        #else
        USB_GET_FIFO_DATA(MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,sizeof(vpin) );
        #endif
        
        gCurVsiInfo[if_id-1].bFormatIndex =LOBYTE( vpin.wIndex ) ;
        gCurVsiInfo[if_id-1].bFrameIndex = HIBYTE( vpin.wIndex );
        
        if( (gCurVsiInfo[if_id-1].bFormatIndex==0) || (gCurVsiInfo[if_id-1].bFormatIndex > TOTAL_NUM_VIDEO_FORMAT ) ) {
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_OUT_OF_RANGE;
            RTNA_DBG_Str(0, " ** VS_PROBE_CONTROL stall\r\n"); 
            break;
        }
        if(gCurVsiInfo[if_id-1].bFormatIndex == YUY2_FORMAT_INDEX) {
            maxframeindex = NUM_YUY2_VIDEO ;    
        }
        else if(gCurVsiInfo[if_id-1].bFormatIndex == MJPEG_FORMAT_INDEX) {
            maxframeindex = NUM_MJPG_VIDEO ;
        }
        else if(gCurVsiInfo[if_id-1].bFormatIndex == FRAME_BASE_H264_INDEX) {
            maxframeindex = NUM_H264_VIDEO ;
        }
        else if(gCurVsiInfo[if_id-1].bFormatIndex == NV12_FORMAT_INDEX) {
            maxframeindex = NUM_NV12_VIDEO ;
        }
#if 1
        dbg_printf(3,"probe:\r\n");
        usb_vc_dump_vsi_info(gCurVsiInfo[if_id-1].bInterface);
#endif                
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;

    case VS_COMMIT_CONTROL:
        tmpptr = (MMP_USHORT *) &vpin;
        USBOutData = 0;           
        #if 0
        for(i = 0;i < /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength / 2;i++) {
            *(tmpptr + i) = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
            *(tmpptr + i) |= pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B << 8;
        }
        #else
        USB_GET_FIFO_DATA(MMPF_USB_EP0,gsUsbCtrlReq.wLength,(MMP_UBYTE *)tmpptr,sizeof(vpin) );
        #endif

        if( ( HIBYTE(vpin.wIndex) != gCurVsiInfo[if_id-1].bFrameIndex ) && (LOBYTE(vpin.wIndex) != gCurVsiInfo[if_id-1].bFormatIndex ) )
        {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_WRONG_STATE;
            RTNA_DBG_Str(0, " ** VS_COMMIT_CONTROL stall\r\n"); 
            break;
        }
        
        gCurVsiInfo[if_id-1].bFormatIndex =LOBYTE( vpin.wIndex ) ;
        gCurVsiInfo[if_id-1].bFrameIndex = HIBYTE( vpin.wIndex );
        
 
#if 1
        dbg_printf(3,"commit:\r\n");
        usb_vc_dump_vsi_info(gCurVsiInfo[if_id-1].bInterface);
#endif                
 
        frameInterval = ( vpin.wIntervalHi << 16 ) | vpin.wIntervalLo ;
        //gbCurFrameRate[if_id-1] = ( 10000000 / frameInterval); 
        gsCurFrameRatex10[if_id-1] = ( (10000000*10) / frameInterval); 
       
        usb_vc_get_video_probe(if_id,gCurVsiInfo[if_id-1].bFormatIndex ,gCurVsiInfo[if_id-1].bFrameIndex ,gsCurFrameRatex10[if_id-1],1);
        
        VcPayloadSize = (MMP_ULONG)(vpin.wMaxPayloadSizeLo + ((MMP_ULONG)vpin.wMaxPayloadSizeHi << 16));
        if(VcPayloadSize != 0) {
            if(VcPayloadSize > (vp.wMaxPayloadSizeLo + ((MMP_ULONG)vp.wMaxPayloadSizeHi << 16)))// BW over current
            {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   
                RTNA_DBG_Str(0, " ** VS_COMMIT_CONTROL stall\r\n"); 
                break;
            } 
            else
            {
            
                
//#ifdef UVC_TEST
#if USB_UVC_BULK_EP			
                packetcount = 0;
                frametoggle = 0x00;           
                gbvcstart = 1;
                gbdrawflag = 0;
                ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                //USB_LEDDisplay(LED_READY, LED_ON);

#if 1  // for skype linux bulk test
                if(gbCheckSystemInitFlag >= 2){    // tomy@2010_09_01, to fix Skype preview issue via BULK transfer on some linux OS
    			    usb_uvc_start_preview();
                }
#else                
    			usb_uvc_start_preview();
#endif    			
			
#else
                ss = MMPF_Video_GetStreamSessionByEp(if_id-1);
                ss->tx_flag = SS_TX_COMMITED ;
                // AIT test code , not for customer
                if((UVC_VCD()==bcdVCD15)&&(gCurVsiInfo[if_id-1].bFormatIndex == FRAME_BASE_H264_INDEX)) {
                    static int _init_commit = 0 ;
                    if(_init_commit==0) {
                        _init_commit = 1; 
                        UVC15_CommitH264FrameDesc(0,0);
                        UVC15_CommitH264FrameDesc(1,0);
                        UVC15_CommitH264CommitDesc(0);
                        dbg_printf(0,"commit-frame-desc\r\n");
                    }
                }
#endif
                USBInPhase = SET_CUR_CMD;
                usb_ep0_null_in();
            }
        } 
        else {
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
        }
        break;
#if 1// Not support in this project        
    case VS_STILL_PROBE_CONTROL:
        tmpptrb = (MMP_UBYTE *) &spin;
        USBOutData = 0;           

        for(i = 0;i < /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength;i++) {
            *(tmpptrb + i) = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        }

        if(spin.bFormatIndex > TOTAL_NUM_VIDEO_FORMAT || spin.bFormatIndex < 1){
            // un-support
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_OUT_OF_RANGE;
            RTNA_DBG_Str(0, " ** VS_STILL_PROBE_CONTROL stall\r\n"); 
            break;
        }
        

       	#if ENABLE_YUY2==1
       	if(spin.bFormatIndex == YUY2_FORMAT_INDEX){//YUY2

            if(spin.bFrameIndex > NUM_YUY2_STILL || spin.bFrameIndex < 1) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;
                RTNA_DBG_Str(0, " ** VS_STILL_PROBE_CONTROL stall\r\n"); 
                break;
            }
      	}
       	#endif
        	

       	#if ENABLE_MJPEG==1
       	if(spin.bFormatIndex == MJPEG_FORMAT_INDEX){//MJPG
             if(spin.bFrameIndex > NUM_MJPG_STILL || spin.bFrameIndex < 1) {
                // un-support
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_OUT_OF_RANGE;
                RTNA_DBG_Str(0, " ** VS_STILL_PROBE_CONTROL stall\r\n"); 
                break;
            }
       	}
       	#endif

		usb_vc_set_still_probe(spin.bFormatIndex,spin.bFrameIndex);
        // UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
        gbCurStillFormatIndex = spin.bFormatIndex;
        gbCurStillFrameIndex = spin.bFrameIndex;
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();
        break;

    case VS_STILL_COMMIT_CONTROL:

        tmpptrb = (MMP_UBYTE *) &spin;
        USBOutData = 0;           

        for(i = 0;i < /*UsbRequestPayload_wLength*/gsUsbCtrlReq.wLength;i++) {
            *(tmpptrb + i) = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;
        }
        if(spin.bFrameIndex != gbCurStillFrameIndex) // wrong frame mode
        {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_WRONG_STATE;
            RTNA_DBG_Str(0, " ** VS_STILL_COMMIT_CONTROL stall\r\n"); 
            break;
        }

        VcPayloadSize = (MMP_ULONG)(spin.bMaxPayloadSize[0] + ((MMP_ULONG)spin.bMaxPayloadSize[1] << 8) 
                                   +  ((MMP_ULONG)spin.bMaxPayloadSize[2] << 16) +  ((MMP_ULONG)spin.bMaxPayloadSize[3] << 24) );
        if(VcPayloadSize != 0) {
            if(VcPayloadSize > (sp.bMaxPayloadSize[0] + ((MMP_ULONG)sp.bMaxPayloadSize[1] << 8) 
                               +  ((MMP_ULONG)sp.bMaxPayloadSize[2] << 16) +  ((MMP_ULONG)sp.bMaxPayloadSize[3] << 24) ))// BW over current
            {
                UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
                gbVCERRCode = CONTROL_WRONG_STATE;   
                RTNA_DBG_Str(0, " ** VS_STILL_COMMIT_CONTROL stall\r\n"); 
                break;
            } 
            else {
 		        usb_vc_set_still_commit(&spin);
                //                UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
                USBInPhase = SET_CUR_CMD;
                usb_ep0_null_in();
            }
        } 
        else {
            //                 UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
            USBInPhase = SET_CUR_CMD;
            usb_ep0_null_in();
        }
        break;
        
    case VS_STILL_IMAGE_TRIGGER_CONTROL:
        USBOutData = 0;           
        gbTriOP = pUSB_CTL->USB_FIFO_EP[0x0].FIFO_B;                   

        //                 UsbWriteEp0CSR(SET_EP0_SERVICED_RXPKTRDY);
        USBInPhase = SET_CUR_CMD;
        usb_ep0_null_in();  

        //usb_vc_preview_disable();

        // sti-image
        //usb_vc_send_image(1); 
        //           EnableDMA();
        //           IBC_Enable();
		/*
		gbvcstart = 0;

        USB_REG_BASE_B[USB_DMA_INTSTS_B] = 0;
        gbstistart = 1;          
        */
        gbStillCaptureEvent = STILL_IMAGE_TRIGGER;
        
        break;
#endif        
    case VS_CONTROL_UNDEFINED:
    case VS_STREAM_ERROR_CODE_CONTROL:
    case VS_GENERATE_KEY_FRAME_CONTROL:
    case VS_UPDATE_FRAME_SEGMENT_CONTROL:
    case VS_SYNCH_DELAY_CONTROL:
    default:
        // impossible flow       
        break;
    }
#endif
}


void usb_vc_out_data(void)
{
    MMP_UBYTE id;

    id = (/*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex >> 8);

    
    if(id == 0x00) {
    
        MMP_UBYTE if_id = ( /*UsbRequestPayload_wIndex*/gsUsbCtrlReq.wIndex & 0xFF) ;

        if( if_id == gbVCVSIF1) {
            usb_vc_vsi_cs_out();
        } 
        // Possible to seperate 2 vsi_cs_out
        else if(if_id == gbVCVSIF2) {
            usb_vc_vsi_cs_out();
        }
        else if(if_id  == gbVCVCIF) {
            usb_vc_vci_cs_out();
        } else {
            UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
            gbVCERRCode = CONTROL_INVALID_UNIT;
        }
    } else if(id == gbVCITID) {
        usb_vc_ct_cs_out();
    }
#if 0     
    else if(id == gbVCOTID) 
    {
    }
    else if(id == gbVCSUID) {
        usb_vc_su_cs_out();
    }
#endif     
    else if(id == gbVCPUID) {
     
        usb_vc_pu_cs_out();
    }
#if USB_UVC_SKYPE
    else if(id == gbVCEU0ID) {
        usb_vc_eu0_cs_out();
    }
#endif
    else if(id == gbVCEU1ID) {
        usb_vc_eu1_cs_out();
    } else if(id == gbVCEU2ID) {
        usb_vc_eu2_cs_out();
    }
    else {
        UsbWriteEp0CSR(SET_EP0_SENDSTALL | SET_EP0_SERVICED_RXPKTRDY);
        gbVCERRCode = CONTROL_INVALID_UNIT;
    }
}

MMP_USHORT usb_vc_cur_fpsx10(MMP_UBYTE ep_id)
{
    return gsCurFrameRatex10[ep_id] ;
}

MMP_ULONG usb_vc_cur_sof(void)
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    SOFNUM = pUSB_DMA->USB_SOF_CNT >> 3 ;
    return SOFNUM ;
}

MMP_ULONG usb_vc_diff_sof(MMP_ULONG t1)
{
    return (SOFNUM - t1) ;
}



MMP_ULONG usb_vc_cur_usof(void)
{
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    uSOFNUM = pUSB_DMA->USB_SOF_CNT ;
    return uSOFNUM ;
//    return uSOFNUM ;
}

MMP_ULONG usb_vc_diff_usof(MMP_ULONG t1)
{
    usb_vc_cur_usof();
    return (uSOFNUM - t1) ;
}

int sys_task_info(void)
{
    OS_TCB task;
    INT8U err,i;
    char  *s ="Ready";
    
    
    //dbg_printf(3,"TaskName   Prio   Stat                StkBottom   StkSize\r\n");
  
                
    for (i=0;i<=OS_LOWEST_PRIO;i++) {
        err = OSTaskQuery(i,&task);
        
        if(err==OS_NO_ERR) {
            switch(task.OSTCBStat)
            {
            case OS_STAT_RDY:
                s = "Ready";
                break ;
            case OS_STAT_SEM:
                s = "Wait Sem";
                break ;
            case OS_STAT_MBOX:
                s = "Wait MBox" ;
                break ;
            case OS_STAT_Q:
                s = "Wait Queue";
                break;
            case OS_STAT_SUSPEND:
                s = "Wait Suspend";
                break;
            case OS_STAT_MUTEX:
                s = "Wait Mutex";
                break;
            case OS_STAT_FLAG:
                s = "Wait Flag";
                break;
                
            }
            /*dbg_printf(3,"%s   %d   %x(%s)    %x    %x\r\n",
            task.OSTCBTaskName,
            task.OSTCBPrio,
            task.OSTCBStat,s,
            task.OSTCBStkBottom,
            task.OSTCBStkSize
            );*/
        }
    }
    //dbg_printf(0,"CPU Usage : %d %\r\n",OSCPUUsage);
    return 0 ;
}

void cali_time_loop(void)
{
extern MMP_ULONG RTNA_CPU_CLK_M ;
#define BASE_MHZ 100
    AITPS_GBL pGBL = AITC_BASE_GBL;
    MMP_ULONG t0,t1 ;
    MMP_ULONG ms = 1000 ,loop ;
//    pGBL->GBL_CLK_EN |= GBL_CLK_ALL;
//    pGBL->GBL_CLK_EN_2 |= GBL_CLK_ALL;

    loop = ms * BASE_MHZ * 1000 / WHILE_CYCLE ;//* 1000 ;
     dbg_printf(3,"CPU MHz : %d, 1 sec loop(based on 100MHz): %d\r\n",RTNA_CPU_CLK_M ,loop);
      
    t0 = usb_vc_cur_sof();
    RTNA_Wait_Count(loop);
    t1 = usb_vc_diff_sof(t0);
    
    dbg_printf(3,"Delay 1 sec , sof elapsed %d ms\r\n",t1);
}

extern MMP_UBYTE    os_created_task_prio[];
extern MMP_UBYTE    os_created_tasks ;
void check_task_stack_init(void)
{
    OS_TCB task;
    MMP_UBYTE err ,i ,prio;
    //dbg_printf(3,"check stack on\r\n");
    
    for (i=0;i<os_created_tasks;i++) {
        prio = os_created_task_prio[i] ;
        err = OSTaskQuery(prio,&task) ;
        if(err==OS_NO_ERR) {
            task.OSTCBStkBottom[0]=0xdeadbeef ;   
        }
    } 
    
}

void check_task_stack_overflow(void)
{
    OS_TCB task;
    MMP_UBYTE err ,i ,prio;
    for (i=0;i<os_created_tasks;i++) {
        prio = os_created_task_prio[i] ;
        err = OSTaskQuery(prio,&task) ;
        if(err==OS_NO_ERR) {
            if( task.OSTCBStkBottom[0]!=0xdeadbeef  ) {
                dbg_printf(3,"#Task:%s stack overflow!\r\n",task.OSTCBTaskName );
                while(1);
            }   
        }
    }
    
}


void power_on_off_test(int count)
{
extern volatile MMPF_BOOTINFO *gsBootInfo;
    int i;
 
    //if(gsBootInfo->sig==VALID_BOOTINFO_SIG) {
    //    if(gsBootInfo->flag & FLAG_ONFF_TIMES_MANUAL) {
    //        count = gsBootInfo->onff_test_times ;
    //    }
    //}

    dbg_printf(3,"#Power on test start,count : %d\r\n",count);
    for(i=0;i<count;i++) {
        USB_LEDDisplay(LED_READY, LED_ON);
        MMPF_OS_Sleep(5000);
        USB_LEDDisplay(LED_READY, LED_OFF);
        MMPF_OS_Sleep(5000);
        dbg_printf(3,"#Power on test : %d times\r\n",count);
    }
}


#if CPU_EXCEPTION_LOG==1
#define BOOTDATA_FLASH_ADDR (256 * 1024 - 256 )
void read_cpu_exception(MMP_ULONG *lr,MMP_ULONG *sp)
{
    
    MMP_ULONG *log_addr ;
    MMPF_SYS_InitFB(0);
   // log_addr = ( MMP_ULONG *)MMPF_SYS_AllocFB("CPU",256,32) ;
   // MMPF_SIF_Init();
   // MMPF_SIF_ReadData(BOOTDATA_FLASH_ADDR,log_addr,256);
    log_addr = ( MMP_ULONG *)(0x00180000 - 8) ;
    *lr = log_addr[0];
    *sp = log_addr[1] ;
    dbg_printf(0,"Rd(LR,SP)=(%x,%x)\r\n",*lr,*sp);
}

void write_cpu_exception(MMP_ULONG lr,MMP_ULONG sp)
{
   // int i ;
    MMP_ULONG *log_addr ;
    MMPF_SYS_InitFB(0);
    log_addr = ( MMP_ULONG *)(0x00180000 - 8) ;
    log_addr[0] = lr ;
    log_addr[1] = sp ;
   // MMPF_SIF_Init();
   // MMPF_SIF_WriteData(BOOTDATA_FLASH_ADDR,log_addr,256);
    dbg_printf(0,"Wr(LR,SP)=(%x,%x)\r\n",lr,sp);
}
#endif




#endif


