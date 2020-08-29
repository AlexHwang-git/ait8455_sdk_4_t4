/*
pcam message handler.
*/

#include "includes_fw.h"
#include "config_fw.h"
#include "mmp_lib.h"
#include "os_wrap.h"
#include "lib_retina.h"

#include "mmpf_pll.h"

#include "pcam_msg.h"
#include "pcam_handler.h"
#include "mmps_display.h"
#include "mmps_sensor.h"
#if(CHIP==P_V2)
#include "mmps_system.h"
#endif
#if(CHIP==VSN_V2) || (CHIP==VSN_V3)
#include "mmpf_system.h"
#endif

#include "mmpd_system.h"
#include "mmps_3gprecd.h"
#include "mmp_reg_gbl.h"
#include "mmp_reg_gpio.h"
#include "mmpf_sensor.h"
//#include "sensor_ctl.h"
//#include "3A_ctl.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbpccam.h"
#include "mmpf_audio_ctl.h"
#include "isp_if.h"
#include "mmps_usb.h"
#include "mmpf_scaler.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_audio.h"
#include "mmpf_usb_h264.h"
#include "mmpf_usbuvc.h"
#include "mmpf_encapi.h"

#include "lib_retina.h"
#if (MIC_SOURCE!=MIC_IN_PATH_AFE)
#include "mmpf_i2cm.h"
#endif

//#include "mmpf_sif.h"
#include "mmp_reg_sif.h"
#include "mmpf_mp4venc.h"
#include "hdm_ctl.h"
#include "mmpf_usbuvch264.h"
#include "mmpf_usbuac.h"
#include "os_cfg.h"
#if ENCODING_UNIT==1
#include "mmpf_usbuvc15h264.h"
#endif
#if SUPPORT_GRA_ZOOM
#define  H264_ALIGN_H   (1)
#else
#define  H264_ALIGN_H   (0)
#endif
#if OMRON_FDTC_SUPPORT 
#include "okaodtapi.h"
#include "okaodef.h"
#include "stdface.h"

#include "mmp_reg_scaler.h" //only for test
#include "mmps_system.h"

extern MMP_BYTE  gbEnableDoFDTC,gbFDTC_Done;
extern MMP_ULONG glFDTC_CurrentBuffer;
#endif


#if (AUDEXT_DAC == DA7211)
  #include "DA7211.h"
#endif

#if (SUPPORT_LED_DRIVER)
#include "Led_ctl.h"
#endif

#if H264_SIMULCAST_EN==1
extern GRA_STREAMING_CFG gGRAStreamingCfg;
extern MMP_ULONG           glGRAPreviewFormat;
extern MMP_ULONG           glGRAPreviewWidth;
extern MMP_ULONG           glGRAPreviewHeight;
#endif

#define FORCE_SIM_H264  (0)

#if FORCE_SIM_H264==1
#define SIM_H264    1 
#else
#define SIM_H264    UVCX_IsSimulcastH264()
#endif


#if USB_UVC_SKYPE
extern MMP_UBYTE gbSkypeMode;
extern MMP_UBYTE gbSkypeEncRes;
#endif


#if  1//RC_JPEG_TARGET_SIZE > 0
extern MMP_BOOL    gbEnableJpegRC ;
#endif

extern MMP_UBYTE gbUVCPara[];

extern MMPF_SENSOR_FUNCTION *gsSensorFunction ;
extern MMPF_OS_FLAGID PCAM_Flag ;
extern MMPS_DISPLAY_OUTPUTPANEL  gsCurrentDisplay;// = MMPS_DISPLAY_MAIN_LCD;
extern MMP_UBYTE gbADCDigitalGain ;
extern MMP_UBYTE gbADCAnalogGain  ;
extern MMP_UBYTE gbADCBoost  ;

//=====================================================
#define CUSTOMER_INIT_DATA_TAG (0xEE)
#define AEC_DEF (0x08)

MMP_ULONG gsHeader;
extern MMP_USHORT  gsAFocus;
extern MMP_USHORT  gsZoom;
extern MMP_USHORT  gsBacklight;
extern MMP_USHORT  gsTemper;
extern MMP_USHORT  gsSaturation;
extern MMP_USHORT  gsContrast;
extern MMP_USHORT  gsBrightness;
extern MMP_SHORT   gsHue;
extern MMP_SHORT   gsGamma;
extern MMP_SHORT   gsSharpness;
extern MMP_SHORT   gsGain ;
extern MMP_LONG    glPan,glTilt  ;
extern MMP_UBYTE   gbAWB,gbAE ;
extern MMP_ULONG   glEV; 
extern MMP_BOOL    gbAutoFocus;
extern MMP_UBYTE   gbBandFilter;
extern void MoveData2SIF(MMP_ULONG ulSifAddr, MMP_ULONG ulSourceAddr, MMP_ULONG ulDataSize);

MMP_ULONG gTmp_Gain;
MMP_UBYTE gSetVideoProperty;

MMP_SHORT AFNearAdjustValue;
MMP_SHORT AFInfiAdjustValue;
MMP_UBYTE gSharpnessIndex;
MMP_UBYTE gContrastIndex;

MMP_UBYTE usStoredBrightStep;
MMP_USHORT usChecksum;

//=====================================================

const __align(4) MMP_UBYTE Customer_Table_VP_Setting_Default[0x30] = {
/*The default must be sync with settings in mmpf_usbcolor.h*/
0xFF,  //tag for video property, 0x50(P) is vald, 0xff is invalid
0xFF,  //tag for video property, 0x56(V) is vald, 0xff is invalid
0x01,  //version - low byte
0x00,  //version - high byte
0x80,  //Brightness, default value:128  +4
0x00,
0x80,  //Contrast, default value:128  +6
0x00,
0x80,   //HUE, default value:128  +8
0x00,
0x80,       //Saturation, default value:128  +10
0x00,
0x80,   //Sharpness, default value:128    +12
0x00,
0x80,       //Gamma, default value:128      +14
0x00,
0xA0,       //WB_tem_cfg, default value:4000  +16
0x0F,
0x01,   //WB_temp_auto_cfg 0:manual mode, 1:auto mode   +18
0x00,   //Backlight   +19
0x00,
0x02,   //flicker 0:disable, 1:50Hz, 60 HZ  +21
0x01,       // auto_af 0:manual, 1:auto   +22
0xFF,   //AF position   +23
0x00,
0x08,   //exposure 0:manual, 1:auto   +25
0x9C,   //exposure value;   +26
0x00,
0x00,
0x00,
0x00,   //gain    +30
0x00,
0x00,  
0x00,
0x01,   //video property store, no_store, auto + 34
0x64,	//zoom	  +35
0x00,
0x00,   //pan     +37
0x00,
0x00,  
0x00,
0x00,   //tilt    +41
0x00,
0x00,  
0x00,
0x0C,	//LED step +45	//Modified by James, change default from step 0 to 12.
0xAD,	//checksum +46
0x14
};

const __align(4) MMP_UBYTE Customer_Table_Setting_Default[0x48] = {
0xEE, // tag
0x09, //version 1
0x48, 0x00,
0xEE, // offset 0x0004
0x01,
0x08, 0x00, // Field length, //included tag,id,length
LOBYTE(0x114D),
HIBYTE(0x114D),
#if (CUSTOMER == XXX)
LOBYTE(0x8458),
HIBYTE(0x8458),
#else
LOBYTE(0x8458),
HIBYTE(0x8458),
#endif 
0xEE, // offset 0x000c
0x13, // serial number tag
0x34, 0x00, // Field length, //included tag,id,length
0x30,
0x03,
#if (CUSTOMER == XXX)
'A', 0x00, 'l', 0x00, 'p', 0x00, 'h', 0x00, 'a', 0x00, ' ', 0x00, 'I', 0x00, 'm', 0x00,
'a', 0x00, 'g', 0x00, 'i', 0x00, 'n', 0x00, 'g', 0x00, ' ', 0x00, 'T', 0x00, 'e', 0x00,
'c', 0x00, 'h', 0x00, '.', 0x00, ' ', 0x00, 'C', 0x00, 'o', 0x00, '.', 0x00,
#else
'0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00,
'0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00,
'0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00, '0', 0x00,
#endif
0xEE,
0x51,  //for VCM tag
0x08,  //VCM length
0x00,
#if (CUSTOMER == XXX)
0xFF,   //inifi +68
0xFF,
0xFF,   //near +70
0xFF,
#elif (CUSTOMER == XXX)
0x08,   //inifi
0x00,
0x78,   //near
0x00,
#endif
};

//extern MMP_BOOL gbVideoEncQualityCustom;
extern MMP_SHORT   gsAudioSamplesPerFrame ;
extern MMP_UBYTE gbCurH264Type ;


MMP_USHORT m_usVideoQuality;
MMP_USHORT  gsCurrentSensor = MAIN_SENSOR ;
MMP_UBYTE gbDevicePowerSavingStatus = 1;
MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt = MMPS_3GPRECD_VIDEO_FORMAT_OTHERS ;
MMP_UBYTE glAudioEnable = FALSE;

MMP_ULONG  glCurSensorGain ; // sean@2011_01_24
//MMP_USHORT gsKeepCurSlotNum;

extern MMP_USHORT  gsVideoFormat;

static MMP_BOOL gbADCMute ;
static PCAM_USB_CTX pCamCtx ;
static PCAM_ASYNC_VC_CFG pCamVCCfg ;

 MMP_UBYTE gbOrigADCDigitalGain   ;
//static MMP_UBYTE gbOrigADCAnalogGain  ;
//static MMP_UBYTE gbOrigADCBoost ;
static void MMPF_Init_DefaultClock(void);
static MMP_USHORT PCAM2MMP_Get_Quality(PCAM_USB_INFO *pcam_info) ;
static MMP_USHORT PCAM2MMP_Get_Resolution(PCAM_USB_INFO *pcam_info);
static MMPS_3GPRECD_VIDEO_FORMAT PCAM2MMP_Get_VideoFormat(PCAM_USB_INFO *pcam_info);
static MMPS_3GPRECD_AUDIO_FORMAT PCAM2MMP_Get_AudioFormat(PCAM_USB_INFO *pcam_info);
static MMPS_3GPRECD_VIDEO_FORMAT PCAM2MMP_Get_VideoFormat2(PCAM_USB_VIDEO_FORMAT pCamVideoFormat);
static MMP_USHORT PCAM2MMP_Get_DebandMode(PCAM_USB_INFO *pcam_info);
static MMP_USHORT PCAM2MMP_Get_AudioInPath(PCAM_AUDIO_CFG *pcam_audio);
static MMPF_AUDIO_LINEIN_CHANNEL PCAM2MMP_Get_AudioInChannel(PCAM_AUDIO_CFG *pcam_info);
static void MMPF_Init_DefGain(void) ;
static MMP_SHORT PCAM2MMP_Normalize_Value(MMP_USHORT val) ;
static MMP_USHORT PCAM2MMP_Normalize_EV(MMP_USHORT val);
static MMP_USHORT PCAM2MMP_Normalize_Gain(MMP_USHORT val);
static MMP_USHORT PCAM2MMP_Normalize_WB(MMP_USHORT val);
static MMP_USHORT PCAM2MMP_Normalize_Shutter(MMP_USHORT val);
static void  GetScalerZoomRange(MMP_USHORT who,MMP_USHORT *target_min,MMP_USHORT *target_max,MMPF_SCALER_ZOOM_INFO  *scalerInfo,PCAM_USB_ZOOM  *zoomInfo);
MMP_BOOL  GetScalerZoomGrab(MMPF_SCALER_PATH pipe,MMP_USHORT who,MMPF_SCALER_ZOOM_INFO  *scalerInfo,PCAM_USB_ZOOM  *zoomInfo,MMP_BOOL lossy_up);
static MMP_USHORT PCAM_USB_SetZoom(MMPF_SCALER_PATH path_sel,PCAM_USB_ZOOM  *zoomInfo,MMP_BOOL start );
static MMP_USHORT PCAM_USB_SetPanTilt(MMPF_SCALER_PATH path_sel,PCAM_USB_PANTILT  *panInfo,MMP_BOOL start,MMP_LONG *ret_pan_off,MMP_LONG *ret_tilt_off);
static MMP_USHORT PCAM_USB_PostPanTilt(MMPF_SCALER_PATH path_sel,MMP_LONG pan_off,MMP_LONG tilt_off);

void MMPF_PCAM_StartUSB(void) ;

//extern PCAM_USB_VIDEO_FORMAT gsVidFmt;
//
// Log the async. mode settings for pCam handler to call async. mode function
//
MMP_USHORT PCAM_SetAsyncVideoControl(MMP_UBYTE bOriginator,MMP_UBYTE bSelector,MMP_UBYTE bAttribute,MMP_UBYTE bValUnit)
{
    pCamVCCfg.pCamEnableAsyncMode = 1; // Enable Async mode or not
    pCamVCCfg.pCamOriginator = bOriginator;
    pCamVCCfg.pCamSelector = bSelector;
    pCamVCCfg.pCamAttribute = bAttribute;
    pCamVCCfg.pCamValUnit = bValUnit ;
    return PCAM_ERROR_NONE ;
}

#if (MIC_SOURCE==MIC_IN_PATH_I2S)
static void MMPF_Init_ExteralDac(void);
#endif

/*
Wolfson DAC W8737
*/
#if AUDEXT_DAC==WM8737 
static void MMPF_InitWM8737(void);
static void MMPF_PowerUpWM8737(MMP_USHORT);
static void MMPF_PowerDownWM8737(void);
void MMPF_WM8737VolumeControl(MMP_UBYTE volume);
/* Gason@20100730, Separate WM8737 function to three function, init , power up and power down*/
static MMPF_I2CM_ATTRIBUTE m_WM8737_I2cmAttribute = {MMPF_I2CM_ID_0, 0x1A, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE, 0, 0, 0, MMPF_I2CM_SPEED_HW_400K, NULL, NULL};
#endif

#if AUDEXT_DAC==DA7211

void MMPF_DA7211_SetSamplingRate(MMP_ULONG ulSamplerate);
static void MMPF_InitDA7211(void);
void MMPF_DA7211_PwUpExtMicIn(void);
//static void MMPF_PowerUpDA7211(void);
static void MMPF_PowerDownDA7211(void);
static void MMPF_DA7211VolumeControl(MMP_UBYTE volume);
static void MMPF_DA7211_SetMute(MMP_UBYTE enable);
static  MMPF_I2CM_ATTRIBUTE gI2cmAttribute_DA7211 = {MMPF_I2CM_ID_0, 0x1A, 8, 8, 0x10,MMP_TRUE, MMP_FALSE, MMP_FALSE, MMP_FALSE, 0, 0, 0,MMPF_I2CM_SPEED_HW_250K/*SW support max 200KHZ*/, NULL, NULL};

#endif


#if (MIC_SOURCE==MIC_IN_PATH_I2S) || (MIC_SOURCE==MIC_IN_PATH_BOTH)
static void MMPF_Init_ExternalDac(void)
{
// Audio IN use I2C interface.
#if AUDEXT_DAC==WM8737 
    MMPF_InitWM8737();
#endif
#if (AUDEXT_DAC == DA7211)
  MMPF_InitDA7211();
#endif
}
#endif

static void MMPF_Init_DefaultClock(void)
{
                               
}

extern MMP_UBYTE    os_created_task_prio[];
extern MMP_UBYTE    os_created_tasks ;

void MMPF_PCAM_WaitTaskIdle(MMP_UBYTE wait_flag)
{
    OS_TCB task;
    MMP_UBYTE err ,i ,prio;
    for (i=0;i<os_created_tasks;i++) {
        prio = os_created_task_prio[i] ;
        err = OSTaskQuery(prio,&task) ;
        if(err==OS_NO_ERR) {
            #if WATCHDOG_RESET_EN==0
            if(task.OSTCBPrio != OSTCBCur->OSTCBPrio) 
            #else
            if( (task.OSTCBPrio != OSTCBCur->OSTCBPrio) && (task.OSTCBPrio != (OS_LOWEST_PRIO - 2) ) )
            #endif
            {
                if(task.OSTCBStat!= wait_flag) {
                    dbg_printf(0,"@ Task Id:%d not idle\r\n",task.OSTCBPrio);
                    i = 0 ;
                    MMPF_OS_Sleep(1);
                } 
            }
        } else {
            dbg_printf(0,"@ Task Id:%d Err:%d\r\n",task.OSTCBPrio,err);
        }
    } 
   // dbg_printf(0,"Tasks Idle,enter sleep\r\n");  
}

// Wait all task suspend then start usb enum.
void MMPF_PCAM_StartUSB(void)
{
extern MMP_UBYTE gbPowerSavingAtBoot;
    MMP_ULONG cpu_sr;
    
    OS_TCB task ;
    MMP_UBYTE err ,i ,prio;
    for (i=0;i<os_created_tasks;i++) {
        prio = os_created_task_prio[i] ;
        #if WATCHDOG_RESET_EN==0
        if(prio!=TASK_PCAM_PRIO)
        #else
        if( (prio!=TASK_PCAM_PRIO)&& (prio!=(OS_LOWEST_PRIO - 2)))
        #endif
        {
            err = OSTaskQuery(prio,&task);
            if(err==OS_NO_ERR) {
                if(task.OSTCBStat == OS_STAT_RDY ) {
                    RTNA_DBG_Str(3,"@ Task #");
                    RTNA_DBG_Byte(3,prio);
                    RTNA_DBG_Str(3," not pend...\r\n");
                    i = 0 ;
                    MMPF_OS_Sleep(1);
                }
            } else {
                RTNA_DBG_Str(3,"@ Task # Err\r\n");
            }
        }
    }   
 //   USB_LEDDisplay(8,1);
    
    //RTNA_DBG_Str(3,"!Start USB Enum...\r\n");
// Disable Task running when USB start to enum
    TASK_CRITICAL(
        //MMPS_USB_SetMode(MMPS_USB_MSDC_MODE);
        MMPF_SetUSBChangeMode(MMPS_USB_MSDC_MODE);
        // Give 2 : no-used module, enter power saving
        if(gbPowerSavingAtBoot==0) {
            //MMPF_SYS_EnterPSMode(2);
            gbPowerSavingAtBoot = 1 ;
        }
    )
 //   USB_LEDDisplay(8,0);

}

static void MMPF_Init_DefGain(void)
{
#if SUPPORT_UAC==1
    gbOrigADCDigitalGain = gbADCDigitalGain;    
#endif    
}

void uvc_init_data(MMP_ULONG cd_vp_tbl_addr, MMP_ULONG cd_tbl_addr)
{
    //MMP_UBYTE ubManufacturerID;
    //MMP_USHORT    usDeviceID;
    //MMP_UBYTE *ptr;
        MMP_UBYTE *cd_tbl_bptr;
        MMP_UBYTE CUSTOMER_INIT_DATA_HEAD_OFFSET = 0;
        MMP_UBYTE VPTBL_INIT_DATA_HEAD_OFFSET = 0;
 
    RTNA_DBG_Str3("MMPF_PCAM_Init_UVC_Table() start...\r\n");
 
    //MMPF_SIF_Init();
    //if (USB_ResetFlash()) {
    /*if (MMPF_SF_Reset()) {
#if EN_DBGLOG_FOR_FLASH
        RTNA_DBG_Str(3, "\r\nSIF init error !!\r\n");
#endif
    }*/
#if 1//debug
    RTNA_DBG_Str3("  CUSTOMER_INIT_FLASH_ADDR = ");
    RTNA_DBG_Long3(CUSTOMER_INIT_FLASH_ADDR);
    RTNA_DBG_Str3("\r\n");
    RTNA_DBG_Str3("  cd_tbl_addr = ");
    RTNA_DBG_Long3(cd_tbl_addr);
    RTNA_DBG_Str3("\r\n");
#endif
    MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR_VP, cd_vp_tbl_addr, 256);
    MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR, cd_tbl_addr, 256);  // read 16KB IQ table into DRAM buffer
   
#if 1//debug
    RTNA_DBG_Str3("  customer_table_valid_flag = ");
    RTNA_DBG_Byte3(*(MMP_UBYTE *)(cd_tbl_addr));
    RTNA_DBG_Str3("\r\n");
