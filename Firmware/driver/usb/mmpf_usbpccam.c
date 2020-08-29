#include "includes_fw.h"
#include "lib_retina.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"

#if 1
#include "mmpf_sensor.h"
#include "mmpf_scaler.h"
#include "mmpf_usbpccam.h"
//#include "mmpf_fs_api.h"
#include "mmp_reg_jpeg.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_vif.h"
//#include "3A_ctl.h"
//#include "preview_ctl.h"

#if 1//(CHIP == P_V2) || (CHIP == VSN_V2)
#include "mmp_reg_ibc.h"
#include "mmp_reg_icon.h"
#endif
#include "mmps_3gprecd.h"
#include "mmpf_usb_h264.h"
#include "mmpf_usbuvc.h"

#include "mmpf_mp4venc.h"
#include "mmp_reg_usb.h"
#include "mmpd_system.h"
#include "mmpf_usbuvc_event.h"

#include "mmps_system.h"
#include "mmpf_usbuvch264.h"
#include "mmpf_h264enc.h"

#if RC_JPEG_TARGET_SIZE > 0
extern RC_CONFIG_PARAM RcConfig;
void *jpeg_rc_hdl ;
#endif
#if 1//RC_JPEG_TARGET_SIZE > 0
MMP_BOOL    gbEnableJpegRC ;
#endif

MMP_ULONG   glPCCamCompressBufAddr;
MMP_ULONG   glPCCamCompressBufSize;
MMP_ULONG   glPCCamCurQCtl;
//MMP_ULONG   glPccamResolution;
//MMP_ULONG   glPCCamJpegSize;
MMP_ULONG   glPCCamCnt0 = 0;
MMP_ULONG   glPCCamCnt1 = 0;

#if 1//OMRON_FDTC_SUPPORT
MMP_BYTE	gbFDTCDone = 0;
MMP_BYTE	gbFDTCUpdate[2] = {0};
#endif

MMP_USHORT  m_usZoomResolution[PIPE_MAX]  ={ 80,80,80 };


#if USB_UVC_SKYPE
extern MMP_UBYTE gbSkypeMode; 
extern MMP_UBYTE gbSkypeEncRes;
#endif

extern	MMP_ULONG	m_glISPBufferStartAddr;
extern MMPF_SENSOR_FUNCTION *gsSensorFunction;
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_UBYTE gbStillCaptureEvent;
//extern MMP_UBYTE usb_fb_toggle;
//extern PCAM_USB_VIDEO_FORMAT gsVidFmt;
//extern void MMPF_USB_ReleaseDm(MMP_UBYTE );

extern MMP_USHORT  gsVideoFormat;

extern MMP_UBYTE  gbUsbHighSpeed;
extern MMP_UBYTE gbCurH264EncId ;

const   MMP_UBYTE Q_table[3][128] = {
// high quality
0x08,0x06,0x06,0x06,0x06,0x06,0x08,0x08,
0x08,0x08,0x0c,0x08,0x06,0x08,0x0c,0x0e,
0x0a,0x08,0x08,0x0a,0x0e,0x10,0x0c,0x0c,
0x0e,0x0c,0x0c,0x10,0x10,0x10,0x12,0x12,
0x12,0x12,0x10,0x14,0x14,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,

0x08,0x0a,0x0a,0x10,0x0e,0x10,0x14,0x14,
0x14,0x14,0x20,0x14,0x14,0x14,0x20,0x20,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
//normal quality
0x09,0x0a,0x0a,0x0a,0x0a,0x0a,0x0c,0x0c,
0x0c,0x0c,0x10,0x0c,0x0a,0x0a,0x10,0x16,
0x10,0x0c,0x0c,0x10,0x16,0x18,0x10,0x10,
0x16,0x10,0x10,0x18,0x18,0x18,0x1c,0x1c,
0x1c,0x1c,0x18,0x20,0x20,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,

0x0a,0x10,0x10,0x18,0x16,0x18,0x20,0x20,
0x20,0x20,0x28,0x20,0x20,0x20,0x28,0x28,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
//low quality
0x0b,0x0c,0x0c,0x0c,0x0c,0x0c,0x10,0x10,
0x10,0x10,0x14,0x10,0x0c,0x0c,0x14,0x1a,
0x14,0x10,0x10,0x14,0x1a,0x1c,0x14,0x14,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x24,0x24,0x24,0x24,0x24,0x24,0x24,0x24,
0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,
0x28,0x28,0x28,0x28,0x28,0x28,0x28,0x28,

0x0c,0x12,0x12,0x1a,0x1a,0x1c,0x24,0x24,
0x24,0x24,0x2c,0x24,0x24,0x24,0x2c,0x2c,
0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,
0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,0x2c,
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
0x30,0x30,0x30,0x30,0x30,0x30,0x30,0x30,
};

extern MMP_USHORT  gsSensorMCModeWidth, gsSensorMCModeHeight;
extern MMP_USHORT  gsSensorLCModeWidth, gsSensorLCModeHeight;
extern MMP_ULONG glPCCAM_VIDEO_BUF_ADDR;
extern MMP_ULONG glPCCAM_VIDEO_BUF_MAX_SIZE;


extern RES_TYPE_CFG gsCurResList[] ;

extern GRA_STREAMING_CFG gGRAStreamingCfg;

void PCCamSetQctl(MMP_ULONG qctl);


STREAM_CFG *gsCurStreamCfg[2] = {0,0} ;

void usb_set_cur_image_pipe(MMP_UBYTE ep_id,STREAM_CFG *cur_pipe)
{
    gsCurStreamCfg[ep_id] = cur_pipe ;
}

//STREAM_CFG *usb_get_cur_image_pipe(MMP_UBYTE ep_id)

STREAM_CFG *usb_get_cur_image_pipe_by_epid(MMP_UBYTE ep_id)
{
extern MMP_BOOL gbSimulcastH264 ;
extern STREAM_CFG gsH264PureStream_Cfg;
extern STREAM_CFG gsH264SimulcastStream_Cfg ;
    STREAM_CFG *cur_pipe ;
    if(UVC_VCD()==bcdVCD15) {
        if( (ep_id==1)&&(!gsCurStreamCfg[ep_id]) ) {
            cur_pipe = &gsH264PureStream_Cfg;
            cur_pipe->pipe_en |= VIDEPEN(1) ;//| (cur_pipe->pipe_en & PIPE_EN_MASK ) ;
            #if H264_SIMULCAST_EN
            if(gbSimulcastH264) {
                cur_pipe->pipe_en |= PIPE_EN_SIMULCAST ;
            }
            else {
                cur_pipe->pipe_en &= ~PIPE_EN_SIMULCAST ;
            }
            #endif
            usb_set_cur_image_pipe(ep_id,cur_pipe);
        }
    }
    return (STREAM_CFG *)gsCurStreamCfg[ep_id] ;
}


STREAM_CFG *usb_get_cur_fdtc_pipe(void)
{
extern STREAM_CFG *gsFdtcStreamCfg;
#if OMRON_FDTC_SUPPORT
    if(usb_uvc_is_fdtc_on()) {
        return gsFdtcStreamCfg ;
    }        
#endif
    return (STREAM_CFG *)0;    
}
extern OS_TCB *OSTCBCur ;
STREAM_CFG *usb_get_cur_image_pipe(PIPE_PATH pipe_id)
{
    MMP_UBYTE ep ;
    STREAM_CFG *cur_stream = 0;
    //
    //
    //
    if(pipe_id >= PIPE_MAX) {
        dbg_printf(3,"[%s]Bad pipe id:%d\r\n",OSTCBCur->OSTCBTaskName,pipe_id);
        pipe_id = 0 ;
    }
    
    #if OMRON_FDTC_SUPPORT
    if(pipe_id==PIPE_2) {
        if(usb_uvc_is_fdtc_on()) {
            return usb_get_cur_fdtc_pipe();
        }    
    }
    #endif
    
    
    for(ep=0;ep < VIDEO_EP_END();ep++) {
        cur_stream = usb_get_cur_image_pipe_by_epid(ep);//gsCurStreamCfg[ep] ;
        if(cur_stream) {
            if( PIPE_EN(cur_stream->pipe_en) & (1 << pipe_id) ) {
                return cur_stream;
            } else cur_stream = 0;
        }
    }
    if(!cur_stream) {
       // dbg_printf(3,"[%s]pipe_id : %d is not allocated \r\n",OSTCBCur->OSTCBTaskName,pipe_id);
    }
    return (STREAM_CFG *)cur_stream; 
}

MMP_UBYTE usb_get_cur_image_ep(PIPE_PATH pipe_id)
{
    MMP_UBYTE ep = 0;
    STREAM_CFG *cur_stream = 0;;
    for(ep=0;ep < VIDEO_EP_END();ep++) {
        cur_stream = usb_get_cur_image_pipe_by_epid(ep);//gsCurStreamCfg[ep]  ;
        if(cur_stream) {
            if( PIPE_EN(cur_stream->pipe_en) & (1 << pipe_id) ) {
                return ep ;
            }
        }
    }
    return ep ; 
}

MMP_UBYTE usb_get_cur_image_pipe_id(STREAM_CFG *cur_stream,MMP_UBYTE format)
{
    MMP_UBYTE pipe,pipe_format,ep ;
    if(!cur_stream) {
        dbg_printf(3,"!cur_stream ,format : %d\r\n",format);
        return 0xFF ;
    }
    
    for(pipe=0;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
        if( PIPE_EN(cur_stream->pipe_en) & (1 << pipe) ) {
            pipe_format = CUR_PIPE_CFG(pipe,cur_stream->pipe_cfg) ;
            //dbg_printf(3,"pipe %d en,format:%x\r\n",pipe,pipe_format);
            if(pipe_format==format) {
               // dbg_printf(3,"usb_get_cur_image_pipe_id : %d\r\n",pipe);
                return pipe ;
            }
        }
    }
    dbg_printf(3,"[%s]bad pipe id : %x,format:%x\r\n",OSTCBCur->OSTCBTaskName,cur_stream,format);
    return 0xFF ;
}

/*
ep_id = 0 
ep_id = 1
ep_id = 0xFF ->auto detect
*/
STREAM_CFG *usb_get_cur_image_pipe_by_format(MMP_UBYTE ep_id,MMP_UBYTE format,PIPE_PATH *pipe_id)
{
    MMP_UBYTE ep = 0,pipe,pipe_format;
    STREAM_CFG *cur_stream = 0;;
    if(ep_id==0xFF) {
        for(ep=0;ep < VIDEO_EP_END();ep++) {
            cur_stream = usb_get_cur_image_pipe_by_epid(ep);//gsCurStreamCfg[ep] ;
            for(pipe=0;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
                if( PIPE_EN(cur_stream->pipe_en) & (1 << pipe) ) {
                    pipe_format = CUR_PIPE_CFG(pipe,cur_stream->pipe_cfg) ;
                    if(pipe_format==format) {
                        //dbg_printf(3,"usb_get_cur_image_pipe_by_format pipe_id:%d,ep:%d\r\n",pipe,ep);
                        *pipe_id = pipe ;
                        return cur_stream ;
                    }
                }
            }
        }
    } else {
        cur_stream = usb_get_cur_image_pipe_by_epid(ep_id);//gsCurStreamCfg[ep_id] ;
        for(pipe=0;pipe <  MMPF_IBC_PIPE_MAX ;pipe++) {
            if( PIPE_EN(cur_stream->pipe_en) & (1 << pipe) ) {
                pipe_format = CUR_PIPE_CFG(pipe,cur_stream->pipe_cfg) ;
                if(pipe_format==format) {
                   // dbg_printf(3,"sb_get_cur_image_pipe_by_format pipe_id1 :%d,ep:%d\r\n",pipe,ep_id);
                    *pipe_id = pipe ;
                    return cur_stream ;
                }
            }
        }
    
    }
    //dbg_printf(3,"can't find pipe for this format : %d\r\n",format);
    return 0; 

}

void usb_get_cur_image_size(MMP_UBYTE pipe,MMP_ULONG *pw, MMP_ULONG *ph)
{
	STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(pipe);
	if(cur_pipe) {
    	*pw = cur_pipe->pipe_w[pipe];
    	*ph = cur_pipe->pipe_h[pipe];
    	
    	//dbg_printf(3,"<pipe : %d,w : %d, h : %d>\r\n",pipe,*pw,*ph);
	}
}

MMP_UBYTE usb_get_cur_image_format_by_pipe(MMP_UBYTE ep_id,PIPE_PATH pipe_id)
{
    STREAM_CFG * cur_stream = usb_get_cur_image_pipe_by_epid(ep_id);
    return CUR_PIPE_CFG(pipe_id,cur_stream->pipe_cfg);
}

void MMPF_Video_EnableJpegISR(void)
{
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    RTNA_AIC_Open(pAIC, AIC_SRC_JPG, jpg_isr_a,
                    AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_JPG);
    
    pJPG->JPG_INT_CPU_SR = JPG_INT_ENC_DONE ;
    pJPG->JPG_INT_CPU_EN = JPG_INT_ENC_DONE ;

}

MMP_BOOL gbJpegCaptureMode = CONTI_PREVIEW_MODE ;
void MMPF_Video_EnableJpeg(void)
{
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    if(pJPG->JPG_CTL & JPG_ENC_EN) {
        //dbg_printf(0,"--jpeg-en-oops\r\n");
    }
    else {
        pJPG->JPG_CTL |= JPG_ENC_EN;
    }
}

void MMPF_Video_SetJpegCaptureMode(JPEG_CAPTURE_MODE mode)
{
    gbJpegCaptureMode = mode ;
    //MMPF_Video_EnableJpeg();
}

JPEG_CAPTURE_MODE MMPF_Video_GetJpegCaptureMode(void)
{
    return gbJpegCaptureMode ;
}

MMP_BOOL MMPF_Video_WaitJpegEncDone(MMP_ULONG timeout_ms)
{
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    MMP_ULONG ms = 0;
    MMP_BOOL  ok = MMP_TRUE ;
    do {
        ms++ ;
        if(ms >= timeout_ms) {
            ok = MMP_FALSE ;
            break;
        }
        MMPF_OS_Sleep(1);
    } while ( pJPG->JPG_CTL & JPG_ENC_EN );
    return ok;
}


void MMPF_Video_SetJpegResolution(MMP_USHORT w,MMP_USHORT h)
{
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    
    //dbg_printf(3,"JPG_BUF_TMP_ST1:%x\r\n",pJPG->JPG_BUF_TMP_ST);
    //dbg_printf(3,"1(W,H):(%d,%d)",pJPG->JPG_ENC_W,pJPG->JPG_ENC_H);
    
    if(pJPG->JPG_CTL & JPG_ENC_EN) {
        dbg_printf(0,"@@JPEG not end..\r\n");
        return ;
    }
    
    pGBL->GBL_RST_CTL01 |=  GBL_JPG_RST ;
    RTNA_WAIT_CYCLE(100);
    pGBL->GBL_RST_CTL01 &= ~GBL_JPG_RST ;
    
    
    pJPG->JPG_ENC_W = (w + 15) & (-16) ;
    pJPG->JPG_ENC_H = (h + 7)  & (-8 ) ;// align to multiple of 8
    //dbg_printf(3,"JPG_BUF_TMP_ST2:%x\r\n",pJPG->JPG_BUF_TMP_ST);
    //dbg_printf(3,"2(W,H):(%d,%d)",pJPG->JPG_ENC_W,pJPG->JPG_ENC_H);
    
}


