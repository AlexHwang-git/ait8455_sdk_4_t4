#include "config_fw.h"
#include "reg_retina.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_jpeg.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_icon.h"
#include "mmp_reg_ibc.h"
#include "mmpf_display.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbcolor.h"
#include "mmpf_usbuvc_event.h"
#include "mmpf_usbuvc_vs.h"
#include "mmpf_usbuvc15h264.h"
#include "mmps_3gprecd.h"
#include "mmpf_timer.h" 
#include "lib_retina.h"
#include "pcam_usb.h"
#include "mmpf_usbuvc_fdtc.h"
#include "mmpd_system.h"
#include "mmpf_usbuvch264.h"
#include "mmpf_system.h"
#include "Led_LM36922.h"

static void usb_uvc_switch_scaler_pllsrc(void);
static void usb_uvc_init_stream(MMP_UBYTE ep_id,MMP_BOOL mjpeg_stream);
static void usb_uvc_start_stream(MMP_UBYTE ep_id,MMP_BOOL mjpeg_stream,STREAM_CFG *stream_cfg,MMP_ULONG res);
static void usb_vc_init_sensor_param(MMP_UBYTE ep_id) ;
static void usb_vc_init_ct_param(void) ;
static void usb_uvc_config_pipe(STREAM_CFG *stream_cfg,PCAM_USB_VIDEO_RES pipe0_res,PCAM_USB_VIDEO_RES pipe1_res,PCAM_USB_VIDEO_RES pipe2_res );
static MMP_BOOL usb_uvc_config_stream(MMP_UBYTE ep_id);
static void usb_uvc_set_h264_mode(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex);
static void usb_uvc_config_header_length(MMP_UBYTE ep_id) ;


extern H264_FORMAT_TYPE gbCurH264Type ; //SKYPE_H264;//UVC_H264 ;
extern MMP_UBYTE        gbCurLocalStreamType ;

extern UVC_VSI_INFO gCurVsiInfo[2] ;
extern MMP_UBYTE    gbStillCaptureEvent ;
extern MMP_BOOL     gbSimulcastH264 ;
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_BOOL     m_bVidRecdPreviewStatus[];
extern MMP_UBYTE    gbDevicePowerSavingStatus;
//extern MMP_UBYTE    gbCurFrameRate[];
extern MMP_USHORT    gsCurFrameRatex10[];
extern MMP_ULONG    glPCCamCnt0,glPCCamCnt1;
extern MMP_UBYTE    gbEnableH264Mode ;
//=====================================================
extern MMP_USHORT  gsAFocus;
extern MMP_USHORT  gsZoom;
extern MMP_USHORT  gsBacklight;
extern MMP_USHORT  gsTemper;
extern MMP_USHORT  gsSaturation;
extern MMP_USHORT  gsContrast;
extern MMP_USHORT  gsBrightness;
extern MMP_SHORT   gsHue;
extern MMP_SHORT   gsGamma;
extern MMP_SHORT   gsSharpness;
extern MMP_SHORT   gsGain ;
extern MMP_LONG    glPan,glTilt  ;
extern MMP_UBYTE   gbAWB,gbAE ;
extern MMP_ULONG   glEV; 
extern MMP_BOOL    gbAutoFocus;
extern MMP_UBYTE   gbBandFilter;
extern MMP_UBYTE   usStoredBrightStep;
//=====================================================

// Stream CFG : YUY2
#if SUPPORT_GRA_ZOOM==0
STREAM_CFG gsYUY2Stream_Cfg =
{
    VIDEPEN(0) |  PIPE0_EN  ,
    PIPE_CFG(0,PIPE_CFG_YUY2),
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA},
} ;
#else
STREAM_CFG gsYUY2Stream_Cfg =
{
    VIDEPEN(0) | /*PIPEEN(GRA_SRC_PIPE) |*/ PIPEEN(GRA_DST_PIPE_1) , 
    /*PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12) |*/ PIPE_CFG(GRA_DST_PIPE_1,PIPE_CFG_YUY2),
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_1},
} ;
#endif

// Stream CFG : Bayer Raw
STREAM_CFG gsRawStream_Cfg =
{
    VIDEPEN(0) | PIPE0_EN  ,//no pipe
    PIPE_CFG(0,PIPE_CFG_YUY2),
    {PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA},
} ;
// Steam CFG : MJPEG
#if SUPPORT_GRA_ZOOM==0
STREAM_CFG gsMJPEGStream_Cfg =
{
    VIDEPEN(0) | PIPE0_EN , 
    PIPE_CFG(0,PIPE_CFG_MJPEG),
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA},
} ;
#else
STREAM_CFG gsMJPEGStream_Cfg =
{
    VIDEPEN(0) | /*PIPEEN(GRA_SRC_PIPE) |*/ PIPEEN(GRA_DST_PIPE_1) , 
    /*PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12) |*/ PIPE_CFG(GRA_DST_PIPE_1,PIPE_CFG_MJPEG),
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_1},
} ;
#endif

#if 0
// Stream CFG : NV12
STREAM_CFG gsNV12Stream_Cfg =
{
    VIDEPEN(0) | PIPE0_EN   ,
    PIPE_CFG(0,PIPE_CFG_NV12),
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA},
} ;
#else
// Stream CFG : NV12 for FDTC in pipe 2
STREAM_CFG gsNV12Stream_Cfg =
{
    VIDEPEN(0) | PIPE2_EN   ,
    PIPE_CFG(2,PIPE_CFG_NV12),
    {PIPE_PH_TYPE_NA,PIPE_PH_TYPE_NA,PIPE_PH_TYPE_1},
} ;

STREAM_CFG *gsFdtcStreamCfg = &gsNV12Stream_Cfg ;

#endif
// Stream CFG : Skype H264 ( YUY2 + H264 )
STREAM_CFG gsSkypeH264Stream_Cfg =
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN  ,
    PIPE_CFG(0,PIPE_CFG_YUY2) | PIPE_CFG(1,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_2,PIPE_PH_TYPE_2,PIPE_PH_TYPE_NA},
} ;