#endif
    RTNA_DBG_Str3("  cd_vp_tbl_addr = ");
    RTNA_DBG_Byte3(*(MMP_UBYTE *)(cd_vp_tbl_addr));
    RTNA_DBG_Str3(" : ");
    RTNA_DBG_Byte3(*(MMP_UBYTE *)(cd_vp_tbl_addr+1));
    RTNA_DBG_Str3(" : ");
    RTNA_DBG_Byte3(*(MMP_UBYTE *)(cd_vp_tbl_addr+2));
    RTNA_DBG_Str3(" : ");
    RTNA_DBG_Byte3(*(MMP_UBYTE *)(cd_vp_tbl_addr+3));
    RTNA_DBG_Str3("\r\n");
 
    // Video Property Table Setting
    //if((*(MMP_UBYTE *)(cd_vp_tbl_addr + VPTBL_INIT_DATA_HEAD_OFFSET) == 0x01)){ // tag for video property, 0x01 is vald, 0xff is invalid 
    if( (((*(MMP_UBYTE *)(cd_vp_tbl_addr + VPTBL_INIT_DATA_HEAD_OFFSET) == 0x50))&&((*(MMP_UBYTE *)(cd_vp_tbl_addr + VPTBL_INIT_DATA_HEAD_OFFSET + 1) == 0x56))) &&
        (((*(MMP_UBYTE *)(cd_vp_tbl_addr + VPTBL_INIT_DATA_HEAD_OFFSET + 2) == 0x01))&&((*(MMP_UBYTE *)(cd_vp_tbl_addr + VPTBL_INIT_DATA_HEAD_OFFSET + 3) == 0x00))) ) {  

	
        RTNA_DBG_Str3(" *** Video Property Table in Flash is valid ***\r\n");
 
        MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR_VP, cd_vp_tbl_addr, CUSTOMER_INIT_FLASH_SIZE);
 
        // Read Customer_Table_VP_Setting_Default - Read Video Property Table
                gsHeader = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+1)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr));
                gsBrightness = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+5)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+4));
                gsContrast = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+7)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+6));
                gsHue = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+9)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+8));
                gsSaturation = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+11)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+10));
                gsSharpness = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+13)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+12));
                gsGamma = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+15))<< 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+14));
                gsTemper = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+17)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+16));
                gbAWB = (*(MMP_UBYTE *)(cd_vp_tbl_addr+18));
                gsBacklight = (*(MMP_UBYTE *)(cd_vp_tbl_addr+20) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+19));
                gbBandFilter = (*(MMP_UBYTE *)(cd_vp_tbl_addr+21));
                gbAutoFocus = (*(MMP_UBYTE *)(cd_vp_tbl_addr+22));
                gsAFocus = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+24)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+23));
                gbAE = (*(MMP_UBYTE *)(cd_vp_tbl_addr+25));
                glEV = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+29)) << 24) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+28)) << 16) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+27)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+26));
                gTmp_Gain = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+33)) << 24) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+32)) << 16) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+31)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+30));
                gSetVideoProperty = (*(MMP_UBYTE *)(cd_vp_tbl_addr+34));
                gsZoom = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+36)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+35));
                glPan = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+33)) << 24) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+39)) << 16) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+38)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+37));
                glTilt = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+44)) << 24) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+43)) << 16) + ((*(MMP_UBYTE *)(cd_vp_tbl_addr+42)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+41));
				usStoredBrightStep = (*(MMP_UBYTE *)(cd_vp_tbl_addr+45));
 				usChecksum = ((*(MMP_UBYTE *)(cd_vp_tbl_addr+47)) << 8) + (*(MMP_UBYTE *)(cd_vp_tbl_addr+46));
 				 
        if (gsSharpness >= 3)
                gSharpnessIndex = 1;
        else
                gSharpnessIndex = 0;
        if (gsContrast >= 25)
                gContrastIndex = 1;
        else
                gContrastIndex = 0;
        }
    else{
        RTNA_DBG_Str3(" ### Video Property Table in Flash is invalid ###\r\n");
 
        // Write default Video Property Table into flash       
        //MMPF_SIF_WriteDataEx(CUSTOMER_INIT_FLASH_ADDR_VP, (MMP_ULONG)&Customer_Table_VP_Setting_Default[0], CUSTOMER_INIT_FLASH_SIZE);
 		//MMPF_SF_WriteData(CUSTOMER_INIT_FLASH_ADDR_VP, (MMP_ULONG)&Customer_Table_VP_Setting_Default[0], CUSTOMER_INIT_FLASH_SIZE);
        ////MMPF_SF_WriteData(CUSTOMER_INIT_FLASH_ADDR_VP, (MMP_ULONG)&Customer_Table_VP_Setting_Default[0], sizeof(Customer_Table_VP_Setting_Default));
		MoveData2SIF(CUSTOMER_INIT_FLASH_ADDR_VP, (MMP_ULONG)&Customer_Table_VP_Setting_Default[0], sizeof(Customer_Table_VP_Setting_Default));

        // Write Customer_Table_VP_Setting_Default - Wrtie Video Property
                gsHeader= (MMP_SHORT)Customer_Table_VP_Setting_Default[0];
                gsBrightness = ((MMP_LONG)Customer_Table_VP_Setting_Default[5] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[4];
                gsContrast = ((MMP_LONG)Customer_Table_VP_Setting_Default[7] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[6];
                gsHue = ((MMP_LONG)Customer_Table_VP_Setting_Default[9] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[8];
                gsSaturation = ((MMP_LONG)Customer_Table_VP_Setting_Default[11] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[10];
                gsSharpness = ((MMP_LONG)Customer_Table_VP_Setting_Default[13] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[12];
                gsGamma =((MMP_LONG)Customer_Table_VP_Setting_Default[15] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[14];
                gsTemper = ((MMP_LONG)Customer_Table_VP_Setting_Default[17] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[16];
                gbAWB = (MMP_SHORT)Customer_Table_VP_Setting_Default[18];
                gsBacklight = ((MMP_LONG)Customer_Table_VP_Setting_Default[20] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[19];
                gbBandFilter = (MMP_LONG)Customer_Table_VP_Setting_Default[21];
                gbAutoFocus = (MMP_LONG)Customer_Table_VP_Setting_Default[22];
                gsAFocus = ((MMP_LONG)Customer_Table_VP_Setting_Default[24] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[23];
                gbAE = (MMP_LONG)Customer_Table_VP_Setting_Default[25];
                glEV = ((MMP_LONG)Customer_Table_VP_Setting_Default[29] << 24) + ((MMP_LONG)Customer_Table_VP_Setting_Default[28] << 16) + ((MMP_LONG)Customer_Table_VP_Setting_Default[27] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[26];
                gTmp_Gain= ((MMP_LONG)Customer_Table_VP_Setting_Default[30] << 24) + ((MMP_LONG)Customer_Table_VP_Setting_Default[32] << 16) + ((MMP_LONG)Customer_Table_VP_Setting_Default[31] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[30];
                gSetVideoProperty = (MMP_LONG)Customer_Table_VP_Setting_Default[34];
                gsZoom = ((MMP_LONG)Customer_Table_VP_Setting_Default[36] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[35];
                glPan = ((MMP_LONG)Customer_Table_VP_Setting_Default[40] << 24) + ((MMP_LONG)Customer_Table_VP_Setting_Default[39] << 16) + ((MMP_LONG)Customer_Table_VP_Setting_Default[38] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[37];
                glTilt = ((MMP_LONG)Customer_Table_VP_Setting_Default[44] << 24) + ((MMP_LONG)Customer_Table_VP_Setting_Default[43] << 16) + ((MMP_LONG)Customer_Table_VP_Setting_Default[42] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[41];
				usStoredBrightStep = (MMP_LONG)Customer_Table_VP_Setting_Default[45];
				usChecksum = ((MMP_LONG)Customer_Table_VP_Setting_Default[47] << 8) + (MMP_LONG)Customer_Table_VP_Setting_Default[46];
        }   
    // Customer Table Setting
    if((*(MMP_UBYTE *)(cd_tbl_addr + CUSTOMER_INIT_DATA_HEAD_OFFSET) == CUSTOMER_INIT_DATA_TAG) &&
        (*(MMP_UBYTE *)(cd_tbl_addr + CUSTOMER_INIT_DATA_HEAD_OFFSET + 1) == 0x09) ){  // valid customer table, table version should be 1
        RTNA_DBG_Str3(" *** customer table in Flash is valid ***\r\n");
 
        MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR, cd_tbl_addr, CUSTOMER_INIT_FLASH_SIZE);  // read 16KB IQ table into DRAM buffer
 
        // Read Customer_Table_Setting_Default
                AFNearAdjustValue = (*(MMP_UBYTE *)(cd_tbl_addr+70));
                AFInfiAdjustValue = (*(MMP_UBYTE *)(cd_tbl_addr+68));
        }
    else{
        RTNA_DBG_Str3(" ### customer table in Flash is invalid ###\r\n");
 
        cd_tbl_bptr = (MMP_UBYTE *)(cd_tbl_addr);
       
                if ( cd_tbl_bptr[4] == 0xEE && cd_tbl_bptr[5] == 0x01 && cd_tbl_bptr[6] == 0x08 && cd_tbl_bptr[7] == 0x00 )
                {
            // load the default customer table except for the default iManufacturer string
                        // byte 18 ~ 63: iManufacturer default string (46 bytes). We skip this area.
            MEMCPY((MMP_UBYTE *)cd_tbl_addr, (MMP_UBYTE *)&Customer_Table_Setting_Default[0], 16);
            MEMCPY((MMP_UBYTE *)cd_tbl_addr+64, (MMP_UBYTE *)&Customer_Table_Setting_Default[64], CUSTOMER_INIT_FLASH_SIZE-64);
                }
                else
                {
            // load the complete default customer table
                MEMCPY((MMP_UBYTE *)cd_tbl_addr, (MMP_UBYTE *)&Customer_Table_Setting_Default[0], CUSTOMER_INIT_FLASH_SIZE);
        }
 
        // Write default customer table into flash
        //MMPF_SIF_WriteDataEx(CUSTOMER_INIT_FLASH_ADDR, (MMP_ULONG)&Customer_Table_Setting_Default[0], CUSTOMER_INIT_FLASH_SIZE);
        //MMPF_SF_WriteData(CUSTOMER_INIT_FLASH_ADDR, (MMP_ULONG)&Customer_Table_Setting_Default[0], CUSTOMER_INIT_FLASH_SIZE);
 		////MMPF_SF_WriteData(CUSTOMER_INIT_FLASH_ADDR, (MMP_ULONG)&Customer_Table_Setting_Default[0], sizeof(Customer_Table_Setting_Default));
        MoveData2SIF(CUSTOMER_INIT_FLASH_ADDR, (MMP_ULONG)&Customer_Table_Setting_Default[0], sizeof(Customer_Table_Setting_Default));

        // Write Customer_Table_Setting_Default
                AFNearAdjustValue = (MMP_ULONG)Customer_Table_Setting_Default[70];
                AFInfiAdjustValue = (MMP_ULONG)Customer_Table_Setting_Default[68];
        }
#if 1 //debug
        dbg_printf(3,"AFNearAdjustValue=%d\r\n",AFNearAdjustValue);
        dbg_printf(3,"AFInfiAdjustValue=%d\r\n",AFInfiAdjustValue);
        dbg_printf(3,"pCam_Video_Property.PU_HEADER=%x\r\n",gsHeader);
        dbg_printf(3,"pCam_Video_Property.PU_BRIGHTNESS=%d\r\n",gsBrightness);
        dbg_printf(3,"pCam_Video_Property.PU_CONTRAST=%d\r\n",gsContrast);
        dbg_printf(3,"pCam_Video_Property.PU_HUE=%d\r\n",gsHue);
        dbg_printf(3,"pCam_Video_Property.PU_SATURATION=%d\r\n",gsSaturation);
        dbg_printf(3,"pCam_Video_Property.PU_SHARPNESS=%d\r\n",gsSharpness);
        dbg_printf(3,"pCam_Video_Property.PU_GAMMA=%d\r\n",gsGamma);
        dbg_printf(3,"pCam_Video_Property.PU_WB_TEMP_CFG=%d\r\n",gsTemper);
        dbg_printf(3,"pCam_Video_Property.PU_WB_TEMP_AUTO_CFG=%d\r\n",gbAWB);
        dbg_printf(3,"pCam_Video_Property.PU_BACKLIGHT=%d\r\n",gsBacklight);
        dbg_printf(3,"pCam_Video_Property.PU_FLIKCER=%d\r\n",gbBandFilter);
        dbg_printf(3,"gbAutoFocus=%d\r\n",gbAutoFocus);
        dbg_printf(3,"gsAFocus=%d\r\n",gsAFocus);
        dbg_printf(3,"gbAE=%d\r\n",gbAE);
        dbg_printf(3,"glEV=%d\r\n",glEV);
        dbg_printf(3,"gGain=%d\r\n",gTmp_Gain);
        dbg_printf(3,"gsZoom=%d\r\n",gsZoom);
		dbg_printf(3,"glPan=%d\r\n",glPan);
		dbg_printf(3,"glTilt=%d\r\n",glTilt);
		dbg_printf(3,"usStoredBrightStep=%d\r\n",usStoredBrightStep);
		dbg_printf(3,"usChecksum=%d\r\n",usChecksum);
#endif
 
/*    gsVIDPID_HEAD_OFFSET = PCAM_Search_CustomerData_Offset((MMP_UBYTE *)cd_tbl_addr, CUSTOMER_INIT_FLASH_SIZE, VIDPID_TAG);
#if 1//debug
    dbg_printf(3, "gsVIDPID_HEAD_OFFSET = 0x%x\r\n", gsVIDPID_HEAD_OFFSET);
#endif
    gsSNSTRING_HEAD_OFFSET = PCAM_Search_CustomerData_Offset((MMP_UBYTE *)cd_tbl_addr, CUSTOMER_INIT_FLASH_SIZE, SNSTRING_TAG);
#if 1//debug
    dbg_printf(3, "gsSNSTRING_HEAD_OFFSET = 0x%x\r\n", gsSNSTRING_HEAD_OFFSET);
#endif
    RTNA_DBG_Str3("MMPF_PCAM_Init_UVC_Table() end...\r\n");*/ 
}

void VideoStoreVideoProperty(void)
{	
	MMP_USHORT CurChecksum = 0;
	MMP_UBYTE i = 0;
	MMP_BYTE	*pro_dma_buff;
	pro_dma_buff = (MMP_BYTE *)CUSTOMER_TABLE_VP_BUF_STA;

    //MMPF_SIF_WriteDataEx(CUSTOMER_INIT_FLASH_ADDR + 0x24, 0xFF, 1);
#if 0
    // Tag
        *(pro_dma_buff+0) = 0xFF;
        *(pro_dma_buff+1) = 0xFF;
 
    // Version
        *(pro_dma_buff+2) = 0xFF;
        *(pro_dma_buff+3) = 0xFF;
 
        //MMPF_SF_WriteData((CUSTOMER_INIT_FLASH_ADDR_VP), (MMP_LONG)VideoPropertyDataBuf, 4);  
 		MoveData2SIF((CUSTOMER_INIT_FLASH_ADDR_VP), CUSTOMER_TABLE_VP_BUF_STA, 4);  
#endif 
   // Tag
        *(pro_dma_buff+0) = 0x50; // Ascii: P
        *(pro_dma_buff+1) = 0x56; // Ascii: V
 
    // Version
        *(pro_dma_buff+2) = 0x01;
        *(pro_dma_buff+3) = 0x00;
        *(pro_dma_buff+4) = gsBrightness&0x00FF;
        *(pro_dma_buff+5) = (gsBrightness >> 8)&0xFF;
        CurChecksum += gsBrightness;
       
        *(pro_dma_buff+6) = gsContrast&0x00FF;
        *(pro_dma_buff+7) = (gsContrast >> 8)&0xFF;
        CurChecksum += gsContrast;
       
        *(pro_dma_buff+8) = gsHue&0x00FF;
        *(pro_dma_buff+9) = (gsHue >> 8)&0xFF;
        CurChecksum += gsHue;
       
        *(pro_dma_buff+10) = gsSaturation&0x00FF;
        *(pro_dma_buff+11) = (gsSaturation >> 8)&0xFF;
        CurChecksum += gsSaturation;
       
        *(pro_dma_buff+12) = gsSharpness&0x00FF;
        *(pro_dma_buff+13) = (gsSharpness >> 8)&0xFF;
        CurChecksum += gsSharpness;
       
        *(pro_dma_buff+14) = gsGamma&0x00FF;
        *(pro_dma_buff+15) = (gsGamma >> 8)&0xFF;
        CurChecksum += gsGamma;       
       
        *(pro_dma_buff+16) = gsTemper&0x00FF;
        *(pro_dma_buff+17) = (gsTemper >> 8)&0xFF;
        CurChecksum += gsTemper;
       
        *(pro_dma_buff+18) = gbAWB;
        CurChecksum += gbAWB;
       
        *(pro_dma_buff+19) = gsBacklight&0x00FF;
        *(pro_dma_buff+20) = (gsBacklight >> 8)&0xFF;
        CurChecksum += gsBacklight;
       
        *(pro_dma_buff+21) = gbBandFilter;
        CurChecksum += gbBandFilter;
       
        *(pro_dma_buff+22) = gbAutoFocus;
        CurChecksum += gbAutoFocus;
       
        *(pro_dma_buff+23) = gsAFocus&0x00FF;
        *(pro_dma_buff+24) = (gsAFocus >> 8)&0xFF;
        CurChecksum += gsAFocus;
 
        if(gbAE==8)
                gbAE = AEC_DEF;
                //gbAE = AEMODE_DEF;
 
        *(pro_dma_buff+25) = gbAE;
        CurChecksum += gbAE;

        *(pro_dma_buff+26) = glEV&0x000000FF;
        *(pro_dma_buff+27) = (glEV >> 8)&0x0000FF;
        *(pro_dma_buff+28) = (glEV >> 16)&0x00FF;
        *(pro_dma_buff+29) = (glEV >> 24)&0xFF;
        CurChecksum += glEV;
       
        *(pro_dma_buff+30) = gTmp_Gain&0x000000F;
        *(pro_dma_buff+31) = (gTmp_Gain >> 8)&0x0000FF;
        *(pro_dma_buff+32) = (gTmp_Gain >> 16)&0x00FF;
        *(pro_dma_buff+33) = (gTmp_Gain >> 24)&0xFF;
        CurChecksum += gTmp_Gain;
 
        *(pro_dma_buff+34) = gSetVideoProperty;
        CurChecksum += gSetVideoProperty;
        
        *(pro_dma_buff+35) = gsZoom&0x00FF;
        *(pro_dma_buff+36) = (gsZoom >> 8)&0xFF;
        CurChecksum += gsZoom;
        
        *(pro_dma_buff+37) = glPan&0x000000FF;
        *(pro_dma_buff+38) = (glPan >> 8)&0x0000FF;
        *(pro_dma_buff+39) = (glPan >> 16)&0x00FF;
        *(pro_dma_buff+40) = (glPan >> 24)&0xFF;
        CurChecksum += glPan;
        
        *(pro_dma_buff+41) = glTilt&0x000000FF;
        *(pro_dma_buff+42) = (glTilt >> 8)&0x0000FF;
        *(pro_dma_buff+43) = (glTilt >> 16)&0x00FF;
        *(pro_dma_buff+44) = (glTilt >> 24)&0xFF;
        CurChecksum += glTilt;

		usStoredBrightStep = LED_Driver_GetBrightnessStep();	
		//dbg_printf(3,"\r\n Get gsBrightness=%d\r\n",gsBrightness);
		//dbg_printf(3,"\r\n Get gsZoom=%d\r\n",gsZoom);
		//dbg_printf(3,"\r\n Get usStoredBrightCode=%d\r\n",usStoredBrightCode);
        *(pro_dma_buff+45) = usStoredBrightStep;
        CurChecksum += usStoredBrightStep;
        //dbg_printf(3," Get usStoredBrightCode LSB=%x MSB=%x \r\n",*(pro_dma_buff+45),*(pro_dma_buff+46));  
                
        if(CurChecksum != usChecksum)
        {        
            *(pro_dma_buff+46) = CurChecksum&0x00FF;
        	*(pro_dma_buff+47) = (CurChecksum >> 8)&0xFF;
        
	        dbg_printf(3," ======================= Write to flash =====================\r\n"); 

	        //MMPF_SF_WriteData((CUSTOMER_INIT_FLASH_ADDR_VP), (MMP_LONG)VideoPropertyDataBuf, LEN_VIDEO_PROPERTY);
	        //MMPF_SF_WriteData((CUSTOMER_INIT_FLASH_ADDR_VP), (MMP_LONG)VideoPropertyDataBuf, sizeof(VideoPropertyDataBuf));
	        
	        ////MoveData2SIF((CUSTOMER_INIT_FLASH_ADDR_VP), (MMP_LONG)VideoPropertyDataBuf, CUSTOMER_INIT_FLASH_SIZE);
	        ////MoveData2SIF((CUSTOMER_INIT_FLASH_ADDR_VP), (MMP_LONG)VideoPropertyDataBuf, sizeof(VideoPropertyDataBuf));
	        
	        //MMPF_SF_EraseSector(CUSTOMER_INIT_FLASH_ADDR_VP);
	        //MMPF_SF_WriteData((CUSTOMER_INIT_FLASH_ADDR_VP), CUSTOMER_TABLE_VP_BUF_STA, CUSTOMER_INIT_FLASH_SIZE);
	        
	        MoveData2SIF((CUSTOMER_INIT_FLASH_ADDR_VP), CUSTOMER_TABLE_VP_BUF_STA, CUSTOMER_INIT_FLASH_SIZE);
	        
	        MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR_VP, CUSTOMER_TABLE_VP_DUMP, CUSTOMER_INIT_FLASH_SIZE);
	      
	        //dbg_printf(3," Get gsBrightness LSB=%x MSB=%x \r\n",(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+4)),(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+5)));      
	        //dbg_printf(3," Get gsZoom LSB=%x MSB=%x \r\n",(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+35)),(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+36)));      
	        //dbg_printf(3," Get usStoredBrightCode LSB=%x MSB=%x \r\n",(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+45)),(*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+46)));
	              
        }else dbg_printf(3," ======================= No need to save video properties =====================\r\n"); 
    
    	for(i=4; i<48;i++)dbg_printf(0,"read videopro[%d] = %x \r\n",i, (*(MMP_UBYTE *)(CUSTOMER_TABLE_VP_DUMP+i)));
      #if 0
    // Tag
        *(pro_dma_buff+0) = 0x50; // Ascii: P
        *(pro_dma_buff+1) = 0x56; // Ascii: V
 
    // Version
        *(pro_dma_buff+2) = 0x01;
        *(pro_dma_buff+3) = 0x00;
   
        MMPF_SF_WriteData((CUSTOMER_INIT_FLASH_ADDR_VP), CUSTOMER_TABLE_VP_BUF_STA, 4); 
	#endif
        /*MoveData2SIF(CUSTOMER_INIT_FLASH_ADDR_VP, (MMP_ULONG)&VideoPropertyDataBuf, 4); 
        //=================================================================================================
		pro_dma_buff = (MMP_BYTE *)CUSTOMER_TABLE_VP_BUF_STA;
		        
        MMPF_SF_ReadData(CUSTOMER_INIT_FLASH_ADDR_VP, pro_dma_buff, 4);
        
        for(i=0; i<4;i++)dbg_printf(0,"read videopro[%d] = %x \r\n",i, *(pro_dma_buff+i));*/
        //=================================================================================================
        
        RTNA_DBG_Str3("\r\n Store Video Property() Finished... \r\n");    
}

MMP_USHORT pCamTaskInited = 0 ;
extern MMP_ULONG glSystemEvent;

void MMPF_PCAM_Task(void)
{
    MMPF_TASK_CFG        task_cfg;
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_msg_t *msg ;
//    RTNA_DBG_Str3("MMPF_PCAM_Task,Payload Header Size:");
//    RTNA_DBG_Byte3(FRAME_PAYLOAD_HEADER_SZ);
//    RTNA_DBG_Str3("\r\n");
    //dbg_printf(3, "#UVC Payload Header Size:%d,PU Bits:%x,CT Bits:%x\r\n",FRAME_PAYLOAD_HEADER_SZ,PU_CS_VAL>>1,CT_CS_VAL>>1);

    #if (SUPPORT_ROTARY_ENCODER)
    AITPS_AIC   pAIC 	= AITC_BASE_AIC;
    MMP_UBYTE bit_val;
    MMP_BOOL IsPreviewEnable;
    MMP_ULONG glKeybounceCount = 0;
    MMP_USHORT usCurBrightCode = 0;
    MMP_UBYTE ubCurBrightStep = 1; // 16 steps: 1 ~ 16
    #endif


	
	RTNA_DBG_Str0("=================PCAM_TASK==================\r\n");
	//GetVideoPro();
    #if (MIC_SOURCE==MIC_IN_PATH_I2S)
    MMPF_Init_ExteralDac();
    #endif
 
// Start enum at last task (lowest prio)    
    MMPF_PCAM_StartUSB();
    MMPF_Init_DefaultClock();
    MMPF_Init_DefGain();

    uvc_init_data(CUSTOMER_TABLE_VP_DUMP, CUSTOMER_TABLE_BUF_STA); 


    pCamTaskInited = 1;
    while(TRUE) {
        msg = MMPF_PCAM_GetMsg();
        if(!msg) {
            continue ;
        }
#if 0
        RTNA_DBG_Str(3,"msg->msg_id:");
        RTNA_DBG_Short(3,msg->msg_id);
        RTNA_DBG_Str(3,"\r\n");
#endif        
        switch(msg->msg_id) {
  #if OMRON_FDTC_SUPPORT 
    case PCAM_MSG_FDTC_START:
    {
        STREAM_CFG *cur_stream ;
        MMP_UBYTE pipe_id ;
        MMP_ULONG tick ;
        cur_stream = usb_get_cur_fdtc_pipe();//usb_get_cur_image_pipe_by_epid(pCamCtx.epId);
        pipe_id = usb_get_cur_image_pipe_id(cur_stream,PIPE_CFG_NV12);
      tick = MMPF_BSP_GetTick();                
      myFaceProcess((char*)glFDTC_CurrentBuffer,cur_stream->pipe_w[pipe_id],cur_stream->pipe_h[pipe_id]);
      tick = MMPF_BSP_GetTick() - tick;
      dbg_printf(0, "FDTC time = %d\r\n", tick);
      gbFDTC_Done = 1;
      break;
        }
  #endif
        case PCAM_MSG_USB_INIT:
            err = PCAM_USB_Init(msg);
            break;
        case PCAM_MSG_USB_PREVIEW_START:
            err = PCAM_USB_PreviewStart(msg);
            break;
        case PCAM_MSG_USB_PREVIEW_STOP:
            err = PCAM_USB_PreviewStop(msg);
            break;
        case PCAM_MSG_USB_EXIT:
            err = PCAM_USB_Exit(msg);
            break;
        case PCAM_MSG_USB_UPDATE_OSD:
            err = PCAM_USB_UpdateOSD(msg);
            break;
        case PCAM_MSG_USB_SET_ATTRS:
            err = PCAM_USB_SetAttrs(msg);
            break;    
        case PCAM_MSG_USB_CAPTURE:
            err = PCAM_USB_TakePicture(msg);
            break ;  
        case PCAM_MSG_USB_AUDIO_START:
            err = PCAM_USB_AudioStart(msg);
            break;
        case PCAM_MSG_USB_AUDIO_STOP:
            err = PCAM_USB_AudioStop(msg);
            break;
        case PCAM_MSG_USB_AUDIO_VOL:
            err = PCAM_USB_SetVolume(msg);
            break;
        case PCAM_MSG_USB_AUDIO_MUTE:
            err = PCAM_USB_SetMute(msg);
            break;
        case PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE:
            err = PCAM_USB_SetSamplingRate(msg);
            break;
        case PCAM_MSG_USB_UPDATE_FW:
            err = PCAM_USB_UpdateFirmware(msg) ;
            break; 
        case PCAM_MSG_USB_CHECK_ALIVE:
            err = PCAM_USB_TaskAlive(msg);
            break ; 
        case PCAM_MSG_USB_CAPTURE_RAW:
            err = PCAM_USB_TakeRawPicture(msg);
            break ;  

        #if (SUPPORT_LED_DRIVER) && (SUPPORT_ROTARY_ENCODER)
        case PCAM_MSG_OTHERS:

            if(glSystemEvent & ROTARY_SWITCH_CW)
            {
                //dbg_printf(0, "ROTARY_SWITCH_CW\r\n");

                // Bouncing time
                MMPF_OS_Sleep_MS(8);

                glSystemEvent &= ~(ROTARY_SWITCH_CW);

                MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
                
                if(IsPreviewEnable)
                {
                   // Close PIO ISR
                   RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_GPIO);

                   // Get current brightness code & step
                   usCurBrightCode = LED_Driver_GetBrightnessCode();
                   ubCurBrightStep = LED_Driver_GetBrightnessStep();
                   dbg_printf(3, "Last step: %d (%d)\r\n",ubCurBrightStep,usCurBrightCode);

                   // Error handle
                   if((usCurBrightCode < 0 ) || (usCurBrightCode > LM36922_MAX_BRIGHT))
                   {
                       dbg_printf(3, "Error! LED_Driver_GetBrightnessCode %d\r\n",usCurBrightCode);
                       usCurBrightCode = LM36922_MAX_BRIGHT;
                   }
                   else
                   {
                       if(ubCurBrightStep < LM36922_BRIGHT_STEP)
                          ++ubCurBrightStep;
                       else
                          ubCurBrightStep = LM36922_BRIGHT_STEP;

			  //if(ubCurBrightStep == 0)
			  //	usCurBrightCode = 0;
			  //else
                       //     usCurBrightCode = ((ubCurBrightStep -1)*LM36922_BRIGHT_PERSTEP) + LM36922_Min_BRIGHT;
                       dbg_printf(3, "Current step: %d\r\n",ubCurBrightStep);
			 LED_Driver_SetBrightnessStep(ubCurBrightStep);
                       // Set brightness code
                       //LED_Driver_SetBrightnessCode(usCurBrightCode);
                   }

                   // Enable PIO ISR
                   RTNA_AIC_IRQ_En(pAIC, AIC_SRC_GPIO);
                }

            }       
            else if(glSystemEvent & ROTARY_SWITCH_CCW)
            {
                // dbg_printf(0, "ROTARY_SWITCH_CCW\r\n");

                // Bouncing time
                MMPF_OS_Sleep_MS(8);

                glSystemEvent &= ~(ROTARY_SWITCH_CCW);

                MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
                
                if(IsPreviewEnable)
                {
                   // Close PIO ISR
                   RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_GPIO);

                   // Get current brightness code & step
                   usCurBrightCode = LED_Driver_GetBrightnessCode();
                   ubCurBrightStep = LED_Driver_GetBrightnessStep();
                   dbg_printf(3, "Last step: %d (%d)\r\n",ubCurBrightStep,usCurBrightCode);

                   // Error handle
                   if((usCurBrightCode < 0 ) || (usCurBrightCode > LM36922_MAX_BRIGHT))
                   {
                       dbg_printf(3, "Error! LED_Driver_GetBrightnessCode %d\r\n",usCurBrightCode);
                       usCurBrightCode = 0;
                   }
                   else
                   {
                       if(ubCurBrightStep == 0)
                          ubCurBrightStep = 0;
                       else
                          --ubCurBrightStep;

			  //if(ubCurBrightStep == 0)
			  //	usCurBrightCode = 0;
			  //else
                       //     usCurBrightCode = ((ubCurBrightStep -1)*LM36922_BRIGHT_PERSTEP) + LM36922_Min_BRIGHT;
                       dbg_printf(3, "Current step: %d (%d)\r\n",ubCurBrightStep);

                       // Set brightness code
                       //LED_Driver_SetBrightnessCode(usCurBrightCode);
                        LED_Driver_SetBrightnessStep(ubCurBrightStep);
                   }

                   // Enable PIO ISR
                   RTNA_AIC_IRQ_En(pAIC, AIC_SRC_GPIO);
                }

            }

            break;
        #endif

        }
        msg->err = err;
        /*Handler processed done*/
        /*Release semaphore*/
        if(msg->msg_sem) {
            MMPF_OS_ReleaseSem(*msg->msg_sem);
        } else {
            pcam_free_msg(msg);
        }
    }
}


pcam_msg_t *MMPF_PCAM_GetMsg(void)
{
    MMPF_OS_FLAGS flags;
    pcam_msg_t *msg;
    /*
    Get message first in case there has multiple messages
    */
    #if 1
    // get non-overwrite msg first
    msg = (pcam_msg_t *)pcam_get_msg();
    if(msg) {
        return msg ;
    }
    else {
    // get overwrite msg
        msg = (pcam_msg_t *)pcam_get_overwr_msg(-1,-1);
        if(msg) {
            return msg;
        }
    }
    #endif
    /*
    Wait mesage event
    */
    MMPF_OS_WaitFlags(PCAM_Flag, (PCAM_FLAG_MSG | PCAM_FLAG_OVERWR_MSG),
                     (MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME),
                     0, &flags);
    if(flags & PCAM_FLAG_OVERWR_MSG) {
        msg = (pcam_msg_t *)pcam_get_overwr_msg(-1,-1);
        if(msg) {
            return msg ;
        }
    }                     
    if(flags & PCAM_FLAG_MSG) {
        msg = (pcam_msg_t *)pcam_get_msg();
        if(msg) {
            return msg ;
        }
    }
    return (pcam_msg_t *)0;
}

MMP_USHORT PCAM_USB_RestartSensor(pcam_msg_t *msg)
{
    dbg_printf(0,"@Sensor Restart\r\n");
    return PCAM_ERROR_NONE ;
}

MMP_USHORT PCAM_USB_Init(pcam_msg_t *msg)
{
    return PCAM_ERROR_NONE ;
}

#include "mmp_reg_rawproc.h"

#if SUPPORT_GRA_ZOOM
//MMPF_SCALER_SOURCE gScalSrc;
MMP_USHORT  gsZoomRatio[2], gsLastZoomRatio[2],gsZoomRatioM[2];
extern MMP_BYTE gbGRAEn[];

#endif
//extern PCAM_USB_VIDEO_RES gsVidRes;
MMP_USHORT PCAM_USB_PreviewStart(pcam_msg_t *msg)
{
extern MMP_ULONG glSkypeH264Hdr ;
extern MMP_ULONG glPCCAM_VIDEO_BUF_ADDR;
extern MMP_ULONG glPCCAM_VIDEO_BUF_MAX_SIZE;
extern MMP_ULONG glUSB_UVC_DMA_EP1_BUF_ADDR[];
extern MMP_ULONG glUSB_UVC_DMA_EP2_BUF_ADDR[];
extern MMP_ULONG glUSB_UVC_ZERO_BUF_ADDR ;
extern MMP_USHORT  gsSensorMCModeWidth, gsSensorMCModeHeight;
extern MMP_UBYTE gbCurH264Type ;
//extern MMP_ULONG glPccamResolution ;pCamEpId
extern MMP_USHORT  UVCTxFIFOSize[] ;
extern MMP_ULONG m_glISPBufferStartAddr;
extern MMP_UBYTE gbUsbHighSpeed;
extern MMP_USHORT gsCurFrameRatex10[] ;
extern MMP_ULONG    glPccamResolution;

static MMP_BOOL     gbIsGraSrcBufAllocated = MMP_FALSE ;
static STREAM_CTL   gsNV12StreamCtl ;

    MMP_SHORT       init_count = 3;
    MMP_ERR         err = MMP_ERR_NONE ;
    MMP_ULONG       cur_buf, ulSize,ulIspDmaOprSize;
    MMP_ULONG       pccam_compbuf_size, pccam_linebuf_size;
    AITPS_GBL       pGBL = AITC_BASE_GBL;
    AITPS_AIC       pAIC = AITC_BASE_AIC;
    PCAM_USB_INFO   *pcam_info = (PCAM_USB_INFO *)msg->msg_data ;
    STREAM_CFG      *cur_stream ;
    STREAM_CTL      cur_bctl ;
    AITPS_RAWPROC   pRAW = AITC_BASE_RAWPROC;
    AITPS_VIF       pVIF = AITC_BASE_VIF;
    MMP_ULONG       t1_sensor_init ;
    MMP_USHORT      aligned_header_size ,aligned_off = 0;
    int             workss = 0 ;
    PCAM_USB_VIDEO_RES ep_res;
    PCAM_USB_VIDEO_FORMAT ep_fmt;
    STREAM_SESSION  *ss ;
    MMP_UBYTE       pipe_id ;
    MMP_ULONG       end_h264_sram_addr  ;
    MMPS_SYSTEM_SRAM_MAP *psrammap = MMPS_System_GetSramMap();
#if SENSOR_IN_H > SENSOR_16_9_H
    MMP_USHORT      scaler_in_h ;
#endif  
    MMP_BOOL        force_uvc15 = MMP_FALSE ;
#if SUPPORT_PCSYNC
    if(pcsync_is_preview()) {
        force_uvc15 = MMP_TRUE ;
    }
#endif
 
#if 1    
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_TRUE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);
#endif    
    MMPD_System_TuneMCIPriority(2);
    
    workss = MMPF_Video_GetWorkingStreamSession();
    ep_res = GetResIndexByEp(pcam_info->pCamEpId);
    ep_fmt = GetFmtIndexByEp(pcam_info->pCamEpId);

    if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV422) {
        if(GetYUY2FrameSize(ep_res) >= 640*360*2) {
            MMPD_System_TuneMCIPriority(3);
        }    
    } else {
        MMP_BOOL is_preview ;    
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview,STREAM_EP_MJPEG);     
        if(is_preview) {
            STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(PIPE_0) ;   
            if(cur_pipe) {
                if ( PIPE_EN(cur_pipe->pipe_en) & PIPE0_EN ) {
                    MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(PIPE_0,cur_pipe->pipe_cfg) ; 
                    if( (pipe_cfg==PIPE_CFG_YUY2) && ( cur_pipe->pipe_w[0] * cur_pipe->pipe_h[0] > 640*480 )) {
                        MMPD_System_TuneMCIPriority(3);
                    }
                }
            }
        }
    }

    dbg_printf(3,"PCAM_USB_PreviewStart epid:%d\r\n",pcam_info->pCamEpId);
    
    glCurSensorGain  = 0; //Reset the saved gain for each preview
   // gsKeepCurSlotNum = 0;
#if SUPPORT_GRA_ZOOM
    //
    //
    // sean@2013_02_15 DEL    
    //if( IS_FIRST_STREAM(workss) ) {
    //    gsZoomRatio = gsLastZoomRatio  = ZOOM_RATIO_BASE;
    //}
    
    
    if(pcam_info->pCamEpId==0) {
        gbGRAEn[0] = MMP_FALSE ;
        gsZoomRatio[0] = gsLastZoomRatio[0] = ZOOM_RATIO_BASE;
        MMPF_Scaler_ResetPathInfo(GRA_DST_PIPE_1);
        if(UVC_VCD()==bcdVCD10) {
            gbGRAEn[1] = MMP_FALSE ;
            gsZoomRatio[1] = gsLastZoomRatio[1] = ZOOM_RATIO_BASE;
            MMPF_Scaler_ResetPathInfo(GRA_DST_PIPE_2);
        }
    }
    else {
        gbGRAEn[1] = MMP_FALSE ;
        gsZoomRatio[1] = gsLastZoomRatio[1] = ZOOM_RATIO_BASE;
        MMPF_Scaler_ResetPathInfo(GRA_DST_PIPE_2);
    }
#endif    
    // Init H264
    if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_H264) {
        MMPS_3GPRECD_InitVideoInfo(); //init all stream default param
    }
     end_h264_sram_addr = psrammap->ulH264Addr ;// 0x100000;
    
#if ALIGN_PAYLOAD==1
    
    if(FRAME_PAYLOAD_HEADER_SZ > 32) {
        aligned_header_size = 64 ;
    
    } else {
        aligned_header_size = 32 ;
    }
    aligned_off = aligned_header_size - FRAME_PAYLOAD_HEADER_SZ ;
#else
    aligned_header_size = FRAME_PAYLOAD_HEADER_SZ ; 
#endif
    
    //RTNA_DBG_Str3("PCAM_USB_PreviewStart\r\n");
 //   dbg_printf(1,"#HZ : %d,Align HZ:%d,OFF H:%d\r\n",FRAME_PAYLOAD_HEADER_SZ,aligned_header_size,aligned_off);
    pCamCtx.epId = pcam_info->pCamEpId ;
    pCamCtx.videoQuality = PCAM2MMP_Get_Quality(pcam_info);
    pCamCtx.videoRes = PCAM2MMP_Get_Resolution(pcam_info);
    pCamCtx.videoFormat = PCAM2MMP_Get_VideoFormat(pcam_info);
    pCamCtx.audioFormat = PCAM2MMP_Get_AudioFormat(pcam_info);
    pCamCtx.debandMode = PCAM2MMP_Get_DebandMode(pcam_info);
    m_VideoFmt = pCamCtx.videoFormat ;
    m_usVideoQuality = pCamCtx.videoQuality ;

    gsVideoFormat = pCamCtx.videoFormat;

    dbg_printf(0, "** videoRes=%d, videoFormat=%d\r\n",
                  pCamCtx.videoRes, pCamCtx.videoFormat /*, glPccamResolution*/);

    cur_stream = usb_get_cur_image_pipe_by_epid(pcam_info->pCamEpId);
    dbg_printf(3,"cur_stream(%x),pipe_en:%x,pipe_cfg:%x,ep_id:%d\r\n",cur_stream,cur_stream->pipe_en,cur_stream->pipe_cfg,pcam_info->pCamEpId);
    
    ss = MMPF_Video_GetStreamSessionByEp(pcam_info->pCamEpId);
    //sean@2012_02_15 fixed bug when video endpoint is 1
    //if( workss < VIDEO_EP_END() ) {
    if( IS_FIRST_STREAM(workss) ) {
        PCAM_USB_Set_DefScalerClipRange();
        
        gbIsGraSrcBufAllocated = MMP_FALSE ;
        MMPF_SYS_InitFB(0);
        MMPF_SYS_InitFB(1);
        // Allocate FrameBuffer for 3A
        // Allocate FrameBuffer for 3A
        #if ISP_BUF_IN_SRAM==0
        ulSize = MMPF_Sensor_CalculateBufferSize();
        #else
        MMPF_Sensor_CalculateBufferSize(&ulSize,&ulIspDmaOprSize);
        #endif
        cur_buf = (MMP_ULONG)MMPF_SYS_AllocFB("ISP_IQ",ulSize,32,0);
        
        #if ISP_BUF_IN_SRAM==0
        MMPF_Sensor_AllocateBuffer(cur_buf);
        #else
        MMPF_Sensor_AllocateBuffer(cur_buf,MMPS_System_GetSramMap()->ulISPDmaOprAddr);
        #endif
        

        //cur_stream = usb_get_cur_image_pipe(pcam_info->pCamEpId);

        //RTNA_DBG_Str(3, "PCAM_USB_PreviewStart()\r\n");
        // 1. Initialize sensor.
        t1_sensor_init = usb_vc_cur_sof();
 init_sensor:
        MMPF_Sensor_SetErr(MMP_ERR_NONE);

        dbg_printf(0,"@ PCAM_USB_PreviewStart, gsCurFrameRatex10[0]: %d\r\n",gsCurFrameRatex10[0]);

        if(gsCurFrameRatex10[0] > 300) {
            MMPS_Sensor_Initialize(gsCurrentSensor, 0, SENSOR_VIDEO_PRW_MODE);
        } else {
            MMPS_Sensor_Initialize(gsCurrentSensor, 1, SENSOR_VIDEO_PRW_MODE);
        }
        
        glUSB_UVC_DMA_EP1_BUF_ADDR[0]  = (MMP_ULONG)MMPF_SYS_AllocFB("UVC10",USB_DMA_BUF_SIZE ,32,0);
#if EP1_DMA_LIST_BUF_CNT==2
        glUSB_UVC_DMA_EP1_BUF_ADDR[1]  = (MMP_ULONG)MMPF_SYS_AllocFB("UVC11",USB_DMA_BUF_SIZE ,32,0);
#endif
        glUSB_UVC_DMA_EP2_BUF_ADDR[0]  = (MMP_ULONG)MMPF_SYS_AllocFB("UVC20",USB_DMA_BUF_SIZE ,32,0);
#if EP2_DMA_LIST_BUF_CNT==2
        glUSB_UVC_DMA_EP2_BUF_ADDR[1]  = (MMP_ULONG)MMPF_SYS_AllocFB("UVC21",USB_DMA_BUF_SIZE ,32,0);
#endif
#if EP1_DMA_LIST_BUF_CNT==1
        glUSB_UVC_DMA_EP1_BUF_ADDR[1] = glUSB_UVC_DMA_EP1_BUF_ADDR[0];
#endif
#if EP2_DMA_LIST_BUF_CNT==1        
        glUSB_UVC_DMA_EP2_BUF_ADDR[1] = glUSB_UVC_DMA_EP2_BUF_ADDR[0];
#endif 
        
        #if H264_CBR_PADDING_EN==1 // CBR bit stuffing
        glUSB_UVC_ZERO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("ZERO", H264_CBR_PADDING_BS  , 32, 0 );
        #endif
        #if SUPPORT_OSD_FUNC==1 // 32x32 rectangle
        pcam_Osd_AllocBuffer(32,32);
        #endif
       
    }
    dbg_printf(3,"pcam ss->streamaddr(%d) : %x\r\n",pcam_info->pCamEpId,ss->streamaddr);
    if(ss->streamaddr) {
        MMPF_SYS_SetCurFBAddr((char *)ss->streamaddr,pcam_info->pCamEpId);
    } else {
        MMP_LONG freesize ;
        ss->streamaddr = (MMP_ULONG)MMPF_SYS_GetCurFBAddr((MMP_LONG *)&freesize,pcam_info->pCamEpId);
    }
    dbg_printf(3,"pcam ss->streamaddr : %x,pipe en:%x,pipe cfg:%x\r\n",ss->streamaddr,cur_stream->pipe_en,cur_stream->pipe_cfg);
    dbg_printf(3,"pcam ss->pipe_id : %x\r\n",ss->pipe_id);
    if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV422) {
        glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(ep_res) ; 
        pipe_id =  usb_get_cur_image_pipe_id(cur_stream ,PIPE_CFG_YUY2);  
    }   
    else if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV420) {
        glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUV420FrameSize(ep_res ) ;
        pipe_id =  usb_get_cur_image_pipe_id(cur_stream ,PIPE_CFG_NV12);
    }
    else if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_MJPEG) {
        glPCCAM_VIDEO_BUF_MAX_SIZE = GetMJPEGFrameSize(ep_res ) ;
        pipe_id =  usb_get_cur_image_pipe_id(cur_stream ,PIPE_CFG_MJPEG);
    }
  else if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_RAW){
    glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(ep_res ) ;
    pipe_id =  usb_get_cur_image_pipe_id(cur_stream ,PIPE_CFG_YUY2);    
  }
    else {
    // H264 : Complicate, TBD
        if(gbCurH264Type==SKYPE_H264) {
            glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(PCCAM_640_480) / 2  ;    
        }
        else  {
            glPCCAM_VIDEO_BUF_MAX_SIZE = GetH264FrameSize( ep_res ) ;
        }    
        pipe_id =  usb_get_cur_image_pipe_id(cur_stream ,PIPE_CFG_H264);
    }
    dbg_printf(3,"#Cur ep id :%d,pipe id : %d\r\n",pCamCtx.epId,pipe_id);
    // Align to 32bytes 
    glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
    
    glPCCAM_VIDEO_BUF_MAX_SIZE += aligned_header_size; 
    
    dbg_printf(3,"glPCCAM_VIDEO_BUF_MAX_SIZE:%d KB\r\n", glPCCAM_VIDEO_BUF_MAX_SIZE >> 10);
    
    // Can't put here for 2 pins streaming
    //UVCX_EnableSliceBasedTx( 0 ); // Also disable in non-h264 format.
    
    if( ep_fmt == PCAM_USB_VIDEO_FORMAT_H264) {
#if SENSOR_IN_H > SENSOR_16_9_H
        MMP_USHORT user_ratio = 0 ;
#endif     
        MMP_USHORT usEncWidth[2], usEncHeight[2];
        MMP_BOOL allocate_jpeg_after_h264 = 0 ;
        MMP_USHORT jpeg_slot = 3 ;
        MMP_USHORT slot_div = 1 ; // 
    
#if H264_SIMULCAST_EN
        if( IS_FIRST_STREAM(workss) ) {
            glGRAPreviewFormat = 0xFF ;
        }
#endif    
        UVCX_EnableSliceBasedTx( 0 );    
    // Start Video Engine
    // Set preview mode.
       // MMPS_3GPRECD_SetPreviewMode(1);
    // Set video format
   
       if ( (gbCurH264Type==UVC_H264) || (gbCurH264Type==SKYPE_H264) || (gbCurH264Type==FRAMEBASE_H264_YUY2) || (gbCurH264Type==FRAMEBASE_H264_MJPEG) ){
#if USB_UVC_H264
            MMP_UBYTE pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);//(cur_stream->pipe_cfg) & PIPE_CFG_MASK ;
            dbg_printf(3,"--<PIPE0_CFG:%d>--\r\n",pipe0_cfg);
            UVCX_EnableSliceBasedTx( 0 );

            if(gbCurH264Type == SKYPE_H264) {
                MMP_UBYTE *ptr ;
                MMP_USHORT i;
                glSkypeH264Hdr = (MMP_ULONG)MMPF_SYS_AllocFB("HDR",glPCCAM_VIDEO_BUF_MAX_SIZE * 2 + 4 + 4 ,32,pcam_info->pCamEpId ) ;
                ptr = (MMP_UBYTE *)glSkypeH264Hdr ;
                for ( i = 0 ; i <(glPCCAM_VIDEO_BUF_MAX_SIZE * 2 + 4 + 4) ;i++) {
                    ptr[i]= 0 ;
                }
                
            }
            if(pipe0_cfg==PIPE_CFG_MJPEG) {
                MMP_USHORT slot ;
                // 1/2 size of YUY2
                //VAR_W(0,cur_cfg->wWidth);
                //VAR_W(0,cur_cfg->wHeight);
                dbg_printf(1,"#H264 W:%d,H:%d\r\n",cur_stream->pipe_w[PIPE_1],cur_stream->pipe_h[PIPE_1]);
                
                glPCCAM_VIDEO_BUF_MAX_SIZE =  (cur_stream->pipe_w[PIPE_1] * cur_stream->pipe_h[PIPE_1]  * 3 )/ 2 ;
                // 1/8 size of YUV420
                glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE / MIN_H264E_COMP_RATIO ) + aligned_header_size;
                if(glPCCAM_VIDEO_BUF_MAX_SIZE < ( 320*240*3/2 ) )
                    glPCCAM_VIDEO_BUF_MAX_SIZE = 320*240*3/2 ;
                    
                    
                glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                    
                if (MMPF_VIDENC_GetOutputSyncMode()== MMPF_VIDENC_OUTPUT_SLICE_SYNC) {
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",4*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    MMPF_SetPCCAMCompressBuf(cur_buf,(glPCCAM_VIDEO_BUF_MAX_SIZE - FRAME_PAYLOAD_HEADER_SZ));
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = 12;
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE/3;
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;    
                    cur_bctl.frame_num = 4 ; 
                }
                else {
                    slot = 3 ;
                    if(/*UVCX_IsSimulcastH264()*/SIM_H264) {
                        slot = 6;//6 ;
                        //dbg_printf(3,"---slots : %d\r\n",slot);
                    }
                    
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",slot*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = slot  ; // 3 ring buffer 
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ; 
                    cur_bctl.frame_num = slot ;    
                }
                cur_stream->pipe_b[PIPE_1] = cur_bctl ;
                gbEnableJpegRC = 0 ;
                
                #if (CHIP==P_V2)
                if( (cur_stream->pipe_w[PIPE_1]==1920) && ( cur_stream->pipe_h[PIPE_1]==1080 )) {
                     
                    //jpeg_slot = 3;
                    allocate_jpeg_after_h264 = 1;
                    #if RC_JPEG_TARGET_SIZE > 0
                    gbEnableJpegRC = 1 ;
                    #endif
                }
                #endif
                
                if(allocate_jpeg_after_h264==0) {                
                    glPCCAM_VIDEO_BUF_MAX_SIZE = GetMJPEGFrameSize(ep_res) + aligned_header_size ;
                   // VAR_W(0,pcam_get_info()->pCamVideoRes);
                    // If 
                    glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                    
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("JPEG",jpeg_slot*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = jpeg_slot  ; // 3 ring buffer
                    cur_bctl.frame_num = jpeg_slot ;  
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                    cur_stream->pipe_b[PIPE_0] = cur_bctl ;
                    // For JPEG 
                    MMPF_SetPCCAMCompressBuf(glPCCAM_VIDEO_BUF_ADDR,(glPCCAM_VIDEO_BUF_MAX_SIZE-aligned_header_size));
                    #if H264_SIMULCAST_EN==1
                    if(  /*UVCX_IsSimulcastH264()*/SIM_H264 ) {
                        glGRAPreviewFormat = PIPE_CFG_MJPEG; // JPEG
                        glGRAPreviewWidth  = cur_stream->pipe_w[PIPE_0] ;
                        glGRAPreviewHeight = cur_stream->pipe_h[PIPE_0] ;
                    }
                    #endif
                 }
                
            }
            // sean@2011_01_22 Dual Stream YUY2 + H264
            else if( (pipe0_cfg==PIPE_CFG_YUY2)||( pipe0_cfg==0 )) {
            //    dbg_printf(1,"#H264 W:%d,H:%d\r\n",cur_cfg->wWidth,cur_cfg->wHeight);
                glPCCAM_VIDEO_BUF_MAX_SIZE =   (cur_stream->pipe_w[PIPE_1] * cur_stream->pipe_h[PIPE_1]  * 3 )/ 2 ;
                // 1/8 size of YUV420
                glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE / MIN_H264E_COMP_RATIO ) + aligned_header_size    ;
                if(glPCCAM_VIDEO_BUF_MAX_SIZE < ( 320*240*3/2 ) )
                    glPCCAM_VIDEO_BUF_MAX_SIZE = 320*240*3/2 ;
                glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                    
                if (MMPF_VIDENC_GetOutputSyncMode()== MMPF_VIDENC_OUTPUT_SLICE_SYNC) { //slice based
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",4*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    MMPF_SetPCCAMCompressBuf(cur_buf,(glPCCAM_VIDEO_BUF_MAX_SIZE - FRAME_PAYLOAD_HEADER_SZ));
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = 12;
                    cur_bctl.frame_num = 4 ;
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE/3;
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                }
                else {
                    MMP_USHORT slot = 3 ;
                    if( /*UVCX_IsSimulcastH264()*/SIM_H264) {
                        slot = 6 ;
                    }
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",slot*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = slot  ; // 3 ring buffer 
                    cur_bctl.frame_num = slot ;
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                }
                cur_stream->pipe_b[PIPE_1] = cur_bctl ;
                
                if(pipe0_cfg==PIPE_CFG_YUY2) {
                #if SUPPORT_GRA_ZOOM
                    // patrick increases YUV buffer size for GRA zoom function
                    glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(PCCAM_2304_1296) + aligned_header_size  ; 
                #else
                  glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(ep_res) + aligned_header_size  ;
                #endif    
                    //VAR_W(0,glPccamResolution);
                //    dbg_printf(1,"#YUY2 Res : %d\r\n",glPccamResolution);
                    
                    glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                    glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("YUY2",2*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                    cur_bctl.rd_index = cur_bctl.wr_index = 0;
                    cur_bctl.total_rd = cur_bctl.total_wr = 0;
                    cur_bctl.slot_num = 2  ; // 3 ring buffer 
                    cur_bctl.frame_num = 2 ;
                    cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                    cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                    cur_stream->pipe_b[PIPE_0] = cur_bctl ;
                    
                    #if H264_SIMULCAST_EN==1
                    if(  /*UVCX_IsSimulcastH264()*/SIM_H264 ) {
                        glGRAPreviewFormat = PIPE_CFG_YUY2; // YUY2
                        glGRAPreviewWidth  = cur_stream->pipe_w[PIPE_0] ;
                        glGRAPreviewHeight = cur_stream->pipe_h[PIPE_0] ;
                    }
                    #endif
                    
                    
                } else {
                #if H264_SIMULCAST_EN==1
                    glGRAPreviewFormat = 0xFF; 
                #endif    
                }
                
            } 
#endif            
        } else {
        #if SUPPORT_PCSYNC
            if(pcsync_is_preview() ) {
                UVCX_EnableSliceBasedTx(0);
            }
            else {
                UVCX_EnableSliceBasedTx(SLOT_RING);
            }
        #else
            UVCX_EnableSliceBasedTx(SLOT_RING);
        #endif
            if (MMPF_VIDENC_GetOutputSyncMode() == MMPF_VIDENC_OUTPUT_SLICE_SYNC) { //slice based
                MMP_USHORT more_buf = 1 ;
                if(SIM_H264) {
                    more_buf = 2 ;
                }
                cur_bctl.rd_index = cur_bctl.wr_index = 0;
                cur_bctl.total_rd = cur_bctl.total_wr = 0;
                cur_bctl.slot_num = 60 * more_buf;
                cur_bctl.frame_num = 3 * more_buf;
                cur_bctl.slot_size =  (glPCCAM_VIDEO_BUF_MAX_SIZE * cur_bctl.frame_num )/cur_bctl.slot_num;
        //align to 32bytes
                cur_bctl.slot_size = (cur_bctl.slot_size + 31 ) & ( -32 );
                glPCCAM_VIDEO_BUF_MAX_SIZE = cur_bctl.slot_size * cur_bctl.slot_num ;
                glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                //MMPF_SetPCCAMCompressBuf(cur_buf,(glPCCAM_VIDEO_BUF_MAX_SIZE - FRAME_PAYLOAD_HEADER_SZ));
                cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;  
            }
            else {
                MMP_USHORT slot = 3 ;
                if( /*UVCX_IsSimulcastH264()*/SIM_H264) {
                    slot = 6 ;
                }
                glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("H264",slot*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                cur_bctl.rd_index = cur_bctl.wr_index = 0;
                cur_bctl.total_rd = cur_bctl.total_wr = 0;
                cur_bctl.slot_num = slot ; // 3 ring buffer 
                cur_bctl.frame_num = slot ;
                cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;
            }     
            cur_stream->pipe_b[pipe_id] = cur_bctl ;
            
            dbg_printf(3,"#### pipe_en : %x,pipe_cfg :%x\r\n",cur_stream->pipe_en,cur_stream->pipe_cfg);
            if( PIPE_EN(cur_stream->pipe_en) & PIPE0_EN) {
                MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg) ;
                if(pipe_cfg ==PIPE_CFG_YUY2) {
                // YUY2
                    
                } 
                else if(pipe_cfg ==PIPE_CFG_MJPEG) {
                // MJPEG, always 320x240 for test
                    if (1/*glPccamResolution==PCCAM_320_240*/) {
                        RTNA_DBG_Str3("FrameBaseH264, JPEG is 320x240\r\n");
                        glPCCAM_VIDEO_BUF_MAX_SIZE = 320 * 240 + aligned_header_size ;
                        glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                        glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("JPEG",3*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                        cur_bctl.rd_index = cur_bctl.wr_index = 0;
                        cur_bctl.total_rd = cur_bctl.total_wr = 0;
                        cur_bctl.slot_num = 3  ; // 3 ring buffer 
                        cur_bctl.frame_num = 3 ;
                        cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                        cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                        cur_stream->pipe_b[PIPE_0] = cur_bctl ;
                        // For JPEG 
                        MMPF_SetPCCAMCompressBuf(glPCCAM_VIDEO_BUF_ADDR,(glPCCAM_VIDEO_BUF_MAX_SIZE-aligned_header_size    ));
                    }                
                }
            }
        }
    // Set video resolution
#if ENCODING_UNIT==1
        if( (UVC_VCD()==bcdVCD15) || (force_uvc15)) {
            dbg_printf(3,"UVC 15 Start Mode\r\n");
            if( SIM_H264 ) {
                UVC15_VIDEO_CONFIG *cfg0,*cfg1 ;
                RES_TYPE_CFG *rescfg;
                MMP_USHORT res1_indx;
                rescfg = GetResCfg(pCamCtx.videoRes) ;
#if SENSOR_IN_H > SENSOR_16_9_H
                user_ratio = rescfg->best_ratio;
                scaler_in_h = rescfg->scaler_input_h;
#endif                 
                cfg0 = UVC15_GetH264StartConfig(0);
                cfg0->res.wWidth =rescfg->res_w;
                cfg0->res.wHeight = rescfg->res_h;
                // NEW
                usEncWidth[0] = ALIGN16(rescfg->res_w);
                if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                    usEncHeight[0] = (rescfg->res_h);
                } else {
                    #if H264_ALIGN_H==1
                    usEncHeight[0] = ALIGN16(rescfg->res_h);
                    #else
                    usEncHeight[0] = (rescfg->res_h);
                    #endif
                }
                
                cfg1 = UVC15_GetH264StartConfig(1);
                
                usEncWidth[1] = ALIGN16(cfg1->res.wWidth);
                if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                    usEncHeight[1] = (cfg1->res.wHeight);
                } else {
                    #if H264_ALIGN_H==1
                    usEncHeight[1] = ALIGN16(cfg1->res.wHeight);
                    #else
                    usEncHeight[1] = (cfg1->res.wHeight);
                    #endif
                }
                
                // Device decide the sub-stream resolution before EU_VIDEO_RESOLUTION_CONTROL
                
                res1_indx = (MMP_USHORT)GetResIndexBySize(cfg1->res.wWidth ,cfg1->res.wHeight);
                UVCX_SetH264Resolution(pCamCtx.videoRes,res1_indx,2);
            
            } else {
                UVC15_VIDEO_CONFIG *cfg ;
                RES_TYPE_CFG *rescfg;
                UVCX_SetH264Resolution(pCamCtx.videoRes,0,1);
                cfg = UVC15_GetH264StartConfig(0);
                rescfg = GetResCfg(pCamCtx.videoRes) ;
#if SENSOR_IN_H > SENSOR_16_9_H
                user_ratio = rescfg->best_ratio;
                scaler_in_h = rescfg->scaler_input_h;
#endif                 
                
                cfg->res.wWidth =rescfg->res_w;
                cfg->res.wHeight = rescfg->res_h;
                // NEW
                usEncWidth[0] = ALIGN16(rescfg->res_w);
                #if H264_ALIGN_H==1
                usEncHeight[0] = ALIGN16(rescfg->res_h);
                #else
                usEncHeight[0] = (rescfg->res_h);
                #endif
                
            }
            // Or ( 1 << EU_AVERAGE_BITRATE_CONTROL) to give a default bitrate when initialize.
            UVC15_SetH264StartMode(UVC15_GetH264StartConfig(0),(1<<EU_START_INIT) | UVC15_GetH264StartBitmap() );
            if( /*UVCX_IsSimulcastH264()*/SIM_H264 ) {
                UVC15_SetH264StartMode(UVC15_GetH264StartConfig(1),(1<<EU_START_INIT) | UVC15_GetH264StartBitmap() );
            }
            #if LYNC_CERT_PATCH
            
            UVC15_SetMultiplexedH264PriorityId();
            #endif
            if(pCamCtx.videoRes==VIDRECD_RESOL_1920x1080) {
                MMPD_H264ENC_SetIMENum(1);
            } else {
                MMPD_H264ENC_SetIMENum(3);
            }
        }
#endif
        
#if USING_UVCX_VIDEO_CONFIG==1
        if( (UVC_VCD()==bcdVCD10)&&(!force_uvc15)) {
        
#if SENSOR_IN_H > SENSOR_16_9_H        
            RES_TYPE_CFG *rescfg;
            rescfg = GetResCfg(pCamCtx.videoRes) ;
            user_ratio = rescfg->best_ratio;
            scaler_in_h = rescfg->scaler_input_h;
#endif
            dbg_printf(3,"UVC 10 Start Mode\r\n");

            if( /*UVCX_IsSimulcastH264()*/SIM_H264) {
                UVCX_VIDEO_CONFIG *cfg ;
                MMP_USHORT res1_indx;
                cfg = UVCX_GetH264StartConfig(1);
                res1_indx = (MMP_USHORT)GetResIndexBySize(cfg->wWidth ,cfg->wHeight);
                UVCX_SetH264Resolution(pCamCtx.videoRes,res1_indx,2);
                
                usEncWidth[1] = ALIGN16(cfg->wWidth);
                
                if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                    usEncHeight[1] = (cfg->wHeight);
                } else {  
                    #if H264_ALIGN_H==1          
                    usEncHeight[1] = ALIGN16(cfg->wHeight);
                    #else
                    usEncHeight[1] = (cfg->wHeight);
                    #endif
                }
                
            } else {
                UVCX_SetH264Resolution(pCamCtx.videoRes,0,1);
            }    
            UVCX_GetH264StartConfig(0)->wWidth  = cur_stream->pipe_w[pipe_id] ;
            UVCX_GetH264StartConfig(0)->wHeight = cur_stream->pipe_h[pipe_id] ;
            // New
            usEncWidth[0] = ALIGN16(cur_stream->pipe_w[pipe_id]);  
            
            if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                usEncHeight[0] = (cur_stream->pipe_h[pipe_id]);
            } else {
                #if H264_ALIGN_H==1          
                usEncHeight[0] = ALIGN16(cur_stream->pipe_h[pipe_id]);
                #else
                usEncHeight[0] = (cur_stream->pipe_h[pipe_id]);
                #endif
            
            }
            // 
    // Use same default setting both fo UCX and FrameBase H264
    // Start stream with id 0
            UVCX_SetH264StartMode(UVCX_GetH264StartConfig(0));
            #if H264_SIMULCAST_EN
            if( /*UVCX_IsSimulcastH264()*/SIM_H264) {
                UVCX_SetH264StartMode(UVCX_GetH264StartConfig(1));
            }    
            #endif
            
            if(pCamCtx.videoRes==VIDRECD_RESOL_1920x1080) {
                if(gbCurH264Type==FRAMEBASE_H264) {
                    MMPD_H264ENC_SetIMENum(3); 
                } else {
                    MMPD_H264ENC_SetIMENum(3);    
                }
            } else {
                MMPD_H264ENC_SetIMENum(15);  
            }
        
        }