void MMPF_Video_SetJpegCompressBuf(MMP_ULONG buf_addr, MMP_ULONG buf_size)
{
    glPCCamCompressBufAddr = buf_addr;
    glPCCamCompressBufSize = buf_size;
    dbg_printf(3,"JPG.CAP buf : %x, size : %d\r\n",buf_addr,buf_size);
}

MMP_ULONG MMPF_Video_GetJpegCompressBuf(MMP_ULONG *buf_size)
{
    *buf_size = glPCCamCompressBufSize;
    return glPCCamCompressBufAddr ;
}

MMP_BOOL MMPF_Video_SetJpegEngine(PIPE_PATH jpeg_pipe,MMP_USHORT jpeg_w,MMP_USHORT jpeg_h)
{
#if 0
    MMPS_SYSTEM_SRAM_MAP *psrammap = MMPS_System_GetSramMap();
    AITPS_JPG   pJPG = AITC_BASE_JPG;

    MMP_ULONG   end_of_jpeg_linebuffer = 0;
    MMP_ULONG   jpeg_line_buf_limit_addr,sram_end;
#if SUPPORT_GRA_ZOOM
    pJPG->JPG_BUF_TMP_2_EN = JPG_SINGLE_LINE_BUF;
    //dbg_printf(3,"#JPG singleline buf on\r\n");
#else
    pJPG->JPG_BUF_TMP_2_EN &= ~JPG_SPLIT_TMP_BUF;
#endif

    sram_end = psrammap->ulSramEndAddr ;
    
    if(jpeg_w > 1920) {
        jpeg_line_buf_limit_addr = psrammap->ulJPGLineBufAddrOver1080p +  psrammap->ulJPGLineBufSizeOver1080p;  
        pJPG->JPG_BUF_TMP_ST = psrammap->ulJPGLineBufAddrOver1080p ; 
    }
    else {
        jpeg_line_buf_limit_addr = psrammap->ulJPGLineBufAddr +  psrammap->ulJPGLineBufSize;
        pJPG->JPG_BUF_TMP_ST = psrammap->ulJPGLineBufAddr;
    }
    
    pJPG->JPG_BUF_CMP_ST = (MMP_ULONG)MMPF_Video_CurWrPtr(jpeg_pipe) + FRAME_PAYLOAD_HEADER_SZ;
    //pJPG->JPG_BUF_CMP_ED =  pJPG->JPG_BUF_CMP_ST + (glPCCamCompressBufSize) - 1;
    pJPG->JPG_BUF_CMP_ED = pJPG->JPG_BUF_CMP_ST + MMPF_Video_GetStreamCtlInfo(jpeg_pipe)->slot_size - FRAME_PAYLOAD_HEADER_SZ;
    
    pJPG->JPG_ENC_W = (jpeg_w+ 15) & (-16) ;
    pJPG->JPG_ENC_H = (jpeg_h+ 7) & (-8) ;// align to multiple of 8
    if( pJPG->JPG_BUF_TMP_2_EN &JPG_SINGLE_LINE_BUF ) {
        end_of_jpeg_linebuffer = pJPG->JPG_BUF_TMP_ST + pJPG->JPG_ENC_W * 16 ;
    } else {
        end_of_jpeg_linebuffer = pJPG->JPG_BUF_TMP_ST + pJPG->JPG_ENC_W * 32 ;
    }
            
    if(end_of_jpeg_linebuffer > jpeg_line_buf_limit_addr ) {
        RTNA_DBG_Str3("[W]JPEG Line buffer is not enough in SRAM\r\n");
        return MMP_FALSE ;
        
    }
#endif    
    return MMP_TRUE ;
    
}

void MMPF_JPG_ISR(void)
{
extern H264_FORMAT_TYPE	gbCurH264Type;
extern volatile MMP_ULONG uSOFNUM ;
extern MMP_ULONG        PTS /*STC*/ ;
extern MMP_UBYTE	    m_bStartPreviewFrameEndSig;
extern MMPF_OS_SEMID 	m_StartPreviewFrameEndSem;
extern MMP_BOOL m_bReceiveStopPreviewSig[] ;
extern MMP_USHORT gsPreviewPath ;
extern	MMP_BOOL		m_bWaitVIFEndSig;
extern MMPF_OS_SEMID	m_PreviewControlSem;
#if 1//(ENABLE_JPEG_ISR)

#if RC_JPEG_TARGET_SIZE > 0
    MMP_ULONG qp, target_size, qp_delta, rc_vop_type=0;
    MMP_BOOL  bSkipFrame ;
    
#endif
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    MMP_BOOL    bVIFEnable ;
    MMP_ULONG   jpegsize = 0 ;
    MMP_USHORT  intsrc;
    STREAM_CFG *cur_pipe ; 
    MMP_UBYTE release = 1 ;
    MMP_BOOL    _2nd_h264 = MMP_FALSE ;
    MMP_ULONG  jpeg_buf_size = 0 ;
    
    
    intsrc = pJPG->JPG_INT_CPU_SR & pJPG->JPG_INT_CPU_EN;
    pJPG->JPG_INT_CPU_SR = intsrc;
    
    if (intsrc & JPG_INT_ENC_DONE) {
       // STREAM_CTL *cur_bctl ;
        
        //MMP_ULONG fb_addr ;
        PIPE_PATH jpeg_pipe = 0 ;
        MMP_UBYTE pipe1_cfg,*buf_addr ;
        //dbg_printf(3,"vs.j:%d\r\n",uSOFNUM );
        cur_pipe = usb_get_cur_image_pipe_by_format(0xFF,PIPE_CFG_MJPEG,&jpeg_pipe);
        pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg);
        
        if(m_bStartPreviewFrameEndSig==1) {
            m_bStartPreviewFrameEndSig = 0;
            MMPF_OS_ReleaseSem(m_StartPreviewFrameEndSem);
            dbg_printf(0,"skip jpeg\r\n");
            goto enc_en ;
        }
        
        #if 0 // TBD
        // JPEG pipe alyways in pipe 0 in simulcast flow
        if(UVCX_IsSimulcastH264()) {
            jpeg_pipe = 0 ;
        }
        #endif
        

        jpegsize = pJPG->JPG_BUF_CMP_SIZE;
      
        #if 0
        dbg_printf(3,"jpg.size:%d,jpg pipe:%d ,ms : %d\r\n",jpegsize,jpeg_pipe,usb_vc_cur_sof());
        #endif
        #if RC_JPEG_TARGET_SIZE > 0
        if( gbEnableJpegRC) {
            qp = MMPF_VidRateCtl_Get_VOP_QP(jpeg_rc_hdl, rc_vop_type, &target_size, &qp_delta, &bSkipFrame,RC_JPEG_TARGET_SIZE);

            if (1/*bSkipFrame*/) {
                MMP_ULONG padding;
                MMPF_VidRateCtl_UpdateModel(jpeg_rc_hdl, rc_vop_type,jpegsize,0, qp, MMP_FALSE, &bSkipFrame, &padding);
                //RTNA_DBG_Str(0, "jpg.rcpreskip\r\n");
                //goto enc_en ;
            }
            
           // dbg_printf(3,"#qp : %d,skip: %d,target:%d,jpeg: %d\r\n",qp,bSkipFrame,target_size,jpegsize);
            
            PCCamSetQctl(qp);
        
        }
        #endif
        if( MMPF_Video_GetJpegCaptureMode()==STILL_CAPTURE_MODE) {
            buf_addr = (MMP_UBYTE *)MMPF_Video_GetJpegCompressBuf(&jpeg_buf_size) ;
            jpeg_buf_size = jpeg_buf_size - FRAME_PAYLOAD_HEADER_SZ ;  
        }
        else {
            jpeg_buf_size =  MMPF_Video_GetStreamCtlInfo(jpeg_pipe)->slot_size - FRAME_PAYLOAD_HEADER_SZ ;
        }
        
        
        // If jpeg over the frame size, skip this frame
        if(jpegsize > jpeg_buf_size ) {
            dbg_printf(3,"JPEG Buf Crash:%d\r\n",jpegsize);
            goto enc_en ;
        }
        
        glPCCamCnt1++;
        if( MMPF_Video_GetJpegCaptureMode()!=STILL_CAPTURE_MODE) {
            buf_addr = MMPF_Video_CurWrPtr(jpeg_pipe);
        }
        //dbg_printf(3,"jpeg addr:%x,pipe:%d\r\n",buf_addr,jpeg_pipe);
        if( (buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ-2]!=0xFF) && (buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ-1]!=0xD9)) {
            dbg_printf(0,"bad jpeg end:%d,addr:%x\r\n",jpegsize,buf_addr);
            if( MMPF_Video_GetJpegCaptureMode()==STILL_CAPTURE_MODE) {
                MMP_USHORT w,h;
                w = pJPG->JPG_ENC_W;
                h = pJPG->JPG_ENC_H ;
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,0,glPCCamCnt1,(VIDEO_STILL_IMAGE_SID|VIDEO_MJPEG_SID),/*STC*/PTS,w,h,ST_JPEG_CAP,ISP_IF_AE_GetRealFPS());
            }
            goto enc_en ;
        }  else {
        #if INSERT_EOISOI==1 // Test code to seperate each frame for demux 
            if(1/* UVCX_IsSimulcastH264() */) {
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+0] = 'E' ;
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+1] = 'O' ;    
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+2] = 'I' ;    
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+3] = 'S' ;
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+4] = 'O' ;    
                buf_addr[jpegsize+FRAME_PAYLOAD_HEADER_SZ+5] = 'I' ;    
                jpegsize += 6 ;
            }
        #endif        
        }   
        if((gbCurH264Type==FRAMEBASE_H264_MJPEG)) {
            
            MMP_USHORT w,h;
            #if 0
            
            if(gGRAStreamingCfg.local_capture_en) {
                dbg_printf(3,"##Send still image payload...\r\n");
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,(VIDEO_STILL_IMAGE_SID|VIDEO_MJPEG_SID),/*STC*/PTS,w,h,ST_JPEG_CAP,ISP_IF_AE_GetRealFPS());
            }
            else {
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,VIDEO_MJPEG_SID,/*STC*/PTS,w,h,ST_MJPEG,ISP_IF_AE_GetRealFPS());
            }
            #elif 0
            
            if( MMPF_Video_GetJpegCaptureMode()==STILL_CAPTURE_MODE) {
                dbg_printf(3,"##Send still image payload...\r\n");
                w = pJPG->JPG_ENC_W;
                h = pJPG->JPG_ENC_H ;
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,(VIDEO_STILL_IMAGE_SID|VIDEO_MJPEG_SID),/*STC*/PTS,w,h,ST_JPEG_CAP,ISP_IF_AE_GetRealFPS());
           
            }
            else {
                MMPF_Display_GetLocalStreamResolution(&w,&h);
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,VIDEO_MJPEG_SID,/*STC*/PTS,w,h,ST_MJPEG,ISP_IF_AE_GetRealFPS());
                MMPF_Video_UpdateWrPtr(jpeg_pipe);

            }
            
            #endif
            
        }
        else {   
            if(cur_pipe->pipe_ph_type[jpeg_pipe]==PIPE_PH_TYPE_3) {
                usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,0,/*STC*/PTS,cur_pipe->pipe_w[jpeg_pipe],cur_pipe->pipe_h[jpeg_pipe],ST_MJPEG,ISP_IF_AE_GetRealFPS());
            } else {          
                usb_uvc_fill_payload_header((MMP_UBYTE *)buf_addr,jpegsize,glPCCamCnt1,0,/*STC*/PTS,cur_pipe->pipe_w[jpeg_pipe],cur_pipe->pipe_h[jpeg_pipe],ST_MJPEG,ISP_IF_AE_GetRealFPS() );
            }
            #if USB_FRAMEBASE_H264_DUAL_STREAM==0
            if ( UVCX_IsSimulcastH264() ) {
                _2nd_h264 = (UVCX_GetH264EncodeBufferMode()!=FRAME_MODE) && (gbCurH264EncId==1) ;
            }
            // Advance to next slot 
            if(!_2nd_h264) {
                MMPF_Video_UpdateWrPtr(jpeg_pipe);
            }
            #else
            MMPF_Video_UpdateWrPtr(jpeg_pipe);
            #endif
            
        }
        if(IS_PIPE01_EN(cur_pipe->pipe_en) ) {
            if( (pipe1_cfg==PIPE_CFG_H264)) {
                release = 0 ;
            }
        }
        
        if(release || UVCX_IsSimulcastH264()) {
            if(usb_vs_sendbytimer()==0){
               // dbg_printf(3,"vs.j:%d\r\n",uSOFNUM );
                MMPF_USB_ReleaseDm(jpeg_pipe);
            }    
        }
enc_en:        
        #if 1
        // Get cur slot addr 
        buf_addr = MMPF_Video_CurWrPtr(jpeg_pipe);
        pJPG->JPG_BUF_CMP_ST = (MMP_ULONG)(buf_addr + FRAME_PAYLOAD_HEADER_SZ);
        pJPG->JPG_BUF_CMP_ED = pJPG->JPG_BUF_CMP_ST + MMPF_Video_GetStreamCtlInfo(jpeg_pipe)->slot_size - FRAME_PAYLOAD_HEADER_SZ;// (glPCCamCompressBufSize)-1;
        //dbg_printf(3,"buf_addr:%x\r\n",buf_addr);
        #endif
        // JPEG enable is triggered at other place
        if(1/*UVCX_IsSimulcastH264()==0*/) { //TBD
            MMPF_SCALER_SWITCH_PATH *scalinfo = MMPF_Scaler_GetPathInfo(jpeg_pipe) ;
            MMP_UBYTE ep = usb_get_cur_image_ep(jpeg_pipe);
            STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep);
            if(gbCurH264Type==FRAMEBASE_H264_MJPEG) {
                if( MMPF_Video_GetJpegCaptureMode()==CONTI_PREVIEW_MODE) {
                    if(UVCX_IsSimulcastH264()==0) { 
                        MMPF_Video_EnableJpeg();
                    }
                    else {
                    
                        if(jpeg_pipe==PIPE_0) {
                        
                        }
                        else {
                            MMPF_Video_EnableJpeg();
                        }
                    }
                }
                else {
                    MMPF_Video_SetJpegCaptureMode(CONTI_PREVIEW_MODE);
                }
                
            } 
            else {
            #if SUPPORT_GRA_ZOOM
                if(scalinfo->flowctl==SCAL_WAIT_ENC_END) {
                    scalinfo->flowctl=SCAL_ENC_END ; 
                    dbg_printf(3,"> jpg end\r\n");   
                } else {    
                    if( UVCX_IsSimulcastH264()==0 ) { // TBD....
                        MMPF_Video_EnableJpeg();
                    }
                    else {
                    }
                    //dbg_printf(3,"J:%d\r\n",usb_vc_cur_usof());
                }
            #else
                MMPF_Video_EnableJpeg();
            #endif 
            }   
        }
    }
#endif
}

void PCCamSetQctl(MMP_ULONG qctl)
{
    AITPS_JPG   pJPG = AITC_BASE_JPG;

    pJPG->JPG_QLTY_CTL_FACTOR_1 = qctl;
    pJPG->JPG_QLTY_CTL_FACTOR_2 = qctl;
}