#if SUPPORT_GRA_ZOOM==0
// Stream CFG : UVC H264 (H264 + MJPEG dual stream)
STREAM_CFG gsH264MJPEGStream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN  ,
    PIPE_CFG(0,PIPE_CFG_MJPEG) | PIPE_CFG(1,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;

// Stream CFG : UVC H264 (H264 + YUY2 dual stream)
STREAM_CFG gsH264YUY2Stream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN ,
    PIPE_CFG(0,PIPE_CFG_YUY2) | PIPE_CFG(1,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;
#else
// Stream CFG : UVC H264 (H264 + MJPEG dual stream)
STREAM_CFG gsH264MJPEGStream_Cfg = 
{
    VIDEPEN(0) | PIPEEN(GRA_DST_PIPE_1) | PIPEEN(GRA_DST_PIPE_2) | PIPEEN(GRA_SRC_PIPE) ,
    PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12) | PIPE_CFG(GRA_DST_PIPE_1,PIPE_CFG_MJPEG) | PIPE_CFG(GRA_DST_PIPE_2,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;

// Stream CFG : UVC H264 (H264 + YUY2 dual stream)
STREAM_CFG gsH264YUY2Stream_Cfg = 
{
    VIDEPEN(0) | PIPEEN(GRA_DST_PIPE_1) | PIPEEN(GRA_DST_PIPE_2) | PIPEEN(GRA_SRC_PIPE) ,
    PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12) | PIPE_CFG(GRA_DST_PIPE_1,PIPE_CFG_YUY2) | PIPE_CFG(GRA_DST_PIPE_2,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;


#endif

#if SUPPORT_GRA_ZOOM==0
// Stream CFG : H264 pure stream (Frame Base H264)
STREAM_CFG gsH264PureStream_Cfg = 
{
    VIDEPEN(0) | PIPE1_EN  ,
    PIPE_CFG(1,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_NA,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;
#else
STREAM_CFG gsH264PureStream_Cfg = 
{
    VIDEPEN(0) | /*PIPEEN(GRA_SRC_PIPE) |*/ PIPEEN(GRA_DST_PIPE_2)  ,
    /*PIPE_CFG(GRA_SRC_PIPE,PIPE_CFG_NV12) |*/ PIPE_CFG(GRA_DST_PIPE_2,PIPE_CFG_H264) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
} ;

#endif
// Stream CFG : UVC H264 (MJPEG + YUY2) 
STREAM_CFG gsMJPEGYUY2Stream_Cfg =
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN ,
    PIPE_CFG(0,PIPE_CFG_YUY2) | PIPE_CFG(1,PIPE_CFG_MJPEG) ,
    {PIPE_PH_TYPE_1,PIPE_PH_TYPE_1,PIPE_PH_TYPE_NA},
};

#if SUPPORT_GRA_ZOOM==0
// Stream Viewer
STREAM_CFG gsH264FrameYUY2Stream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN ,
    PIPE_CFG(0,PIPE_CFG_YUY2) | PIPE_CFG(1,PIPE_CFG_H264) ,
    PIPE_PH_TYPE_3,PIPE_PH_TYPE_3,
} ;

STREAM_CFG gsH264FrameMJPEGStream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN ,
    PIPE_CFG(0,PIPE_CFG_MJPEG) | PIPE_CFG(1,PIPE_CFG_H264) ,
    PIPE_PH_TYPE_3,PIPE_PH_TYPE_3,
} ;
#else
// Stream Viewer
STREAM_CFG gsH264FrameYUY2Stream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN | PIPE2_EN,
    PIPE_CFG(2,PIPE_CFG_NV12) | PIPE_CFG(0,PIPE_CFG_YUY2) | PIPE_CFG(1,PIPE_CFG_H264) ,
    PIPE_PH_TYPE_3,PIPE_PH_TYPE_3,PIPE_PH_TYPE_NA
} ;

STREAM_CFG gsH264FrameMJPEGStream_Cfg = 
{
    VIDEPEN(0) | PIPE0_EN | PIPE1_EN | PIPE2_EN,
    PIPE_CFG(2,PIPE_CFG_NV12) | PIPE_CFG(0,PIPE_CFG_MJPEG) | PIPE_CFG(1,PIPE_CFG_H264) ,
    PIPE_PH_TYPE_3,PIPE_PH_TYPE_3,PIPE_PH_TYPE_NA
} ;

#endif



static MMPF_FCTL_LINK  gUvcPreviewFctlCfg[3] = {
    {MMPF_SCALER_PATH_0, MMPF_ICO_PIPE_0, MMPF_IBC_PIPE_0},
    {MMPF_SCALER_PATH_1, MMPF_ICO_PIPE_1, MMPF_IBC_PIPE_1},
    {MMPF_SCALER_PATH_2, MMPF_ICO_PIPE_2, MMPF_IBC_PIPE_2}
};

MMP_ULONG gbTestEnDbgOut = 3;

void usb_uvc_start_preview(MMP_UBYTE ep_id,MMP_BOOL change_res)
{
    int workss ;
    STREAM_SESSION *ss ;
   // UVC_VSI_INFO   *curvsi;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    MMP_BOOL IsPreviewEnable;
    MMP_ULONG dt0,dt1;
    MMP_ULONG pt0,pt1;
    
    MMP_UBYTE dbg_l = RTNA_DBG_Level(gbTestEnDbgOut);
    
	RTNA_DBG_Str(0, "----------start_preview------------------\r\n"); 
    //usb_uvc_set_h264_mode(vp.wIndex&0xFF,vp.wIndex>>8);
    gbChangeH264Res = change_res ;
#if 1
    if( (UVC_VCD()==bcdVCD15)&&(gCurVsiInfo[ep_id].bFormatIndex==FRAME_BASE_H264_INDEX)) {
    // saved cur frame descriptor / commit descriptor into internal struct
    // These function is temp for test
        // can't put commit here
        if(change_res==0) {
        //    dbg_printf(3,"Commit H264 frame & commit info...\r\n");
        //    UVC15_CommitH264FrameDesc(0,0);
        //    UVC15_CommitH264FrameDesc(1,0);
        //    UVC15_CommitH264CommitDesc(0);
        }
    } 
    
    STREAM_EP_YUY2 = 0;
    STREAM_EP_MJPEG= 0;
    STREAM_EP_NV12 = 0;
    
    if (VIDEO_EP_END()==2) {
        STREAM_EP_H264 = 1; // 2 video endpoint, H264 is in endpoint 2
    } else {
        STREAM_EP_H264 = 0; // 1 video endpoint, H264 is in endpoint 1
    
    }
    
    dbg_printf(3,"STREAM_EP_H264 :%d\r\n",STREAM_EP_H264);
 
 
    
#endif 

#if 1 // AIT specified test code
    if(!gbChangeH264BufMode) {   
        usb_uvc_set_h264_mode(gCurVsiInfo[ep_id].bFormatIndex,gCurVsiInfo[ep_id].bFrameIndex) ;
    }
#endif
    
    
    gbStillCaptureEvent = STILL_IMAGE_WAITING;
    #if 1
    check_task_stack_init();
    #endif
    
    workss = MMPF_Video_GetWorkingStreamSession();
    MMPS_3GPRECD_GetPreviewDisplay(&IsPreviewEnable);
    
    // Blink 6.4 Test
    if( workss <= VIDEO_EP_END() ) {
    
        MMP_BOOL resume = 0;
        PCAM_USB_VIDEO_FORMAT ep_fmt;
        MMP_UBYTE prio_err,old_prio,new_prio;

        MMP_BOOL mjpeg_stream ;
        MMPF_Video_SetStreamSessionVsi(ep_id);      
        
        // Change video format to Raw mode if necessary 
        if (USB_IsBayerRawPreview()) {
            STREAM_SESSION *ss;
            RTNA_DBG_Str0("#Bayer capture\r\n");
            pcam_get_info()->pCamVideoFormat = PCAM_USB_VIDEO_FORMAT_RAW;
            ss = MMPF_Video_GetStreamSessionByEp(ep_id);
            ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_RAW;
        }

        
        ep_fmt = GetFmtIndexByEp(ep_id);
        if(workss==VIDEO_EP_END()) {
            //MMPF_VIF_EnableInputInterface(MMP_FALSE);
        }
      //  gsVidFmt = pcam_get_info()->pCamVideoFormat;
      //  gsVidRes =  curvsi->bFrameIndex - 1;// glPccamResolution ;
        dt0 = usb_vc_cur_sof();
        
        if(! gbRestartPreview) {
            UsbDmaStop(ep_id);
        }
        // Init FDTC 
        //#if OMRON_FDTC_SUPPORT
        //usb_uvc_init_fdtc(1);
        //#endif
        
        prio_err = MMPF_OS_ChangePrio(TASK_AUDIO_RECORD_PRIO,TASK_1ST_PRIO);
        mjpeg_stream = usb_uvc_config_stream(ep_id);

        
        
        if( UVCX_IsSimulcastH264() ) {
            MMP_BOOL is_preview = MMP_FALSE ;
            if(UVC_VCD()==bcdVCD15) {
                MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview,STREAM_EP_H264 );
                
            }
            if(!gbChangeH264BufMode && !is_preview ) {
                MMP_BOOL scal0,scal1 ;
                scal0 = 0;//MMPF_Scaler_IsScalingUp(PIPE_0,gsZoom);
                scal1 = 0;//MMPF_Scaler_IsScalingUp(PIPE_1,gsZoom);

                if( scal0 ||  scal1 ) {
                    UVCX_SetH264EncodeBufferMode(REALTIME_MODE);
                }
                else {
                    UVCX_SetH264EncodeBufferMode(FRAME_MODE);
                }
            }
            
        }
        else {
            UVCX_SetH264EncodeBufferMode(FRAME_MODE/*REALTIME_MODE*/);
        }
        
       
        if(gbRestartPreview) {
             MMP_BOOL scalup = MMPF_Scaler_IsScalingUp(PIPE_0,gsZoom);
             if(scalup) {
                gbRestartPreview |= ( MMPF_SCALER_SOURCE_GRA << 4 ) ;
             }
             else {
                gbRestartPreview |= ( MMPF_SCALER_SOURCE_ISP << 4 ) ;
             }
             
             dbg_printf(3,"--gbRestartPreview:%x\r\n",gbRestartPreview);
        
        }
        //RTNA_DBG_Str(3,"USB_VideoPreviewStart\r\n");
        dbg_printf(0,"usb_uvc_start_preview - epid:%d, mjpeg:%d, ep_fmt:%d\r\n",ep_id, mjpeg_stream, ep_fmt);
        
        pt0 = usb_vc_cur_sof();

        USB_VideoPreviewStart(ep_id,PCAM_API/*PCAM_BLOCKING*/);
        if(MMPF_Sensor_GetErr()!=MMP_ERR_NONE) {
            m_VideoFmt = MMPS_3GPRECD_VIDEO_FORMAT_OTHERS ;
            prio_err = MMPF_OS_ChangePrio(TASK_1ST_PRIO,TASK_AUDIO_RECORD_PRIO); 
            return ;
        }

        pt1 = usb_vc_diff_sof(pt0);
        dbg_printf(3,"#Preview API time : %d ms\r\n",pt1);
        RTNA_DBG_Str3("uvc_init\r\n");
        uvc_init(0,ep_id);
        MMPF_USB_ResetDMAList(ep_id);
        MMPF_USB_ResetDMA(ep_id);

        if( ep_fmt != PCAM_USB_VIDEO_FORMAT_RAW) {
            // sean@2011_01_20 : Disable raw preview
            MMP_BOOL tmp =MMP_FALSE;
            MMPF_Sensor_SetParam(0,&tmp);
            usb_uvc_init_stream(ep_id,mjpeg_stream);
        }  
        else{        
            // Enable VIF input
            MMP_BOOL tmp =MMP_TRUE;
            MMPF_VIF_EnableInputInterface(MMP_TRUE);
            RTNA_DBG_Str(0,"MMPF_Sensor_CheckFrameStart\r\n");
            MMPF_Sensor_CheckFrameStart(2);
            RTNA_DBG_Str(0,"MMPF_RAWPROC_StoreSensorData\r\n");
            MMPF_RAWPROC_StoreSensorData(MMP_TRUE,MMP_FALSE);
            RTNA_DBG_Str(0,"MMPF_Sensor_SetParam\r\n");
            MMPF_Sensor_SetParam(0,&tmp);
            RTNA_AIC_IRQ_En(pAIC, AIC_SRC_VIF);
            m_bVidRecdPreviewStatus[ep_id] = 1;
        }
        
        
        #if USB_SPEEDUP_PREVIEW_TIME==1
        usb_vs_next_packet(ep_id);
        #else            
        usb_vs_send_image(0,0);
        #endif
        RTNA_DBG_Str3("#Streaming\r\n");

        gbDevicePowerSavingStatus = 0;
        #if SUPPORT_AUTO_FOCUS
        //ISP_IF_AF_Control(ISP_AF_STOP); //do not need 20121129
         MMPF_Sensor_AutoFocus();
       #endif

        prio_err = MMPF_OS_ChangePrio(TASK_1ST_PRIO,TASK_AUDIO_RECORD_PRIO); 
        
    }
    dt1 = usb_vc_diff_sof(dt0);
    dbg_printf(3,"#Preview interface time : %d ms\r\n",dt1);
    //if(workss < VIDEO_EP_END()) {
    if( IS_FIRST_STREAM(workss) && (change_res==0)) {
        dbg_printf(3,"#Preview Init PWM\r\n");
        #if (CHIP==P_V2)
        MMPF_PWM_SetAttribe(&pulseA);	
        MMPF_PWM_EnableInterrupt(MMPF_PWM_ID_1,MMP_TRUE,NULL,2);
        MMPF_PWM_ControlSet(MMPF_PWM_ID_1, (PWM_PULSE_A_FIRST|PWM_AUTO_CYC|PWM_PULSE_A_NEG|PWM_EN));
        #endif
        
        #if (CHIP == VSN_V2)||(CHIP==VSN_V3)
        //MMPF_PWM_Initialize();
        #endif        
    } 
    
	gbChangeH264Res = 0 ;
	#if DUMP_REG_EN
	dump_gbl_dram_reg();
	dump_vif_reg();
	dump_scaler_reg();
	dump_ibc_reg();
	dump_jpeg_reg();
	#endif
	RTNA_DBG_Level(dbg_l);
	//LED_Driver_SetBrightnessCode(usStoredBrightStep*LM36922_BRIGHT_PERSTEP);
	//LED_Driver_SetBrightnessStep(usStoredBrightStep);

	//dump_vif_reg();
	//dump_scaler_reg();
}



void usb_uvc_stop_preview(MMP_UBYTE ep_id,MMP_BOOL change_res)
{
extern MMP_USHORT  gsZoomRatio[2], gsLastZoomRatio[2],gsZoomRatioM[2];
extern MMP_ULONG           glGRAPreviewFormat;
extern GRA_STREAMING_CFG gGRAStreamingCfg;
extern MMP_BYTE gbGRAEn[];
#if USB_FRAMEBASE_H264_DUAL_STREAM
    extern MMP_UBYTE gbSignalType;
#endif	

    STREAM_SESSION *ss ;
    volatile MMP_UBYTE *REG_BASE_B = (volatile MMP_UBYTE *) (0x80000000);
    MMP_BOOL IsPreviewEnable;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    int         workss    ;
    MMP_UBYTE   dbg_l = RTNA_DBG_Level(3);
    
    VideoStoreVideoProperty();
    dbg_l = RTNA_DBG_Level(0);
    
    //SaveVideoPro();
    gbChangeH264Res = change_res ;
    if(ep_id==0) {
        usb_vs_timer_close(MMPF_TIMER_1);
    }
    if(ep_id < VIDEO_EP_END()) {
        MMP_UBYTE prio_err,old_prio,new_prio;
       // dbg_level = 3;
        MMP_UBYTE   pipe0_cfg;
        MMP_UBYTE   pipe1_cfg;
        MMP_UBYTE   pipe2_cfg;
        STREAM_CFG *stream_cfg =  usb_get_cur_image_pipe_by_epid(ep_id);
        PCAM_USB_VIDEO_FORMAT ep_fmt ;
        MMP_UBYTE   grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;
        
        #if 1
        check_task_stack_overflow();
        #endif
        workss = MMPF_Video_GetWorkingStreamSession();
        
        ss = MMPF_Video_GetStreamSessionByEp(ep_id);
        ep_fmt = GetFmtIndexByEp(ep_id);
        
        #if OMRON_FDTC_SUPPORT
        if( usb_uvc_is_fdtc_on() && IS_FIRST_STREAM(workss) ) {
            STREAM_CFG *stream_fdtc = usb_get_cur_fdtc_pipe();
            MMPF_IBC_SetInterruptEnable(usb_uvc_get_preview_fctl_link(2)->ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
            MMPF_Display_StopPreview(2); //sean@2011_06_8
            
            pIBC->IBC_P2_INT_CPU_EN &= ~(IBC_INT_FRM_RDY);
            // sean@2011_05_10, check if IBC Store is on or not
            if (pIBC->IBCP_2.IBC_BUF_CFG & IBC_STORE_EN)
            {
                while(!(pIBC->IBC_P2_INT_CPU_SR & IBC_INT_FRM_RDY)){
                ;//dbg_printf(3,"YUY2.pipe0:Wait Last Frame\r\n");
                }
            }
            //pIBC->IBCP_2.IBC_BUF_CFG &= ~(IBC_STORE_EN);
            MMPF_IBC_SetStoreEnable( PIPE_2,MMP_FALSE);
            RTNA_DBG_Str(3,"<Fmt : FDTC Stop.NV12@pipe2>\r\n");  


        }
        #endif
        
        pipe0_cfg = CUR_PIPE_CFG(PIPE_0,stream_cfg->pipe_cfg);//( stream_cfg->pipe_cfg   ) & PIPE_CFG_MASK ;
        pipe1_cfg = CUR_PIPE_CFG(PIPE_1,stream_cfg->pipe_cfg);//( stream_cfg->pipe_cfg>>4) & PIPE_CFG_MASK;
        pipe2_cfg = CUR_PIPE_CFG(PIPE_2,stream_cfg->pipe_cfg);
        //RTNA_DBG_Str(0, "SYS_FLAG_USB_STOP_PREVIEW\r\n");
        // dbg_printf(0,"Stop preview,pipe_en:%x,epid:%d\r\n",stream_cfg->pipe_en,ep_id);
        // Enable change priority 
        prio_err = MMPF_OS_ChangePrio(TASK_AUDIO_RECORD_PRIO,TASK_1ST_PRIO);
        
       // dbg_printf(3,"#Change Prio Err:%x,old prio:%d,new prio:%d\r\n",prio_err,old_prio,new_prio);
        /* if(gbStopPreviewEvent == 1) */ 
        {
        //gbvcstart = 0;
        //ss->tx_flag &= ~SS_TX_STREAMING ;
        //ss->pipe_id = 0xFF; 
        //SetH264Mode(0);//disable H264 mode	
        
        
        //ISP_IF_3A_Control(ISP_3A_DISABLE);
        if(( PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN ) /*||(UVCX_IsSimulcastH264())*/){
        // Enable IBC Frame END
            MMPF_IBC_SetInterruptEnable(usb_uvc_get_preview_fctl_link(0)->ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);

            MMPF_IBC_SetInterruptEnable(usb_uvc_get_preview_fctl_link(0)->ibcpipeID, MMPF_IBC_EVENT_FRM_ST, MMP_FALSE);

            if(pipe0_cfg==PIPE_CFG_MJPEG) {
                pJPG->JPG_INT_CPU_EN &= ~JPG_INT_ENC_DONE;
                MMPF_Display_StopPreview(0); //sean@2011_06_8
                RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_JPG);
                if((pGBL->GBL_CLK_DIS0 & GBL_CLK_JPG_DIS) == 0x0) {
                    pJPG->JPG_INT_CPU_SR = JPG_INT_ENC_DONE;
                }
                // sean@2013_03_20, disable scaler when stop
                MMPF_Scaler_SetEnable(usb_uvc_get_preview_fctl_link(0)->scalerpath, MMP_FALSE);
                    
                //gbdrawflag=0; 
                //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                RTNA_DBG_Str(3,"<Fmt : Stop.MJPEG@pipe0>\r\n");    
                dbg_printf(0,"<Fmt : Stop.MJPEG@pipe0>\r\n");
            }
            else if ((pipe0_cfg==PIPE_CFG_YUY2)/*||(pipe0_cfg==PIPE_CFG_NV12)*/) {
            
                if(ep_fmt == PCAM_USB_VIDEO_FORMAT_RAW) {
                    MMP_BOOL tmp = MMP_FALSE ;
                    ISP_IF_3A_Control(ISP_3A_DISABLE);
                    //sean@2011_01_19, fixed bug which can not back to normal mode
                    USB_EnableBayerRawPreview(0);
                    MMPF_Sensor_SetParam(0,&tmp);
                    //sean@2011_01_19, fixed bug which can not back to normal mode
                    MMPF_RAWPROC_EnableRawPath(MMP_FALSE);
                    //gbdrawflag=0;
                    //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                    RTNA_DBG_Str(3,"<Fmt : Stop.Raw>\r\n");
                } else {
                    
                    //if( !(grapath )  || ((grapath&0x02) && (pipe0_cfg==PIPE_CFG_YUY2) )) {
                    if(1) {
                        if(1/*UVCX_IsSimulcastH264()==0*/) { // TBD
                            MMPF_Display_StopPreview(0); //sean@2011_06_8
                            
                            pIBC->IBC_P0_INT_CPU_EN &= ~(IBC_INT_FRM_RDY);
                            // sean@2011_05_10, check if IBC Store is on or not
                            if (pIBC->IBCP_0.IBC_BUF_CFG & IBC_STORE_EN)
                            {
                                while(!(pIBC->IBC_P0_INT_CPU_SR & IBC_INT_FRM_RDY)){
                                ;//dbg_printf(3,"YUY2.pipe0:Wait Last Frame\r\n");
                                }
                            }
                            //pIBC->IBCP_0.IBC_BUF_CFG &= ~(IBC_STORE_EN);
                            MMPF_IBC_SetStoreEnable( PIPE_0,MMP_FALSE);
                        }  else {
                        }
                        //gbdrawflag=0;
                        //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                        RTNA_DBG_Str(3,"<Fmt : Stop.YUY2@pipe0>\r\n");
                    }  
                }
                // sean@2013_03, disable scaler when stop
                MMPF_Scaler_SetEnable(usb_uvc_get_preview_fctl_link(0)->scalerpath, MMP_FALSE);
                
            }
            else if(pipe0_cfg==PIPE_CFG_H264) {
            // H264
                 //RTNA_DBG_Str(3,"<Fmt : Stop.H264@pipe0>\r\n");
                 dbg_printf(0,"<Fmt : Stop.H264@pipe0>\r\n");
               // MMPF_Sensor_CheckFrameStart(2);  // wait 2 frame for H264 encode end
                USB_VideoPreviewStop(PCAM_API);
                //gbdrawflag=0;
                //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
               
            }
            
            // Close GRA loop
            if ( PIPEEN(GRA_DST_PIPE_1)==PIPE0_EN ) {
                OS_CPU_SR   cpu_sr;
                dbg_printf(0,"close gra loop0\r\n");
                IRQ_LOCK(
                    gbGRAEn[0] = MMP_FALSE ;
                    gsZoomRatio[0] = gsLastZoomRatio[0] = ZOOM_RATIO_BASE;
                    MMPF_Scaler_ResetPathInfo(MMPF_SCALER_PATH_0);
                )
            }
            
            if( (UVC_VCD()==bcdVCD15)&&(UVCX_IsSimulcastH264() )) {
                glGRAPreviewFormat = 0xFF ;
                if(!IS_FIRST_STREAM(workss)) {
                    gGRAStreamingCfg.frm_buf_format[GRA_STREAMING2] = 0xFF ;
                    gGRAStreamingCfg.streaming_num = MMPF_VIDENC_GetTotalPipeNum();    
                }
            }
            
        }
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN ) {
            MMPF_IBC_SetInterruptEnable(usb_uvc_get_preview_fctl_link(1)->ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
            if(pipe1_cfg==PIPE_CFG_MJPEG) {
                pJPG->JPG_INT_CPU_EN &= ~JPG_INT_ENC_DONE;
                MMPF_Display_StopPreview(1); //sean@2011_06_8
                RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_JPG);
                // Blink 6.4 test
                if((pGBL->GBL_CLK_DIS0 & GBL_CLK_JPG_DIS) == 0x0) {
                    pJPG->JPG_INT_CPU_SR = JPG_INT_ENC_DONE;
                }
                // sean@2013_03_20, disable scaler when stop
                MMPF_Scaler_SetEnable(usb_uvc_get_preview_fctl_link(1)->scalerpath, MMP_FALSE);
                
                //gbdrawflag=0; 
                //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                RTNA_DBG_Str(3,"<Fmt : Stop.MJPEG@pipe1>\r\n");
                dbg_printf(0,"<Fmt : Stop.MJPEG@pipe1>\r\n");
            } 
            else if ((pipe1_cfg==PIPE_CFG_YUY2) /*||(pipe1_cfg==PIPE_CFG_NV12)*/) {
            
                if(ep_fmt == PCAM_USB_VIDEO_FORMAT_RAW) {
                    MMP_BOOL tmp = MMP_FALSE ;
                    //sean@2011_01_19, fixed bug which can not back to normal mode
                    USB_EnableBayerRawPreview(0);
                    MMPF_Sensor_SetParam(0,&tmp);
                    //sean@2011_01_19, fixed bug which can not back to normal mode
                    MMPF_RAWPROC_EnableRawPath(MMP_FALSE);
                    //gbdrawflag=0;
                    //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                    RTNA_DBG_Str(3,"<Fmt : Stop.Raw>\r\n");
                
                } else {
                    
                    //if( ( !(grapath & 0x01) )  || ((grapath&0x02) && (pipe1_cfg==PIPE_CFG_YUY2) )) {
                    if(1) {
                        MMPF_Display_StopPreview(1); //sean@2011_06_8
                        pIBC->IBC_P1_INT_CPU_EN &= ~(IBC_INT_FRM_RDY);
                        // sean@2011_05_10, check if IBC Store is on or not
                        if (pIBC->IBCP_1.IBC_BUF_CFG & IBC_STORE_EN)
                        {
                            while(!(pIBC->IBC_P1_INT_CPU_SR & IBC_INT_FRM_RDY)){
                                ;//dbg_printf(3,"YUY2.pipe1:Wait Last Frame\r\n");
                            }
                        }
                        //pIBC->IBCP_1.IBC_BUF_CFG &= ~(IBC_STORE_EN);   
                        MMPF_IBC_SetStoreEnable( PIPE_1,MMP_FALSE);  
                        //gbdrawflag=0;
                        //ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                        RTNA_DBG_Str(3,"<Fmt : Stop.YUY2@pipe1>\r\n");
                    }
                } 
                // sean@2013_03_20, disable scaler when stop
                MMPF_Scaler_SetEnable(usb_uvc_get_preview_fctl_link(1)->scalerpath, MMP_FALSE);
                 
            }
            else {
                if(UVC_VCD()==bcdVCD10) {
                //    SetH264Mode(0);
                }
            // H264
                 RTNA_DBG_Str(3,"<Fmt : Stop.H264@pipe1>\r\n");
                 dbg_printf(0,"<Fmt : Stop.H264@pipe1>\r\n");
                // MMPF_Sensor_CheckFrameStart(2);  // wait 2 frame for H264 encode end
                USB_VideoPreviewStop(PCAM_API);
                
                // reset reak-time H264 for profile switch
                if(1){
                    AITPS_GBL pGBL = AITC_BASE_GBL;
                    //reset h264
                    pGBL->GBL_RST_REG_EN &= ~GBL_REG_H264_RST;
                    pGBL->GBL_RST_CTL01 |= (GBL_H264_RST);
                    RTNA_WAIT_CYCLE(100);
                    pGBL->GBL_RST_CTL01 &= ~(GBL_H264_RST);
                    
                    #if H264_SIMULCAST_EN
                    // reset for H264 rt / fr switch
                    if( gbChangeH264BufMode ) {
                        MMP_BOOL ibc_pipe[MMPF_IBC_PIPE_MAX] ;
                        MMPF_IBC_Reset(ibc_pipe);
                        MMPF_IBC_Restore(ibc_pipe);
                        if(UVC_VCD()==bcdVCD15) {
                        //    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                            
                        }
                    }
                    #endif
                }
                
            }
            
                        // Close GRA loop
            if ( PIPEEN(GRA_DST_PIPE_2)==PIPE1_EN ) {
                OS_CPU_SR   cpu_sr;
                dbg_printf(0,"close gra loop1\r\n");
                IRQ_LOCK(
                    gbGRAEn[1] = MMP_FALSE ;
                    gsZoomRatio[1] = gsLastZoomRatio[1] = ZOOM_RATIO_BASE;
                    MMPF_Scaler_ResetPathInfo(MMPF_SCALER_PATH_1);
                )
                #if H264_SIMULCAST_EN
                if( (UVC_VCD()==bcdVCD15) && (UVCX_IsSimulcastH264())) {
                    MMP_BOOL is_preview ;
                    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview,0);
                    if(is_preview && !gbChangeH264BufMode) {
                    /*
                        m_bVidRecdPreviewStatus[0] = 0 ;
                        gbGRAEn[0] = MMP_FALSE ;
                        gsZoomRatio[0] = gsLastZoomRatio[0] = ZOOM_RATIO_BASE;
                        MMPF_Scaler_ResetPathInfo(MMPF_SCALER_PATH_0);
                     */    
                        USB_RestartPreview(0);
                    }   
                }
                #endif
            }
            // Disable icon
            pcam_Osd_ConfigCurrentStream(0,0,0);
             // Reset simulcast every preview.
            if(UVC_VCD()==bcdVCD15) {
                if( (!gbChangeH264BufMode) && ( (ep_id==1)&&(!gbChangeH264Res) ) ) {
                    UVCX_SetSimulcastH264(0) ;
                }
            }
            else {
                if(!gbChangeH264BufMode) {
                    UVCX_SetSimulcastH264(0) ;
                }
            }
        }

        // Stop GRA IBC source
        if( IS_FIRST_STREAM(workss)/* || gbChangeH264BufMode */) {
            if(( PIPE_EN(stream_cfg->pipe_en) & PIPE2_EN )){
                PIPE_PATH pipe_id = usb_get_cur_image_pipe_id(stream_cfg,PIPE_CFG_NV12) ;
                dbg_printf(3,"<Fmt : Stop NV12-GRA@pipe%d,en:%x,cfg:%x\r\n",pipe_id,stream_cfg->pipe_en,stream_cfg->pipe_cfg);
                MMPF_IBC_SetInterruptEnable(usb_uvc_get_preview_fctl_link(pipe_id)->ibcpipeID, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
                MMPF_Display_StopPreview(pipe_id); //sean@2011_06_8
                MMPF_IBC_SetStoreEnable( pipe_id,MMP_FALSE);
                // sean@2013_03_20, disable scaler when stop
                MMPF_Scaler_SetEnable(pipe_id, MMP_FALSE);
                
                
                if(!gbChangeH264BufMode) {
                    if( ((m_bVidRecdPreviewStatus[0]==MMP_FALSE) && (m_bVidRecdPreviewStatus[1]==MMP_FALSE))) {
                        dbg_printf(3,"#disable vif/isp/ibc\r\n");
                // Disable VIF/ISP/IBC in stop preview.
                        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIF);
                        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ISP);
                        RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_IBC);   
                    }
                }
             }
        }
        else if(gbChangeH264BufMode) {
            MMPF_IBC_SetStoreEnable( PIPE_2,MMP_FALSE);
        }

        #if SUPPORT_PCSYNC
        if(!pcsync_is_preview())
        {
        #endif
            MMPF_Video_CloseStreamSession(ep_id) ;
        #if SUPPORT_PCSYNC
        }
        #endif
        m_bVidRecdPreviewStatus[ep_id] = 0;
        
        // blink test
        //if(workss < VIDEO_EP_END()) {
        if( IS_FIRST_STREAM(workss) ) {
            #if OMRON_FDTC_SUPPORT
            usb_uvc_close_fdtc();
            #endif
           // m_bVidRecdPreviewStatus[0] = 0;
            USB_VideoPowerDown(PCAM_API /*PCAM_BLOCKING*/,SEN_POWER_OFF);
        } else {
        // Try to back to 30 fps in uvc1.5 in case 1080p H264 with yuy2
            MMPF_Display_SetGRAFps(MMP_FALSE,0) ;
        }
        prio_err = MMPF_OS_ChangePrio(TASK_1ST_PRIO,TASK_AUDIO_RECORD_PRIO);
        // for notchfilter to know format change
        m_VideoFmt = MMPS_3GPRECD_VIDEO_FORMAT_OTHERS ;

        }
        
    }   
	gbChangeH264Res =  0;
