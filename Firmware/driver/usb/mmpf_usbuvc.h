#ifndef USBUVC_H
#define USBUVC_H
#include "config_fw.h"

#include "mmpf_typedef.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usb_h264.h"
#include "mmpf_dma.h"
#include "mmpf_usbvend.h"

#if USB_UVC_SKYPE
#include "mmpf_usbskypeh264.h"
#endif

#define bcdVCD10 0x0100
#define bcdVCD15 0x0150

#define char2int32(x,y,u,v)			(unsigned long) (((unsigned long)x)<<24)|(((unsigned long)y)<<16)|(((unsigned long)u)<<8)|((unsigned long)(v))

typedef __packed struct
{
    MMP_USHORT bmHint;                                  //
    MMP_USHORT wIndex; 									// H byte -> Format Index, L Byte -> Frame Index 
    MMP_USHORT wIntervalLo;                             //
    MMP_USHORT wIntervalHi;                             //
    MMP_USHORT wKeyFrameRate;                           //
    MMP_USHORT wPFrameRate;                             //
    MMP_USHORT wCompQuality;                            //
    MMP_USHORT wCompWinSize;                            //
    MMP_USHORT wDelay;                                  //
    MMP_USHORT wMaxFrameSizeLo;                         //
    MMP_USHORT wMaxFrameSizeHi;                         //
    MMP_USHORT wMaxPayloadSizeLo;                       //
    MMP_USHORT wMaxPayloadSizeHi;                       //
	                                                    //
	//-----------------------------------//
	//-		UVC 1.1 starts from here	-//
	//-----------------------------------//
	MMP_USHORT dwClockFrequencyLo;                      //
	MMP_USHORT dwClockFrequencyHi;                      //
	MMP_UBYTE  bmFramingInfo;                           //
	MMP_UBYTE  bPreferedVersion;                        //
	MMP_UBYTE  bMinVersion;                             //
	MMP_UBYTE  bMaxVersion;                             //
	                                                    //
	//-----------------------------------//
	//-		UVC 1.5 starts from here	-//
	//-----------------------------------//
	MMP_UBYTE  bUsage;                                  //
	MMP_UBYTE  bBitDepthLuma;                           // Represents bit_depth_luma_minus8 + 8, which must be the same as bit_depth_chroma_minus8 + 8.
	MMP_UBYTE  bmSetting;                               // bmSetting
	MMP_UBYTE  bMaxNumberOfRefFramesPlus1;              // Host indicates the maximum number of frames stored for use as references.
	MMP_USHORT bmRateControlModes;                      // This field contains 4 subfields, each of which is a 4 bit number.
	MMP_USHORT bmLayoutPerStream[4];                    // SVC layering structure for simulcast stream with stream_id 0.
} VIDEO_PROBE;


typedef struct
{
    MMP_UBYTE bFormatIndex;
    MMP_UBYTE bFrameIndex;
    MMP_UBYTE bCompressIndex;
    MMP_UBYTE bMaxFrameSize[4];
    MMP_UBYTE bMaxPayloadSize[4];
} STILL_PROBE;

typedef struct _VC_CMD_CFG
{
    MMP_ULONG  bCmdCap;
    MMP_UBYTE  bInfoCap;
    MMP_USHORT bCmdLen ;
    MMP_UBYTE  bInfoLen;
    /*If cmd > 4, the following data is a pointer*/
    MMP_ULONG  dwResVal;
    MMP_ULONG  dwDefVal;
    MMP_ULONG  dwMinVal;
    MMP_ULONG  dwMaxVal;
    MMP_ULONG  dwCurVal;
} VC_CMD_CFG ;

#define BFH_FID     (0)
#define BFH_EOF     (1)
#define BFH_PTS     (2)
#define BFH_SCR     (3)
#define BFH_RES     (4)
#define BFH_EOS     (4) // UVC1.5
#define BFH_STI     (5)
#define BFH_ERR     (6)
#define BFH_EOH     (7)
#define BFH_BIT(x)  ( 1 << x )

