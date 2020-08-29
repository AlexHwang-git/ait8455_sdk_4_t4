#ifndef _MMPF_USBUVC_FDTC_H
#define _MMPF_USBUVC_FDTC_H
#include "mmp_lib.h"

void usb_uvc_init_fdtc(MMP_BOOL en);
void usb_uvc_close_fdtc(void);
MMP_BOOL usb_uvc_prepareframe_fdtc(MMP_ULONG y_frame_addr,MMP_ULONG size);
void usb_uvc_enable_fdtc(MMP_BOOL en);
MMP_BOOL usb_uvc_is_fdtc_on(void);
MMP_BOOL usb_uvc_is_fdtc_idle(void);
#endif