MMP_ULONG GetYUY2FrameSize(MMP_UBYTE resolution)
{
     RES_TYPE_CFG *cur_res = GetResCfg(resolution);
    if(cur_res) {
    /*
        RTNA_DBG_Str3("YUY2.W:");
        RTNA_DBG_Short3(cur_res->res_w);
        RTNA_DBG_Str3("\r\n");
        
        RTNA_DBG_Str3("YUY2.H:");
        RTNA_DBG_Short3(cur_res->res_h);
        RTNA_DBG_Str3("\r\n");
     */   
        return cur_res->res_w * cur_res->res_h * 2;
    }
    return 0 ;
}

// Set JPEG max size = 1/2 of YUV422 size
MMP_ULONG GetMJPEGFrameSize(MMP_UBYTE resolution)
{
    RES_TYPE_CFG *cur_res = GetResCfg(resolution) ;
   // VAR_W(0,cur_res->res_w);
   // VAR_W(0,cur_res->res_h);
   
    if(cur_res) {
        return cur_res->res_w * cur_res->res_h ; 
    }
    return 0 ;
}

MMP_ULONG GetYUV420FrameSize(MMP_UBYTE resolution)
{
    RES_TYPE_CFG *cur_res = GetResCfg(resolution) ;
    if(cur_res) {
#if 0
        RTNA_DBG_Str3("[Cur Res]:W=");
        RTNA_DBG_Short3(cur_res->res_w);
        RTNA_DBG_Str3(",H=");
        RTNA_DBG_Short3(cur_res->res_h);
        RTNA_DBG_Str3("\r\n");
        
#endif    
        return ( cur_res->res_w * cur_res->res_h * 3 ) / 2; 
    }
    return 0 ;

}

// Set H264 max size = 1/10 of YUV420 size
MMP_ULONG GetH264FrameSize(MMP_UBYTE resolution)
{
    MMP_ULONG yuv420_size = GetYUV420FrameSize(resolution);
    MMP_ULONG tmp=0;
	tmp = yuv420_size / MIN_H264E_COMP_RATIO;
	
    if(tmp <= ( 320*240*3/2 ) ) {
        tmp = 320*240*3/2 ;
        return tmp ;
    }
    return tmp ;
    
}

MMP_USHORT GetScalerRefInHeight(void)
{
    MMP_BOOL fps_60 = MMP_FALSE;
    if ( (usb_vc_cur_fpsx10(0) > 300 ) || (usb_vc_cur_fpsx10(1) > 300) ) {
        fps_60 = MMP_TRUE ;    
    }
    return (fps_60)?SENSOR_16_9_H_60FPS : SENSOR_16_9_H;
}

MMP_USHORT GetScalerInHeight(MMP_USHORT w,MMP_USHORT h)
{
    PCAM_USB_VIDEO_RES res;
    RES_TYPE_CFG *cur_res ;
    MMP_BOOL fps_60 = MMP_FALSE;
    if ( (usb_vc_cur_fpsx10(0) > 300 ) || (usb_vc_cur_fpsx10(1) > 300) ) {
        fps_60 = MMP_TRUE ;    
    }
#if 0   
    res = GetResIndexBySize( w, h);
    cur_res = GetResCfg(res) ;
    if(cur_res) {
        return cur_res->scaler_input_h ;
    }
#endif    
    return fps_60?SENSOR_16_9_H_60FPS:SENSOR_16_9_H;
}

MMP_USHORT GetScalerUserRatio(MMP_USHORT w,MMP_USHORT h)
{
    PCAM_USB_VIDEO_RES res;
    RES_TYPE_CFG *cur_res ;
    MMP_BOOL fps_60 = MMP_FALSE;
    if ( (usb_vc_cur_fpsx10(0) > 300 ) || (usb_vc_cur_fpsx10(1) > 300) ) {
        fps_60 = MMP_TRUE ;    
    }
    
    res = GetResIndexBySize( w, h);
    cur_res = GetResCfg(res) ;
    if(cur_res) {
        return cur_res->best_ratio ;
    }
    return (fps_60)?SENSOR_16_9_H_60FPS : SENSOR_16_9_H;
}

void SetScalerRefInOutHeight(PIPE_PATH pathsel,MMP_USHORT real_w,MMP_USHORT real_h)
{
    MMP_BOOL fps_60 = MMP_FALSE;
    MMP_USHORT scaler_ref_h =GetScalerRefInHeight() ;
    MMP_USHORT scaler_in_h = GetScalerInHeight(real_w,real_h);
    MMP_USHORT scaler_in_w = SENSOR_16_9_W ;
    if ( (usb_vc_cur_fpsx10(0) > 300 ) || (usb_vc_cur_fpsx10(1) > 300) ) {
        fps_60 = MMP_TRUE ;    
    }
    MMPF_Scaler_SetRefInHeight(scaler_ref_h);  
    MMPF_Scaler_SetRefOutHeight(pathsel,real_h);  
    if(fps_60) {
        scaler_in_w = SENSOR_16_9_W_60FPS ;
    }
    MMPF_Scaler_SetZoomSensorInput(pathsel,scaler_in_w,scaler_in_h);
}

RES_TYPE_CFG *GetResCfg(MMP_UBYTE resolution)
{
    MMP_UBYTE i =0 ;
    RES_TYPE_LIST res_type ;
    do {
        res_type = gsCurResList[i].res_type ;
        if(res_type==(RES_TYPE_LIST)resolution) {
            return &gsCurResList[i] ;
        }
        i++ ;
    } while (res_type !=PCCAM_RES_NUM ) ;
    return 0 ;
}


PCAM_USB_VIDEO_RES GetResIndexBySize(MMP_USHORT w,MMP_USHORT h)
{
    RES_TYPE_LIST i ;
    RES_TYPE_CFG  *res ;
    for(i =0 ; i < PCCAM_RES_NUM ;i++) {
        res = GetResCfg(i);
        if( (res->res_w==w) && (res->res_h==h) ) {
            return res->res_type ;
        }
    }    
    return PCCAM_RES_NUM ;
}

PCAM_USB_VIDEO_RES GetResIndexByEp(MMP_UBYTE ep_id)
{
    STREAM_SESSION *ss;
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    return ss->curvsi.vsi_frmindx  ;
 }

PCAM_USB_VIDEO_RES GetFirstStreamingRes(void)
{
    STREAM_SESSION *ss;
    MMP_UBYTE i;
    for(i=0;i<VIDEO_EP_END();i++) {
        ss = MMPF_Video_GetStreamSessionByEp(i);
        if(ss->tx_flag & SS_TX_STREAMING) {
            return ss->curvsi.vsi_frmindx ;
        }
    }
    return PCCAM_RES_NUM ;
}


PCAM_USB_VIDEO_FORMAT GetFmtIndexByEp(MMP_UBYTE ep_id)
{
    STREAM_SESSION *ss;
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    if(UVC_VCD()==bcdVCD15) {
        return ss->curvsi.vsi_fmtindx  ;
    } else {
        return pcam_get_info()->pCamVideoFormat;
    }
}


void SetFmtIndexByEp(MMP_UBYTE ep_id, PCAM_USB_VIDEO_FORMAT fmt)
{
    STREAM_SESSION *ss;
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    ss->curvsi.vsi_fmtindx = fmt  ;
    
}

MMP_USHORT GetZoomResolution(PIPE_PATH pipe,MMP_USHORT w,MMP_USHORT h,MMP_USHORT user_ratio)
{
    RES_TYPE_LIST i ;
    RES_TYPE_CFG  *res ;
    MMP_USHORT  item_at = 0;
    
    #if SCALER_ZOOM_LEVEL==BEST_LEVEL
    RATIO_TBL  *cur_ratio_tbl;
    if((w==0)&&(h==0)) {
        m_usZoomResolution[pipe] = RATIO_M ;
        return m_usZoomResolution[pipe] ;     
    }
    #if SENSOR_IN_H > SENSOR_16_9_H
    cur_ratio_tbl = MMPF_Scaler_GetBestM(pipe,&item_at,gsSensorMCModeWidth,MMPF_Scaler_GetCurInfo(pipe)->usSensorHeight,w,h,user_ratio);
    #else
    cur_ratio_tbl = MMPF_Scaler_GetBestM(pipe,&item_at,gsSensorMCModeWidth,gsSensorMCModeHeight,w,h,user_ratio);
    #endif 
    m_usZoomResolution[pipe] = cur_ratio_tbl->m; 
    //dbg_printf(3,"Ratio Tbl (n,m,item) : (%d,%d,%d)\r\n",cur_ratio_tbl->n,cur_ratio_tbl->m,item_at);  
    #else
    m_usZoomResolution[pipe] = RATIO_M ;
    #endif
    
   // dbg_printf(3,"[%d,%d]Scaler M : %d,at item#:%d\r\n",w,h,m_usZoomResolution,item_at);
    return m_usZoomResolution[pipe];
}


void InitScale(STREAM_CFG *stream_cfg,MMP_ULONG res,MMP_BOOL skipsetscaler)
{
static MMP_BOOL gbIsNv12ScalerSet = MMP_FALSE ;

    MMPF_SCALER_FIT_RANGE fitrange;
    MMPF_SCALER_GRABCONTROL grabctl,grabctlin;
    MMP_USHORT  oversample = 0;
   // MMP_USHORT  gsJpegWidth, gsJpegHeight;
    MMP_UBYTE   ctl_reg;
    MMP_BOOL    userDef = MMP_FALSE ;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
    AITPS_IBC   pIBC = AITC_BASE_IBC;
    AITPS_ICOB  pICOB = AITC_BASE_ICOB;
    
    MMPF_SCALER_PANTILT_INFO *panTilt ;
    MMP_UBYTE cur_pipe , i ;
    //MMPF_SCALER_PATH scaler_path ;
   // RES_TYPE_CFG *cur_res ;
    MMP_UBYTE pipe0_cfg = CUR_PIPE_CFG(PIPE_0,stream_cfg->pipe_cfg);//(stream_cfg->pipe_cfg>>4) & PIPE_CFG_MASK ;
    MMP_UBYTE pipe1_cfg = CUR_PIPE_CFG(PIPE_1,stream_cfg->pipe_cfg);
    MMP_UBYTE pipe2_cfg = CUR_PIPE_CFG(PIPE_2,stream_cfg->pipe_cfg);
    int workss = MMPF_Video_GetWorkingStreamSession();
    
    //  AITPS_JPG   pJPG    = AITC_BASE_JPG;
#if SUPPORT_DIGITAL_PAN 
    // sean@2010_09_20, clean up pan tilt information.
        panTilt = MMPF_Scaler_GetCurPanTiltInfo(usb_uvc_get_preview_fctl_link(0)->scalerpath);
        panTilt->usStep = 0 ;
        panTilt->ulTargetPan = 0 ;
        panTilt->ulTargetTilt = 0 ;
        panTilt->usPanEnd = 0 ;
        panTilt->usTiltEnd = 0 ;
    
        panTilt = MMPF_Scaler_GetCurPanTiltInfo(usb_uvc_get_preview_fctl_link(1)->scalerpath);
        panTilt->usStep = 0 ;
        panTilt->ulTargetPan = 0 ;
        panTilt->ulTargetTilt = 0 ;
        panTilt->usPanEnd = 0 ;
        panTilt->usTiltEnd = 0 ;
#endif
  
    if( IS_FIRST_STREAM(workss) ) {
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ISP, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_TRUE);
        
        gbIsNv12ScalerSet = MMP_FALSE ;
    }
// Init scaler in grpahic engine in simulcast H264
    if(skipsetscaler/*UVCX_IsSimulcastH264()*/) {
        dbg_printf(3,"scaler setting are skip\r\n");
        return ;
    }	                       
	                      
    //if(workss < VIDEO_EP_END()) {
    if( IS_FIRST_STREAM(workss) ) {
        // do software reset over jpeg and image control block
        #if WATCHDOG_RESET_EN==1
        pGBL->GBL_RST_REG_EN &=  ~GBL_ICON_IBC_RST ;
        #endif
        pGBL->GBL_RST_CTL01 |= (GBL_ICON_IBC_RST | GBL_JPG_RST);
        RTNA_WAIT_CYCLE(100);
        pGBL->GBL_RST_CTL01 &= ~(GBL_ICON_IBC_RST | GBL_JPG_RST);
        #if WATCHDOG_RESET_EN==1
         pGBL->GBL_RST_REG_EN |=  GBL_ICON_IBC_RST ;
        #endif
    }
#if (BIND_SENSOR_MT9T002) || (BIND_SENSOR_OV4688) || (BIND_SENSOR_OV4689)
    if(0)
#else
    if(gsJpegWidth > gsSensorMCModeWidth || gsJpegHeight > gsSensorMCModeHeight)
#endif  
    {  
        gsSensorFunction->MMPF_Sensor_ChangeMode(1, 0);
        fitrange.usInWidth = gsSensorLCModeWidth;
        fitrange.usInHeight = gsSensorLCModeHeight;
        oversample = 1;
    } 
    else {
#if 0
        if(gbStillCaptureEvent == 0){  // if event is not still capture
            gsSensorFunction->MMPF_Sensor_ChangeMode(1, 1);
        }
#endif //ooxxooxx

        fitrange.usInWidth = gsSensorMCModeWidth;
        fitrange.usInHeight = gsSensorMCModeHeight;
    }
    //dbg_printf(3, " >> PIPE_EN = %x\r\n", PIPE_EN(stream_cfg->pipe_en));
    
#if INIT_SCALE_PATH_EN==1
    if(PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN) {
        cur_pipe = PIPE_0 ;
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(0)->scalerpath);
        RTNA_DBG_Str3("$PIPE0 EN\r\n");
    } 
    if(PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN) {
        cur_pipe = PIPE_1 ;
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(1)->scalerpath);
        RTNA_DBG_Str3("$PIPE1 EN\r\n");
    }
    if(PIPE_EN(stream_cfg->pipe_en) & PIPE2_EN) {
        cur_pipe = PIPE_2 ;
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(2)->scalerpath);
        RTNA_DBG_Str3("$PIPE2 EN\r\n");
    }
#endif


    //scaler
    fitrange.fitmode = MMPF_SCALER_FITMODE_OUT;
    fitrange.usFitResol = GetZoomResolution(0,0,0,0);
  //  fitrange.usOutWidth = gsJpegWidth;
  //  fitrange.usOutHeight = gsJpegHeight;
#if  (USB_LYNC_TEST_MODE==1)&&(SCALER_ZOOM_LEVEL!=BEST_LEVEL)
    //if(stream_cfg->pipe_en != (PIPE0_EN|PIPE1_EN) ) {
    if( IS_PIPE01_EN(stream_cfg->pipe_en) == 0) {
        MMP_USHORT w,h ;
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN) {
            w = stream_cfg->pipe_w[PIPE_0] ;
            h = stream_cfg->pipe_h[PIPE_0] ;
        } else {
            w = stream_cfg->pipe_w[PIPE_1] ;
            h = stream_cfg->pipe_h[PIPE_1] ;
        }
        
        if(res==PCCAM_352_288) {
            userDef = TRUE ;
            grabctl.usScaleN = 0;
            grabctl.usScaleM = 108;//320;
            grabctl.usScaleXN = 22;//66 ;
            grabctl.usScaleYN = 24;//72 ;
            grabctl.usStartX= ( 469/*475*/ - w) / 2 + 1 ;      
            grabctl.usStartY= ( 288 - h) / 2 + 1 ;       
            grabctl.usEndX=  grabctl.usStartX + w - 1 ;      
            grabctl.usEndY=  grabctl.usStartY + h - 1 ;     
        } else if (res==PCCAM_176_144) {
            userDef = TRUE ;
            grabctl.usScaleN = 0;
            grabctl.usScaleM = 108;//320;
            grabctl.usScaleXN = 11;//33 ;
            grabctl.usScaleYN = 12;//36 ;
            grabctl.usStartX= ( 234 - w) / 2/* + 1*/ ;      
            grabctl.usStartY= ( 144 - h) / 2 + 1 ;       
            grabctl.usEndX=  grabctl.usStartX + w - 1 ;      
            grabctl.usEndY=  grabctl.usStartY + h - 1 ;     
           
        }
    }