typedef __packed struct _UVC_VIDEO_PAYLOAD_HEADER
{
    MMP_UBYTE bHLE;
    MMP_UBYTE bBFH;
    MMP_UBYTE bPTS[4] ;
    MMP_UBYTE bSCR[4] ;
    MMP_UBYTE bSOFNUM[2] ;
#if H264_SIMULCAST_EN
    MMP_UBYTE bSLI[2] ;
    MMP_UBYTE bAlign4[2];
#endif    
} UVC_VIDEO_PAYLOAD_HEADER;

#if H264_SIMULCAST_EN
extern MMP_USHORT UVC_VIDEO_PAYLOAD_HEADER_LEN[2] ;
#else
#define UVC_VIDEO_PAYLOAD_HEADER_LEN sizeof(UVC_VIDEO_PAYLOAD_HEADER)
#endif

typedef struct _UVC_VSI_INFO
{
    MMP_UBYTE bInterface  ;
    MMP_UBYTE bFormatIndex;
    MMP_UBYTE bFrameIndex ;
}  UVC_VSI_INFO ; 


typedef enum _UVC_VSI_STATE
{
    VSI_RESUME = 0 ,
    VSI_PAUSE = 1 
} UVC_VSI_STATE ;

typedef enum _UVC_VSI_LOCAL_PATH
{
    VSI_NORMAL_PATH= 0 ,
    VSI_GRA_PATH   = 1 
} UVC_VSI_LOCAL_PATH ;

typedef struct _DataExchangeParam
{
	MMP_USHORT	id;			//command ID
	MMP_ULONG	data_len;	//transfer lens
	MMP_ULONG	cur_offset;	//current read/write offset
	MMP_BYTE	*ptr;		//data buffer
}DataExchangeParam;

// videocontrol interface control selectors
#define  VC_CONTROL_UNDEFINED          0x00
#define  VC_VIDEO_POWER_MODE_CONTROL   0x01
#define  VC_REQUEST_ERROR_CODE_CONTROL 0x02
#define  VC_RESERVED                   0x03

// VC Status packet definitions (attribute)
#define VC_STS_ATTR_UPDATE_VAL         0x00
#define VC_STS_ATTR_UPDATE_INFO        0x01
#define VC_STS_ATTR_UPDATE_FAIL        0x02
#define VC_STS_ATTR_UPDATE_MIN         0x03 // UVC1.5
#define VC_STS_ATTR_UPDATE_MAX         0x04 // UVC1.5


// terminal control selectors
#define  TE_CONTROL_UNDEFINED          0x00

// selector unit control selectors
#define  SU_CONTROL_UNDEFINED          0x00
#define  SU_INPUT_SELECT_CONTROL       0x01

// camera terminal control selectors
#define  CT_CONTROL_UNDEFINED                  0x00
#define  CT_SCANNING_MODE_CONTROL              0x01
#define  CT_AE_MODE_CONTROL                    0x02
#define  CT_AE_PRIORITY_CONTROL                0x03
#define  CT_EXPOSURE_TIME_ABSOLUTE_CONTROL     0x04
#define  CT_EXPOSURE_TIME_RELATIVE_CONTROL     0x05
#define  CT_FOCUS_ABSOLUTE_CONTROL             0x06
#define  CT_FOCUS_RELATIVE_CONTROL             0x07
#define  CT_FOCUS_AUTO_CONTROL                 0x08
#define  CT_IRIS_ABSOLUTE_CONTROL              0x09
#define  CT_IRIS_RELATIVE_CONTROL              0x0A
#define  CT_ZOOM_ABSOLUTE_CONTROL              0x0B
#define  CT_ZOOM_RELATIVE_CONTROL              0x0C
#define  CT_PANTILT_ABSOLUTE_CONTROL           0x0D
#define  CT_PANTILT_RELATIVE_CONTROL           0x0E
#define  CT_ROLL_ABSOLUTE_CONTROL              0x0F
#define  CT_ROLL_RELATIVE_CONTROL              0x10

