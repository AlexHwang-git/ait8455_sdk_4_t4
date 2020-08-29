#include "config_fw.h"
#include "mmp_register.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_dma.h"
#include "mmp_reg_usb.h"
#include "mmp_reg_icon.h"

#include "pcam_api.h"
#include "mmpf_usb_h264.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbuvc_vs.h"
#include "text_ctl.h"
#include "lib_retina.h"
#include "mmpf_h264enc.h"
#include "mmpf_usbuvch264.h"
#include "mmpf_usbuvc15h264.h"



static MMP_ULONG usb_vs_map_ext_flag(MMP_ULONG flag);
static MMP_ULONG usb_vs_move_packet(STREAM_DMA_BLK *dma_blk,MMP_UBYTE endbit,MMP_UBYTE sti_mode,MMP_UBYTE *pad_app3_header,MMP_ULONG *sent_size);
static MMP_ULONG usb_vs_empty_packet(STREAM_DMA_BLK *dma_blk,MMP_UBYTE endbit,MMP_UBYTE sti_mode);
static void usb_vs_advance_to_lastframe(STREAM_CFG *cur_stream,MMP_UBYTE pipe);
static void usb_vs_align_jpeg_width_height(MMP_UBYTE *jpeg,MMP_ULONG len,MMP_USHORT new_w,MMP_USHORT new_h);
static MMP_UBYTE *usb_vs_search_app3_insert_addr(MMP_UBYTE *jpeg,MMP_ULONG len,MMP_USHORT tag);
static MMP_ERR usb_vs_wait_dma_done(MMPF_DMA_M_ID DMA_ID,MMP_ULONG timeout); // timeout is loop count
static void _dma_status_callback (void);
static MMP_ERR usb_vs_trig_dma_move (MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,MMP_ULONG ulCount);

static void init_running_res(void);
static short get_running_res_index(unsigned short w, unsigned short h);
static volatile MMP_UBYTE *usb_vs_fill_header(MMP_UBYTE ep_id,volatile MMP_UBYTE *ptr,MMP_UBYTE bfh,MMP_ULONG pts,MMP_ULONG *stc,MMP_USHORT *sofnum,MMP_USHORT layerid);
static void usb_vs_draw_face_pos(MMP_ULONG frame_addr,MMP_USHORT frame_w) ;
extern MMPF_OS_FLAGID SYS_Flag_Hif;


extern H264_FORMAT_TYPE gbCurH264Type;
extern MMP_UBYTE  gbUsbHighSpeed;
extern MMP_ULONG glPCCAM_VIDEO_BUF_ADDR;
extern MMP_ULONG glPCCAM_VIDEO_BUF_MAX_SIZE;
extern MMP_ULONG glUSB_UVC_DMA_EP1_BUF_ADDR[];
extern MMP_ULONG glUSB_UVC_DMA_EP2_BUF_ADDR[];
//extern MMP_ULONG glUSB_UVC_DMALIST_EP1_ADDR[];
//extern MMP_ULONG glUSB_UVC_DMALIST_EP2_ADDR[];
extern MMP_ULONG glUSB_UVC_ZERO_BUF_ADDR;

extern MMP_ULONG glFrameStartSOF ;

extern ISP_UINT32 gTextColorFormat;
extern ISP_UINT32 gTextPreviewBufAddr;
extern ISP_UINT32 gTextPreviewWidth ;


volatile MMP_ULONG PTS_UVC[2];
volatile MMP_ULONG PTS;
//MMP_ULONG glDiffSend ;


#if H264_POC_DUMP_EN
MMP_USHORT gsH264FrameSeq[2];
#endif
#if 1//PCCAM_MSB==1
 static RES_TYPE_CFG     gsRunningRes[2] ;
#endif 



// sean@2011_01_22 for uvc yuy2 + h264 dual stream and wraped by jpeg header
// The header for H264+YUY2 payload format
__align(4)  static char gbJpegHeader[] = 
{
    0xFF,0xD8,0xFF,0xDD,0x00,0x04,0x00,0x00,
    // mark for jpeg snooper tool warning
    /*
    0xFF,0xDA,0x00,0x0C,
    0x03,0x01,0x00,0x02,0x11,
    0x03,0x11,0x00,0x3F,0x00,
    */
    0xFF,0xD9
#if (INSERT_EOISOI==1)
    // pattern to seperate end of frame in stream
    ,'E','O','I','S','O','I' 
#endif    
};


static MMP_ULONG usb_vs_map_ext_flag(MMP_ULONG flag)
{
    MMP_ULONG ext_bits = 0;
    MMP_ULONG stream_bits = 0,layer_bits = 0 ,lid = 0,sid = 0;
#if H264_SIMULCAST_EN
    sid = H264_UVC_PH_STREAM_ID(flag);
#endif    
    if(flag & H264_UVC_PH_FLAG_IDR ) {
        ext_bits |= UVC_PH_FLAG_IDR ;
    }
    if(flag & H264_UVC_PH_FLAG_EOF ) {
        ext_bits |= UVC_PH_FLAG_EOF ;
    }
    if(flag & H264_UVC_PH_FLAG_SOF) {
        ext_bits |= UVC_PH_FLAG_SOF ;
    }
    lid = H264_UVC_PH_LAYER_ID(flag);
    layer_bits  = UVC_PH_LID_BITS(lid) ;
    stream_bits = UVC_PH_SID_BITS(sid) ;
    return (ext_bits | stream_bits | layer_bits ) ;
}

void usb_fill_payload_header (MMP_UBYTE eid,MMP_UBYTE *frame_ptr, MMP_ULONG framelength, MMP_ULONG frameseq,
                                MMP_ULONG flag, MMP_ULONG timestamp, MMP_USHORT w, MMP_USHORT h,
                                MMP_USHORT framerate, MMP_UBYTE ubPipe)
{
    MMP_UBYTE   ph_type, format_type;
    const STREAM_CFG  *cur_pipe = usb_get_cur_image_pipe(ubPipe);

    #if 0
    ph_type = cur_pipe->pipe[ubPipe].pipe_ph_type;
    #elif 1
    ph_type = cur_pipe->pipe_ph_type[ubPipe];
    #else
    if (ubPipe == 0) {
        ph_type = cur_pipe->pipe0_ph_type;
    }
    else if (ubPipe == 1) {
        ph_type = cur_pipe->pipe1_ph_type;
    }
    #endif
    // For AIT stream veiewer to seperate 2 H264 stream
    if(eid==0) {
        format_type = (MMPF_VIDMGR_GetMtsMuxModeEnable())? ST_M2TS: ST_H264;
    }
    else {
        format_type = (MMPF_VIDMGR_GetMtsMuxModeEnable())? ST_M2TS: ST_H264_2;
    }

    if (ph_type == PIPE_PH_TYPE_2) {
        usb_skype_fill_payload_header (frame_ptr, framelength,
            frameseq, 0, timestamp, w, h, format_type, framerate);
    }
    else if (ph_type == PIPE_PH_TYPE_3) {
        usb_frameh264_fill_payload_header (frame_ptr, framelength,
            frameseq, flag, timestamp, w, h, format_type, framerate);
    } 
    else {
        usb_uvc_fill_payload_header (frame_ptr, framelength,
            frameseq, flag, timestamp, w, h, format_type, framerate);
    }
}


void usb_uvc_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate)
{
#if H264_POC_DUMP_EN
extern MMPF_H264ENC_ENC_INFO *H264EncInfo ;
#endif

//extern MMP_ULONG glFrameStartSOF ;
    STREAM_CFG *cur_pipe ;//= usb_get_cur_image_pipe();
    //PIPE_PATH pipe = PIPE_0 ;
    MMP_UBYTE pipe_cfg;
    
    struct _UVC_H264_PH *header = (struct _UVC_H264_PH *)frame_ptr ;
    MMP_SHORT delay = 0 ;
    MMP_UBYTE ep_id = 0 ;
    // YUY2 only , just fill framelehgth
   
    if(type==ST_YUY2) {
        ep_id = STREAM_EP_YUY2 ;
        pipe_cfg = PIPE_CFG_YUY2 ;
        
        //pipe = usb_get_cur_image_pipe_id();
    }
    else if ( type==ST_MJPEG) {
        ep_id = STREAM_EP_MJPEG; 
        pipe_cfg = PIPE_CFG_MJPEG ;
    }
    else if( type==ST_H264) {
        ep_id = STREAM_EP_H264;
        pipe_cfg = PIPE_CFG_H264 ;
    }
    else if (type==ST_NV12) {
        ep_id = STREAM_EP_NV12 ;
        pipe_cfg = PIPE_CFG_NV12 ;
    }
    #if OMRON_FDTC_SUPPORT
    if(usb_uvc_is_fdtc_on()&&(pipe_cfg==PIPE_CFG_NV12)) {
        cur_pipe = usb_get_cur_fdtc_pipe();
    } else {
        cur_pipe = usb_get_cur_image_pipe_by_epid(ep_id);
    }    
    #else 
    cur_pipe = usb_get_cur_image_pipe_by_epid(ep_id);
    #endif
    
    //pipe = usb_get_cur_image_pipe_id(cur_pipe,pipe_cfg);
    if(header) {
        header->wVersion = UVC_H264_VER;//payload version is different from spec. version //gsUVCXVersion ;
        header->wHeaderLen = UVC_H264_PH_LEN ; // not include dwPayloadSize
        switch(type) {
        case ST_YUY2:
            header->dwStreamType = 0x32595559 ;
            break;
        case ST_NV12:
            header->dwStreamType = 0x3231564E ;
            break;
        case ST_H264:
            header->dwStreamType = 0x34363248 ;
            break;         
        default:
            header->dwStreamType = 0x34363248 ;
            break;                
        }
        
        header->wImageWidth = w ;
        header->wImageHeight = h ;
        header->dwFrameRate = (10000000/framerate);

        #if (FLAG_EOF_IN_LAST_SLICE == 1)
        //if ( UVCX_IsSliceBasedTx(pipe) ) {
        if ( 1/*UVCX_IsSliceBasedTxStream(cur_pipe)*/ ) { // Alyways 1 for simulcast
            MMP_ULONG ext_bits = usb_vs_map_ext_flag(flag) ;
            header->dwFrameRate = (header->dwFrameRate & ~UVC_PH_EXT_BIT_MASK) | ext_bits ;//flag ;
            //dbg_printf(3,"flag : %x to ext_bits :%x\r\n",flag,ext_bits);
        } else {
            //dbg_printf(3,"eos\r\n");
        }
        #endif

#if ALIGN_PAYLOAD==0        
        header->dwFrameSeq = frameseq ;
        //if(type==ST_H264)
        //dbg_printf(3,"Fill seq:%d\r\n",frameseq);
        header->wReserved = 0 ;
#endif
        header->dwTimeStamp = timestamp ;
        header->wDelay = 0 ;
       //header->dwFlag = flag ;
        header->dwPayloadSize = framelength ;
        // sean@2011_01_22, add encoder delay
        // From VIF to Encode Dump
        // Delay spec : From "End of Exposure" to "data send to USB"
        // So we can't set delay in here
#if H264_POC_DUMP_EN
        header->wDelay = H264EncInfo->frame_num ;
#else        
        header->wDelay = 0 ;
#endif
    }
}

void usb_skype_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate)
{ 
#if USB_UVC_SKYPE

    struct _SKYPE_H264_PH *header = (struct _SKYPE_H264_PH *)frame_ptr ;
    if(header) {
        header->dwTimeStamp = timestamp ;
        switch(type) {
        case ST_YUY2:
            header->bStreamType = SkypeYUY2 ;
            header->bStreamID = SkypePreviewStream ;
            break;
        case ST_NV12:
            header->bStreamType = SkypeNV12 ;
            header->bStreamID = SkypePreviewStream ;
            break;
        case ST_H264:
            header->bStreamType = SkypeMJPEG ;
            header->bStreamID = SkypeMainStream ;
            break;         
        case ST_MJPEG:
            header->bStreamType = SkypeH264 ;
            header->bStreamID = SkypePreviewStream ;
            break;         
        }
        
        header->wFrameSeq = (MMP_USHORT)frameseq ;
        header->dwPayloadOffset = 0 ; // OOXX should fixed
        header->dwPayloadSize = framelength ;
        
#if ALIGN_PAYLOAD == 0      
        header->wVersion = SKYPE_VERSION ;
        header->wImageWidth = w ;
        header->wImageHeight = h ;
        header->wDelay = 0 
        header->dwFrameRate = (10000000/framerate);
#endif
        
    }
#endif
    
}
void usb_frameh264_fill_payload_header(MMP_UBYTE *frame_ptr,MMP_ULONG framelength,MMP_ULONG frameseq,MMP_ULONG flag,MMP_ULONG timestamp,MMP_USHORT w,MMP_USHORT h,MMP_USHORT type,MMP_USHORT framerate)
{ 
#if USB_FRAMEBASE_H264_DUAL_STREAM==1
#define UVC_H264_VER09 0x0090
    struct _FRAME_H264_PH *header = (struct _FRAME_H264_PH *)frame_ptr ;
    STREAM_CFG *cur_pipe ;//= usb_get_cur_image_pipe();
    //PIPE_PATH pipe = PIPE_0 ;
    MMP_UBYTE pipe_cfg;
    MMP_UBYTE ep_id = 0 ;
    if(type==ST_YUY2) {
        ep_id = STREAM_EP_YUY2 ;
        pipe_cfg = PIPE_CFG_YUY2 ;
    }
    else if ( type==ST_MJPEG) {
        ep_id = STREAM_EP_MJPEG; 
        pipe_cfg = PIPE_CFG_MJPEG ;
    }
    else if( (type==ST_H264)||(type==ST_H264_2)) {
        ep_id = STREAM_EP_H264;
        pipe_cfg = PIPE_CFG_H264 ;
    }
    else if (type==ST_NV12) {
        ep_id = STREAM_EP_NV12 ;
        pipe_cfg = PIPE_CFG_NV12 ;
    }
    
    cur_pipe = usb_get_cur_image_pipe_by_epid(ep_id);
    //pipe = usb_get_cur_image_pipe_id(cur_pipe,pipe_cfg);
    if(header) {
        header->wVersion = UVC_H264_VER09 ;
        header->wHeaderLen = UVC_H264_PH_LEN ;
        header->wStreamType = type ;
        header->wImageWidth = w ;
        header->wImageHeight = h ;
        #if ALIGN_PAYLOAD==0
        header->dwFrameSeq = frameseq ;
        #endif
        header->dwTimeStamp = timestamp ;
        header->wFrameRate = (10000000/framerate);
        #if 0//(FLAG_EOF_IN_LAST_SLICE == 1)
        if ( UVCX_IsSliceBasedTxStream(cur_pipe) ) {
            header->dwFrameRate = (header->dwFrameRate & ~UVC_PH_EXT_BIT_MASK) | flag ;
        } else {
        }
        #endif
        header->dwPayloadSize = ALIGN32(framelength) ;
        header->dwReserved = framelength ;
#if H264_SIMULCAST_EN
        if(pipe_cfg==PIPE_CFG_H264) {
            header->dwFlag = H264_UVC_PH_STREAM_ID(flag);
            //dbg_printf(3,"sid = %d\r\n",header->dwFlag);
        }
        else {
            header->dwFlag = 0;
        }
#endif    
    }
#endif
}