#endif   
    for(cur_pipe=0;cur_pipe<MMPF_IBC_PIPE_MAX;cur_pipe++) {
        if(PIPE_EN(stream_cfg->pipe_en) & (1<<cur_pipe)) {
            MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(cur_pipe,stream_cfg->pipe_cfg);
            MMPF_FCTL_LINK      *pPreviewFctl = usb_uvc_get_preview_fctl_link(cur_pipe);
            MMP_USHORT user_ratio,scaler_input_h,scaler_input_w;
#if SENSOR_IN_H > SENSOR_16_9_H
            scaler_input_h = GetScalerInHeight(stream_cfg->pipe_w[cur_pipe],stream_cfg->pipe_h[cur_pipe]);
#endif            
            user_ratio = GetScalerUserRatio(stream_cfg->pipe_w[cur_pipe],stream_cfg->pipe_h[cur_pipe]);
            if( (pipe_cfg == PIPE_CFG_NV12) && (/*!IS_FIRST_STREAM(workss)*/gbIsNv12ScalerSet) ) {
                dbg_printf(3,"SCAL2:NV12 configed\r\n");
            } else {
#if SENSOR_IN_H > SENSOR_16_9_H
                // H264 need 16 align, MJPEG need 8 align
                //if( (pipe_cfg!=PIPE_CFG_H264) && (pipe_cfg!=PIPE_CFG_MJPEG)) {
                if(pipe_cfg==PIPE_CFG_NV12 ) {
                    scaler_input_h = (usb_vc_cur_fpsx10(0) > 300)?SENSOR_16_9_H_60FPS : SENSOR_16_9_H;
                }
                scaler_input_w = (usb_vc_cur_fpsx10(0) > 300)?gsSensorMCModeWidth : gsSensorLCModeWidth;
                MMPF_Scaler_SetZoomSensorInput(cur_pipe,scaler_input_w,scaler_input_h);
                fitrange.usInHeight = scaler_input_h;
#else
                if( usb_vc_cur_fpsx10(0) > 300) {
                    scaler_input_w = gsSensorMCModeWidth ;
                    scaler_input_h = gsSensorMCModeHeight;
                }
                else {
                    scaler_input_w = gsSensorLCModeWidth ;
                    scaler_input_h = gsSensorLCModeHeight;
                }
                MMPF_Scaler_SetZoomSensorInput(cur_pipe,scaler_input_w,scaler_input_h);
#endif
            
                fitrange.usOutWidth  = stream_cfg->pipe_w[cur_pipe] ;
                if(pipe_cfg==PIPE_CFG_MJPEG) {
                    fitrange.usOutHeight = ( stream_cfg->pipe_h[cur_pipe]+ 7 ) & (-8) ;
                    fitrange.usOutWidth  = ( stream_cfg->pipe_w[cur_pipe]+ 15 ) & (-16) ;
                }
                else if (pipe_cfg==PIPE_CFG_H264) {
                    fitrange.usOutHeight =  ( stream_cfg->pipe_h[cur_pipe]+ 15 ) & (-16) ;
                }
                else {
                    fitrange.usOutHeight = stream_cfg->pipe_h[cur_pipe] ;
                }

                if(usb_vc_cur_fpsx10(0) > 300) {
                    if(fitrange.usOutHeight > scaler_input_h) {
                        fitrange.usOutHeight = scaler_input_h ;
                    }
                    if(fitrange.usOutWidth > scaler_input_w) {
                        fitrange.usOutWidth = scaler_input_w ;
                    }
                }

                // patrick debug only
                dbg_printf(0, "=== InitScale pipe#%d ===\r\n", cur_pipe);
                
                /*
				if(pipe_cfg!=PIPE_CFG_MJPEG) { 
				    fitrange.usOutHeight =  ( stream_cfg->pipe_h[cur_pipe]+ 15 ) & (-16) ;
                }
                */

                dbg_printf(0,"=== fitrange In (%d,%d) ===\r\n",fitrange.usInWidth,fitrange.usInHeight );
                dbg_printf(0,"=== fitrange Out(%d,%d) ===\r\n",fitrange.usOutWidth,fitrange.usOutHeight );

                if(userDef==MMP_FALSE) {
                    fitrange.usFitResol = GetZoomResolution(pPreviewFctl->scalerpath,fitrange.usOutWidth,fitrange.usOutHeight,user_ratio);
                }
                
                
                #if (USB_LYNC_TEST_MODE==1) && (SCALER_ZOOM_LEVEL==BEST_LEVEL)
                {
                     MMP_UBYTE pipe_en = stream_cfg->pipe_en & ~(1 << GRA_SRC_PIPE) ;
                     if( IS_PIPE0_EN(pipe_en) && (cur_pipe == PIPE_0)/*|| IS_PIPE1_EN(pipe_en)*/) {
                        if( (res==PCCAM_352_288) || (res==PCCAM_176_144) ) {
                            userDef = TRUE ;
                            MMPF_Scaler_GetBestGrabRange(pPreviewFctl->scalerpath,MMPF_Scaler_GetRatioCurItem(pPreviewFctl->scalerpath),&fitrange,&grabctl,&grabctlin,USB_LYNC_CIF_RATIO) ;  
                            dbg_printf(3,"--Lync CIF ratio:en\r\n");
                        }
                    }
                
                }
                #endif
                // scaler initialize here for all format.s
                if(1/*pipe_cfg != PIPE_CFG_H264*/) {
                    MMPF_Scaler_SetOutputFormat(pPreviewFctl->scalerpath, MMPF_SCALER_COLOR_YUV422);
                    MMPF_Scaler_SetEngine(userDef, pPreviewFctl->scalerpath, &fitrange, &grabctl);
                    MMPF_Scaler_SetLPF(pPreviewFctl->scalerpath, &fitrange, &grabctl);
                    userDef = MMP_FALSE ; 
                } else {
                    MMPF_FCTL_PREVIEWATTRIBUTE *pFctl = MMPF_Fctl_GetPreviewAttributes( usb_uvc_get_preview_fctl_link(cur_pipe)->ibcpipeID ) ;
                    grabctl = pFctl->grabctl ;
                }
                // Fixed bug for 4x zoom config
                
        #if SUPPORT_DIGITAL_ZOOM
                //MMPF_Scaler_AccessGrabArea(MMP_FALSE, pPreviewFctl->scalerpath,&grabctlin,0);
                // Sensor input width & height
                if(grabctl.usScaleN) {   
                    MMPF_Scaler_SetZoomParams(pPreviewFctl->scalerpath,&grabctl,fitrange.usInWidth,fitrange.usInHeight,
                                                (fitrange.usInWidth  + grabctl.usScaleM) / (grabctl.usScaleM * 2),
                                                (fitrange.usInHeight + grabctl.usScaleM) / (grabctl.usScaleM * 2) );
                    if(grabctl.usScaleN > grabctl.usScaleM) {                                     
                        MMPF_Scaler_SetZoomRange(pPreviewFctl->scalerpath,grabctl.usScaleN,grabctl.usScaleN);  
                    } else {
                        MMPF_Scaler_SetZoomRange(pPreviewFctl->scalerpath,grabctl.usScaleN,grabctl.usScaleM);
                    }
                } else {
                #if USB_LYNC_TEST_MODE
                    MMPF_Scaler_SetZoomParams(pPreviewFctl->scalerpath,&grabctl,fitrange.usInWidth,fitrange.usInHeight,
                                                (fitrange.usInWidth  + grabctl.usScaleXM) / (grabctl.usScaleXM * 2),
                                                (fitrange.usInHeight + grabctl.usScaleYM) / (grabctl.usScaleYM * 2) );
                    MMPF_Scaler_SetZoomRangeEx(pPreviewFctl->scalerpath,grabctl.usScaleXN,grabctl.usScaleXM,grabctl.usScaleYN,grabctl.usScaleYM);  
                #endif    
                } 
                MMPF_Scaler_SetDigitalZoom(pPreviewFctl->scalerpath,MMPF_SCALER_ZOOMSTOP,0,MMP_FALSE);                
        #endif 
        #if 0
                dbg_printf(3,"zoom pipe %d,grab(%d,%d)-( %d,%d,%d,%d )\r\n",
                            cur_pipe, 
                            grabctl.usEndX - grabctl.usStartX + 1 ,
                            grabctl.usEndY - grabctl.usStartY + 1 ,
                            grabctl.usStartX,grabctl.usStartY,grabctl.usEndX,grabctl.usEndY);
                            
        #endif                     
                // scaler initialize here for all format.s             
                if(1/*pipe_cfg != PIPE_CFG_H264*/) {
                    MMPF_Scaler_SetEnable(pPreviewFctl->scalerpath, MMP_TRUE);
                }
                
                if(pipe_cfg==PIPE_CFG_MJPEG) {
                    MMPF_Scaler_SetOutputColor(pPreviewFctl->scalerpath, MMP_FALSE, MMPF_SCALER_COLRMTX_FULLRANGE);
                } else {
                    MMPF_Scaler_SetOutputColor(pPreviewFctl->scalerpath, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
                }
                
                if(pipe_cfg==PIPE_CFG_NV12) {
                    gbIsNv12ScalerSet = MMP_TRUE ;
                    MMPF_Scaler_SetOutputColor(pPreviewFctl->scalerpath, MMP_FALSE, MMPF_SCALER_COLRMTX_FULLRANGE);
                }
            }
        }
    }
    if( IS_FIRST_STREAM(workss) ) {    
#if SUPPORT_GRA_ZOOM
        // ooxxooxx
        // config PIPE_0 for GRA
        if( MMPF_Video_IsLoop2GRA(stream_cfg) ) {
            	MMPF_GRAPHICS_BUFATTRIBUTE src;
    			MMPF_GRAPHICS_RECT rect;
            
        	#if	0// zoom case => scaler bypass
                userDef = TRUE ;
                fitrange.usFitResol = 30;
                grabctl.usScaleN = 30;
                grabctl.usScaleM = 30;
                grabctl.usStartX= ( fitrange.usInWidth - stream_cfg->pipe_w[0]) / 2 + 1 ;      
                grabctl.usStartY= ( fitrange.usInHeight - stream_cfg->pipe_h[0]) / 2 + 1 ;       
                grabctl.usEndX=  grabctl.usStartX + stream_cfg->pipe_w[0] - 1 ;      
                grabctl.usEndY=  grabctl.usStartY + stream_cfg->pipe_h[0] - 1 ;			        	
            #endif    
                		      
          		// set GRA attribute      
          		src.usWidth		 = stream_cfg->pipe_w[GRA_SRC_PIPE];
    	        src.usHeight 	 = stream_cfg->pipe_h[GRA_SRC_PIPE];	
    	        src.usLineOffset = src.usWidth;
    	        src.colordepth	 = MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;	//MMPF_GRAPHICS_COLORDEPTH_YUV422;
                src.ulBaseAddr 	 = 0;
           		src.ulBaseUAddr  = src.ulBaseVAddr = 0;
            	rect.usLeft   = rect.usTop = 0;
    	        rect.usWidth  = stream_cfg->pipe_w[GRA_SRC_PIPE];
    	        rect.usHeight = stream_cfg->pipe_h[GRA_SRC_PIPE];			        
            	MMPF_Graphics_SetScaleAttribute(&src, src.ulBaseAddr, src.colordepth,\
    												&rect, 1, MMPF_GRAPHICS_SCAL_FB);	
        }
#endif        
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_VI, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ISP, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_FALSE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_FALSE);
    }    
}

void InitIBC(MMPF_IBC_PIPEID cur_pipe,MMP_UBYTE pipe_cfg,MMP_USHORT w,MMP_USHORT h)
{
    MMP_ULONG wr_ptr ;
    MMPF_ICO_PIPEATTRIBUTE  IconAttr;
    MMPF_IBC_PIPEATTRIBUTE  IbcAttr;
    MMPF_FCTL_LINK          *pPreviewFctl;
    pPreviewFctl = usb_uvc_get_preview_fctl_link(cur_pipe);
    IconAttr.inputsel = pPreviewFctl->scalerpath;
    IconAttr.bDlineEn = MMP_TRUE;
    MMPF_ICON_SetAttributes (pPreviewFctl->icopipeID, &IconAttr);
    wr_ptr = (MMP_ULONG)MMPF_Video_CurWrPtr(cur_pipe);
    switch(pipe_cfg) {
    case PIPE_CFG_YUY2:
    case PIPE_CFG_NV12:
        IbcAttr.function = MMPF_IBC_FX_TOFB;
        IbcAttr.ulBaseAddr = wr_ptr + FRAME_PAYLOAD_HEADER_SZ;
        if (pipe_cfg ==PIPE_CFG_NV12) {
            IbcAttr.ulBaseUAddr = IbcAttr.ulBaseAddr  + w * h;
            IbcAttr.ulBaseVAddr = IbcAttr.ulBaseUAddr + w * h/4;
            IbcAttr.colorformat = MMPF_IBC_COLOR_NV12;
            dbg_printf(3,"NV12/Y PIPE %d enable\r\n",cur_pipe);
        }
        else {
            IbcAttr.ulBaseUAddr = IbcAttr.ulBaseVAddr = 0;
            IbcAttr.colorformat = MMPF_IBC_COLOR_YUV422_YUYV;
            dbg_printf(3,"YUY2 PIPE %d enable\r\n",cur_pipe);

        }
    #if USB_SPEEDUP_PREVIEW_TIME==0                
        MMPF_IBC_SetStoreEnable(pPreviewFctl->ibcpipeID, MMP_TRUE);
    #endif                
    break;
    case PIPE_CFG_MJPEG:
        IbcAttr.function = MMPF_IBC_FX_JPG;
        IbcAttr.ulBaseAddr = IbcAttr.ulBaseUAddr = IbcAttr.ulBaseVAddr = 0;
        IbcAttr.colorformat = MMPF_IBC_COLOR_YUV422;
        dbg_printf(3,"MJPEG PIPE %d enable\r\n",cur_pipe);
        break;
    default:
        DBG_S(0, "#Error : Unknow config for PIPEx\r\n");
        break;
    }
    IbcAttr.bMirrorEnable = MMP_FALSE;
    IbcAttr.usMirrorWidth = 0; // TBD : JPEG
    IbcAttr.ulLineOffset = 0;
    IbcAttr.InputSource = pPreviewFctl->icopipeID;
    MMPF_IBC_SetAttributes(pPreviewFctl->ibcpipeID, &IbcAttr);
    // Disable frame start intr.
    MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);


    if(pipe_cfg==PIPE_CFG_MJPEG) {
        MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);
        // sean@2013_03_05 add
        MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_ST, MMP_FALSE);
         
    } else {
// sean@2013_09_17 alyways enable IBC frame ready intr.
        if(0/*UVCX_IsSimulcastH264()&&(pipe_cfg==PIPE_CFG_NV12)*/) {
            if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);
            }
            else {
                MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);
            }
        }
        else {
            MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);
        }
        //if((ss->tx_flag & SS_TX_RT_MODE)&&(pipe_cfg==PIPE_CFG_YUY2)) {
        //    MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_ST, MMP_TRUE);
        //}
    }

}

