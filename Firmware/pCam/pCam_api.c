/*
pCAM APIs
*/

//#include "stdarg.h"
//#include "stdio.h"
#include "os_wrap.h"
#include "mmp_lib.h"
#include "lib_retina.h"
#include "pCam_api.h"
#include "pCam_msg.h"

MMP_USHORT MMPF_PCAM_SendMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
MMP_USHORT MMPF_PCAM_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
MMP_USHORT pcam_usb_sendmsg(MMP_USHORT pCamNonBlockingCall,MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
pcam_msg_t *MMPF_PCAM_MakeMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
/*
static MMP_USHORT pCamNonBlockingCall = 0;
*/
static PCAM_USB_INFO pCamUsbInfo =
{
0,// Add endpoint id
0,
PCAM_USB_VIDEO_FORMAT_MJPEG, // change default to MJPEG
PCAM_USB_HIGH_Q,
PCAM_USB_RESOL_1280x720,
PCAM_USB_DEBAND_60HZ,
PCAM_USB_AUDIO_FORMAT_AAC
} ;

static PCAM_AUDIO_CFG pCamAudioCfg = 
{
AUDIN_SAMPLERATE,
#if (MIC_SOURCE==MIC_IN_PATH_AFE)
// 1. From Line Jack : Using Single End input
// 2. From Diff MIC : Using Diff input
PCAM_AUDIO_AFE_DIFF, 
#elif (MIC_SOURCE==MIC_IN_PATH_I2S)
PCAM_AUDIO_IN_I2S,
#endif

PCAM_AUDIO_LINEIN_DUAL,
0,0
} ;



MMP_USHORT MMPF_PCAM_SendMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret,err ;
    pcam_msg_t *msg;

    msg = pcam_allocate_msg();
    if(!msg ){
        return PCAM_SYS_ERR ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = pcam_send_msg(msg,&err);
    /*destroy message*/
    pcam_free_msg( (void *)msg);
    if(ret) {
        return ret ;
    }
    return err;
}

MMP_USHORT MMPF_PCAM_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret=PCAM_ERROR_NONE ;
    pcam_msg_t *msg;
    msg = pcam_allocate_msg();
    if(!msg ){
        return PCAM_SYS_ERR ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = pcam_post_msg(msg);
    return ret;
}

MMP_USHORT MMPF_PCAM_PostOverWrMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
extern MMPF_OS_FLAGID PCAM_Flag ;
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pcam_msg_t *msg;

    msg = pcam_get_overwr_msg(msg_id,msg_sub_id);
    if(!msg ){
        return PCAM_SYS_ERR ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    err = MMPF_OS_SetFlags(PCAM_Flag, PCAM_FLAG_OVERWR_MSG, MMPF_OS_FLAG_SET);
    return err;
}


pcam_msg_t *MMPF_PCAM_MakeMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    pcam_msg_t *msg;
    msg = pcam_allocate_msg();
    if(!msg ){
        return (pcam_msg_t *)0 ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    return msg ; 
}

/*
Blocking call can't used in ISR
*/
void pcam_usb_set_nonblocking(MMP_USHORT nonblocking)
{
    //pCamNonBlockingCall = nonblocking;
}


MMP_USHORT pcam_usb_init(void)
{
    MMP_USHORT err ;
    err = pcam_usb_sendmsg(PCAM_BLOCKING,0,TASK_PCAM_PRIO,PCAM_MSG_USB_INIT,0,(void *)0);
    return err ;           
}


MMP_USHORT pcam_usb_preview(MMP_USHORT pCamNonBlockingCall,MMP_UBYTE ep_id)
{
    MMP_USHORT err ;
    pCamUsbInfo.pCamEpId = ep_id;
    if(pCamNonBlockingCall==PCAM_API) {

        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_PREVIEW_START,0,(void *)&pCamUsbInfo) ;
        if(msg) {
           // RTNA_DBG_Str(3,"[API] :pcam_usb_preview\r\n"); 
            PCAM_USB_PreviewStart(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }

    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_PREVIEW_START,0,(void *)&pCamUsbInfo);
    }
    return err ;           
}

MMP_USHORT pcam_usb_preview_stop(MMP_USHORT pCamNonBlockingCall)
{
    MMP_USHORT err ;
    if(pCamNonBlockingCall==PCAM_API) {
    
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_PREVIEW_STOP,0,(void *)0) ;
        if(msg) {
           // RTNA_DBG_Str(3,"[API] :pcam_usb_stop\r\n"); 
            PCAM_USB_PreviewStop(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
        
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_PREVIEW_STOP,0,(void *)0);
    }
    return err ;           
}

