//==============================================================================
//
//  File        : pCam_usb.c
//  Description : Called by UVC,UAC
//
//==============================================================================
#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_typedef.h"
#include "mmp_register.h"
#include "mmp_reg_gpio.h"
#include "mmp_reg_gbl.h"

#include "mmpf_usbuvc.h"
#include "pCam_usb.h"
#include "pCam_msg.h"
#include "mmps_3gprecd.h"
#include "pcam_handler.h"
#include "mmpf_i2cm.h"
#include "mmpf_usbcolor.h"

//==============================================================================
//
//                              Define
//
//==============================================================================
// ----------------------------------------------------
#include "mmpf_mp4venc.h"

 
MMP_ULONG glSkypeH264Hdr;
MMP_ULONG glPCCAM_VIDEO_BUF_ADDR = 0;
MMP_ULONG glPCCAM_VIDEO_BUF_MAX_SIZE = 0;
MMP_ULONG glUSB_UVC_DMA_EP1_BUF_ADDR[2] = { 0 , 0 };
MMP_ULONG glUSB_UVC_DMA_EP2_BUF_ADDR[2] = { 0 , 0 };
MMP_ULONG glUSB_UVC_ZERO_BUF_ADDR ;
MMP_UBYTE gsUSBXU_I2C_ID ;
MMP_BOOL  gbBayerRawPreviewEn = 0;
MMP_BOOL  gbNotchFilterEn = 1 ;

//==============================================================================
//
//                              External
//
//==============================================================================

//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_BOOL USB_IsPreviewActive(void)
{   
    MMP_BOOL en ;
    MMPS_3GPRECD_GetPreviewDisplay(&en) ;
    return en; 
}

MMP_USHORT USB_SetAsyncVideoControl(MMP_UBYTE bOriginator,MMP_UBYTE bSelector,MMP_UBYTE bAttribute,MMP_UBYTE bValUnit)
{
    PCAM_SetAsyncVideoControl(bOriginator, bSelector, bAttribute, bValUnit);    
}


/*
UVC/UAC -> pCam API
*/

// UVC preview start
MMP_USHORT USB_VideoPreviewStart(MMP_UBYTE ep_id,MMP_USHORT nonblocking)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    // sean@2010_08_24,no need 
    //pcam_usb_set_nonblocking(nonblocking);
    //pcam_usb_init() ; 
    pcam_usb_set_nonblocking(nonblocking);
    pcam_usb_preview(nonblocking,ep_id);
    return err ;
}


MMP_USHORT USB_ForceH264IDRFrame(void)
{
    MMPF_VIDENC_SetParameter(0, MMPF_VIDENC_ATTRIBUTE_FORCE_I, (void*)MMPF_VIDENC_PICCTL_IDR_RESYNC);
    return PCAM_ERROR_NONE;
}

// UVC preview stop
MMP_USHORT USB_VideoPreviewStop(MMP_USHORT nonblocking)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    // sean@2010_12_28, alyways stop bayer raw preview when stop
    USB_EnableBayerRawPreview(0);
    pcam_usb_set_nonblocking(nonblocking);
    err = pcam_usb_preview_stop(nonblocking);
    return err;
}

// UVC power down
MMP_USHORT USB_VideoPowerDown(MMP_USHORT nonblocking,PCAM_USB_SENSOR_OFF_MODE poweroff_mode)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_usb_set_nonblocking(nonblocking);
    err = pcam_usb_exit(nonblocking, poweroff_mode);
    return err;
}

// UAC audio start
MMP_USHORT USB_AudioPreviewStart(MMP_USHORT nonblocking)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_usb_set_nonblocking(nonblocking);
    err = pcam_usb_audio_start(nonblocking);
    return err ;
}

// UAC audio stop
MMP_USHORT USB_AudioPreviewStop(MMP_USHORT nonblocking)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_usb_set_nonblocking(nonblocking);
    err = pcam_usb_audio_stop(nonblocking);
    return err ;
}