#if USB_FRAMEBASE_H264_DUAL_STREAM
    if(!gbChangeH264BufMode) {
        gbSignalType = (MMP_UBYTE)-1;
    }
#endif	
    RTNA_DBG_Level(dbg_l);
    ////VideoStoreVideoProperty();
    dbg_printf(0,"<END>\r\n\r\n\r\n");
}

#if USB_SUSPEND_TEST
void usb_uvc_suspend(void)
{
    AITPS_AIC   	pAIC = AITC_BASE_AIC;
    STREAM_SESSION *ss;
    /*
    ss =  MMPF_Video_GetStreamSessionByEp(0); 
    ss->tx_flag &=~SS_TX_STREAMING ;
    ss =  MMPF_Video_GetStreamSessionByEp(1); 
    ss->tx_flag &=~SS_TX_STREAMING ;
    */
	//gbvcstart = 0;
//	UsbDmaStop(PCCAM_TX_EP_ADDR); // comment out for blink test
// sean@2011_03_30, don't power off sensor in here for preview->suspend->wait up 
//    USB_VideoPowerDown(PCAM_API,SEN_POWER_OFF) ;
#if SUPPORT_UAC
    USB_AudioPreviewStop(PCAM_API/*PCAM_NONBLOCKING*/); // 2010_08_02 change to API call
#endif
#if 1
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_HOST);
RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_GPIO);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_DMA);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_IBC);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIF);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ISP);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ME);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIDEO);
//RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_I2S);
#endif


}
#endif