MMP_USHORT pcam_usb_exit(MMP_USHORT pCamNonBlockingCall,PCAM_USB_SENSOR_OFF_MODE poweroff_mode)
{
    MMP_USHORT err ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_EXIT,0,(void *)&poweroff_mode) ;
        if(msg) {
            RTNA_DBG_Str(3,"[API] :pcam_usb_exit\r\n"); 
            PCAM_USB_Exit(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }else{
        	RTNA_DBG_Str(3,"[API] :pcam_usb_exit make msg fail\r\n"); 
        }
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_EXIT,0,(void *)&poweroff_mode);
    }
    return err ;           
}

#if OMRON_FDTC_SUPPORT
MMP_USHORT pcam_usb_fdtc(void)
{
    MMP_USHORT err ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_FDTC_START,0,(void *)0) ;
        if(msg) {
            RTNA_DBG_Str(3,"[API] :pcam_usb_fdtc\r\n"); 
           // PCAM_USB_Exit(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }else{
        	RTNA_DBG_Str(3,"[API] :pcam_usb_fdtc make msg fail\r\n"); 
        }
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_FDTC_START,0,(void *)0);
    }
    return err ;           
}


#endif

MMP_USHORT pcam_usb_osd_update(void) 
{
    MMP_USHORT err ;
    err = pcam_usb_sendmsg(PCAM_BLOCKING,0,TASK_PCAM_PRIO,PCAM_MSG_USB_UPDATE_OSD,0,(void *)0);
    return err ;           
}