#endif
        dbg_printf(3,"Config H264 Enigne\r\n");

    // Stop preview in case current is in preview.
        if(UVC_VCD()==bcdVCD10) {    
            extern MMP_BOOL     m_bVidRecdPreviewStatus[] ;
            MMP_UBYTE ep = usb_get_cur_image_ep(pipe_id);
            if (m_bVidRecdPreviewStatus[ep] ^ MMP_FALSE) {
                MMPF_Fctl_EnablePreview((MMPF_IBC_PIPEID)pipe_id, MMP_FALSE, MMP_FALSE);
                if( (VIDEO_EP_END()==1) ||(m_bVidRecdPreviewStatus[0]==MMP_FALSE)&&(gbChangeH264Res==0)) {
                #if SUPPORT_GRA_ZOOM==0
                    dbg_printf(3,"#reset vi module & mci\r\n");
                    MMPD_System_TuneMCIPriority(1);
                #endif    
                    MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE_VI, MMP_FALSE);
                    MMPF_VIDENC_SoftStop();
                    
                } else {
                    dbg_printf(3,"#keep vi module & mci\r\n");
                }
                m_bVidRecdPreviewStatus[ep] = MMP_FALSE;
            }
        }
        
        // Call MMPS_3GPRECD_SetMemoryMapH264()
        if(1) { 
            MMP_ULONG   stackaddr;
            MMP_LONG    freesize ;
            MMPF_VIDENC_FRAME_LIST  InputFrameList[2];
            // Allocate memory
            #if SUPPORT_GRA_ZOOM
                MMPF_VIDENC_CURBUF_MODE_CTL CurCtl = {MMPF_VIDENC_CURBUF_RT, MMPF_VIDENC_BUFCFG_NONE,MMPF_VIDENC_BUFCFG_SRAM,PIPE_1};    
                if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                    CurCtl.InitCurBufMode = MMPF_VIDENC_CURBUF_FRAME ;
                    CurCtl.CurBufFrameConfig = MMPF_VIDENC_BUFCFG_DRAM ;
                    CurCtl.CurBufRTConfig =MMPF_VIDENC_BUFCFG_NONE ;
                } 
            #else
            MMPF_VIDENC_CURBUF_MODE_CTL CurCtl = {MMPF_VIDENC_CURBUF_FRAME, MMPF_VIDENC_BUFCFG_DRAM,MMPF_VIDENC_BUFCFG_NONE,PIPE_1};    
            #endif
            CurCtl.ubRTSrcPipe = pipe_id ;
            MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&CurCtl);
            MMPF_VIDENC_SetParameter(1, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&CurCtl);
            if(SIM_H264) {
                MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE, (void*)MMPF_VIDENC_RTFCTL_ACTIVE);
                MMPF_VIDENC_SetParameter(1, MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE, (void*)MMPF_VIDENC_RTFCTL_ACTIVE);
            }
            else {
                MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE, (void*)MMPF_VIDENC_RTFCTL_PASSIVE);
                MMPF_VIDENC_SetParameter(1, MMPF_VIDENC_ATTRIBUTE_RTFCTL_MODE, (void*)MMPF_VIDENC_RTFCTL_PASSIVE);
            }
            stackaddr = ((MMP_ULONG) MMPF_SYS_GetCurFBAddr(&freesize,pcam_info->pCamEpId) + 1023 )&(-1024);
            //RTNA_DBG_Str3("  == Set H264 memory map begin ==\r\n");
            dbg_printf(0,"  == Set H264 memory map begin (%s) mode ==\r\n",CurCtl.InitCurBufMode==MMPF_VIDENC_CURBUF_FRAME?"FRAME":"RT");
            RTNA_DBG_Str3("----H264 FB.Start(SRAM) :");RTNA_DBG_Long3(end_h264_sram_addr);RTNA_DBG_Str3("\r\n");
            RTNA_DBG_Str3("----H264 FB.Start(DRAM) :");RTNA_DBG_Long3(stackaddr);RTNA_DBG_Str3("\r\n");

            //MMPS_3GPRECD_SetResolution(usEncWidth, usEncHeight, 1);
            if(pipe_id!=PIPE_2) {
                MMPS_3GPRECD_SetMemoryMapH264(&end_h264_sram_addr, &stackaddr, InputFrameList, H264E_RINGBUF_SIZE);
                dbg_printf(3, "===> usEncHeight[0] = %d\r\n", usEncHeight[0]);
                #if SENSOR_IN_H > SENSOR_16_9_H
                MMPS_3GPRECD_SetPreviewConfig(gsSensorMCModeWidth, scaler_in_h,usEncWidth[0], usEncHeight[0], 
                InputFrameList, (MMPF_FCTL_LINK *)usb_uvc_get_preview_fctl_link(pipe_id),GetZoomResolution(pipe_id,usEncWidth[0], usEncHeight[0],user_ratio ),0);
                #else
                MMPS_3GPRECD_SetPreviewConfig(gsSensorMCModeWidth, gsSensorMCModeHeight,usEncWidth[0], usEncHeight[0], 
                InputFrameList, (MMPF_FCTL_LINK *)usb_uvc_get_preview_fctl_link(pipe_id),GetZoomResolution(pipe_id,usEncWidth[0], usEncHeight[0],0 ),0);
                #endif
                
                if( /*UVCX_IsSimulcastH264()*/SIM_H264) {
                    MMP_ULONG encid;
                    MMP_UBYTE _2nd_h264_pipe = PIPE_0,i; 
                    if(UVCX_GetH264EncodeBufferMode()!=FRAME_MODE) {
                        _2nd_h264_pipe = PIPE_1;
                    }
                    #if SENSOR_IN_H > SENSOR_16_9_H
                    MMPS_3GPRECD_SetPreviewConfig(gsSensorMCModeWidth, scaler_in_h,usEncWidth[1], usEncHeight[1], 
                    InputFrameList, (MMPF_FCTL_LINK *)usb_uvc_get_preview_fctl_link(_2nd_h264_pipe),GetZoomResolution(pipe_id,usEncWidth[1], usEncHeight[1],user_ratio ),1);
                    #else
                    MMPS_3GPRECD_SetPreviewConfig(gsSensorMCModeWidth, gsSensorMCModeHeight,usEncWidth[1], usEncHeight[1], 
                    InputFrameList, (MMPF_FCTL_LINK *)usb_uvc_get_preview_fctl_link(_2nd_h264_pipe),GetZoomResolution(pipe_id,usEncWidth[1], usEncHeight[1],0 ),1);
                    #endif
                    gGRAStreamingCfg.gra_loop_stop = MMP_FALSE ;
                    gGRAStreamingCfg.gra_loop_state = GRA_LOOP_RUN ;
                    gGRAStreamingCfg.work_streaming = GRA_STREAMING0 ;
                    #if STREAMING_START_PIPE==0
                    gGRAStreamingCfg.streaming_num = MMPF_VIDENC_GetTotalPipeNum()+((glGRAPreviewFormat==0xFF)?0:1);
                    #else
                    gGRAStreamingCfg.streaming_num = 2 ;
                    #endif
                	gGRAStreamingCfg.frm_buf_format[GRA_STREAMING0] = PIPE_CFG_H264; //0:YUV420 for H264
                	gGRAStreamingCfg.frm_buf_w[GRA_STREAMING0] = usEncWidth[0] ;	
                	gGRAStreamingCfg.frm_buf_h[GRA_STREAMING0] = usEncHeight[0];
 
 
                 	gGRAStreamingCfg.frm_buf_format[GRA_STREAMING1] = PIPE_CFG_H264; //0:YUV420 for H264
                	gGRAStreamingCfg.frm_buf_w[GRA_STREAMING1] = usEncWidth[1] ;	
                	gGRAStreamingCfg.frm_buf_h[GRA_STREAMING1] = usEncHeight[1];
                    if (glGRAPreviewFormat != 0xFF) {
                        gGRAStreamingCfg.frm_buf_format[GRA_STREAMING2] = glGRAPreviewFormat;
                        gGRAStreamingCfg.frm_buf_w[GRA_STREAMING2] = glGRAPreviewWidth;
                        gGRAStreamingCfg.frm_buf_h[GRA_STREAMING2] = glGRAPreviewHeight;
                    }
               	    // link slot buffer pipe to enc id
                    MMPF_VIDMGR_SetOutputPipe(1,pipe_id );
                }
                else {
                    gGRAStreamingCfg.gra_loop_stop = MMP_FALSE ;
                    gGRAStreamingCfg.gra_loop_state = GRA_LOOP_STOP ;
                    gGRAStreamingCfg.streaming_num = 0;
                }
                // link slot buffer pipe to enc id
                MMPF_VIDMGR_SetOutputPipe(0,pipe_id );
            }
            else {
                dbg_printf(3,"PIPE 2 not support H264\r\n");
            }
            //MMPD_System_TuneMCIPriority(2);

            MMPF_SYS_SetCurFBAddr((char*)stackaddr,pcam_info->pCamEpId);

            RTNA_DBG_Str3("----H264 FB.End(SRAM) :");RTNA_DBG_Long3(end_h264_sram_addr);RTNA_DBG_Str3("\r\n");
            RTNA_DBG_Str3("----H264 FB.End(DRAM) :");RTNA_DBG_Long3(stackaddr);RTNA_DBG_Str3("\r\n");
            
        }
        
        if(allocate_jpeg_after_h264) {
            MMP_ULONG freesize ,curaddr ;
            curaddr = (MMP_ULONG)MMPF_SYS_GetCurFBAddr((MMP_LONG *)&freesize,pcam_info->pCamEpId) ;
            glPCCAM_VIDEO_BUF_MAX_SIZE = ( freesize / jpeg_slot ) - 1024 ;
            glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
            dbg_printf(3,"#JPEG bufsize : %d KB\r\n",glPCCAM_VIDEO_BUF_MAX_SIZE >> 10);
            glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("JPEG",jpeg_slot*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
            cur_bctl.rd_index = cur_bctl.wr_index = 0;
            cur_bctl.total_rd = cur_bctl.total_wr = 0;
            cur_bctl.slot_num = jpeg_slot  ; // 3 ring buffer 
            cur_bctl.frame_num = jpeg_slot ;
            cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
            cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
            cur_stream->pipe_b[PIPE_0] = cur_bctl ;
            // For JPEG 
            MMPF_SetPCCAMCompressBuf(glPCCAM_VIDEO_BUF_ADDR,(glPCCAM_VIDEO_BUF_MAX_SIZE-aligned_header_size));
         }
    }
    //#if (CHIP==P_V2)
    else if( PCAM2MMP_Get_VideoFormat2(ep_fmt) ==MMPS_3GPRECD_VIDEO_FORMAT_RAW){
        RTNA_DBG_Str(1,"Start RAW setting...\r\n");
        // pGBL->GBL_CLK_EN_2 |= GBL_CLK_RAWPROC ;
        // pGBL->GBL_CLK_EN |= GBL_CLK_DMA;
        glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("RAW",2 * glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
        MMPF_SetPCCAMCompressBuf(cur_buf,(glPCCAM_VIDEO_BUF_MAX_SIZE - aligned_header_size));
        cur_bctl.rd_index = cur_bctl.wr_index = 0;
        cur_bctl.total_rd = cur_bctl.total_wr = 0;
        cur_bctl.slot_num = 2 ; // 3 ring buffer 
        cur_bctl.frame_num = 2 ;
        cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
        cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
        cur_stream->pipe_b[PIPE_0] = cur_bctl ;
        // cur_stream->pipe1_b = cur_bctl ;
        MMPF_RAWPROC_Reset();
        MMPF_RAWPROC_SetRawBuffer(0,glPCCAM_VIDEO_BUF_ADDR + aligned_header_size);
        MMPF_RAWPROC_SetRawBuffer(1,glPCCAM_VIDEO_BUF_ADDR +  glPCCAM_VIDEO_BUF_MAX_SIZE + aligned_header_size);

        //MMPF_RAWPROC_SetRawBuffer(2,glPCCAM_VIDEO_BUF_ADDR +  glPCCAM_VIDEO_BUF_MAX_SIZE*2 + FRAME_PAYLOAD_HEADER_SZ);
        MMPF_RAWPROC_SetCaptureStoreAddr();   
        MMPF_RAWPROC_EnableRawPath(MMP_TRUE);
    }
  //#endif
    else {
    
        MMP_USHORT slot = 3; //
        #if USB_LYNC_TEST_MODE==0
        if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV422) {
            if ( (ep_res==PCCAM_2304_1536) || (ep_res==PCCAM_2304_1296) ) {
                slot = 2 ;
            }
        }
        #else
        
        #endif
        if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_MJPEG) {
            if(gbCurH264Type==UVC_YUY2) {
                slot =2;
            } else {
                slot =3 ;
            }
        } else {
            slot =3 ;
            if ( (ep_res==PCCAM_2304_1536) || (ep_res==PCCAM_2304_1296) ) {
                slot = 2 ;
            }
        }
        
        glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("PCAM",slot * glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
        // Set 1st MJPEG compression address
        // For JPEG 
        if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_MJPEG) {
            MMPF_SetPCCAMCompressBuf(glPCCAM_VIDEO_BUF_ADDR,(glPCCAM_VIDEO_BUF_MAX_SIZE-aligned_header_size));    
        }
        cur_bctl.rd_index = cur_bctl.wr_index = 0;
        cur_bctl.total_rd = cur_bctl.total_wr = 0;
        cur_bctl.slot_num = slot ; // 3 ring buffer 
        cur_bctl.frame_num = slot ;
        cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
        cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
        cur_stream->pipe_b[PIPE_0] = cur_bctl ;
        cur_stream->pipe_b[PIPE_1] = cur_bctl ;
        cur_stream->pipe_b[PIPE_2] = cur_bctl ;
        
    //dbg_printf(3,"BUF_SIZE = %d\r\n", glPCCAM_VIDEO_BUF_MAX_SIZE);
        dbg_printf(3, "BUFF: (pipe0, pipe1) = (0x%x, 0x%x)\r\n", cur_stream->pipe_b[PIPE_0].buf_addr, cur_stream->pipe_b[PIPE_1].buf_addr);        
      //  dbg_printf(0,"#cur slot #:%d\r\n",cur_bctl.slot_num);
        if(gbCurH264Type==UVC_YUY2) {

            //MMP_ULONG yuy2_size = GetYUY2FrameSize(gsVidRes) + aligned_header_size  ;
            MMP_ULONG yuy2_size = cur_stream->pipe_w[PIPE_0] * cur_stream->pipe_h[PIPE_0] * 2 + aligned_header_size  ;
            MMP_ULONG yuy2_addr ;
            //VAR_W(0,glPccamResolution);
        //    dbg_printf(1,"#YUY2 Res : %d\r\n",glPccamResolution);
            
            yuy2_size = (yuy2_size + 31 ) & (-32) ;
            yuy2_addr = (MMP_ULONG)MMPF_SYS_AllocFB("YUY2",2*yuy2_size,32,pcam_info->pCamEpId);
            cur_bctl.rd_index = cur_bctl.wr_index = 0;
            cur_bctl.total_rd = cur_bctl.total_wr = 0;
            cur_bctl.slot_num = 2  ; // 3 ring buffer 
            cur_bctl.slot_size =  yuy2_size ; 
            cur_bctl.buf_addr =  yuy2_addr ;     
            cur_stream->pipe_b[PIPE_0] = cur_bctl ;
        
        }  


        #if H264_SIMULCAST_EN==1
        if(UVC_VCD()==bcdVCD15) {
            glGRAPreviewFormat = (ep_fmt ==PCAM_USB_VIDEO_FORMAT_MJPEG) ? PIPE_CFG_MJPEG : PIPE_CFG_YUY2; // YUY2
            glGRAPreviewWidth  = cur_stream->pipe_w[PIPE_0] ;
            glGRAPreviewHeight = cur_stream->pipe_h[PIPE_0] ;
            if(SIM_H264) {
                gGRAStreamingCfg.frm_buf_format[GRA_STREAMING2] = glGRAPreviewFormat;
                gGRAStreamingCfg.frm_buf_w[GRA_STREAMING2] = glGRAPreviewWidth;
                gGRAStreamingCfg.frm_buf_h[GRA_STREAMING2] = glGRAPreviewHeight;
                // Don't reset frm_buf_index and streaming_num here
                //gGRAStreamingCfg.frm_buf_index[GRA_STREAMING2] = 0; // TBD
            }
        }
        #endif
        
      
    }
    

    #if OMRON_FDTC_SUPPORT
    {
        STREAM_CFG *fdtc_cfg = usb_get_cur_fdtc_pipe() ;
        MMP_ULONG  size ,addr ;
        
        size =( fdtc_cfg->pipe_w[PIPE_2] * fdtc_cfg->pipe_h[PIPE_2] * 3 ) / 2;
        addr = (MMP_ULONG)MMPF_SYS_AllocFB("FDTC_F",2 * size,32,pcam_info->pCamEpId);
        cur_bctl.rd_index = cur_bctl.wr_index = 0;
        cur_bctl.total_rd = cur_bctl.total_wr = 0;
        cur_bctl.slot_num = 2 ; 
        cur_bctl.frame_num = 2 ;
        cur_bctl.slot_size =  size ; 
        cur_bctl.buf_addr =  addr  ; 
        cur_bctl.off_header = aligned_off;    
        fdtc_cfg->pipe_b[PIPE_2] = cur_bctl ;
                 
      if(gbEnableDoFDTC){
        glFDTC_CurrentBuffer = FRAME_POOL ;//(MMP_ULONG)MMPF_SYS_AllocFB("FDTC_Y", 1024*1024, 32);
        MMPF_MMU_ConfigWorkingBuffer(glFDTC_CurrentBuffer, glFDTC_CurrentBuffer+640*480, 1);
        initFDTCMemory();
        dbg_printf(3,"FDTC_Y : %x\r\n",glFDTC_CurrentBuffer);
      }
  }
  #elif SUPPORT_GRA_ZOOM
    if( PIPE_EN(cur_stream->pipe_en) & PIPE2_EN) {
        MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(PIPE_2,cur_stream->pipe_cfg) ;
        if(pipe_cfg ==PIPE_CFG_YUY2) {
        // YUY2
          glPCCAM_VIDEO_BUF_MAX_SIZE = GetYUY2FrameSize(ep_res) + aligned_header_size  ;
            glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
            glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("YUY2_PIPE2",2*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
            cur_bctl.rd_index = cur_bctl.wr_index = 0;
            cur_bctl.total_rd = cur_bctl.total_wr = 0;
            cur_bctl.slot_num = 2  ; // 3 ring buffer 
            cur_bctl.frame_num = 2 ;
            cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
            cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
            cur_stream->pipe_b[PIPE_2] = cur_bctl ;
            dbg_printf(3, "max_size=%d, buf_addr=x%x\r\n", glPCCAM_VIDEO_BUF_MAX_SIZE, glPCCAM_VIDEO_BUF_ADDR);    
        }  
        else if(pipe_cfg ==PIPE_CFG_MJPEG) {
        // MJPEG, always 320x240 for test
            if (1/*glPccamResolution==PCCAM_320_240*/) {
                RTNA_DBG_Str3("FrameBaseH264, JPEG is 320x240\r\n");
                glPCCAM_VIDEO_BUF_MAX_SIZE = 320 * 240 + aligned_header_size ;
                glPCCAM_VIDEO_BUF_MAX_SIZE = (glPCCAM_VIDEO_BUF_MAX_SIZE + 31 ) & (-32) ;
                glPCCAM_VIDEO_BUF_ADDR = (MMP_ULONG)MMPF_SYS_AllocFB("JPEG_PIPE2",3*glPCCAM_VIDEO_BUF_MAX_SIZE,32,pcam_info->pCamEpId);
                cur_bctl.rd_index = cur_bctl.wr_index = 0;
                cur_bctl.total_rd = cur_bctl.total_wr = 0;
                cur_bctl.slot_num = 3  ; // 3 ring buffer 
                cur_bctl.frame_num = 3 ;
                cur_bctl.slot_size =  glPCCAM_VIDEO_BUF_MAX_SIZE ; 
                cur_bctl.buf_addr =  glPCCAM_VIDEO_BUF_ADDR ;     
                cur_stream->pipe_b[PIPE_2] = cur_bctl ;
                // For JPEG 
                MMPF_SetPCCAMCompressBuf(glPCCAM_VIDEO_BUF_ADDR,(glPCCAM_VIDEO_BUF_MAX_SIZE-aligned_header_size    ));
            }                
        }   
        else if( pipe_cfg==PIPE_CFG_NV12) {
#define NV12_FIXED_ADDR (1)
        
            if(/*IS_FIRST_STREAM(workss)*/!gbIsGraSrcBufAllocated) {
            //if(1) {
                MMP_USHORT slot = 2 ;
                MMP_ULONG size,addr;
                MMP_LONG pstart,pend;
                size =( cur_stream->pipe_w[GRA_SRC_PIPE] * ALIGN16(cur_stream->pipe_h[GRA_SRC_PIPE] )* 3 ) / 2 + aligned_header_size;
	            //size =( 1920 * 1088 * 3 ) / 2 + aligned_header_size;
	            
	            // Fixed bug for out of memory 
                if(ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV422) {
                    MMP_ULONG size1 = GetYUY2FrameSize(ep_res) ;
                    if(size1 >= GetYUY2FrameSize(PCCAM_1920_1080)) {
                        slot = 1;
                    }
                
                }
                
                #if NV12_FIXED_ADDR==1
                MMPF_SYS_GetFBAddrArea(&pstart,&pend,0);
                size = (size + 255) & (-256) ;
                addr = pend - slot * size ;
                dbg_printf(3,"@[0][GRA_NV12](Addr,Size) : (%x,%x)\r\n",addr,size); 
                #else
                addr = (MMP_ULONG)MMPF_SYS_AllocFB("GRA_NV12",slot * size,32, 0/*pcam_info->pCamEpId*/);
                #endif
                
                //glPCCAM_VIDEO_BUF_ADDR = addr;
                cur_bctl.rd_index = cur_bctl.wr_index = 0;
                cur_bctl.total_rd = cur_bctl.total_wr = 0;
                cur_bctl.slot_num = slot ; 
                cur_bctl.frame_num = slot ;
                cur_bctl.slot_size =  size ; 
                cur_bctl.buf_addr =  addr  ; 
                cur_bctl.off_header = aligned_off;    
                cur_stream->pipe_b[GRA_SRC_PIPE] = cur_bctl ;
                // Assign to a global data
                gsNV12StreamCtl = cur_bctl ;
                gbIsGraSrcBufAllocated = MMP_TRUE ;  
            } else {
                /*
                STREAM_CFG *cur_pipe ;
                if(gbChangeH264Res) {
                    cur_pipe = usb_get_cur_image_pipe_by_epid(pcam_info->pCamEpId);
                }
                else {
                    cur_pipe = usb_get_cur_image_pipe_by_epid((pcam_info->pCamEpId==1)?0:1);
                }
                */
                if( (ep_fmt ==PCAM_USB_VIDEO_FORMAT_YUV422) &&( UVC_VCD()==bcdVCD15 ) ) {
                    MMP_ULONG size1 = GetYUY2FrameSize(ep_res) ;
                    if(size1 >= GetYUY2FrameSize(PCCAM_1920_1080)) {
                        STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
                        if(gsNV12StreamCtl.slot_num==2) {
                            gsNV12StreamCtl.buf_addr += gsNV12StreamCtl.slot_size ;
                            gsNV12StreamCtl.slot_num = 1;
                            gsNV12StreamCtl.frame_num = 1;
                            gsNV12StreamCtl.rd_index = gsNV12StreamCtl.wr_index = 0 ;
                            gsNV12StreamCtl.total_rd = gsNV12StreamCtl.total_wr = 0 ;
                            if(cur_pipe) {
                                cur_pipe->pipe_b[GRA_SRC_PIPE] = gsNV12StreamCtl ; 
                            }
                        }
                    }
                }
                cur_stream->pipe_b[GRA_SRC_PIPE] = gsNV12StreamCtl;// cur_pipe->pipe_b[GRA_SRC_PIPE];
            }        
        }
   }
  
  #endif
    
  //MMPD_System_TuneMCIPriority(2);
    MMPF_Video_Init_Buffer(pcam_info->pCamEpId,aligned_off);
    if(0) { // DEBUG ONLY
        MMP_LONG free ;
        MMPF_SYS_GetCurFBAddr(&free,0);
        MMPF_SYS_GetCurFBAddr(&free,1);
    }
    return PCAM_ERROR_NONE ;
}