//
// Return captured size. (glPCCamJpegSize)
//
MMP_ULONG usb_vc_take_raw_picture(MMP_UBYTE raw_mode, MMP_ULONG srcAddr,MMP_ULONG dstAddr)
{
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;
    MMP_ULONG size ,raw_w,raw_h;
    RTNA_DBG_Str3("take raw pict...\r\n");
    USB_TakeRawPicture(PCAM_BLOCKING,srcAddr + 32) ;
    #if(CHIP==P_V2)
    raw_w = (pVIF->VIF_GRAB_PIXL_ED - pVIF->VIF_GRAB_PIXL_ST +1 );
    raw_h = (pVIF->VIF_GRAB_LINE_ED - pVIF->VIF_GRAB_LINE_ST +1 );
    #endif
    #if(CHIP==VSN_V2)||(CHIP==VSN_V3)
    raw_w = (pVIF->VIF_GRAB[vid].PIXL_ED - pVIF->VIF_GRAB[vid].PIXL_ST +1 );
    raw_h = (pVIF->VIF_GRAB[vid].LINE_ED - pVIF->VIF_GRAB[vid].LINE_ST +1 );
    #endif
    
                        
    size = (raw_mode==0) ? ( (raw_w) * (raw_h) * 4 / 3 ): (raw_w) * (raw_h);
                    
    FB_BASE_B[srcAddr + 28] = ( MMP_UBYTE ) ( size >> 24 ) & 0xFF;
    FB_BASE_B[srcAddr + 29] = ( MMP_UBYTE ) ( size >> 16 ) & 0xFF;
    FB_BASE_B[srcAddr + 30] = ( MMP_UBYTE ) ( size >> 8  ) & 0xFF;
    FB_BASE_B[srcAddr + 31] = ( MMP_UBYTE ) ( size & 0xFF);
    
    RTNA_DBG_Str3("Raw Data Src Addr:");RTNA_DBG_Long3(srcAddr);RTNA_DBG_Str3("\r\n");
    RTNA_DBG_Str3("Raw Data Dst Addr:");RTNA_DBG_Long3(dstAddr);RTNA_DBG_Str3("\r\n");
    RTNA_DBG_Str3("Raw Data Size    :");RTNA_DBG_Long3(size);RTNA_DBG_Str3("\r\n");
    RTNA_DBG_Str3("Raw Data W    :");RTNA_DBG_Long3(raw_w);RTNA_DBG_Str3("\r\n");
    RTNA_DBG_Str3("Raw Data H   :");RTNA_DBG_Long3(raw_h);RTNA_DBG_Str3("\r\n");
   // MMPF_USB_ReleaseDm is updated               
   // if(MMPF_DMA_MoveData0(srcAddr + 28, (dstAddr), (size + 4), MMPF_USB_ReleaseDm)) {
   //     RTNA_DBG_Str(0, "MMPF_DMA_MoveData Fail\r\n");
   //     return 0 ;
   // }
    RTNA_DBG_Str3("take raw pict...end\r\n");
    return size ;
}

#if ENABLE_1152_648_MODE==1
static void usb_uvc_switch_scaler_pllsrc(void)
{
// VSN_V3 : TBD
}
#endif

MMPF_FCTL_LINK *usb_uvc_get_preview_fctl_link (MMP_UBYTE ubPipe)
{
    return &(gUvcPreviewFctlCfg[ubPipe]);
}