#define  CT_PRIVACY_CONTROL                    0x11
#define  CT_FOCUS_SIMPLE_CONTROL               0x12
#define  CT_WINDOW                             0x13
#define  CT_REGION_OF_INTEREST                 0x14

// CT support ,by customer requirement, change the value
#if SUPPORT_AUTO_FOCUS
#define CT_CS_VAL  ( ( 1 << CT_AE_MODE_CONTROL                ) | \
                     ( 1 << CT_AE_PRIORITY_CONTROL            ) | \
                     ( 1 << CT_EXPOSURE_TIME_ABSOLUTE_CONTROL ) | \
                     ( 1 << CT_ZOOM_ABSOLUTE_CONTROL          ) | \
                     ( 1 << CT_PANTILT_ABSOLUTE_CONTROL       ) | \
                     ( 1 << CT_FOCUS_AUTO_CONTROL             ) | \
                     ( 1 << CT_FOCUS_ABSOLUTE_CONTROL         ) )  
#else
#define CT_CS_VAL  ( ( 1 << CT_AE_MODE_CONTROL                ) | \
                     ( 1 << CT_AE_PRIORITY_CONTROL            ) | \
                     ( 1 << CT_EXPOSURE_TIME_ABSOLUTE_CONTROL ) | \
                     ( 1 << CT_ZOOM_ABSOLUTE_CONTROL          ) | \
                     ( 1 << CT_PANTILT_ABSOLUTE_CONTROL       ) )

#endif

// processing unit control selectors
#define  PU_CONTROL_UNDEFINED                      0x00
#define  PU_BACKLIGHT_COMPENSATION_CONTROL         0x01
#define  PU_BRIGHTNESS_CONTROL                     0x02
#define  PU_CONTRAST_CONTROL                       0x03
#define  PU_GAIN_CONTROL                           0x04
#define  PU_POWER_LINE_FREQUENCY_CONTROL           0x05
#define  PU_HUE_CONTROL                            0x06
#define  PU_SATURATION_CONTROL                     0x07
#define  PU_SHARPNESS_CONTROL                      0x08
#define  PU_GAMMA_CONTROL                          0x09
#define  PU_WHITE_BALANCE_TEMPERATURE_CONTROL      0x0A
#define  PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL 0x0B
#define  PU_WHITE_BALANCE_COMPONENT_CONTROL        0x0C
#define  PU_WHITE_BALANCE_COMPONENT_AUTO_CONTROL   0x0D
#define  PU_DIGITAL_MULTIPLIER_CONTROL             0x0E
#define  PU_DIGITAL_MULTIPLIER_LIMIT_CONTROL       0x0F
#define  PU_HUE_AUTO_CONTROL                       0x10
#define  PU_ANALOG_VIDEO_STANDARD_CONTROL          0x11
#define  PU_ANALOG_LOCK_STATUS_CONTROL             0x12
#define  PU_CONTRAST_AUTO_CONTROL                  0x13

// CT support ,by customer requirement, change the value
#define PU_CS_VAL ( ( 1 << PU_BRIGHTNESS_CONTROL                    ) | \
                    ( 1 << PU_CONTRAST_CONTROL                      ) | \
                    ( 1 << PU_SATURATION_CONTROL                    ) | \
                    ( 1 << PU_SHARPNESS_CONTROL                     ) | \
                    ( 1 << PU_WHITE_BALANCE_TEMPERATURE_CONTROL     ) | \
                    ( 1 << PU_BACKLIGHT_COMPENSATION_CONTROL        ) | \
                    ( 1 << PU_GAIN_CONTROL                          ) | \
                    ( 1 << PU_POWER_LINE_FREQUENCY_CONTROL          ) | \
                    ( 1 << PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL)   )