//extern PCAM_USB_VIDEO_FORMAT gsVidFmt ;
MMP_USHORT PCAM_USB_PreviewStop(pcam_msg_t *msg)
{
    extern MMP_BOOL m_bVidRecdPreviewStatus[];
    extern MMP_UBYTE gbChangeH264Res ;
    extern MMP_USHORT	gsPreviewPath;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    #if 0
    MMP_UBYTE   ep = usb_get_cur_image_ep(PIPE_1); // TBD
    #else
    MMP_UBYTE   ep = STREAM_EP_H264 ;
    #endif
    STREAM_CFG  *cur_pipe = usb_get_cur_image_pipe_by_epid(ep);
    PIPE_PATH  pipe = usb_get_cur_image_pipe_id(cur_pipe,PIPE_CFG_H264) ;
    
    MMP_UBYTE grapath = MMPF_Video_IsLoop2GRA(cur_pipe) ;
    
    // sean@2010_09_13, move to here !
    // Only H264 call this function
    if(1/*gsVidFmt == PCAM_USB_VIDEO_FORMAT_H264*/){ 
        MMPS_3GPRECD_StopRecord(); // < 1 frame time
       // RTNA_DBG_Str3("MMPS_3GPRECD_StopRecord\r\n");
    }
    dbg_printf(3, "USB_VideoPreviewStop(),ep:%d,pipe:%d\r\n",ep,pipe);
    if (m_bVidRecdPreviewStatus[ep] ^ MMP_FALSE) {
        // For dynamic res change
        //MMPF_Fctl_EnablePreview(MMPF_IBC_PIPE_1, MMP_FALSE, MMP_FALSE);
        if( (VIDEO_EP_END()==1) ||(m_bVidRecdPreviewStatus[0]==MMP_FALSE)&&(gbChangeH264Res==0)) {
            // For dynamic res change,moved here, 2 frame time
            MMPF_Fctl_EnablePreview(pipe, MMP_FALSE, MMP_FALSE);
            #if SUPPORT_GRA_ZOOM==0
            dbg_printf(3,"#reset vi module & mci\r\n");
            MMPD_System_TuneMCIPriority(1);
            MMPD_System_ResetHModule(MMPD_SYSTEM_HMODULETYPE_VI, MMP_FALSE);
            #endif
            MMPF_VIDENC_SoftStop();
        } 
        else if (gbChangeH264Res) {
            // prevent H264 rt-mode double frame start
            MMPF_Fctl_EnablePreview(pipe, MMP_FALSE, MMP_FALSE);
            MMPF_VIDENC_SoftStop();
            MMPF_Scaler_SwitchPath(MMP_TRUE,pipe,MMPF_SCALER_SOURCE_GRA,MMP_TRUE); 
           //dbg_printf(0,"<H264SCAL->GRA>\r\n");  
        }
        else {
            #if 1 // sean@2013_02_05
            MMPF_Fctl_EnablePreview(pipe, MMP_FALSE, MMP_FALSE);
            MMPF_VIDENC_SoftStop();
            #endif
            dbg_printf(3,"#keep vi module & mci\r\n");
        }
        m_bVidRecdPreviewStatus[ep] = MMP_FALSE;
    }
    #if SUPPORT_GRA_ZOOM==0
    if( (m_bVidRecdPreviewStatus[0]==MMP_FALSE) && (gbChangeH264Res==0)) {
        dbg_printf(3,"#disable vif/isp/ibc\r\n");
// Disable VIF/ISP/IBC in stop preview.
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIF);
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ISP);
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_IBC);   
    }
    #else
    if( ((m_bVidRecdPreviewStatus[0]==MMP_FALSE) && (gbChangeH264Res==0)) && (grapath==0)) {
        dbg_printf(3,"#disable vif/isp/ibc\r\n");
// Disable VIF/ISP/IBC in stop preview.
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIF);
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ISP);
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_IBC);   
    }
    
    #endif
    
    return PCAM_ERROR_NONE ;
}