void usb_uvc_init_preview_fctl_link(void)
{
    MMP_USHORT  i ;
    AITPS_IBC   pIBC    = AITC_BASE_IBC ;
    AITPS_ICOB  pICOB    = AITC_BASE_ICOB;
	volatile AITPS_IBCP pIbcPipeCtl;

    MMP_UBYTE h264_pipe_id ;
    MMP_UBYTE jpeg_pipe_id ;
    
    h264_pipe_id =(gsH264PureStream_Cfg.pipe_en & PIPE_EN_MASK) ;
    jpeg_pipe_id =(gsMJPEGStream_Cfg.pipe_en    & PIPE_EN_MASK) ;
    
    h264_pipe_id &= ~PIPEEN(GRA_SRC_PIPE) ;
    h264_pipe_id >>= 1 ;
    
    jpeg_pipe_id &= ~PIPEEN(GRA_SRC_PIPE) ;
    jpeg_pipe_id >>= 1 ;
    
    dbg_printf(3,">> JPEG->IBC%d\r\n",jpeg_pipe_id);
    dbg_printf(3,">> H264->IBC%d\r\n",h264_pipe_id);
    MMPF_IBC_EncoderIBCSrc(jpeg_pipe_id, MMPF_IBC_FX_JPG);
    MMPF_IBC_EncoderIBCSrc(h264_pipe_id, MMPF_IBC_FX_H264);

	
	// config IBC SRC->ICON
	pIbcPipeCtl = &(pIBC->IBCP_0);
    pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_SEL_MASK;
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(0);	
 	pIbcPipeCtl = &(pIBC->IBCP_1);
    pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_SEL_MASK;
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(1);	
 	pIbcPipeCtl = &(pIBC->IBCP_2);
    pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_SEL_MASK;
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(2);	
    
    // config ICON src->SCALER
    pICOB->ICO_DLINE_CFG[0] |= ICO_DLINE_SRC_SEL(0);
    pICOB->ICO_DLINE_CFG[1] |= ICO_DLINE_SRC_SEL(1);
    pICOB->ICO_DLINE_CFG[2] |= ICO_DLINE_SRC_SEL(2);
    
   
}

extern MMP_USHORT gsPreviewPath ;// sean@2011_06_08 // |= (1 << usIBCPipe);
extern MMP_BOOL m_bReceiveStopPreviewSig[] ;
//extern H264_FORMAT_TYPE gbCurH264Type; 
static MMP_BOOL usb_uvc_config_stream(MMP_UBYTE ep_id)
{
    STREAM_SESSION *ss ;
    MMP_BOOL mjpeg_stream = 0 ;
    STREAM_CFG *stream_cfg = 0;
    PCAM_USB_VIDEO_FORMAT vidFmt ;
    MMP_UBYTE pipe0_cfg; 
    MMP_UBYTE pipe1_cfg;
    MMP_UBYTE free_pipe_id ;
    PCAM_USB_VIDEO_RES res;
    
#if ENABLE_1152_648_MODE==1
    usb_uvc_switch_scaler_pllsrc();         
#endif  
#if OMRON_FDTC_SUPPORT  
    usb_uvc_init_fdtc(1);           
#endif   
#if 0 // Move out of here
    if (USB_IsBayerRawPreview()) {
        STREAM_SESSION *ss;
        RTNA_DBG_Str0("#Bayer capture\r\n");
        pcam_get_info()->pCamVideoFormat = PCAM_USB_VIDEO_FORMAT_RAW;
        ss = MMPF_Video_GetStreamSessionByEp(ep_id);
        ss->curvsi.vsi_fmtindx = PCAM_USB_VIDEO_FORMAT_RAW;
    }
#endif    
    
    vidFmt = GetFmtIndexByEp(ep_id);
    
    
    //vidFmt =gsVidFmt;// pcam_get_info()->pCamVideoFormat ;
    if(vidFmt != PCAM_USB_VIDEO_FORMAT_H264) {
        if(UVC_VCD()==bcdVCD10) {
            if(gbCurH264Type!=UVC_YUY2) {
                gbCurH264Type = INVALID_H264 ;
            }
        }
    }
    if(vidFmt == PCAM_USB_VIDEO_FORMAT_MJPEG) {
        mjpeg_stream = 1 ;
        stream_cfg =&gsMJPEGStream_Cfg ; 
        if(gbCurH264Type==UVC_YUY2) {
            stream_cfg = &gsMJPEGYUY2Stream_Cfg;
        }   
        dbg_printf(0,"mjpeg pipe:%x, pipe_en : %x,pipe_cfg : %x\r\n",stream_cfg,stream_cfg->pipe_en,stream_cfg->pipe_cfg);
    }
    else if(vidFmt == PCAM_USB_VIDEO_FORMAT_YUV422){
        mjpeg_stream = 0;
        stream_cfg = &gsYUY2Stream_Cfg ;
        dbg_printf(0,"YUV422 pipe:%x, pipe_en : %x,pipe_cfg : %x\r\n",stream_cfg,stream_cfg->pipe_en,stream_cfg->pipe_cfg);
    }
    else if(vidFmt == PCAM_USB_VIDEO_FORMAT_YUV420){
        mjpeg_stream = 0;
        stream_cfg = &gsNV12Stream_Cfg ;
    }else if(vidFmt == PCAM_USB_VIDEO_FORMAT_RAW){
        mjpeg_stream = 0;
        stream_cfg = &gsRawStream_Cfg ;
    }
    else {  // H264 mode
        switch(gbCurH264Type) {
        #if USB_FRAMEBASE_H264_DUAL_STREAM==1
        case FRAMEBASE_H264_YUY2:
            mjpeg_stream = 0 ;
            stream_cfg = &gsH264FrameYUY2Stream_Cfg ;
            stream_cfg->pipe_en = VIDEPEN(STREAM_EP_H264) | (stream_cfg->pipe_en & PIPE_EN_MASK ) ;
            dbg_printf(0,"  <Framebase H264+YUY2>\r\n");
            break;
        case FRAMEBASE_H264_MJPEG:
            mjpeg_stream = 1 ;
            stream_cfg = &gsH264FrameMJPEGStream_Cfg ;
            stream_cfg->pipe_en = VIDEPEN(STREAM_EP_H264) | (stream_cfg->pipe_en & PIPE_EN_MASK ) ;
            dbg_printf(0,"  <Framebase H264+MJPEG>\r\n");
            break;
        #endif
        case FRAMEBASE_H264:
            mjpeg_stream = 0 ;
            stream_cfg = &gsH264PureStream_Cfg ;
            stream_cfg->pipe_en = VIDEPEN(STREAM_EP_H264) | (stream_cfg->pipe_en & PIPE_EN_MASK ) ;
            dbg_printf(0,"  <Framebase H264>\r\n");
            break;
        case SKYPE_H264:
            break;
        case UVC_H264:               
    #if USB_UVC_H264
            RTNA_DBG_Str3("[!]UVC H264 is defined\r\n");
// sean@2011_01_22, streaming type select
            switch(gbCurLocalStreamType) {
            case ST_MJPEG:
                mjpeg_stream = 1 ;
                stream_cfg = &gsH264MJPEGStream_Cfg ;
                break;
            case ST_YUY2:
                mjpeg_stream = 0;
                stream_cfg = &gsH264YUY2Stream_Cfg ;
                break;
            case ST_UNDEF:
                mjpeg_stream = 0 ;
                stream_cfg = &gsH264PureStream_Cfg; 
                
            }
    #else
            while(1) {
                RTNA_DBG_Str0("[!]UVC H264 is not defined\r\n");
            }
    #endif
            break ;
            
        }
        
        if(gbSimulcastH264) {
            stream_cfg->pipe_en  |= PIPE_EN_SIMULCAST;
        }    
        else {
            stream_cfg->pipe_en &= ~PIPE_EN_SIMULCAST;
        }
        
    }  
    #if SUPPORT_GRA_ZOOM
    if(UVC_VCD()==bcdVCD15) {
        MMPF_Video_SetLoop2GRA(stream_cfg,MMP_TRUE);    
    }
    else {
        MMPF_Video_SetLoop2GRA(stream_cfg,MMP_FALSE/*MMP_TRUE*/); // UVC 1.1 enable now  
    }
    #endif  
    #if SUPPORT_PCSYNC
    if(pcsync_is_preview() ) {
        MMPF_Video_SetLoop2GRA( stream_cfg,MMP_FALSE);    
    }
    #endif
 //   usb_set_cur_image_pipe(stream_cfg);
    
    MMPF_Video_AllocStreamPipe(ep_id,stream_cfg);

 // sean@2011_04_13, config width & height into stream cfg here.
    pipe0_cfg = CUR_PIPE_CFG(PIPE_0,stream_cfg->pipe_cfg);// stream_cfg->pipe_cfg & PIPE_CFG_MASK ;
    pipe1_cfg = CUR_PIPE_CFG(PIPE_1,stream_cfg->pipe_cfg);//(stream_cfg->pipe_cfg>>4) & PIPE_CFG_MASK ;
// sean@2011_06_08
    // TBD
    //m_bReceiveStopPreviewSig[ep_id] = MMP_FALSE;

    res = GetResIndexByEp(ep_id);

    // Single stream @ pipe 0    
    if( IS_PIPE0_EN(stream_cfg->pipe_en)  ) {
        dbg_printf(0,"Single stream @ pipe 0\r\n");
        m_bReceiveStopPreviewSig[0] = MMP_FALSE;
        usb_uvc_config_pipe(stream_cfg,res,PCCAM_RES_NUM,PCCAM_RES_NUM);
        gsPreviewPath |= (1 << 0) ;
        SetScalerRefInOutHeight( PIPE_0,stream_cfg->pipe_w[PIPE_0],stream_cfg->pipe_h[PIPE_0]);
        
    }
    // Single stream @ pipe 1
    else if( IS_PIPE1_EN(stream_cfg->pipe_en) ) {
        dbg_printf(0,"Single stream @ pipe 1\r\n");
        m_bReceiveStopPreviewSig[1] = MMP_FALSE;
        if(pipe1_cfg==PIPE_CFG_H264) {
            PCAM_USB_INFO *pcamInfo = pcam_get_info(); 
            usb_uvc_config_pipe(stream_cfg,PCCAM_RES_NUM,pcamInfo->pCamVideoRes,PCCAM_RES_NUM);    
        } else {
             usb_uvc_config_pipe(stream_cfg,PCCAM_RES_NUM,res,PCCAM_RES_NUM);
             gsPreviewPath |= (1 << 1) ;
        }
        SetScalerRefInOutHeight( PIPE_1,stream_cfg->pipe_w[PIPE_1],stream_cfg->pipe_h[PIPE_1]);
    }
    else if( IS_PIPE2_EN(stream_cfg->pipe_en) ) {
        dbg_printf(0,"Single stream @ pipe 2\r\n");
        m_bReceiveStopPreviewSig[2] = MMP_FALSE;
        usb_uvc_config_pipe(stream_cfg,PCCAM_RES_NUM,PCCAM_RES_NUM,res);
        gsPreviewPath |= (1 << 2) ;
        SetScalerRefInOutHeight( PIPE_2,stream_cfg->pipe_w[PIPE_2],stream_cfg->pipe_h[PIPE_2]);
    }
    else {
    	PCAM_USB_VIDEO_RES res_pipe2 = PCCAM_RES_NUM;
        m_bReceiveStopPreviewSig[0] = MMP_FALSE;
        m_bReceiveStopPreviewSig[1] = MMP_FALSE;
    #if 1//SUPPORT_GRA_ZOOM 
	    if(1/*UVC_VCD()==bcdVCD15*/) {
	        res_pipe2 = PCCAM_1920_1080;//PCCAM_1280_720;
	        m_bReceiveStopPreviewSig[2] = MMP_FALSE;
	    }    
    #endif    
        if(pipe0_cfg==PIPE_CFG_H264) {
            PCAM_USB_INFO *pcamInfo = pcam_get_info(); 
            usb_uvc_config_pipe(stream_cfg,pcamInfo->pCamVideoRes,res,res_pipe2);      
        } 
        else if(pipe1_cfg==PIPE_CFG_H264) {
            PCAM_USB_INFO *pcamInfo = pcam_get_info(); 
            usb_uvc_config_pipe(stream_cfg,res,pcamInfo->pCamVideoRes,res_pipe2);
        } 
        else {
            // pipe 1 is jpeg use main resolution variable
            // pipe 0 is yuy2 , now is same res with mjpeg
            dbg_printf(0,"pipe0: yuy2 & pipe1: jpeg\r\n");
            usb_uvc_config_pipe(stream_cfg,res,res,res_pipe2);  
        }
        if(PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN) {
            gsPreviewPath |= (1 << 0) ;
            SetScalerRefInOutHeight( PIPE_0,stream_cfg->pipe_w[PIPE_0],stream_cfg->pipe_h[PIPE_0]);
        }
        if(PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN) {
            gsPreviewPath |= (1 << 1) ;
            SetScalerRefInOutHeight( PIPE_1,stream_cfg->pipe_w[PIPE_1],stream_cfg->pipe_h[PIPE_1]);
        }
        if(PIPE_EN(stream_cfg->pipe_en) & PIPE2_EN) {
            gsPreviewPath |= (1 << 2) ;
            SetScalerRefInOutHeight( PIPE_2,stream_cfg->pipe_w[PIPE_2],stream_cfg->pipe_h[PIPE_2]);
        }
    }
    
    
    #if OMRON_FDTC_SUPPORT
    stream_cfg = usb_get_cur_fdtc_pipe(); ;
    if ( IS_PIPE2_EN(stream_cfg->pipe_en) ) {
        m_bReceiveStopPreviewSig[2] = MMP_FALSE;
        usb_uvc_config_pipe(stream_cfg,PCCAM_RES_NUM,PCCAM_RES_NUM,PCCAM_640_360); 
        gsPreviewPath |= (1 << 2) ; 
        dbg_printf(3,"#fdtc pipe is on (640,360)\r\n");
    }
    #endif
    

    #if SUPPORT_GRA_ZOOM
    if(1/*UVC_VCD()==bcdVCD15*/) {
        MMP_BOOL pipe1_en = MMP_FALSE ;
        if(PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN) {
            if( (stream_cfg->pipe_w[PIPE_1] > LOSSY_ZOOM_MAX_W) &&(stream_cfg->pipe_h[PIPE_1] > LOSSY_ZOOM_MAX_H ) ) {
                dbg_printf(3,"#Too Big-Res...\r\n");
                MMPF_Video_SetLoop2GRA(stream_cfg,MMP_FALSE); 
            
            }
            pipe1_en = MMP_TRUE ;
        } 
        
        if(PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN) {
        
            if( (stream_cfg->pipe_w[PIPE_0] > LOSSY_ZOOM_MAX_W) &&(stream_cfg->pipe_h[PIPE_0] > LOSSY_ZOOM_MAX_H ) ) {
                dbg_printf(3,"#Too Big-Res...\r\n");
                MMPF_Video_SetLoop2GRA(stream_cfg,MMP_FALSE); 
            }
            
            if(pipe1_en==MMP_FALSE) {
                if( ( stream_cfg->pipe_w[PIPE_0]  * stream_cfg->pipe_h[PIPE_0] ) == 352 * 288 ) {
                    dbg_printf(3,"--Disable Lossy for Lync CIF\r\n");
                    MMPF_Video_SetLoop2GRA(stream_cfg,MMP_FALSE); 
                }
            }
        }    
       
          
    } 
    #endif
// for different uvc header length in uvc15
    usb_uvc_config_header_length(ep_id) ;
        
    dbg_printf(3,"<usb_uvc_config_stream:%x\r\n",gsPreviewPath);
    return mjpeg_stream ;
}