MMP_USHORT USB_AudioSetSampleRate(MMP_USHORT nonblocking,MMP_ULONG sample_rate)
{
  //  RTNA_DBG_Str3("USB_AudioSetSampleRate\r\n");
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_samplerate(nonblocking,sample_rate);
}


// 1 : Mute , 0 : UnMute
MMP_USHORT USB_AudioSetMute(MMP_USHORT nonblocking,MMP_BOOL mute)
{
    //RTNA_DBG_Str3("USB_AudioSetMute\r\n");
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_mute(nonblocking,mute);
}

// vol : UAC unit
MMP_USHORT USB_AudioSetVolume(MMP_USHORT nonblocking,MMP_SHORT vol)
{
    //RTNA_DBG_Str3("USB_AudioSetVolume\r\n");
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_volume(nonblocking,vol);
    
}
// sean@2011_01_24 UVC set exposure time
MMP_USHORT USB_SetExposureTime(MMP_USHORT nonblocking,MMP_ULONG val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_EXPOSURE_TIME,&val);
}

MMP_ULONG USB_GetExposureTime(void)
{
    MMP_ULONG ev_time ; // 100uS base

    ev_time = ISP_IF_AE_GetShutter() * 10000 / ISP_IF_AE_GetShutterBase();
    //dbg_printf(1,"#Exposure Time:%d\r\n",ev_time );
    return ev_time ;
    
}
// UVC set saturation
MMP_USHORT USB_SetSaturation(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_SATURATION,&val);
}

// UVC set constrast
MMP_USHORT USB_SetContrast(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_CONTRAST,&val);
}

// UVC set brightness
MMP_USHORT USB_SetBrightness(MMP_USHORT nonblocking,MMP_SHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_BRIGHTNESS,&val);
}

// UVC set hue
MMP_USHORT USB_SetHue(MMP_USHORT nonblocking,MMP_SHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_HUE,&val);
}

// UVC set gamma
MMP_USHORT USB_SetGamma(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_GAMMA,&val);
}

// UVC set backlight
MMP_USHORT USB_SetBacklight(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_BACKLIGHT,&val);
}

// UVC set sharpness
MMP_USHORT USB_SetSharpness(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_SHARPNESS,&val);
}

// UVC set gain
MMP_USHORT USB_SetGain(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_GAIN,&val);
}

MMP_USHORT USB_GetGain(void)
{
extern MMP_ULONG  glCurSensorGain ;
    MMP_USHORT val ;
    val =  ( ( glCurSensorGain - 64 ) * GAIN_MAX ) / (511-64) ;  
    return val ;         
}

// UVC set lens position
MMP_USHORT USB_SetLensPosition(MMP_USHORT nonblocking,MMP_USHORT val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_LENS,&val);
}

// UVC auto focus
MMP_USHORT USB_EnableAF(MMP_USHORT nonblocking,MMP_BOOL val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_AF,&val);
}

MMP_USHORT USB_SetSimpleFocus(MMP_USHORT nonblocking,MMP_UBYTE val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_SF,&val);
}
// UVC enable AE
MMP_USHORT USB_EnableAE(MMP_USHORT nonblocking,MMP_BOOL val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_AE,&val);
}

MMP_USHORT USB_SetAEPriority(MMP_USHORT nonblocking,MMP_BOOL val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_AE_PRIO,&val);
}

// UVC set awb mode
MMP_USHORT USB_SetAWBMode(MMP_USHORT nonblocking,MMP_UBYTE val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_WB,&val);
}

// UVC set awb temp
MMP_USHORT USB_SetAWBTemp(MMP_USHORT nonblocking,MMP_USHORT  val)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_WB_TEMP,&val);
}