// extension unit control selectors
#define  XU_CONTROL_UNDEFINED                      0x00
#if USB_UVC_H264==1 
// H264 XU for LGT begin
#define  XU_ENCODER_VIDEO_FORMAT_CONTROL           0x01
    #define XU_ENCODER_VIDEO_FORMAT_LEN            (10)
#define  XU_ENCODER_CONFIGURATION_CONTROL          0x02
    #define XU_ENCODER_CONFIGURATION_LEN           (25) 
#define  XU_RATE_CONTROL                           0x03
    #define XU_RATE_CONTROL_LEN                    (8)
#define  XU_FRAME_TYPE_CONTROL                     0x04
    #define XU_FRAME_TYPE_LEN                      (1)
#define  XU_CAMERA_DELAY_CONTROL                   0x05
    #define XU_CAMERA_DELAY_LEN                    (2)
#define  XU_FILTER_CONTROL                         0x06            
    #define XU_FILTER_LEN                          (2)
#endif

// videostreaming interface control selectors
// Interface control request ( wValue )
#define  VS_CONTROL_UNDEFINED                0x00
#define  VS_PROBE_CONTROL                    0x01
#define  VS_COMMIT_CONTROL                   0x02
#define  VS_STILL_PROBE_CONTROL              0x03
#define  VS_STILL_COMMIT_CONTROL             0x04
#define  VS_STILL_IMAGE_TRIGGER_CONTROL      0x05
#define  VS_STREAM_ERROR_CODE_CONTROL        0x06
#define  VS_GENERATE_KEY_FRAME_CONTROL       0x07
#define  VS_UPDATE_FRAME_SEGMENT_CONTROL     0x08
#define  VS_SYNCH_DELAY_CONTROL              0x09

#if 1
//VC descriptor type 
#define CS_UNDEFINED                         0x20
#define CS_DEVICE                            0x21
#define CS_CONFIGURATION                     0x22
#define CS_STRING                            0x23
#define CS_INTERFACE                         0x24
#define CS_ENDPOINT                          0x25

//Video Interface Class Code
#define SC_UNDEFINED                    0x00
#define SC_VIDEOCONTROL                 0x01
#define SC_VIDEOSTREAMING               0x02
#define SC_VIDEO_INTERFACE_COLLECTION   0x03

//VC Interface descriptor subtype 
#define VC_DESCRIPTOR_UNDEFINED         0x00
#define VC_HEADER                       0x01
#define VC_INPUT_TERMINAL               0x02
#define VC_OUTPUT_TERMINAL              0x03
#define VC_SELECTOR_UNIT                0x04
#define VC_PROCESSING_UNIT              0x05
#define VC_EXTENSION_UNIT               0x06

//VS Interface descriptor type
#define VS_UNDEFINED                    0x00
#define VS_INPUT_HEADER                 0x01
#define VS_OUTPUT_HEADER                0x02
#define VS_STILL_IMAGE_FRAME            0x03
#define VS_FORMAT_UNCOMPRESSED          0x04
#define VS_FRAME_UNCOMPRESSED           0x05
#define VS_FORMAT_MJPEG                 0x06
#define VS_FRAME_MJPEG                  0x07
#define VS_FORMAT_MPEG2TS               0x0A
#define VS_FORMAT_DV                    0x0C
#define VS_COLORFORMAT                  0x0D
#define VS_FORMAT_FRAME_BASED           0x10
#define VS_FRAME_FRAME_BASED            0x11
#define VS_FORMAT_STREAM_BASED          0x12

//endpoint descriptor subtype
#define EP_UNDEFINED                    0x00
#define EP_GENERAL                      0x01
#define EP_ENDPOINT                     0x02
#define EP_INTERRUPT                    0x03
#endif

// capability info 
#define  INFO_GET_SUPPORT                0x01
#define  INFO_SET_SUPPORT                0x02
#define  INFO_AUTO_MODE_SUPPORT          0x04
#define  INFO_AUTOUPDATE_CONTROL_SUPPORT 0x08
#define  INFO_ASYNC_CONTROL_SUPPORT      0x10