static void usb_uvc_config_header_length(MMP_UBYTE ep_id)
{
    UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] = sizeof(UVC_VIDEO_PAYLOAD_HEADER) ;
    #if H264_SIMULCAST_EN
    if( (UVC_VCD()==bcdVCD15) && (ep_id==STREAM_EP_H264) && ( UVCX_IsSimulcastH264() )) {
    } 
    else {
        UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id]= 12 ;
    }
    dbg_printf(0,"--ep_id :%d,uvc headerlen : %d\r\n",ep_id, UVC_VIDEO_PAYLOAD_HEADER_LEN[ep_id] );
    
    #endif
}

static void usb_uvc_config_pipe(STREAM_CFG *stream_cfg,PCAM_USB_VIDEO_RES pipe0_res,PCAM_USB_VIDEO_RES pipe1_res,PCAM_USB_VIDEO_RES pipe2_res)
{
    PCAM_USB_INFO *pcamInfo = pcam_get_info();
    RES_TYPE_CFG *cur_res ;
    PCAM_USB_VIDEO_RES pipe_res[MMPF_IBC_PIPE_MAX] ;
    MMP_UBYTE cur_pipe; 
    
    pipe_res[0] = pipe0_res ;
    pipe_res[1] = pipe1_res ;
    pipe_res[2] = pipe2_res ;
    
    for(cur_pipe=0;cur_pipe < MMPF_IBC_PIPE_MAX ; cur_pipe++) {
        if(PIPE_EN(stream_cfg->pipe_en) & (1<<cur_pipe)) {
            cur_res = GetResCfg(pipe_res[cur_pipe]);
            if(cur_res) {
                if(cur_res) {
                    stream_cfg->pipe_w[cur_pipe] = cur_res->res_w ;
                    stream_cfg->pipe_h[cur_pipe] = cur_res->res_h ;
                } else {
                    stream_cfg->pipe_w[cur_pipe] = 0 ;
                    stream_cfg->pipe_h[cur_pipe] = 0 ;
                }
                
                dbg_printf(0,"stream_cfg(%x)->pipe_w[%d] :%d\r\n",stream_cfg,cur_pipe,stream_cfg->pipe_w[cur_pipe]);
                dbg_printf(0,"stream_cfg(%x)->pipe_h[%d] :%d\r\n",stream_cfg,cur_pipe,stream_cfg->pipe_h[cur_pipe]);
                
            }    
        }
    }

       
    //dbg_printf(3,"stream_cfg->pipe_en :%x\r\n",stream_cfg->pipe_en);
    //dbg_printf(3,"stream_cfg->pipe_cfg:%x\r\n",stream_cfg->pipe_cfg);
}

static void usb_uvc_init_stream(MMP_UBYTE ep_id,MMP_BOOL mjpeg_stream)
{
    STREAM_CFG *stream_cfg ;
    PCAM_USB_VIDEO_FORMAT vidFmt = GetFmtIndexByEp(ep_id);// gsVidFmt;//pcam_get_info()->pCamVideoFormat;
    PCAM_USB_VIDEO_RES    res = GetResIndexByEp(ep_id);
    
    stream_cfg = usb_get_cur_image_pipe_by_epid(ep_id);
    if(stream_cfg) {
        int workss ;
        stream_cfg->pipe_sync = 0;
        workss = MMPF_Video_GetWorkingStreamSession();
#if OMRON_FDTC_SUPPORT
        if( IS_FIRST_STREAM(workss) ) {
            usb_uvc_start_stream(0xFD,0,usb_get_cur_fdtc_pipe(),PCCAM_640_360);
        }
#endif       
        //
        // to prevent scaler 0 or 1 with double frame start
        // 
        if(!IS_FIRST_STREAM(workss) && (vidFmt==PCAM_USB_VIDEO_FORMAT_H264)) {
            MMPF_Display_SkipGraLoop(MMP_TRUE);
        }

        usb_uvc_start_stream(ep_id,mjpeg_stream,stream_cfg,res);
        if(vidFmt==PCAM_USB_VIDEO_FORMAT_H264) {
            MMP_BOOL bComplete;
            //RTNA_DBG_Str(0, "MMPS_3GPRECD_StartRecord begin...\r\n");
       #if H264_MTS_SUPPORT==1    
            if (MMPF_VIDMGR_GetMtsMuxModeEnable()) { //in mts mode
                MMPF_VIDMGR_StartMts(0);
            }
       #endif
            #if 1 
            // Wait Zoom & Pan finished, then start record   
            do {
                MMPF_Scaler_CheckZoomComplete(0, &bComplete);
                MMPF_OS_Sleep(1);
            } while (!bComplete);
            do {
                MMPF_Scaler_CheckPanTiltComplete(0, &bComplete);
                MMPF_OS_Sleep(1);
            } while (!bComplete);
            #endif
            dbg_printf(0,"#--H264 Start--#\r\n");
            
            #if LYNC_CERT_PATCH
            if( (UVC_VCD()==bcdVCD15) && UVCX_IsSimulcastH264() ) {
                MMPF_VIDENC_RegisterStartInstanceCallBack(MMP_TRUE);
            }           
            else {
                MMPF_VIDENC_RegisterStartInstanceCallBack(MMP_FALSE);
            } 
                      
            #else
            MMPF_VIDENC_RegisterStartInstanceCallBack(MMP_FALSE);
            #endif
            #if LYNC_CERT_TEST_EN
            //MMPF_VIDENC_RegisterStartInstanceCallBack(MMP_TRUE);
            #endif
            MMPS_3GPRECD_StartRecord();                
            //RTNA_DBG_Str(0, "MMPS_3GPRECD_StartRecord end...\r\n");
            
        }
        
        if(!IS_FIRST_STREAM(workss) && (vidFmt==PCAM_USB_VIDEO_FORMAT_H264) ) {
            MMPF_Display_SkipGraLoop(MMP_FALSE);
        }
    }
}        

