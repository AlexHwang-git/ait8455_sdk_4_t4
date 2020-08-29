#ifndef _MMPF_USBUVC_HW_H
#define _MMPF_USBUVC_HW_H
#include "mmp_lib.h"
void USB_WAIT_EP0_READY(void);
void USB_PUT_EP0_DATA(MMP_UBYTE req,MMP_USHORT data_len,MMP_UBYTE *data);
void USB_PUT_FIFO_DATA(MMP_UBYTE req,MMP_UBYTE ep,MMP_USHORT data_len ,MMP_UBYTE *data,MMP_UBYTE unit);
void USB_GET_FIFO_DATA(MMP_UBYTE ep,MMP_USHORT fifo_len,MMP_UBYTE *buf,MMP_USHORT buf_len) ;
void usb_vc_fifo_copy(MMP_UBYTE ep_id,MMP_ULONG src_addr,MMP_USHORT xsize);
MMP_USHORT usb_vc_get_sof_ticks(void);
#endif