#if USB_UVC_SKYPE
MMP_UBYTE *usb_skype_fill_header(MMP_UBYTE *frame_hdr1,MMP_UBYTE *h264_hdr2,SkypeH264EndPointType ep_type)
{
extern MMP_ULONG glSkypeH264Hdr ;
    struct _SKYPE_H264_PH *hdr1 = (struct _SKYPE_H264_PH *)frame_hdr1 ;
    struct _SKYPE_H264_PH *hdr2 = (struct _SKYPE_H264_PH *)h264_hdr2 ;
    MMP_UBYTE *ptr_item ,*ptr_hdr ;
    ptr_hdr = (MMP_UBYTE *)glSkypeH264Hdr ;
    
    ptr_item = (MMP_UBYTE *)&hdr2->dwTimeStamp ;
    set_quad(ptr_hdr,  *(MMP_ULONG *)(ptr_item+4));
    ptr_hdr+=4;
    set_quad(ptr_hdr,*(MMP_ULONG *)(ptr_item+0));
    
    ptr_hdr+=sizeof(MMP_ULONG64);
    *ptr_hdr++ = hdr2->bStreamID ;
    *ptr_hdr++ = hdr2->bStreamType ;
    set_double(ptr_hdr,hdr2->wFrameSeq);
    ptr_hdr += sizeof(MMP_USHORT) ;
    set_quad(ptr_hdr,hdr2->dwPayloadOffset);
    ptr_hdr += sizeof(MMP_ULONG );
    set_quad(ptr_hdr,hdr2->dwPayloadSize);
    ptr_hdr += sizeof(MMP_ULONG );
    
    ptr_item = (MMP_UBYTE *)&hdr1->dwTimeStamp ;
    set_quad(ptr_hdr,  *(MMP_ULONG *)(ptr_item+4));
    ptr_hdr+=4;
    set_quad(ptr_hdr,*(MMP_ULONG *)(ptr_item+0));
    ptr_hdr+=sizeof(MMP_ULONG64);
    *ptr_hdr++ = hdr1->bStreamID ;
    *ptr_hdr++ = hdr1->bStreamType ;
    set_double(ptr_hdr,hdr1->wFrameSeq);
    ptr_hdr += sizeof(MMP_USHORT) ;
    set_quad(ptr_hdr,hdr1->dwPayloadOffset);
    ptr_hdr += sizeof(MMP_ULONG );
    set_quad(ptr_hdr,hdr1->dwPayloadSize);
    ptr_hdr += sizeof(MMP_ULONG );
        

    if(ep_type==SingleEndPoint) {
        set_quad(ptr_hdr,2);
    } else {
        dbg_printf(3,"#Only 1 endpoint now\r\n");
    }
    ptr_hdr+=4 ;
    
    set_quad(ptr_hdr,SKYPE_MAGIC);
    
    return (MMP_UBYTE *)glSkypeH264Hdr ;

}
#endif

void usb_vs_dump_info(MMP_ULONG dmabuf)
{
    volatile USB_DMA_DATA_INFO *data_info;
    data_info = (USB_DMA_DATA_INFO *)(dmabuf + USB_DMA_DATA_INFO_OFFSET);
    dbg_printf(3,"@%d,%x\r\n",data_info->dmalist_status,data_info);
}


MMP_UBYTE usb_vs_send_image(MMP_UBYTE *sti_mode,MMP_UBYTE ep_id)
{
// 0 : Local preview run it fps
// 1 : Local preview sync to h264
#define USB_VS_WAIT_ALL_PIPE_FRAME      (0)

#define USB_VS_WAIT_NEW_FRAME(pipe)  MMPF_Video_IsEmpty( pipe ) 


#define IDR_SET_STI_BIT_EN  (0) // Can not turn on in !win-8 os
#define LOG_JITTER_EN   (0)
#define MAX_APP3_LEN    ( 65531 ) // Let 0xFFE3 + Len Field(2Bytes) + 65531 = 65535 = 64KB
extern MMP_USHORT gsCurFrameRatex10[] ;
static MMP_LONG vs_frame_t1 ;
static MMP_BOOL gbSVCSSL_start[MAX_NUM_ENC_SET] ;    //start or stop
static MMP_BYTE gbSVCSSL_bitmap ;  // active streamp bitmap
static MMP_BYTE gbSVCSSL_layer[MAX_NUM_ENC_SET] ;   // layer id of dependent
static STREAM_CFG cur_stream_tmp[2];
static MMP_ULONG sof_old = 1 ;
//extern volatile MMP_ULONG uSOFNUM;

    AITPS_IBC   pIBC    = AITC_BASE_IBC;
#if CAPTURE_BAYER_RAW_ENABLE
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    AITPS_ISP   pISP = AITC_BASE_ISP;
#endif    
    MMP_ULONG   jpegSize;
    MMP_UBYTE   endbit = 0;
    MMP_ULONG  xsize;
    volatile MMP_UBYTE *ptt;
    volatile MMP_UBYTE *cur_pt;
    MMP_ULONG buf_addr;
#if USB_UVC_BULK_EP			
    MMP_USHORT r,d,a;
#endif
#if USB_UVC_SKYPE
    MMP_UBYTE skip_dummy;
#endif

    PCAM_USB_VIDEO_FORMAT vidFmt = GetFmtIndexByEp(ep_id) ;// gsVidFmt;
	FRAME_PAYLOAD_HEADER *frame_ph ;
	STREAM_CFG *cur_stream = (STREAM_CFG *)usb_get_cur_image_pipe_by_epid(ep_id);
	STREAM_CFG *cur_stream_tx;
    STREAM_DMA_BLK *dma_blk ;
    
    MMP_UBYTE   *app3_header = 0 ;
    MMP_ULONG   send_size = 0;
    MMP_UBYTE   pipe0_cfg,pipe1_cfg,pipe2_cfg;
    MMP_BOOL    emptyblk = 0;
    MMP_UBYTE   grapath = MMPF_Video_IsLoop2GRA(cur_stream);
    
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    
    if(ss->packetcount==0) {
    
        cur_stream_tmp[ep_id] = *(STREAM_CFG *)usb_get_cur_image_pipe_by_epid(ep_id);
        cur_stream_tx = &cur_stream_tmp[ep_id] ;
        cur_stream_tx->pipe_en &= ~PIPEEN(GRA_SRC_PIPE);
        //cur_stream_tx = cur_stream;

        usb_vs_monitor_icon_dline();
        //dbg_printf(3,"vs.s:%d\r\n",usb_vc_cur_sof() );
        // sean@2013_01_28 for H264 AU keep same
        // ss->STC = 0 ; // Reset STC to zero so that we can get new STC for whole frame packets.
         // First Frame, wait all pipe 's frame are ready ~
        if(ss->usbframecount==0) {
            *sti_mode = 0 ; // Reset STI mode in case non-UVC1.5 H264 was set.    
        #if LOG_JITTER_EN
        vs_frame_t1 = 0;
        #endif
            init_running_res();
#if H264_POC_DUMP_EN
            gsH264FrameSeq[0] = gsH264FrameSeq[1] = 0;
#endif 
            if(ep_id==0) {
                UsbISODmaListTxInit(ep_id,glUSB_UVC_DMA_EP1_BUF_ADDR[0] ,glUSB_UVC_DMA_EP1_BUF_ADDR[1]);
            } else {
                UsbISODmaListTxInit(ep_id,glUSB_UVC_DMA_EP2_BUF_ADDR[0] ,glUSB_UVC_DMA_EP2_BUF_ADDR[1]);
            }
            
            if( (ss->tx_flag & SS_TX_RT_MODE)==0) {
                MMP_UBYTE pipe,pipe_cfg ;
                for(pipe=STREAMING_START_PIPE;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
                    if( PIPE_EN(cur_stream_tx->pipe_en) & (1 << pipe) ) {
                        pipe_cfg = CUR_PIPE_CFG(pipe,cur_stream_tx->pipe_cfg);
                        if(pipe_cfg!=PIPE_CFG_NV12) {
                            while( (MMPF_Video_IsEmpty(pipe)) ){
                                 if(  (ss->tx_flag & SS_TX_STREAMING ) == 0 ) {
                                     return UVC_SEND_IMG_RET_CONTINUE ;
                                 }
                                 //RTNA_DBG_Str0("s0");
                                 MMPF_OS_Sleep(1);
                            }
                        }
                    }
                }

                if( (UVC_VCD()==bcdVCD15) && (gbCurH264Type==FRAMEBASE_H264)) {
                    UVC15_GetH264StartStopLayer(gbSVCSSL_start,(MMP_UBYTE *)&gbSVCSSL_bitmap,(MMP_UBYTE *)gbSVCSSL_layer) ;
                }
            }
        } else {
            if( (ss->tx_flag & SS_TX_RT_MODE)==0) {
                MMP_UBYTE pipe,pipe_cfg ;
                for(pipe=STREAMING_START_PIPE;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
                    if( PIPE_EN(cur_stream_tx->pipe_en) & (1 << pipe) ) {
                        pipe_cfg = CUR_PIPE_CFG(pipe,cur_stream_tx->pipe_cfg);
                        if(pipe_cfg!=PIPE_CFG_NV12) {
                            while( (MMPF_Video_IsEmpty(pipe)) ){
                                 if(  ((ss->tx_flag & SS_TX_STREAMING ) == 0) || ((ss->tx_flag & SS_TX_PAUSE ) == 0) ) {
                                     return UVC_SEND_IMG_RET_CONTINUE ;
                                 }
                                 //RTNA_DBG_Str0("s0");
                                 MMPF_OS_Sleep(1);
                            }
                            #if USB_VS_WAIT_ALL_PIPE_FRAME==0
                            if(UVCX_IsSimulcastH264()) {
                                break ;
                            }
                            #endif
                        }
                    }
                }
            }

            #if LOG_JITTER_EN
            if(vs_frame_t1) {
                MMP_LONG vs_frame_time ;// = usb_vc_cur_usof() ;
                MMP_LONG diff_t ;// = (gsCurFrameRatex10[ep_id] - vs_frame_time ) ;
                vs_frame_time  = usb_vc_cur_usof() >> 3 ;
                diff_t = vs_frame_time - vs_frame_t1 ;
                vs_frame_t1 = vs_frame_time ;
                //dbg_printf(3,"vs_frame_time: %d ms\r\n",vs_frame_time );
                if( diff_t < 0) { 
                    diff_t = 0 - diff_t ;
                }
                
                diff_t = ((1000*10)/gsCurFrameRatex10[ep_id]  - diff_t ) ;
                if( diff_t < 0) { 
                    diff_t = 0 - diff_t ;
                }
                
                if(diff_t >= 3 ) {
                    dbg_printf(3,"vs.jitter : %d ms\r\n",diff_t ); 
                }
            } else {
                vs_frame_t1 = usb_vc_cur_usof() >> 3;
                //dbg_printf(3,"vs_frame_t1: %d ms\r\n",vs_frame_t1 );
            }
            #endif
        }
        
        ss->frame_stime = MMPF_BSP_GetTick();
        cur_stream->pipe_sync = 0 ;
         
        //sean@2011_02_25, one PTS per frame
        //PTS = STC ;
        // This is for null frame only.
        // If not null frame, PTS_UVC get from payload header timestamp.
        PTS_UVC[ep_id] = PTS ; 
        
        pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);
        pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_stream->pipe_cfg);
        pipe2_cfg = CUR_PIPE_CFG(PIPE_2,cur_stream->pipe_cfg);
        
    	//dbg_printf(3,"vs(%x),pipe0_cfg : %x,pipe1_cfg:%x\r\n",cur_stream,pipe0_cfg,pipe1_cfg);
    	
        if( UVCX_IsSimulcastH264() || ( IS_PIPE01_EN(cur_stream_tx->pipe_en)) ) {
            MMP_UBYTE pipe ,no_local_stream = 0;
            // Dual stream , H264 always in pipe1 now
            if( (gbCurH264Type == UVC_H264) ||(gbCurH264Type == FRAMEBASE_H264) || (gbCurH264Type == FRAMEBASE_H264_YUY2) || (gbCurH264Type == FRAMEBASE_H264_MJPEG)) { 
                 // Get MJPEG/YUY2 in usb_local_address
               if((pipe0_cfg==PIPE_CFG_MJPEG ) || (pipe0_cfg==PIPE_CFG_YUY2) || (pipe0_cfg==PIPE_CFG_NV12)) {
                   pipe = 0 ;
                } else {
                   pipe = 1 ;
                }
                
                #if H264_SIMULCAST_EN
                #if STREAMING_START_PIPE==0
                if( MMPF_Video_DataCount(pipe) <= 0 ) {
                    if(gbCurH264Type == FRAMEBASE_H264) {
                        no_local_stream = 1;
                    }
                    else {
                        return UVC_SEND_IMG_RET_CONTINUE ;
                    }
                }
                else {
                    usb_vs_advance_to_lastframe(cur_stream,pipe) ;
                }
                #endif
                #else
                // Fixed Dual stream tx stuck
                if( 1/* !grapath */) {
	                // Simulcast without local stream
	                if( UVCX_IsSimulcastH264() && (pipe0_cfg==0) ) {
	                    no_local_stream = 1 ;
	                } else {
	                
	                    if( MMPF_Video_DataCount(pipe) <= 0 ) {
	                        if(UVCX_IsSimulcastH264()&&(pipe0_cfg==PIPE_CFG_YUY2) ) {
	                            no_local_stream = 1;
	                        } else{
	                            return UVC_SEND_IMG_RET_CONTINUE ;
	                        }
	                    } else {
	                        usb_vs_advance_to_lastframe(cur_stream,pipe) ;
	                    }
	                }
                } else {
                	no_local_stream = 1 ;//TBD
                }
                
                #endif
                
            } else if(gbCurH264Type==UVC_YUY2){
                 // YUY2 in usb_local_address
                if(pipe0_cfg==PIPE_CFG_YUY2) {
                    pipe = 0;
                } else {
                    pipe = 1; 
                } 
                usb_vs_advance_to_lastframe(cur_stream,pipe) ;
                  
            } else if(gbCurH264Type==SKYPE_H264){
                if( (pipe0_cfg==PIPE_CFG_YUY2) || (pipe0_cfg==PIPE_CFG_MJPEG) ) {
                    pipe = 0 ; // PIPE 0 :YUY2 or MJPEG
                }    
                else {
                    pipe = 1 ; // PIPE 1 :YUY2 or MJPEG
                } 
                
                if( (pipe0_cfg==PIPE_CFG_MJPEG) && (MMPF_Video_DataCount(pipe) <= 0 ) ) {
                    return UVC_SEND_IMG_RET_CONTINUE ;
                } else { 
                    usb_vs_advance_to_lastframe(cur_stream,pipe) ;   
                }
            }
            if(no_local_stream==0) {
                ss->frameaddr = (MMP_ULONG)MMPF_Video_CurRdPtr(pipe) ; 
            } else {
                ss->frameaddr  = 0 ;
                goto skip_local_stream ;
            }
        } 
        // Only pipe0 output
        else if( IS_PIPE0_EN(cur_stream_tx->pipe_en ) ) { 
            if( (pipe0_cfg != PIPE_CFG_H264) && (!(ss->tx_flag & SS_TX_RT_MODE) ) ) {
                usb_vs_advance_to_lastframe(cur_stream,PIPE_0) ;   
            }
            ss->frameaddr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_0) ; // PIPE 0
        } 
        // Only pipe1 output 
        else if( IS_PIPE1_EN(cur_stream_tx->pipe_en ) ) { 
            if(pipe1_cfg != PIPE_CFG_H264) {
                usb_vs_advance_to_lastframe(cur_stream,PIPE_1) ;   
            }
            ss->frameaddr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_1) ; // PIPE 1
        }
        else if( IS_PIPE2_EN(cur_stream_tx->pipe_en ) ) { 
             if(pipe2_cfg != PIPE_CFG_H264) {
                usb_vs_advance_to_lastframe(cur_stream,PIPE_2) ;   
            }
            ss->frameaddr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_2) ; // PIPE 1
        }
        
        
        
        
        if(ss->frameaddr ==0) {
            //RTNA_DBG_Str3("Strange usb_local_address\r\n");
            ss->frameaddr  = glPCCAM_VIDEO_BUF_ADDR ;
        } else {
            //usb_check_reframe--;
        }
 skip_local_stream:  
        ss->tx_flag&=~SS_TX_CLOSE_STREAMING ;
        ss->tx_flag&=~SS_TX_PRECLOSE_STREAMING ;
     }
    
    ptt = (volatile MMP_UBYTE *) ss->frameaddr ;   
 
    if(ss->packetcount == 0) {  //first package of frame
        ss->packetcount = 1 ; // prevent re-enter this function
        //glDiffSend = usb_vc_cur_sof();
        if ( ep_id==0 ) {
            MMPF_Video_InitDMABlk(ep_id,ss->maxpacketsize-UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id],glUSB_UVC_DMA_EP1_BUF_ADDR[0] ,glUSB_UVC_DMA_EP1_BUF_ADDR[1]);
        } else {
            MMPF_Video_InitDMABlk(ep_id,ss->maxpacketsize-UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id],glUSB_UVC_DMA_EP2_BUF_ADDR[0] ,glUSB_UVC_DMA_EP2_BUF_ADDR[1]);        
        }
        
        frame_ph = (FRAME_PAYLOAD_HEADER *)ptt;
        //dbg_printf(3,"frame_ph:%x\r\n",frame_ph);
        // single stream PTS...
        if(frame_ph) {
            PTS_UVC[ep_id] = frame_ph->UVC_H264_PH.dwTimeStamp;
        }
        //dbg_printf(3,"PTS_UVC : %x\r\n",PTS_UVC);
        if(vidFmt == PCAM_USB_VIDEO_FORMAT_MJPEG){
            //glFrameLength = frame_ph->UVC_H264_PH.dwPayloadSize ;
            ss->framelength = frame_ph->UVC_H264_PH.dwPayloadSize ;
            // Single MJPEG stream
            if(gbCurH264Type!=UVC_YUY2) {
                MMP_ULONG dt, fps ;
                MMP_UBYTE *header_addr; 
                header_addr = (MMP_UBYTE *)(ptt + FRAME_PAYLOAD_HEADER_SZ) ;
                // Re-adjust Jpeg height if the input height is not multiple of 8
                usb_vs_align_jpeg_width_height( (MMP_UBYTE *)(header_addr),ss->framelength,frame_ph->UVC_H264_PH.wImageWidth,frame_ph->UVC_H264_PH.wImageHeight);
                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(header_addr),ss->framelength,0 );
                if(emptyblk) {
                    ss->packetcount = 0;
                    ss->tx_flag |= (SS_TX_CLOSE_STREAMING | SS_TX_PRECLOSE_STREAMING) ;
                    return  UVC_SEND_IMG_RET_CONTINUE ;       
                    
                }
                
                dt = 1000 / (usb_vc_cur_sof() - sof_old );
                //dbg_printf(3,"ss->framelength : %d,fps:%d\r\n",ss->framelength,dt );
                
                sof_old = usb_vc_cur_sof() ;
            } else {
            // Dual stream MJPEG + YUY2
                MMP_UBYTE *jpeg_ptr,*jpeg_payload_ptr ;
                FRAME_PAYLOAD_HEADER *jpeg_ph ;
                if(pipe0_cfg==PIPE_CFG_MJPEG) {
                    jpeg_ptr = MMPF_Video_CurRdPtr(PIPE_0) ;
                } else {
                    jpeg_ptr = MMPF_Video_CurRdPtr(PIPE_1) ;
                } 
                
                jpeg_ph = (FRAME_PAYLOAD_HEADER *)( jpeg_ptr );
                // YUY2 && MJPEG LEN
                if(ss->framelength/*glFrameLength*/ && jpeg_ph->UVC_H264_PH.dwPayloadSize ) {
                
                    MMP_ULONG app3_insert_addr = 0,i,app3_search_len;
                    MMP_SHORT delay ;
                    MMP_ULONG cur_addr,off,tx_len,yuy2_off=0,yuy2_blk_size ;
                    MMP_UBYTE *jpeg_payload_ptr = (MMP_UBYTE *)jpeg_ptr + FRAME_PAYLOAD_HEADER_SZ ;
                    // Send both of YUY2 and MJPEG
                    // PTS use MJPEG payload header timestamp
                    PTS_UVC[ep_id] = frame_ph->UVC_H264_PH.dwTimeStamp;
                    app3_search_len = jpeg_ph->UVC_H264_PH.dwPayloadSize ;
                    // Re-adjust Jpeg height if the input height is not multiple of 8
                    usb_vs_align_jpeg_width_height( (MMP_UBYTE *)(jpeg_payload_ptr),app3_search_len,frame_ph->UVC_H264_PH.wImageWidth,frame_ph->UVC_H264_PH.wImageHeight);
                    app3_insert_addr = (MMP_ULONG)usb_vs_search_app3_insert_addr((MMP_UBYTE *)(jpeg_payload_ptr),app3_search_len,0xFFDD );
                    off = app3_insert_addr - (MMP_ULONG)jpeg_payload_ptr ;
                    MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr      ), off,0);
                    cur_addr = (MMP_ULONG)frame_ph ; // insert YUY2
                    yuy2_blk_size =FRAME_PAYLOAD_HEADER_SZ + ss->framelength/*glFrameLength*/ ;
                    do {
                        tx_len = (yuy2_blk_size > MAX_APP3_LEN ) ? MAX_APP3_LEN : yuy2_blk_size ;
                        MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)(cur_addr + yuy2_off), tx_len,0 );
                        yuy2_off+= tx_len ;
                        yuy2_blk_size -= tx_len ;
                    } while(yuy2_blk_size );
                    MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr + off), app3_search_len - off,0 );
                    // Both pipe can send...
                    cur_stream->pipe_sync = PIPE_NEXT_FRAME(0) | PIPE_NEXT_FRAME(1) ;
                     
                } else {
                    //emptyblk = MMPF_Video_AddDMABlk(0,(MMP_ULONG)(jpeg_ptr), 0,0);
					if(jpeg_ph->UVC_H264_PH.dwPayloadSize) {
					    // Only JPEG, no YUY2
					    PTS_UVC[ep_id] = frame_ph->UVC_H264_PH.dwTimeStamp;
						MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_ptr + FRAME_PAYLOAD_HEADER_SZ),jpeg_ph->UVC_H264_PH.dwPayloadSize,0 );
						cur_stream->pipe_sync =  PIPE_NEXT_FRAME(1) ;
					} else {
						emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_ptr), 0,0);
				    }                   
                }
            }
            
        }
        else if((vidFmt == PCAM_USB_VIDEO_FORMAT_YUV422) || (vidFmt == PCAM_USB_VIDEO_FORMAT_YUV420)
			|| (vidFmt == PCAM_USB_VIDEO_FORMAT_RAW) ){
	        PCAM_BAYER_INFO *bayerInfo = (PCAM_BAYER_INFO *)USB_GetBayerRawInfo();
            ss->framelength = frame_ph->UVC_H264_PH.dwPayloadSize ;
            //dbg_printf(3,"ptt : %x,ss->framelength:%d,%d\r\n",ptt,ss->framelength,ss->usbframecount);
            
            if (vidFmt == PCAM_USB_VIDEO_FORMAT_RAW ) {            
                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ptt + FRAME_PAYLOAD_HEADER_SZ),ss->framelength,0 );
            } else {
               // dbg_printf(3,"ss->framelength : %d\r\n",ss->framelength);
                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ptt + FRAME_PAYLOAD_HEADER_SZ),ss->framelength,0 );
                
                if(emptyblk) {
                    ss->packetcount = 0;
                    ss->tx_flag |= (SS_TX_CLOSE_STREAMING | SS_TX_PRECLOSE_STREAMING) ;
                    return  UVC_SEND_IMG_RET_CONTINUE ;       
                    
                }
                
            }
