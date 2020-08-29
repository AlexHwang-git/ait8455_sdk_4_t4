#ifndef _PCAM_MSG_H
#define _PCAM_MSG_H
#include "os_wrap.h"
#include "mmpf_msg.h"

#define PCAM_FLAG_MSG           (0x00000001) // The message queue at task queue
#define PCAM_FLAG_OVERWR_MSG    (0x00000002) // The message's data block would be overwrite by next request. 
#define PCAM_FLAG_WATCHDOG      (0x00000004)
    
enum {
    PCAM_ERROR_NONE=0,
    /*system error*/
    PCAM_SYS_ERR,
    PCAM_SYS_TIMEOUT,
    /*pcam error*/
    PCAM_USB_INIT_ERR,
    PCAM_USB_PREVIEW_ERR,
    PCAM_USB_CAPTURE_ERR,
    PCAM_USB_ZOOM_ERR,
    /*misc*/
    PCAM_USB_FIRMWARE_ERR,
#if H264_SIMULCAST_EN
    PCAM_USB_H264_SWITCH_RT_MODE,
    PCAM_USB_H264_SWITCH_FR_MODE,
#endif    
    PCAM_ERROR_MAX
};


enum {
    /*Preview part */
    PCAM_MSG_USB_INIT = 0 ,
    PCAM_MSG_USB_PREVIEW_START  ,
    PCAM_MSG_USB_PREVIEW_STOP  ,
    PCAM_MSG_USB_UPDATE_OSD,
    PCAM_MSG_USB_SET_ATTRS ,
    PCAM_MSG_USB_CAPTURE,  
    PCAM_MSG_USB_EXIT ,
    PCAM_MSG_USB_RESTART_SENSOR, 
    /*Audio Part */
    PCAM_MSG_USB_AUDIO_START,
    PCAM_MSG_USB_AUDIO_SET_SAMPLINGRATE,
    PCAM_MSG_USB_AUDIO_STOP,
    PCAM_MSG_USB_AUDIO_MUTE,
    PCAM_MSG_USB_AUDIO_VOL,
    PCAM_MSG_USB_AUDIO_RESET,
    
    /*DFU part*/
    PCAM_MSG_USB_UPDATE_FW,
    /*Debug*/
    PCAM_MSG_USB_CHECK_ALIVE,
    /*Take Raw data*/
    PCAM_MSG_USB_CAPTURE_RAW,
    /* FDTC part */
    PCAM_MSG_FDTC_INIT,
    PCAM_MSG_FDTC_START,
    PCAM_MSG_FDTC_STOP,

    /* Others part */
    PCAM_MSG_OTHERS,
    
    PCAM_MSG_MAX_NUM
} ; 

typedef struct _msg_t pcam_msg_t;

MMP_USHORT pcam_init_msg_queue(void);
pcam_msg_t *pcam_allocate_msg( void );
void pcam_free_msg(void *msg);
MMP_USHORT pcam_send_msg(pcam_msg_t *msg,MMP_USHORT *ret_err);
MMP_USHORT pcam_post_msg(pcam_msg_t *msg);
pcam_msg_t *pcam_get_msg(void);
pcam_msg_t *pcam_get_overwr_msg(MMP_ULONG msg_id,MMP_ULONG msg_sub_id);
MMP_USHORT MMPF_USBCTL_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data);
#endif
