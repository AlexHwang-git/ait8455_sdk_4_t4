#ifndef _PCAM_USB_H
#define _PCAM_USB_H
#include "config_fw.h"
#include "includes_fw.h"
#include "mmpf_typedef.h"
#include "pcam_api.h"
MMP_BOOL   USB_IsPreviewActive(void);
MMP_USHORT USB_VideoPreviewStart(MMP_UBYTE ep_id,MMP_USHORT nonblocking);
MMP_USHORT USB_ForceH264IDRFrame(void);
MMP_USHORT USB_VideoPreviewStop(MMP_USHORT nonblocking);
MMP_USHORT USB_VideoPowerDown(MMP_USHORT nonblocking,PCAM_USB_SENSOR_OFF_MODE poweroff_mode);
MMP_USHORT USB_AudioPreviewStart(MMP_USHORT nonblocking);
MMP_USHORT USB_AudioPreviewStop(MMP_USHORT nonblocking);
MMP_ULONG  USB_GetExposureTime(void);
MMP_USHORT USB_SetExposureTime(MMP_USHORT nonblocking,MMP_ULONG val);
MMP_USHORT USB_SetSaturation(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_SetContrast(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_SetBrightness(MMP_USHORT nonblocking,MMP_SHORT val);
MMP_USHORT USB_SetHue(MMP_USHORT nonblocking,MMP_SHORT val);
MMP_USHORT USB_SetGamma(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_SetBacklight(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_SetSharpness(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_SetGain(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_GetGain(void);
MMP_USHORT USB_SetLensPosition(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_EnableAF(MMP_USHORT nonblocking,MMP_BOOL val);
MMP_USHORT USB_EnableAE(MMP_USHORT nonblocking,MMP_BOOL val);
MMP_USHORT USB_SetAWBMode(MMP_USHORT nonblocking,MMP_UBYTE val);
MMP_USHORT USB_SetAWBTemp(MMP_USHORT nonblocking,MMP_USHORT val);
MMP_USHORT USB_GetAWBTemp(void);
MMP_USHORT USB_SetPowerLineFreq(MMP_USHORT nonblocking,MMP_UBYTE val);
MMP_USHORT USB_SetH264Resolution(MMP_USHORT nonblocking,PCAM_USB_VIDEO_RES pCamRes);
MMP_USHORT USB_SetVideoFormat(MMP_USHORT nonblocking,PCAM_USB_VIDEO_FORMAT pCamVidFmt);
MMP_USHORT USB_SetVideoQuality(MMP_USHORT nonblocking,PCAM_USB_VIDEO_QUALITY pCamQ);
MMP_USHORT USB_SetVideoResolution(MMP_USHORT nonblocking,PCAM_USB_VIDEO_RES pCamRes);
void USB_LEDDisplay(MMP_UBYTE num, MMP_UBYTE status) ;
void USB_WriteI2C(MMP_USHORT addr, MMP_USHORT val,MMP_UBYTE mode);
MMP_USHORT USB_ReadI2C(MMP_USHORT addr,MMP_UBYTE mode);
MMP_USHORT USB_UpdateFirmware(MMP_USHORT nonblocking,MMP_USHORT step) ; 
MMP_USHORT USB_CheckTaskAlive(MMP_USHORT heartbeat);
MMP_USHORT USB_SetDigitalZoom(MMP_USHORT nonblocking,MMP_USHORT dir,MMP_USHORT range_min,MMP_USHORT range_max,MMP_USHORT range_step);
MMP_USHORT USB_SetDigitalPanTilt(MMP_USHORT nonblocking,MMP_LONG pan_min,MMP_LONG pan_max,MMP_LONG tilt_min,MMP_LONG tilt_max,MMP_USHORT total_step);
MMP_USHORT USB_TakeRawPicture(MMP_USHORT nonblocking,MMP_ULONG addr) ;
MMP_USHORT USB_AudioSetMute(MMP_USHORT nonblocking,MMP_BOOL mute) ;
MMP_USHORT USB_AudioSetVolume(MMP_USHORT nonblocking,MMP_SHORT vol);
MMP_USHORT USB_AudioSetSampleRate(MMP_USHORT nonblocking,MMP_ULONG sample_rate);
MMP_USHORT USB_SetAsyncVideoControl(MMP_UBYTE bOriginator,MMP_UBYTE bSelector,MMP_UBYTE bAttribute,MMP_UBYTE bValUnit);
PCAM_BAYER_INFO *USB_GetBayerRawInfo(void);
void USB_EnableBayerRawPreview(MMP_BOOL enable);
MMP_BOOL USB_IsBayerRawPreview(void);
void USB_EnableNotchFilter(MMP_BOOL enable);
void USB_SetEdgeByResolution(MMP_USHORT res);
MMP_USHORT USB_ForceBlackEnable(MMP_USHORT nonblocking,MMP_BOOL en);
MMP_USHORT USB_SetSimpleFocus(MMP_USHORT nonblocking,MMP_UBYTE val);

#if OMURON_FDTC_SUPPORT
MMP_USHORT USB_VideoFDTC_Start(MMP_USHORT nonblocking);
MMP_USHORT USB_VideoFDTC_Stop(MMP_USHORT nonblocking);
#endif
#if H264_SIMULCAST_EN
void USB_SetH264EncodeBufferMode(MMP_BOOL frame_mode);
void USB_RestartPreview(MMP_UBYTE ep);
#endif
MMP_USHORT USB_ScalingOSD(MMP_USHORT nonblocking,MMP_USHORT buf_id,MMP_USHORT ratio_m,MMP_USHORT ratio_n);
MMP_USHORT USB_SetPreviewEffect(MMP_USHORT nonblocking,PCAM_USB_PREVIEW_EFFECT effect,MMP_LONG val,MMP_USHORT on_ms,MMP_USHORT off_ms,MMP_USHORT times);

#endif