static void usb_uvc_start_stream(MMP_UBYTE ep_id,MMP_BOOL mjpeg_stream,STREAM_CFG *stream_cfg,MMP_ULONG res)
{
extern MMP_UBYTE gbAEPriority ;
extern MMP_BOOL gbConverTest30FPS ;

static MMP_BOOL gbIsNv12IBCSet = MMP_FALSE ;

    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    MMP_ERR     err = MMP_ERR_NONE ;
    
    MMP_UBYTE   pipe0_cfg;
    MMP_UBYTE   pipe1_cfg;
    MMP_UBYTE   pipe2_cfg;
    
    MMP_UBYTE   pipe_cfg ;
    
    PCAM_USB_VIDEO_FORMAT ep_fmt ;
    STREAM_SESSION *ss = 0 ;
    int         workss ;
    
    MMPF_ICO_PIPEATTRIBUTE  IconAttr;
    MMPF_IBC_PIPEATTRIBUTE  IbcAttr;
    MMPF_FCTL_LINK          *pPreviewFctl;
    
    
    workss = MMPF_Video_GetWorkingStreamSession();
    if(ep_id < VIDEO_EP_END() ) {
        ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    } 
    //pipe0_cfg = ( stream_cfg->pipe_cfg) & PIPE_CFG_MASK ;
    //pipe1_cfg = (stream_cfg->pipe_cfg>>4) & PIPE_CFG_MASK;
    pipe0_cfg = CUR_PIPE_CFG(PIPE_0,stream_cfg->pipe_cfg);
    pipe1_cfg = CUR_PIPE_CFG(PIPE_1,stream_cfg->pipe_cfg);
    pipe2_cfg = CUR_PIPE_CFG(PIPE_2,stream_cfg->pipe_cfg);
    
    dbg_printf(3,"ssep_id:%d,jpegs:%d,pipe_en:%x\r\n",ep_id,mjpeg_stream,stream_cfg->pipe_en);
    dbg_printf(3,"pipe_cfg(%x,%x,%x),stream_cfg:%x\r\n",pipe0_cfg,pipe1_cfg,pipe2_cfg,stream_cfg->pipe_cfg);
    
    if(!mjpeg_stream) { // YUY2 + H264
       // RTNA_DBG_Str(0,"InitScale() begin...\r\n");
        InitScale(stream_cfg,res,0/*UVCX_IsSimulcastH264()*/);
        RTNA_DBG_Str(3,"->InitScale()\r\n"); // 0->3   
    } else {            // MJPEG+ H264
        MMPF_InitPCCam(stream_cfg,res,0/*UVCX_IsSimulcastH264()*/);
        RTNA_DBG_Str(3,"->MMPF_InitPCCam()\r\n");// 0->3    
    }   
    #if 0//SUPPORT_GRA_ZOOM // TEST ONLY 
    if(MMPF_Video_IsLoop2GRA(stream_cfg) && (pipe1_cfg==PIPE_CFG_YUY2)) {
        if(res==PCCAM_1920_1080) {
            gsCurFrameRatex10[ep_id] = 50;
        }
        else if(res==PCCAM_1280_720) {
            gsCurFrameRatex10[ep_id] = 100;
        }
    }
    #endif
    //gbCurFrameRate[ep_id]  = 30 ;
    //gsCurFrameRatex10[ep_id] = 200; // 800x600 24 fps test
    // Keep the current FPS first
    // This line is to keep the frame rate in static variable.
    ss->tx_flag &= ~SS_TX_SYNC_STREAMING ;
    if(ep_id < VIDEO_EP_END() ) {
        if( (UVC_VCD()==bcdVCD10) || (ss->tx_flag & SS_TX_RT_MODE)) {
			dbg_printf(3,"FPS,3\r\n");      
            PCAM_USB_Set_FPSx10(gbAEPriority,0,gsCurFrameRatex10[ep_id]);
            //ISP_IF_AE_SetMaxFPS(30);
        } else {
            ss->tx_flag &= ~SS_TX_SYNC_STREAMING ;
            
            
        	if(ep_id==0) {
        	    if(gsCurFrameRatex10[ep_id] < 300) {
        	        ss->tx_flag |= SS_TX_SYNC_STREAMING ;    
        	    }
        	    usb_vs_initial_fps(gsCurFrameRatex10[ep_id]) ;
        	    usb_vs_timer_open(gsCurFrameRatex10[ep_id],MMPF_TIMER_1);
        	    
        	}
        	// Fixed to max fps in sensor
            dbg_printf(3,"FPS,2\r\n");          	
            PCAM_USB_Set_FPSx10(gbAEPriority,0,300);
            //ISP_IF_AE_SetMaxFPS(30);
        }
    } 
    #if LYNC_CERT_TEST_EN
    if(gbConverTest30FPS) {
        PCAM_USB_Set_FPSx10(gbAEPriority,0,300);
    }
    else {
        PCAM_USB_Set_FPSx10(gbAEPriority,0,150);
    }
    #endif
    
    // sean@2013_02_16, workaround test
    if(0/* !IS_FIRST_STREAM(workss) */) {
        MMP_UBYTE grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;
        if( grapath & 2) {
            MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(1)->scalerpath);
        }
        else if(grapath & 4) {
            MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(0)->scalerpath);
        }
        dbg_printf(0,"--in GRA loop, switch to ISP\r\n");
    }
    
    
    if( IS_FIRST_STREAM(workss) ) {
        gbIsNv12IBCSet = MMP_FALSE ;
    }
    
    if(1/*!mjpeg_stream*/) {
        MMP_LONG wr_ptr ;
        MMP_UBYTE cur_pipe ;
        //gbVideoCurOutputFrame = 0;
        //glVideoValidFrameCount = 0;
        glPCCamCnt0 = glPCCamCnt1 = 0;
        for(cur_pipe=0;cur_pipe < MMPF_IBC_PIPE_MAX ; cur_pipe++) {
            if(PIPE_EN(stream_cfg->pipe_en) & (1<<cur_pipe)) {
                pipe_cfg = CUR_PIPE_CFG(cur_pipe,stream_cfg->pipe_cfg) ; 
                if( (pipe_cfg == PIPE_CFG_NV12) && ( /*!IS_FIRST_STREAM(workss) */gbIsNv12IBCSet) ) {
                    dbg_printf(3,"IBC2:NV12 configed\r\n");
                    if(UVCX_IsSimulcastH264()) {
                        if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                            MMPF_IBC_SetInterruptEnable(cur_pipe, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);
                        }
                        else {
                            MMPF_IBC_SetInterruptEnable(cur_pipe, MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);
                        
                        }
                    }
                    // 
                    else {
                    // UVC1.5, local + simulcate h264 -> stop simulcast h264, restart local preview.
                        if(UVC_VCD()==bcdVCD15) {
                            MMPF_IBC_SetInterruptEnable(cur_pipe, MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);       
                        }
                    }
                }
                else {
                    if(  pipe_cfg != PIPE_CFG_H264 ) {
                        pPreviewFctl = usb_uvc_get_preview_fctl_link(cur_pipe);
                        IconAttr.inputsel = pPreviewFctl->scalerpath;
                        IconAttr.bDlineEn = MMP_TRUE;
                        MMPF_ICON_SetAttributes (pPreviewFctl->icopipeID, &IconAttr);
                        wr_ptr = (MMP_ULONG)MMPF_Video_CurWrPtr(cur_pipe);
                        switch (pipe_cfg) {
                        case PIPE_CFG_YUY2:
                        case PIPE_CFG_NV12:
                            IbcAttr.function = MMPF_IBC_FX_TOFB;
                            IbcAttr.ulBaseAddr = wr_ptr + FRAME_PAYLOAD_HEADER_SZ;
                            if (pipe_cfg ==PIPE_CFG_NV12) {
                                IbcAttr.ulBaseUAddr = IbcAttr.ulBaseAddr  + stream_cfg->pipe_w[pipe_cfg]*stream_cfg->pipe_h[pipe_cfg];
                                IbcAttr.ulBaseVAddr = IbcAttr.ulBaseUAddr + stream_cfg->pipe_w[pipe_cfg]*stream_cfg->pipe_h[pipe_cfg]/4;
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
                            DBG_S(0, "#Error : Unknow config for PIPE0\r\n");
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
                            if(UVCX_IsSimulcastH264()&&(pipe_cfg==PIPE_CFG_NV12)) {
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
                            if((ss->tx_flag & SS_TX_RT_MODE)&&(pipe_cfg==PIPE_CFG_YUY2)) {
                                MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_ST, MMP_TRUE);
                            }
                            #if YUY2_848_480_30FPS==FIRE_AT_FRAME_START
                            if((ss->tx_flag & SS_TX_BY_ONE_SHOT)&&(pipe_cfg==PIPE_CFG_YUY2)) {
                                MMPF_IBC_SetInterruptEnable(pPreviewFctl->ibcpipeID, MMPF_IBC_EVENT_FRM_ST, MMP_TRUE);
                            }
                            #endif
                        }
                        
                        
                        if(pipe_cfg==PIPE_CFG_NV12) {
                            gbIsNv12IBCSet = MMP_TRUE ;
                        }
                    }
                }
            }
        }                    
    } 
    if(ep_id >= VIDEO_EP_END() ) {
        return ;
    }
    if(mjpeg_stream) {
        glPCCamCnt0 = glPCCamCnt1 = 0;
    #if 1//(ENABLE_JPEG_ISR)
        RTNA_AIC_Open(pAIC, AIC_SRC_JPG, jpg_isr_a,
                        AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
        RTNA_AIC_IRQ_En(pAIC, AIC_SRC_JPG);
        
        pJPG->JPG_INT_CPU_SR = JPG_INT_ENC_DONE ;
        pJPG->JPG_INT_CPU_EN = JPG_INT_ENC_DONE ;
    #endif
    }
    if( IS_FIRST_STREAM(workss) ) {
        // SEAN : HARD CODING
        // Hard - codeing....
        //IBC1 -> H264
        //IBC0 -> JPEG
        if(1/*UVC_VCD()==bcdVCD15*/) {// both mode inited
            usb_uvc_init_preview_fctl_link();
        }
        MMPF_Sensor_3A_SetInterrupt(MMP_TRUE);
        // Enable clock, TBD
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_GRA, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_DMA, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_IBC, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_SCAL, MMP_TRUE);
        MMPD_System_EnableClock(MMPD_SYSTEM_CLK_MODULE_ICON, MMP_TRUE);

        RTNA_AIC_IRQ_En(pAIC, AIC_SRC_IBC); 
	    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_ISP);    
        RTNA_AIC_IRQ_En(pAIC, AIC_SRC_VIF);
    // switch to gra path before VIF output is on    
    #if  INIT_SCALE_PATH_EN==0 
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(0)->scalerpath);
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(1)->scalerpath);
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(2)->scalerpath);
    #endif
    }
    
    ep_fmt = GetFmtIndexByEp(ep_id);
    
    
    ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    ss->tx_flag |= SS_TX_CONFIG_END ;
    dbg_printf(3,"[ep:%d]start stream : ss->tx_flag : %x\r\n",ep_id,ss->tx_flag);
    if(ep_fmt==PCAM_USB_VIDEO_FORMAT_H264) {
        PIPE_PATH pipe_id;
        STREAM_CFG *cur_stream;
        
        cur_stream = usb_get_cur_image_pipe_by_format(ep_id,PIPE_CFG_H264,&pipe_id);
        if(cur_stream) {
          	dbg_printf(3, "H264 VIF en\r\n");
            if (MMPF_Fctl_EnablePreview((MMPF_IBC_PIPEID)pipe_id, MMP_TRUE, MMP_FALSE)) {
          	    dbg_printf(3, " video preview failed ................\r\n");
                return ;
            }
            
            if(cur_stream->pipe_en & PIPE_EN_SIMULCAST) {
                if (MMPF_Fctl_EnablePreview((MMPF_IBC_PIPEID)MMPF_IBC_PIPE_0, MMP_TRUE, MMP_FALSE)) {
              	    dbg_printf(3, " video preview failed ................\r\n");
                    return ;
                }
                #if SUPPORT_OSD_FUNC
                pcam_Osd_ConfigCurrentStream(MMP_TRUE,cur_stream->pipe_w[PIPE_1],cur_stream->pipe_h[PIPE_1]);
                #endif
                if(pipe0_cfg==PIPE_CFG_MJPEG) {
                    dbg_printf(3,"pipe0-jpg-off-ready-ibc\r\n");
                    MMPF_IBC_SetInterruptEnable(MMPF_IBC_PIPE_0, MMPF_IBC_EVENT_FRM_RDY, MMP_FALSE);
                }
            }
        } else {
            dbg_printf(3,"Unknown H264 pipe\r\n");
        }
        //MMPF_Scaler_SetForceSwitchPath(GRA_DST_PIPE_2) ;
        
    }
    else {
        //m_bVidRecdPreviewStatus[ep_id] = 1;
        if( IS_FIRST_STREAM(workss) ) {
            MMPF_VIF_EnableInputInterface(MMP_TRUE);
        }
        dbg_printf(3, "Non H264 VIF en\r\n");
        
        //MMPF_Scaler_SetForceSwitchPath(GRA_DST_PIPE_1) ;
    }
    m_bVidRecdPreviewStatus[ep_id] = 1;
    
    //if(workss < VIDEO_EP_END()) {
    if( IS_FIRST_STREAM(workss) ) {
        MMP_USHORT skipframe = 3 ;
        #if SUPPORT_PCSYNC
        if(pcsync_is_preview()) {
            skipframe = 0 ;
        }
        #endif
    	MMPF_Display_SkipFrame(skipframe);
    	#if SUPPORT_PCSYNC
    	if(!pcsync_is_preview())
    	{
    	#endif
            usb_vc_init_sensor_param(ep_id);
            ISP_IF_AE_SetFastMode(1); 
            //RTNA_DBG_Str3("Enable AE Fast Mode\r\n");   
        #if SUPPORT_PCSYNC
        }         
        #endif
        #if SUPPORT_AUTO_FOCUS
        if(gbStillCaptureEvent == STILL_IMAGE_WAITING){
            USB_EnableAF(PCAM_API,gbAutoFocus) ;
        }
        #endif
    }
    else {
        if(UVC_VCD()==bcdVCD15) {
        // sean@2013_11_20 [BEGIN] , patch for ss->tx_flag[7] is clean by unknown reason
            MMPF_SCALER_SOURCE target_scaler_path = MMPF_SCALER_SOURCE_ISP ;
             if( UVCX_IsSimulcastH264()) {
                if(UVCX_GetH264EncodeBufferMode()!=FRAME_MODE) {
                    target_scaler_path = MMPF_SCALER_SOURCE_GRA ;
                }
            }
            do {        
                while( (ss->tx_flag & SS_TX_CONFIG_END ) ) {
                    MMPF_OS_Sleep(1);
                    dbg_printf(3,"#C,");
                }
                if( target_scaler_path == MMPF_Scaler_GetPath( usb_uvc_get_preview_fctl_link(PIPE_1)->scalerpath ) ) {
                    break;
                }
                else {
                    ss->tx_flag |= SS_TX_CONFIG_END ;
                }
            } while ( 1 );
        // sean@2013_11_20 [END] , patch for ss->tx_flag[7] is clean by unknown reason
            usb_vc_init_ct_param();
        }
        else {
        #if H264_SIMULCAST_EN
            while( (ss->tx_flag & SS_TX_CONFIG_END ) ) {
                MMPF_OS_Sleep(1);
                dbg_printf(3,"#C,");
            }
            usb_vc_init_ct_param();
        
        #endif
        }
    }
}