//void MMPF_InitPCCam(void)
void MMPF_InitPCCam(STREAM_CFG *stream_cfg,MMP_ULONG res,MMP_BOOL skipsetscaler)
{

    MMP_ULONG   jpeg_frame_size ;
    
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    MMP_ULONG   end_of_jpeg_linebuffer = 0;
    MMP_ULONG   jpeg_line_buf_limit_addr,sram_end;//SRAM_AREA_FOR_CRITICAL_REGION ;
    int i;
    MMP_UBYTE pipe1_cfg = CUR_PIPE_CFG(PIPE_1,stream_cfg->pipe_cfg);//(stream_cfg->pipe_cfg>>4) & PIPE_CFG_MASK ;
    MMP_UBYTE cur_pipe,jpeg_pipe = PIPE_0 ;
    
    STREAM_CTL *bctl ;
    
    MMPS_SYSTEM_SRAM_MAP *psrammap = MMPS_System_GetSramMap();
    
    //jpeg_line_buf_limit_addr = (MMP_ULONG)&Image$$ALL_SRAM$$Base ;
    //sram_end = (MMP_ULONG)&Image$$SRAM_END$$Base ;


    jpeg_line_buf_limit_addr = psrammap->ulJPGLineBufAddr +  psrammap->ulJPGLineBufSize;
    sram_end = psrammap->ulSramEndAddr ;
    
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_TRUE);
    
    if(skipsetscaler==0) {
        InitScale(stream_cfg,res,0);
    } else {
        AITPS_GBL   pGBL = AITC_BASE_GBL;
        pGBL->GBL_CLK_DIS0 &= (~(GBL_CLK_SCAL_DIS | GBL_CLK_VI_DIS | GBL_CLK_JPG_DIS));
        pGBL->GBL_CLK_DIS1 &= (~(GBL_CLK_ICON_DIS | GBL_CLK_IBC_DIS | GBL_CLK_DMA_DIS | GBL_CLK_GRA_DIS));
                      
    }
    
#if SUPPORT_GRA_ZOOM
    pJPG->JPG_BUF_TMP_2_EN = JPG_SINGLE_LINE_BUF;
    dbg_printf(3,"#JPG singleline buf on\r\n");
#else
    pJPG->JPG_BUF_TMP_2_EN &= ~JPG_SPLIT_TMP_BUF;
#endif
    
    pJPG->JPG_BUF_TMP_ST = psrammap->ulJPGLineBufAddr;

    for(cur_pipe=0;cur_pipe < MMPF_IBC_PIPE_MAX ; cur_pipe++) {
        if(PIPE_EN(stream_cfg->pipe_en) & (1<<cur_pipe)) {
            if( CUR_PIPE_CFG(cur_pipe,stream_cfg->pipe_cfg) == PIPE_CFG_MJPEG ) {
                MMP_UBYTE *wr_ptr,*base_ptr ;
                
                MMP_USHORT jpeg_w =  (stream_cfg->pipe_w[cur_pipe] + 15) & (-16) ;
                if(jpeg_w > 1920) {
                    jpeg_line_buf_limit_addr = psrammap->ulJPGLineBufAddrOver1080p +  psrammap->ulJPGLineBufSizeOver1080p;  
                    pJPG->JPG_BUF_TMP_ST = psrammap->ulJPGLineBufAddrOver1080p ; 
                }
                bctl = &stream_cfg->pipe_b[cur_pipe] ;
                base_ptr = (MMP_UBYTE *)bctl->buf_addr ;
                wr_ptr = &base_ptr[bctl->wr_index] ;
                wr_ptr += bctl->off_header;
                
                pJPG->JPG_BUF_CMP_ST = (MMP_ULONG)wr_ptr + FRAME_PAYLOAD_HEADER_SZ;
                pJPG->JPG_BUF_CMP_ED = pJPG->JPG_BUF_CMP_ST + bctl->slot_size - FRAME_PAYLOAD_HEADER_SZ ;
                
                //pJPG->JPG_BUF_CMP_ST = (MMP_ULONG)MMPF_Video_CurWrPtr(cur_pipe) + FRAME_PAYLOAD_HEADER_SZ;
                //pJPG->JPG_BUF_CMP_ED =  pJPG->JPG_BUF_CMP_ST + (glPCCamCompressBufSize) - 1;
                //pJPG->JPG_BUF_CMP_ED = pJPG->JPG_BUF_CMP_ST + MMPF_Video_GetStreamCtlInfo(cur_pipe)->slot_size - FRAME_PAYLOAD_HEADER_SZ;
                
                pJPG->JPG_ENC_W = (stream_cfg->pipe_w[cur_pipe] + 15) & (-16) ;
                pJPG->JPG_ENC_H = (stream_cfg->pipe_h[cur_pipe] + 7) & (-8) ;// align to multiple of 8
                if( pJPG->JPG_BUF_TMP_2_EN &JPG_SINGLE_LINE_BUF ) {
                    end_of_jpeg_linebuffer = pJPG->JPG_BUF_TMP_ST + pJPG->JPG_ENC_W * 16 ;
                } else {
                    end_of_jpeg_linebuffer = pJPG->JPG_BUF_TMP_ST + pJPG->JPG_ENC_W * 32 ;
                }
                jpeg_pipe = cur_pipe;
            }
        }
    }

    #if 0
    dbg_printf(3,"#JPG[W,H]:(%d,%d)\r\n",pJPG->JPG_ENC_W,pJPG->JPG_ENC_H);
    dbg_printf(3,"#pJPG->JPG_BUF_CMP_ST : %x,size:%x\r\n", pJPG->JPG_BUF_CMP_ST,glPCCamCompressBufSize);
    dbg_printf(3,"#JPG.linebuf[Start->End,Limit]:(%x->%x ,%x)\r\n",pJPG->JPG_BUF_TMP_ST,end_of_jpeg_linebuffer,jpeg_line_buf_limit_addr);
    #endif
    
    
    //dbg_printf(3,"sram end : %x\r\n",sram_end);
    
    if(end_of_jpeg_linebuffer > jpeg_line_buf_limit_addr ) {
        MMP_ULONG line_buffer ,freesize;
        line_buffer = MMPF_SYS_GetCurFBAddr(&freesize,STREAM_EP_MJPEG);
        pJPG->JPG_BUF_TMP_ST = line_buffer ;
        end_of_jpeg_linebuffer = pJPG->JPG_BUF_TMP_ST + pJPG->JPG_ENC_W * 32 ;
        RTNA_DBG_Str3("[W]JPEG Line buffer is not enough in SRAM\r\n");
        
    }

    pJPG->JPG_CTL = JPG_ENC_MARKER_EN | JPG_ENC_FMT_YUV422;
    for(i = 0;i < 64;i++) {
        *(AITC_BASE_TBL_Q + i) = Q_table[2][i];
        *(AITC_BASE_TBL_Q + i + 64) = Q_table[2][i + 64];
        *(AITC_BASE_TBL_Q + i + 128) = Q_table[2][i + 64];
    }
    if(gbUsbHighSpeed==1)
    {
        glPCCamCurQCtl = 0x0010;
        if( (stream_cfg->pipe_en&PIPE_EN_MASK)==(PIPE0_EN|PIPE1_EN) ) {
            if( (pipe1_cfg==PIPE_CFG_H264)) {
              //  glPCCamCurQCtl = 0x0040 ; // For bandwidth issue
            }
        }
    }
    else
    {//Gason@1224,change.
    /*
	   if(res <= PCCAM_640_480)
          glPCCamCurQCtl = 0x0050;
       else if((res > PCCAM_640_480) && (res <= PCCAM_320_240))
          glPCCamCurQCtl = 0x0010;
       else if((res > PCCAM_320_240) && (res <= PCCAM_800_448))
          glPCCamCurQCtl = 0x0050;
       else 
          glPCCamCurQCtl = 0x0120;      
    */
        // fixed full speed some resolutions' quality are bad
        MMP_ULONG size = GetMJPEGFrameSize(res) ;
        if(size > 640*480) {
            glPCCamCurQCtl = 0x0120; 
        }
        else {
            glPCCamCurQCtl = ( size * 80 ) / (640 * 480) ;
        }
    }
    
    PCCamSetQctl(glPCCamCurQCtl);
    
#if RC_JPEG_TARGET_SIZE > 0
    if( gbEnableJpegRC ) {
        jpeg_frame_size = ( stream_cfg->pipe_b[jpeg_pipe].slot_size > RC_JPEG_TARGET_SIZE ) ? RC_JPEG_TARGET_SIZE : stream_cfg->pipe_b[jpeg_pipe].slot_size   ;
        
    	RcConfig.MaxIWeight = 1024;//5000;
    	RcConfig.MinIWeight = 1024;//1500;
    	RcConfig.MaxBWeight = 0;//1000;
    	RcConfig.MinBWeight = 0;//800;
    	
    	
        RcConfig.VBV_Delay = (jpeg_frame_size * 8 * stream_cfg->pipe_b[jpeg_pipe].slot_num ) / 2; 
    	RcConfig.TargetVBVLevel	    = 500;
    	//RcConfig.TargetVBVLevel = (RcConfig.VBV_Delay * 500) / 1000;
    	
    	RcConfig.InitWeight[0] = 1024;
    	RcConfig.InitWeight[1] = 1024;
    	RcConfig.InitWeight[2] = 0; 
    	
    	RcConfig.InitQP[0]          = glPCCamCurQCtl;
        RcConfig.InitQP[1]          = glPCCamCurQCtl;
        RcConfig.InitQP[2]          = glPCCamCurQCtl;
        RcConfig.MaxQPDelta[0]      = 4;
        RcConfig.MaxQPDelta[1]      = 4;
        RcConfig.MaxQPDelta[2]      = 4;
        RcConfig.SkipFrameThreshold = 900 ;
        RcConfig.rc_mode            = MMPF_VIDENC_RC_MODE_CBR;//VBR
        RcConfig.bPreSkipFrame      = MMP_FALSE; 
        dbg_printf(3,"#Init JPEG RC\r\n");
        MMPF_VidRateCtl_Init( &jpeg_rc_hdl,3,MMPF_MP4VENC_FORMAT_MJPEG,jpeg_frame_size,jpeg_frame_size*8*ISP_IF_AE_GetRealFPS(),0,0,TRUE,RcConfig);
    }
#endif
    MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_JPG, MMP_FALSE);
}


MMP_USHORT MMPF_GetJfifTag(MMP_USHORT *pTagID, MMP_USHORT *pTagLength, MMP_UBYTE **ppImgAddr, MMP_ULONG length)
{
    MMP_ULONG i;

    i = 0;

    while(*(*ppImgAddr + i) != 0xFF && i < length)
        i++;

    if(i >= length)
        return 1 ; // 1 : error

    *ppImgAddr = *ppImgAddr + i;

    *pTagID = (**ppImgAddr << 8) + *(*ppImgAddr + 1);
    if(*pTagID >= 0xFFD0 && *pTagID <= 0xFFD9) {
        *pTagLength = 0;
    } else {
        *pTagLength = (*(*ppImgAddr + 2) << 8) + *(*ppImgAddr + 3);
    }
    return 0;
}

void MMPF_Video_SignalFrameDone(MMP_UBYTE eid,PIPE_PATH UvcPipePath)
{
#if H264_SIMULCAST_EN
    if(UVCX_IsSimulcastH264()==0) {
        MMPF_USB_ReleaseDm(UvcPipePath);
    }
    else {
        if( (UVCX_GetH264EncodeBufferMode()==FRAME_MODE) ) {
            if( (gGRAStreamingCfg.streaming_num <= 2) || (UVC_VCD()==bcdVCD15)) {
                //dbg_printf(3,"releasedm:%d\r\n",gGRAStreamingCfg.work_streaming);
                //if(eid==1) {
                    MMPF_USB_ReleaseDm(UvcPipePath);
                //}
            }
        }
        else {
            if(UVC_VCD()==bcdVCD15) {
                    MMPF_USB_ReleaseDm(UvcPipePath);
            } else {
                if(eid==1) {
                    MMPF_USB_ReleaseDm(UvcPipePath);
                }
            }
        }
    }
#else    
    MMPF_USB_ReleaseDm(UvcPipePath);
#endif    
}


void MMPF_Video_GetBufBound(PIPE_PATH pipe,MMP_ULONG *lowbound,MMP_ULONG *highbound)
{
     STREAM_CTL *cur_bctl = MMPF_Video_GetStreamCtlInfo( pipe);
    #if (SLOT_RING == 1)
    *lowbound = cur_bctl->buf_addr;
    *highbound = cur_bctl->buf_addr + cur_bctl->slot_size*cur_bctl->slot_num;
    #else
    *lowbound = (MMP_ULONG)MMPF_Video_CurWrPtr(pipe) + FRAME_PAYLOAD_HEADER_SZ;
    *highbound = *lowbound + cur_bctl->slot_size - FRAME_PAYLOAD_HEADER_SZ;
    #endif
     
}


void MMPF_Video_GetSlotInfo(MMP_BOOL ring_buf_mode,PIPE_PATH pipe,MMP_ULONG *addr,MMP_ULONG *slot_size,MMP_USHORT *slot_num)
{
    STREAM_CTL *cur_bctl = MMPF_Video_GetStreamCtlInfo( pipe);
    *slot_num = 0 ;
    *slot_size = 0 ;
    if(cur_bctl) {
        if(ring_buf_mode) {
            *addr = cur_bctl->buf_addr;
            *slot_size = cur_bctl->slot_size ; 
            *slot_num  = cur_bctl->slot_num ;
        
        } else {
            *addr = (MMP_ULONG)MMPF_Video_CurWrPtr(pipe);
            *slot_size = cur_bctl->slot_size ;
            *slot_num = 1 ;
        }
    } else {
        dbg_printf(3,"[%d]Bad pipe :%d\r\n",ring_buf_mode,pipe);
    }    
}

STREAM_CTL *MMPF_Video_GetStreamCtlInfo(PIPE_PATH pipe)
{
    STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(pipe);
    if(pipe >= MMPF_IBC_PIPE_MAX) {
        RTNA_DBG_Str3(" Error pipe \r\n");
        while(1);
    }
    return &cur_stream->pipe_b[pipe];
}

MMP_BOOL MMPF_Video_IsEmpty(PIPE_PATH pipe)
{
    STREAM_CTL *cur_bctl ;
//    dbg_printf(3,"E[%d]\r\n",pipe);
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    return ( cur_bctl->total_rd >= cur_bctl->total_wr );
}

MMP_ULONG MMPF_Video_GetFreeBufSize( PIPE_PATH pipe)
{
    STREAM_CTL *cur_bctl;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    return cur_bctl->slot_size * MMPF_Video_GetEmptySlotNum(pipe);
}