// UVC get awb temp, already translate to 0~10000
MMP_USHORT USB_GetAWBTemp(void)
{
    MMP_ULONG val = ISP_IF_AWB_GetColorTemp();
    dbg_printf(0,"wb temp : %d\r\n",val);
    return  val;//( (val -2800 )* 10000 ) / (7500-2800);
}
//
// UVC set digital zoom
//
MMP_USHORT USB_SetDigitalZoom(MMP_USHORT nonblocking,MMP_USHORT dir,MMP_USHORT range_min,MMP_USHORT range_max,MMP_USHORT range_step)
{
    PCAM_USB_ZOOM val ;
   // RTNA_DBG_Str(3,"USB_SetDigitalZoom\r\n");
    
    val.Dir = dir ;
    val.RangeMin = range_min ;
    val.RangeMax = range_max ;
    val.RangeStep = range_step;
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_DIGZOOM,&val);

}


//
// UVC set digital pan & Tilt
//
MMP_USHORT USB_SetDigitalPanTilt(MMP_USHORT nonblocking,MMP_LONG pan_min,MMP_LONG pan_max,MMP_LONG tilt_min,MMP_LONG tilt_max,MMP_USHORT total_step)
{
    PCAM_USB_PANTILT val ;
   // RTNA_DBG_Str(3,"USB_SetDigitalPanTilt\r\n");
    
    val.PanMin  = pan_min  ;
    val.PanMax  = pan_max;
    val.TiltMin = tilt_min  ;
    val.TiltMax = tilt_max;
    val.Steps = total_step ;
    
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_DIGPAN,&val);

}
// UVC set line freq ( 50/60 debanding )
MMP_USHORT USB_SetPowerLineFreq(MMP_USHORT nonblocking,MMP_UBYTE val)
{
    PCAM_USB_DEBAND deband = PCAM_USB_DEBAND_60HZ ;
    if(val==BAND_50HZ) {
       deband = PCAM_USB_DEBAND_50HZ ; 
    }   
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_DEBAND,&deband);
}

// UVC set H264(only) resolution
MMP_USHORT USB_SetH264Resolution(MMP_USHORT nonblocking,PCAM_USB_VIDEO_RES pCamRes)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;    
    MMP_BOOL IsPreviewEnable;
    MMP_UBYTE ep = usb_get_cur_image_ep(PIPE_1); // TBD
    //dbg_printf(3,"-->set h264 res : %d\r\n",pCamRes);
    pcam_usb_set_nonblocking(nonblocking);
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&IsPreviewEnable,STREAM_EP_H264);
    if(IsPreviewEnable==0) {
        err = pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_H264_RES,&pCamRes);
    }
    return err;
}

// UVC set video format
MMP_USHORT USB_SetVideoFormat(MMP_USHORT nonblocking,PCAM_USB_VIDEO_FORMAT pCamVidFmt)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;    
    MMP_BOOL IsPreviewEnable;

    pcam_usb_set_nonblocking(nonblocking);
    MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
    if(IsPreviewEnable==0) {
        err = pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_VIDEO_FORMAT,&pCamVidFmt);
    }
    return err;
}

// UVC set video quality
MMP_USHORT USB_SetVideoQuality(MMP_USHORT nonblocking,PCAM_USB_VIDEO_QUALITY pCamQ)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_VIDEO_QUALITY,&pCamQ);
}


// UVC set video resolution
MMP_USHORT USB_SetVideoResolution(MMP_USHORT nonblocking,PCAM_USB_VIDEO_RES pCamRes)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_VIDEO_RES,&pCamRes);
}

void USB_LEDDisplay(MMP_UBYTE num, MMP_UBYTE status)
{
    AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    if(status == LED_ON) {
        pGPIO->GPIO_DATA[0] |= (1 << num);
    } else {
        pGPIO->GPIO_DATA[0] &= ~(1 << num);
    }
}

MMP_USHORT USB_UpdateFirmware(MMP_USHORT nonblocking,MMP_USHORT step) 
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_update_firmware(nonblocking,step);

}

