#ifndef _MMPF_USBUVC_EVENT_H
#define _MMPF_USBUVC_EVENT_H
#include "mmp_lib.h"
#include "mmpf_fctl.h"

void usb_uvc_init_preview_fctl_link(void);
MMPF_FCTL_LINK *usb_uvc_get_preview_fctl_link (MMP_UBYTE ubPipe);
void usb_uvc_start_preview(MMP_UBYTE ep_id,MMP_BOOL change_res);
void usb_uvc_stop_preview(MMP_UBYTE ep_id,MMP_BOOL change_res);
MMP_ULONG usb_vc_take_raw_picture(MMP_UBYTE raw_mode, MMP_ULONG srcAddr,MMP_ULONG dstAddr);

#endif