MMP_ULONG MMPF_Video_GetEmptySlotNum(PIPE_PATH pipe)
{
    STREAM_CTL *cur_bctl;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    if (cur_bctl->total_wr < cur_bctl->total_rd) {
        return cur_bctl->slot_num;
    }
    else if (cur_bctl->total_wr >= (cur_bctl->total_rd+cur_bctl->slot_num)) {
        return 0;
    }
    else {
        return cur_bctl->slot_num - (MMP_ULONG)(cur_bctl->total_wr - cur_bctl->total_rd);
    }

}

MMP_ULONG MMPF_Video_GetEmptyFrameNum(PIPE_PATH pipe)
{
    STREAM_CTL *cur_bctl;
    MMP_USHORT slots_per_frame,frames ;
    MMP_USHORT free_slots ;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    slots_per_frame = (cur_bctl->slot_num / cur_bctl->frame_num) ;
    free_slots = MMPF_Video_GetEmptySlotNum(pipe);
    frames =  free_slots / slots_per_frame ;
    if(frames > cur_bctl->frame_num) {
        frames = cur_bctl->frame_num ;
    }
    return frames ;
}


MMP_BOOL MMPF_Video_IsFull(PIPE_PATH pipe)
{
    //STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(ep_id);
    STREAM_CTL *cur_bctl ;
    int off ;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    off = (int)(cur_bctl->total_wr - cur_bctl->total_rd ) ;
    if(off < 0) {
        //dbg_printf(0,"[%d]Over Read Wr,Rd=(%d,%d)\r\n",pipe,cur_bctl->total_wr,cur_bctl->total_rd);
        /*
        dbg_printf(0,"[%d]Over Read:\r\n",pipe);
        dbg_printf(0,"wr : %d\r\n",cur_bctl->total_wr);
        dbg_printf(0,"rd : %d\r\n",cur_bctl->total_rd);
        */
    }
    if(off >= cur_bctl->slot_num) {
        return 1 ;
    }
    return 0 ;
}

void MMPF_Video_UpdateWrPtr(PIPE_PATH pipe)
{
    STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(pipe);
    STREAM_CTL *cur_bctl ;
    MMP_UBYTE pipe0_cfg , pipe1_cfg , pipe2_cfg ;
    MMP_USHORT slot_cnt = 1;
    MMP_UBYTE ph_t = cur_stream->pipe_ph_type[pipe]; // sean@2013_01_03 bug fixed
    MMP_BOOL  slicetx = 0;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    //if(pipe == PIPE_1){	// H.264 is only in PIPI_1 
    	slicetx  = UVCX_IsSliceBasedTxStream(cur_stream);
    //}	
// add "+ cur_bctl->off_header" to fix bug if payload header size != 32 
    if( slicetx ) {
        if (ph_t == PIPE_PH_TYPE_2) {
            struct _SKYPE_H264_PH *header = (struct _SKYPE_H264_PH *)(cur_bctl->buf_addr + cur_bctl->wr_index + cur_bctl->off_header);
            slot_cnt = (header->dwPayloadSize + FRAME_PAYLOAD_HEADER_SZ + cur_bctl->off_header + cur_bctl->slot_size - 1)
                        / cur_bctl->slot_size;
            //dbg_printf(3,"payloadsize:%d,off_header : %d,slot_size:%d\r\n",header->dwPayloadSize ,cur_bctl->off_header , cur_bctl->slot_size ); 
        }
        else {
            struct _UVC_H264_PH *header = (struct _UVC_H264_PH *)(cur_bctl->buf_addr + cur_bctl->wr_index + cur_bctl->off_header);
            slot_cnt = (header->dwPayloadSize + FRAME_PAYLOAD_HEADER_SZ + cur_bctl->off_header + cur_bctl->slot_size - 1)
                        / cur_bctl->slot_size;
            //dbg_printf(3,"payloadsize:%d,off_header : %d,slot_size:%d\r\n",header->dwPayloadSize ,cur_bctl->off_header , cur_bctl->slot_size ); 
        }
    }    
    if( slicetx ) {
        cur_bctl->wr_index += (cur_bctl->slot_size * slot_cnt);
        cur_bctl->total_wr += slot_cnt;
        if( cur_bctl->wr_index >= (cur_bctl->slot_size * cur_bctl->slot_num)) {
            cur_bctl->wr_index -= (cur_bctl->slot_size * cur_bctl->slot_num);
        }
    } else {
        cur_bctl->wr_index += cur_bctl->slot_size ;
        cur_bctl->total_wr += 1 ;
        if( cur_bctl->wr_index == (cur_bctl->slot_size * cur_bctl->slot_num) ) {
            cur_bctl->wr_index = 0 ;
        }
    }   
}

void MMPF_Video_UpdateRdPtr(PIPE_PATH pipe)
{
    STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(pipe);
    STREAM_CTL *cur_bctl ;
    FRAME_PAYLOAD_HEADER *ph ;
#if 1//(SLOT_RING == 1)
    MMP_USHORT slots =1 ;
#endif
    MMP_BOOL    slicetx ;
    slicetx  = UVCX_IsSliceBasedTxStream(cur_stream);
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    /*Clean sent frame payload size to zero !*/
    ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(pipe) ;
    if( ph ) {
        if( slicetx /*UVCX_IsSliceBasedTx(pipe)*/ ) {
        	slots = ( ph->UVC_H264_PH.dwPayloadSize + FRAME_PAYLOAD_HEADER_SZ +  cur_bctl->off_header + cur_bctl->slot_size - 1)
                        / cur_bctl->slot_size;
        }                    
        ph->UVC_H264_PH.dwPayloadSize = 0;
        ph->UVC_H264_PH.dwTimeStamp = 0; 
    } else {
        dbg_printf(3,"bad ph\r\n");
    }

    //if( !UVCX_IsSliceBasedTx(pipe) ) {
    if( !slicetx) {
        cur_bctl->rd_index += cur_bctl->slot_size ;
        cur_bctl->total_rd += 1 ;
        if(cur_bctl->rd_index >= ( cur_bctl->slot_size * cur_bctl->slot_num ) ) {
            cur_bctl->rd_index -= cur_bctl->slot_size * cur_bctl->slot_num ;
            if(cur_bctl->rd_index!=0) {
                RTNA_DBG_Str3("Bad Rd Indx\r\n");
            }    
        }
    } else {        
    	cur_bctl->rd_index += slots*cur_bctl->slot_size;
    	cur_bctl->total_rd +=  slots;
    	if(cur_bctl->rd_index  >= ( cur_bctl->slot_size * cur_bctl->slot_num ) ) {
            cur_bctl->rd_index -= cur_bctl->slot_size * cur_bctl->slot_num;    
        }
    }
}

void MMPF_Video_UpdateRdPtrByPayloadLength(/*PIPE_PATH pipe*/MMP_UBYTE ep_id)
{
    STREAM_CFG *cur_stream ,cur_stream_tmp=*(STREAM_CFG *)usb_get_cur_image_pipe_by_epid(ep_id);
    MMP_BOOL pipe01_sim ;
    
    cur_stream = &cur_stream_tmp ;
    
    cur_stream->pipe_en &=~PIPEEN(GRA_SRC_PIPE);
    
    if(UVC_VCD()==bcdVCD10) { 
        pipe01_sim = IS_PIPE01_EN(cur_stream->pipe_en) /*|| (UVCX_IsSimulcastH264()) */;
    } else {
        pipe01_sim = IS_PIPE01_EN(cur_stream->pipe_en) ;
    }
    if( pipe01_sim ) {
        FRAME_PAYLOAD_HEADER *ph0,*ph1,*ph2 ;
        ph0 = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_0) ;
        ph1 = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1) ;
        if(ph0) {
            if(ph0->UVC_H264_PH.dwPayloadSize && (cur_stream->pipe_sync&0x01 ) ) {
                MMPF_Video_UpdateRdPtr(PIPE_0) ;   
            }    
        }
        if(ph1) {
            if(ph1->UVC_H264_PH.dwPayloadSize && (cur_stream->pipe_sync&0x02 ) ) {
                MMPF_Video_UpdateRdPtr(PIPE_1) ; 
            }    
        }
        
        // Update one more slot in simulcast H264
        if( cur_stream->pipe_sync&0x04  ) {
            ph1 = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1) ;
            if(ph1->UVC_H264_PH.dwPayloadSize) {
                //dbg_printf(3,"update wr1\r\n");
                MMPF_Video_UpdateRdPtr(PIPE_1) ; 
            }
        }
        
        if( cur_stream->pipe_sync&0x08  ) {
            ph1 = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1) ;
            if(ph1->UVC_H264_PH.dwPayloadSize) {
                //dbg_printf(3,"update wr1\r\n");
                MMPF_Video_UpdateRdPtr(PIPE_1) ; 
            }
        }
        
        if( cur_stream->pipe_sync&0x10  ) {
            ph2 = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_2) ;
            if(ph2->UVC_H264_PH.dwPayloadSize) {
                //dbg_printf(3,"update wr1\r\n");
                MMPF_Video_UpdateRdPtr(PIPE_2) ; 
            }
        }
        
        
    } 
    else if( IS_PIPE0_EN(cur_stream->pipe_en)) { 
        FRAME_PAYLOAD_HEADER *ph ;
        ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_0) ;
        if(ph) {
            if(ph->UVC_H264_PH.dwPayloadSize) {
                //dbg_printf(3,"pipe0:%d\r\n",ph->UVC_H264_PH.dwPayloadSize );
                MMPF_Video_UpdateRdPtr(PIPE_0) ;
            }    
        }
    }
    else if( IS_PIPE1_EN(cur_stream->pipe_en)) { 
        FRAME_PAYLOAD_HEADER *ph ;
        ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_1) ;
        if(ph) {
            if(ph->UVC_H264_PH.dwPayloadSize) {
                //dbg_printf(3,"pipe1:%d\r\n",ph->UVC_H264_PH.dwPayloadSize );
                MMPF_Video_UpdateRdPtr(PIPE_1) ;
            }    
        }
    }
    else {
        FRAME_PAYLOAD_HEADER *ph ;
        ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(PIPE_2) ;
        if(ph) {
            if(ph->UVC_H264_PH.dwPayloadSize) {
                MMPF_Video_UpdateRdPtr(PIPE_2) ;
            }    
        }
    }
}


MMP_UBYTE *MMPF_Video_CurRdPtr(PIPE_PATH pipe)
{
    //STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(ep_id);
    STREAM_CTL *cur_bctl ;
    MMP_UBYTE  *cur_ptr ;
    MMP_UBYTE  *cur_header ;
    
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    cur_ptr = (MMP_UBYTE *)cur_bctl->buf_addr ;
    //dbg_printf(3,"Cur rd,wr:%d,%d\r\n",cur_bctl->rd_index,cur_bctl->wr_index);
    
    #if ALIGN_PAYLOAD==0
    return &cur_ptr[cur_bctl->rd_index] ;
    #else
    cur_header = &cur_ptr[cur_bctl->rd_index] ;
    cur_header += cur_bctl->off_header;
    return cur_header; 
    #endif
}

#if 1//PCCAM_MSB==1
MMP_UBYTE *MMPF_Video_NextRdPtr(PIPE_PATH pipe,MMP_USHORT next_n)
{
    MMP_UBYTE  *cur_ptr ;
    MMP_UBYTE  *cur_header ;
    MMP_ULONG rd_index ;
    
    STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(pipe);
    STREAM_CTL *cur_bctl ;
    FRAME_PAYLOAD_HEADER *ph ;
#if 1//(SLOT_RING == 1)
    MMP_USHORT slots;
#endif
    MMP_BOOL    slicetx ;
    slicetx  = UVCX_IsSliceBasedTxStream(cur_stream);
    
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    if(cur_bctl->slot_size==0){
        dbg_printf(0,"fatal error:slot size==0\r\n");
        return 0 ;
    }
    cur_ptr = (MMP_UBYTE *)cur_bctl->buf_addr ;
    ph = (FRAME_PAYLOAD_HEADER *)MMPF_Video_CurRdPtr(pipe) ;
    if( ph ) {
#if 1
    if(slicetx ) {
        if(ph->UVC_H264_PH.dwPayloadSize) {
            slots = ( ph->UVC_H264_PH.dwPayloadSize + FRAME_PAYLOAD_HEADER_SZ + cur_bctl->slot_size - 1 + cur_bctl->off_header )
                            / cur_bctl->slot_size;
        } else {
            return 0;
        }
    }           
#endif
    }
    rd_index = cur_bctl->rd_index ;
    if( !slicetx  ) {
        rd_index += ( cur_bctl->slot_size * next_n );
        if(rd_index >= ( cur_bctl->slot_size * cur_bctl->slot_num ) ) {
            rd_index -= cur_bctl->slot_size * cur_bctl->slot_num ;
        }
    } else {
    	rd_index += slots*cur_bctl->slot_size;
    	if(rd_index >= ( cur_bctl->slot_size * cur_bctl->slot_num ) ) {
            rd_index -= cur_bctl->slot_size * cur_bctl->slot_num;    
        }
    }
    #if ALIGN_PAYLOAD==0
    return &cur_ptr[rd_index] ;
    #else
    cur_header = &cur_ptr[rd_index] ;
    cur_header += cur_bctl->off_header;
    return cur_header; 
    #endif
 
}
#endif


MMP_UBYTE *MMPF_Video_CurWrPtr( PIPE_PATH pipe)
{
    //STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(ep_id);
    STREAM_CTL *cur_bctl ;
    MMP_UBYTE  *cur_ptr ;
    MMP_UBYTE  *cur_header ;
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    cur_ptr = (MMP_UBYTE *)cur_bctl->buf_addr ;
    //dbg_printf(3,"Cur wr,rd:%d,%d\r\n",cur_bctl->wr_index,cur_bctl->rd_index);
    #if ALIGN_PAYLOAD==0
    return &cur_ptr[cur_bctl->wr_index] ;
    #else
    cur_header = &cur_ptr[cur_bctl->wr_index] ;
    cur_header += cur_bctl->off_header;
    return cur_header; 
    #endif

}

int MMPF_Video_DataCount(PIPE_PATH pipe)
{
   // STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe(ep_id);
    STREAM_CTL *cur_bctl ;
    int off ;
    
    cur_bctl = MMPF_Video_GetStreamCtlInfo(pipe) ;
    off = (int)(cur_bctl->total_wr - cur_bctl->total_rd) ;
    
    return off ;
}