MMP_USHORT USB_CheckTaskAlive(MMP_USHORT heartbeat)
{
    pcam_usb_set_nonblocking(PCAM_NONBLOCKING);
    return pcam_usb_task_alive(heartbeat);
    
}

MMP_USHORT USB_TakeRawPicture(MMP_USHORT nonblocking,MMP_ULONG addr) 
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_take_rawpicture(nonblocking,addr);

}

void USB_SetEdgeByResolution(MMP_USHORT res)
{
#if (CHIP==P_V2)
    volatile MMP_USHORT *REG_BASE_W = (volatile MMP_USHORT *)0x80000000;
    MMP_USHORT edge ;
    RES_TYPE_CFG *cur_res = GetResCfg(res);
   // dbg_printf(3,"#Cur edge:%x\r\n",REG_BASE_W[0x1140>>1]);
    if( (cur_res->res_w <= 640) && (cur_res->res_h<=480) ) {
        edge = 0x00A0 ;
    } else {
        edge = 0x0200 ;
    }
    REG_BASE_W[0x1140>>1] = edge ; 
#endif
}


void USB_SetEdgeByResolutionByEp(MMP_UBYTE ep_id)
{
#if (CHIP==P_V2)
    volatile MMP_USHORT *REG_BASE_W = (volatile MMP_USHORT *)0x80000000;
    MMP_USHORT edge ;
    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe_by_epid(ep_id);
    MMP_USHORT res_w,res_h ;
    
    if(cur_pipe->pipe_en & PIPE0_EN) {
        res_w = cur_pipe->pipe0_w ;
        res_h = cur_pipe->pipe0_h ;
    } else {
        res_w = cur_pipe->pipe1_w ;
        res_h = cur_pipe->pipe1_h ;
    }
    
   // dbg_printf(3,"#Cur edge:%x\r\n",REG_BASE_W[0x1140>>1]);
    if( (res_w <= 640) && (res_h<=480) ) {
        edge = 0x00A0 ;
    } else {
        edge = 0x0200 ;
    }
    REG_BASE_W[0x1140>>1] = edge ; 
#endif    
   // dbg_printf(3,"#[%d,%d]Switch Edge to : %x\r\n", cur_res->res_w,cur_res->res_h,edge);
}

 
 
MMP_USHORT USB_ForceBlackEnable(MMP_USHORT nonblocking,MMP_BOOL en)
{
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_FORCE_BLACK,&en);
}

/*************************************************************************************************/
/* Write I2C Routines via UVC command                                                                     */
/*************************************************************************************************/
void USB_WriteI2C(MMP_USHORT addr, MMP_USHORT val,MMP_UBYTE mode)
{
	MMP_USHORT I2CClockDiv = 0;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    
	MMPF_I2CM_ATTRIBUTE uI2cmAttribute =  {MMPF_I2CM_ID_0, 0, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE, 0, 0, 0, MMPF_I2CM_SPEED_HW_250K, NULL, NULL};
#if (CHIP==P_V2)
    uI2cmAttribute.uI2cmSpeed = MMPF_I2CM_SPEED_HW_400K;
#endif
#if (CHIP==VSN_V2) || (CHIP==VSN_V3)
    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
    pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_I2C_DIS);//20110715@DongQ, for I2C read write at any time.
    MMPF_OS_Sleep_MS(1);
#endif
	I2CClockDiv = 200;//G0Clock/1000 * 512/380;
	uI2cmAttribute.ubSlaveAddr = gsUSBXU_I2C_ID;
#if (CHIP==P_V2)
	if((gsUSBXU_I2C_ID > 0) && (pGBL->GBL_CLK_EN & GBL_CLK_VI))
#endif
#if (CHIP==VSN_V2) || (CHIP==VSN_V3)	
	if((gsUSBXU_I2C_ID > 0) && ((pGBL->GBL_CLK_DIS0 & GBL_CLK_VI_DIS) == 0x0))
