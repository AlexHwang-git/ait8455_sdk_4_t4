#ifndef _MMPF_USBUVC_VS_H
#define _MMPF_USBUVC_VS_H

#include "mmp_lib.h"
#include "mmpf_usbskypeh264.h"
#include "mmpf_timer.h"
#include "mmpf_usbpccam.h"

#define UVC_SEND_IMG_RET_CONTINUE 0x00
#define UVC_SEND_IMG_RET_END_FRAME 0x01

void usb_fill_payload_header(MMP_UBYTE eid,MMP_UBYTE *frame_ptr, MMP_ULONG framelength, MMP_ULONG frameseq,MMP_ULONG flag, MMP_ULONG timestamp, MMP_USHORT w, MMP_USHORT h,MMP_USHORT framerate, MMP_UBYTE ubPipe);

void usb_uvc_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate);
void usb_skype_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate);
void usb_frameh264_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate);

#if USB_UVC_SKYPE
MMP_UBYTE *usb_skype_fill_header(MMP_UBYTE *frame_hdr1,MMP_UBYTE *h264_hdr2,SkypeH264EndPointType ep_type);
#endif


MMP_UBYTE usb_vs_send_image(MMP_UBYTE *sti_mode,MMP_UBYTE ep_id);
MMP_BOOL  usb_vs_trigger_next_dma(MMP_UBYTE ep_id);
void usb_vs_next_packet(MMP_UBYTE ep_id);
void usb_vs_next_frame(MMP_UBYTE ep_id,STREAM_CFG *cur_pipe,MMP_UBYTE pipe_cfg_format);
MMP_BOOL usb_vs_send_packet(MMP_ULONG cur_pt,MMP_ULONG xsize,MMP_UBYTE ep_id); 
void usb_vs_send_repack_packet(MMP_ULONG cur_pt,MMP_USHORT xsize,MMP_UBYTE ep_id);
void usb_vs_fill_svc_repack_info(MMP_UBYTE *frame_ptr,void *repack_info) ;
MMP_BOOL  usb_vc_trigger_next_dma(MMP_UBYTE ep_id);

void usb_vs_initial_fps(MMP_USHORT fps);
void usb_vs_timer_open(MMP_USHORT fps,MMPF_TIMER_ID id);
void usb_vs_timer_close(MMPF_TIMER_ID id) ;
MMP_BOOL usb_vs_release_dmabuf(MMP_UBYTE ep_id,MMP_ULONG dma_buf);
MMP_ULONG usb_vs_request_dmabuf(MMP_UBYTE ep_id,STREAM_DMA_BLK *dma_blk);

void usb_vs_timer_enable(MMPF_TIMER_ID id,MMP_BOOL en);
void usb_vs_timer_1stenable(MMPF_TIMER_ID id);
void usb_vs_monitor_icon_dline(void);
MMP_ULONG usb_vs_get_rt_mode_ptr(STREAM_SESSION *ss);
void usb_vs_put_rt_mode_ptr(STREAM_SESSION *ss,MMP_ULONG size);
MMP_BOOL usb_vs_check_incomplete_tx(MMP_UBYTE ep_id);
void usb_uvc_fill_cbr_padding_bytes(MMP_UBYTE *frame_ptr,MMP_ULONG bytes) ;
MMP_ULONG usb_uvc_get_cbr_padding_bytes(MMP_UBYTE *frame_ptr) ;
void usb_vs_zero_cbr_padding_buffer(void);
MMP_ULONG usb_vs_get_cbr_padding_buffer(void);
#endif
