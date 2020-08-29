#ifndef _PCAM_API_H
#define _PCAM_API_H
#include "os_wrap.h"
#include "mmp_lib.h"

#define PCAM_BLOCKING       (0) // Blocking call, can't call by ISR
#define PCAM_NONBLOCKING    (1) // Non-blocking call, can call by ISR
#define PCAM_API            (2) // Function call directly
#define PCAM_OVERWR         (3) // Non-blocking , message block will be overwritten by next msg
typedef enum {
    PCAM_USB_VIDEO_FORMAT_OTHERS = 0x00,
    PCAM_USB_VIDEO_FORMAT_H263,
    PCAM_USB_VIDEO_FORMAT_MP4V,	
    PCAM_USB_VIDEO_FORMAT_H264,
    PCAM_USB_VIDEO_FORMAT_MJPEG, 
    PCAM_USB_VIDEO_FORMAT_YUV422,
    PCAM_USB_VIDEO_FORMAT_YUV420, 
    PCAM_USB_VIDEO_FORMAT_RAW,
    PCAM_USB_VIDEO_FORMAT_NUM 
    
} PCAM_USB_VIDEO_FORMAT ;

typedef enum {
    PCAM_USB_HIGH_Q,
    PCAM_USB_NORMAL_Q,
    PCAM_USB_LOW_Q,
    PCAM_USB_QUALITY_NUM 
} PCAM_USB_VIDEO_QUALITY ;