MMP_USHORT PCAM_USB_UpdateOSD(pcam_msg_t *msg)
{
   
    return PCAM_ERROR_NONE ;
}

MMP_USHORT PCAM_USB_Exit(pcam_msg_t *msg)
{
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    PCAM_USB_SENSOR_OFF_MODE off_mode = *(PCAM_USB_SENSOR_OFF_MODE *)msg->msg_data; 
// Power down sensor & AIT Hardware module.
    if(gbDevicePowerSavingStatus == 1) {
        //RTNA_DBG_Str(3,"#-0-#\r\n");
        return PCAM_ERROR_NONE;
    }
    MMPD_Sensor_PowerDown(gsCurrentSensor,(MMP_UBYTE)off_mode);
#if 1   
  #if 1
      pGBL->GBL_RST_CTL01 |= (GBL_JPG_RST |
                  GBL_ICON_IBC_RST |
                  #if (CHIP == VSN_V2)
                  GBL_SCAL_RST |
                  #endif
                  #if (CHIP == VSN_V3)
                  GBL_SCAL0_RST |
                  #endif
                  GBL_VI_RST |
                  GBL_DMA_RST |
                  GBL_H264_RST |
                  GBL_ISP_RST | GBL_GRA_RST); // sean@for no isp frame end test
      #if(CHIP == VSN_V3)
      pGBL->GBL_RST_CTL3 |=  (GBL_SCAL1_RST | 
                  GBL_SCAL2_RST);
      #endif //#if(CHIP == VSN_V3)
        RTNA_WAIT_CYCLE(100);
        pGBL->GBL_RST_CTL01 &= ~(GBL_JPG_RST |
                  GBL_ICON_IBC_RST |
                  #if (CHIP == VSN_V2)
                  GBL_SCAL_RST |
                  #endif
                  #if (CHIP == VSN_V3)
                  GBL_SCAL0_RST |
                  #endif
                  GBL_VI_RST |
                  GBL_DMA_RST |
                  GBL_H264_RST |
                  GBL_ISP_RST | GBL_GRA_RST); // sean@for no isp frame end test
      #if(CHIP == VSN_V3)
      pGBL->GBL_RST_CTL3 &=  ~(GBL_SCAL1_RST | 
                  GBL_SCAL2_RST);
      #endif //#if(CHIP == VSN_V3)
    
    #endif
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_RAWPROC, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_H264, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_FALSE);
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
#endif    

    gbDevicePowerSavingStatus = 1; 
    return PCAM_ERROR_NONE ;
}

// Log the latest pan information.        
extern MMP_USHORT gsPanEnd,gsTiltEnd ;
MMP_USHORT expLines;

MMP_USHORT PCAM_USB_SetAttrs(pcam_msg_t *msg)
{
extern MMP_USHORT gsCurFrameRatex10[];
    MMP_BOOL IsPreviewEnable;
    MMP_USHORT err = PCAM_ERROR_NONE ;
    PCAM_USB_INFO pCamInfo = *(PCAM_USB_INFO *)msg->msg_data; 
#if 0
    RTNA_DBG_Str(3,"PCAM_USB_SetAttrs:");
    RTNA_DBG_Short(3,msg->msg_sub_id);
    RTNA_DBG_Str(3,"\r\n");
#endif
    
    switch(msg->msg_sub_id){
    case PCAM_USB_SETTING_VIDEO_RES:
        pCamCtx.videoRes = PCAM2MMP_Get_Resolution(&pCamInfo) ;
        break;
    case PCAM_USB_SETTING_H264_RES:// set uvc h264 resolution
        pCamCtx.videoRes = PCAM2MMP_Get_Resolution(&pCamInfo) ;
        pCamCtx.videoFormat = PCAM2MMP_Get_VideoFormat(&pCamInfo) ;
        m_VideoFmt = pCamCtx.videoFormat ; 
        break ;
    case PCAM_USB_SETTING_VIDEO_FORMAT:
        pCamCtx.videoFormat = PCAM2MMP_Get_VideoFormat(&pCamInfo);
        m_VideoFmt = pCamCtx.videoFormat ; 
        break ;
    case PCAM_USB_SETTING_VIDEO_QUALITY:
        pCamCtx.videoQuality = PCAM2MMP_Get_Quality(&pCamInfo);
        m_usVideoQuality = pCamCtx.videoQuality ;
        break;
    case PCAM_USB_SETTING_DEBAND:
        pCamCtx.debandMode = PCAM2MMP_Get_DebandMode(&pCamInfo);
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) { 
            if(pCamCtx.debandMode==SENSOR_DSC_DEBAND_60HZ) {
                ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_60HZ);
            } else if(pCamCtx.debandMode==SENSOR_DSC_DEBAND_50HZ) {
                ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_50HZ);
            } else {
                ISP_IF_AE_SetFlicker(ISP_AE_FLICKER_AUTO); //test for AUTO 
            }
        }
        break ;
            
    case PCAM_USB_SETTING_AUDIO_FORMAT:
        pCamCtx.audioFormat = PCAM2MMP_Get_AudioFormat(&pCamInfo);
        break;
// sean@2011_01_24 , add exposure time
    case PCAM_USB_SETTING_EXPOSURE_TIME:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
#if 0
            //MMP_ULONG expLines;
            // pCamInfo.pCamExpTime is range from 5~2047, means 1/2048s ~ 1/2s
            MMP_USHORT expLines = PCAM2MMP_Normalize_Shutter(pCamInfo.pCamExpTime) ;
            
            //dbg_printf(1,"#Exposure Lines:%d\r\n",expLines );
      ISP_IF_AE_SetShutter(expLines, ISP_IF_AE_GetShutterBase());  
#endif
	  // pCamInfo.pCamExpTime is range from 5~2047, means 1/2048s ~ 1/2s
	  //MMP_USHORT expLines = PCAM2MMP_Normalize_Shutter(pCamInfo.pCamExpTime) ;
	  
	  //dbg_printf(1,"#Exposure Lines:%d pCamExpTime:%d \r\n",expLines, pCamInfo.pCamExpTime );
	  //ISP_IF_AE_SetShutter(expLines, ISP_IF_AE_GetShutterBase()); 
	  
	  dbg_printf(1,"pCamInfo.pCamExpTime:%d \r\n",pCamInfo.pCamExpTime );
	  
	  //normalized_value = PCAM2MMP_Normalize_Shutter(pCamInfo.pCamExpTime) ; //maping 5~16383(-11~+1) to 0~3000
	  
	  switch (pCamInfo.pCamExpTime){
		  case 5: //-11
			  if(gbBandFilter == BAND_50HZ) expLines = 120;
			  else if(gbBandFilter == BAND_60HZ) expLines = 100;
			  else expLines = 120;
				  
			  break ;
		  case 10: //-10
			  if(gbBandFilter == BAND_50HZ) expLines = 240;
			  else if(gbBandFilter == BAND_60HZ) expLines = 200;
			  else expLines = 240;
		  
			  break ;
		  case 20: //-9
			  if(gbBandFilter == BAND_50HZ) expLines = 360;
			  else if(gbBandFilter == BAND_60HZ) expLines = 400;
			  else expLines = 360;
			  
			  break ;
		  case 39:
			  if(gbBandFilter == BAND_50HZ) expLines = 480;
			  else if(gbBandFilter == BAND_60HZ) expLines = 500;
			  else expLines = 480;  
			  
			  break ;
		  case 78:
			  if(gbBandFilter == BAND_50HZ) expLines = 600;
			  else if(gbBandFilter == BAND_60HZ) expLines = 600;
			  else expLines = 600;
			  
			  break ;
		  case 156:
			  if(gbBandFilter == BAND_50HZ) expLines = 720;
			  else if(gbBandFilter == BAND_60HZ) expLines = 800;
			  else expLines = 720;
			  
			  break ;
		  case 312:
			  if(gbBandFilter == BAND_50HZ) expLines = 960;
			  else if(gbBandFilter == BAND_60HZ) expLines = 1000;
			  else expLines = 960;
			  
			  break ;		  
		  case 625:
			  if(gbBandFilter == BAND_50HZ) expLines = 1200;
			  else if(gbBandFilter == BAND_60HZ) expLines = 1200;
			  else expLines = 1200;
		  
			  break ;
		  case 1250:
			  if(gbBandFilter == BAND_50HZ) expLines = 1440;
			  else if(gbBandFilter == BAND_60HZ) expLines = 1400;
			  else expLines = 1440;	
			  
			  break ;	  
		  case 2500:
			  if(gbBandFilter == BAND_50HZ) expLines = 1560;
			  else if(gbBandFilter == BAND_60HZ) expLines = 1600;
			  else expLines = 1560;	
			  
			  break ;
		  case 5000:
			  if(gbBandFilter == BAND_50HZ) expLines = 1680;
			  else if(gbBandFilter == BAND_60HZ) expLines = 1800;
			  else expLines = 1680;	
			  
			  break ;
		  case 10000:
			  if(gbBandFilter == BAND_50HZ) expLines = 1800;
			  else if(gbBandFilter == BAND_60HZ) expLines = 2000;
			  else expLines = 1800;
	  
			  break ;
		  case 16383: //+1
			  if(gbBandFilter == BAND_50HZ) expLines = 1920;
			  else if(gbBandFilter == BAND_60HZ) expLines = 2200;
			  else expLines = 1920;
	  
			  break ;		  
	  }
	  
	  //dbg_printf(1,"#Exposure Lines:%d ISP_IF_AE_GetShutterBase:%d \r\n",expLines, ISP_IF_AE_GetShutterBase() );
	  ISP_IF_AE_SetShutter((expLines* ISP_IF_AE_GetShutterBase())/12000 , ISP_IF_AE_GetShutterBase()); 
	  //dbg_printf(1,"normalized_value:%d Set AE Shutter:%d ISP_IF_AE_GetShutterBase:%d \r\n",(normalized_value, expLines* ISP_IF_AE_GetShutterBase())/12000, ISP_IF_AE_GetShutterBase() );
	  
	  //dbg_printf(0,"expLines : %d ISP_IF_AE_GetShutterBase : %d \r\n",expLines,ISP_IF_AE_GetShutterBase());


      gsSensorFunction->MMPF_Sensor_SetSensorShutter(0, 0);
      RTNA_DBG_Str(0, "Get expo");       
      RTNA_DBG_Short(3, USB_GetExposureTime());

	  RTNA_DBG_Str(0, "\r\n"); 
	  dbg_printf(0,"expLines: %d gbBandFilter:%d \r\n", expLines, gbBandFilter);
	  
      RTNA_DBG_Str(0, "\r\n"); 
        }
        break ;
        
    case PCAM_USB_SETTING_SATURATION: // 0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(pCamInfo.pCamSaturation);
            //gsSensorFunction->MMPF_Sensor_SetSaturation(setvalue);
            //VAR_W(0,setvalue);
            ISP_IF_F_SetSaturation(setvalue);
        }
        break ;
    case PCAM_USB_SETTING_CONTRAST:// 0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(pCamInfo.pCamContrast) ;
            //gsSensorFunction->MMPF_Sensor_SetContrast(setvalue);
            ISP_IF_F_SetContrast(setvalue);
        }
        break ;
    case PCAM_USB_SETTING_BRIGHTNESS://0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
#if 1
            //MMP_USHORT setvalue = PCAM2MMP_Normalize_EV(pCamInfo.pCamBrightness) ;
            //ISP_IF_AE_SetEV(setvalue);
            MMP_USHORT setvalue = PCAM2MMP_Normalize_Value(pCamInfo.pCamBrightness) ;
            ISP_IF_F_SetBrightness(setvalue);
#else
// sean@2011_01_24
            ISP_IF_IQ_SetYC(pCamInfo.pCamBacklight * 0x20 + 0x40, pCamInfo.pCamBrightness);
#endif
        }
        break ;
    case PCAM_USB_SETTING_HUE: // 0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(pCamInfo.pCamHue) ;
           // gsSensorFunction->MMPF_Sensor_SetHue(setvalue);
           ISP_IF_F_SetHue(setvalue);
        }
        break ;
    case PCAM_USB_SETTING_GAMMA: // 0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(pCamInfo.pCamGamma);
            //gsSensorFunction->MMPF_Sensor_SetGamma(setvalue);
            ISP_IF_F_SetGamma(setvalue);
        }
        break ;
    case PCAM_USB_SETTING_BACKLIGHT:
         MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
 #if 1
           // 842ooxx  
           // gsSensorFunction->MMPF_Sensor_SetBacklight(pCamInfo.pCamBacklight);
           
            if(pCamInfo.pCamBacklight) {
                //ISP_IF_F_SetWDR(255);
            } else {
                //ISP_IF_F_SetWDR(0);
            }
           
           // Yout = Yin * backlight_ratio + brightness_offset, same as brightness
#else
// sean@2011_01_24
           ISP_IF_IQ_SetYC(pCamInfo.pCamBacklight * 0x20 + 0x40, pCamInfo.pCamBrightness);
#endif
        }
        break ;
    case PCAM_USB_SETTING_SHARPNESS: // 0 ~ 255 -> -100 ~ 100
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_SHORT setvalue =  PCAM2MMP_Normalize_Value(pCamInfo.pCamSharpness);
           // gsSensorFunction->MMPF_Sensor_SetSharpness(setvalue);
           ISP_IF_F_SetSharpness(setvalue);
       //    ISP_IF_F_SetCISharpness(setvalue);
        }
        break ;
    case PCAM_USB_SETTING_GAIN: // 0 ~ 255 -> GainBase ~ 8*GainBase
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            //MMP_SHORT setvalue =  PCAM2MMP_Normalize_Value(pCamInfo.pCamGain);
            //gsSensorFunction->MMPF_Sensor_SetSharpness(setvalue);
            // sean@2011_01_24 add
            MMP_USHORT setvalue = PCAM2MMP_Normalize_Gain(pCamInfo.pCamGain) ;
            //dbg_printf(1,"pCamVal:%d to Sensor Gain:%d",pCamInfo.pCamGain,setvalue);
            // ISP D-Gain return to 1X
            //20121023 kyle
            ISP_IF_AE_SetGain(setvalue, ISP_IF_AE_GetGainBase());
            gsSensorFunction->MMPF_Sensor_SetSensorGain(ISP_IF_AE_GetGain());
            //RTNA_DBG_Str(0, "Set gain2");       
            //RTNA_DBG_Short(3, setvalue);
            //RTNA_DBG_Str(0, "\r\n");             
        }
        break ;
            
    case PCAM_USB_SETTING_WB:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            //gsSensorFunction->MMPF_Sensor_SetAWBEnable(pCamInfo.pCamWB); 
            if(pCamInfo.pCamWB) {
                ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO); 
            } else {
                ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL); 

            } 
            //RTNA_DBG_Str(0, "WB");       
            //RTNA_DBG_Str(0, "\r\n");              
        }
                
        break;
    case PCAM_USB_SETTING_LENS:
       // VAR_W(0,pCamInfo.pCamLensPos);
        //gsSensorFunction->MMPF_Sensor_SetAFPosition(pCamInfo.pCamLensPos);
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
    #if SUPPORT_AUTO_FOCUS
        //    ISP_IF_MOTOR_GoToAFPos();    
            //SetMotorPos(pCamInfo.pCamLensPos);
            gsSensorFunction->MMPF_Sensor_SetAFPosition(pCamInfo.pCamLensPos);
        #endif
        }
        break ;
    case PCAM_USB_SETTING_AF:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable && pCamInfo.pCamEnableAF) {
            ISP_IF_AF_Control(ISP_AF_START);
        } else {
            ISP_IF_AF_Control(ISP_AF_STOP);
        }
        break;
    case PCAM_USB_SETTING_AE:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable ) {
            gsSensorFunction->MMPF_Sensor_SetAEEnable(pCamInfo.pCamEnableAE);
        }
        break;
    case PCAM_USB_SETTING_AE_PRIO:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable ) {
            dbg_printf(3,"FPS,1\r\n");        
            if(pCamInfo.pCamEnableAEPrio==1) {
               // PCAM_USB_Set_FPS(0) ; // Auto FPS
               PCAM_USB_Set_FPSx10( pCamInfo.pCamEnableAEPrio,0,-1);
            } else {
               // PCAM_USB_Set_FPS(-1); // restore original 
               PCAM_USB_Set_FPSx10( pCamInfo.pCamEnableAEPrio,0,-1);
            }
        }
        break;
        
        
    case PCAM_USB_SETTING_WB_TEMP:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            MMP_USHORT val;
            val = PCAM2MMP_Normalize_WB(pCamInfo.pCamWBTemp);
            ISP_IF_AWB_SetColorTemp(val);
            //RTNA_DBG_Str(0, "color temp.");       
            //RTNA_DBG_Short(3, pCamInfo.pCamWBTemp);
            //RTNA_DBG_Str(0, "\r\n");             
        }
        break;
    case PCAM_USB_SETTING_DIGZOOM:
#if SUPPORT_DIGITAL_ZOOM    
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            PCAM_USB_ZOOM  *zoomInfo = &pCamInfo.pCamDigZoom ;
            MMPF_SCALER_PATH path_sel = MMPF_SCALER_PATH_0 ;
            STREAM_CFG *cur_pipe , cur_pipe_tmp = *usb_get_cur_image_pipe_by_epid(pCamCtx.epId);
            MMP_BOOL bComplete ;
            AITPS_VIF   pVIF = AITC_BASE_VIF;
            MMP_UBYTE   grapath = 0;
            MMP_USHORT  err1 = 0,err2 = 0 ;
            
            cur_pipe = &cur_pipe_tmp ;
            cur_pipe->pipe_en &= ~PIPEEN(GRA_SRC_PIPE);
           
            #if SUPPORT_GRA_ZOOM
            grapath = MMPF_Video_IsLoop2GRA(cur_pipe) ;
            #endif   
            dbg_printf(0,"ZOOM :--- : %x\r\n",grapath);
            
            #if H264_SIMULCAST_EN
            if(UVCX_IsSimulcastH264()) {
                if(1/*UVCX_GetH264EncodeBufferMode()==FRAME_MODE*/) {
                    cur_pipe->pipe_en &= ~PIPE0_EN ;
                } else {
                    cur_pipe->pipe_en |= PIPE0_EN ;
                }
            }
            #endif
            
            if( !(grapath & (0x02 | 0x04) )) {          
                do {
                    MMPF_Scaler_CheckZoomComplete(path_sel, &bComplete);
                    MMPF_OS_Sleep(1);
                } while (!bComplete);
            }
            if(UVC_VCD()==bcdVCD15) { // Cause H264 hang up
                MMP_BOOL enable;
                MMPS_3GPRECD_GetPreviewDisplayByEpId(&enable,STREAM_EP_MJPEG) ;
                if( enable && (!UVCX_IsSimulcastH264() )) {
                    cur_pipe->pipe_en |= PIPE0_EN ;
                }   
                else {
                    cur_pipe->pipe_en &= ~PIPE0_EN ;
                }
                MMPS_3GPRECD_GetPreviewDisplayByEpId(&enable,STREAM_EP_H264) ;
                if(enable) {
                    cur_pipe->pipe_en |= PIPE1_EN ;
                } 
                else {
                    cur_pipe->pipe_en &= ~PIPE1_EN ;
                }
            }
            else {
            }
            
            //dbg_printf(3,"Set ZOOM pipe :%x,cur_pipe : %x,ep_id : %d\r\n",cur_pipe->pipe_en,cur_pipe,pCamCtx.epId);
            if ( (cur_pipe->pipe_en&PIPE_EN_MASK) == PIPE0_EN) {
                err1 = PCAM_USB_SetZoom(MMPF_SCALER_PATH_0,zoomInfo,MMP_TRUE);
                if(err1) {
                    goto set_zoom_end;
                }
            }
            else if ( (cur_pipe->pipe_en&PIPE_EN_MASK) == PIPE1_EN) {
                err2 = PCAM_USB_SetZoom(MMPF_SCALER_PATH_1,zoomInfo,MMP_TRUE);
                if(err2) {
                    goto set_zoom_end;
                }
            }
            else {

                OS_CPU_SR   cpu_sr = 0;
                err1 = PCAM_USB_SetZoom(MMPF_SCALER_PATH_0,zoomInfo,MMP_FALSE);
                if( (err1==PCAM_USB_H264_SWITCH_RT_MODE) || (err1==PCAM_USB_H264_SWITCH_FR_MODE)) {
                    goto set_zoom_end ;
                }
                err2 = PCAM_USB_SetZoom(MMPF_SCALER_PATH_1,zoomInfo,MMP_FALSE);
                if( (err2==PCAM_USB_H264_SWITCH_RT_MODE) || (err2==PCAM_USB_H264_SWITCH_FR_MODE)) {
                    goto set_zoom_end ;
                }
                IRQ_LOCK(
                    if(err1==MMP_ERR_NONE) {
                        MMPF_Scaler_EnableZoom(MMPF_SCALER_PATH_0);
                    }
                    if(err2==MMP_ERR_NONE) {    
                        MMPF_Scaler_EnableZoom(MMPF_SCALER_PATH_1);
                    }
                    
                    if(err1 && err2) {
                        goto set_zoom_end ;
                    }
                    
                )
                // pipe 1 is in gra loop
                if( grapath & 0x04) {
                    MMPF_Display_SetGRAFps(1,0);
                }
                
            }
            
            // To check if H264 is not start
            if(cur_pipe->pipe_en & PIPE1_EN ) {
                MMPD_MP4VENC_FW_OP status_vid ;    
                MMPD_VIDENC_GetStatus(&status_vid);
                if(status_vid!=MMPD_MP4VENC_FW_OP_START) {
                    dbg_printf(3,"_1st_gra_loop...\r\n");
                    MMPF_Scaler_SetForceSwitchPath(MMPF_SCALER_PATH_1);
                    goto _1st_gra_loop ;
                }
            }
            
            if(grapath & (0x02 | 0x04)) {
                MMP_USHORT ms = 0;
                
                if(grapath & 0x02) {
                    if(ISP_IF_AE_GetRealFPS()) {
                        ms = (1000 * 10)/ ISP_IF_AE_GetRealFPSx10() ;
                    } else {
                        ms = (1000 * 10)/ gsCurFrameRatex10[0] ;
                    }
                }    
                
                if(ms < 33) ms = 33 ;// TEST  
               // ms = 0;//test  
                do {
                    MMPF_Scaler_CheckZoomComplete(path_sel, &bComplete);
                    MMPF_OS_Sleep(1);
                } while (!bComplete);
                // Slow down UI 
                //MMPF_OS_Sleep(3 * ( ));
                dbg_printf(0,"____ms : %d\r\n",ms);
                MMPF_OS_Sleep(4*ms);
            }
            
            // else 
_1st_gra_loop:            
            if( grapath&1 ) {
                MMPF_SCALER_ZOOM_INFO *scalerInfo;
                MMPF_SCALER_SWITCH_PATH *pinfo ;
                
                // Handle Pipe 0
                pinfo = MMPF_Scaler_GetPathInfo(MMPF_SCALER_PATH_0) ;
                if( ((grapath&2)==0) || pinfo->forceswitch ) {
                    if ( (cur_pipe->pipe_en&PIPE_EN_MASK) & PIPE0_EN) {
                        scalerInfo = MMPF_Scaler_GetCurInfo(MMPF_SCALER_PATH_0);   
                        if (scalerInfo->grabCtl.usScaleN > scalerInfo->grabCtl.usScaleM) {
                            
                            dbg_printf(3,"Task:pipe0 1st N/M %d> %d\r\n",scalerInfo->grabCtl.usScaleN ,scalerInfo->grabCtl.usScaleM );
                            
                            // Wait path to GRA
                            while( MMPF_Scaler_GetPath(MMPF_SCALER_PATH_0)!= MMPF_SCALER_SOURCE_GRA) { 
                               //dbg_printf(3,"G0");
                                MMPF_OS_Sleep(1);
                            }
                            // Wait scaler finished
                            do {
                                MMPF_Scaler_CheckZoomComplete(MMPF_SCALER_PATH_0, &bComplete);
                                //dbg_printf(3,"Z0");
                                MMPF_OS_Sleep(1);
                            } while (!bComplete);
                            
                        }     
                    }
                }
                pinfo = MMPF_Scaler_GetPathInfo(MMPF_SCALER_PATH_1) ;
                
                // Handle Pipe 1
                if( ((grapath&4)==0) || pinfo->forceswitch ) {
                    if ( (cur_pipe->pipe_en&PIPE_EN_MASK) & PIPE1_EN) {
                        scalerInfo = MMPF_Scaler_GetCurInfo(MMPF_SCALER_PATH_1);   
                        if (scalerInfo->grabCtl.usScaleN > scalerInfo->grabCtl.usScaleM) {
                            //MMPD_MP4VENC_FW_OP status_vid ;
                            //MMPF_SCALER_SWITCH_PATH *pinfo ;
                            //MMPD_VIDENC_GetStatus(&status_vid);
                            //if(status_vid!=MMPD_MP4VENC_FW_OP_START) {
                            //    MMPF_Scaler_SetForceSwitchPath(MMPF_SCALER_PATH_1);    
                            //}
                            dbg_printf(3,"Task:pipe1 1st N/M %d> %d\r\n",scalerInfo->grabCtl.usScaleN ,scalerInfo->grabCtl.usScaleM );
                            //pinfo = MMPF_Scaler_GetPathInfo(MMPF_SCALER_PATH_1) ;
                            // Wait path to GRA
                            while( MMPF_Scaler_GetPath(MMPF_SCALER_PATH_1)!= MMPF_SCALER_SOURCE_GRA) {
                                //dbg_printf(3,"G1");
                                MMPF_OS_Sleep(1);
                            }
                            // Wait scaler finished
                            do {
                                MMPF_Scaler_CheckZoomComplete(MMPF_SCALER_PATH_1, &bComplete);
                                //dbg_printf(3,"Z1");
                                MMPF_OS_Sleep(1);
                            } while (!bComplete);
                            
                        }     
                    }
                }
            }
set_zoom_end:  
#if H264_SIMULCAST_EN
            if( (err2==PCAM_USB_H264_SWITCH_RT_MODE) || (err1 == PCAM_USB_H264_SWITCH_RT_MODE) ) {
                USB_SetH264EncodeBufferMode(MMP_FALSE);
            }
            else if( (err2==PCAM_USB_H264_SWITCH_FR_MODE) || (err1 == PCAM_USB_H264_SWITCH_FR_MODE) ) {
                USB_SetH264EncodeBufferMode(MMP_TRUE);
            }
            
            MMPF_Display_ResumeSimulcast();
            //MMPF_OS_Sleep(1000);
#endif
                    
            dbg_printf(GRA_DBG_LEVEL,"ZOOM :+++\r\n");
            
        }
#endif        
        break;
    case PCAM_USB_SETTING_DIGPAN:
#if SUPPORT_DIGITAL_PAN    // TBD
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            STREAM_SESSION *ss ;
            MMP_LONG pan_off0,tilt_off0;
            MMP_LONG pan_off1,tilt_off1;
            MMP_LONG pan_off2,tilt_off2;
            
            STREAM_CFG *cur_pipe  , cur_pipe_tmp = *usb_get_cur_image_pipe_by_epid(pCamCtx.epId);
            PCAM_USB_PANTILT  *panInfo = &pCamInfo.pCamDigPan ;
            MMP_UBYTE   grapath = 0;
            MMP_BOOL is_preview[2] = {0,0};
            
            cur_pipe = &cur_pipe_tmp;
            
            MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG) ;
            if(UVC_VCD()==bcdVCD15) {
                MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264) ;
            }
            
            // sean@2013_07_22, fixed bug
            if(is_preview[0]) {
                STREAM_CFG *pipe0 = usb_get_cur_image_pipe_by_epid(STREAM_EP_MJPEG);
                cur_pipe->pipe_en |= ( pipe0->pipe_en & PIPE_EN_LOOP2GRA ) ; 
                ss = MMPF_Video_GetStreamSessionByEp(0) ;
                while( ss->tx_flag & SS_TX_CONFIG_END) {
                    //dbg_printf(3,"P0,");
                    MMPF_OS_Sleep(1);
                }
            }
            if(is_preview[1]) {
                STREAM_CFG *pipe1 = usb_get_cur_image_pipe_by_epid(STREAM_EP_H264);
                cur_pipe->pipe_en |= ( pipe1->pipe_en & PIPE_EN_LOOP2GRA ) ; 
                ss = MMPF_Video_GetStreamSessionByEp(1) ;
                while( ss->tx_flag & SS_TX_CONFIG_END) {
                    //dbg_printf(3,"P1,");
                    MMPF_OS_Sleep(1);
                }    
            }
            cur_pipe->pipe_en &= ~PIPEEN(GRA_SRC_PIPE);

            #if SUPPORT_GRA_ZOOM
            grapath = MMPF_Video_IsLoop2GRA(cur_pipe) ;
            #endif    
            
            
            if(UVC_VCD()==bcdVCD15) {
                if(is_preview[0]) {
                    if(grapath & 0x02) {
                        cur_pipe->pipe_en &= ~PIPE0_EN ;
                        cur_pipe->pipe_en |=  PIPE2_EN ;
                    } else {
                        cur_pipe->pipe_en |= PIPE0_EN ;
                    }
                }   
                else {
                    cur_pipe->pipe_en &= ~PIPE0_EN ;
                }
                
                if(is_preview[1]) {
                    if(grapath & 0x04) {
                        cur_pipe->pipe_en &= ~PIPE1_EN ;
                        cur_pipe->pipe_en |=  PIPE2_EN ;
                    } else {
                        cur_pipe->pipe_en |= PIPE1_EN ;
                    }    
                } 
                else {
                    cur_pipe->pipe_en &= ~PIPE1_EN ;
                }
            }
            else {
                if( PIPE_EN(cur_pipe->pipe_en) & PIPE0_EN) {
                    if(grapath & 0x02) {
                        cur_pipe->pipe_en &= ~PIPE0_EN ;
                        cur_pipe->pipe_en |=  PIPE2_EN ;
                    }
                }
                if( PIPE_EN(cur_pipe->pipe_en) & PIPE1_EN) {
                    if(grapath & 0x04) {
                        cur_pipe->pipe_en &= ~PIPE1_EN ;
                        cur_pipe->pipe_en |=  PIPE2_EN ;
                    }
                }
                
            }


            #if H264_SIMULCAST_EN
            // In simulcast, we need to disable pipe 0 when pan/tilt
            // because, the pipe0's data is from pipe 1
            if(UVCX_IsSimulcastH264()) {
                if(1 /* UVCX_GetH264EncodeBufferMode()==FRAME_MODE*/) {
                    cur_pipe->pipe_en &= ~PIPE0_EN ;
                } else {
                    cur_pipe->pipe_en |= PIPE0_EN ;
                }
            }
            #endif


            dbg_printf(GRA_DBG_LEVEL,"PAN :--- %x\r\n",grapath);
            if ( (cur_pipe->pipe_en&PIPE_EN_MASK) == PIPE0_EN) {
                PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_0,panInfo,MMP_TRUE,&pan_off0,&tilt_off0);
            }
            else if ( (cur_pipe->pipe_en&PIPE_EN_MASK) == PIPE1_EN) {
                PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_1,panInfo,MMP_TRUE,&pan_off1,&tilt_off1);
            } 
            else if ( (cur_pipe->pipe_en&PIPE_EN_MASK) == PIPE2_EN) {
                PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_2,panInfo,MMP_TRUE,&pan_off2,&tilt_off2);
            }
            else {
                MMP_BOOL bComplete ;
                OS_CPU_SR cpu_sr = 0;
                
                if(1/*!(grapath& (0x02 | 0x04) )*/) {
                    
                    if(grapath & 2) {
                        PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_2,panInfo,MMP_FALSE,&pan_off2,&tilt_off2);
                    } 
                    else {
                        PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_0,panInfo,MMP_FALSE,&pan_off0,&tilt_off0);
                    }
                    if(grapath & 4) {
                        PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_2,panInfo,MMP_FALSE,&pan_off2,&tilt_off2);
                    }
                    else {
                        PCAM_USB_SetPanTilt(MMPF_SCALER_PATH_1,panInfo,MMP_FALSE,&pan_off1,&tilt_off1);
                    }
                    IRQ_LOCK(
                        if(grapath&2) {
                            MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH_2) ;
                        }
                        else {
                            MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH_0) ;
                        }
                        if(grapath&4) {
                            MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH_2) ;
                        }
                        else {
                            MMPF_Scaler_SetDigitalPanTilt(MMPF_SCALER_PATH_1) ;
                        }
                    )
                    
                    do {
                        MMPF_Scaler_CheckPanTiltComplete(MMPF_SCALER_PATH_0, &bComplete);
                        MMPF_OS_Sleep(1);
                    } while(!bComplete) ;
                    if(grapath&2) {
                        PCAM_USB_PostPanTilt(MMPF_SCALER_PATH_2,pan_off2,tilt_off2);
                    }
                    else {
                        PCAM_USB_PostPanTilt(MMPF_SCALER_PATH_0,pan_off0,tilt_off0);
                    }    
                    
                    if(grapath&4) {
                        PCAM_USB_PostPanTilt(MMPF_SCALER_PATH_2,pan_off2,tilt_off2);
                    } 
                    else {
                        PCAM_USB_PostPanTilt(MMPF_SCALER_PATH_1,pan_off1,tilt_off1);
                    }
                } 
                else {
                    dbg_printf(GRA_DBG_LEVEL,"#TBD:Scaliing up Pan/Tilt \r\n");
                }
            }
            dbg_printf(GRA_DBG_LEVEL,"PAN :+++\r\n");
        } 
#endif        
        break;
    case PCAM_USB_SETTING_FORCE_BLACK:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            PCAM_USB_Set_Black(pCamInfo.pCamForceBlack);
                
        } 
        break;  
           
    case PCAM_USB_SETTING_OSD_OP:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            PCAM_USB_ZOOM  *osdOp = &pCamInfo.pCamOsdOp ;   
            pcam_Osd_ScalingBuf(osdOp->Dir,osdOp->RangeMin,osdOp->RangeMax); 
        }
        break ;
    case PCAM_USB_SETTING_EFFECT:
        MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
        if(IsPreviewEnable) {
            PCAM_USB_INFO *pInfo = (PCAM_USB_INFO *)msg->msg_data;
            PCAM_USB_EFFECT_CTL *effOp =  &pInfo->pCamEffect ;
            dbg_printf(0,"effOp->times : %d,%x\r\n",effOp->times,effOp);
            if(!effOp->times) {
                goto exit_attr ;
            }
            if(effOp->times) {
                if(effOp->times&1) {
                    if(effOp->effect==WB_EFFECT) {
                        MMP_USHORT val;
                        if(effOp->orig_auto) {
                            ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO); 
                        } 
                        else {
                            ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL); 
                        }
                        val = PCAM2MMP_Normalize_WB(effOp->orig_val);  
                        ISP_IF_AWB_SetColorTemp(val);      
                    }
                    else if (effOp->effect==SATURATION_EFFECT){
                        MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(effOp->orig_val);
                        ISP_IF_F_SetSaturation(setvalue);                
                    }
                    
                }
                else {
                    
                    if(effOp->effect==WB_EFFECT) {
                        MMP_USHORT val;
                        ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL); 
                        val = PCAM2MMP_Normalize_WB(effOp->new_val); 
                        ISP_IF_AWB_SetColorTemp(val); 
                    }
                    else {
                        MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(effOp->new_val);
                        ISP_IF_F_SetSaturation(setvalue); 
                    }
                    //MMPF_OS_Sleep(effOp->on_ms);
                }
                MMPF_OS_Sleep(effOp->off_ms);
                effOp->times--;
            }
            if(!effOp->times) {
                if(effOp->effect==WB_EFFECT) {
                    MMP_USHORT val;
                    if(effOp->orig_auto) {
                        ISP_IF_AWB_SetMode(ISP_AWB_MODE_AUTO); 
                    } 
                    else {
                        ISP_IF_AWB_SetMode(ISP_AWB_MODE_MANUAL); 
                    }
                    val = PCAM2MMP_Normalize_WB(effOp->orig_val);  
                    ISP_IF_AWB_SetColorTemp(val);      
                }
                else if (effOp->effect==SATURATION_EFFECT){
                    MMP_SHORT setvalue = PCAM2MMP_Normalize_Value(effOp->orig_val);
                    ISP_IF_F_SetSaturation(setvalue);                
                }
                
            }
            else {
                pcam_usb_set_attributes(PCAM_NONBLOCKING,PCAM_USB_SETTING_EFFECT,(void *)effOp); 
                
            }
        }
        break ; 
                     
    }  
exit_attr:       
    // Change to sync. mode
    pCamVCCfg.pCamEnableAsyncMode = 0 ;
               
    //RTNA_DBG_Str(3,"PCAM_USB_SetAttrs end\r\n");
    return PCAM_ERROR_NONE ;
}

static MMP_USHORT PCAM_USB_SetZoom(MMPF_SCALER_PATH path_sel,PCAM_USB_ZOOM  *zoomInfo,MMP_BOOL start )
{
#define Regulus_TRY	(1)
#if H264_SIMULCAST_EN
extern MMP_UBYTE gbChangeH264BufMode ;
#endif
    MMP_BOOL bComplete ,lossy_up = MMP_TRUE ;
    MMPF_SCALER_ZOOM_INFO  *scalerInfo ;
    MMP_USHORT target_min,target_max ,target_val;
    MMP_USHORT target_minx,target_maxx ,target_valx;
    MMP_USHORT target_miny,target_maxy ,target_valy;
    
    scalerInfo = MMPF_Scaler_GetCurInfo(path_sel);
    if(1/*zoomInfo->Dir==2*/) {
        AITPS_SCAL  pSCAL    = AITC_BASE_SCAL;
        MMP_UBYTE   lpfDownSample ;
        if(path_sel==MMPF_SCALER_PATH_0) {
            target_min = (MMP_USHORT )pSCAL->SCAL_SCAL_0_H_N;
            target_minx = (MMP_USHORT)pSCAL->SCAL_SCAL_0_H_N;
            target_miny = (MMP_USHORT)pSCAL->SCAL_SCAL_0_V_N;
            lpfDownSample = pSCAL->SCAL_LPF_SCAL_CTL & SCAL_LPF_DNSAMP_MASK ;
        } else if(path_sel==MMPF_SCALER_PATH_1) {
            target_min =  (MMP_USHORT)pSCAL->SCAL_SCAL_1_H_N;
            target_minx = (MMP_USHORT)pSCAL->SCAL_SCAL_1_H_N;
            target_miny = (MMP_USHORT)pSCAL->SCAL_SCAL_1_V_N;
            lpfDownSample = pSCAL->SCAL_LPF1_SCAL_CTL & SCAL_LPF_DNSAMP_MASK ;
        } else {
            target_min =  (MMP_USHORT)pSCAL->SCAL_SCAL_2_H_N;
            target_minx = (MMP_USHORT)pSCAL->SCAL_SCAL_2_H_N;
            target_miny = (MMP_USHORT)pSCAL->SCAL_SCAL_2_V_N;
            lpfDownSample = pSCAL->SCAL_LPF2_SCAL_CTL & SCAL_LPF_DNSAMP_MASK ;
        } 
        
        //dbg_printf(3,"Get target_min:%d\r\n",target_min);
        if(lpfDownSample==SCAL_LPF_DNSAMP_1_2) {
            target_min  = target_min >> 1;
            target_minx = target_minx >> 1;
            target_miny = target_miny >> 1;
        }
        if(lpfDownSample==SCAL_LPF_DNSAMP_1_4) {
            target_min  = target_min >> 2;
            target_minx = target_minx >> 2;
            target_miny = target_miny >> 2;
        }
        
        #if SCALER_ZOOM_LEVEL==BEST_LEVEL
        {
            MMP_BOOL ret ;
            MMPF_SCALER_SWITCH_PATH *pinfo = MMPF_Scaler_GetPathInfo(path_sel);
            STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(path_sel);
            MMP_UBYTE gra_path ;
            gra_path = MMPF_Video_IsLoop2GRA(cur_pipe) ;
            
            if(!gra_path) {
                lossy_up = MMP_FALSE ;
            }
			#ifdef Regulus_TRY
			
		    //scalerInfo->usInputWidth  = SENSOR_16_9_W ;
    		//scalerInfo->usInputHeight = SENSOR_16_9_H ;
			if(usb_vc_cur_fpsx10(0) > 300 )
			{
	    		scalerInfo->usSensorWidth = SENSOR_16_9_W_60FPS ;
    			scalerInfo->usSensorHeight = SENSOR_16_9_H_60FPS ;
			}
			else
			{
	    		scalerInfo->usSensorWidth = SENSOR_16_9_W ;
    			scalerInfo->usSensorHeight = SENSOR_16_9_H ;
			}   
    		//scalerInfo->grabCtl.usStartX = (scalerInfo->usInputWidth - cur_pipe->pipe_w[path_sel])/2 ;
    		//scalerInfo->grabCtl.usStartY = (scalerInfo->usInputHeight - cur_pipe->pipe_h[path_sel])/2;
    		//scalerInfo->grabCtl.usEndX = scalerInfo->grabCtl.usStartX +  cur_pipe->pipe_w[path_sel] - 1 ;
    		//scalerInfo->grabCtl.usEndY = scalerInfo->grabCtl.usStartY +  cur_pipe->pipe_h[path_sel] - 1 ;
			
			#endif
            ret = GetScalerZoomGrab(path_sel,0,scalerInfo,zoomInfo,lossy_up);
            if(ret==MMP_FALSE) {
                dbg_printf(3,">Lossy up denied\r\n");
                return PCAM_ERROR_NONE;
            }
            #if H264_SIMULCAST_EN
            if( (!gbChangeH264BufMode) && ( UVCX_IsSimulcastH264() ) ) {
                MMP_UBYTE pipe_format ;
                pipe_format = CUR_PIPE_CFG(path_sel,cur_pipe->pipe_cfg) ;
                
                if( 1/*(pipe_format==PIPE_CFG_H264)*/  ){
                    
                    if (scalerInfo->grabCtl.usScaleN > scalerInfo->grabCtl.usScaleM) {
                        if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                            dbg_printf(3,"<H264 Try RT mode\r\n");
                            return PCAM_USB_H264_SWITCH_RT_MODE ;
                        }
                    }
                    else {
                        if(UVCX_GetH264EncodeBufferMode()==REALTIME_MODE) {
                            dbg_printf(3,"<H264 Try FR mode\r\n");
                            return PCAM_USB_H264_SWITCH_FR_MODE ;
                        }
                    }
                   
                }
            }
            #endif
            if( (scalerInfo->grabCtl.usScaleN==pinfo->n) && (scalerInfo->grabCtl.usScaleM==pinfo->m) ) {
                //dbg_printf(3,"Task:scalerInfo(N/M)=(%d/%d)\r\n",scalerInfo->grabCtl.usScaleN,scalerInfo->grabCtl.usScaleM);
                return PCAM_USB_ZOOM_ERR;
            }
        }
        #endif
        if(scalerInfo->usBaseN) {
            GetScalerZoomRange(0,&target_min,&target_max,scalerInfo,zoomInfo);
           // dbg_printf(3,"Zoom target : %d -> %d,Dir:%d,LPF_DS:%d\r\n",target_min,target_max,zoomInfo->Dir,lpfDownSample);
        } else {
            GetScalerZoomRange(1,&target_minx,&target_maxx,scalerInfo,zoomInfo);
            GetScalerZoomRange(2,&target_miny,&target_maxy,scalerInfo,zoomInfo);
           //dbg_printf(3,"Zoom X %d -> %d,Dir:%d\r\n",target_minx,target_maxx,zoomInfo->Dir);
           //dbg_printf(3,"Zoom Y %d -> %d,Dir:%d\r\n",target_miny,target_maxy,zoomInfo->Dir);
        
        }
        
        if(1/*target_min != target_max*/) {
            MMPF_SCALER_PANTILT_INFO *panTilt ;

            // Reset Pan & Tilt End flag for each zoom
            panTilt = MMPF_Scaler_GetCurPanTiltInfo(path_sel);
            panTilt->usPanEnd = 0 ;
            panTilt->usTiltEnd = 0 ;
            if(scalerInfo->usBaseN) {
                #if SUPPORT_GRA_ZOOM
                if(1) 
                #else
                if(scalerInfo->grabCtl.usScaleN <= scalerInfo->grabCtl.usScaleM) 
                #endif
                {
                    MMPF_Scaler_SetZoomRange(path_sel,target_min,target_max);
                } else {
                   // dbg_printf(3,"[Skip set zoom range]\r\n");
                }
            } else {
                MMPF_Scaler_SetZoomRangeEx(path_sel,target_minx,target_maxx,target_miny,target_maxy);
            }
            if(zoomInfo->Dir==0) {
                //target_val = target_max ;
                MMPF_Scaler_SetDigitalZoom(path_sel,MMPF_SCALER_ZOOMIN,(MMP_USHORT)-1,start); // 1 -> -1 for one step zoom
            } else {
                //target_val = target_min ;
                MMPF_Scaler_SetDigitalZoom(path_sel,MMPF_SCALER_ZOOMOUT,(MMP_USHORT)-1,start);
            } 
            
        } 
     }
    return PCAM_ERROR_NONE ;
}


static MMP_USHORT PCAM_USB_SetPanTilt(MMPF_SCALER_PATH path_sel,PCAM_USB_PANTILT  *panInfo,MMP_BOOL start,MMP_LONG *ret_pan_off,MMP_LONG *ret_tilt_off)
{
    MMP_BOOL    bComplete;  
    MMP_LONG    pan_off=0,tilt_off=0,off_x,off_y ;
    MMP_USHORT  limitW,limitH, grabW,grabH ,curStartX,curStartY ;
    MMPF_SCALER_PANTILT_INFO *scalerInfo ;
    scalerInfo = MMPF_Scaler_GetCurPanTiltInfo(path_sel);
    MMPF_Scaler_SetPanTiltParams(path_sel,panInfo->PanMax/3600,panInfo->TiltMax/3600,panInfo->Steps,&pan_off,&tilt_off) ;
    *ret_pan_off = pan_off;
    *ret_tilt_off = tilt_off;
    if(start==MMP_TRUE) {
        MMPF_Scaler_SetDigitalPanTilt(path_sel) ;
        do {
            MMPF_Scaler_CheckPanTiltComplete(path_sel, &bComplete);
            MMPF_OS_Sleep(1);
        } while(!bComplete);
        if((scalerInfo->usPanEnd==1)&&(pan_off) ) {
            scalerInfo->usPanEnd = 2 ;
        }
        if((scalerInfo->usTiltEnd==1)&&(tilt_off) ) {
            scalerInfo->usTiltEnd = 2 ;
        }
    }
    return PCAM_ERROR_NONE ;
}


static MMP_USHORT PCAM_USB_PostPanTilt(MMPF_SCALER_PATH path_sel,MMP_LONG pan_off,MMP_LONG tilt_off)
{
    MMPF_SCALER_PANTILT_INFO *scalerInfo ;
    scalerInfo = MMPF_Scaler_GetCurPanTiltInfo(path_sel);
    if((scalerInfo->usPanEnd==1)&&(pan_off) ) {
        scalerInfo->usPanEnd = 2 ;
    }
    if((scalerInfo->usTiltEnd==1)&&(tilt_off) ) {
        scalerInfo->usTiltEnd = 2 ;
    }

}

MMP_USHORT PCAM_USB_TakePicture(pcam_msg_t *msg)
{
    return PCAM_ERROR_NONE  ;
}

MMP_USHORT PCAM_USB_AudioStart(pcam_msg_t *msg)
{
#if SUPPORT_UAC==1
    AITPS_AUD   pAUD    = AITC_BASE_AUD;
    MMP_USHORT err = PCAM_ERROR_NONE ;
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
  MMP_USHORT audin = AFE_IN ;
#elif (MIC_SOURCE==MIC_IN_PATH_I2S)||(MIC_SOURCE == MIC_IN_PATH_BOTH)
  MMP_USHORT audin = I2S_IN ;
#endif
    
    PCAM_AUDIO_CFG *pcam_audio = (PCAM_AUDIO_CFG *)msg->msg_data;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    MMP_USHORT i;

    RTNA_DBG_Str(3, "PCAM_USB_AudioStart()\r\n");

	pcam_audio->pCamAudioChannel = PCAM_AUDIO_LINEIN_L;			//modify by Jared 20170810, on Razer project, there only Lift channel mic inupt.
    pCamCtx.audioInPath = PCAM2MMP_Get_AudioInPath(pcam_audio) ;
    pCamCtx.lineInChannel = PCAM2MMP_Get_AudioInChannel(pcam_audio) ;
 #if (MIC_SOURCE == MIC_IN_PATH_BOTH)
    audin = I2S_IN | AFE_IN;
 #else    
    if(pCamCtx.audioInPath==AUDIO_IN_I2S) {
        audin = I2S_IN ;
    } else {
        audin = AFE_IN ;
    }
#endif    

    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_AUD_DIS);
    if (audin & AFE_IN) {
      MMPF_Audio_EnableAFEClock(MMP_TRUE, MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));
      // sean@2012_11_09, remove the codes
      //MMPF_OS_Sleep_MS(5);
    } 
    
    MMPF_SetVoiceInPath(pCamCtx.audioInPath);
    // sean@2012_11_09, remove the codes
    //if (audin & AFE_IN) {
    //  MMPF_Audio_EnableAFEClock(MMP_FALSE, 0);
    //}
    //pGBL->GBL_CLK_DIS0 |= (GBL_CLK_AUD_DIS);
    
    
     
    MMPF_Audio_SetLineInChannel(pCamCtx.lineInChannel);
    MMPF_Audio_SetEncodeFormat(VIDEO_AAC_REC_MODE);  //MMPD_AUDIO_VAAC_ENCODE
    MMPF_Audio_SetEncodeLength(0xffffffff);
    // sean@2012_11_09, remove the codes
    //pGBL->GBL_CLK_DIS0 &= (~ GBL_CLK_AUD_DIS);
    //enable codec clock (afe path)
    //if (audin & AFE_IN) {
    //  MMPF_Audio_EnableAFEClock(MMP_TRUE, MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));
    //  // sean@2012_11_09, don;t need to wait
    //  //MMPF_OS_Sleep_MS(5);
    //}
    
    if(audin&I2S_IN) {
        MMPF_Audio_SetI2SInFormat(1) ; // Set slave
        #if AUDEXT_DAC==WM8737
        MMPF_PowerUpWM8737(pcam_audio->pCamSampleRate) ;
        #endif
        #if (AUDEXT_DAC == DA7211)
    MMPF_InitDA7211();
    MMPF_DA7211_PwUpExtMicIn();
    MMPF_DA7211_SetSamplingRate(pcam_audio->pCamSampleRate);
        #endif
    }  
    
    if(audin ==(AFE_IN | I2S_IN)) {  
        MMPF_Audio_InitializeEncodeFIFO(I2S_IN, /*ENC_SAMPLES_TIME*/ AFE_SAMPLES_TIME*((MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate)*AUDIN_CHANNEL)/1000)  /*32*/);
        MMPF_Audio_InitializeEncodeFIFO(AFE_IN, /*ENC_SAMPLES_TIME*/ AFE_SAMPLES_TIME*((MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate)*AUDIN_CHANNEL)/1000)  /*32*/);
        MMPF_InitAudioSetting(I2S_IN, MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));   //initial iis registers        
        MMPF_InitAudioSetting(AFE_IN, MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));   //initial iis registers        
        MMPF_Audio_SetMux(I2S_IN, 1);    
        MMPF_Audio_SetMux(AFE_IN, 1);    
    } else {
        MMPF_Audio_InitializeEncodeFIFO(audin, /*ENC_SAMPLES_TIME*/ AFE_SAMPLES_TIME*((MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate)*AUDIN_CHANNEL)/1000)  /*32*/);
        MMPF_InitAudioSetting(audin, MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));   //initial iis registers        
        MMPF_Audio_SetMux(audin, 1);
        
    }
#if ADC_PERFORMANCE_TEST==1
    MMPF_Audio_TestPerformance();
#endif
    
    
    // Save startup setting.
    //sean@2012_11_09
    if( !pcam_audio->pCamMute ) {
        gbOrigADCDigitalGain = gbADCDigitalGain  ;
    }
    MMPF_Audio_InitEncoder(pcam_audio->pCamSampleRate, ENC_SAMPLES_TIME*((pcam_audio->pCamSampleRate*AUDIN_CHANNEL)/1000) ) ;
    glAudioEnable = TRUE;
    return err ;
#else
    return PCAM_ERROR_NONE ;
#endif    
}

MMP_USHORT PCAM_USB_AudioStop(pcam_msg_t *msg)
{
#if SUPPORT_UAC==1 
    MMP_USHORT  err = PCAM_ERROR_NONE ;
    MMP_USHORT  audin = AFE_IN ;
    
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    volatile MMP_USHORT  i;

   // RTNA_DBG_Str(3, "USB_AUStop()\r\n");
    MMPF_Audio_ExitEncoder();
    #if (MIC_SOURCE == MIC_IN_PATH_BOTH)
        audin = I2S_IN | AFE_IN;
    #else
    if(pCamCtx.audioInPath==AUDIO_IN_I2S) {
        audin = I2S_IN ;
    } 
    #endif
    
    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_AUD_DIS);
    for(i = 0;i < 2000;i++)
        ;
    
    
    if(audin & I2S_IN) {
        MMPF_Audio_SetMux(I2S_IN, 0);
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_AUD_FIFO);
#if AUDEXT_DAC==WM8737
        MMPF_PowerDownWM8737();
#endif        
    }
#if (MIC_SOURCE != MIC_IN_PATH_I2S)
    if(audin & AFE_IN) {
        MMPF_Audio_SetMux(AFE_IN, 0);
        MMPF_CloseADC();        
        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_AFE_FIFO);
    }
#endif    
    MMPF_Audio_EnableAFEClock(MMP_FALSE, 0);
    pGBL->GBL_CLK_DIS0 |= (GBL_CLK_AUD_DIS);
    
    
  {    
    MMP_USHORT  csr;
    csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
        csr = csr & TXCSR_RW_MASK;
        UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_FLUSHFIFO);
        UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_FLUSHFIFO);
        // Clean IN-token, so that never get incorrect IN-token next time
        csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR) ;
        UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr & ~TX_UNDERRUN_BIT);
         
    }
    glAudioEnable = FALSE;

    return err ;
#else
    return PCAM_ERROR_NONE ;
#endif    
}

#if AUDEXT_DAC==WM8737
// volume range is from 0 to 0xFF, 0xC3 is 0 db
void MMPF_WM8737VolumeControl(MMP_UBYTE volume)
{
  AITPS_GBL   pGBL = AITC_BASE_GBL;
    unsigned int clock=0;

  #if (CHIP==P_V2)
  clock =  pGBL->GBL_CLK_EN ;
  pGBL->GBL_CLK_EN |= (GBL_CLK_I2CM|GBL_CLK_VI);
    #endif
    
  #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
  pGBL->GBL_CLK_DIS0 &= ~(GBL_CLK_VI_DIS);
  pGBL->GBL_CLK_DIS1 &= ~(GBL_CLK_I2C_DIS);
    #endif
    
  MMPF_I2cm_Initialize(&m_WM8737_I2cmAttribute);  
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x01, volume);
  if(volume >= 0)
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x03, volume);
  else
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x03, 0);
  #if (CHIP==P_V2)
  pGBL->GBL_CLK_EN  = clock;
  #endif
  
}
#endif