void MMPF_Video_Init_Buffer(MMP_UBYTE ep_id,MMP_USHORT off_header)
{
    MMP_USHORT i ;
    STREAM_CFG *cur_stream =(STREAM_CFG *)usb_get_cur_image_pipe_by_epid(ep_id);
    STREAM_CTL *cur_bctl ;
    FRAME_PAYLOAD_HEADER *ph ;
    MMP_LONG   freefbsize;
    MMP_UBYTE  cur_pipe ;
    for(cur_pipe=0;cur_pipe<MMPF_IBC_PIPE_MAX;cur_pipe++) {
        if(PIPE_EN(cur_stream->pipe_en) & (1<<cur_pipe)) {
            cur_bctl = &cur_stream->pipe_b[cur_pipe];
            #if 0
            dbg_printf(3,"#pipe %d slot size : %d\r\n",cur_pipe,cur_bctl->slot_size);
            dbg_printf(3,"#pipe %d slot num  : %d\r\n",cur_pipe,cur_bctl->slot_num );
            dbg_printf(3,"#pipe %d frame num  : %d\r\n",cur_pipe,cur_bctl->frame_num );
            #endif
            
            if(cur_bctl->buf_addr) {
                for(i=0;i<cur_bctl->slot_num;i++) {
                    ph = (FRAME_PAYLOAD_HEADER *)(cur_bctl->buf_addr + i * cur_bctl->slot_size + off_header );
                    ph->UVC_H264_PH.dwPayloadSize = 0; 
                }
            }
            cur_bctl->off_header = off_header;
        }
    }

    
    dbg_printf(3,"Payload headeroff:%d,size:%d\r\n",cur_bctl->off_header,FRAME_PAYLOAD_HEADER_SZ) ;
    MMPF_SYS_GetCurFBAddr(&freefbsize,ep_id);
    dbg_printf(3,"Free FB size : %d KB\r\n",freefbsize / 1024 ); // 0->3
    
    if(freefbsize < 0) {
        dbg_printf(0,"Out of FB \r\n");
        while(1);
    }
}


//sean@2011_01_22, extend dma block to 30 for yuy2 + h264 
#define DMA_MAX_BLOCKS  30 // extend to 20 for YUY2 payload
STREAM_DMA_BLK gsDMAMaxBlocks[2][DMA_MAX_BLOCKS] ;
STREAM_DMA_BLK *gsDMABlkFirstDesc[2] ;
STREAM_DMA_BLK *gsDMABlkCurDesc[2] ;
MMP_USHORT     gsDMAActiveBlocks[2] ;
// Add ep_id for multiple endpoint
void MMPF_Video_InitDMABlk(MMP_USHORT ep_id,MMP_USHORT max_dsize,MMP_ULONG dmabuf1,MMP_ULONG dmabuf2) 
{
    static MMP_ULONG cpu_sr;
    int i ;
    MMP_UBYTE gbl_ctl_flag = 0 ;
    STREAM_SESSION *ss ;
    if(ep_id >= VIDEO_EP_END()) {
        return ;
    }
    //dbg_printf(3,"ep_id:%d,maxpktsize:%d,flag:%x\r\n",ep_id,max_dsize,gbl_ctl_flag);
    //dbg_printf(3,"buf1:%x,buf2:%x\r\n",dmabuf1,dmabuf2);
    // May init from different task. 
    // Lock the global struture
    IRQ_LOCK(
        ss = MMPF_Video_GetStreamSessionByEp(ep_id);
        //if(ss->tx_flag & SS_TX_BY_FIFO) {
        //    gbl_ctl_flag |= TX_MODE_BIT(TX_BY_FIFO) ;
        //}
        
        gbl_ctl_flag |= TX_MODE_BIT( SS_TX_MODE(ss->tx_flag) ) ;
        
        if(ep_id==1) {
            gbl_ctl_flag |= TX_BY_EP2 ;
        }
        
        for(i=0;i<DMA_MAX_BLOCKS;i++) {
            gsDMAMaxBlocks[ep_id][i].max_dsize = max_dsize ;
            gsDMAMaxBlocks[ep_id][i].next_blk = 0;
            gsDMAMaxBlocks[ep_id][i].header_len = 0;
            gsDMAMaxBlocks[ep_id][i].blk_addr = 0;
            gsDMAMaxBlocks[ep_id][i].blk_size = 0;
            gsDMAMaxBlocks[ep_id][i].cur_addr   = 0;
            gsDMAMaxBlocks[ep_id][i].tx_len     = 0;
            gsDMAMaxBlocks[ep_id][i].tx_packets     = 0;
            gsDMAMaxBlocks[ep_id][i].dma_buf[0] = dmabuf1 ;
            gsDMAMaxBlocks[ep_id][i].dma_buf[1] = dmabuf2 ;
            gsDMAMaxBlocks[ep_id][i].ctrl_flag = gbl_ctl_flag ;
        }
        gsDMABlkFirstDesc[ep_id] = 0;
        gsDMABlkCurDesc[ep_id] = 0;
        gsDMAActiveBlocks[ep_id] = 0; 
    )
    
}

//
// return :
// 0 : The block size is not empty
// 1 : The block size is empty
//
MMP_BOOL MMPF_Video_AddDMABlk(MMP_USHORT ep_id,MMP_ULONG header_len,MMP_ULONG blk_addr,MMP_ULONG blk_size,MMP_USHORT ctl_flag)
{
    static MMP_ULONG cpu_sr;
    STREAM_DMA_BLK *blk,*nextBlk;
    
    
    if(ep_id >= VIDEO_EP_END()) {
        return 1;
    }
    IRQ_LOCK(
    
 
        nextBlk = (STREAM_DMA_BLK *)&gsDMAMaxBlocks[ep_id][gsDMAActiveBlocks[ep_id] ] ; 
        nextBlk->next_blk = 0;
        nextBlk->header_len = header_len ;
        nextBlk->blk_addr = blk_addr ;
        nextBlk->blk_size = blk_size ;
        nextBlk->cur_addr = blk_addr ;
        nextBlk->ctrl_flag |= ctl_flag ; // = -> |= 
        if(!gsDMABlkFirstDesc[ep_id]) {
            gsDMABlkFirstDesc[ep_id] = nextBlk ;
            gsDMABlkCurDesc[ep_id] = nextBlk ;
        } else {
            blk = gsDMABlkFirstDesc[ep_id] ;
            while(blk->next_blk) {
                blk = (STREAM_DMA_BLK *)blk->next_blk ;
            }
            blk->next_blk = (MMP_ULONG)nextBlk ;
        }
        gsDMAActiveBlocks[ep_id]++;
    // sean@2011_01_22, lock system if dma block overflow
        if(gsDMAActiveBlocks[ep_id] >= DMA_MAX_BLOCKS) {
            RTNA_DBG_Str0("#Too Many DMA Blocks...\r\n");
            while(1);
        }
    )
    
    if(blk_size) {
        return 0 ;
    }
    return 1 ;
}

MMP_BOOL MMPF_Video_AddDMABlkH264SliceMode(MMP_USHORT ep_id,MMP_ULONG header_len,MMP_ULONG blk_addr,MMP_UBYTE sid,MMP_BOOL sof)
{
#define OUTPUT_32BYTES_HDR  0
    MMP_BOOL emptyblk = 0 ;
    STREAM_SESSION *ss ;
    MMP_ULONG low_end, high_end;
    STREAM_CFG      *cur_stream ;
    STREAM_CTL      *cur_ctl ;
   // MMP_UBYTE       cur_pipe,pipe_cfg;
    
    FRAME_PAYLOAD_HEADER *ph;
    
#if H264_CBR_PADDING_EN==1
    MMP_ULONG cbr_padding_bytes = 0;
#endif
#if SUPPORT_LYNC_SIMULCAST
    MMP_BOOL rm_prefix = MMP_FALSE ;
    MMPF_H264ENC_ENC_INFO   *pEnc = MMPF_H264ENC_GetHandle(sid);
    if( UVCX_IsSimulcastH264() ) {
        if( (pEnc->total_layers==1) && (pEnc->cohdr_option & MMPF_H264ENC_COHDR_PNALU ) && (!sof) ) {
           // rm_prefix = MMP_TRUE ; // Lync spec.
        }
    }
#endif     
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    cur_stream = usb_get_cur_image_pipe_by_epid(ep_id);
    
    //dbg_printf(3,"blk addr : %x,ep_id :%d\r\n",ep_id,blk_addr );
    
    ph = (FRAME_PAYLOAD_HEADER *)blk_addr ;
    if(!ph || !cur_stream || !ss) {
        return 1;
    }
    if(ss->framelength) {
        MMP_USHORT lastSlice = 0 ;
        MMP_USHORT rm_len = 0,tx_len = ss->framelength ;
        MMP_ULONG  tx_addr = blk_addr + FRAME_PAYLOAD_HEADER_SZ ; 
#if SUPPORT_LYNC_SIMULCAST
        if(rm_prefix) {
            MMP_UBYTE *ptr8  = (MMP_UBYTE *)( tx_addr) ;
            MMP_ULONG *ptr32 = (MMP_ULONG *)( tx_addr) ; 
            if( ptr32[0]==0x01000000) {
                if(ptr8[4]==0x0E) {
                    rm_len = 8 ;
                }
                else if (ptr8[4]==0x6E) {
                    rm_len = 9;
                }
                //dbg_printf(0,"--rm.len : %d\r\n",rm_len);
            }
           // rm_len = 0 ;
            tx_addr += rm_len ;
            tx_len  -= rm_len ;
        }
#endif    
        
        if( IS_PIPE0_EN(cur_stream->pipe_en) ) {
            cur_ctl =  &cur_stream->pipe_b[PIPE_0];
        } else {
            cur_ctl =  &cur_stream->pipe_b[PIPE_1];
        }
        
        //Get Slot Boundary.
        low_end =  cur_ctl->buf_addr;
        high_end = low_end + cur_ctl->slot_size * cur_ctl->slot_num;
#if OUTPUT_32BYTES_HDR==1 // TESTCODE
        if((MMP_ULONG)(blk_addr -6 + 6 + FRAME_PAYLOAD_HEADER_SZ + ss->framelength) > high_end) {
            MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(blk_addr - 6), high_end - (MMP_ULONG)(blk_addr - 6 ) ,0);  
            lastSlice =( ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_EOF ) ? DMA_CF_EOF : 0;
            #if H264_CBR_PADDING_EN==1
            cbr_padding_bytes = usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)blk_addr) ;
            if(cbr_padding_bytes) {
                MMPF_Video_AddDMABlk(ep_id,header_len, low_end, ss->framelength + 32 - (high_end - (MMP_ULONG)(blk_addr - 6)) ,0);
                MMPF_Video_AddDMABlk(ep_id,header_len, usb_vs_get_cbr_padding_buffer(),cbr_padding_bytes ,DMA_CF_DUMMY | lastSlice);
            } 
            else {
                MMPF_Video_AddDMABlk(ep_id,header_len,low_end , ss->framelength + 32 - (high_end - (MMP_ULONG)(blk_addr - 6)) ,lastSlice);
            }
            #else
            MMPF_Video_AddDMABlk(ep_id,header_len, low_end, ss->framelength + 32 - (high_end - (MMP_ULONG)(blk_addr - 6)) ,lastSlice);
            #endif
            //dbg_printf(3,"  slice 1:%d,size:%d,%d\r\n",lastSlice,ph->UVC_H264_PH.dwPayloadSize,ss->framelength );
        }	
        else {                
            lastSlice =( ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_EOF ) ? DMA_CF_EOF : 0;
            #if H264_CBR_PADDING_EN==1
            cbr_padding_bytes = usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)blk_addr) ;
            if(cbr_padding_bytes) {
                MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(blk_addr - 6),ss->framelength  + 6 + FRAME_PAYLOAD_HEADER_SZ ,0); 
                MMPF_Video_AddDMABlk(ep_id,header_len, usb_vs_get_cbr_padding_buffer(),cbr_padding_bytes ,DMA_CF_DUMMY | lastSlice);
                //dbg_printf(3,"last slice [%d]: %d\r\n",lastSlice,cbr_padding_bytes);
            }
            else {
                MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(blk_addr - 6),ss->framelength  + 6 + FRAME_PAYLOAD_HEADER_SZ ,lastSlice); 
            }
            #else
            MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(blk_addr - 6),ss->framelength  + 6 + FRAME_PAYLOAD_HEADER_SZ ,lastSlice); 
            #endif
            //dbg_printf(3,"  slice 2:%d,size:%d,%d\r\n",lastSlice,ph->UVC_H264_PH.dwPayloadSize,ss->framelength );
        }	
#else
        if((MMP_ULONG)( tx_addr + tx_len ) > high_end) {
            MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(tx_addr), high_end - (MMP_ULONG)(tx_addr ) ,0);  
            lastSlice =( ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_EOF ) ? DMA_CF_EOF : 0;
            
            #if H264_CBR_PADDING_EN==1
            cbr_padding_bytes = usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)(blk_addr) ) ;
            if(cbr_padding_bytes) {
                MMPF_Video_AddDMABlk(ep_id,header_len, low_end, tx_len - (high_end - (MMP_ULONG)(tx_addr  )) ,0);
                MMPF_Video_AddDMABlk(ep_id,header_len, usb_vs_get_cbr_padding_buffer(),cbr_padding_bytes ,DMA_CF_DUMMY | lastSlice);
                //dbg_printf(3,"1_last slice [%d]: %d\r\n",lastSlice,cbr_padding_bytes);
            }
            else {
                MMPF_Video_AddDMABlk(ep_id,header_len, low_end, tx_len - (high_end - (MMP_ULONG)(tx_addr)) ,lastSlice);
            }
            #else
            MMPF_Video_AddDMABlk(ep_id,header_len, low_end, tx_len - (high_end - (MMP_ULONG)(tx_addr )) ,lastSlice);
            #endif
        }	
        else {                
            lastSlice =( ph->UVC_H264_PH.dwFrameRate & UVC_PH_FLAG_EOF ) ? DMA_CF_EOF : 0;
            #if H264_CBR_PADDING_EN==1
            cbr_padding_bytes = usb_uvc_get_cbr_padding_bytes( (MMP_UBYTE *)( blk_addr )) ;
            if(cbr_padding_bytes) {
                MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(tx_addr ),tx_len  ,0);
                MMPF_Video_AddDMABlk(ep_id,header_len, usb_vs_get_cbr_padding_buffer(),cbr_padding_bytes ,DMA_CF_DUMMY | lastSlice);
                //dbg_printf(3,"2_last slice [%d]: %d\r\n",lastSlice,cbr_padding_bytes);
            }
            else {
                MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(tx_addr),tx_len  ,lastSlice);
            }
            #else
            MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(tx_addr ), tx_len ,lastSlice); 
            #endif
            
        }	

#endif

    } else {
        emptyblk = MMPF_Video_AddDMABlk(ep_id,header_len,(MMP_ULONG)(blk_addr + FRAME_PAYLOAD_HEADER_SZ),0 ,DMA_CF_EOF);
    }
    return emptyblk ;
}

STREAM_DMA_BLK *MMPF_Video_CurBlk(MMP_USHORT ep_id)
{
    static MMP_ULONG cpu_sr;
    STREAM_DMA_BLK *blk ;
    if(ep_id >= VIDEO_EP_END()) {
        return (STREAM_DMA_BLK *)0;
    }
    IRQ_LOCK(
        blk = gsDMABlkCurDesc[ep_id];
    )
    
    return blk ;
}


MMP_BOOL MMPF_Video_NextBlk(MMP_USHORT ep_id,MMP_BOOL tx_by_fifo)
{
    static MMP_ULONG cpu_sr;
    MMP_BOOL ret = 1;
    
    if(ep_id >= VIDEO_EP_END()) {
        return 0;
    }
    IRQ_LOCK(    
        if(gsDMABlkCurDesc[ep_id]==0) {
            ret = 0;
        }
        if(!gsDMABlkCurDesc[ep_id]->next_blk) {
            ret = 0;
        }
        if(ret) {
            gsDMABlkCurDesc[ep_id] = (STREAM_DMA_BLK *)gsDMABlkCurDesc[ep_id]->next_blk ;
        }
    )
    
    return ret;
}