//
// Revised the list by Firmware Functional Spec. for AIT_0214_2011.pdf
//
typedef enum {

    PCCAM_640_480 = 0,
    PCAM_USB_RESOL_640x480 = 0,
    VIDRECD_RESOL_640x480 = 0,

    PCCAM_1920_1080=1,
    PCAM_USB_RESOL_1920x1080 = 1,
    VIDRECD_RESOL_1920x1080 = 1,

    PCCAM_1280_720 = 2,
    PCAM_USB_RESOL_1280x720 = 2,
    VIDRECD_RESOL_1280x720 = 2,
    
    PCCAM_640_360 = 3,
    PCAM_USB_RESOL_640x360 = 3,
    VIDRECD_RESOL_640x360 = 3,

    PCCAM_1280_720_60 = 4,
    PCAM_USB_RESOL_1280x720_60 = 4,
    VIDRECD_RESOL_1280x720_60 = 4,
    
    PCCAM_640_360_60 = 5,
    PCAM_USB_RESOL_640x360_60 = 5,
    VIDRECD_RESOL_640x360_60 = 5,
    
    PCCAM_864_480_60 = 6,
    PCAM_USB_RESOL_864x480_60 = 6,
    VIDRECD_RESOL_864x480_60 = 6,	

    PCCAM_864_480 = 7,
    PCAM_USB_RESOL_864x480 = 7,
    VIDRECD_RESOL_864x480 = 7,	

    PCCAM_160_90 = 8,
    PCAM_USB_RESOL_160x90  =8,
    VIDRECD_RESOL_160x90  =8,

    
    PCCAM_160_120 = 9,
    PCAM_USB_RESOL_160x120  =9,
    VIDRECD_RESOL_160x120  =9,
    
    PCCAM_176_144  =7,
    PCAM_USB_RESOL_176x144  =7,
    VIDRECD_RESOL_176x144  =7,
	
	PCCAM_320_180 = 8,
	PCAM_USB_RESOL_320x180 = 8,
	VIDRECD_RESOL_320x180 = 8,
	
	PCCAM_320_240 = 9,
	PCAM_USB_RESOL_320x240 = 9,
	VIDRECD_RESOL_320x240 = 9,
	
    PCCAM_352_288 = 10,
    PCAM_USB_RESOL_352x288 = 10,
    VIDRECD_RESOL_352x288 = 10,
    
    PCCAM_432_240 = 11,
    PCAM_USB_RESOL_432_240 = 11,
    VIDRECD_RESOL_432x240 = 11,
    /*
    PCCAM_640_360 = 8,
    PCAM_USB_RESOL_640x360 = 8,
    VIDRECD_RESOL_640x360 = 8,
    */
    PCCAM_800_448 = 12,
    PCAM_USB_RESOL_800x448 = 12,
    VIDRECD_RESOL_800x448 = 12,
    
    PCCAM_800_600 = 13,  
    PCAM_USB_RESOL_800x600 = 13,
    VIDRECD_RESOL_800x600 = 13,
    /*
    PCCAM_864_480 = 11,
    PCAM_USB_RESOL_864x480 = 11,
    VIDRECD_RESOL_864x480 = 11,
    */
    PCCAM_960_720 = 14,
    PCAM_USB_RESOL_960x720 = 14,
    VIDRECD_RESOL_960x720 = 14,
    
    PCCAM_1024_576 = 15,
    PCAM_USB_RESOL_1024x576 = 15,
    VIDRECD_RESOL_1024x576 = 15,
    /*
    PCCAM_1280_720 = 14,
    PCAM_USB_RESOL_1280x720 = 14,
    VIDRECD_RESOL_1280x720 = 14,
    */
    PCCAM_1600_896 = 16,
    PCAM_USB_RESOL_1600x896 = 16,
    VIDRECD_RESOL_1600x896 = 16,
    
    /*
    PCCAM_1920_1080=16,
    PCAM_USB_RESOL_1920x1080 = 16,
    VIDRECD_RESOL_1920x1080 = 16,
    */
    PCCAM_2304_1296=17,
    PCAM_USB_RESOL_2304x1296 = 17,
    VIDRECD_RESOL_2304x1296 = 17,
    
    PCCAM_2304_1536=18,
    PCAM_USB_RESOL_2304x1536 = 18,
    VIDRECD_RESOL_2304x1536 = 18,
 
    /*--Skype--*/
    //PCCAM_176_144  =20,
    //PCAM_USB_RESOL_176x144  =20,

    PCCAM_272_144  =19,
    PCAM_USB_RESOL_272x144  =19,
    VIDRECD_RESOL_272x144  =19,
    
    PCCAM_368_208 = 20,
    PCAM_USB_RESOL_368x208 = 20,
    VIDRECD_RESOL_368x208 = 20,

    
    PCCAM_384_216 = 21,
    PCAM_USB_RESOL_384_216  = 21,
    VIDRECD_RESOL_384x216  = 21,
    
    PCCAM_480_272 = 22,
    PCAM_USB_RESOL_480x272 = 22,
    VIDRECD_RESOL_480x272 = 22,
    
    PCCAM_624_352 = 23,
    PCAM_USB_RESOL_624x352 = 23,
    VIDRECD_RESOL_624x352 = 23,
    
    PCCAM_912_512 = 24,
    PCAM_USB_RESOL_912x512 = 24,
    VIDRECD_RESOL_912x512 = 24,
    
#if 1//USING_EXT_USB_DESC==0
    PCCAM_424_240 = 25,
    PCAM_USB_RESOL_424x240 = 25,
    VIDRECD_RESOL_424x240 = 25,
    
    PCCAM_480_270 = 26,
    PCAM_USB_RESOL_480x270 = 26,
    VIDRECD_RESOL_480x270 = 26,

    PCCAM_848_480 = 27,
    PCAM_USB_RESOL_848x480 = 27,
    VIDRECD_RESOL_848x480 = 27,
    
    PCCAM_960_540 = 28,
    PCAM_USB_RESOL_960x540 = 28,
    VIDRECD_RESOL_960x540 = 28,
    
    PCCAM_RES_NUM=29,
    PCAM_USB_RESOL_RES_NUM=29, 
    VIDRECD_RESOL_MAX_NUM =29
#else
    PCCAM_848_480 = 25,
    PCAM_USB_RESOL_848x480 = 25,
    VIDRECD_RESOL_848x480 = 25,
    
    PCCAM_RES_NUM=26,
    PCAM_USB_RESOL_RES_NUM=26, 
    VIDRECD_RESOL_MAX_NUM =26

#endif

    
} RES_TYPE_LIST ,PCAM_USB_VIDEO_RES ;

typedef enum {
    PCAM_USB_AUDIO_FORMAT_AAC,
    PCAM_USB_AUDIO_FORMAT_AMR,
    PCAM_USB_AUDIO_FORMAT_NUM 
} PCAM_USB_AUDIO_FORMAT ;


typedef enum {
	PCAM_USB_DEBAND_60HZ,
	PCAM_USB_DEBAND_50HZ
} PCAM_USB_DEBAND ;