#endif		
    {
        if(mode==0) {//2A1D
            uI2cmAttribute.ubRegLen = 16 ;
            uI2cmAttribute.ubDataLen = 8 ;
        }
        else if(mode==1) { //1A1D
            uI2cmAttribute.ubRegLen = 16 ;
            uI2cmAttribute.ubDataLen = 8 ;
        }
        MMPF_I2cm_WriteReg(&uI2cmAttribute,addr, val);
	}
}

/*************************************************************************************************/
/* Read I2C Routines via UVC command                                                                     */
/*************************************************************************************************/
MMP_USHORT USB_ReadI2C(MMP_USHORT addr,MMP_UBYTE mode)
{
	MMP_USHORT I2CClockDiv = 0;
	MMP_USHORT ret = 0;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
	MMPF_I2CM_ATTRIBUTE uI2cmAttribute =  {MMPF_I2CM_ID_0, 0, 8, 8, 0, MMP_FALSE, MMP_FALSE, MMP_FALSE, MMP_FALSE, 0, 0, 0, MMPF_I2CM_SPEED_HW_250K, NULL, NULL};
#if (CHIP==P_V2)
    uI2cmAttribute.uI2cmSpeed = MMPF_I2CM_SPEED_HW_400K;
#endif
#if (CHIP==VSN_V2) || (CHIP==VSN_V3)
    pGBL->GBL_CLK_DIS0 &= (~GBL_CLK_VI_DIS);
    pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_I2C_DIS);//20110715@DongQ, for I2C read write at any time.
    MMPF_OS_Sleep_MS(3);
#endif
	I2CClockDiv = 200;//G0Clock/1000 * 512/380;
	uI2cmAttribute.ubSlaveAddr = gsUSBXU_I2C_ID;
	
#if (CHIP==P_V2)	
	if((gsUSBXU_I2C_ID > 0) && (pGBL->GBL_CLK_EN & GBL_CLK_VI))
#endif
#if (CHIP==VSN_V2) || (CHIP==VSN_V3)	
    if((gsUSBXU_I2C_ID > 0) && ((pGBL->GBL_CLK_DIS0 & GBL_CLK_VI_DIS) == 0x0))	
#endif   
    { 
        if(mode==0) {//2A1D
            uI2cmAttribute.ubRegLen = 16 ;
            uI2cmAttribute.ubDataLen = 8 ;
        }
        else if(mode==1) { //1A1D
            uI2cmAttribute.ubRegLen = 16 ;
            uI2cmAttribute.ubDataLen = 8 ;
        }
	   ret = MMPF_I2cm_ReadReg(&uI2cmAttribute,addr, &ret);

	}
	
	return ret;
}

PCAM_BAYER_INFO *USB_GetBayerRawInfo(void)
{
static PCAM_BAYER_INFO gsBayerRawInfo = 
{
#if BIND_SENSOR_MT9T002==1
    ( BAYER_10BPP | BAYER_PACKED | BAYER_RGBORDER(1) ),
    2304,1536
#endif
#if BIND_SENSOR_OV2710==1
    ( BAYER_10BPP | BAYER_PACKED | BAYER_RGBORDER(3) ),
    1928,1092
#endif
#if BIND_SENSOR_S5K5B3GX==1
    ( BAYER_10BPP | BAYER_PACKED | BAYER_RGBORDER(3) ),
    1928,1092
#endif
} ;
    return &gsBayerRawInfo ;    
}


#if OMRON_FDTC_SUPPORT
MMP_USHORT USB_VideoFDTC_Start(MMP_USHORT nonblocking)
{
	MMP_BOOL IsPreviewEnable;
	MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_usb_set_nonblocking(nonblocking);
    MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);    
    //pcam_usb_set_nonblocking(PCAM_NONBLOCKING);
    
	if(IsPreviewEnable==1) {
	    err = pcam_usb_fdtc();
	}    
   /* else{
		resetFDTCMemory();
		
    }*/
	return err;
}

#endif