// banding filter
#define BAND_DISBALE             0x00
#define BAND_50HZ                0x01
#define BAND_60HZ                0x02

// total numbers of video frames
#if USING_EXT_USB_DESC==1
#define NUM_YUY2_VIDEO PCCAM_RES_NUM
#define NUM_MJPG_VIDEO PCCAM_RES_NUM
#define NUM_H264_VIDEO PCCAM_RES_NUM
#define NUM_NV12_VIDEO (PCCAM_RES_NUM)
#else

#if ENABLE_YUY2
#define NUM_YUY2_VIDEO (2)//(17) // (29)// (PCCAM_RES_NUM)
#else
#define NUM_YUY2_VIDEO (0)
#endif


#if ENABLE_MJPEG
#define NUM_MJPG_VIDEO (4)//(17) // (29) //(PCCAM_RES_NUM)
#else
#define NUM_MJPG_VIDEO (0)
#endif

#if ENABLE_FRAME_BASE_H264
#define NUM_H264_VIDEO (4)//(17) // (29)
#else
#define NUM_H264_VIDEO (0)
#endif

#if ENABLE_NV12
#define NUM_NV12_VIDEO (29)//(PCCAM_RES_NUM)
#else
#define NUM_NV12_VIDEO (0)
#endif

#endif
// total numbers of still frames
#define NUM_YUY2_STILL 3//3
#define NUM_MJPG_STILL 0//5
#define NUM_UYVY_STILL 0//3

//still capture
#define STILL_IMAGE_WAITING 0
#define STILL_IMAGE_TRIGGER 1
#define STILL_IMAGE_PREVIEW_STOP 2
#define STILL_IMAGE_SENDING 3
#define STILL_IMAGE_FINISH 4



MMP_USHORT UVC_VCD(void) ;
void SET_UVC_VCD(MMP_USHORT ver);
MMP_USHORT VIDEO_EP_END(void) ;

void usb_vc_cfg_video_probe(MMP_USHORT indx,MMP_ULONG max_frame_size,MMP_ULONG max_payload_size,MMP_ULONG frameInterval);
void usb_vc_get_video_probe(MMP_UBYTE IfIndex,MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex,MMP_USHORT fpsx10,MMP_BOOL upd);


void usb_vc_set_still_commit(STILL_PROBE* pSCIN);
void usb_vc_get_still_probe(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex ,STILL_PROBE* pSP);
void usb_vc_set_still_probe(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex);
void usb_vc_set_still_resolution(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex);
MMP_UBYTE SetH264Mode(MMP_UBYTE Mode);
void uvc_init(MMP_BOOL reset_val,MMP_UBYTE ep_id);
void usb_vc_req_proc(MMP_UBYTE req);
void usb_vc_update_automode(void);
void usb_vc_out_data(void);
void usb_vc_capture_trigger(MMP_UBYTE pressed);
void usb_vc_update_async_mode(MMP_UBYTE bOriginator,MMP_UBYTE bSelector,MMP_UBYTE bAttribute,void *bValue,MMP_UBYTE bValUnit);
// sean@2011_01_22,TimeStamp Function
MMP_ULONG usb_vc_cur_sof(void) ;
MMP_ULONG usb_vc_diff_sof(MMP_ULONG t1);
MMP_ULONG usb_vc_cur_usof(void) ;
MMP_ULONG usb_vc_diff_usof(MMP_ULONG t1);
int sys_task_info(void);
void check_task_stack_init(void);
void check_task_stack_overflow(void);
#define IS_FIRST_STREAM(workss)  ( (gbChangeH264Res)?0: (VIDEO_EP_END()==1) || (workss < VIDEO_EP_END()) )
MMP_USHORT usb_vc_cur_fpsx10(MMP_UBYTE ep_id);
#endif