#if DRAW_TEXT_FEATURE_EN==ON
            if(vidFmt == PCAM_USB_VIDEO_FORMAT_YUV422) {
				gTextColorFormat = ISP_PREVIEW_FORMAT_YUV422;
				gTextPreviewWidth = cur_stream->pipe_w[PIPE_0] ;
				gTextPreviewBufAddr = (MMP_ULONG)(ptt + FRAME_PAYLOAD_HEADER_SZ);
       			// print string on preview frames
			    VR_PrintStringOnPreview();
			}    
#endif
            // Just for test only
            #if OMRON_FDTC_SUPPORT
            if(vidFmt == PCAM_USB_VIDEO_FORMAT_YUV420) {
                //usb_vs_draw_face_pos((MMP_ULONG)(ptt + FRAME_PAYLOAD_HEADER_SZ),cur_stream->pipe_w[PIPE_2]);
            }
            #endif
        } else { // H.264
        
            MMP_UBYTE framebase_h264 = 0 ;
            // H264 stream:
            // 1. Pure Frame base stream
            // 2. Skype Mode B
            // 3. LGT H264 + MJPG
            // single pipe , treat it as Framebase H264
            if( ( (PIPE_EN(cur_stream->pipe_en) & PIPE0_EN)== 0 ) || (gbCurH264Type==FRAMEBASE_H264) ){
                //RTNA_DBG_Str3("FrameBase.H264\r\n");
                framebase_h264 = 1;
            }
            if(frame_ph) {
                if(gbCurH264Type==SKYPE_H264) {
                    // First Part Length is MJPEG or YUY2
                    ss->framelength = frame_ph->SKYPE_H264_PH.dwPayloadSize ;
                } 
                else if ((gbCurH264Type==FRAMEBASE_H264_YUY2)||(gbCurH264Type==FRAMEBASE_H264_MJPEG)) {
                #if USB_FRAMEBASE_H264_DUAL_STREAM==1
                    ss->framelength = frame_ph->FRAME_H264_PH.dwPayloadSize ;
                #endif    
                }
                else { 
                    // First Part Length is MJPEG or YUY2
                    ss->framelength  = frame_ph->UVC_H264_PH.dwPayloadSize ;
                }
            } else {
                ss->framelength  = 1 ; // simulcast with local preview frame
            }   
            switch(gbCurH264Type) {
            #if USB_FRAMEBASE_H264_DUAL_STREAM
                case FRAMEBASE_H264_YUY2:
                case FRAMEBASE_H264_MJPEG:
                #if USB_FRAMEBASE_H264_DUAL_STREAM==1
                {
                    FRAME_PAYLOAD_HEADER *h264_ph,*h264_ph_s1 = 0 ; 
                    MMP_ULONG h264_addr,yuy2_addr,h264_addr_s1 ;
                    
                    h264_addr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_1) ; 
                    h264_ph = (FRAME_PAYLOAD_HEADER *)h264_addr ;
                    
                    #if H264_SIMULCAST_EN
                    if(UVCX_IsSimulcastH264()) {
                        h264_addr_s1 = (MMP_ULONG)MMPF_Video_NextRdPtr(PIPE_1,1) ;
                        if(h264_addr_s1) {
                            h264_ph_s1 = (FRAME_PAYLOAD_HEADER *)( h264_addr_s1 );
                        }
                    }
                    #endif
                    
                    if(ss->usbframecount==0) {
                        emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(h264_addr), 0,0 );
                    }
                    else {
                        MMP_BOOL send_frame = MMP_FALSE ;

                    #if STREAMING_START_PIPE==0
                        if( ss->framelength ) 
                    #else
                        if( 1/*ss->framelength*/) 
                    #endif
                        {    
                            MMP_USHORT pipe_sync = 0;
                            MMP_USHORT index_s0 = ST_UNDEF,index_s1 = ST_UNDEF ;
                            
                            
                            #if USB_VS_WAIT_ALL_PIPE_FRAME==0
                            if(UVCX_IsSimulcastH264()) {
                                send_frame = MMP_TRUE ;
                            }
                            #endif
                            
                            #if 1
                            if(h264_ph->FRAME_H264_PH.dwPayloadSize) {
                            #if H264_CBR_PADDING_EN
                                usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)h264_addr );
                            #endif    
                                index_s0 = h264_ph->FRAME_H264_PH.wStreamType ;
                                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(h264_addr),h264_ph->FRAME_H264_PH.dwPayloadSize+FRAME_PAYLOAD_HEADER_SZ ,DMA_CF_EOF);    
                                send_frame = MMP_TRUE ;
                                pipe_sync |= PIPE_NEXT_FRAME(1) ;
                                //dbg_printf(3,"s0(%d) : %d\r\n",h264_ph->FRAME_H264_PH.dwPayloadSize,usb_vc_cur_usof());
                            }
                            #endif
                            
                            if(h264_ph_s1) {
                                if(h264_ph_s1->FRAME_H264_PH.dwPayloadSize) {
                                    MMP_BOOL send_s1 = MMP_FALSE ;
                                    #if H264_CBR_PADDING_EN
                                    usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)h264_addr_s1 );
                                    #endif
                                    index_s1 = h264_ph_s1->FRAME_H264_PH.wStreamType ;
                                    if(index_s1==ST_UNDEF) {
                                    } 
                                    else if(index_s0==index_s1) {
                                    }
                                    else {
                                        send_s1 = MMP_TRUE ;
                                    }
                                    
                                    
                                    if(send_s1) {
                                        emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(h264_addr_s1),h264_ph_s1->FRAME_H264_PH.dwPayloadSize+FRAME_PAYLOAD_HEADER_SZ ,DMA_CF_EOF);    
                                        send_frame = MMP_TRUE ;
                                        pipe_sync |= PIPE_NEXT_FRAME(2) ;
                                        //dbg_printf(3,"H264(%d,%d):%d\r\n",h264_ph_s1->FRAME_H264_PH.wImageWidth,h264_ph_s1->FRAME_H264_PH.wImageHeight,h264_ph_s1->FRAME_H264_PH.dwPayloadSize);
                                        //dbg_printf(3,"s1(%d) : %d\r\n",h264_ph_s1->FRAME_H264_PH.dwPayloadSize,usb_vc_cur_usof());
                                    }
                                }
                            }
                            if(send_frame==MMP_TRUE) {
                                #if STREAMING_START_PIPE==0
                                if(ss->framelength) {
                                    emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr),ss->framelength+FRAME_PAYLOAD_HEADER_SZ ,DMA_CF_EOF); 
                                }
                                else {
                                
                                }
                                #endif
                                pipe_sync |= PIPE_NEXT_FRAME(0) ;    
                                cur_stream->pipe_sync = pipe_sync ;
                               // dbg_printf(3,"sync : %x : %d\r\n",cur_stream->pipe_sync,usb_vc_cur_usof());
                                
                            }
                            else {
                                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr), 0,0);
                              }
                            //dbg_printf(3,"pipe_sync:%x\r\n",cur_stream->pipe_sync);
                        }
                        else {
                            emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr), 0,0);
                        }
                    }
                }
                #endif
                
                break;
            #endif
            case FRAMEBASE_H264:
                if(framebase_h264) {
                    // dbg_printf(3,"h264len : %d\r\n",ss->framelength);
                    PIPE_PATH pipe = usb_get_cur_image_pipe_id(cur_stream,PIPE_CFG_H264);
                    ss->frameaddr = (MMP_ULONG)MMPF_Video_CurRdPtr( /*PIPE_1*/pipe) ; 
                    frame_ph = (FRAME_PAYLOAD_HEADER *)ss->frameaddr;
                    ss->framelength = frame_ph->UVC_H264_PH.dwPayloadSize  ;
                   // dbg_printf(3,"@");
                    //dbg_printf(3,"[pipe : %d]ss->framelength:%d(%d,%d)\r\n",pipe,ss->framelength,frame_ph->UVC_H264_PH.wImageWidth,frame_ph->UVC_H264_PH.wImageHeight);
                    if(ss->usbframecount==0) {
                        emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr + FRAME_PAYLOAD_HEADER_SZ), 0 ,DMA_CF_EOF);
                    } else {
                        MMP_UBYTE c,eof ;
                        MMP_ULONG cbr_padding_bytes = 0 ;
                        if(ss->framelength) {
                            MMP_BOOL  sof ;
                            MMP_UBYTE sid = 0,lid = 0,pid = 0;
                            MMP_BOOL drop = MMP_FALSE ;
                            MMP_UBYTE i,streams = (UVCX_IsSimulcastH264()) ? MAX_NUM_ENC_SET : 1 ;
                            
                            // FIXBUG : UVC1.5 STI bit should be set when it is an IDR-slice
                            if(1/*UVC_VCD()==bcdVCD15*/) {
                                //UVC15_GetH264StartStopLayer(gbSVCSSL_start,&gbSVCSSL_bitmap,gbSVCSSL_layer) ;
                                
                                if(UVC_VCD()==bcdVCD15) {
                                    #if IDR_SET_STI_BIT_EN
                                        *sti_mode = ( frame_ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_IDR )?1:0 ;
                                    #else
                                        *sti_mode = 0 ;
                                    #endif    
                                        sid = UVC_PH_SID(frame_ph->UVC_H264_PH.dwFrameRate) ;
                                        lid = UVC_PH_LID(frame_ph->UVC_H264_PH.dwFrameRate) ;
                                        sof = ( frame_ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_SOF )?MMP_TRUE:MMP_FALSE ;
                                        for(i=0;i<streams;i++) {
                                            if(gbSVCSSL_bitmap & (1<< i) ) { // stream 0
                                                if( sid==i) {
                                                    if( gbSVCSSL_start[sid] ) { // start stream 0 layer
                                                        if( lid <= gbSVCSSL_layer[sid]) {
                                                            drop = MMP_FALSE ;
                                                        }
                                                        else {
                                                            drop = MMP_TRUE ;
                                                        }
                                                    }
                                                    else { // stop stream 0 layer
                                                        if( lid >= gbSVCSSL_layer[sid]) {
                                                            drop = MMP_TRUE ;
                                                        } 
                                                        else {
                                                            drop = MMP_FALSE ;
                                                        }
                                                    }
                                                    break ;
                                                }
                                            }
                                        }
                                        if(drop) {
                                            dbg_printf(3,"(sid,lid) : (%d,%d)->(%d)(%d,%d)\r\n",sid,lid,drop,frame_ph->UVC_H264_PH.wImageWidth,frame_ph->UVC_H264_PH.wImageHeight);
                                        }
                                
                                }
                                else {
                                    drop = MMP_FALSE ;
                                }
                                
                                if(drop) { // Force no drop
                                    // force to next frame
                                    ss->packetcount = 0;
                                    ss->tx_flag |= (SS_TX_CLOSE_STREAMING | SS_TX_PRECLOSE_STREAMING) ;
                                    cur_stream->pipe_sync =  PIPE_NEXT_FRAME(pipe) ;
                                    return  UVC_SEND_IMG_RET_END_FRAME ;       
                                } 
                                
                                if( (UVC_VCD()==bcdVCD15) && UVCX_IsSimulcastH264() ) {
                                    //UVC15_VIDEO_CONFIG *config = (UVC15_VIDEO_CONFIG *)UVC15_GetH264StartConfig(sid) ;
                                    //pid = config->bPriority[lid] ;    
                                    ss->uvc15_layerid = SVC_LAYER_ID(0,0,lid,sid,0);
                                }
                                else {
                                    ss->uvc15_layerid = 0;
                                }
                                
                                
                            }
                            
                            if(!UVCX_IsSliceBasedTxStream(cur_stream) ) {
                                #if H264_CBR_PADDING_EN==1
                                cbr_padding_bytes = usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)ss->frameaddr ) ;
                                if(cbr_padding_bytes) {
                                    //dbg_printf(3,"vs.padding : %d\r\n",cbr_padding_bytes);
                                    emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr + FRAME_PAYLOAD_HEADER_SZ),ss->framelength ,0);
                                    emptyblk = MMPF_Video_AddDMABlk(ep_id,0, usb_vs_get_cbr_padding_buffer(),cbr_padding_bytes ,DMA_CF_DUMMY | DMA_CF_EOF);
                                } else {
                                    emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr + FRAME_PAYLOAD_HEADER_SZ),ss->framelength ,DMA_CF_EOF);
                                }
                                #else
                                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ss->frameaddr + FRAME_PAYLOAD_HEADER_SZ),ss->framelength ,DMA_CF_EOF);
                                #endif
                            } else {    
                                emptyblk = MMPF_Video_AddDMABlkH264SliceMode(ep_id,0,(MMP_ULONG)ss->frameaddr,sid,sof);
                                
                            }
                
                        } else {
                             emptyblk =  MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)ss->frameaddr, 0,0);
                        }
                    }
                }
                break ;
            case SKYPE_H264:
#if USB_UVC_SKYPE
                if(SKYPE_VERSION==0x14) {	
                    
                    //FRAME_PAYLOAD_HEADER *yuv_ph ;
                    FRAME_PAYLOAD_HEADER *h264_ph ;
                    MMP_ULONG h264_addr,yuy2_addr ;
                    yuy2_addr = (MMP_ULONG)ptt ;
                     
                    if( pipe0_cfg==PIPE_CFG_H264 )
                        h264_addr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_0) ; // YUY2
                    else
                        h264_addr  = (MMP_ULONG)MMPF_Video_CurRdPtr(PIPE_1) ; // YUY2
                     
                    h264_ph = (FRAME_PAYLOAD_HEADER *)h264_addr ;
                    
                    // Force to send dummy frame
                    if(ss->usbframecount==0) {
                    	emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(yuy2_addr + FRAME_PAYLOAD_HEADER_SZ), 0,0 );
                    } else {
                        // H264 && YUY2 LEN
                        if( h264_ph->UVC_H264_PH.dwPayloadSize && ss->framelength/*glFrameLength*/ ) {
                            MMP_UBYTE *hdr ;
                            MMP_UBYTE *h264_payload_ptr = (MMP_UBYTE *)h264_addr + FRAME_PAYLOAD_HEADER_SZ ;
                            MMP_UBYTE *yuy2_res_ptr = (MMP_UBYTE *)yuy2_addr +  FRAME_PAYLOAD_HEADER_SZ - 4;
                            
                            MMP_ULONG h264_payload_size;
                            h264_payload_size = h264_ph->UVC_H264_PH.dwPayloadSize ;
                            
                            // H264. valid frame 
                            MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)h264_payload_ptr, h264_payload_size, 0);
                            // YUY2 valid frame 
                            if(pipe0_cfg==PIPE_CFG_YUY2) {
                                set_double(yuy2_res_ptr, cur_stream->pipe0_w );
                                yuy2_res_ptr+=2;
                                set_double(yuy2_res_ptr, cur_stream->pipe0_h );
                                MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(yuy2_addr + FRAME_PAYLOAD_HEADER_SZ - 4), ss->framelength/*glFrameLength*/ + 4 , 0 );
                            } else {
                                MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(yuy2_addr + FRAME_PAYLOAD_HEADER_SZ ), ss->framelength/*glFrameLength*/, 0 );
                            }
                            
                            hdr = usb_skype_fill_header( (MMP_UBYTE *)frame_ph, (MMP_UBYTE *)h264_ph, SingleEndPoint );
                            
                            MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(hdr),SKYPE_HEADER_LEN + SKYPE_HEADER_LEN + 4 + 4 , 0 );
                            // Both pipe can send...
                            cur_stream->pipe_sync = PIPE_NEXT_FRAME(0) | PIPE_NEXT_FRAME(1) ;
                        } else {
                            emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(ptt), 0,0);
                        }
                    }
                }
#endif  
                break;   
            case UVC_H264:
#if USB_UVC_H264
                {
                    MMP_UBYTE *jpeg_ptr,*jpeg_payload_ptr,*h264_ptr,*h264_ptr_s1=0;// ,*h264_payload_ptr ;
                    MMP_UBYTE *yuy2_ptr,*yuy2_payload_ptr;
                    
                    MMP_ULONG app3_insert_addr = 0,i,app3_search_len;
                    FRAME_PAYLOAD_HEADER *h264_ph,*h264_ph_s1=0;
                  
                    if( pipe0_cfg==PIPE_CFG_H264 )
                        h264_ptr = MMPF_Video_CurRdPtr(PIPE_0) ;
                    else {
                        h264_ptr = MMPF_Video_CurRdPtr(PIPE_1) ;
                        #if 0
                        if(h264_ptr[4+FRAME_PAYLOAD_HEADER_SZ]==0x67) {
                        dbg_printf(3,"[%x %x %x %x %x]\r\n",h264_ptr[0+FRAME_PAYLOAD_HEADER_SZ]
                                                           ,h264_ptr[1+FRAME_PAYLOAD_HEADER_SZ]
                                                           ,h264_ptr[2+FRAME_PAYLOAD_HEADER_SZ]
                                                           ,h264_ptr[3+FRAME_PAYLOAD_HEADER_SZ]
                                                           ,h264_ptr[4+FRAME_PAYLOAD_HEADER_SZ]);
                                                           
                        }
                        #endif
                        
                        #if H264_SIMULCAST_EN
                        if(UVCX_IsSimulcastH264()) {
                            h264_ptr_s1 = MMPF_Video_NextRdPtr(1,1) ;
                            if(h264_ptr_s1) {
                                h264_ph_s1 = (FRAME_PAYLOAD_HEADER *)( h264_ptr_s1 );
                            }
                        }
                        #endif
                    }
                    h264_ph = (FRAME_PAYLOAD_HEADER *)( h264_ptr );
                    // sean@2011_01_22, add yuy2 + h264
                    if(pipe0_cfg==PIPE_CFG_MJPEG) {
                        jpeg_ptr = (MMP_UBYTE *)frame_ph ;
                        jpeg_payload_ptr = (jpeg_ptr + FRAME_PAYLOAD_HEADER_SZ );
                        app3_search_len = ss->framelength/*glFrameLength*/ ;
                    } else {
                        jpeg_payload_ptr = (MMP_UBYTE *)gbJpegHeader;
                        app3_search_len = sizeof(gbJpegHeader);
                        yuy2_ptr = (MMP_UBYTE *)frame_ph ;
                        yuy2_payload_ptr = (yuy2_ptr + FRAME_PAYLOAD_HEADER_SZ );
                    }
                    //glFrameLength = frame_ph->UVC_H264_PH.dwPayloadSize ;
                    // Force 1st frame is Zero frame
                    if(ss->usbframecount==0) {
                    	emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr), 0 ,0);
                    } else {
                        MMP_BOOL send_frame = MMP_FALSE ;
                        //dbg_printf(0,"JPEG size : %d,H264 size :%d\r\n",ss->framelength,h264_ph->UVC_H264_PH.dwPayloadSize);
                        #if H264_SIMULCAST_EN
                        if( ss->framelength ) 
                        #else
                        if( ss->framelength && h264_ph->UVC_H264_PH.dwPayloadSize ) 
                        #endif
                        {
                            MMP_SHORT sid_s0=-1,sid_s1=-1;
                            
                            MMP_SHORT delay ;
                            MMP_ULONG off ;
                            MMP_USHORT pipe_sync = 0;
                            MMP_SHORT index_s0 = -1 ,index_s1 = -1;
                            //
                            // PTS use H264 payload header timestamp
                            //
                            if(h264_ph->UVC_H264_PH.dwPayloadSize) {
                                PTS_UVC[ep_id] = h264_ph->UVC_H264_PH.dwTimeStamp;
                                send_frame = MMP_TRUE ;
                                sid_s0 = UVC_PH_SID(h264_ph->UVC_H264_PH.dwFrameRate);
                                //h264_ph->UVC_H264_PH.dwFrameRate &=~UVC_PH_EXT_BIT_MASK ;
                                //dbg_printf(3,"sid_s0 :%d,%x\r\n",sid_s0,h264_ph->UVC_H264_PH.dwFrameRate);
                            }
                            
                            if (h264_ph_s1) {
                                if(h264_ph_s1->UVC_H264_PH.dwPayloadSize) {
                                    PTS_UVC[ep_id] = h264_ph_s1->UVC_H264_PH.dwTimeStamp;
                                    send_frame = MMP_TRUE ;
                                    sid_s1 = UVC_PH_SID(h264_ph_s1->UVC_H264_PH.dwFrameRate);
                                    //h264_ph_s1->UVC_H264_PH.dwFrameRate &=~UVC_PH_EXT_BIT_MASK ;
                                    //dbg_printf(3,"sid_s1 :%d,%x\r\n",sid_s1,h264_ph_s1->UVC_H264_PH.dwFrameRate);
                                }
                            }
                            #if USB_VS_WAIT_ALL_PIPE_FRAME==0
                            if( UVCX_IsSimulcastH264() ) {
                                send_frame = MMP_TRUE ;// For this, send local preview even h264 is not ready
                            }
                            #endif
                            

                            if(send_frame) {
                                
                                //dbg_printf(3,"PTS_UVC:%x\r\n",PTS_UVC);
                                if(pipe0_cfg==PIPE_CFG_MJPEG) {
                                    // Re-adjust Jpeg height if the input height is not multiple of 8
                                    usb_vs_align_jpeg_width_height( (MMP_UBYTE *)(jpeg_payload_ptr),app3_search_len,frame_ph->UVC_H264_PH.wImageWidth,frame_ph->UVC_H264_PH.wImageHeight);
                                }
                                app3_insert_addr = (MMP_ULONG)usb_vs_search_app3_insert_addr((MMP_UBYTE *)(jpeg_payload_ptr),app3_search_len,0xFFDD );
                                off = app3_insert_addr-(MMP_ULONG)jpeg_payload_ptr ;
                                
                                //dbg_printf(0,"JPEG size : %d,H264 size :%d\r\n",ss->framelength,h264_ph->UVC_H264_PH.dwPayloadSize);
                                // dbg_printf(0,"ver : %x,type:%x\r\n",h264_ph->UVC_H264_PH.wVersion,h264_ph->UVC_H264_PH.dwStreamType);
                                // Header blk
                                MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr      ), off,0);
                                
                                // H264 payload....
                                delay = (MMP_SHORT)usb_vc_diff_sof(glFrameStartSOF) ;
                                //dbg_printf(0,"H264.Seq.1:%d\r\n",h264_ph->UVC_H264_PH.dwFrameSeq);
                               // dbg_printf(0,"#stream delay:%d ms\r\n", delay);
                               
                                if(sid_s0!=-1/*h264_ph->UVC_H264_PH.dwPayloadSize*/) {
                                    MMP_ULONG h264_blk_size = h264_ph->UVC_H264_PH.dwPayloadSize ;
                                    MMP_ULONG tx_len , cur_addr,h264_off = 0;
                                    
                                    
                                    #if H264_CBR_PADDING_EN==1
                                    MMP_ULONG   cbr_padding_bytes =usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)h264_ph );
                                    h264_ph->UVC_H264_PH.dwPayloadSize += cbr_padding_bytes ;
                                    #endif
                                    
                                    h264_ph->UVC_H264_PH.dwFrameRate &=~UVC_PH_EXT_BIT_MASK ;
                                    
                                    index_s0 = sid_s0;//get_running_res_index(h264_ph->UVC_H264_PH.wImageWidth,h264_ph->UVC_H264_PH.wImageHeight);

                                        
                                    #if H264_POC_DUMP_EN    
                                        //dbg_printf(0,"<frseq:%d,%d\r\n",h264_ph->UVC_H264_PH.wReserved,gsH264FrameSeq);
                                        if( (h264_ph->UVC_H264_PH.wDelay)&&(h264_ph->UVC_H264_PH.wDelay - gsH264FrameSeq[index]) > 1 ) {
                                            dbg_printf(0,"  <H264 Lost Frame %d->%d,%d>\r\n",gsH264FrameSeq[index_s0],h264_ph->UVC_H264_PH.wDelay,h264_ph->UVC_H264_PH.wDelay - gsH264FrameSeq[index_s0]); 
                                            //MMPF_OS_Sleep(1000);
                                            //while(1);   
                                        }
                                        if(h264_ph->UVC_H264_PH.wDelay == gsH264FrameSeq[index_s0]) {
                                            dbg_printf(0,"Resend s1# : %d\r\n",gsH264FrameSeq[index_s0]);
                                        }
                                        
                                        gsH264FrameSeq[index_s0] = h264_ph->UVC_H264_PH.wDelay ;
                                        //dbg_printf(3,"s0_seq:%d\r\n",h264_ph->UVC_H264_PH.wDelay );
                                    #else
                                        if(delay) {
                                            h264_ph->UVC_H264_PH.wDelay = delay ;
                                        }
                                    #endif    
                                        cur_addr = (MMP_ULONG )h264_ptr;//h264_payload_ptr ;
                                        //dbg_printf(3,"#%d,size : %d\r\n",ss->usbframecount,h264_blk_size);
                                        h264_blk_size += FRAME_PAYLOAD_HEADER_SZ ;
                                        do {
                                            tx_len =( h264_blk_size > MAX_APP3_LEN ) ?MAX_APP3_LEN : h264_blk_size ;
                                            MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)(cur_addr + h264_off), tx_len,0 );
                                            h264_off += tx_len ;
                                            h264_blk_size -= tx_len ;
                                        } while ( h264_blk_size ) ;
                                        
                                        #if H264_CBR_PADDING_EN==1
                                        if(cbr_padding_bytes) {
                                            //dbg_printf(3,"vs.padding : %d\r\n",cbr_padding_bytes);
                                            do {
                                                tx_len =( cbr_padding_bytes > MAX_APP3_LEN ) ?MAX_APP3_LEN : cbr_padding_bytes ;
                                                MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)( usb_vs_get_cbr_padding_buffer() ), tx_len,DMA_CF_DUMMY );
                                                cbr_padding_bytes -= tx_len ;
                                            } while (cbr_padding_bytes);
                                        }
                                        #endif
                                        pipe_sync |= PIPE_NEXT_FRAME(1) ;
                                }
                                #if H264_SIMULCAST_EN
                                //if(h264_ph_s1 &&(h264_ph_s1->UVC_H264_PH.dwPayloadSize ) ) {
                                if( (sid_s1!=-1)&&(sid_s1!=sid_s0) ) {
                                    //short index;
                                    MMP_ULONG h264_blk_size = h264_ph_s1->UVC_H264_PH.dwPayloadSize ;
                                    MMP_ULONG tx_len , cur_addr,h264_off = 0;
                                    
                                    #if H264_CBR_PADDING_EN==1
                                    MMP_ULONG   cbr_padding_bytes =usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)h264_ph_s1 );
                                    h264_ph_s1->UVC_H264_PH.dwPayloadSize += cbr_padding_bytes ;
                                    #endif
                                    h264_ph_s1->UVC_H264_PH.dwFrameRate &=~UVC_PH_EXT_BIT_MASK ;
                                     
                                    index_s1 = sid_s1;//get_running_res_index(h264_ph_s1->UVC_H264_PH.wImageWidth,h264_ph_s1->UVC_H264_PH.wImageHeight);
                                    //if( index_s0==-1) {
                                    #if H264_POC_DUMP_EN    
                                    //dbg_printf(0,"<frseq:%d,%d\r\n",h264_ph->UVC_H264_PH.wReserved,gsH264FrameSeq);
                                    if( (h264_ph_s1->UVC_H264_PH.wDelay)&&(h264_ph_s1->UVC_H264_PH.wDelay - gsH264FrameSeq[index_s1]) > 1 ) {
                                        dbg_printf(0,"  <H264 Lost Frame %d->%d,%d>\r\n",gsH264FrameSeq[index_s1],h264_ph->UVC_H264_PH.wDelay,h264_ph->UVC_H264_PH.wDelay - gsH264FrameSeq[index_s1]); 
                                    }
                                    if(h264_ph->UVC_H264_PH.wDelay == gsH264FrameSeq[index_s1]) {
                                        dbg_printf(0,"Resend s1# : %d\r\n",gsH264FrameSeq[index_s1]);
                                    }

                                    gsH264FrameSeq[index_s1] = h264_ph->UVC_H264_PH.wDelay ;
                                    //dbg_printf(3,"s0_seq:%d\r\n",h264_ph->UVC_H264_PH.wDelay );
                                    #else
                                    if(delay) {
                                        h264_ph_s1->UVC_H264_PH.wDelay = delay ;
                                    }
                                    #endif    
                                    cur_addr = (MMP_ULONG )h264_ptr_s1;//h264_payload_ptr ;
                                    //dbg_printf(3,"#%d,size : %d\r\n",ss->usbframecount,h264_blk_size);
                                    h264_blk_size += FRAME_PAYLOAD_HEADER_SZ ;
                                    do {
                                        tx_len =( h264_blk_size > MAX_APP3_LEN ) ?MAX_APP3_LEN : h264_blk_size ;
                                        MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)(cur_addr + h264_off), tx_len,0 );
                                        h264_off += tx_len ;
                                        h264_blk_size -= tx_len ;
                                    } while ( h264_blk_size ) ;

                                    #if H264_CBR_PADDING_EN==1
                                    if(cbr_padding_bytes) {
                                        //dbg_printf(3,"vs.padding : %d\r\n",cbr_padding_bytes);
                                        do {
                                            tx_len =( cbr_padding_bytes > MAX_APP3_LEN ) ?MAX_APP3_LEN : cbr_padding_bytes ;
                                            MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)( usb_vs_get_cbr_padding_buffer() ), tx_len,DMA_CF_DUMMY );
                                            cbr_padding_bytes -= tx_len ;
                                        } while (cbr_padding_bytes);
                                    }
                                    #endif
                                    pipe_sync |= PIPE_NEXT_FRAME(2) ;
                                }
                                #endif
                                   
                                   
                                // sean@2011_01_22, YUY2 payload insert 
                                if(frame_ph) {
                                    if(frame_ph->UVC_H264_PH.dwPayloadSize&&pipe0_cfg==(PIPE_CFG_YUY2)) {
                                        
                                        MMP_ULONG yuy2_blk_size = frame_ph->UVC_H264_PH.dwPayloadSize ;
                                        MMP_ULONG tx_len,cur_addr,yuy2_off = 0;
                                        if(delay) {
                                            frame_ph->UVC_H264_PH.wDelay = delay ;
                                        }                            
                                       // dbg_printf(3,"yuy2 path\r\n");
                                        yuy2_blk_size += FRAME_PAYLOAD_HEADER_SZ ;
                                        cur_addr = (MMP_ULONG )yuy2_ptr;
                                        
                                        do {
                                            tx_len =( yuy2_blk_size > MAX_APP3_LEN ) ?MAX_APP3_LEN : yuy2_blk_size ;
                                            MMPF_Video_AddDMABlk(ep_id,4,(MMP_ULONG)(cur_addr + yuy2_off), tx_len,0 );
                                            yuy2_off += tx_len ;
                                            yuy2_blk_size -= tx_len ;
                                        } while ( yuy2_blk_size ) ;
                                        pipe_sync |= PIPE_NEXT_FRAME(0) ;           
                                   }
                                } else {
                                    
                                }                          
                                MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr + off), app3_search_len - off,0 );
                                pipe_sync |= PIPE_NEXT_FRAME(0) ;
                                cur_stream->pipe_sync = pipe_sync ;
                                
                            }
                            else {
                                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr      ), 0 ,0);
                            }                                
                            
                        } else {
                            // dbg_printf(0,"JPEG size : %d,H264.Seq.1:%d,size :%d\r\n",glFrameLength,h264_ph->UVC_H264_PH.dwFrameSeq,h264_ph->UVC_H264_PH.dwPayloadSize);
                       // Zero length frame or H264 length = 0
                            if(0/*pipe0_cfg==PIPE_CFG_MJPEG*/) {
                                // If JPEG is good , send JPEG
                                 MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr      ), ss->framelength/*glFrameLength*/ ,0);
                                 if(ss->framelength) {
                                    cur_stream->pipe_sync =  PIPE_NEXT_FRAME(0) ;
                                    if(frame_ph->UVC_H264_PH.dwPayloadSize) {
                                        cur_stream->pipe_sync |= PIPE_NEXT_FRAME(1) ;
                                    }
                                 }
                            } else {
                                emptyblk = MMPF_Video_AddDMABlk(ep_id,0,(MMP_ULONG)(jpeg_payload_ptr      ), 0 ,0);
                            }
                        }
                    }
                    
                }  
#endif                  
                break;                                     
            }
        } //if(m_VideoFmt

        //ss->tx_flag &= ~SS_TX_DMA_DONE ;
        ss->cursendptr = 0;
        ss->cursendsize = 0;
        ss->tx_flag &= ~SS_TX_LAST_PACKET;

    }//if(packetcount == 0) 
    if(ss->tx_flag & SS_TX_RT_MODE) {
        if( !usb_vs_get_rt_mode_ptr( ss ) ) {
            if(ss->packetcount & 1) {
                MMPF_OS_Sleep(1); // prevent lock
            }
            return UVC_SEND_IMG_RET_CONTINUE ;
        }
    }