typedef enum {
    PCAM_USB_SETTING_H264_RES, // set uvc h264 resolution
    PCAM_USB_SETTING_VIDEO_RES,
    PCAM_USB_SETTING_VIDEO_FORMAT,
    PCAM_USB_SETTING_VIDEO_QUALITY,
    PCAM_USB_SETTING_DEBAND,
    PCAM_USB_SETTING_AUDIO_FORMAT ,
    
    PCAM_USB_SETTING_EXPOSURE_TIME, //sean@2011_01_24, add exposure time
    PCAM_USB_SETTING_SATURATION,
    PCAM_USB_SETTING_CONTRAST,
    PCAM_USB_SETTING_BRIGHTNESS,
    PCAM_USB_SETTING_HUE,
    PCAM_USB_SETTING_GAMMA,
    PCAM_USB_SETTING_BACKLIGHT,
    PCAM_USB_SETTING_SHARPNESS,
    PCAM_USB_SETTING_GAIN,
    PCAM_USB_SETTING_WB,
    PCAM_USB_SETTING_LENS,
    PCAM_USB_SETTING_AF,
    PCAM_USB_SETTING_AE,
    PCAM_USB_SETTING_AE_PRIO,
    PCAM_USB_SETTING_WB_TEMP,
    PCAM_USB_SETTING_DIGZOOM,
    PCAM_USB_SETTING_DIGPAN,
    PCAM_USB_SETTING_FORCE_BLACK,
    PCAM_USB_SETTING_SF,
    PCAM_USB_SETTING_OSD_OP,
    PCAM_USB_SETTING_EFFECT,
    PCAM_USB_SETTING_ALL
} PCAM_USB_SETTING_CONTEXT ;

typedef enum {
    WB_EFFECT,
    SATURATION_EFFECT
} PCAM_USB_PREVIEW_EFFECT ;

typedef struct _PCAM_USB_EFFECT_CTL
{
    PCAM_USB_PREVIEW_EFFECT effect ;
    MMP_UBYTE orig_auto ;
    MMP_LONG orig_val,new_val ;
    MMP_USHORT on_ms,off_ms ,times ;
} PCAM_USB_EFFECT_CTL; 

typedef struct _PCAM_USB_ZOOM {
    MMP_USHORT Dir ; // 0 : zoom in, 1 : zoom out, 2 : zoom stop
    MMP_USHORT RangeStep; // How many step, defined at UVC.
    MMP_USHORT RangeMin;  // The zoom range for each step, floating
    MMP_USHORT RangeMax;  // The zoom range for each step, floating
} PCAM_USB_ZOOM ; 

typedef struct _PAN_USB_PANTILT {
    MMP_LONG    PanMin ;
    MMP_LONG    PanMax ;
    MMP_LONG    TiltMin;
    MMP_LONG    TiltMax;
    MMP_USHORT  Steps  ;
} PCAM_USB_PANTILT ;

typedef struct _PCAM_USB_INFO {
    MMP_UBYTE               pCamEpId ;
    MMP_USHORT              pCamSetIndex ;
    PCAM_USB_VIDEO_FORMAT   pCamVideoFormat ;
    PCAM_USB_VIDEO_QUALITY  pCamVideoQuality;
    PCAM_USB_VIDEO_RES      pCamVideoRes    ;
    PCAM_USB_DEBAND         pCamDebandMode  ;
    PCAM_USB_AUDIO_FORMAT   pCamAudioFormat ;
    MMP_ULONG               pCamExpTime    ; //sean@2011_01_24
    MMP_USHORT              pCamSaturation ; // saturation
    MMP_USHORT              pCamContrast   ;
    MMP_USHORT              pCamSharpness  ;
    MMP_USHORT              pCamGain ;
    MMP_USHORT              pCamBrightness ;
    MMP_USHORT              pCamHue ;
    MMP_USHORT              pCamGamma;
    
    MMP_USHORT              pCamBacklight  ;
    MMP_UBYTE               pCamWB ;
    MMP_USHORT              pCamWBTemp;
    MMP_USHORT              pCamLensPos ;
    MMP_BOOL                pCamEnableAF ;
    MMP_BOOL                pCamEnableAE ;
    MMP_BOOL                pCamEnableAEPrio;
    PCAM_USB_ZOOM           pCamDigZoom ;
    PCAM_USB_PANTILT        pCamDigPan  ;  //TBD
    MMP_BOOL                pCamForceBlack;
    
    PCAM_USB_ZOOM           pCamOsdOp;
    
    PCAM_USB_EFFECT_CTL     pCamEffect;
} PCAM_USB_INFO ;