MMP_USHORT pcam_usb_set_attributes(MMP_USHORT pCamNonBlockingCall,MMP_UBYTE pcamIndex, void *pcamValue ) 
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
   // PCAM_USB_INFO   pCamInfo ; 
   //RTNA_DBG_Str(3,"pcam_usb_set_attributes\r\n");
	switch (pcamIndex)
	{
        case PCAM_USB_SETTING_H264_RES:
            pCamUsbInfo.pCamVideoFormat = PCAM_USB_VIDEO_FORMAT_H264 ;
            pCamUsbInfo.pCamVideoRes = *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_VIDEO_RES:
            pCamUsbInfo.pCamVideoRes = *(MMP_UBYTE *)pcamValue ;
            break;    
        case PCAM_USB_SETTING_VIDEO_FORMAT:
            pCamUsbInfo.pCamVideoFormat = *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_VIDEO_QUALITY:
            pCamUsbInfo.pCamVideoQuality= *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_DEBAND:
            pCamUsbInfo.pCamDebandMode =  *(MMP_UBYTE *)pcamValue ;
            break ;
        case PCAM_USB_SETTING_AUDIO_FORMAT:
            pCamUsbInfo.pCamAudioFormat= *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_SATURATION:
            pCamUsbInfo.pCamSaturation = *(MMP_USHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_CONTRAST:
            pCamUsbInfo.pCamContrast = *(MMP_USHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_BRIGHTNESS:
            pCamUsbInfo.pCamBrightness = *(MMP_SHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_HUE:
            pCamUsbInfo.pCamHue = *(MMP_SHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_GAMMA:
            pCamUsbInfo.pCamGamma = *(MMP_SHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_BACKLIGHT:
            pCamUsbInfo.pCamBacklight = *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_SHARPNESS:
            pCamUsbInfo.pCamSharpness = *(MMP_USHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_GAIN:
            pCamUsbInfo.pCamGain = *(MMP_USHORT *)pcamValue ;
            break;    
        case PCAM_USB_SETTING_WB:
            pCamUsbInfo.pCamWB = *(MMP_UBYTE *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_LENS:
            pCamUsbInfo.pCamLensPos = *(MMP_USHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_AF:
            pCamUsbInfo.pCamEnableAF = *(MMP_BOOL *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_AE:
            pCamUsbInfo.pCamEnableAE = *(MMP_BOOL *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_WB_TEMP:
            pCamUsbInfo.pCamWBTemp = *(MMP_USHORT *)pcamValue ;
            break ; 
        case PCAM_USB_SETTING_DIGZOOM:
            pCamUsbInfo.pCamDigZoom = *(PCAM_USB_ZOOM *)pcamValue ;
            break ;    
        case PCAM_USB_SETTING_DIGPAN:
            pCamUsbInfo.pCamDigPan = *(PCAM_USB_PANTILT *)pcamValue ;
            break; 
//sean@2011_01_24, add exposure time  
        case PCAM_USB_SETTING_EXPOSURE_TIME:
            pCamUsbInfo.pCamExpTime = *(MMP_ULONG *)pcamValue ;
            break;
        case PCAM_USB_SETTING_FORCE_BLACK:
            pCamUsbInfo.pCamForceBlack = *(MMP_BOOL *)pcamValue;           
            break;
        case PCAM_USB_SETTING_AE_PRIO:
            pCamUsbInfo.pCamEnableAEPrio = *(MMP_BOOL *)pcamValue; 
            break ;
        case PCAM_USB_SETTING_OSD_OP:
            pCamUsbInfo.pCamOsdOp = *(PCAM_USB_ZOOM *)pcamValue ;
            break;   
        case PCAM_USB_SETTING_EFFECT:
            //dbg_printf(3,"in : %x,out :%x\r\n",pcamValue,&pCamUsbInfo.pCamEffect);
            if(pcamValue==&pCamUsbInfo.pCamEffect) {
            
            } 
            else {
                if(!pCamUsbInfo.pCamEffect.times) {
                    pCamUsbInfo.pCamEffect = *(PCAM_USB_EFFECT_CTL *)pcamValue ;
                }
                else {
                    //dbg_printf(0,"--Effect action busy\r\n");
                    return PCAM_ERROR_NONE ;
                }
            }
            break ;
                    
        default:
            return err ;	
    }
    pCamUsbInfo.pCamSetIndex = pcamIndex ; // global variable, can't reference !
    
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_SET_ATTRS,pcamIndex,(void *)&pCamUsbInfo) ;
        if(msg) {
           // RTNA_DBG_Str(3,"[API] : Set Attr\r\n");
            PCAM_USB_SetAttrs(msg);   
            pcam_free_msg(msg);
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
        
    } else if( pCamNonBlockingCall == PCAM_OVERWR) {
        err = pcam_usb_sendoverwrmsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_SET_ATTRS,pcamIndex,(void *)&pCamUsbInfo) ;    
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_SET_ATTRS,pcamIndex,(void *)&pCamUsbInfo);
    }
    return err ;
}

int AlternateSamplingRate=0;
MMP_USHORT pcam_usb_audio_start(MMP_USHORT pCamNonBlockingCall)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pCamAudioCfg.pCamSampleRate = AlternateSamplingRate;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_START,0,(void *)&pCamAudioCfg) ;
        if(msg) {
            PCAM_USB_AudioStart(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_START,0,(void *)&pCamAudioCfg);
    }
    return err ;
}

MMP_USHORT pcam_usb_audio_stop(MMP_USHORT pCamNonBlockingCall)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_STOP,0,(void *)0) ;
        if(msg) {
            //RTNA_DBG_Str(3,"[API]:AudioStop\r\n");
            PCAM_USB_AudioStop(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_STOP,0,(void *)0);
    }
    
    return err ;
}

MMP_USHORT pcam_usb_take_picture(void) 
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    
    err = pcam_usb_sendmsg(PCAM_BLOCKING,0,TASK_PCAM_PRIO,PCAM_MSG_USB_CAPTURE,0,(void *)0);
    return err ;
}

MMP_USHORT pcam_usb_take_rawpicture(MMP_USHORT pCamNonBlockingCall,MMP_ULONG addr)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_CAPTURE_RAW,0,(void *)&addr) ;
        if(msg) {
            PCAM_USB_TakeRawPicture(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_CAPTURE_RAW,0,(void *)&addr);
    }    
    return 0;
}


MMP_USHORT pcam_usb_update_firmware(MMP_USHORT pCamNonBlockingCall,MMP_USHORT step)
{
static MMP_USHORT updateStep ;
    MMP_USHORT err = PCAM_ERROR_NONE ;
    updateStep = step ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_UPDATE_FW,0,(void *)&updateStep) ;
        if(msg) {
            PCAM_USB_UpdateFirmware(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_UPDATE_FW,0,(void *)&updateStep);
    }
    return err ;
}