MMP_UBYTE *MMPF_Video_GetBlkAppHeader(STREAM_DMA_BLK *dma_blk)
{
// sean@2011_01_22, app marker using app3 or app4
//#define APP_MARKER 0xFFE3 // move to config_fw.h

#if APP_MARKER==0xFFE3
static MMP_UBYTE gbAppMarker[4] = {0xFF,0xE3,0x00,0x00 } ;
#endif
#if APP_MARKER==0xFFE4
static MMP_UBYTE gbAppMarker[4] = {0xFF,0xE4,0x00,0x00 } ;
#endif

    MMP_USHORT app_len = 0;
    if(dma_blk&& (dma_blk->tx_len==0)) {
    
        if( dma_blk->header_len == 4 ) { 
            app_len = dma_blk->blk_size + 2 ;
            gbAppMarker[2] = ( app_len >> 8 ) & 0xFF ;
            gbAppMarker[3] = ( app_len &  0xFF ) ;
            return (MMP_UBYTE *)gbAppMarker ;
        }
        
        return (MMP_UBYTE *)0;
    }
    return (MMP_UBYTE *)0;
}


extern MMPF_OS_FLAGID   SYS_Flag_Hif;
#include "mmpf_msg.h"
//#include "ucos_ii.h"

//extern OS_TCB *OSTCBCur ;

void MMPF_USB_ReleaseDm(MMP_UBYTE pipe)
{
    MMP_UBYTE ep_id = usb_get_cur_image_ep(pipe) ;
    STREAM_SESSION *ss =  MMPF_Video_GetStreamSessionByEp(ep_id);
    //dbg_printf(0,"[%s] ep/pipe id : %d/%d,flag:%x\r\n",OSTCBCur->OSTCBTaskName,ep_id,pipe,ss->tx_flag);
    if(ss->tx_flag & SS_TX_CLOSE_STREAMING) {
        //dbg_printf(0,"[%s]release ep/pipe id : %d/%d,flag:%x\r\n",OSTCBCur->OSTCBTaskName,ep_id,pipe,ss->tx_flag);
  		usb_vs_next_packet(ep_id);
    }
}

extern STREAM_SESSION glStreamSession[];
void MMPF_Video_InitStreamSession(MMP_UBYTE ep_id,MMP_UBYTE tx_flag,MMP_USHORT max_packet_size)
{
    AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    
    int workss ;
    
    workss = MMPF_Video_GetWorkingStreamSession();
    if(workss==0) {
        int i;
        for(i=0;i<VIDEO_EP_END();i++) {
            if( !(tx_flag & SS_TX_STREAMING) ){ // Fixed dynamic change resolution
                MMPF_Video_GetStreamSessionByEp(i)->streamaddr = 0;
            }
        }
    }   
    
    ss->frameaddr = 0;
    ss->framelength =0;
    ss->packetcount =0;
    ss->usbframecount=0;
    ss->tx_flag =tx_flag ;
    ss->frametoggle =0;
    ss->ep_id = ep_id;
    ss->pipe_id = 0xFF;
    ss->maxpacketsize = max_packet_size;
    ss->STC = 0 ;
    
    ss->frame_stime = 0 ;
    ss->frame_etime = 0 ;
    // Init necessary parameters so that
    // no wrong stream format go into repack flow
    if(ep_id==0) {
        pUSB_CTL->USB_TX_INT_EN  &= ~EP1_TX_INT_BIT ;    
    } else {
        pUSB_CTL->USB_TX_INT_EN  &= ~EP2_TX_INT_BIT ;  
    }
    
    ss->uvc15_layerid = 0;
    //dbg_printf(3,"epid:%d is dma mode,max pkt size:%d\r\n",ep_id,max_packet_size);
    //dbg_printf(3,"ss->streamaddr : %x,ss->pipe_id :%d\r\n",ss->streamaddr,ss->pipe_id);
}

void MMPF_Video_CloseStreamSession(MMP_UBYTE ep_id)
{
extern MMPF_OS_SEMID    gUSBDMASem[] ;
    MMP_USHORT csr,ep ;
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    // Add for test
    UsbDmaStop(ep_id);
    ss->tx_flag &= ~SS_TX_NEXT_PACKET ; 
    ss->tx_flag &= ~(SS_TX_STREAMING | SS_TX_CONFIG_END);
    // ss->usbframecount = 0 ; // avoid the race condition from send_image
    //ss->pipe_id = 0xFF; 


    while( !(ss->tx_flag & SS_TX_EOS ) ) {
        //dbg_printf(3,"<closess :ss->tx_flag : %x>\r\n",  ss->tx_flag );
        //MMPF_OS_ReleaseSem( gUSBDMASem[ep_id] ); 
        MMPF_USB_ReleaseDMAList( ep_id );
        MMPF_USB_ReleaseDMA( ep_id); 
        MMPF_OS_Sleep(1);
    }
// move to here after streaming stop
    ss->pipe_id = 0xFF; 

    // sean@2012_08_15 reset stream pipe config
    usb_set_cur_image_pipe(ep_id,0);
    dbg_printf(3,"<stream session closed:[%x]>\r\n",ss->tx_flag);
}

void MMPF_Video_SetStreamSessionVsi(MMP_UBYTE ep_id)
{
extern UVC_VSI_INFO    gCurVsiInfo[]; 
extern MMP_USHORT gsCurFrameRatex10[];
    MMP_USHORT w,h ;
    STREAM_SESSION *ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    UVC_VSI_INFO *vsi = (UVC_VSI_INFO *)&gCurVsiInfo[ep_id];
    ss->curvsi.vsi_if = vsi->bInterface ;
    if(vsi->bFormatIndex==YUY2_FORMAT_INDEX) {
        ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_YUV422 ;
    }
    else if(vsi->bFormatIndex==MJPEG_FORMAT_INDEX) {
        ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_MJPEG ;
    }
    else if (vsi->bFormatIndex==FRAME_BASE_H264_INDEX) {
        ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_H264;
    }
    else if (vsi->bFormatIndex==NV12_FORMAT_INDEX) {
        ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_YUV420 ;
    }
    // remap uvc frame index to internal res index
    ss->curvsi.vsi_frmindx = MMPF_Video_GetInteralVsiFrameIndex(vsi->bFrameIndex); //  start from 0  mapping UVC frame index to internal index
    dbg_printf(3,"streamvsi : (%d,%d,%d)\r\n", ss->curvsi.vsi_if,ss->curvsi.vsi_fmtindx,ss->curvsi.vsi_frmindx );
    
    ss->maxdmadatasize = (ep_id==0)?EP1_DMA_LIST_MAX_SIZE:EP2_DMA_LIST_MAX_SIZE;
    ss->tx_flag &= ~ ( SS_TX_BY_ONE_SHOT | SS_TX_RT_MODE) ;
    if(ss->curvsi.vsi_fmtindx==PCAM_USB_VIDEO_FORMAT_YUV422) {
    #if YUY2_848_480_30FPS > 0
        if( (ss->curvsi.vsi_frmindx ==PCCAM_848_480) && (gsCurFrameRatex10[ep_id]>=300)) {
            MMP_ULONG maxpktsize = ss->maxpacketsize - UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] ;
            ss->tx_flag |= SS_TX_BY_ONE_SHOT ;
            ss->maxdmadatasize = GetYUY2FrameSize(PCCAM_848_480);
            ss->maxdmadatasize = maxpktsize  * ((ss->maxdmadatasize + maxpktsize  - 1) / maxpktsize  );
            dbg_printf(3,"<ss->one shot mode:%d\r\n",ss->maxdmadatasize);

        }    
    #endif
    #if YUY2_TX_RT_MODE==1
        if((UVC_VCD()==bcdVCD10)  ) { // For Lync Test uvc1.0 only
            if( (ss->curvsi.vsi_frmindx == PCCAM_640_480) ||
                (ss->curvsi.vsi_frmindx == PCCAM_640_360) ||
                (ss->curvsi.vsi_frmindx == PCCAM_320_240) ||
                (ss->curvsi.vsi_frmindx == PCCAM_320_180) ||
                (ss->curvsi.vsi_frmindx == PCCAM_424_240) ) { // 320x180,320x240,424x240,640x360,640x480
                
                if( (gsCurFrameRatex10[ep_id]>=300)) {
                    ss->tx_flag |= SS_TX_RT_MODE ;
                    dbg_printf(0,"<*ss->tx_rt_mode>\r\n");
                }
                else {
                    ss->tx_flag &= ~SS_TX_RT_MODE ;
                }
            }
        }
    #endif        
    }
    
}

MMP_UBYTE MMPF_Video_GetInteralVsiFrameIndex(MMP_UBYTE frameindex)
{
#if 1 // AIT     
    RES_TYPE_CFG *res ;
    MMP_UBYTE index ;
    res = GetResCfg(frameindex-1);
    index = GetResIndexBySize(res->res_w,res->res_h) ;
    return index ;
#endif

#if 0
    // Logitech
#endif    
}



STREAM_SESSION *MMPF_Video_GetStreamSessionByEp(MMP_UBYTE ep_id)
{
    if(ep_id >= VIDEO_EP_END() ) {
        //dbg_printf(3,"Bad ep_id :%d link to SS\r\n",ep_id);
        return (STREAM_SESSION *)0 ;
    }
   // dbg_printf(3,"getss.pipeid:%d\r\n",glStreamSession[ep_id].pipe_id);
    return (STREAM_SESSION *)&glStreamSession[ep_id] ;
}

// pipe_id : start from 0
STREAM_SESSION *MMPF_Video_GetStreamSessionByPipe(MMP_UBYTE pipe_id)
{
    STREAM_SESSION *ss ;
    MMP_UBYTE i ;
    if(pipe_id >= 2 ) {
        return (STREAM_SESSION *)0 ;
    }
    for(i=0;i<VIDEO_EP_END();i++) {
        ss = MMPF_Video_GetStreamSessionByEp(i) ;
        if(ss->pipe_id==pipe_id) {
            return ss;
        }
    }
    return (STREAM_SESSION *)0 ;
}


int MMPF_Video_GetWorkingStreamSession(void)
{
    MMP_UBYTE i ;
    int c = 0 ;
    STREAM_SESSION *ss;
    for(i=0;i< VIDEO_EP_END();i++) {
        ss=MMPF_Video_GetStreamSessionByEp(i);  
        if( ss->tx_flag & SS_TX_STREAMING ) {
            c++ ;
        }  
    }
    //dbg_printf(3,"worksss : %d\r\n",c);
    return c ;
}

void MMPF_Video_AllocStreamPipe(MMP_UBYTE ep_id,STREAM_CFG *stream_cfg_in )
{
    STREAM_SESSION *ss ;
    STREAM_CFG *stream_cfg,stream_cfg_tmp = *stream_cfg_in ;
    stream_cfg = &stream_cfg_tmp ;
    stream_cfg->pipe_en &= ~PIPEEN(GRA_SRC_PIPE) ;
    
    ss=MMPF_Video_GetStreamSessionByEp(ep_id);
    
    
    if( IS_PIPE01_EN(stream_cfg->pipe_en)) {
        ss->pipe_id = 0;
    } 
    else if ( IS_PIPE0_EN(stream_cfg->pipe_en) ) {
        ss->pipe_id = 0;
    }
    else if ( IS_PIPE1_EN(stream_cfg->pipe_en) ) {
        ss->pipe_id = 1 ;
    }  
    else if ( IS_PIPE2_EN(stream_cfg->pipe_en) ) {
        ss->pipe_id = 2 ;
    } 
    dbg_printf(3,"allocstream_epid:%d,pipeid:%d,scfg:%x,en:%x\r\n",ep_id,ss->pipe_id,stream_cfg_in,stream_cfg_in->pipe_en);
    usb_set_cur_image_pipe(ep_id,stream_cfg_in);
}


MMP_UBYTE MMPF_Video_GetFreeStreamPipe(void)
{
    STREAM_SESSION *ss ;
    ss = MMPF_Video_GetStreamSessionByEp(0);
    if(ss->pipe_id!=0xFF) {
        return 1 ;
    } else {
        return 0 ;
    }
    
}

MMP_BOOL MMPF_Video_IsSyncStreamMode(PIPE_PATH pipe)
{
    STREAM_SESSION *ss ;
    ss = MMPF_Video_GetStreamSessionByPipe(pipe);
    return (ss->tx_flag & SS_TX_SYNC_STREAMING) ? TRUE : FALSE ;
}


MMP_UBYTE MMPF_Video_IsLoop2GRA(STREAM_CFG *stream_cfg)
{
    MMP_UBYTE gra_path = 0;
    MMP_UBYTE cur_pipe_en = 0;
    MMP_BOOL enable;
    #if SUPPORT_GRA_ZOOM
    if( UVC_VCD()==bcdVCD10 ) { // turn on lossy zoom for uvc1.0
    //    sean@2013_08_06 fixed bug for returning wrong information in uvc 1.0
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&enable,STREAM_EP_MJPEG) ;
        if(enable) {
            if( PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN ) {
                cur_pipe_en |= PIPEEN(GRA_DST_PIPE_1);
            }
            if( PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN ) {
                cur_pipe_en |= PIPEEN(GRA_DST_PIPE_2);
            }
        
        }
    } else {
    
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&enable,STREAM_EP_MJPEG);
        if (enable)	{	
            cur_pipe_en |= PIPEEN(GRA_DST_PIPE_1);	
        }
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&enable,STREAM_EP_H264 ); 
        if (enable)	{
        	cur_pipe_en |= PIPEEN(GRA_DST_PIPE_2);	
        }
    }
    
   // dbg_printf(3,"#Is Loop2GRA : %x,%x\r\n",stream_cfg->pipe_en,stream_cfg->pipe_cfg);
    if((stream_cfg->pipe_en&PIPE_EN_LOOP2GRA)) {  
        gra_path |= 0x1 ; // stream config support GRA loop
        if( cur_pipe_en & PIPEEN(GRA_DST_PIPE_1) ) {
            if( MMPF_Scaler_GetPath(GRA_DST_PIPE_1)==MMPF_SCALER_SOURCE_GRA ) {
                gra_path |= 0x02 ; // pipe 0 is in GRA loop
            }
        }
        if( cur_pipe_en & PIPEEN(GRA_DST_PIPE_2) ) {
            if( MMPF_Scaler_GetPath(GRA_DST_PIPE_2)==MMPF_SCALER_SOURCE_GRA ) {
                gra_path |= 0x04 ; // pipe 1 is in GRA loop
            }
        }
    }  
    #endif
    return gra_path ;
}

MMP_BOOL MMPF_Video_SetLoop2GRA(STREAM_CFG *stream_cfg,MMP_BOOL en)
{
    if(en) {
        stream_cfg->pipe_en  |= PIPE_EN_LOOP2GRA ;   
        stream_cfg->pipe_en  |= PIPEEN(GRA_SRC_PIPE);  
        stream_cfg->pipe_cfg |= PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12);
    } 
    else {
        stream_cfg->pipe_en  &= ~PIPE_EN_LOOP2GRA ;
        stream_cfg->pipe_en  &= ~PIPEEN(GRA_SRC_PIPE);  
        stream_cfg->pipe_cfg &= ~PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12);
    }
    dbg_printf(3,"#Loop2GRA : en: %x,cfg:%x\r\n",stream_cfg->pipe_en,stream_cfg->pipe_cfg);
}


#endif
