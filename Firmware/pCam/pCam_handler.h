#ifndef _PCAM_HANDLER_H
#define _PCAM_HANDLER_H
#include "os_wrap.h"
#include "mmp_lib.h"
#include "pcam_msg.h"
#include "pcam_api.h"
#include "mmps_3gprecd.h"
#include "mmpf_audio_ctl.h"

typedef struct _PCAM_USB_CTX {
    MMP_UBYTE  epId ;
    MMPS_3GPRECD_VIDEO_FORMAT videoFormat;
    MMP_USHORT videoQuality;
    MMP_USHORT videoRes;
    MMP_USHORT debandMode;
    MMPS_3GPRECD_AUDIO_FORMAT audioFormat;
    MMP_USHORT audioInPath;
    MMPF_AUDIO_LINEIN_CHANNEL lineInChannel ;
    
} PCAM_USB_CTX;


pcam_msg_t *MMPF_PCAM_GetMsg(void);


MMP_USHORT PCAM_USB_Init(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_RestartSensor(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_PreviewStart(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_PreviewStop(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_Exit(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_UpdateOSD(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_SetAttrs(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_TakePicture(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_AudioStart(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_AudioStop(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_UpdateFirmware(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_TaskAlive(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_TakeRawPicture(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_SetVolume(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_SetMute(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_SetSamplingRate(pcam_msg_t *msg);
MMP_USHORT PCAM_USB_Set_FPS(MMP_SHORT fps) ;
MMP_USHORT PCAM_USB_Set_FPSx10( MMP_BOOL fix_fps,MMP_SHORT minfps_x10,MMP_SHORT maxfps_x10);
MMP_USHORT PCAM_USB_Set_Black(MMP_BOOL en) ;
void PCAM_USB_SetGain_UAC2ADC(MMP_SHORT voldb,MMP_USHORT sr);
void MMPF_PCAM_WaitTaskIdle(MMP_UBYTE wait_flag);
void PCAM_USB_SetAudioGain(MMP_UBYTE pga_gain, MMP_UBYTE boostdb,MMP_UBYTE dig_gain);
MMP_USHORT PCAM_USB_Set_DefScalerClipRange(void);

#endif