#if USB_WORKING_MODE==USB_DMA_LIST_MODE
    dma_blk = MMPF_Video_CurBlk(ep_id);
    app3_header = MMPF_Video_GetBlkAppHeader(dma_blk) ;
    cur_pt = (volatile MMP_UBYTE *)usb_vs_move_packet(dma_blk,endbit,*sti_mode,app3_header ,&ss->cursendsize );
    if(cur_pt==0) {
        MMP_BOOL send_endbit = 1 ;
        if(UVCX_IsSliceBasedTxStream(cur_stream)  ) {
            send_endbit = (dma_blk->ctrl_flag & DMA_CF_EOF) ? 1 : 0 ;
        }
        if( ! MMPF_Video_NextBlk(ep_id,0) ) {
            endbit = BFH_BIT( BFH_EOS ) ;
            if(send_endbit) {
                endbit |= BFH_BIT( BFH_EOF ) ; //0x02 ;
                if(vidFmt == PCAM_USB_VIDEO_FORMAT_MJPEG){
                    if( usb_vs_check_incomplete_tx(ep_id) ) {
                        endbit |= BFH_BIT( BFH_ERR );
                    }
                }
            }    
            if( !(ss->tx_flag & SS_TX_BY_ONE_SHOT) ) {
            
                cur_pt = (volatile MMP_UBYTE *) usb_vs_empty_packet(dma_blk,endbit,*sti_mode) ;
                xsize = UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id];
                ss->tx_flag |= SS_TX_PRECLOSE_STREAMING ;
            } else {
                xsize = 0;
                ss->tx_flag |= (SS_TX_CLOSE_STREAMING | SS_TX_PRECLOSE_STREAMING) ;
            }
        } else {
            xsize = ss->cursendsize ; /*send_size ;*/
        } 
    } else {
        xsize = ss->cursendsize ;//send_size ;
    }
#endif

    
    if(cur_pt) {
        MMP_BOOL eof_pkt ;
        eof_pkt = usb_vs_send_packet((MMP_ULONG)cur_pt,xsize,ep_id);
        //dbg_printf(3,"send : %d,%x\r\n",xsize,endbit);
        if( eof_pkt && (ss->tx_flag & (SS_TX_BY_ONE_SHOT|SS_TX_RT_MODE) ) ) {
           ss->tx_flag |= SS_TX_PRECLOSE_STREAMING ;
           endbit = BFH_BIT( BFH_EOS ) | BFH_BIT( BFH_EOF ) ;
        }
        ss->packetcount++; 
    }
    //dbg_printf(3,"ss->packetcount:%d\r\n",ss->packetcount);
    
    if(endbit & BFH_BIT( BFH_EOS ) ) {
       // MMP_ULONG diff  = usb_vc_diff_sof(glDiffSend);
        if(endbit & BFH_BIT( BFH_EOF )) {
            //dbg_printf(3,"<EOF>:%d\r\n",usb_vc_cur_sof());
            
            //dbg_printf(3,"<EOF>\r\n");
            ss->frametoggle++ ;
            ss->frametoggle = ss->frametoggle & 1; 
            ss->usbframecount++;
            ss->STC = 0 ;
            if(ss->usbframecount == 20){
	            ISP_IF_AE_SetFastMode(0);
	            //RTNA_DBG_Str3("Disable AE Fast Mode\r\n");
            }
            if( (UVC_VCD()==bcdVCD15) && (gbCurH264Type==FRAMEBASE_H264)) {
                UVC15_GetH264StartStopLayer(gbSVCSSL_start,(MMP_UBYTE *)&gbSVCSSL_bitmap,(MMP_UBYTE *)gbSVCSSL_layer) ;
            }
            
        }
        else {
           // dbg_printf(3,"<EOS>\r\n");
        }
        ss->packetcount = 0;
    }  // end of if(endbit == 0x02)
    
    if( (endbit & BFH_BIT( BFH_EOS ) )&&(emptyblk==0) ){
        ss->frame_etime = MMPF_BSP_GetTick();
        //dbg_printf(3,"<NextFrame>\r\n");
        return UVC_SEND_IMG_RET_END_FRAME;
    }
    return UVC_SEND_IMG_RET_CONTINUE ;
}


MMP_ULONG usb_vs_request_dmabuf(MMP_UBYTE ep_id,STREAM_DMA_BLK *dma_blk)
{
static MMP_ULONG cpu_sr ;
extern MMP_UBYTE gDMAListBufCnt[];
    MMP_UBYTE i;//,buf_cnt[] = { EP1_DMA_LIST_BUF_CNT,EP2_DMA_LIST_BUF_CNT} ;
    volatile USB_DMA_DATA_INFO *data_info ;
    MMP_ULONG free_addr = 0,used_addr ;
    AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
    // Wait dma buffer...
    if( MMPF_USB_WaitDMAList( ep_id ) ) {
        return 0;
    }
    IRQ_LOCK(
        if(usb_cur_dma_addr(ep_id)==0) {
            for(i=0;i<gDMAListBufCnt[ep_id];i++) {
                data_info =(USB_DMA_DATA_INFO *)(dma_blk->dma_buf[i] + USB_DMA_DATA_INFO_OFFSET);
                if(data_info->dmalist_status==DMALIST_INVALID) {
                    data_info->dmalist_status =  DMALIST_BUSY;  
                    free_addr = dma_blk->dma_buf[i]  ;
                    break ;
                }
            }
        } else {
            used_addr = usb_cur_dma_addr(ep_id) - USB_DMA_DATA_LIST_OFFSET ;
            if(used_addr == dma_blk->dma_buf[0]) {
                free_addr = dma_blk->dma_buf[1] ;
            } else {
                free_addr = dma_blk->dma_buf[0] ;
            }
            data_info =(USB_DMA_DATA_INFO *)(free_addr + USB_DMA_DATA_INFO_OFFSET);
            data_info->dmalist_status =  DMALIST_BUSY; 
        }
    )
    if (free_addr==0) {
        dbg_printf(3,"Bad DMABuf addr:%x\r\n",free_addr);
        while(1);
    }
    //dbg_printf(3,"G[%d,%x]\r\n",ep_id,free_addr);
    return free_addr ;
}

MMP_BOOL usb_vs_release_dmabuf(MMP_UBYTE ep_id,MMP_ULONG dma_buf)
{
    static MMP_ULONG cpu_sr;
    MMP_BOOL ret = MMP_TRUE ;
    volatile USB_DMA_DATA_INFO *data_info ;
    //dbg_printf(3,"r");   
    IRQ_LOCK(
        data_info =(USB_DMA_DATA_INFO *)(dma_buf + USB_DMA_DATA_INFO_OFFSET); 
        if(data_info->dmalist_status!=DMALIST_BUSY) {
            ret = MMP_FALSE ;
        }
        data_info->dmalist_status = DMALIST_INVALID ;
    )
    MMPF_USB_ReleaseDMAList( ep_id );
    if( !ret ) {
        dbg_printf(3,"*R[%d,%x]\r\n",ep_id,dma_buf);
    } 
    return ret ;
    
    
}

// return 
// 1 : last packet
// 0 : continue
static MMP_ULONG usb_vs_move_packet(STREAM_DMA_BLK *dma_blk,MMP_UBYTE endbit,MMP_UBYTE sti_mode,MMP_UBYTE *pad_app3_header,MMP_ULONG *sent_size)
{
    #if (USB_WORKING_MODE==USB_DMA_LIST_MODE)
    volatile USB_DMA_DATA_INFO *data_info ;
    #endif
    MMP_UBYTE tx_mode ,ep,ep_id ;
    //MMP_USHORT *cur_size , max_dsize ;
    volatile MMP_UBYTE *cur_pt = 0;
    STREAM_SESSION *ss;
    MMP_ULONG  max_blk_size = 0;
    
    if(dma_blk->ctrl_flag & TX_BY_EP2) {
        ep = PCCAM_TX_EP2_ADDR;
        ep_id = 1 ;
    } else {
        ep = PCCAM_TX_EP1_ADDR;
        ep_id = 0 ;
    }
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    if(dma_blk->blk_size) {
        if(dma_blk->tx_len >= dma_blk->blk_size) {
        
        } else{
            cur_pt = (volatile MMP_UBYTE *)usb_vs_request_dmabuf(ep_id,dma_blk) ;
            if(!cur_pt) {
                return 0;
            }
        }
    }
    *sent_size = 0 ;
    // Double buffer need to push last packet out.
    if( (dma_blk->tx_len==0)&&(dma_blk->blk_size)) {
        if(pad_app3_header&&dma_blk->header_len) {
            int i;
            for(i=0;i<dma_blk->header_len;i++) {
                cur_pt[UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]+i] = pad_app3_header[i] ;
            }
        }
    }
    tx_mode = TX_MODE(dma_blk->ctrl_flag);
#if (CHIP==VSN_V2)||(CHIP==VSN_V3)
    {
    #if (USB_WORKING_MODE==USB_DMA_LIST_MODE)
        MMP_ULONG max_pkt_size = ( dma_blk->max_dsize /*+ UVC_VIDEO_PAYLOAD_HEADER_LEN*/);
        max_blk_size = ( ss->maxdmadatasize / max_pkt_size ) * max_pkt_size ;
    #endif
    #if (USB_WORKING_MODE==USB_DMA_MODE)
        max_blk_size = dma_blk->max_dsize ;
    #endif
        if(UVC_VCD()==bcdVCD15) {
            if(gbUsbHighSpeed) {
                ss->dma_timeout =   ( ( max_blk_size / max_pkt_size ) >> 3 );
            }
            else {
                ss->dma_timeout =  ( ( max_blk_size / max_pkt_size )  );
            }
            ss->dma_timeout += 2;//1;
        }
        else {
            ss->dma_timeout = 0 ;
        }
       // dbg_printf(3,"-DMA timeout : %d,%d\r\n",ep_id,ss->dma_timeout);
    }
#endif
    if(cur_pt) {
        usb_vs_fill_header(ep_id, cur_pt,
                           endbit | BFH_BIT(BFH_PTS) | BFH_BIT(BFH_SCR) | ss->frametoggle |( sti_mode ? BFH_BIT(BFH_STI) : 0 ),
                       PTS_UVC[ep_id],&ss->STC,&ss->SOFNUM,ss->uvc15_layerid);
    }                       
    // UVC header                        
    *sent_size += UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]  ;
    if(dma_blk->blk_size) {
        MMP_ULONG left_size =  dma_blk->blk_size - dma_blk->tx_len ;
        MMP_ULONG tx_size = 0 , tx_header_len = 0;
        
        if(dma_blk->tx_len >= dma_blk->blk_size) {
            //usb_vs_release_dmabuf(ep_id,(MMP_ULONG )cur_pt) ;
            return 0 ;
        }
        
        if(max_blk_size >= left_size ) {
            tx_size = left_size ;
        } else {
            tx_size = max_blk_size ;    
        }  
        if(!dma_blk->tx_len) {
            tx_header_len = dma_blk->header_len ;
            if(tx_header_len) {
                // - App3 marker
                //tx_size -= tx_header_len ;
                // Fixed the bug if tx_size < tx_header_len
                // The tx_size would go to a big value
                if((tx_size+tx_header_len) > max_blk_size/* dma_blk->max_dsize*/){
                    // to avoid transfer packet size > maximum payload size
                    tx_size = (max_blk_size/*dma_blk->max_dsize*/ - tx_header_len) ;
                }
                                    
            }
        } else {
            tx_header_len = 0 ;
        }
        #if (USB_WORKING_MODE == USB_DMA_MODE)
        usb_vs_trig_dma_move((MMP_ULONG)dma_blk->cur_addr, (MMP_ULONG)&cur_pt[UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]+tx_header_len], tx_size);
        #endif
            
        #if (USB_WORKING_MODE==USB_DMA_LIST_MODE)
        data_info = (USB_DMA_DATA_INFO *)(cur_pt + USB_DMA_DATA_INFO_OFFSET);
        data_info->dmalist_addr  = (MMP_ULONG)(cur_pt + USB_DMA_DATA_LIST_OFFSET);
        data_info->data_header_len = tx_header_len ;
        data_info->data_addr = dma_blk->cur_addr;
        data_info->max_dsize = dma_blk->max_dsize ;
        #endif   
        *sent_size += (tx_size + tx_header_len ) ; // + App3 marker
        
        #if H264_CBR_PADDING_EN==1
        data_info->dma_flag = dma_blk->ctrl_flag ;
        if(dma_blk->ctrl_flag & DMA_CF_DUMMY) {
             //dbg_printf(3,"dma_blk:%x\r\n",dma_blk->cur_addr);    
        }
        else {
            dma_blk->cur_addr+=tx_size ;
        }
        #else
        dma_blk->cur_addr+=tx_size ;
        #endif
        
        dma_blk->tx_len+=tx_size ;
        dma_blk->tx_packets++;
        if(ss->tx_flag & SS_TX_RT_MODE) {
            usb_vs_put_rt_mode_ptr(ss,tx_size );
        }
        return (MMP_ULONG)cur_pt;
    
    } 
    //usb_vs_release_dmabuf(ep_id,(MMP_ULONG )cur_pt) ;
    return 0;                                         
    
}
 
MMP_BOOL  usb_vs_trigger_next_dma(MMP_UBYTE ep_id)
{
    MMP_UBYTE endbit = 0 ;
    MMP_ULONG xsize ;
    STREAM_SESSION *ss;
    STREAM_DMA_BLK *dma_blk = MMPF_Video_CurBlk(ep_id);
    volatile MMP_UBYTE *cur_pt ;
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    cur_pt = (volatile MMP_UBYTE *)usb_vs_move_packet(dma_blk,endbit,0,0 ,&ss->cursendsize );
    if(cur_pt==0) {
        return 1; // Last packet.
    } 
    xsize = ss->cursendsize ;
    usb_vs_send_packet((MMP_ULONG)cur_pt,xsize,ep_id);
    ss->packetcount++; 
    return 0 ;
}
 
void usb_vs_next_packet(MMP_UBYTE ep_id)
{
    MMP_UBYTE ret;
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id) ;
    #if SUPPORT_PCSYNC
    if(!pcsync_is_preview()) 
    {
    #endif
        ss->tx_flag |= SS_TX_NEXT_PACKET ;
        if(ep_id==1){ // 2nd endpoint use usb fifo test, can be DMA or FIFO mode
            ret = MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_USB_EP2, MMPF_OS_FLAG_SET);
        } else {
            
            ret = MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_USB_EP1, MMPF_OS_FLAG_SET);
        }
    #if SUPPORT_PCSYNC
    }
    #endif
        
}

void usb_vs_next_frame(MMP_UBYTE ep_id,STREAM_CFG *cur_pipe,MMP_UBYTE pipe_cfg_format)
{
    MMP_UBYTE pipe,format;
    if(cur_pipe) {
        for(pipe=0;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
            if( PIPE_EN(cur_pipe->pipe_en) & (1 << pipe) ) {
                format = CUR_PIPE_CFG(pipe,cur_pipe->pipe_cfg) ;
                if(format==pipe_cfg_format) {
                    if( (!MMPF_Video_IsEmpty(pipe))  ) {
                        usb_vs_next_packet(ep_id);
                    }
                }
            }
        }    
    }
}