MMP_USHORT pcam_usb_task_alive(MMP_USHORT heartbeat)
{
extern MMP_USHORT pCamTaskInited ;
    if(pCamTaskInited) {
        return  pcam_usb_sendmsg(PCAM_BLOCKING,0,TASK_PCAM_PRIO,PCAM_MSG_USB_CHECK_ALIVE,0,(void *)&heartbeat);
    }
    return 0;
}



PCAM_USB_INFO *pcam_get_info(void)
{
    return (PCAM_USB_INFO *)&pCamUsbInfo ;
}

MMP_USHORT pcam_usb_sendmsg(MMP_USHORT pCamNonBlockingCall,MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    
    if(pCamNonBlockingCall==PCAM_NONBLOCKING) {
    /*
        RTNA_DBG_Str(3,"<< pcam_usb_postmsg : ");
        RTNA_DBG_Long(3,msg_id);
        RTNA_DBG_Str(3,",");

        RTNA_DBG_Str(3,"attr:");
        RTNA_DBG_Short(3,msg_sub_id);
        RTNA_DBG_Str(3,">>\r\n");
    */
        err = MMPF_PCAM_PostMsg(src_id,dst_id,msg_id,msg_sub_id,(void *)data);
        //pCamNonBlockingCall = PCAM_BLOCKING;
    } else {
    /*
        RTNA_DBG_Str(3,">> pcam_usb_sendmsg : ");
        RTNA_DBG_Long(3,msg_id);
        RTNA_DBG_Str(3,",");
        
        RTNA_DBG_Str(3,"attr:");
        RTNA_DBG_Short(3,msg_sub_id);
        RTNA_DBG_Str(3,">>\r\n");
    */
        err =MMPF_PCAM_SendMsg(src_id,dst_id,msg_id,msg_sub_id,(void *)data);
    }
    return err ;
}

MMP_USHORT pcam_usb_sendoverwrmsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    err = MMPF_PCAM_PostOverWrMsg(src_id,dst_id,msg_id,msg_sub_id,(void *)data);
    return err ;
}

MMP_USHORT pcam_usb_set_samplerate(MMP_USHORT pCamNonBlockingCall,MMP_ULONG samplerate)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    if(AlternateSamplingRate==samplerate) // gason@add.
    	return err;
    
    pCamAudioCfg.pCamSampleRate = (MMP_USHORT)samplerate ;
    
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE,0,(void *)&pCamAudioCfg) ;
        if(msg) {
            PCAM_USB_SetSamplingRate(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else if( pCamNonBlockingCall == PCAM_OVERWR) {
        err = pcam_usb_sendoverwrmsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE,0,(void *)&pCamAudioCfg) ;
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE,0,(void *)&pCamAudioCfg);
    }
    return err ;
}


MMP_USHORT pcam_usb_set_mute(MMP_USHORT pCamNonBlockingCall,MMP_BOOL mute)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pCamAudioCfg.pCamMute = mute ;
    
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_MUTE,0,(void *)&pCamAudioCfg) ;
        if(msg) {
            PCAM_USB_SetMute(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else if( pCamNonBlockingCall == PCAM_OVERWR) {
        err = pcam_usb_sendoverwrmsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_MUTE,0,(void *)&pCamAudioCfg) ;
    } else {
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_MUTE,0,(void *)&pCamAudioCfg);
    }
    return err ;
}

MMP_USHORT pcam_usb_set_volume(MMP_USHORT pCamNonBlockingCall,MMP_SHORT vol)
{
    MMP_USHORT err = PCAM_ERROR_NONE ;
    pCamAudioCfg.pCamVolume = vol ;
    if(pCamNonBlockingCall==PCAM_API) {
        pcam_msg_t *msg = MMPF_PCAM_MakeMsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_VOL,0,(void *)&pCamAudioCfg) ;
        if(msg) {
            PCAM_USB_SetVolume(msg);
            pcam_free_msg(msg); 
            //pCamNonBlockingCall = PCAM_BLOCKING;   
        }
    } else if( pCamNonBlockingCall == PCAM_OVERWR) {
        err = pcam_usb_sendoverwrmsg(0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_VOL,0,(void *)&pCamAudioCfg) ;
    } else {       
        err = pcam_usb_sendmsg(pCamNonBlockingCall,0,TASK_PCAM_PRIO,PCAM_MSG_USB_AUDIO_VOL,0,(void *)&pCamAudioCfg);
    }
    return err ;
}