#include "mmpf_usbuac.h"

void PCAM_USB_SetAudioGain(MMP_UBYTE pga_gain, MMP_UBYTE boostdb,MMP_UBYTE dig_gain)
{
    
    MMP_ULONG cpu_sr ;
    #if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
    IRQ_LOCK(
        MMPF_Audio_SetADCAnalogGain(pga_gain,boostdb);
        MMPF_Audio_SetADCDigitalGain(dig_gain);
    )
    #endif
    #if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
    //dbg_printf(3,"PGA : %x, DIG : d%d\r\n",pga_gain,dig_gain);
    MMPF_Audio_SetTargetADCGain(pga_gain,10);
    MMPF_Audio_SetADCDigitalGain(dig_gain);
    #endif
}

MMP_USHORT PCAM_USB_SetVolume(pcam_msg_t *msg)
{
#if SUPPORT_UAC==1
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    PCAM_AUDIO_CFG *pcam_audio = (PCAM_AUDIO_CFG *)msg->msg_data;
    MMP_SHORT voldb = pcam_audio->pCamVolume ;
    MMP_UBYTE dgain=0;
    //MMP_UBYTE dgain=0,boostdb = 0,pga_gain;
    //dbg_printf(0,"#[%d]pcam_audio->pCamVolume : %x\r\n",glAudioEnable,pcam_audio->pCamVolume);
    //dbg_printf(0,"#pcam_audio->pCamMute : %x\r\n",pcam_audio->pCamMute);
    #if(MIC_SOURCE == MIC_IN_PATH_AFE)
    PCAM_USB_SetGain_UAC2ADC(voldb,MMPF_Audio_GetRealSampleRate(pcam_audio->pCamSampleRate));
    #endif
    
    if(!glAudioEnable) {
        // sean@2012_11_09 add
        if(pcam_audio->pCamMute) {
            gbOrigADCDigitalGain = gbADCDigitalGain ;
            gbADCDigitalGain = 0;
        } else {
            gbADCDigitalGain = gbOrigADCDigitalGain ;
        }
        return PCAM_ERROR_NONE ;
    } 
    
#if(MIC_SOURCE == MIC_IN_PATH_AFE)
    #if ADC_FINE_TUNE_POP_NOISE==0
    MMPF_Audio_SetADCAnalogGain(gbADCAnalogGain,gbADCBoost);
    #else
    PCAM_USB_SetAudioGain(gbADCAnalogGain,gbADCBoost,gbADCDigitalGain);
    #endif
    
#endif        
    if(!pcam_audio->pCamMute) {
        #if(MIC_SOURCE == MIC_IN_PATH_AFE)
            #if CHIP==VSN_V3
            #if ADC_FINE_TUNE_POP_NOISE==0 
            MMPF_Audio_SetADCDigitalGain(gbADCDigitalGain);
            #endif
            #endif 
        #elif(MIC_SOURCE == MIC_IN_PATH_I2S)
            ISP_IF_3A_Control(ISP_3A_PAUSE);
            #if AUDEXT_DAC==WM8737
            //WM8737 logic, dgain is for APGA
            //the range check should be done before here. 0x8000 will not call here.
            dgain = (MMP_UBYTE) ((MMP_BYTE)((voldb - FIXED_PREAMP) >> 7) + 0xC3);
            if ((voldb == (MMP_SHORT)0x8000) || (voldb == VOL_MIN)) {
                dgain = 0;
            }
             //0xC3 == 0db
            MMPF_WM8737VolumeControl(dgain);        
            #endif 
            ISP_IF_3A_Control(ISP_3A_RECOVER);     
        #endif

    }
    //dbg_printf(3,"setvolume : BAD(%x,%x,%x)\r\n",pAFE->AFE_ADC_BOOST_CTL,pAFE->AFE_ADC_LCH_PGA_GAIN_CTL,pAFE->AFE_ADC_LCH_DIGITAL_VOL);
#endif    
     return PCAM_ERROR_NONE ;
}

void PCAM_USB_SetGain_UAC2ADC(MMP_SHORT voldb,MMP_USHORT sr)
{
#if SUPPORT_UAC==1
    MMP_BYTE dig_gain ;
    MMP_UBYTE dgain=0,boostdb = 0,pga_gain=0;
    MMP_SHORT voldigital = 0,update = 0;
    //dbg_printf(1,"#VOL from UAC : %d db\r\n",voldb);    
    if(voldb!=(MMP_SHORT)0x8000) {
        voldigital = (voldb & 255) / 128 ;
        voldb = voldb / 256 ; // 1 db Step
        //if(voldigital==3) {
        //    voldb++ ;
        //    //voldigital = 1 ;
        //}
    } else {
        voldb = VOL_MIN / 256;// 0 ; // minum gain
        //voldigital = 0 ; // test
        
    }
    //dbg_printf(3,"#uac.VOL :%d db\r\n",voldb);   
    //dbg_printf(3,"#VOL db from UAC : %d db,voldigital : %d\r\n",voldb,voldigital);    
    if(voldb) {
        #if ADC_FINE_TUNE_POP_NOISE == FIXED_BOOST_GAIN
        boostdb = AFE_FIX_BOOST_GAIN ;
        voldb -= (MMP_SHORT)AFE_FIX_BOOST_GAIN ;
        #else
        if(voldb >= 40) {
            boostdb = 40 ;
            voldb -= boostdb;
        } else if (voldb >= 30) {
            boostdb = 30 ;
            voldb -= boostdb ;
        } else if(voldb >=20) {
            boostdb = 20;
            voldb -= boostdb ;
        } else {
            boostdb = 0 ;
            
        }
        #endif
        // 8423
        //pga_gain = ( voldb >> 1 ) + 0x05 ; // 0x05 : 0dB
        
        #if ADC_FINE_TUNE_POP_NOISE==DC_OFFSET_OFF 
        pga_gain = voldb + AFE_ADC_0DB ; // 0x0b : 0dB
        #endif
        
        #if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
        {
            dig_gain = (MMP_BYTE)voldb - ADC_FIX_PGA_GAIN ;
            dig_gain = DEFAULT_ADC_DIGITAL_GAIN + 2 * dig_gain ;
        }
        #endif
        
        #if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
        if(voldb > 0) {
            dig_gain = DEFAULT_ADC_DIGITAL_GAIN ;
            pga_gain = voldb + AFE_ADC_0DB ; 
        }
        else {
            pga_gain =  AFE_ADC_0DB ; 
            dig_gain = DEFAULT_ADC_DIGITAL_GAIN + 2 *  (MMP_BYTE)voldb ;
        }
        #endif
        update = 1 ;
    } else {
    //    gbADCDigitalGain = 0 ; // Should not turn off digital gain except mute command
    }
    
    
    if(update) {
    
    #if ADC_FINE_TUNE_POP_NOISE==DC_OFFSET_OFF
        gbADCAnalogGain  = pga_gain ;
    #endif
    #if ADC_FINE_TUNE_POP_NOISE==FIXED_PGA_GAIN
        gbADCDigitalGain = dig_gain ;
    #endif
    #if ADC_FINE_TUNE_POP_NOISE==FIXED_BOOST_GAIN
        gbADCDigitalGain = dig_gain ;
        gbADCAnalogGain  = pga_gain ;
    #endif
        gbADCBoost = boostdb;
    }

    
    if(gbADCMute) {
        gbOrigADCDigitalGain = gbADCDigitalGain ;
        gbADCDigitalGain = 0;
    }
    //dbg_printf(3,"uac2adc : BADO(%x,%x,d%d,d%d)\r\n",gbADCBoost,gbADCAnalogGain,gbADCDigitalGain,gbOrigADCDigitalGain);
#endif    
}

MMP_USHORT PCAM_USB_SetSamplingRate(pcam_msg_t *msg)
{
#if SUPPORT_UAC==1

    if(glAudioEnable) {
        PCAM_USB_AudioStop(msg);    
        {   
            PCAM_AUDIO_CFG *pcam_audio = (PCAM_AUDIO_CFG *)msg->msg_data; 
            MMP_USHORT  csr;
            csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR);
            csr = csr & TXCSR_RW_MASK;
            UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_FLUSHFIFO);
            UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr | SET_TX_FLUSHFIFO);
            csr = UsbReadTxEpCSR(PCCAM_AU_EP_ADDR) ;
            UsbWriteTxEpCSR(PCCAM_AU_EP_ADDR, csr & ~TX_UNDERRUN_BIT);
            // Reset audio max pkt size when change sampling rate
            usb_audio_set_max_pktsize(pcam_audio->pCamSampleRate,0);
        }
        PCAM_USB_AudioStart(msg);
    }

#endif    
    return PCAM_ERROR_NONE;
}
MMP_USHORT PCAM_USB_SetMute(pcam_msg_t *msg)
{
#if SUPPORT_UAC==1
  //  AITPS_AUD   pAUD    = AITC_BASE_AUD;
    AITPS_AFE   pAFE    = AITC_BASE_AFE;
    PCAM_AUDIO_CFG *pcam_audio = (PCAM_AUDIO_CFG *)msg->msg_data;
    MMP_SHORT voldb = pcam_audio->pCamVolume ;
    MMP_UBYTE dgain ;

    //RTNA_DBG_Str3("PCAM_USB_SetMute:");RTNA_DBG_Byte3(pcam_audio->pCamMute);RTNA_DBG_Str3("\r\n");
    
    gbADCMute = pcam_audio->pCamMute ;
    dbg_printf(3,"mute ? :%s\r\n",gbADCMute?"on":"off");
    if(!glAudioEnable) {
#if MIC_SOURCE==MIC_IN_PATH_AFE
        if(pcam_audio->pCamMute) {
            gbOrigADCDigitalGain = gbADCDigitalGain ;
            gbADCDigitalGain = 0;
        } else {
            gbADCDigitalGain = gbOrigADCDigitalGain ;
        }
#endif
        return PCAM_ERROR_NONE ;
    }
    if(pcam_audio->pCamMute) {

#if MIC_SOURCE==MIC_IN_PATH_AFE
        if(gbADCDigitalGain) {
            gbOrigADCDigitalGain = gbADCDigitalGain ;
        }
        MMPF_Audio_SetADCDigitalGain(0) ; // mute
#elif MIC_SOURCE==MIC_IN_PATH_I2S

    #if AUDEXT_DAC==WM8737
        MMPF_WM8737VolumeControl(0);        
    #endif

#endif


    } else {
#if (MIC_SOURCE==MIC_IN_PATH_AFE)

        // restore 
        if(gbADCDigitalGain) {
            MMPF_Audio_SetADCDigitalGain(gbADCDigitalGain);
        } else {
            MMPF_Audio_SetADCDigitalGain(gbOrigADCDigitalGain);
        }
        
#elif (MIC_SOURCE==MIC_IN_PATH_I2S)

    #if AUDEXT_DAC==WM8737
        //WM8737 logic, dgain is for APGA
        //the range check should be done before here. 0x8000 will not call here.
        dgain = (MMP_UBYTE) (((/*dB*/voldb - FIXED_PREAMP) / 128) + 0xC3);
        if (voldb == (MMP_SHORT)0x8000) {
            dgain = 0;
        }

        MMPF_WM8737VolumeControl(dgain);        
    #endif

#endif

    }
    
    //dbg_printf(3,"Dig.Gain(orig,cur) = (d%d,d%d)\r\n",gbOrigADCDigitalGain,gbADCDigitalGain);
    
    //dbg_printf(3,"mute : BAD(%x,%x,d%d)\r\n",pAFE->AFE_ADC_BOOST_CTL,pAFE->AFE_ADC_LCH_PGA_GAIN_CTL,pAFE->AFE_ADC_LCH_DIGITAL_VOL);
 #endif
    
    return PCAM_ERROR_NONE ;
}

extern MMP_UBYTE eu1_get_mmp_result_val[];

MMP_USHORT PCAM_USB_UpdateFirmware(pcam_msg_t *msg)
{
    MMP_USHORT step = *(MMP_USHORT *)msg->msg_data;
    MMP_ERR   status;

    RTNA_DBG_Str( 3,"PCAM_USB_UpdateFirmware:");
    RTNA_DBG_Short(3,step);
    RTNA_DBG_Str(3,"\r\n");
    switch(step) {
    case 0:
        USB_DownloadFWInitialize();
        break;
    case 1:
        if(gbUVCPara[0] == 0){
            RTNA_DBG_Str( 3,"Old Firmware Package\r\n");
            MMPF_SYS_WatchDogEn(0);
            status = USB_Burning840FW2SFlash();
            MMPF_SYS_WatchDogEn(1);
        }
        else{
            RTNA_DBG_Str( 3,"New Firmware Package\r\n");
            MMPF_SYS_WatchDogEn(0);
            #if (CHIP==P_V2)
            status = USB_BurningROMPackage();
            #endif
            MMPF_SYS_WatchDogEn(1);
        }
        if(status == 0){
            eu1_get_mmp_result_val[0] = 0;  // No Error
        }
        else{
            eu1_get_mmp_result_val[0] = 0x82;  // Burning FW Error
        }
        break;    
    case 2:
        return PCAM_USB_FIRMWARE_ERR ;
    }   
    return PCAM_ERROR_NONE ;
    
}

MMP_USHORT PCAM_USB_TaskAlive(pcam_msg_t *msg)
{
   
    volatile    MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *)0x80000000;
    MMP_USHORT heartbeat = *(MMP_USHORT *)msg->msg_data;
#if HEARTBEAT_LED_EN==1
    RTNA_DBG_Str(3,"[PCAM] : Alive\r\n");
#endif    
#if HEARTBEAT_LED_EN==1
    USB_LEDDisplay(TEST_PIN1,heartbeat);
#endif    
    return PCAM_ERROR_NONE ;
}


MMP_USHORT PCAM_USB_EraseFlash(MMP_UBYTE mode, MMP_ULONG addr)
{
    AITPS_GBL pGBL = AITC_BASE_GBL;
  AITPS_SIF pSIF = AITC_BASE_SIF;
    MMP_ULONG clock;

    RTNA_DBG_Str( 3,"PCAM_USB_EraseFlash:");
    RTNA_DBG_Byte(3,mode);
    RTNA_DBG_Str( 3," : ");
    RTNA_DBG_Long(3,addr);
    RTNA_DBG_Str(3,"\r\n");
    
    switch(mode) {
    case 1:  // Chip Erase
        break;
    case 2:  // Sector Erase
        clock = pGBL->GBL_CLK_DIS1;
        pGBL->GBL_CLK_DIS0 &= ~GBL_CLK_BS_SPI_DIS;
        pSIF->SIF_CLK_DIV = 4;
    MMPF_SF_EraseSector(addr);
    pGBL->GBL_CLK_DIS1 = clock;
        break;    
    default:
        return PCAM_USB_FIRMWARE_ERR ;
    }   
    return PCAM_ERROR_NONE ;
    
}

//------------------------------------------------------------------------------
//  Function    : RawImageCapture
//------------------------------------------------------------------------------
MMP_USHORT PCAM_USB_TakeRawPicture(pcam_msg_t *msg/*MMP_ULONG ulRawStoreAddr*/)
{

  return MMP_ERR_NONE;    
}

MMP_USHORT PCAM_USB_Set_FPS(MMP_SHORT fps) 
{
static MMP_SHORT gsCurFPS ;

 //   VAR_W(3,fps);
    
    if(fps==0) {
        ISP_IF_AE_SetFPS(0);
        return 0;
    }
    
    if(fps > 0) {
        gsCurFPS = fps ;
    }
    VAR_W(3,gsCurFPS);
    
    ISP_IF_AE_SetFPS(gsCurFPS);
    return MMP_ERR_NONE;
}

//
// input :
// 50  : 5   fps
// 75  : 7.5 fps
// 150 : 15  fps

MMP_USHORT PCAM_USB_Set_FPSx10( MMP_BOOL var_fps,MMP_SHORT minfps_x10,MMP_SHORT maxfps_x10)
{
static MMP_SHORT gsCurMaxFPSx10,gsCurMinFPSx10 = 50 ;

    if(var_fps) {
        ISP_IF_AE_SetFPSx10(0);  // auto
        if(minfps_x10 > 0) {     
            gsCurMinFPSx10 = minfps_x10 ;
        } 
        if(maxfps_x10 > 0) {
            gsCurMaxFPSx10 = maxfps_x10 ;
        }
        ISP_IF_AE_SetMinFPSx10(gsCurMinFPSx10); // min
        ISP_IF_AE_SetMaxFPSx10(gsCurMaxFPSx10); // max
        dbg_printf(0,"Auto FPS,range:%d ~ %d\r\n",gsCurMinFPSx10,gsCurMaxFPSx10);
    } else {
        if(maxfps_x10 > 0) {
            gsCurMaxFPSx10 = maxfps_x10 ;
        }
        ISP_IF_AE_SetFPSx10(gsCurMaxFPSx10);  // fixed 
        dbg_printf(0,"Fixed FPS,range:%d\r\n",gsCurMaxFPSx10);
    }
    
    return MMP_ERR_NONE;
}



static MMP_SHORT m_ScalerClipDef[PIPE_MAX][3][2] = {
   {
    { -1, -1},
    { -1, -1},
    { -1, -1}
   }, 
   {
    { -1, -1},
    { -1, -1},
    { -1, -1}
   }, 
   {
    { -1, -1},
    { -1, -1},
    { -1, -1}
   }
};


MMP_USHORT PCAM_USB_Set_DefScalerClipRange(void)
{
    PIPE_PATH   pipe_id ;
    for(pipe_id=0;pipe_id < PIPE_MAX;pipe_id++) {
        m_ScalerClipDef[pipe_id][0][0] = -1 ;
        m_ScalerClipDef[pipe_id][1][0] = -1 ;
        m_ScalerClipDef[pipe_id][2][0] = -1 ;                                                             
    } 
    return MMP_ERR_NONE ;  
}

MMP_USHORT PCAM_USB_Set_Black(MMP_BOOL en)
{
#if (CHIP==P_V2)
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *)0x80000000;
    if(en) {
        REG_BASE_B[0x475] = 0 ;
        REG_BASE_B[0x476] = 0 ;
        REG_BASE_B[0x477] = 0 ;
            
    } else {
        MMP_UBYTE i;
        PCAM_USB_VIDEO_FORMAT fmt ;
        for(i=0;i<VIDEO_EP_END();i++) {
            fmt = GetFmtIndexByEp(i);
            if((fmt==PCAM_USB_VIDEO_FORMAT_YUV422) || (fmt==PCAM_USB_VIDEO_FORMAT_H264)) {
                MMPF_Sensor_SetColorTransform(1);
                dbg_printf(3,"ColorRange for YUY2/H264\r\n");
                return MMP_ERR_NONE ;
            }
        }
        dbg_printf(3,"ColorRange for MJPEG\r\n");
        MMPF_Sensor_SetColorTransform(1);
    }
#endif
#if (CHIP==VSN_V2) ||(CHIP==VSN_V3)
static MMP_SHORT m_ScalerClipBlack[3][2] = {
    {   0 , 0 },
    { 128, 128},
    { 128, 128}
};


static MMP_UBYTE m_ScalerMatrixCtl[PIPE_MAX];

    PIPE_PATH   pipe_id ;
    for(pipe_id=0;pipe_id < PIPE_MAX;pipe_id++) {
        if(!en) {
            if(m_ScalerClipDef[pipe_id][0][0]!=-1) {
                MMPF_Scaler_SetOutputRange(pipe_id,m_ScalerClipDef[pipe_id][0][0],m_ScalerClipDef[pipe_id][0][1],
                                                  m_ScalerClipDef[pipe_id][1][0],m_ScalerClipDef[pipe_id][1][1],
                                                  m_ScalerClipDef[pipe_id][2][0],m_ScalerClipDef[pipe_id][2][1]);
                MMPF_Scaler_SetMatrixCtl(pipe_id,m_ScalerMatrixCtl[pipe_id]);                                                  
                m_ScalerClipDef[pipe_id][0][0] = -1 ;
                m_ScalerClipDef[pipe_id][1][0] = -1 ;
                m_ScalerClipDef[pipe_id][2][0] = -1 ;                                                             
            }
        }   
        else {
            if(m_ScalerClipDef[pipe_id][0][0]==-1) {
                MMPF_Scaler_GetOutputRange(pipe_id,( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][0][0],( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][0][1],
                                                  ( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][1][0],( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][1][1],
                                                  ( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][2][0],( MMP_UBYTE *)&m_ScalerClipDef[pipe_id][2][1]);
                                                  
                MMPF_Scaler_SetOutputRange(pipe_id  ,m_ScalerClipBlack[0][0],m_ScalerClipBlack[0][1],
                                                    m_ScalerClipBlack[1][0],m_ScalerClipBlack[1][1],
                                                    m_ScalerClipBlack[2][0],m_ScalerClipBlack[2][1]);
                                                    
                m_ScalerMatrixCtl[pipe_id] = MMPF_Scaler_GetMatrixCtl(pipe_id);
                MMPF_Scaler_SetMatrixCtl(pipe_id,m_ScalerMatrixCtl[pipe_id] | SCAL_COLRMTX_EN ) ;
            }                
        } 
    }
#endif

    return MMP_ERR_NONE;
}

static MMP_USHORT PCAM2MMP_Get_Quality(PCAM_USB_INFO *pcam_info)
{
    MMP_USHORT q = VIDRECD_QUALITY_HIGH ;
    switch(pcam_info->pCamVideoQuality) {
    case PCAM_USB_HIGH_Q:
        q = VIDRECD_QUALITY_HIGH ;
        break ;
    case PCAM_USB_NORMAL_Q:
        q = VIDRECD_QUALITY_MID ;
        break ;
    case PCAM_USB_LOW_Q:
        q = VIDRECD_QUALITY_LOW ;
        break ;
    default:
        q = VIDRECD_QUALITY_HIGH ; //pcam_info->pCamVideoQuality ;
        break ;    
    }
    return q ;
}



static MMP_USHORT PCAM2MMP_Get_Resolution(PCAM_USB_INFO *pcam_info)
{
    return pcam_info->pCamVideoRes;
}

static MMPS_3GPRECD_VIDEO_FORMAT PCAM2MMP_Get_VideoFormat(PCAM_USB_INFO *pcam_info)
{
    MMPS_3GPRECD_VIDEO_FORMAT fmt = MMPS_3GPRECD_VIDEO_FORMAT_H264 ;
    switch(pcam_info->pCamVideoFormat) {
    case PCAM_USB_VIDEO_FORMAT_H263:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_H263 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_MP4V:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_MP4V ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_H264:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_H264 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_MJPEG:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_MJPEG ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_YUV422:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_YUV422 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_YUV420:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_YUV420 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_RAW:
    fmt = MMPS_3GPRECD_VIDEO_FORMAT_RAW;
    break;        
    }
    return fmt ;
}

static MMPS_3GPRECD_VIDEO_FORMAT PCAM2MMP_Get_VideoFormat2(PCAM_USB_VIDEO_FORMAT pCamVideoFormat)
{
    MMPS_3GPRECD_VIDEO_FORMAT fmt = MMPS_3GPRECD_VIDEO_FORMAT_H264 ;
    switch(pCamVideoFormat) {
    case PCAM_USB_VIDEO_FORMAT_H263:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_H263 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_MP4V:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_MP4V ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_H264:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_H264 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_MJPEG:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_MJPEG ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_YUV422:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_YUV422 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_YUV420:
        fmt = MMPS_3GPRECD_VIDEO_FORMAT_YUV420 ;
        break ;
    case PCAM_USB_VIDEO_FORMAT_RAW:
    fmt = MMPS_3GPRECD_VIDEO_FORMAT_RAW;
    break;        
    }
    return fmt ;
}

static MMPS_3GPRECD_AUDIO_FORMAT PCAM2MMP_Get_AudioFormat(PCAM_USB_INFO *pcam_info)
{
    MMPS_3GPRECD_AUDIO_FORMAT fmt = MMPS_3GPRECD_AUDIO_FORMAT_AAC ;
    switch(pcam_info->pCamAudioFormat) {
    case PCAM_USB_AUDIO_FORMAT_AAC:
        fmt = MMPS_3GPRECD_AUDIO_FORMAT_AAC;
        break;
    case PCAM_USB_AUDIO_FORMAT_AMR:
        fmt = MMPS_3GPRECD_AUDIO_FORMAT_AMR;
        break;        
    }
    return fmt ;
}

static MMP_USHORT PCAM2MMP_Get_DebandMode(PCAM_USB_INFO *pcam_info)
{
    MMP_USHORT deband = SENSOR_DSC_DEBAND_60HZ ;
    if(pcam_info->pCamDebandMode==PCAM_USB_DEBAND_50HZ) {
        deband = SENSOR_DSC_DEBAND_50HZ ;
    }
    return deband ;
}

static MMP_USHORT PCAM2MMP_Get_AudioInPath(PCAM_AUDIO_CFG *pcam_audio)
{
    MMP_USHORT audin = AUDIO_IN_AFE_DIFF ;
    switch(pcam_audio->pCamAudioInPath) {
    case PCAM_AUDIO_IN_I2S:
        audin = AUDIO_IN_I2S ; 
        break; 
    case PCAM_AUDIO_AFE_SING:
        audin = AUDIO_IN_AFE_SING ; 
        break; 
    case PCAM_AUDIO_AFE_DIFF:
        audin = AUDIO_IN_AFE_DIFF ; 
        break; 
    case PCAM_AUDIO_AFE_DIFF2SING:
        audin = AUDIO_IN_AFE_DIFF2SING ; 
        break; 
    }
    return audin ;
}

static MMPF_AUDIO_LINEIN_CHANNEL PCAM2MMP_Get_AudioInChannel(PCAM_AUDIO_CFG *pcam_audio)
{
    MMPF_AUDIO_LINEIN_CHANNEL channel = MMPF_AUDIO_LINEIN_DUAL ;
    switch(pcam_audio->pCamAudioChannel) {
    case PCAM_AUDIO_LINEIN_DUAL:
        channel = MMPF_AUDIO_LINEIN_DUAL ;
        break; 
    case PCAM_AUDIO_LINEIN_R:
        channel = MMPF_AUDIO_LINEIN_R ;
        break; 
    case PCAM_AUDIO_LINEIN_L:
        channel = MMPF_AUDIO_LINEIN_L ;
        break; 
    }
    return channel ;
}

static MMP_SHORT PCAM2MMP_Normalize_Value(MMP_USHORT val)
{
    MMP_SHORT n_val = (MMP_SHORT)(( ( 200.0 * val ) / 255 ) - 100 ) ;
   // VAR_W(0,n_val) ;
    return n_val ;   
}


static MMP_USHORT PCAM2MMP_Normalize_EV(MMP_USHORT val)
{
    MMP_USHORT n_val = (MMP_USHORT)(( ( 20.0 * val ) / 255 )  ) ;
    //VAR_W(0,n_val) ;
    return n_val ;   
    
}
// UVC range : 0~ 255
// AIT Gain range :  GainBase ~ 8 * GainBase //64~511
static MMP_USHORT PCAM2MMP_Normalize_Gain(MMP_USHORT val)
{
    //return  (MMP_USHORT)(val * ( 511 - 64 ) / 255 + 64) ;   
    return  (MMP_USHORT)(val * (ISP_IF_AE_GetGainBase() * 7) / 255 + ISP_IF_AE_GetGainBase()) ;   
   
}

//
// mapping 0~10000 to 2800~7500
//
static MMP_USHORT PCAM2MMP_Normalize_WB(MMP_USHORT val)
{
    return val;// 2800+ (val * (7500-2800 ))/10000 ;
}

// UVC range : 5 ~ 2047
// AIT Shutter range :  1/2048s ~ 1/2s
static MMP_USHORT PCAM2MMP_Normalize_Shutter(MMP_USHORT val)
{
  MMP_USHORT n_shift = 14;
  if(val > 1280)
    return ISP_IF_AE_GetShutterBase()>>2;
  else{
    while(val>0){
      val = val>>1;
      n_shift--;
    }
    RTNA_DBG_Short(3, n_shift);
        RTNA_DBG_Str(0, "\r\n"); 
    return ISP_IF_AE_GetShutterBase()>>n_shift;
  
  } 
   
}