static MMP_ULONG usb_vs_empty_packet(STREAM_DMA_BLK *dma_blk,MMP_UBYTE endbit,MMP_UBYTE sti_mode)
{
#if (USB_WORKING_MODE==USB_DMA_LIST_MODE)
    volatile USB_DMA_DATA_INFO *data_info ;
#endif
    MMP_UBYTE ep,ep_id;
    MMP_ULONG  stc = 0 ;
    MMP_USHORT sof = 0 ;
    
    STREAM_SESSION *ss;
    volatile MMP_UBYTE *cur_pt ;
    if(dma_blk->ctrl_flag&TX_BY_EP2) {
        ep = PCCAM_TX_EP2_ADDR ;
        ep_id = 1 ;
    } else {
        ep = PCCAM_TX_EP1_ADDR;
        ep_id = 0 ;
    }
    ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    cur_pt = (volatile MMP_UBYTE *)usb_vs_request_dmabuf(ep_id,dma_blk) ;
    if(cur_pt) {
        usb_vs_fill_header(ep_id, cur_pt,
                            endbit | BFH_BIT(BFH_PTS) | BFH_BIT(BFH_SCR) | ss->frametoggle |( sti_mode ? BFH_BIT(BFH_STI) : 0 ),
                            PTS_UVC[ep_id], &stc,&sof ,ss->uvc15_layerid);
    #if (USB_WORKING_MODE==USB_DMA_LIST_MODE)
        {
            USB_DMA_DATA_INFO *data_info = (USB_DMA_DATA_INFO *)(cur_pt + USB_DMA_DATA_INFO_OFFSET );
            data_info->dmalist_addr  = (MMP_ULONG)(cur_pt + USB_DMA_DATA_LIST_OFFSET);
            data_info->data_header_len = 0 ;
            data_info->data_addr = 0;
            data_info->max_dsize = 0 ;
        }
    #endif
        dma_blk->tx_packets++;
    }
    return (MMP_ULONG)cur_pt ;
} 

#pragma arm section code , rwdata ,  zidata 


/*
return 
0: Continue
1: EOF
*/
MMP_BOOL usb_vs_send_packet(MMP_ULONG cur_pt,MMP_ULONG xsize,MMP_UBYTE ep_id) 
{
//extern MMP_BOOL MMPF_USB_WaitDMA(MMP_UBYTE ep_id) ;
#if 0//USB_UVC_BULK_EP
    MMP_USHORT r,d,a;
#endif
    
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
   // MMP_USHORT tx_size = ss->maxpacketsize ;//UVCTxFIFOSize[ep_id] ;
    MMP_USHORT ep ;
    MMP_UBYTE tx_mode ;
    MMP_BOOL  abort ;
    
    if(ep_id==0) {
        ep = PCCAM_TX_EP1_ADDR ;
    } else {
        ep = PCCAM_TX_EP2_ADDR ;
    }
    
#if USB_UVC_BULK_EP        
    if(xsize < 0x200) {
        UsbDmaTxConfig(ep, (MMP_ULONG)cur_pt, 0x0, xsize - 1, 0x0, 0x0);
    } 
    else {
        r = xsize % 0x200;
        d = xsize >> 9;
        a = r ? 1 : 0;
        if(r == 0)
            r = 0x200;
        UsbDmaTxConfig(ep, (MMP_ULONG)cur_pt, 0x1FF, r - 1, d - 1 + a, 0x0);
    }
#else  
    return UsbISODmaListTxConfig(ep_id,(MMP_ULONG)cur_pt,xsize,USB_DMA_EN);
#endif
}

static void usb_vs_advance_to_lastframe(STREAM_CFG *cur_stream,MMP_UBYTE pipe)
{
    STREAM_CTL  *cur_bctl ;
    OS_CPU_SR   cpu_sr = 0;
    //
    // Always get the latest frame of YUY2 or MJPEG
    //
    // Lock IRQ when update the pointer
    IRQ_LOCK(
        
        cur_bctl = &cur_stream->pipe_b[pipe];
        
        if(cur_bctl->wr_index == 0){
            cur_bctl->rd_index = (cur_bctl->slot_size * (cur_bctl->slot_num-1)) ;
            cur_bctl->total_rd = cur_bctl->total_wr - 1;
        }
        else{
            cur_bctl->rd_index = cur_bctl->wr_index - cur_bctl->slot_size ;
            cur_bctl->total_rd = cur_bctl->total_wr - 1;
        }
    )

}

static void usb_vs_align_jpeg_width_height(MMP_UBYTE *jpeg,MMP_ULONG len,MMP_USHORT new_w,MMP_USHORT new_h)
{
    MMP_USHORT tagID=0,tagLength=0 ;
    MMP_UBYTE  *jpegBuf = jpeg ;
    if( ( (new_h & 7)==0) && ((new_w & 15)==0) ) {
        return ;
    }
    do {
        if(!MMPF_GetJfifTag( &tagID ,&tagLength , &jpegBuf , len ) ) {
            if(tagID==0xFFC0) {
                break;
            } 
            len = len - tagLength - 2 ;
            jpegBuf += (tagLength + 2);
       }
    } while( len > 0);
    jpegBuf[5] = (new_h >> 8  );
    jpegBuf[6] = (new_h & 0xFF);
    jpegBuf[7] = (new_w >> 8  );
    jpegBuf[8] = (new_w & 0xFF);
   // dbg_printf(3,"jpeg h : %d,new_h : %d\r\n",(jpegBuf[5] << 8) | jpegBuf[6],new_h );
   // dbg_printf(3,"jpeg w : %d,new_w : %d\r\n",(jpegBuf[7] << 8) | jpegBuf[8],new_h );

}


static MMP_UBYTE *usb_vs_search_app3_insert_addr(MMP_UBYTE *jpeg,MMP_ULONG len,MMP_USHORT tag)
{
    MMP_USHORT tagID=0,tagLength=0 ;
    MMP_UBYTE  *jpegBuf = jpeg ;
    //sean@2011_09_28
    int  applen = 0 ;
    do {
        if(!MMPF_GetJfifTag( &tagID ,&tagLength , &jpegBuf , len ) ) {
            len = len - tagLength - 2 ;
            jpegBuf += (tagLength + 2);
       }
    } while(tagID != tag && len > 0); 
    
    
    //sean@2011_09_28
    applen = (MMP_ULONG)jpegBuf - (MMP_ULONG)jpeg ;
    if(applen > 0x300) {
        while(1) {
            dbg_printf(0,"Bad App4 off : %d,%d\r\n",applen,len);
        }
    }
    //---------------
    return jpegBuf ;        
}

volatile MMP_BOOL    m_bUvcDmaMoveWorking = MMP_FALSE;
static MMP_ERR usb_vs_wait_dma_done(MMPF_DMA_M_ID DMA_ID,MMP_ULONG timeout) // timeout is loop count
{
    #if 1 //(CHIP == VSN_V2)
    MMP_ULONG dma_timeout = 0 ;
    do {
        if (m_bUvcDmaMoveWorking == MMP_FALSE)
            break;
    } while (dma_timeout++ < timeout);

    if(dma_timeout >= timeout) {
       // AITPS_DMA   pDMA = AITC_BASE_DMA;
       // MMP_USHORT  DMA_INT = DMA_INT_R0 ;
/*
        RTNA_DBG_PrintShort(0,pDMA->DMA_INT_CPU_EN);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_SRC_ADDR);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_DST_ADDR);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_SRC_OFST);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_DST_OFST);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_PIX_W);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_PIX_H);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_CTL);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_MIRROR_EN);
*/
        RTNA_DBG_Str(0, "---DMA rotate timeout---\r\n");
        return MMP_DMA_ERR_BUSY ; 
    }
    #endif

    #if 0 //(CHIP == P_V2)
    #if 1 //DMA_R
    AITPS_DMA   pDMA = AITC_BASE_DMA;   
    MMP_ULONG dma_timeout = 0 ;
    MMP_USHORT DMA_INT = DMA_INT_R0;

    do {
        if(pDMA->DMA_INT_CPU_SR & DMA_INT) {
			pDMA->DMA_INT_CPU_SR = DMA_INT;
            break;
        }
    } while (dma_timeout++ < timeout ) ;
    if(dma_timeout >= timeout) {
        AITPS_DMA   pDMA = AITC_BASE_DMA;
        MMP_USHORT  DMA_INT = DMA_INT_R0 ;
        RTNA_DBG_PrintShort(0,pDMA->DMA_INT_CPU_EN);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_SRC_ADDR);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_DST_ADDR);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_SRC_OFST);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_DST_OFST);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_PIX_W);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_PIX_H);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_CTL);
        RTNA_DBG_PrintLong(0,pDMA->DMA_R[DMA_ID].DMA_R_MIRROR_EN);

        RTNA_DBG_Str(0, "---DMA rotate timeout---\r\n");
        return MMP_DMA_ERR_BUSY ; 
    }
    #else
    AITPS_DMA   pDMA = AITC_BASE_DMA;   
    MMP_ULONG dma_timeout = 0 ;
    MMP_USHORT DMA_INT = DMA_INT_M1 ;
    
    /*if(DMA_ID == MMPF_DMA_M_1) {
    	 DMA_INT = DMA_INT_M0 ;
    	 DMA_ID = MMPF_DMA_M_0;
    }*/
    if(DMA_ID==MMPF_DMA_M_0) {
        DMA_INT = DMA_INT_M0 ;
    }
    do {
        if((pDMA->DMA_INT_CPU_SR & DMA_INT)|
        ((pDMA->DMA_INT_CPU_EN & DMA_INT)==0)&&((pDMA->DMA_M[DMA_ID].DMA_M_LEFT_BYTE)==0)) {
            pDMA->DMA_INT_CPU_SR = DMA_INT;
            break;
        }
    } while (dma_timeout++ < timeout ) ; 

    if(dma_timeout >= timeout) {
        RTNA_DBG_Short(0,pDMA->DMA_INT_CPU_EN);
        RTNA_DBG_Long(0,pDMA->DMA_M[DMA_ID].DMA_M_SRC_ADDR);
        RTNA_DBG_Long(0,pDMA->DMA_M[DMA_ID].DMA_M_DST_ADDR);
        RTNA_DBG_Long(0,pDMA->DMA_M[DMA_ID].DMA_M_BYTE_CNT);
        RTNA_DBG_Long(0,pDMA->DMA_M[DMA_ID].DMA_M_LEFT_BYTE);
        RTNA_DBG_Str(0, "---DMA move timeout---\r\n");
        return MMP_DMA_ERR_BUSY ; 
    }
    #endif
    #endif

    return MMP_ERR_NONE ;
}

static void _dma_status_callback (void)
{
    m_bUvcDmaMoveWorking = MMP_FALSE;
}

static MMP_ERR usb_vs_trig_dma_move (MMP_ULONG ulSrcaddr, MMP_ULONG ulDstaddr,MMP_ULONG ulCount)
{
    if (m_bUvcDmaMoveWorking == MMP_TRUE) {
        RTNA_DBG_Str(0, "uvc dma busy\r\n");
        return MMP_DMA_ERR_BUSY;
    }

    m_bUvcDmaMoveWorking = MMP_TRUE;

    #if (CHIP == VSN_V2)||(CHIP==VSN_V3)
    MMPF_DMA_RotateData0(ulSrcaddr, ulDstaddr, ulCount, ulCount, 1,
                    DMA_R_BPP_8|DMA_R_BLK_16X16, DMA_R_MIRROR_DISABLE,
                    _dma_status_callback);
    #endif

    #if (CHIP == P_V2)
    MMPF_DMA_MoveData1(ulSrcaddr, ulDstaddr, ulCount, _dma_status_callback);
    #endif

    return MMP_ERR_NONE;
}

static void init_running_res(void)
{
    int i ;
    for(i=0;i<sizeof(gsRunningRes) / sizeof(RES_TYPE_CFG) ;i++) {
        gsRunningRes[i].res_type = (RES_TYPE_LIST)-1;
        gsRunningRes[i].res_w =0;
        gsRunningRes[i].res_h =0;
    }
}

static short get_running_res_index(unsigned short w, unsigned short h)
{
	int i;
	//dbg_printf(3,"[w,h]=(%d,%d)\r\n",w,h);
	for(i=0;i<sizeof(gsRunningRes) / sizeof(RES_TYPE_CFG);i++) {
		if(gsRunningRes[i].res_type!=(RES_TYPE_LIST)-1) {
			if( (gsRunningRes[i].res_w==w) &&(gsRunningRes[i].res_h==h) ) {
				return (short)gsRunningRes[i].res_type ;
			} 
		} else {
			gsRunningRes[i].res_w = w ;
			gsRunningRes[i].res_h = h ;
			gsRunningRes[i].res_type = i;
			return (short)gsRunningRes[i].res_type ;
		}
	}
	return -1 ;
}

#if H264_SIMULCAST_EN
MMP_USHORT UVC_VIDEO_PAYLOAD_HEADER_LEN[2] ;
#endif
static volatile MMP_UBYTE *usb_vs_fill_header(MMP_UBYTE ep_id,volatile MMP_UBYTE *ptr,MMP_UBYTE bfh,MMP_ULONG pts,MMP_ULONG *stc,MMP_USHORT *sofnum,MMP_USHORT layerid)
{
extern MMP_ULONG glCPUFreq ;
    UVC_VIDEO_PAYLOAD_HEADER *pUVCHeader = (UVC_VIDEO_PAYLOAD_HEADER *)ptr ;
    
#if 1
    // Get current system clock
    // UVC 1.5 STC keep same within the same frame.
    if(*stc==0) {
        // 2012_04_26, use frame number from register instead of SOFNUM 
        *sofnum = usb_vc_get_sof_ticks(); // 11 bits
        //*stc = (*sofnum) * /*glCPUFreq*/USB_STC_CLK + MMPF_BSP_GetTickClock() ;
        *stc = MMPF_BSP_GetTick()*USB_STC_CLK_KHZ + MMPF_BSP_GetTickClock() ;
    }
#endif  
    pUVCHeader->bHLE = UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] ;
    pUVCHeader->bBFH = bfh ;
    
    pUVCHeader->bPTS[0] =  pts & 0xFF ;
    pUVCHeader->bPTS[1] = (pts >> 8 )& 0xFF ;
    pUVCHeader->bPTS[2] = (pts >> 16)& 0xFF ;
    pUVCHeader->bPTS[3] = (pts >> 24)& 0xFF ;
    pUVCHeader->bSCR[0] =  *stc & 0xFF ;
    pUVCHeader->bSCR[1] = (*stc >> 8 )& 0xFF ;
    pUVCHeader->bSCR[2] = (*stc >> 16)& 0xFF ;
    pUVCHeader->bSCR[3] = (*stc >> 24)& 0xFF ;
    pUVCHeader->bSOFNUM[0] = (*sofnum >> 0)& 0xFF ;
    pUVCHeader->bSOFNUM[1] = (*sofnum >> 8)& 0xFF ;
    if( (UVC_VCD()==bcdVCD15) &&(ep_id==STREAM_EP_H264)) {
        pUVCHeader->bSLI[0] = (layerid >> 0)& 0xFF ;
        pUVCHeader->bSLI[1] = (layerid >> 8)& 0xFF ;
    } 
    //dbg_printf(3,"[%x]uvcflag:%x\r\n",ptr,bfh);
    return ptr + pUVCHeader->bHLE ;

}