typedef enum {
    PCAM_AUDIO_IN_NONE,
    PCAM_AUDIO_IN_I2S ,
    PCAM_AUDIO_AFE_SING,
    PCAM_AUDIO_AFE_DIFF,
    PCAM_AUDIO_AFE_DIFF2SING,
    PCAM_AUDIO_IN_NUM      
} PCAM_AUDIO_IN_PATH;

typedef enum {
    PCAM_AUDIO_LINEIN_DUAL,
    PCAM_AUDIO_LINEIN_R,
    PCAM_AUDIO_LINEIN_L,
    PCAM_AUDIO_LINEIN_NUM    
} PCAM_AUDIO_LINEIN_CHANNEL ;

typedef struct _PCAM_AUDIO_CFG {
    MMP_USHORT pCamSampleRate ;
    PCAM_AUDIO_IN_PATH pCamAudioInPath;
    PCAM_AUDIO_LINEIN_CHANNEL pCamAudioChannel; 
    MMP_SHORT pCamVolume;
    MMP_BOOL  pCamMute ;        
} PCAM_AUDIO_CFG ;

// VC async. control block
typedef struct _PCAM_ASYNC_VC_CFG
{
    MMP_BOOL  pCamEnableAsyncMode; // Enable Async mode or not
    MMP_UBYTE pCamOriginator ;
    MMP_UBYTE pCamSelector ;
    MMP_UBYTE pCamAttribute ;
    MMP_UBYTE pCamValUnit ;
} PCAM_ASYNC_VC_CFG ;

#define BAYER_10BPP     (0<<0)
#define BAYER_12BPP     (1<<0)

#define BAYER_PACKED    (0<<1)
#define BAYER_UNPACKED  (1<<1)

#define BAYER_RGBORDER(x) ( (x&0x3) << 2 )

typedef struct _PCAM_BAYER_INFO
{
    MMP_USHORT pCamBayerInfo ;
    MMP_USHORT pCamBayerW ;
    MMP_USHORT pCamBayerH ;
    
} PCAM_BAYER_INFO ;

typedef enum {
    SEN_POWER_OFF=0,
    SEN_STANDBY  
} PCAM_USB_SENSOR_OFF_MODE ; 

MMP_USHORT pcam_usb_init(void);
MMP_USHORT pcam_usb_preview(MMP_USHORT pCamNonBlockingCall,MMP_UBYTE ep_id);
MMP_USHORT pcam_usb_preview_stop(MMP_USHORT pCamNonBlockingCall);
MMP_USHORT pcam_usb_exit(MMP_USHORT pCamNonBlockingCall,PCAM_USB_SENSOR_OFF_MODE poweroff_mode);
MMP_USHORT pcam_usb_osd_update(void) ;
MMP_USHORT pcam_usb_set_attributes(MMP_USHORT pCamNonBlockingCall,MMP_UBYTE pcamIndex, void *pcamValue ); 
MMP_USHORT pcam_usb_take_picture(void) ;
PCAM_USB_INFO *pcam_get_info(void);
void pcam_usb_set_nonblocking(MMP_USHORT nonblocking);

MMP_USHORT pcam_usb_audio_start(MMP_USHORT pCamNonBlockingCall);
MMP_USHORT pcam_usb_audio_stop(MMP_USHORT pCamNonBlockingCall);
MMP_USHORT pcam_usb_update_firmware(MMP_USHORT pCamNonBlockingCall,MMP_USHORT step);
MMP_USHORT pcam_usb_task_alive(MMP_USHORT heartbeat) ;
MMP_USHORT pcam_usb_take_rawpicture(MMP_USHORT pCamNonBlockingCall,MMP_ULONG addr);

MMP_USHORT pcam_usb_set_samplerate(MMP_USHORT pCamNonBlockingCall,MMP_ULONG samplerate);
MMP_USHORT pcam_usb_set_mute(MMP_USHORT pCamNonBlockingCall,MMP_BOOL mute);
MMP_USHORT pcam_usb_set_volume(MMP_USHORT pCamNonBlockingCall,MMP_SHORT vol) ;

MMP_USHORT pcam_usb_sendmsg(MMP_USHORT pCamNonBlockingCall,MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
MMP_USHORT pcam_usb_sendoverwrmsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);

#if 1//OMURON_FDTC_SUPPORT
MMP_USHORT pcam_usb_fdtc(void);
MMP_USHORT pcam_usb_fdtc_stop(void);
#endif

#endif