static void  GetScalerZoomRange(MMP_USHORT who,MMP_USHORT *target_min,MMP_USHORT *target_max,MMPF_SCALER_ZOOM_INFO  *scalerInfo,PCAM_USB_ZOOM  *zoomInfo)
{
#if SCALER_ZOOM_LEVEL!=BEST_LEVEL
    MMP_USHORT target_val;
    if(who==0) {
        *target_max = scalerInfo->usBaseN   + (scalerInfo->grabCtl.usScaleM - scalerInfo->usBaseN)   * zoomInfo->RangeMax / zoomInfo->RangeStep ;
    }
#if USB_LYNC_TEST_MODE
    else if(who==1) {
        *target_max = scalerInfo->usBaseXN + (scalerInfo->grabCtl.usScaleXM - scalerInfo->usBaseXN) * zoomInfo->RangeMax / zoomInfo->RangeStep ;
    
    }
    else if(who==2) {
        *target_max = scalerInfo->usBaseYN + (scalerInfo->grabCtl.usScaleXM - scalerInfo->usBaseYN) * zoomInfo->RangeMax / zoomInfo->RangeStep ;
    }
#endif
    
    if(*target_max > scalerInfo->grabCtl.usScaleM) {
        *target_max = scalerInfo->grabCtl.usScaleM ;
    }
    
    if(*target_min > scalerInfo->grabCtl.usScaleM) {
        *target_min = *target_max ;
    }
    if(*target_min > *target_max) {
        zoomInfo->Dir = 1 ;
        target_val = *target_max ;
        *target_max = *target_min;
        *target_min = target_val ;
    } else {
        zoomInfo->Dir = 0 ;
    }
#else
    if(who==0) {
        *target_min = scalerInfo->usRangeMinN ;
        *target_max = scalerInfo->usRangeMaxN ;
    } 
    else if (who==1) {
        *target_min = scalerInfo->usRangeMinXN ;
        *target_max = scalerInfo->usRangeMaxXN ;
    }
    else if (who==2) {
        *target_min = scalerInfo->usRangeMinYN ;
        *target_max = scalerInfo->usRangeMaxYN ;
    
    }
#endif
    
}
#if SCALER_ZOOM_LEVEL==BEST_LEVEL
MMP_BOOL  GetScalerZoomGrab(MMPF_SCALER_PATH pipe,MMP_USHORT who,MMPF_SCALER_ZOOM_INFO  *scalerInfo,PCAM_USB_ZOOM  *zoomInfo,MMP_BOOL lossy_up)
{
#define RATIO_1_1   CAL_BASE
#define Regulus_TRY	(1)
#define NEW_BASE	(1440)

    MMP_USHORT cur_item ;
    MMP_ULONG  target_val;
    MMPF_SCALER_FIT_RANGE fit_range ;
    MMPF_SCALER_GRABCONTROL grab_ctl,grab_ctl_in;
    RATIO_TBL *new_tbl,*old_tbl;
    RATIO_TBL *base_tbl = MMPF_Scaler_GetBaseRatioTbl(pipe) ;

	#ifdef Regulus_TRY
    MMP_ULONG  targetTBL[5][5]=
	{{2560, 1440, 1920, 1080, 972},
	 {2400, 1350, 1920, 1080, 1036},
	 {2240, 1260, 1920, 1080, 1110},
	 {2080, 1170, 1920, 1080, 1196},
	 {1920, 1080, 1920, 1080, 1296}
	};
	
	MMP_ULONG  RatioTBL[5] =
	{
		100,
		125,
		150,
		175,
		200
	};

	MMP_ULONG  RatioTBL_4_3[5] =
	{
		100,
		175,
		200,
		250,
		300
	};
	RES_TYPE_CFG *cur_res;
	PCAM_USB_VIDEO_RES res ;
	#endif
    
    cur_item = MMPF_Scaler_GetRatioCurItem(pipe);
    old_tbl = MMPF_Scaler_GetCurRatioTbl(pipe);
    if(who==0) {
        MMP_USHORT x_scale,y_scale,rangeMax ;
        /* For win-8 cancel
        if(zoomInfo->RangeMax == zoomInfo->RangeMin) {
            dbg_printf(3,"Task:Zoom(%d,%d)\r\n",zoomInfo->RangeMin,zoomInfo->RangeMax);
            return MMP_TRUE;
        }
        */
        rangeMax = zoomInfo->RangeMax;
		
try_again:
	    #ifdef Regulus_TRY
	    res = GetFirstStreamingRes();
		cur_res = GetResCfg(res);
		if((cur_res->res_w == 1920) && (cur_res->res_h == 1080))
		{
			target_val = targetTBL[(rangeMax-100) / 10][4]; 
    	    fit_range.usInWidth   = targetTBL[(rangeMax-100) / 10][0];
        	fit_range.usInHeight  = targetTBL[(rangeMax-100) / 10][1];
        	fit_range.usOutWidth = targetTBL[(rangeMax-100) / 10][2] ;
        	fit_range.usOutHeight = targetTBL[(rangeMax-100) / 10][3] ;		
		}else if((cur_res->res_w == 640) && (cur_res->res_h == 480))
		{
    	    fit_range.usInWidth   = scalerInfo->usSensorWidth * 100 / RatioTBL_4_3[(rangeMax-100) / 10];
    	    fit_range.usInWidth   = (fit_range.usInWidth + 15 ) & (-16);// boundary at 8 or 16
        	fit_range.usInHeight  = scalerInfo->usSensorHeight * 100 / RatioTBL_4_3[(rangeMax-100) / 10];
        	fit_range.usInHeight  =  (fit_range.usInHeight + 15) & (-16); // boundary at 8 or 16
			target_val = cur_res->res_w * RATIO_1_1 / fit_range.usInWidth;
			target_val = ((target_val + 1) >> 1) << 1;
        	fit_range.usOutWidth = cur_res->res_w;
        	fit_range.usOutHeight = cur_res->res_h;			
			dbg_printf(0,"4:3 Scale in: W:%d H:%d RatioTBL:%d \r\n", fit_range.usInWidth, fit_range.usInHeight, RatioTBL_4_3[(rangeMax-100) / 10]);
		}else
		{
    	    fit_range.usInWidth   = scalerInfo->usSensorWidth * 100 / RatioTBL[(rangeMax-100) / 10];
    	    fit_range.usInWidth   = (fit_range.usInWidth + 15 ) & (-16);// boundary at 8 or 16
        	fit_range.usInHeight  = scalerInfo->usSensorHeight * 100 / RatioTBL[(rangeMax-100) / 10];
        	fit_range.usInHeight  =  (fit_range.usInHeight + 15) & (-16); // boundary at 8 or 16
			target_val = cur_res->res_w * RATIO_1_1 / fit_range.usInWidth;
			target_val = ((target_val + 1) >> 1) << 1;
        	fit_range.usOutWidth = cur_res->res_w;
        	fit_range.usOutHeight = cur_res->res_h;
			dbg_printf(0,"16:9 Scale in: W:%d H:%d RatioTBL:%d \r\n", fit_range.usInWidth, fit_range.usInHeight, RatioTBL[(rangeMax-100) / 10]);
		}
		#else
        target_val =( base_tbl->ratio * rangeMax) / 100 ;
		#endif 
        
        dbg_printf(3,"[%d]target_val:%d,ratio:%d,rangemax:%d\r\n",pipe,target_val,base_tbl->ratio,zoomInfo->RangeMax);
        //dbg_printf(3,"input (w,h) : (%d,%d)\r\n",scalerInfo->usInputWidth,scalerInfo->usInputHeight);
        if(lossy_up==MMP_FALSE) {
            if(target_val > RATIO_1_1) {
                return MMP_FALSE ;
            }
        }
        #if 0 //
        //fit_range.fitmode = MMPF_SCALER_FITMODE_OUT ;
        //fit_range.usFitResol = new_tbl->m ;
	#if 0	
        switch(zoomInfo->RangeMax)
        {
        	case 100:	scalerInfo->usInputWidth = 2544;
					scalerInfo->usInputHeight = 1432;
					break;

        	case 108:	scalerInfo->usInputWidth = 2432;
					scalerInfo->usInputHeight = 1368;
					break;

        	case 116:	scalerInfo->usInputWidth = 2304;
					scalerInfo->usInputHeight = 1304;
					break;
        	case 124:	scalerInfo->usInputWidth = 2176;
					scalerInfo->usInputHeight = 1224;
					break;
        	case 132:	scalerInfo->usInputWidth = 2048;
					scalerInfo->usInputHeight = 1160;
					break;
        	case 140:	scalerInfo->usInputWidth = 1920;
					scalerInfo->usInputHeight = 1080;
					break;
				
        }
		scalerInfo->usSensorWidth = scalerInfo->usInputWidth;
		scalerInfo->usSensorHeight = scalerInfo->usInputHeight;
	#endif	
        dbg_printf(3,"scalerInfo %d  ,%d  \r\n",scalerInfo->usInputWidth, scalerInfo->usInputHeight);
        fit_range.usInWidth   = scalerInfo->usInputWidth;
        fit_range.usInHeight  = scalerInfo->usInputHeight;
        fit_range.usOutWidth = scalerInfo->grabCtl.usEndX - scalerInfo->grabCtl.usStartX + 1 ;
        fit_range.usOutHeight = scalerInfo->grabCtl.usEndY - scalerInfo->grabCtl.usStartY + 1 ;
        #endif

                
        new_tbl = MMPF_Scaler_GetRatioTblByRatio(pipe,target_val,MMP_TRUE);
        if(cur_item > MMPF_Scaler_GetRatioCurItem(pipe) ) {
            zoomInfo->Dir = 1 ; // <- zoom out
        }
        else {
            zoomInfo->Dir = 0; // -> zoom in
        }
        fit_range.fitmode = MMPF_SCALER_FITMODE_OUT ;
        fit_range.usFitResol = new_tbl->m ;
        #if 0
        fit_range.usInWidth   = scalerInfo->usInputWidth;
        fit_range.usInHeight  = scalerInfo->usInputHeight;
        fit_range.usOutWidth = scalerInfo->grabCtl.usEndX - scalerInfo->grabCtl.usStartX + 1 ;
        fit_range.usOutHeight = scalerInfo->grabCtl.usEndY - scalerInfo->grabCtl.usStartY + 1 ;
        #endif
        if(new_tbl->n > new_tbl->m) {
         //   dbg_printf(3,"Fit (%d/%d) - (%d,%d) -> (%d,%d)\r\n",new_tbl->n,fit_range.usFitResol,fit_range.usInWidth ,fit_range.usInHeight,fit_range.usOutWidth,fit_range.usOutHeight);
            fit_range.usInWidth  = ((fit_range.usOutWidth * new_tbl->m / new_tbl->n + 1) >> 1) << 1;
            fit_range.usInHeight = ((fit_range.usOutHeight * new_tbl->m / new_tbl->n + 1) >> 1) << 1;
            scalerInfo->usInputWidth  = fit_range.usInWidth ;
            scalerInfo->usInputHeight = fit_range.usInHeight;
        } else {
            scalerInfo->usInputWidth = scalerInfo->usSensorWidth;
            scalerInfo->usInputHeight =scalerInfo->usSensorHeight;
            fit_range.usInWidth   = scalerInfo->usInputWidth;
            fit_range.usInHeight  = scalerInfo->usInputHeight;
        }
        
        
        dbg_printf(0,"N/M(%d/%d) - (%d,%d) -> (%d,%d)\r\n",new_tbl->n,fit_range.usFitResol,fit_range.usInWidth ,fit_range.usInHeight,fit_range.usOutWidth,fit_range.usOutHeight);
        //MMPF_Scaler_GetBestFitScale( &fit_range,&grab_ctl);
        grab_ctl = scalerInfo->grabCtl ;
        if(MMPF_Scaler_GetBestGrabRange(pipe,MMPF_Scaler_GetRatioCurItem(pipe),&fit_range,&grab_ctl,&grab_ctl_in,0)==MMP_SCALER_ERR_N) {
            dbg_printf(3,"--try new range:%d\r\n",rangeMax );
            rangeMax++;
            goto try_again;
        }
        //dbg_printf(3,"\r\nuvc.zoom : %d -> %d\r\n",zoomInfo->RangeMin,zoomInfo->RangeMax);
        //dbg_printf(3,"ratio : %d\r\n",old_tbl->ratio,new_tbl->ratio);
        scalerInfo->grabCtl = grab_ctl ;
        scalerInfo->grabCtlIn = grab_ctl_in;
        scalerInfo->usBaseN = grab_ctl.usScaleN ;
        scalerInfo->usRangeMinN = grab_ctl.usScaleN ;
        scalerInfo->usRangeMaxN = grab_ctl.usScaleN ;
        if(grab_ctl.usScaleN) {
            dbg_printf(3,"Try New(N/M) : (%d/%d)\r\n",scalerInfo->grabCtl.usScaleN,scalerInfo->grabCtl.usScaleM);
           /*
            dbg_printf(3,"new.grab(%d,%d)( %d,%d,%d,%d )\r\n",scalerInfo->grabCtl.usScaleN,scalerInfo->grabCtl.usScaleM,
                                                              scalerInfo->grabCtl.usStartX,scalerInfo->grabCtl.usEndX,
                                                              scalerInfo->grabCtl.usStartY,scalerInfo->grabCtl.usEndY);
            dbg_printf(3,"new.grabin(%d,%d)( %d,%d,%d,%d )\r\n",scalerInfo->grabCtlIn.usScaleN,scalerInfo->grabCtlIn.usScaleM,
                                                              scalerInfo->grabCtlIn.usStartX,scalerInfo->grabCtlIn.usEndX,
                                                              scalerInfo->grabCtlIn.usStartY,scalerInfo->grabCtlIn.usEndY);
            */
        } 
        else {
            scalerInfo->usBaseXN = grab_ctl.usScaleXN ;
            scalerInfo->usBaseYN = grab_ctl.usScaleYN ;
            scalerInfo->usRangeMinXN = grab_ctl.usScaleXN ;
            scalerInfo->usRangeMaxXN = grab_ctl.usScaleXN ;
            scalerInfo->usRangeMinYN = grab_ctl.usScaleYN ;
            scalerInfo->usRangeMaxYN = grab_ctl.usScaleYN ;
           // dbg_printf(3,"new.grabx(%d/%d)-(%d,%d)\r\n",grab_ctl.usScaleXN,grab_ctl.usScaleXM,grab_ctl.usStartX,grab_ctl.usEndX);
           // dbg_printf(3,"new.graby(%d/%d)-(%d,%d)\r\n",grab_ctl.usScaleYN,grab_ctl.usScaleYM,grab_ctl.usStartY,grab_ctl.usEndY);
        
        }                                                  
        //dbg_printf(3,"New Ratio #%d,N/M :%d/%d\r\n",new_tbl->ratio,new_tbl->n,new_tbl->m ); 
        
        
    }
    return MMP_TRUE; 
}
#endif

#if AUDEXT_DAC==WM8737
/* Gason@20100730, Separate WM8737 function to three function, init , power up and power down*/
//static MMPF_I2CM_ATTRIBUTE m_WM8737_I2cmAttribute = {MMPF_I2CM_ID_0, 0x1A, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE, 0, 0, 0, MMPF_I2CM_SPEED_HW_400K, NULL, NULL};
static void MMPF_InitWM8737(void)
{
  AITPS_PAD   pPAD  = AITC_BASE_PAD;
  AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
  //volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) (0x80000000);
  //REG_BASE_B[0x9D06] = 0x24; //WM8737 power enable, pull high 
    //RTNA_WAIT_MS(1);
    #if (CHIP==P_V2)
  pGBL->GBL_CLK_EN |= (GBL_CLK_I2CM|GBL_CLK_VI);
  #endif
    #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
  pGBL->GBL_CLK_DIS0 &= ~(GBL_CLK_VI_DIS);
  pGBL->GBL_CLK_DIS1 &= ~(GBL_CLK_I2C_DIS); 
    #endif
    
  MMPF_I2cm_Initialize(&m_WM8737_I2cmAttribute);  
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x0F<<1, 00);
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x15);//0x15
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x14,0x0B);
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x0E,0x41);
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x01,0xC3);
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x03,0xC3); 
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x04,0x77);  
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x06,0x77);
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x0D,0xFF);
  
  #if (CHIP==P_V2)
  pGBL->GBL_CLK_EN &= ~(GBL_CLK_I2CM|GBL_CLK_VI);
  #endif
  
  MMPF_OS_Sleep_MS(1);
}

static void MMPF_PowerUpWM8737(MMP_USHORT sampling_rate)
{
  //AITPS_PAD   pPAD  = AITC_BASE_PAD;
  
  AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
  AITPS_GBL   pGBL = AITC_BASE_GBL;
    #if (CHIP==P_V2)
  clock =  pGBL->GBL_CLK_EN ;
  pGBL->GBL_CLK_EN |= (GBL_CLK_I2CM|GBL_CLK_VI);
  #endif
  #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
  pGBL->GBL_CLK_DIS0 &= ~(GBL_CLK_VI_DIS);
  pGBL->GBL_CLK_DIS1 &= ~(GBL_CLK_I2C_DIS);
  #endif
  
    RTNA_WAIT_MS(5);
  
  MMPF_I2cm_Initialize(&m_WM8737_I2cmAttribute);  
  if(sampling_rate == 8000)
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x09);
  else if(sampling_rate == 16000)
      MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x15);
    else if(sampling_rate == 24000)  
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x39);
  else if(sampling_rate == 32000)
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x19);
  else// 48K
    MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x08<<1+ 0,0x01);
  
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x0D, 0xFF);
  #if (CHIP==P_V2)
  pGBL->GBL_CLK_EN  = clock;
  #endif
  
  RTNA_WAIT_MS(5);
}

static void MMPF_PowerDownWM8737(void)
{
  AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
  AITPS_GBL   pGBL = AITC_BASE_GBL;
  #if (CHIP==P_V2)
    MMP_ULONG clock=0;
  clock =  pGBL->GBL_CLK_EN ;
    // sensor PD_12A disable
  pGBL->GBL_CLK_EN |= (GBL_CLK_I2CM|GBL_CLK_VI);
  #endif
  #if (CHIP==VSN_V2) || (CHIP==VSN_V3)
  pGBL->GBL_CLK_DIS0 &= ~(GBL_CLK_VI_DIS);
  pGBL->GBL_CLK_DIS1 &= ~(GBL_CLK_I2C_DIS);
  #endif
  MMPF_I2cm_Initialize(&m_WM8737_I2cmAttribute);  
  MMPF_I2cm_WriteReg(&m_WM8737_I2cmAttribute,0x0D, 0x80);
  #if (CHIP==P_V2)
  pGBL->GBL_CLK_EN  = clock;
  #endif
  
   // MMPF_OS_Sleep_MS(1);
}


#endif


#if (AUDEXT_DAC == DA7211)
int DA7211_Initial_Success = 0;
MMP_ULONG glDA7211SampleRate;
MMP_BOOL DA7211_powerup_success = MMP_FALSE;

void DA7211_Write(MMP_USHORT RegAddr, MMP_USHORT RegData)
{
  MMPF_I2cm_WriteReg(&gI2cmAttribute_DA7211, RegAddr, RegData);
}

MMP_UBYTE GetSampleRate(void)
{ 
  switch(glDA7211SampleRate)
  {
    case 8000:
      return DA7211_FS_8KHZ;
      break;
    case 11025:
      return DA7211_FS_11_025KHZ;
      break;
    case 12000:
      return DA7211_FS_12KHZ;
      break;
    case 16000:
      RTNA_DBG_Str(0, "da7211 set sample rate 16k\r\n");
      return DA7211_FS_16KHZ;
      break;
    case 22050:
      return DA7211_FS_22_05KHZ;
      break;
    case 24000:
      return DA7211_FS_24KHZ;
      break;
    case 32000:
      return DA7211_FS_32KHZ;
      break;
    case 44100:
      return DA7211_FS_44_1KHZ;
      break;
    case 48000:
      return DA7211_FS_48KHZ;
      break;
    case 81000:
      return DA7211_FS_88_1KHZ;
      break;
    case 96000:
      return DA7211_FS_96KHZ;
      break;
  }
  RTNA_DBG_Str(3, "Error sample rate\r\n");
  return 0;
}


void MMPF_DA7211_SetSamplingRate(MMP_ULONG ulSamplerate)
{
  glDA7211SampleRate = ulSamplerate;
  DA7211_Write(DA7211_PLL, (DA7211_PLL_DIS | DA7211_MCLK_DET_DIS | GetSampleRate()));
    return;
}

void MMPF_DA7211_InitExtMicIn(void)
{
    RTNA_DBG_Str(0, "InitExtMicIn \r\n");

  DA7211_Write(DA7211_SOFT_MUTE    ,DA7211_SOFTMUTE_START | DA7211_RAMP_RAMPING | DA7211_MUTE_RATE_1_SAMPLE);
  DA7211_Write(DA7211_CONTROL      ,DA7211_BIAS_EN | DA7211_REG_EN);
  
  DA7211_Write(DA7211_DAI_CFG1  ,DA7211_DAI_MODE_MASTER | DA7211_DAI_FRAME_2X_DAI_WORD | DA7211_DAI_WORD_16BITS);
  DA7211_Write(DA7211_PLL_DIV3  ,DA7211_PLL_BYP_EN | DA7211_PLL_MCLK_RANGE_10_20);

  //DA7211_Write(DA7211_PLL          ,DA7211_PLL_DIS | DA7211_MCLK_DET_DIS | DA7211_FS_44_1KHZ);
  DA7211_Write(DA7211_PLL          ,DA7211_PLL_DIS | DA7211_MCLK_DET_DIS | DA7211_FS_16KHZ);//GetSampleRate());
  
  DA7211_Write(DA7211_MIC_L    ,DA7211_MIC_L_AMP_EN | DA7211_MIC_BIAS_EN | DA7211_MIC_BIAS_SEL_1_6 | 0x01);
  DA7211_Write(DA7211_INMIX_L    ,DA7211_IN_L_PGA_EN | DA7211_IN_L_MIC_L);
  DA7211_Write(DA7211_INMIX_R    ,DA7211_IN_L_PGA_EN | DA7211_IN_L_MIC_R);
  DA7211_Write(DA7211_IN_GAIN      ,0xFF);
  DA7211_Write(DA7211_ADC_HPF      ,DA7211_ADC_HPF_EN);
  DA7211_Write(DA7211_ADC      ,DA7211_ADC_R_CH_EN | DA7211_ADC_L_CH_EN);
  DA7211_Write(DA7211_ZEROX        ,0xFF);
  DA7211_Write(DA7211_DAI_CFG3     ,DA7211_DAI_EN | DA7211_DAI_OE_EN);
}

void MMPF_DA7211_PwUpExtMicIn(void)
{
    RTNA_DBG_Str(0, "PwUpExtMicIn \r\n");

  //OS_ENTER_CRITICAL();
  DA7211_Write(DA7211_STARTUP2     ,DA7211_DAC_STANDBY | DA7211_HP_STANDBY | DA7211_OUT1_STANDBY);
  DA7211_Write(DA7211_STARTUP3     ,DA7211_ADC_NORMAL | DA7211_AUX1_STANDBY | DA7211_MIC_NORMAL);
  DA7211_Write(DA7211_STARTUP1     ,DA7211_SC_MST_ACTIVE);
  RTNA_WAIT_MS(300);

  DA7211_Write(DA7211_SOFT_MUTE    ,DA7211_SOFTMUTE_DIS | DA7211_RAMP_RAMPING);
  //OS_EXIT_CRITICAL();
  DA7211_powerup_success = MMP_TRUE;

}

void MMPF_InitDA7211(void)
{
  
  RTNA_DBG_Str(0, "MMPF_InitDA7211: ");
  if(DA7211_Initial_Success == MMP_TRUE) // if CXNT initial succeed, then it does not need second time.
    return;
  else {
    //OS_ENTER_CRITICAL();
    MMPF_I2cm_Initialize(&gI2cmAttribute_DA7211);
    MMPF_DA7211_InitExtMicIn();
    //OS_EXIT_CRITICAL();
      DA7211_Initial_Success = MMP_TRUE;
  }
}

#if 0
static void MMPF_PowerUpDA7211(void)
{
    RTNA_DBG_Str(3, "PwUpExtDac \r\n");
    
  //OS_ENTER_CRITICAL();
  DA7211_Write(DA7211_STARTUP2      ,DA7211_DAC_NORMAL | DA7211_HP_NORMAL | DA7211_OUT1_STANDBY);
  DA7211_Write(DA7211_STARTUP3      ,DA7211_ADC_STANDBY | DA7211_AUX1_STANDBY | DA7211_MIC_STANDBY);
  DA7211_Write(DA7211_STARTUP1      ,DA7211_SC_MST_ACTIVE);
  RTNA_WAIT_MS(300);

  DA7211_Write(DA7211_SOFT_MUTE     ,DA7211_SOFTMUTE_DIS | DA7211_RAMP_RAMPING);
  DA7211_Write(DA7211_HP_L_VOL      ,0x30); //0db
  DA7211_Write(DA7211_HP_R_VOL      ,0x30); //0db
  //OS_EXIT_CRITICAL();
}
#endif

static void MMPF_PowerDownDA7211(void)
{
  if(DA7211_powerup_success == MMP_TRUE)
  {
      RTNA_DBG_Str(0, "MMPF_PowerDownDA7211(): PwDownExtMicIn \r\n");
    
    //OS_ENTER_CRITICAL();
    DA7211_Write(DA7211_STARTUP2   ,DA7211_DAC_STANDBY | DA7211_HP_STANDBY | DA7211_OUT1_STANDBY);
    DA7211_Write(DA7211_STARTUP3   ,DA7211_ADC_STANDBY | DA7211_AUX1_STANDBY | DA7211_MIC_STANDBY);
    DA7211_Write(DA7211_STARTUP1   ,DA7211_SC_MST_LOW_PWR);
    RTNA_WAIT_MS(300);
    
    DA7211_Write(DA7211_ZEROX    ,0x00);
    DA7211_Write(DA7211_MIC_L    ,DA7211_MIC_L_AMP_DIS | DA7211_MIC_BIAS_DIS);
    DA7211_Write(DA7211_MIC_R    ,DA7211_MIC_R_AMP_DIS | DA7211_MIC_BIAS_DIS);
    DA7211_Write(DA7211_ADC      ,DA7211_ADC_CH_DIS);
    DA7211_Write(DA7211_INMIX_L    ,DA7211_IN_L_PGA_DIS);
    DA7211_Write(DA7211_INMIX_R    ,DA7211_IN_R_PGA_DIS);
    DA7211_Write(DA7211_ADC_HPF    ,DA7211_ADC_HPF_DIS);
    DA7211_Write(DA7211_DAI_CFG3   ,DA7211_DAI_DIS | DA7211_DAI_OE_DIS);
    DA7211_Initial_Success = MMP_FALSE;
    //OS_EXIT_CRITICAL();
  }
  else return;

}

static void MMPF_DA7211_SetMute(MMP_UBYTE enable)
{
  AITPS_GBL   pGBL = AITC_BASE_GBL;
    MMP_USHORT clock0 = 0x0, clock1 = 0x0;
  MMP_USHORT  data = 0xFF;
  
  RTNA_DBG_Str(0, "MMPF_DA7211_SetMute \r\n");
  {
    clock0 = pGBL->GBL_CLK_DIS0;
    clock1 = pGBL->GBL_CLK_DIS1;
    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
    pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_I2C_DIS);
    
      MMPF_OS_Sleep_MS(1);
    
    MMPF_I2cm_Initialize(&gI2cmAttribute_DA7211); 
      MMPF_I2cm_ReadReg(&gI2cmAttribute_DA7211,DA7211_SOFT_MUTE, &data);


      if(enable){
        RTNA_DBG_Str(0, "MMPF_DA7211_SetMute(EN)\r\n"); 
          data = DA7211_SOFTMUTE_START;
      }
      else{
        RTNA_DBG_Str(0, "MMPF_DA7211_SetMute(DIS)\r\n"); 
          data =DA7211_SOFTMUTE_DIS;
      }
        
    MMPF_I2cm_WriteReg(&gI2cmAttribute_DA7211,DA7211_SOFT_MUTE, data);

    pGBL->GBL_CLK_DIS0 = clock0;
    pGBL->GBL_CLK_DIS1 = clock1;
  }
}

static void MMPF_DA7211VolumeControl(MMP_UBYTE volume)
{
  return;
}

#endif