//
// Using XU to call this function to enable Bayer Raw preview
// By YUY2 index
//
void USB_EnableBayerRawPreview(MMP_BOOL enable)
{
    gbBayerRawPreviewEn = enable;
    //VAR_B(3,gbBayerRawPreviewEn); // 0->3 for dynamic change res
}

MMP_BOOL USB_IsBayerRawPreview(void)
{
    return gbBayerRawPreviewEn ;
}

void USB_EnableNotchFilter(MMP_BOOL enable)
{
    gbNotchFilterEn = enable ;
    dbg_printf(1,"gbNotchFilterEn : %d\r\n",gbNotchFilterEn);
}

void USB_SetH264EncodeBufferMode(MMP_BOOL frame_mode)
{
#if H264_SIMULCAST_EN
    MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_H264_RT_FR_SWITCH,0,(void *)frame_mode);
#endif
}


void USB_RestartPreview(MMP_UBYTE ep)
{
#if H264_SIMULCAST_EN
    MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_RESTART_PREVIEW,0,(void *)ep);
#endif

}

void USB_RestartSensor(MMP_UBYTE ep)
{
    MMPF_SYS_PostMsg(1,TASK_SYS_PRIO,SYS_MSG_USB_RESTART_SENSOR,0,(void *)ep);

}


MMP_USHORT USB_ScalingOSD(MMP_USHORT nonblocking,MMP_USHORT buf_id,MMP_USHORT ratio_m,MMP_USHORT ratio_n)
{
    PCAM_USB_ZOOM val ;
    val.Dir  = buf_id  ;
    val.RangeStep  = 0;
    val.RangeMin = ratio_m  ;
    val.RangeMax = ratio_n;
    pcam_usb_set_nonblocking(nonblocking);
    return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_OSD_OP,(void *)&val);

}

PCAM_USB_EFFECT_CTL pCamEffectCtl ;
// WB = 3200
// SAturation = 50

MMP_USHORT USB_SetPreviewEffect(MMP_USHORT nonblocking,PCAM_USB_PREVIEW_EFFECT effect,MMP_LONG val,MMP_USHORT on_ms,MMP_USHORT off_ms,MMP_USHORT times)
{
extern MMP_UBYTE   gbAWB;
extern MMP_USHORT  gsSaturation;
extern MMP_USHORT  gsTemper ;

    if(times==0) {
        pCamEffectCtl.times = 0 ;
        return PCAM_ERROR_NONE;
    }
    /*
    if(pCamEffectCtl.times != 0) {
        dbg_printf(3,"Effecting,remaining:%d times\r\n",pCamEffectCtl.times );
        return PCAM_ERROR_NONE;
    }
    */
    pCamEffectCtl.times = times * 2 ;
    pCamEffectCtl.on_ms = on_ms ;
    pCamEffectCtl.off_ms = off_ms ;
    
    if(effect==WB_EFFECT) {
        pCamEffectCtl.effect   = effect ;
        pCamEffectCtl.orig_auto = gbAWB  ;
        if(pCamEffectCtl.orig_auto) {
            pCamEffectCtl.orig_val = USB_GetAWBTemp();
        } 
        else {
            pCamEffectCtl.orig_val = gsTemper ;
        }
        pCamEffectCtl.new_val = val ;
        USB_SetAWBMode(PCAM_API,MMP_FALSE);
        USB_SetAWBTemp(PCAM_API,val);
    }
    else if(effect==SATURATION_EFFECT) {
        pCamEffectCtl.effect   = effect ;
        pCamEffectCtl.orig_val = gsSaturation ;
        pCamEffectCtl.new_val = val ;
        USB_SetSaturation(PCAM_API,val);
    }
    else {
        pCamEffectCtl.times = 0 ;
    }
    if(pCamEffectCtl.times ) {
        return pcam_usb_set_attributes(nonblocking,PCAM_USB_SETTING_EFFECT,(void *)&pCamEffectCtl);   
    }
    return PCAM_ERROR_NONE ;
}