static void usb_vc_init_ct_param(void)
{
    MMP_BOOL bComplete;
#if CT_CS_VAL & (1 << CT_ZOOM_ABSOLUTE_CONTROL) 
    dbg_printf(3,"#2nd-zoom :%d\r\n",gsZoom);
    USB_SetDigitalZoom(PCAM_API,2,ZOM_MIN,gsZoom,ZOM_MAX);
#endif 
    // Wait zoom finished to get grab in parameter then pan / tilt
    do {
        MMPF_Scaler_CheckZoomComplete(0, &bComplete);
        MMPF_OS_Sleep(1);
    } while (!bComplete);
// Not Ready Temp marked    
#if CT_CS_VAL & (1 << CT_PANTILT_ABSOLUTE_CONTROL) 
    USB_SetDigitalPanTilt(PCAM_API,PAN_MIN,glPan,TILT_MIN,glTilt,(PAN_MAX-PAN_MIN)/PANTILT_RES );
#endif 

}

static void usb_vc_init_sensor_param(MMP_UBYTE ep_id)
{
    // sean@201_08_24, change to api call for suspend/wakeup preview test
#if CT_CS_VAL & (1 << CT_ZOOM_ABSOLUTE_CONTROL) 
    dbg_printf(3,"#init-zoom :%d\r\n",gsZoom);
    USB_SetDigitalZoom(PCAM_API,2,ZOM_MIN,gsZoom,ZOM_MAX);
#endif 
// Not Ready Temp marked  
#if CT_CS_VAL & (1 << CT_PANTILT_ABSOLUTE_CONTROL) 
    USB_SetDigitalPanTilt(PCAM_API,PAN_MIN,glPan,TILT_MIN,glTilt,(PAN_MAX-PAN_MIN)/PANTILT_RES );
#endif 


    //dbg_printf(3,"init_param,awb:%d\r\n",gbAWB);
#if PU_CS_VAL & (1 << PU_WHITE_BALANCE_TEMPERATURE_AUTO_CONTROL )     
    if(gbAWB==AWB_MIN) { // disable
        // Set WB tempature
        USB_SetAWBTemp(PCAM_API,gsTemper);
        // Set Manual WB
        USB_SetAWBMode(PCAM_API,gbAWB );
    }
#endif  
    if(gbAE==AEMODE_MANUAL) {
#if CT_CS_VAL & (1 << CT_AE_MODE_CONTROL)         
        USB_EnableAE(PCAM_API,MMP_FALSE) ;
        USB_SetExposureTime(PCAM_API,glEV);
#endif
#if PU_CS_VAL & (1 << PU_GAIN_CONTROL)
        USB_SetGain(PCAM_API,gsGain);
#endif
    }
#if CT_CS_VAL & (1 << CT_FOCUS_AUTO_CONTROL)
    if(gbAutoFocus==AUTOAF_MIN) {
        USB_EnableAF(PCAM_API, gbAutoFocus);
        USB_SetLensPosition(PCAM_API,gsAFocus);  
    }
#endif
#if PU_CS_VAL & (1 << PU_POWER_LINE_FREQUENCY_CONTROL)     
    USB_SetPowerLineFreq(PCAM_API, gbBandFilter);
#endif
#if PU_CS_VAL & (1 << PU_SATURATION_CONTROL) 
    USB_SetSaturation(PCAM_API,gsSaturation);
#endif
#if PU_CS_VAL & (1 << PU_CONTRAST_CONTROL)      
    USB_SetContrast(PCAM_API,gsContrast);
#endif    
#if PU_CS_VAL & (1 << PU_BRIGHTNESS_CONTROL)
    USB_SetBrightness(PCAM_API,gsBrightness);
#endif    
#if PU_CS_VAL & (1 << PU_BACKLIGHT_COMPENSATION_CONTROL)
    USB_SetBacklight(PCAM_API,gsBacklight);
#endif
#if PU_CS_VAL & (1 << PU_SHARPNESS_CONTROL)         
    USB_SetSharpness(PCAM_API,gsSharpness);
#endif   
#if PU_CS_VAL & (1 << PU_GAMMA_CONTROL)         
    USB_SetGamma(PCAM_API,gsGamma);
#endif
#if PU_CS_VAL & (1 << PU_HUE_CONTROL)         
    USB_SetHue(PCAM_API,gsHue);
#endif
    // Patch for Loic VGA edge roughness issue
    //USB_SetEdgeByResolution(glPccamResolution);
}


static void usb_uvc_set_h264_mode(MMP_UBYTE FormatIndex,MMP_UBYTE FrameIndex)
{

    if(UVC_VCD()==bcdVCD15) {
        return ;
    }
    
 	if((gbCurH264Type==UVC_H264)||(gbCurH264Type==UVC_YUY2)||(gbCurH264Type==FRAMEBASE_H264_YUY2)||(gbCurH264Type==FRAMEBASE_H264_MJPEG)) {
    
        PCAM_USB_VIDEO_RES h264Res ;
        
		//glPccamResolution = FrameIndex -1;   // descriptor frame index start from 1

        h264Res = FrameIndex -1;//glPccamResolution ;
	     
        if(gbCurH264Type==UVC_H264) {
#if USB_UVC_H264

            // sean@2011_01_22, limit the local preview resolution
            if(gbCurLocalStreamType==ST_MJPEG) {
                if(h264Res==PCCAM_1920_1080 ) {
                   // glPccamResolution =  PCCAM_640_360;
                    RTNA_DBG_Str0("JPEG= PCCAM_640_360\r\n");
                    gCurVsiInfo[0].bFrameIndex = PCCAM_640_360 + 1 ; 
                } 
                else if (h264Res==PCCAM_1280_720) {
                    //glPccamResolution =PCCAM_640_360 ;
                    RTNA_DBG_Str0("JPEG= PCCAM_640_360\r\n");
                    gCurVsiInfo[0].bFrameIndex = PCCAM_640_360 + 1;
                }
            } else {
            //glPccamResolution = PCCAM_1280_720 ;
                if( (h264Res==PCCAM_1920_1080) || (h264Res==PCCAM_1280_720) ) {
                   // glPccamResolution = PCCAM_640_360;
                    gCurVsiInfo[0].bFrameIndex = PCCAM_640_360 + 1 ;
                    RTNA_DBG_Str0("YUY2= PCCAM_640_360\r\n"); 
                }
            }
#endif	     
	     } 
	     else if(gbCurH264Type==FRAMEBASE_H264_YUY2) {
            gCurVsiInfo[0].bFrameIndex = PCCAM_640_360 + 1 ;
            RTNA_DBG_Str0("YUY2= PCCAM_640_360\r\n"); 
         }   
	     else if(gbCurH264Type==FRAMEBASE_H264_MJPEG) {
	        if(gbSimulcastH264) {
                gCurVsiInfo[0].bFrameIndex = PCCAM_1280_720 + 1 ;//FrameIndex;//PCCAM_640_360 + 1 ;
            }
            else {
                gCurVsiInfo[0].bFrameIndex = PCCAM_1280_720 + 1 ;//FrameIndex;//PCCAM_640_360 + 1 ;
            }
           // RTNA_DBG_Str0("MJPEG= PCCAM_640_360\r\n"); 
	     }
	     dbg_printf(3,"#CurH264Type :%d,H264 Res:%d,Local Res:%d\r\n",gbCurH264Type,h264Res,gCurVsiInfo[0].bFrameIndex);
         if(gbCurH264Type!= UVC_YUY2) {
            USB_SetH264Resolution(PCAM_API,h264Res);	
         }
    }
}