static MMPF_TIMER_ID gbVsTimerId = MMPF_TIMER_1 ;
static MMP_BOOL gbVsSendByTimer = 0,gbVsTimer1stEn = 0;
static MMP_USHORT gsVsSendByTimerFpsx10 ;
static MMP_USHORT gsVsSendByTimerInitialFpsx10 ;
static void usb_vs_timer_isr(void) ;

void usb_vs_initial_fps(MMP_USHORT fpsx10)
{
    MMP_ULONG yuy2_addr ;
    gsVsSendByTimerInitialFpsx10 = fpsx10 ;
    gbVsTimer1stEn = MMP_FALSE;
    dbg_printf(3,"Timer Sending FPS : %d,%d\r\n",gsVsSendByTimerInitialFpsx10,gbVsTimer1stEn);
}

void usb_vs_timer_open(MMP_USHORT fpsx10,MMPF_TIMER_ID id)
{
    MMP_ULONG us = ( 1000*1000*10 / fpsx10 ) ;
    gbVsTimerId = id ;
    MMPF_Timer_OpenUs(id, us , usb_vs_timer_isr );
    MMPF_Timer_OpenInterrupt( id );
    MMPF_Timer_EnableInterrupt( id , MMP_FALSE /*MMP_TRUE*/);
    
    gbVsSendByTimer = 1;
    gsVsSendByTimerFpsx10 = fpsx10;
    //dbg_printf(3,"VS by timer fpsx10 : %d,init fpsx10 : %d\r\n",gsVsSendByTimerFpsx10,gsVsSendByTimerInitialFpsx10);
}

void usb_vs_timer_close(MMPF_TIMER_ID id)
{
    MMPF_Timer_EnableInterrupt(id, MMP_FALSE);
    MMPF_Timer_Close(id);
    
    gbVsSendByTimer = 0;
}

void usb_vs_timer_1stenable(MMPF_TIMER_ID id)
{
    STREAM_SESSION *ss = (STREAM_SESSION *)MMPF_Video_GetStreamSessionByEp(0);
    if( (ss->tx_flag & SS_TX_STREAMING) && !(ss->tx_flag & SS_TX_RT_MODE) ) {
        if(gbVsSendByTimer==0) {
            dbg_printf(3,"#WT,");        
        } else {
            if(gbVsTimer1stEn==MMP_FALSE) {
                dbg_printf(3,"<++>Timer1 on..\r\n");
                usb_vs_timer_enable(id,MMP_TRUE);
                gbVsTimer1stEn= MMP_TRUE ;
            }
        }
    }
}


void usb_vs_timer_enable(MMPF_TIMER_ID id,MMP_BOOL en)
{
    if(gbVsSendByTimer) {
        MMPF_Timer_EnableInterrupt( id , en );
    }
}

void usb_vs_timer_isr(void)
{
//extern volatile MMP_ULONG uSOFNUM ;
extern MMPF_OS_SEMID    gSyncFrameSem ;
extern MMP_USHORT gsCurFrameRatex10[];
static MMP_BOOL gbRestoreFps = 0;
    MMP_SHORT realfpsx10 = ISP_IF_AE_GetRealFPSx10();
    MMP_SHORT newfpsx10 ;
    
    if(gbVsSendByTimer) {
        STREAM_SESSION *ss = (STREAM_SESSION *)MMPF_Video_GetStreamSessionByEp(0);
        STREAM_CFG *cur_stream ,cur_stream_tmp = *usb_get_cur_image_pipe_by_epid(0);
       
        cur_stream = &cur_stream_tmp ;
        cur_stream->pipe_en &= ~PIPEEN(GRA_SRC_PIPE);
        dbg_printf(3,"timer fps : %d, sensor fps : %d\r\n",gsVsSendByTimerFpsx10,realfpsx10);
        if(ss->tx_flag & SS_TX_STREAMING) {
            if(gsVsSendByTimerFpsx10 > realfpsx10) {
                usb_vs_timer_close(gbVsTimerId);
                usb_vs_timer_open(realfpsx10,gbVsTimerId);
                usb_vs_timer_enable(gbVsTimerId,MMP_TRUE);
                
                gsCurFrameRatex10[0] = realfpsx10 ;

                gbRestoreFps = 1; 
            } else {
                if( (gbRestoreFps) && (realfpsx10 > gsVsSendByTimerFpsx10) ) {
                
                    usb_vs_timer_close(gbVsTimerId);
                    usb_vs_timer_open(gsVsSendByTimerInitialFpsx10,gbVsTimerId);
                    usb_vs_timer_enable(gbVsTimerId,MMP_TRUE);
                    gsCurFrameRatex10[0] = gsVsSendByTimerInitialFpsx10 ; 
                    
                    gbRestoreFps = 0;
                } else {
                    MMP_UBYTE pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_stream->pipe_cfg);
                    MMP_UBYTE pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_stream->pipe_cfg);
                    if( IS_PIPE0_EN(cur_stream->pipe_en ) &&(pipe0_cfg!=PIPE_CFG_H264)){
                        //dbg_printf(3,"tx_flag:%x,t : %d\r\n",ss->tx_flag,usb_vc_cur_sof());
                        MMPF_USB_ReleaseDm(PIPE_0);
                    }
                    
                    if( IS_PIPE1_EN(cur_stream->pipe_en ) &&(pipe1_cfg!=PIPE_CFG_H264)){
                        MMPF_USB_ReleaseDm(PIPE_1);
                    }
                }
            }
        }
    }
}

MMP_BOOL usb_vs_sendbytimer(void)
{
    return gbVsSendByTimer ;
}


#if OMRON_FDTC_SUPPORT
static void usb_vs_draw_face_pos(MMP_ULONG frame_addr,MMP_USHORT frame_w)
{
#define BLK 0
#define WHT (0xfc00)
	static MMP_UBYTE cntFDTC = 0;
	MMP_BYTE *resultFDTC;
	MMP_ULONG idx = 0, pos[4], j;
	MMP_LONG lSize = 0;
	
	gTextColorFormat = ISP_PREVIEW_FORMAT_YUV420;
	gTextPreviewWidth = frame_w;
	gTextPreviewBufAddr = (MMP_ULONG)(frame_addr);

	lSize = GetFacePostion((void**)(&resultFDTC));
	
	for (idx = 1; idx < lSize; idx += 8) {
		pos[0] = *(resultFDTC+ idx + 0) + ((MMP_USHORT)(*(resultFDTC + idx + 1)) << 8);	//LeftTopX
		pos[1] = *(resultFDTC+ idx + 2) + ((MMP_USHORT)(*(resultFDTC + idx + 3)) << 8);	//LeftTopY
		pos[2] = *(resultFDTC+ idx + 4) + ((MMP_USHORT)(*(resultFDTC + idx + 5)) << 8);	//RightBottomX
		pos[3] = *(resultFDTC+ idx + 6) + ((MMP_USHORT)(*(resultFDTC + idx + 7)) << 8);	//RightBottomY
		
		if(pos[0]>0xFEFF) pos[0]=0;
		if(pos[1]>0xFEFF) goto drawX_TOP;
		for (j=pos[0];j<pos[2];j++) {
			VR_PrintString((ISP_INT8*)"|",  pos[1], j, WHT, BLK);//-	LeftTop to RightTop
			if (j==640) {
				pos[2]=640;
				goto drawX_TOP;
			}
		}
		drawX_TOP:{if(pos[2]>640) pos[2]=640;}
		
		if(pos[1]>0xFEFF) pos[1]=0;
		if(pos[0]==0) goto drawY_Left;
		for (j=pos[1];j<pos[3];j++) {
			VR_PrintString((ISP_INT8*)"-",  j, pos[0], WHT, BLK);//|	LeftTop to LeftBottom
			if (j==480) {
				pos[3]=480;
				goto drawY_Left;
			}
		}
		drawY_Left:{if(pos[3]>480) pos[3]=480;}
		
		if(pos[3]==480) goto drawX_Bottom; 
		for (j=pos[0];j<pos[2];j++) {
			VR_PrintString((ISP_INT8*)"|",  pos[3], j, WHT, BLK);//-	LeftBottom to RightBottom
		}
		drawX_Bottom:{}
		
		if(pos[2]==640) goto drawY_Right;
		for (j=pos[1];j<pos[3];j++) {
			VR_PrintString((ISP_INT8*)"-",  j, pos[2], WHT, BLK);//|	RightTop to RightBottom
		}
		drawY_Right:{}
	}

}
#endif

MMP_BOOL out_delay_line = 0;
void usb_vs_monitor_icon_dline(void)
{
#define CHECK_ICON_DLINE    (1)
#if CHECK_ICON_DLINE
    AITPS_ICOB  pICOB = AITC_BASE_ICOB;  
    //dbg_printf(3,"<DL_CFG> = [%x,%x,%x]\r\n",  pICOB->ICO_DLINE_CFG[0]    , pICOB->ICO_DLINE_CFG[1]    , pICOB->ICO_DLINE_CFG[2]    );
    if(out_delay_line) {
        dbg_printf(3,"<DL_USE> = [%x,%x,%x]\r\n",  pICOB->ICO_DLINE_MAX_USE[0], pICOB->ICO_DLINE_MAX_USE[1], pICOB->ICO_DLINE_MAX_USE[2]);
    }
#endif
}

MMP_ULONG usb_vs_get_rt_mode_ptr(STREAM_SESSION *ss)
{
   // MMP_UBYTE *buf_addr = (MMP_UBYTE *)MMPF_Video_CurRdPtr(ss->pipe_id);
    AITPS_IBC  pIBC = AITC_BASE_IBC ;
    AITPS_IBCP pIBCP ;
    int cur_size ;
    if(ss->pipe_id==PIPE_0) {
        pIBCP = &(pIBC->IBCP_0) ;
    }
    else if (ss->pipe_id==PIPE_1) {
        pIBCP = &(pIBC->IBCP_1) ;
    }
    else {
        pIBCP = &(pIBC->IBCP_2) ;
    }

    if(pIBCP->IBC_WR_PTR < (MMP_ULONG)ss->cursendptr) {
        cur_size = (int) (ss->framelength - (MMP_ULONG)ss->cursendptr); 
        //dbg_printf(3,"cur_size = %d,maxdmadatasize : %d\r\n", cur_size ,ss->maxdmadatasize);
    } else {
        cur_size = (int) ( pIBCP->IBC_WR_PTR - (MMP_ULONG)ss->cursendptr); 
        if(cur_size < ss->maxdmadatasize) {
            // For small resolution
            if(ss->framelength) {
                if(cur_size >= ss->framelength) {
                    return ss->framelength ;
                }
            }
            return 0 ;
        }
    }
    
    return cur_size ;
    
}

void usb_vs_put_rt_mode_ptr(STREAM_SESSION *ss,MMP_ULONG size)
{
    ss->cursendptr += size ;
    //dbg_printf(3,"ss->cursendptr : %x\r\n",ss->cursendptr);
}

MMP_BOOL usb_vs_check_incomplete_tx(MMP_UBYTE ep_id)
{
    MMP_USHORT csr ;
    if(ep_id==0) {
        csr = UsbReadTxEpCSR(PCCAM_TX_EP1_ADDR);
    }
    else {
        csr = UsbReadTxEpCSR(PCCAM_TX_EP2_ADDR);
    }    
 
    if(csr & TX_INCOMPTX_BIT) {
        dbg_printf(3,"--incomplete-tx[%d]\r\n",ep_id);
        csr &= ~TX_INCOMPTX_BIT ;
        UsbWriteTxEpCSR( (ep_id==0)?PCCAM_TX_EP1_ADDR:PCCAM_TX_EP2_ADDR , csr);
        return MMP_TRUE ;
    }  
    return FALSE;
}

#if H264_CBR_PADDING_EN==1
void usb_uvc_fill_cbr_padding_bytes(MMP_UBYTE *frame_ptr,MMP_ULONG bytes) 
{
    struct _UVC_H264_PH *header = (struct _UVC_H264_PH *)frame_ptr ;
    //dbg_printf(3,"__dummy_b:%d\r\n",bytes);
    header->wVersion   = ( bytes & 0xFFFF        );
    header->wHeaderLen = ((bytes >> 16) & 0xFFFF );
}

MMP_ULONG usb_uvc_get_cbr_padding_bytes(MMP_UBYTE *frame_ptr) 
{
    MMP_ULONG padding = 0 ;
    struct _UVC_H264_PH *header = (struct _UVC_H264_PH *)frame_ptr ;
    
    padding = (header->wVersion ) | (header->wHeaderLen << 16);
    

    header->wVersion   = UVC_H264_VER;
    header->wHeaderLen = UVC_H264_PH_LEN ;
    
    return padding ;
}


void usb_vs_zero_cbr_padding_buffer(void)
{
static MMP_BOOL zero_buf_inited[2] = {0,0} ;
    MMP_USHORT i ,size = H264_CBR_PADDING_BS ;
    MMP_ULONG st,et,*ptr = (MMP_ULONG *)glUSB_UVC_ZERO_BUF_ADDR ;
    MMP_UBYTE ep_id = (UVC_VCD()==bcdVCD15)?1:0 ;
    
    if(!zero_buf_inited[ep_id] ) {
        st = usb_vc_cur_usof();
        for( i = 0 ; i < ( size >> 2 );i++) {
            ptr[i] = 0 ;
        }
        zero_buf_inited[ep_id] = MMP_TRUE;
        et = usb_vc_cur_usof();
        dbg_printf(3,"CBR-bitstuffing buffer..zero it\r\n");
    }
}


MMP_ULONG usb_vs_get_cbr_padding_buffer(void)
{
    return glUSB_UVC_ZERO_BUF_ADDR;
}
#endif

