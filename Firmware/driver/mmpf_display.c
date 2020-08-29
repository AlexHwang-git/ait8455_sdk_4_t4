//==============================================================================
//
//  File        : mmpf_display.c
//  Description : Firmware Display Control Function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"

#include "mmp_reg_display.h"
#include "mmp_reg_video.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_vif.h"
#include "mmp_reg_gpio.h"
#include "mmp_reg_graphics.h"
//#include "mmp_reg_ccir.h"
#include "mmp_reg_h264enc.h"
#include "mmp_reg_jpeg.h"

#include "mmpf_hif.h"
#include "mmpf_display.h"
#include "mmpf_scaler.h"
#include "mmpf_vif.h"
#if (VIDEO_R_EN)
#include "mmpf_mp4venc.h"
#endif
#if (SENSOR_EN)
#include "mmpf_sensor.h"
#include "mmpf_rawproc.h"
#endif
#if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW))
#include "mmpf_dma.h"
#endif

#if defined(ALL_FW)||(SENSOR_EN)
//extern	void VR_3A_SetHardwareBuffer(MMP_ULONG ulStartAddr);
//#include "3A_ctl.h"
#include "isp_if.h"
#endif

#if defined(ALL_FW)
#include "mmps_3gprecd.h"
#include "mmpf_usbpccam.h"
#include "mmpf_usbvend.h"
#include "mmpf_usb_h264.h"
#include "mmpf_usbuvc.h"
#include "mmpf_h264enc.h"
#endif
#include "mmp_reg_dma.h"
#include "mmpf_usbuvc_event.h"
#if H264_SIMULCAST_EN
#include "mmpf_usbuvch264.h"
#endif
#include "pCam_osd_api.h"

#define __OSD_STR   "123"
//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
#if (CHIP == P_V2)
#define	FLM_GPIO_NUM	8  //TBD
#endif
MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4];
MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];
MMP_UBYTE   gbPreviewBufferCount[MMPF_IBC_PIPE_MAX];

MMP_USHORT  gsPreviewBufWidth[MMPF_IBC_PIPE_MAX][4];
MMP_USHORT  gsPreviewBufHeight[MMPF_IBC_PIPE_MAX][4];
MMP_USHORT  gsPreviewBufCurWidth[MMPF_IBC_PIPE_MAX];
MMP_USHORT  gsPreviewBufCurHeight[MMPF_IBC_PIPE_MAX];

#if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW))
MMP_ULONG   glRotateBufAddr[MMPF_IBC_PIPE_MAX];             // dst Y buffer of rotate DMA
MMP_ULONG   glRotateUBufAddr[MMPF_IBC_PIPE_MAX];            // dst U buffer of rotate DMA
MMP_ULONG   glRotateVBufAddr[MMPF_IBC_PIPE_MAX];            // dst V buffer of rotate DMA
MMP_UBYTE   gbRotateBufferCount;            // dst buffer count of rotate DMA
//static MMP_USHORT  gsIBCtoDMAPipe = 0;      // keep which pipe links IBC with rotate DMA
MMP_UBYTE   gbRotateDoneBufIdx = 0;         // keep which buffer has the rotated frame
MMP_UBYTE   gbRotateCurBufIdx = 0;          // keep which buffer is to be a dst buffer in turn
extern MMPF_OS_FLAGID SYS_Flag_Hif;
#endif

MMP_UBYTE   gbExposureDone;
MMP_UBYTE	gbExposureDoneFrame[MMPF_IBC_PIPE_MAX];
MMP_USHORT	gsPreviewPath;
MMP_ULONG   glVideoValidFrameCount;
MMP_UBYTE   gbCurIBCBuf[MMPF_IBC_PIPE_MAX];
MMP_UBYTE                   gbIBCLinkEncId[MMPF_IBC_PIPE_MAX];
MMP_UBYTE                   gbIBCLinkEncFromGraList[MMPF_IBC_PIPE_MAX][4];
MMP_UBYTE                   gbIBCLinkEncFromGraNum[MMPF_IBC_PIPE_MAX];
IbcLinkCallBackFunc         *IBCLinkCallbackList[MMPF_IBC_PIPE_MAX][4];
MMP_UBYTE                   gbIBCLinkCallbackNum[MMPF_IBC_PIPE_MAX];
MMPF_IBC_LINK_TYPE			gIBCLinkType[MMPF_IBC_PIPE_MAX];
MMPF_DISPLAY_PREVIEW_MODE	gPreviewMode[MMPF_IBC_PIPE_MAX];
MMPF_DISPLAY_WINID			gPreviewWinID[MMPF_IBC_PIPE_MAX];

MMP_BOOL		m_bReceiveStopPreviewSig[MMPF_IBC_PIPE_MAX];
MMPF_OS_SEMID	m_PreviewControlSem;
/*MMP_BOOL*/MMP_UBYTE	m_bStartPreviewFrameEndSig;
MMP_BOOL		m_bWaitVIFEndSig = MMP_FALSE;
MMPF_OS_SEMID 	m_StartPreviewFrameEndSem;



#if (SENSOR_EN)
extern	MMPF_SENSOR_FUNCTION *gsSensorFunction;
#endif

extern  MMP_ULONG   glSystemEvent;

//extern MMP_ULONG   glPccamResolution;
//extern void MMPF_USB_ReleaseDm(MMP_UBYTE ep_id);
//extern volatile MMP_UBYTE usb_fb_toggle;
extern MMP_ULONG   glPCCamCompressBufAddr;
extern MMP_ULONG glPCCamCnt0; // sperate for MJPEG + YUY2 
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
//extern MMP_UBYTE gbStillCaptureEvent;
//extern MMP_ULONG glPCCAM_VIDEO_BUF_ADDR;
extern MMP_ULONG glPCCAM_VIDEO_BUF_MAX_SIZE;
extern H264_FORMAT_TYPE	gbCurH264Type;

extern IbcCallBackFunc *CallBackFuncIbc[MMPF_IBC_EVENT_MAX][MMPF_IBC_PIPE_MAX];

extern MMP_USHORT      gsVidRecdStatus;                    // tomy@2011_02_25


#if SUPPORT_GRA_ZOOM
extern  MMP_USHORT  gsSensorLCModeWidth, gsSensorLCModeHeight;
extern  MMP_USHORT	gsZoomRatio[2],gsLastZoomRatio[2],gsZoomRatioM[2];
extern	MMPF_SCALER_SOURCE	gScalSrc;
MMP_BOOL	gbGRAEn[2];

MMP_USHORT gsGraDstZoomRatioM[2] = {ZOOM_RATIO_BASE,ZOOM_RATIO_BASE};
MMP_USHORT gsGraDstZoomRatioN[2] = {ZOOM_RATIO_BASE,ZOOM_RATIO_BASE};

#endif

#if H264_SIMULCAST_EN
GRA_STREAMING_CFG    gGRAStreamingCfg;
MMP_ULONG   glGRAPreviewFormat;
MMP_ULONG   glGRAPreviewWidth;
MMP_ULONG   glGRAPreviewHeight;
MMP_ULONG   glGRASrcYAddr,glGRASrcUAddr,glGRASrcVAddr ;

void MMPF_Display_Simulcast(void);
#endif

extern volatile MMP_ULONG PTS;

MMP_BOOL gbSkipGraLoop = MMP_FALSE;


//==============================================================================
//
//                              FUNCTION PROTOTYPES
//
//==============================================================================

MMP_USHORT MMPF_Display_GetGRAFpsLimitation(STREAM_CFG *cur_pipe0,STREAM_CFG *cur_pipe1);
MMP_USHORT MMPF_Display_SetGRAMCI(void);

static void MMPF_Display_ConfigGRASrcScaler(MMP_USHORT gra_src_w,MMP_USHORT gra_src_h);
static MMP_BOOL MMPF_Display_ReConfigGRASrcScaler(MMP_USHORT new_crop_w,MMP_USHORT new_crop_h);

static MMP_USHORT MMPF_Display_ConfigGRADstScaler(MMP_UBYTE dst_pipe,STREAM_CFG *cur_pipe,MMP_USHORT usCropWidth,MMP_USHORT usCropHeight,MMP_USHORT usRatioM);
static void MMPF_Display_ConfigGRADstIbc(MMP_UBYTE usIBCPipe,MMP_UBYTE format);

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_GpioISR
//  Description :
//------------------------------------------------------------------------------

void MMPF_Display_GpioISR(void)
{
	AITPS_GPIO   pGPIO  = AITC_BASE_GPIO;
	
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_ISR
//  Description :
//------------------------------------------------------------------------------

void MMPF_Display_ISR(void)
{
	#if (defined(ALL_FW)&&(PCAM_EN==0))
    AITPS_DSPY  pDSPY    = AITC_BASE_DSPY;
	MMP_USHORT	intsrc;

	intsrc = pDSPY->DSPY_INT_CPU_EN & pDSPY->DSPY_INT_CPU_SR;
	pDSPY->DSPY_INT_CPU_SR = intsrc;
	#endif
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Display_IbcISR
//  Description : 
//------------------------------------------------------------------------------
void MMPF_Display_IbcISR(void)
{
//extern MMP_ULONG STC;
extern MMP_ULONG glFrameStartSOF ;
//extern MMP_ULONG glDiffSend ;
    MMP_ULONG   diffMs;
    
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    AITPS_VIF	pVIF	= AITC_BASE_VIF;
	MMP_UBYTE	intsrc[MMPF_IBC_PIPE_MAX] ,pipe ;
    MMP_BOOL    bVIFEnable;
    MMP_ULONG   jpegSize, buf_addr;
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    MMP_UBYTE   yuv_en = 0 ,yuv_pipe = 0xFF ;
    MMP_UBYTE   pipe0_cfg,pipe1_cfg,pipe2_cfg ;
    
    STREAM_CFG *cur_pipe_ep[MMPF_IBC_PIPE_MAX] = { 0,0,0 } ;//= usb_get_cur_image_pipe();
    STREAM_CTL *cur_ctl ;//= &cur_pipe->pipe0_b;
    
    STREAM_SESSION *ss ;
    
	intsrc[0] = pIBC->IBC_P0_INT_CPU_EN & pIBC->IBC_P0_INT_CPU_SR;
	pIBC->IBC_P0_INT_CPU_SR = intsrc[0];

	intsrc[1] = pIBC->IBC_P1_INT_CPU_EN & pIBC->IBC_P1_INT_CPU_SR;
	pIBC->IBC_P1_INT_CPU_SR = intsrc[1];
	
	intsrc[2] = pIBC->IBC_P2_INT_CPU_EN & pIBC->IBC_P2_INT_CPU_SR;
	pIBC->IBC_P2_INT_CPU_SR = intsrc[2];
	
	if(intsrc[2]) {
    //  dbg_printf(0,"ibcsrc(%x,%x,%x)\r\n",intsrc[0],intsrc[1],intsrc[2]);
    }	
	for(pipe=0;pipe < MMPF_IBC_PIPE_MAX ; pipe++) {
	    yuv_pipe = 0xFF ; // Reset yuv_pipe to invalid ( bug fix )
	    if(intsrc[pipe]) {
	        MMP_UBYTE ep;
	        ep  = usb_get_cur_image_ep(pipe);
	        ss = MMPF_Video_GetStreamSessionByEp(ep);
            if( ss->tx_flag & SS_TX_STREAMING) {
                //dbg_printf(3,"ibc.ep:%d,pipe:%d\r\n",ep,pipe);
                cur_pipe_ep[pipe] = usb_get_cur_image_pipe(pipe);
            }
            
            //dbg_printf(3,"pipe:%d,ep:%d,ss->tx_flag : %x,pipeaddr:%x\r\n",pipe,ep,ss->tx_flag ,cur_pipe_ep[pipe]);
            
	    }
	    if(intsrc[pipe] & IBC_INT_FRM_RDY) {
	        if(cur_pipe_ep[pipe]) {
	            cur_ctl = &cur_pipe_ep[pipe]->pipe_b[pipe];
        		if(m_bStartPreviewFrameEndSig==(1+pipe)) {
        			m_bStartPreviewFrameEndSig = 0;
        			MMPF_OS_ReleaseSem(m_StartPreviewFrameEndSem);
        			dbg_printf(0,"skip pipe %d\r\n",pipe);
        			return ;
        		}
                pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_pipe_ep[pipe]->pipe_cfg) ;
                pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_pipe_ep[pipe]->pipe_cfg) ;
                pipe2_cfg = CUR_PIPE_CFG(PIPE_2,cur_pipe_ep[pipe]->pipe_cfg) ;
	            //glDiffSend = usb_vc_cur_sof();
	            if(pipe==PIPE_0) {
    	            if ( ( PIPE_EN(cur_pipe_ep[pipe]->pipe_en) & PIPE0_EN) /*||UVCX_IsSimulcastH264() */) {
     	                if ((pipe0_cfg == PIPE_CFG_YUY2) || (pipe0_cfg == PIPE_CFG_NV12)) {
    	                    yuv_pipe = PIPE_0 ;
    	                }
    	                else if (pipe0_cfg == PIPE_CFG_H264) {
                            if(gsVidRecdStatus == MMPF_MP4VENC_FW_STATUS_START){  // tomy@2011_02_25, to fix the corrupted 1st frame in every H.264 transmission
                                MMPF_Display_FrameDoneTrigger(pipe); 
                            }
    	                } 
    	            }
 	            }
	            else if(pipe==PIPE_1) {
	                //dbg_printf(3,"pipe_en:%x,pipe_cfg:%x,gsVidRecdStatus:%d\r\n", PIPE_EN(cur_pipe_ep[pipe]->pipe_en),pipe1_cfg,gsVidRecdStatus);
    	            if ( ( PIPE_EN(cur_pipe_ep[pipe]->pipe_en) & PIPE1_EN) /*||UVCX_IsSimulcastH264()*/ ) {
     	                if ((pipe1_cfg == PIPE_CFG_YUY2) || (pipe1_cfg == PIPE_CFG_NV12)) {
    	                     yuv_pipe = PIPE_1 ;
    	                }
    	                else if (pipe1_cfg == PIPE_CFG_H264) {
                            if(gsVidRecdStatus == MMPF_MP4VENC_FW_STATUS_START){  // tomy@2011_02_25, to fix the corrupted 1st frame in every H.264 transmission
                                //dbg_printf(3,"W%d\r\n",gGRAStreamingCfg.work_streaming);
                                MMPF_Display_FrameDoneTrigger(pipe); 
                            }
    	                } 
    	            }
 	            }
	            else {
	                yuv_pipe = PIPE_2 ;
	            }
	        }    
	        
            if ( (yuv_pipe != 0xFF ) && !(ss->tx_flag & SS_TX_RT_MODE) ) {
                MMPF_NotifyDataReady(yuv_pipe);
            }
            else if( (pipe2_cfg == PIPE_CFG_NV12) ) {
            
            #if H264_SIMULCAST_EN
                if( (pipe==PIPE_1) && (pipe1_cfg==PIPE_CFG_H264)) {
                
                } else {
                    MMPF_NotifyDataReady(pipe);
                }
            #else
                MMPF_NotifyDataReady(pipe);
            #endif    
            }
            
	    }
	}
    for(pipe=0;pipe <MMPF_IBC_PIPE_MAX;pipe++) {
        if( intsrc[pipe]  & IBC_INT_FRM_END) {
           	if (m_bReceiveStopPreviewSig[pipe]) {
           	    dbg_printf(3,"IBC.FrmEnd : %d\r\n",pipe);
           		MMPF_IBC_SetInterruptEnable(pipe,MMPF_IBC_EVENT_FRM_END,MMP_FALSE);
                MMPF_IBC_SetStoreEnable(pipe,MMP_FALSE);
    			if (gsPreviewPath == 0) {
                    MMPF_VIF_IsInterfaceEnable(&bVIFEnable);
                    if(bVIFEnable){
                        m_bWaitVIFEndSig = MMP_TRUE;
                    	pVIF->VIF_INT_CPU_SR[0] |= VIF_INT_GRAB_END;
                    	pVIF->VIF_INT_CPU_EN[0] |= VIF_INT_GRAB_END;
                    }
                    else
    				    MMPF_OS_ReleaseSem(m_PreviewControlSem);
    			}			    	
    			else {
    				MMPF_OS_ReleaseSem(m_PreviewControlSem);
    			}				
    		}
        
        }
    }
	
	
	for(pipe=0;pipe < MMPF_IBC_PIPE_MAX ; pipe++) {
	    if (intsrc[pipe] & IBC_INT_FRM_ST) {
	        MMP_BOOL ibc_store_en , one_shot_at_frame_start = MMP_FALSE ;
	        MMP_UBYTE ep  = usb_get_cur_image_ep(pipe);
	        ss = MMPF_Video_GetStreamSessionByEp(ep);
            // Tx Real-Time Mode
            ibc_store_en = MMPF_IBC_GetStoreEnable(pipe);
            #if YUY2_848_480_30FPS==FIRE_AT_FRAME_START
            if(ss->tx_flag & SS_TX_BY_ONE_SHOT) {
                one_shot_at_frame_start =  MMP_TRUE ;
            }
            #endif
            
            if( (ibc_store_en ) && ( (ss->tx_flag & SS_TX_RT_MODE) || one_shot_at_frame_start) ){
            
                PIPE_PATH rt_mode_pipe = PIPE_0 ;
                
                STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(pipe);
                MMP_ULONG buf_addr;//= (MMP_ULONG )MMPF_Video_CurWrPtr(pipe);
                MMP_ULONG size ;//= cur_pipe->pipe_w[pipe]* cur_pipe->pipe_h[pipe] * 2;
                
                rt_mode_pipe = usb_get_cur_image_pipe_id(cur_pipe,PIPE_CFG_YUY2);
                
                if(rt_mode_pipe==pipe) {
                    buf_addr = (MMP_ULONG )MMPF_Video_CurWrPtr(rt_mode_pipe);
                    size = cur_pipe->pipe_w[pipe]* cur_pipe->pipe_h[pipe] * 2;
                    glPCCamCnt0++;
                    usb_uvc_fill_payload_header((MMP_UBYTE *)buf_addr,size,glPCCamCnt0,0,/*STC*/PTS,cur_pipe->pipe_w[yuv_pipe],cur_pipe->pipe_h[yuv_pipe],ST_YUY2,ISP_IF_AE_GetRealFPS() );
                   //MMPF_USB_ReleaseDm(rt_mode_pipe);
                    if(one_shot_at_frame_start) {
                        MMPF_USB_ReleaseDm(STREAM_EP_YUY2);
                    }
                    else {    
                        usb_vs_next_packet(STREAM_EP_YUY2);
                    }    
                   // dbg_printf(3,"release dm ,addr:%x,cnt:%d\r\n",buf_addr,glPCCamCnt0);
                }
                //dbg_printf(3,"release dm ,addr:%x,cnt:%d\r\n",buf_addr,glPCCamCnt0);
            }	
    	    if (CallBackFuncIbc[MMPF_IBC_EVENT_FRM_ST][pipe]) {
    	        IbcCallBackFunc *Callback = CallBackFuncIbc[MMPF_IBC_EVENT_FRM_ST][pipe];
    	        CallBackFuncIbc[MMPF_IBC_EVENT_FRM_ST][pipe] = NULL;
    	        Callback();
    	    }
	    }
	}
}

MMP_ULONG glCurGraSrcAddr ;
MMP_USHORT gsCurGraSrcWidth  ;
MMP_USHORT  gsCurGraSrcHeight  ;
MMP_BOOL gbH264Encoding = MMP_FALSE;

MMP_ERR MMPF_NotifyDataReady(MMP_UBYTE yuv_pipe)
{
extern  MMP_USHORT gsKeepCurSlotNum  ;
	extern  MMP_USHORT  gsSensorMCModeWidth, gsSensorMCModeHeight;
	extern  MMP_BYTE	gbFDTCDone, gbFDTCUpdate[2];
	extern  MMPF_SCALER_FIT_RANGE	gFitRange[2];
	extern  MMPF_SCALER_GRABCONTROL gGrabCtl[2];
    extern  MMP_USHORT gsCurFrameRatex10[] ;
    static  MMP_USHORT frame_idx,frame_fps = 60 ;

#if 1// YUY2_TX_RT_MODE
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(0);
#endif
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    AITPS_SCAL  pSCAL 	= AITC_BASE_SCAL;
    
    MMP_ULONG size=0,buf_addr  ;
    STREAM_CFG *cur_pipe, *target_pipe ;
    STREAM_CTL *cur_ctl ;
    MMP_UBYTE release = 1 ;
    MMP_UBYTE pipe_cfg;
    volatile AITPS_IBCP pIBCP ;
    static MMP_UBYTE frameCnt = 0;
    MMP_USHORT	usCropWidth, usCropHeight;    
    MMP_BOOL    do_fdtc = 0, yuy2_end = MMP_FALSE ;
    MMP_UBYTE   gra_path = 0 ;
    cur_pipe = usb_get_cur_image_pipe(yuv_pipe);
    
    #if (CHIP==VSN_V2)||(CHIP==VSN_V3)
    if(yuv_pipe==PIPE_0) {
        pIBCP = &(pIBC->IBCP_0) ;
    }
    else if (yuv_pipe==PIPE_1) {
        pIBCP = &(pIBC->IBCP_1) ;
    }
    else {
        pIBCP = &(pIBC->IBCP_2) ;
    }
    cur_ctl = &cur_pipe->pipe_b[yuv_pipe];
    
    #endif
    
    pipe_cfg =  CUR_PIPE_CFG(yuv_pipe,cur_pipe->pipe_cfg);
    
    if(UVCX_IsSimulcastH264()==0) {
       MMPF_IBC_SetStoreEnable( yuv_pipe,MMP_FALSE);
    }
    
    buf_addr = (MMP_ULONG)MMPF_Video_CurWrPtr(yuv_pipe);
    //if(yuv_pipe==GRA_SRC_PIPE) {
     //   dbg_printf(3,"pipe : %d(%x), buf_addr : 0x%x\r\n", yuv_pipe, pipe_cfg,buf_addr);
    //}
    // Change STC to PTS
    if(pipe_cfg==PIPE_CFG_YUY2) {
        glPCCamCnt0++ ;
        size = cur_pipe->pipe_w[yuv_pipe]* cur_pipe->pipe_h[yuv_pipe] * 2;
        if(cur_pipe->pipe_ph_type[yuv_pipe]==PIPE_PH_TYPE_3) {
            //size = cur_pipe->pipe_w[yuv_pipe]* cur_pipe->pipe_h[yuv_pipe] * 3 / 2;	// patrick change format to NV12
        	        
            usb_frameh264_fill_payload_header ((MMP_UBYTE *)buf_addr,size,glPCCamCnt0,0,/*STC*/PTS,cur_pipe->pipe_w[yuv_pipe],cur_pipe->pipe_h[yuv_pipe],ST_YUY2,ISP_IF_AE_GetRealFPS());
	    
        } else {
            usb_uvc_fill_payload_header((MMP_UBYTE *)buf_addr,size,glPCCamCnt0,0,/*STC*/PTS,cur_pipe->pipe_w[yuv_pipe],cur_pipe->pipe_h[yuv_pipe],ST_YUY2,ISP_IF_AE_GetRealFPS() );
        }
        #if 0//SUPPORT_GRA_ZOOM
        {
            MMPF_SCALER_SWITCH_PATH *scalinfo = MMPF_Scaler_GetPathInfo(yuv_pipe) ;
            if(scalinfo->flowctl==SCAL_WAIT_ENC_END) {
                scalinfo->flowctl=SCAL_ENC_END ;
                dbg_printf(3,"> yuy2 end\r\n");      
            }
        }
        #endif
    } else if (pipe_cfg == PIPE_CFG_NV12) {
        MMP_UBYTE grapath ;
        #if SUPPORT_GRA_ZOOM==0
    	size = cur_pipe->pipe_w[yuv_pipe] * cur_pipe->pipe_h[yuv_pipe] * 3 / 2;
        usb_uvc_fill_payload_header((MMP_UBYTE *)buf_addr,size,0/*glPCCamCnt0*/,0,/*STC*/PTS,cur_pipe->pipe_w[yuv_pipe],cur_pipe->pipe_h[yuv_pipe],ST_NV12,ISP_IF_AE_GetRealFPS() );
        #else	
        grapath = MMPF_Video_IsLoop2GRA(cur_pipe) ;
    	if( grapath ) {
    	    MMP_UBYTE pipe_cfg_dst ;
    	    //MMPF_SCALER_ZOOM_INFO *scalinfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_1);
    	    MMPF_SCALER_SWITCH_PATH *scalinfo[2] = {0,0};
    	    STREAM_CFG *cur_pipe[2] = {0,0} ;
    	    
          	cur_pipe[0] = usb_get_cur_image_pipe(GRA_DST_PIPE_1) ;
          	cur_pipe[1] = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
    	    
    	    if(grapath & 2) { // gra -> pipe 0
    	        scalinfo[0] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_1) ;
        		//if ( (grapath & 0x02 ) == 0){
            	if(scalinfo[0]->flowctl==SCAL_ENC_END) {
        			gbGRAEn[0] = MMP_FALSE;
        		//	gsZoomRatio = ZOOM_RATIO_BASE;
        			dbg_printf(3, "[NV12] :%d,gbGRAEn = %d\r\n",GRA_DST_PIPE_1,gbGRAEn[0]);
        		}
    	    }
    	    // TBD
    	    if(grapath & 4) { // gra -> pipe 1
    	        scalinfo[1] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_2) ;
    	        if(scalinfo[1]->flowctl==SCAL_ENC_END) {
    	            gbGRAEn[1] = MMP_FALSE;
    	            dbg_printf(3, "[NV12] :%d,gbGRAEn = %d\r\n",GRA_DST_PIPE_2,gbGRAEn[1]);
    	        }
    	    }
    	    
    	    // -------------------------MODIFY------------------------------------------
    	    do {
    	        MMP_USHORT crop_w[2] = {0,0},crop_h[2] = {0,0};
        		
        		if(!UVCX_IsSimulcastH264() ) {
            		if((/*grapath & 2*/gbGRAEn[0])&&(cur_pipe[0] )) {
            		    if(gsZoomRatio[0] > gsZoomRatioM[0] ) {
            		        crop_w[0] = ( ( ALIGN16(cur_pipe[0]->pipe_w[GRA_DST_PIPE_1]) * gsZoomRatioM[0] / gsZoomRatio[0] + 1 ) >> 1 ) << 1 ;
             		        crop_h[0] = ( ( ALIGN16(cur_pipe[0]->pipe_h[GRA_DST_PIPE_1]) * gsZoomRatioM[0] / gsZoomRatio[0] + 1 ) >> 1 ) << 1 ;
             		    }
            		}
        		}
        		if((/*grapath & 4*/gbGRAEn[1])&&(cur_pipe[1] )) {
        		    if(gsZoomRatio[1] > gsZoomRatioM[1] ) {
        		        crop_w[1] = ( ( ALIGN16(cur_pipe[1]->pipe_w[GRA_DST_PIPE_2]) * gsZoomRatioM[1] / gsZoomRatio[1] + 1 ) >> 1 ) << 1 ;
         		        crop_h[1] = ( ( ALIGN16(cur_pipe[1]->pipe_h[GRA_DST_PIPE_2]) * gsZoomRatioM[1] / gsZoomRatio[1] + 1 ) >> 1 ) << 1 ;
         		    }
        		}
        		usCropWidth  = VR_MAX(crop_w[0],crop_w[1]);
        		usCropHeight = VR_MAX(crop_h[0],crop_h[1]);
        		
        		
        		//pipe_cfg_dst =  CUR_PIPE_CFG(GRA_DST_PIPE_2,cur_pipe->pipe_cfg);
        		if(1/*pipe_cfg_dst==PIPE_CFG_H264*/) { // OOXX test
        		    usCropHeight = ALIGN16(usCropHeight);
        		}
        		
        		#if 0
        		dbg_printf(3,"--n/m 0(%d,%d)\r\n",  gsZoomRatio[0], gsZoomRatioM[0] );
        		dbg_printf(3,"--n/m 1(%d,%d)\r\n",  gsZoomRatio[1], gsZoomRatioM[1] );

    		    dbg_printf(3,"--en  (%d,%d) (%d,%d)-(%d,%d)\r\n",  gbGRAEn[0], gbGRAEn[1]  ,
    		                                                       cur_pipe[0]->pipe_w[GRA_DST_PIPE_1],cur_pipe[0]->pipe_h[GRA_DST_PIPE_1],
    		                                                       cur_pipe[1]->pipe_w[GRA_DST_PIPE_2],cur_pipe[1]->pipe_h[GRA_DST_PIPE_2]);
    		    
    		    dbg_printf(3,"--crop(%d,%d) (%d,%d)-(%d,%d)\r\n", usCropWidth, usCropHeight,crop_w[0],crop_h[0],crop_w[1],crop_h[1]);
    		    #endif
    		    
    		    //
    		    // If 2 stream resolutions are different.
    		    // Need to check if pipe 2 crop area is changed by zoom ratio
    		    //
    		    if(MMPF_Display_ReConfigGRASrcScaler(usCropWidth,usCropHeight)) {
    		        gsLastZoomRatio[0] = gsLastZoomRatio[1] = ZOOM_RATIO_BASE ;
    		    }
    		} while(0);
    		//---------------------------------------------------------------------------
    		//dbg_printf(3,"NV12[%d] addr:%x\r\n",yuv_pipe,buf_addr);
    		if(UVCX_IsSimulcastH264()) {
    		    MMPD_MP4VENC_FW_OP status_vid ; 
    		    MMPD_VIDENC_GetStatus(&status_vid);
    		    if(status_vid==MMPD_MP4VENC_FW_OP_START) {
    		    
    		        if(gGRAStreamingCfg.streaming_num==3) {
    		            if(gbGRAEn[1] && !gbGRAEn[0] ) {
    		                MMPF_Display_SyncH264Ratio();    
    		            }
    		        }
                    if( (MMPF_Display_ConfigGRA(yuv_pipe,buf_addr+FRAME_PAYLOAD_HEADER_SZ, usCropWidth, usCropHeight) == MMP_FALSE) ) {
                        gbH264Encoding = MMP_TRUE ;
                        glCurGraSrcAddr = buf_addr+FRAME_PAYLOAD_HEADER_SZ ;
                        gsCurGraSrcWidth = usCropWidth ;
                        gsCurGraSrcHeight = usCropHeight ;
                    
                        MMPF_VIDENC_TriggerFrameDone(0, (MMP_UBYTE*)NULL, (MMP_UBYTE*)NULL, 0,
                                                    (MMP_ULONG*)NULL, (MMP_ULONG*)NULL, (MMP_ULONG*)NULL);
                                                    
                                                  
                        MMPF_VIDENC_TriggerFrameDone(1, (MMP_UBYTE*)NULL, (MMP_UBYTE*)NULL, 0,
                                                    (MMP_ULONG*)NULL, (MMP_ULONG*)NULL, (MMP_ULONG*)NULL);
                    } 
                    else {
                        //dbg_printf(3,"skip one frame \r\n");
                    }                                                   
                                                    
                } 
                else {
                    gbH264Encoding = MMP_FALSE ;
    		        if(MMPF_Display_ConfigGRA(0, buf_addr+FRAME_PAYLOAD_HEADER_SZ, usCropWidth, usCropHeight)==MMP_FALSE) {
    		            MMPF_Display_DoGRA(0, buf_addr+FRAME_PAYLOAD_HEADER_SZ, usCropWidth, usCropHeight,gbSkipGraLoop) ;
    		        }
                }    
    		} else {
    		    
    		    MMPF_Display_TriggerGRA(yuv_pipe, buf_addr+FRAME_PAYLOAD_HEADER_SZ, usCropWidth, usCropHeight) ; // OOXX
    		}
    	}
        #endif        
        
    }
     
   // yuv_ph = (FRAME_PAYLOAD_HEADER *)buf_addr ;
    // Advance to next slot 
    //dbg_printf(3,"> FDTC:(0x%x, 0x%x)\r\n", cur_pipe->pipe_en, IS_PIPE01_EN(cur_pipe->pipe_en));
    if( IS_PIPE01_EN(cur_pipe->pipe_en)) {
        MMP_UBYTE pipe0_cfg, pipe1_cfg ;
        pipe0_cfg =  CUR_PIPE_CFG(PIPE_0,cur_pipe->pipe_cfg);  
        pipe1_cfg =  CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg);  
		if( (pipe1_cfg==PIPE_CFG_H264) || (pipe1_cfg==PIPE_CFG_MJPEG) ) {
            release = 0 ;
        }
        
    }
    // don't release dm when pipe=2
 	#if SUPPORT_GRA_ZOOM       
    if ( MMPF_Video_IsLoop2GRA(cur_pipe) ) {
        if ( yuv_pipe==GRA_SRC_PIPE) {	
    	    release = 0 ;
    	}
    }
 	#endif
 	#if YUY2_848_480_30FPS==FIRE_AT_FRAME_START
 	if (ss->tx_flag & SS_TX_BY_ONE_SHOT) {
 	    release = 0;
 	}
 	#endif
 	    
    if(release || (UVCX_IsSimulcastH264() && (pipe_cfg==PIPE_CFG_YUY2)) ) { // TBD
        if(usb_vs_sendbytimer()==0){
            MMPF_USB_ReleaseDm(yuv_pipe);
        }  
     }
    
    gra_path = MMPF_Video_IsLoop2GRA(cur_pipe) ;
    
    if( !(ss->tx_flag & SS_TX_RT_MODE)) {
        if(MMPF_Video_IsSyncStreamMode(yuv_pipe) && (pipe_cfg == PIPE_CFG_YUY2) )
        {
            MMP_USHORT a = 0,b=0;// = (frame_fps*10) / ( gsCurFrameRatex10[0] << 1) ;
            MMP_SHORT realfpsx10 = ISP_IF_AE_GetRealFPSx10();
            
            frame_idx = ( glPCCamCnt0 % frame_fps )  ;    
            //dbg_printf(3,"real fps : %d,%d,%d\r\n",realfpsx10,gsCurFrameRatex10[0],frame_idx);
            if(realfpsx10&&gsCurFrameRatex10[0]) {
                if(realfpsx10 > gsCurFrameRatex10[0]) {
                    a = (frame_fps*10) / ( (realfpsx10-gsCurFrameRatex10[0]) << 1) ;
                    b = (frame_fps*10) / ( gsCurFrameRatex10[0] << 1) ;
                    if ( gsCurFrameRatex10[0] > (realfpsx10/2) ) {
                        if( (frame_idx %  a ) ) {
                            MMPF_Video_UpdateWrPtr(yuv_pipe);
                            yuy2_end = MMP_TRUE ;
                        }
                    } 
                    else  {
                         if( (frame_idx %  b )==0 ) {
                                MMPF_Video_UpdateWrPtr(yuv_pipe);
                                yuy2_end = MMP_TRUE ;
                            }
                         }
                    }
                else {
                    MMPF_Video_UpdateWrPtr(yuv_pipe);
                    yuy2_end = MMP_TRUE ;
                }

            }
            else {
                MMPF_Video_UpdateWrPtr(yuv_pipe);
                yuy2_end = MMP_TRUE ;
            }
            
        } else {
            MMPF_Video_UpdateWrPtr(yuv_pipe);
            yuy2_end = MMP_TRUE ;
        }
    
    }
    // Fixed zoom to lossless bug
    else {
        yuy2_end = MMP_TRUE ;
    }
    if(yuy2_end) {//SUPPORT_GRA_ZOOM
        MMPF_SCALER_SWITCH_PATH *scalinfo = MMPF_Scaler_GetPathInfo(yuv_pipe) ;
        if(scalinfo->flowctl==SCAL_WAIT_ENC_END) {
            scalinfo->flowctl=SCAL_ENC_END ;
            dbg_printf(3,"> yuy2 end\r\n");      
        }
    }
    
    // Get cur slot addr 
    buf_addr = (MMP_ULONG)MMPF_Video_CurWrPtr(yuv_pipe);
    
    if(1/*! UVCX_IsSimulcastH264()*/) {
        pIBCP->IBC_ADDR_Y_ST = buf_addr + FRAME_PAYLOAD_HEADER_SZ  ;
    }
    
    //if(yuv_pipe==0) {
    //	dbg_printf(0,"ibc_%d ptr:%x\r\n",yuv_pipe, (MMP_ULONG)(buf_addr + FRAME_PAYLOAD_HEADER_SZ)) ;
    //}
#if SUPPORT_GRA_ZOOM
    if ( ( gra_path ) && (pipe_cfg==PIPE_CFG_NV12 )) {
    	//extern MMP_BYTE gbJPEGSkip;
        AITPS_ISP  pISP  = AITC_BASE_ISP;
        AITPS_JPG  pJPG = AITC_BASE_JPG;
        MMPF_SCALER_SWITCH_PATH *scalpath[2] = {0,0};
        MMP_BOOL is_preview[2] ;
        
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG);
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264 );
        
        // patrick increases YUV buffer size for GRA zoom function (larger than 640x360)
        //if (gScalSrc == MMPF_SCALER_SOURCE_GRA){
        if ( (gra_path & 0x02)  && (is_preview[0]) ){
            MMP_UBYTE pipe_cfg_dst ;
            MMPF_SCALER_ZOOM_INFO *scalinfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_1);
            scalpath[0] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_1);
            pipe_cfg_dst = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe->pipe_cfg);
            /* sean@2013_02_15 DEL
            if(pipe_cfg_dst==PIPE_CFG_H264) {
                usCropHeight = ALIGN16(usCropHeight);
            }
            */
        	size = usCropWidth * usCropHeight ; // OOXX
        	// Update zoom ratio after change scaler source
        	
        	if( (scalpath[0]->n != ZOOM_RATIO_BASE)&& (scalpath[0]->m != ZOOM_RATIO_BASE) ) {
            	if( (gsZoomRatio[0] != scalpath[0]->n ) || (gsZoomRatioM[0] != scalpath[0]->m ) ) {
            	    dbg_printf(GRA_DBG_LEVEL,"0:IBC2.New(N/M):(%d/%d)-(%d/%d),GRA=(%d, %d)\r\n",gsZoomRatio[0],gsZoomRatioM[0],
            	                scalpath[0]->n, scalpath[0]->m,
            	                gbGRAEn[0], gbGRAEn[1]);
            	    gbGRAEn[0] = is_preview[0] ? MMP_TRUE : MMP_FALSE;
            	    gsZoomRatio[0]  = scalpath[0]->n ;
            	    gsZoomRatioM[0] = scalpath[0]->m ; 
            	    gsLastZoomRatio[0] = ZOOM_RATIO_BASE ; // force enter to reset scaler range
            	    
            	} else {
                    MMPF_SCALER_ZOOM_INFO *zoominfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_1);
                    //MMPF_SCALER_SWITCH_PATH *scalinfo = MMPF_Scaler_GetPathInfo() ;
                    if(scalpath[0]->flowctl==SCAL_WAIT_ENC_END) {
                        //dbg_printf(3,"=-Ohi[%d,%d / %d]\r\n",gsZoomRatio , zoominfo->grabCtl.usScaleN,zoominfo->grabCtl.usScaleM);
                        dbg_printf(3,"=-Ohi[%d,%d / %d]\r\n",gsZoomRatio[0]  , scalpath[0]->n,scalpath[0]->m);
                        //if(zoominfo->grabCtl.usScaleN<=zoominfo->grabCtl.usScaleM) {
                        if(scalpath[0]->n<=scalpath[0]->m) {
                            gbGRAEn[0] = MMP_FALSE ;
                            //gbGRAEn[1] = MMP_FALSE ;
                    	    gsZoomRatio[0]  = scalpath[0]->n ;
                    	    gsZoomRatioM[0] = scalpath[0]->m ; 
                    	    gsLastZoomRatio[0] = ZOOM_RATIO_BASE ; // force enter to reset scaler range
                    	    scalpath[0]->flowctl=SCAL_ENC_END;
                        }
                    }
            	}
        	}
        	
        	
        }
        if ( (gra_path & 0x04)  && (is_preview[1]) ){

            MMPF_SCALER_ZOOM_INFO *scalinfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_2);
            scalpath[1] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_2);
        	size = usCropWidth * usCropHeight ; // OOXX
            if( (scalpath[1]->n != ZOOM_RATIO_BASE)&& (scalpath[1]->m != ZOOM_RATIO_BASE) ) {
            	//if(gsZoomRatio != scalpath[1]->n ) {
            	if( (gsZoomRatio[1] != scalpath[1]->n ) || (gsZoomRatioM[1]  != scalpath[1]->m ) ) 
            	{
            	    dbg_printf(GRA_DBG_LEVEL,"1:IBC2.New(N/M):(%d/%d)-(%d/%d),GRA=(%d, %d)\r\n",gsZoomRatio[1] ,gsZoomRatioM[1] ,
            	                scalpath[1]->n, scalpath[1]->m,
            	                gbGRAEn[0], gbGRAEn[1]);
            	    gbGRAEn[1] = is_preview[1] ? MMP_TRUE : MMP_FALSE;
            	    gsZoomRatio[1]  = scalpath[1]->n ;
            	    gsZoomRatioM[1]  = scalpath[1]->m ; 
                    gsLastZoomRatio[1]  = ZOOM_RATIO_BASE ; // force enter to reset scaler range
                    // workaround for simulcast local res < h264 res
                    if( UVCX_IsSimulcastH264() ) {
                        if(gGRAStreamingCfg.streaming_num==3) {
                            MMPF_Display_SyncH264Ratio();
                        }
                    }
                    
                } else	{
                #if 1 // OOXX test
                    MMPF_SCALER_ZOOM_INFO *zoominfo = (MMPF_SCALER_ZOOM_INFO *)MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_2);
                    //MMPF_SCALER_SWITCH_PATH *scalinfo = MMPF_Scaler_GetPathInfo() ;
                    if(scalpath[1]->flowctl==SCAL_WAIT_ENC_END) {
                        //dbg_printf(3,"=-Ohi[%d,%d / %d]\r\n",gsZoomRatio , zoominfo->grabCtl.usScaleN,zoominfo->grabCtl.usScaleM);
                        dbg_printf(3,"=-Ohi[%d,%d / %d]\r\n",gsZoomRatio[1]  , scalpath[1]->n,scalpath[1]->m);
                        //if(zoominfo->grabCtl.usScaleN<=zoominfo->grabCtl.usScaleM) {
                        if(scalpath[1]->n<=scalpath[1]->m) {
                            //gbGRAEn[0] = MMP_FALSE ;
                            gbGRAEn[1] = MMP_FALSE ;
                    	    gsZoomRatio[1]   = scalpath[1]->n ;
                    	    gsZoomRatioM[1]  = scalpath[1]->m ; 
                    	    gsLastZoomRatio[1]  = ZOOM_RATIO_BASE ; // force enter to reset scaler range
                    	    scalpath[1]->flowctl=SCAL_ENC_END;
                            // workaround for simulcast local res < h264 res
                            if( UVCX_IsSimulcastH264() ) {
                                if(1/*UVC_VCD()==bcdVCD10*/) {
                                    gbGRAEn[0] = gbGRAEn[1] ;
                                    gsZoomRatio[0] = gsZoomRatioM[0]  = ZOOM_RATIO_BASE ;
                                    gsLastZoomRatio[0]  = ZOOM_RATIO_BASE ; 
                                }
                            }
                    	    
                        }
                    }
                #else
                    //dbg_printf(3,"TBD flow\r\n");
                #endif    
                }
            
            }
        
        }     
        if( !(((gra_path & 0x02)  && (is_preview[0]))||((gra_path & 0x04)  && (is_preview[1]))) )	{
        	size = cur_pipe->pipe_w[yuv_pipe]* cur_pipe->pipe_h[yuv_pipe];
        	//gbGRAEn = 0;
        }	
               
               
        pIBCP->IBC_ADDR_U_ST = pIBCP->IBC_ADDR_Y_ST + size;
        pIBCP->IBC_ADDR_V_ST = pIBCP->IBC_ADDR_U_ST + (size/4);
		//dbg_printf(3, "ISPISR = x%x, gsZoomRatio = %d \r\n", pISP->ISP_INT_CPU_SR, gsZoomRatio);
    }
#else
    if(pipe_cfg == PIPE_CFG_NV12) {
        // TBD
        size = cur_pipe->pipe_w[yuv_pipe]* cur_pipe->pipe_h[yuv_pipe];
        pIBCP->IBC_ADDR_U_ST = pIBCP->IBC_ADDR_Y_ST + size;
        pIBCP->IBC_ADDR_V_ST = pIBCP->IBC_ADDR_U_ST + (size/4);
    }
#endif
    // We can turn on IBC 2 only if FDTC is in idle
    // TBD
    //dbg_printf(3,"pipe : %d,gra_path:%d\r\n",yuv_pipe,gra_path );
    if ( (gra_path)&&(yuv_pipe==GRA_SRC_PIPE) ) {
    #if H264_SIMULCAST_EN
        if(! UVCX_IsSimulcastH264()) {
            MMPF_IBC_SetStoreEnable( yuv_pipe,((gra_path&0x2)||(gra_path&0x4))?MMP_TRUE:MMP_FALSE);
        }
        else {
            if( gbGRAEn[0] ||  gbGRAEn[1] ) {
                MMPF_IBC_SetStoreEnable( yuv_pipe,((gra_path&0x2)||(gra_path&0x4))?MMP_TRUE:MMP_FALSE);
            }
        }
    #else
        
    #endif
            
    } else {
        if(! UVCX_IsSimulcastH264()) {
            MMPF_IBC_SetStoreEnable( yuv_pipe,MMP_TRUE);
        }
    }
    return MMP_ERR_NONE ;
}

MMP_BOOL MMPF_Display_SetGRAFps(MMP_BOOL graloop,MMP_USHORT fps);

MMP_BOOL MMPF_Display_SetGRAFps(MMP_BOOL graloop,MMP_USHORT fps)
{
extern MMP_USHORT gsCurFrameRatex10[];
extern MMP_UBYTE  gbAEPriority ;
static MMP_USHORT gbGraFps10 = 0,gbRtTxMode = 0;    
    MMP_BOOL  is_preview[2] = {0,0};
    MMP_UBYTE pipe_cfg[2] = {0xFF,0xFF};
    STREAM_CFG *cur_pipe[2] = {0,0};
    
    if(UVC_VCD()==bcdVCD15) {
        STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(0);
 
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG);
        MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264 );
        if(is_preview[0]) {
       	    cur_pipe[0] = usb_get_cur_image_pipe(GRA_DST_PIPE_1) ;
       	}
       	if(is_preview[1]) {
      	    cur_pipe[1] = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
      	}
      	
      	if(cur_pipe[0]) {
            pipe_cfg[0] = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe[0]->pipe_cfg);
        }
        if(cur_pipe[1]) {
            pipe_cfg[1] = CUR_PIPE_CFG(GRA_DST_PIPE_2,cur_pipe[1]->pipe_cfg);
        }
        if(fps) {
            gbGraFps10 = fps ;
            return MMP_TRUE ;
        }
        if( graloop) {
            if( is_preview[1] ) {
                if( is_preview[0] && (pipe_cfg[0]==PIPE_CFG_YUY2)) {
                    MMPF_Display_SetGRAMCI(); 
                    if(gbGraFps10!=300) {
                        MMP_USHORT newfps = MMPF_Display_GetGRAFpsLimitation( cur_pipe[0], cur_pipe[1]);
                        if(newfps < 300) {
                            //gbRtTxMode = ss->tx_flag & SS_TX_RT_MODE ;
                            //ss->tx_flag &= ~SS_TX_RT_MODE ;
                            PCAM_USB_Set_FPSx10(gbAEPriority,0,newfps /*300*/);   
                            dbg_printf(0,"--fps : %d\r\n", newfps);
                            gbGraFps10 = 300 ;
                            return MMP_TRUE ;
                        }
                    }
                    
                }
            } else {
                if(gbGraFps10) {
                    
                    PCAM_USB_Set_FPSx10(gbAEPriority,0,gbGraFps10/*300*/);
                    dbg_printf(0,"++fps : %d\r\n", gbGraFps10 );
                    gbGraFps10 = 0 ;
                    MMPF_Display_SetGRAMCI();  
                    //ss->tx_flag |= gbRtTxMode ;
                }
            }
        } else {
            if(gbGraFps10) {
                PCAM_USB_Set_FPSx10(gbAEPriority,0,gbGraFps10/*300*/);
                dbg_printf(0,"@@fps : %d\r\n", gbGraFps10 );
                gbGraFps10 = 0 ;
                MMPF_Display_SetGRAMCI();
                //ss->tx_flag |= gbRtTxMode ;
            }
        }
    }
    return FALSE ;
}

MMP_USHORT MMPF_Display_SetGRAMCI(void)
{
	volatile MMP_UBYTE* REG_BASE_B = (volatile MMP_UBYTE*)0x80007700;
	
    if(gsZoomRatio[GRA_DST_PIPE_2] >= 2 * gsZoomRatioM[GRA_DST_PIPE_2]) {
        REG_BASE_B[0x64] = 0x7 ; // ibc 0 na
        REG_BASE_B[0x84] = 0x7 ; // ibc 0 row hit
        REG_BASE_B[0xA4] = 0x11; // ibc 0 conti
    }
    else if(gsZoomRatio[GRA_DST_PIPE_2] >= gsZoomRatioM[GRA_DST_PIPE_2] * 6 / 5) {
        REG_BASE_B[0x64] = 0x7; // ibc 0 na
        REG_BASE_B[0x84] = 0x7; // ibc 0 row hit
        REG_BASE_B[0xA4] = 0x11; // ibc 0 conti
    }
    else {
        REG_BASE_B[0x64] = 0x11; // ibc 0 na
        REG_BASE_B[0x84] = 0x11; // ibc 0 row hit
        REG_BASE_B[0xA4] = 0x11; // ibc 0 conti
    }  
    dbg_printf(3,"--mci(%x,%x,%x)\r\n",REG_BASE_B[0x64],REG_BASE_B[0x84],REG_BASE_B[0xA4] );
    return 0;
}

MMP_USHORT MMPF_Display_GetGRAFpsLimitation(STREAM_CFG *cur_pipe0,STREAM_CFG *cur_pipe1)
{
    MMP_USHORT newfps = 300;// gsCurFrameRatex10[0] ;
    
    if( cur_pipe1->pipe_w[PIPE_1] * cur_pipe1->pipe_h[PIPE_1] >= 1920*1080 ) {
        if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 1920*1080 ) {
            newfps = 50 ;
        }
        else if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 1600*896 ) {
            newfps = 75 ;
        }
        else if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 1280*720 ) {
            newfps = 100 ;
        }
        else if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 1024*576 ) {
            newfps = 150 ;
        }
        else if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 960 * 720 ) {
            newfps = 150 ;
        }
        else if(cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0]  >= 960 * 540 ) {
            newfps = 200 ;
        }
        else if( cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 912 * 512 ) {
            newfps = 240 ;
        }
        else if(cur_pipe0->pipe_w[PIPE_0] * cur_pipe0->pipe_h[PIPE_0] >= 640 * 360 ) {
            newfps = 240 ;
        }
        
    }
    
    return newfps ;
}



void MMPF_Display_CallBackGRA(void);
MMP_USHORT gsPixlDly[2] = {2,2} ;
MMP_USHORT gsLineDly[2] = {0x60,0x60} ;
extern GRA_STREAMING_CFG gGRAStreamingCfg;

MMP_ERR MMPF_Display_TriggerGRA(MMP_UBYTE eid/*yuv_pipe*/, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight)
{
#define LINE_DELAY_TEST (0)
#if SUPPORT_GRA_ZOOM
static  MMP_SHORT   skipYUY2Frame = -1 ;
    MMP_BOOL        skipOneFrame = 0 ;

    AITPS_SCAL  pSCAL   = AITC_BASE_SCAL;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    MMPF_SCALER_FIT_RANGE fitrange;
    MMPF_SCALER_GRABCONTROL grabctl;
    STREAM_CFG *cur_pipe[2] = {0,0};
    STREAM_CFG *stream_cfg = 0;
    
    MMP_UBYTE pipe_cfg[2] = {0xFF,0xFF};
    MMP_USHORT gra_src_w = usCropWidth ,gra_src_h = usCropHeight;
    MMPF_SCALER_SWITCH_PATH *scalinfo[2] = {0,0};
    MMP_BOOL   stop_gra = MMP_FALSE ;
    MMP_UBYTE  dst_pipe = 0,start_pipe = 0 ,end_pipe = 2,grapath;
    MMP_BOOL  is_preview[2] = {0,0};
    MMP_BOOL  yuv_on = MMP_FALSE ;
  //  dbg_printf(3,"Scaling up Crop(%d,%d)\r\n",usCropWidth,usCropHeight);
  	//dbg_printf(3," #Zoom (%d,%d)\r\n", gsZoomRatio, gsLastZoomRatio);
  	if(!usCropWidth || !usCropHeight) {
  	   // dbg_printf(3,"--Cropzero(%d,%d)\r\n",usCropWidth,usCropHeight);
  	    skipOneFrame = 1 ;
  	}
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG);
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264 );
    
    
  	cur_pipe[0] = usb_get_cur_image_pipe(GRA_DST_PIPE_1) ;
  	cur_pipe[1] = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
    scalinfo[0] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_1) ;
    scalinfo[1] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_2) ;
    if(cur_pipe[0]) {
        pipe_cfg[0] = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe[0]->pipe_cfg);
        stream_cfg = cur_pipe[0];
        if( is_preview[0] && ( pipe_cfg[0] == PIPE_CFG_YUY2) ) {
            if(UVC_VCD()==bcdVCD15) {
                yuv_on = MMP_TRUE ; 
            }    
        }
    }
    if(cur_pipe[1]) {
        pipe_cfg[1] = CUR_PIPE_CFG(GRA_DST_PIPE_2,cur_pipe[1]->pipe_cfg); 
        stream_cfg = cur_pipe[1] ;
    }
    
	//dbg_printf(3," #cur_pipe (%x,%x)\r\n", cur_pipe[0], cur_pipe[1]);
     
#if 0//(GRA_SRC_FIXED_SIZE==1)&&(SUPPORT_GRA_ZOOM==GRA_ZOOM_BY_CROP)
    gra_src_w = cur_pipe0->pipe_w[GRA_SRC_PIPE];
    gra_src_h = cur_pipe0->pipe_h[GRA_SRC_PIPE];
#endif    
    fitrange.fitmode = MMPF_SCALER_FITMODE_OUT;
    
    if(UVC_VCD()==bcdVCD15) {
        if(is_preview[0]) {
            start_pipe = 0;
        }
        else {
            start_pipe = 1;
        }
        if(is_preview[1]) {
            end_pipe = 2 ;
        }
        else {
            end_pipe = 1  ;
        }
    }
    else {
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN ) {
            start_pipe = 0;
        }
        else {
            start_pipe = 1;
        }
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN ) {
            end_pipe = 2 ;
        }
        else {
            end_pipe = 1 ;
        }
        
    }
    
    grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;
    
    for(dst_pipe = start_pipe; dst_pipe < end_pipe;dst_pipe++ ) {
       // dbg_printf(3,"T(N,M,LN) : (%d,%d,%d)\r\n",gsZoomRatio,gsZoomRatioM,gsLastZoomRatio);
        if(gsZoomRatio[dst_pipe] != gsLastZoomRatio[dst_pipe]){
            if(gbGRAEn[dst_pipe]) {// over 1X Zoom => trigger GRA
                 if(gsZoomRatio[dst_pipe] > gsZoomRatioM[dst_pipe] ) {	// roll-back to fix single stream zoom
                    MMP_BOOL skip_more_frames;  
                    skip_more_frames = MMPF_Display_SetGRAFps( 1 , 0 );   
                    if(skip_more_frames && (skipYUY2Frame==-1) ){
                        MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPE_0,MMP_FALSE);
                    }	
                    dbg_printf(GRA_DBG_LEVEL, "T:GRA-s p%d(%d,%d) crop( %d, %d). \r\n",dst_pipe, gsZoomRatio[dst_pipe],gsZoomRatioM[dst_pipe], usCropWidth, usCropHeight);
                    // update grab range to fit the configs of destination pipes.
                    MMPF_Display_ConfigGRADstScaler(dst_pipe, cur_pipe[dst_pipe], usCropWidth,usCropHeight,gsZoomRatioM[dst_pipe] );
                    //dbg_printf(3, "GRA->P%d(%d, %d) -> (%d,%d)\r\n", GRA_DST_PIPE_1,grabctl.usStartX, grabctl.usEndX,grabctl.usStartY, grabctl.usEndY);
                    // update grab range to fit the config of source pipe
                    MMPF_Display_ConfigGRASrcScaler(gra_src_w,gra_src_h);       
                    if(pipe_cfg[dst_pipe]==PIPE_CFG_MJPEG) {
                        MMPF_Video_EnableJpeg();
                        dbg_printf(GRA_DBG_LEVEL,"#T:GRA(J-Enc),FCTL:%d\r\n",scalinfo[dst_pipe]->flowctl) ;   
                    } 
                    gsLastZoomRatio[dst_pipe] = gsZoomRatio[dst_pipe] ;
                    #if SCALER_ZOOM_LEVEL==BEST_LEVEL  
                    MMPF_Scaler_DisableZoom(dst_pipe);
                    #endif      
                    skipOneFrame = 1;
                    if(skip_more_frames) {
                        skipYUY2Frame = 5;
                    }

                } else {
                    dbg_printf(GRA_DBG_LEVEL,"T:Unexpect N/M(%d/%d)\r\n",gsZoomRatio[dst_pipe],gsZoomRatioM[dst_pipe]);
                    dbg_printf(GRA_DBG_LEVEL,"Too fast Op[%d]....\r\n",dst_pipe);
                    if(scalinfo[dst_pipe]->set) {
                        if(scalinfo[dst_pipe]->flowctl==SCAL_WAIT_ENC_END) {
                            scalinfo[dst_pipe]->flowctl = SCAL_ENC_END ;
                        }
                    }
                    gsLastZoomRatio[dst_pipe] = ZOOM_RATIO_BASE ;
                }

            } else {
                 if(MMPF_Scaler_GetLineDelay(dst_pipe)) {
                    MMPF_Display_SetGRAFps( 0 , 0 );
                    dbg_printf(GRA_DBG_LEVEL, "<T:ISP-s:%d,en(%d,%d),Z:%d\r\n",dst_pipe,gbGRAEn[0],gbGRAEn[1],usb_vc_cur_usof());
                    MMPF_Scaler_SetPixelLineDelay(dst_pipe, 0, 0);
                    
                    MMPF_Scaler_SetZoomSinglePipe(dst_pipe);  // OOXX ALIGN16 

                    #if SUPPORT_CHANGE_AE_WINDOW
                    // AE window change to GRA SRC pipe
                    MMPF_Sensor_UpdateAEWindow((1 << dst_pipe),dst_pipe);
                    #endif            

                    #if SCALER_ZOOM_LEVEL==BEST_LEVEL  
                    MMPF_Scaler_DisableZoom(dst_pipe);
                    #endif
                    skipOneFrame = 1;      
                    if(pipe_cfg[dst_pipe]==PIPE_CFG_MJPEG) {
                        MMPF_Video_EnableJpeg();
                        dbg_printf(GRA_DBG_LEVEL,"#T:ISP(J-Enc)\r\n") ;   
                    } 
                    // MMPF_IBC_SetStoreEnable();
                    gsLastZoomRatio[dst_pipe] = ZOOM_RATIO_BASE;
                    //sean@2013_02_16 ADD
                    //gsZoomRatio[dst_pipe] = ZOOM_RATIO_BASE;
                    if(scalinfo[dst_pipe]->flowctl==SCAL_ENC_END) {
                        scalinfo[dst_pipe]->flowctl = SCAL_READY_TO_SWITCH;
                        dbg_printf(GRA_DBG_LEVEL, ">T :set S_R_T_S%d <<\r\n",dst_pipe); 
                        stop_gra = MMP_TRUE ;		
                    }
                }    	
            } 		
        }
	}
	//else  // removed it
	//if(!skipOneFrame)
	if( (! skipOneFrame ) && (!gbSkipGraLoop) )
	{
    	//if(gScalSrc == MMPF_SCALER_SOURCE_GRA){	// patrick: GRA grab zoom in
    	gsLineDly[0] = gsLineDly[1] = 0;
    	gsPixlDly[0] = gsPixlDly[1] = 2;
    	
    	if(gbGRAEn[GRA_DST_PIPE_1] || gbGRAEn[GRA_DST_PIPE_2]){
            MMPF_GRAPHICS_BUFATTRIBUTE src, dst;
            AITPS_GRA	pGRA = AITC_BASE_GRA;
            MMP_USHORT usPixlDly = gsPixlDly[0], usLineDly = gsLineDly[0];
    		
            dst.ulBaseAddr 		= (dst.ulBaseUAddr = (dst.ulBaseVAddr = 0));
            src.ulBaseAddr 		= buf_addr /*+ FRAME_PAYLOAD_HEADER_SZ*/;
            src.ulBaseUAddr 	= src.ulBaseAddr + gra_src_w * gra_src_h;//usCropWidth * usCropHeight;
            src.usLineOffset 	= gra_src_w;//usCropWidth;
    		src.colordepth 		= MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;// MMPF_GRAPHICS_COLORDEPTH_YUV422;			        
            src.usWidth 		= gra_src_w;//usCropWidth;//cur_pipe->pipe_w[yuv_pipe];
            src.usHeight 		= gra_src_h;//usCropHeight;//cur_pipe->pipe_h[yuv_pipe];
            MMPF_Graphics_SetCrop(  0/*(cur_pipe->pipe_w[yuv_pipe] - usCropWidth)/2*/, \
            						0/*(cur_pipe->pipe_h[yuv_pipe] - usCropHeight)/2*/, \
            						usCropWidth, usCropHeight,
            						src.ulBaseAddr,src.ulBaseUAddr,src.ulBaseUAddr);
            						
            /* comment to roll-back            						
            #if SUPPORT_OSD_FUNC            						
    	    if(pcam_Osd_IsEnable()) {
    	        MMP_USHORT orig_w,orig_h;
    	        pcam_Osd_GetCurrentStreamRes(&orig_w,&orig_h);
    	        pcam_Osd_ConfigCurrentStream(MMP_TRUE,usCropWidth,usCropHeight);
    	        pcam_SetOsdString2YuvBuffer(__OSD_STR, _strlen(__OSD_STR), src.ulBaseAddr,src.ulBaseUAddr);
    	        pcam_Osd_ConfigCurrentStream(MMP_TRUE,orig_w,orig_h);
    	    }
            #endif
            */				
                        					
            if (gbGRAEn[GRA_DST_PIPE_1]) {
                // no need to config dst scaler, this function is not for simulcast
                // MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_1, cur_pipe[GRA_DST_PIPE_1], usCropWidth,usCropHeight,gsZoomRatioM[GRA_DST_PIPE_1]  );
                
                // The scaler line delay is depend on format.
         		if(pipe_cfg[GRA_DST_PIPE_1] == PIPE_CFG_MJPEG) {
                    //usLineDly = 0x40;//0x30;
                    gsLineDly[GRA_DST_PIPE_1] = 0x40;
        		}
        		else if(pipe_cfg[GRA_DST_PIPE_1]==PIPE_CFG_YUY2) {
        		    //usLineDly = 0xc0;
        		    gsLineDly[GRA_DST_PIPE_1] = 0xc0;
                    if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 1600*1200) {
                        //usPixlDly = 0x06;
                        gsPixlDly[GRA_DST_PIPE_1] = 0x06;
                    }
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 1280*720) {
                        //usPixlDly = 0x05; // 3->4 for yuy2 720p + h264 720p
                        gsPixlDly[GRA_DST_PIPE_1] = 0x05;
                    } 
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 864*480) {
                        gsPixlDly[GRA_DST_PIPE_1] = 0x04;
                    }
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 864*480) {
                        //usPixlDly = 0x02;
                        gsPixlDly[GRA_DST_PIPE_1] = 0x03;
                        //usLineDly = 0xf0;
                    }   
                    else {
                        gsPixlDly[GRA_DST_PIPE_1] = 0x02;
                    } 
                    
         		}
                else if(pipe_cfg[GRA_DST_PIPE_1]==PIPE_CFG_H264) {
                    if(gsZoomRatio[GRA_DST_PIPE_1] >= 2 * gsZoomRatioM[GRA_DST_PIPE_1]) {
                        //usLineDly = 0x88 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x88 ;
                    }
                    else if(gsZoomRatio[GRA_DST_PIPE_1] >= gsZoomRatioM[GRA_DST_PIPE_1] * 6 / 5) {
                        //usLineDly = 0x60 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x60 ;
                    }
                    else {
                        //usLineDly = 0x30 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x30 ;
                    }
                }
                usLineDly = gsLineDly[GRA_DST_PIPE_1] ;
    		    MMPF_Scaler_SetPixelLineDelay(GRA_DST_PIPE_1, 0, usLineDly);
            }
            if (gbGRAEn[GRA_DST_PIPE_2]){
                // no need for non-simulcast
                // MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_2, cur_pipe[GRA_DST_PIPE_2], usCropWidth,usCropHeight,gsZoomRatioM[GRA_DST_PIPE_2]  );
           
                // h264 endpoint only
                if(pipe_cfg[GRA_DST_PIPE_2]==PIPE_CFG_H264) {
                    if(gsZoomRatio[GRA_DST_PIPE_2] >= 2 * gsZoomRatioM[GRA_DST_PIPE_2]) {
                        //usLineDly = 0x88 ;
                        gsLineDly[GRA_DST_PIPE_2] =  (yuv_on) ? 0x98:0x88;
                    }
                    else if(gsZoomRatio[GRA_DST_PIPE_2] >= gsZoomRatioM[GRA_DST_PIPE_2] * 6 / 5) {
                        //usLineDly = 0x80;// 0x60 ;
                        gsLineDly[GRA_DST_PIPE_2] =  (yuv_on) ? 0x80:0x60;
                    }
                    else {
                        //usLineDly = 0x30;
                        gsLineDly[GRA_DST_PIPE_2] = (yuv_on) ? 0x60:0x30;
                    }  
                    usLineDly = gsLineDly[GRA_DST_PIPE_2] ;
                                    
                }          
                MMPF_Scaler_SetPixelLineDelay(GRA_DST_PIPE_2, 0, usLineDly);
			    
            }
            usPixlDly = VR_MAX(gsPixlDly[GRA_DST_PIPE_1],gsPixlDly[GRA_DST_PIPE_2]);
            //dbg_printf(3,"G21:%d\r\n",usb_vc_cur_usof());
            #if H264_SIMULCAST_EN
            {
            
                MMPF_GRAPHICS_RECT rect ;
                rect.usWidth           = src.usWidth;
                rect.usHeight          = src.usHeight;
                rect.usLeft            = 0;
                rect.usTop             = 0;
                MMPF_Graphics_SetScaleAttribute(&src, 0, MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE,
                                                &rect, 1, MMPF_GRAPHICS_SCAL_FB);
           
            }
            MMPF_Graphics_Scale_P0(src, dst, MMPF_Display_CallBackGRA, usPixlDly, 0);
            #else
            MMPF_Graphics_Scale(src, dst, MMPF_Display_CallBackGRA, usPixlDly, 0);
            #endif
        }	
        else {
           // dbg_printf(3,"OUT of Handle path\r\n");

        }
    }
    if(skipYUY2Frame > 0 ) {
        skipYUY2Frame--;
    } else if(skipYUY2Frame==0){
        MMPF_IBC_SetStoreEnable(MMPF_IBC_PIPE_0,MMP_TRUE);
        skipYUY2Frame = -1 ;
    }
#endif	//SUPPORT_GRA_ZOOM
    if(stop_gra) {
        dbg_printf(3,"--stop gra\r\n");
        return MMP_GRA_ERR_BUSY ;
    }
	return MMP_ERR_NONE ; 
}

void MMPF_Display_CallBackGRA(void)
{
    //dbg_printf(3,"GRA loop done\r\n");
    //dbg_printf(3,"G21e:%d\r\n",usb_vc_cur_usof());
    MMPF_Video_UpdateRdPtr(GRA_SRC_PIPE);
}

#if H264_SIMULCAST_EN
MMP_BOOL MMPF_Display_ConfigGRA(MMP_UBYTE eid/*yuv_pipe*/, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight)
{
    MMP_BOOL        skipOneFrame = 0 ;
#if SUPPORT_GRA_ZOOM

    AITPS_SCAL  pSCAL   = AITC_BASE_SCAL;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    MMPF_SCALER_FIT_RANGE fitrange;
    MMPF_SCALER_GRABCONTROL grabctl;
    STREAM_CFG *cur_pipe[2] = {0,0};
    STREAM_CFG *stream_cfg = 0;
    
    //MMP_UBYTE pipe_cfg = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe->pipe_cfg);
    MMP_UBYTE pipe_cfg[2] = {0xFF,0xFF};
    //dbg_printf(3, "### FRAMEBASE_H264_YUY2...trigger graphics\r\n");
    MMP_USHORT gra_src_w = usCropWidth ,gra_src_h = usCropHeight;
    MMPF_SCALER_SWITCH_PATH *scalinfo[2] = {0,0};
    MMP_BOOL   stop_gra = MMP_FALSE ;
    MMP_UBYTE  dst_pipe = 0,start_pipe = 0 ,end_pipe = 2,grapath;
    MMP_BOOL  is_preview[2] = {0,0};
    MMP_BOOL  yuv_on = MMP_FALSE ;
  //  dbg_printf(3,"Scaling up Crop(%d,%d)\r\n",usCropWidth,usCropHeight);
  	//dbg_printf(3," #Zoom (%d,%d)\r\n", gsZoomRatio, gsLastZoomRatio);
  	if(!usCropWidth || !usCropHeight) {
  	   // dbg_printf(3,"--Cropzero(%d,%d)\r\n",usCropWidth,usCropHeight);
  	    skipOneFrame = 1 ;
  	    return skipOneFrame ;
  	}
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG);
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264 );
    
    
  	cur_pipe[0] = usb_get_cur_image_pipe(GRA_DST_PIPE_1) ;
  	cur_pipe[1] = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
    scalinfo[0] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_1) ;
    scalinfo[1] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_2) ;
    if(cur_pipe[0]) {
        pipe_cfg[0] = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe[0]->pipe_cfg);
        stream_cfg = cur_pipe[0];
        if( is_preview[0] && ( pipe_cfg[0] == PIPE_CFG_YUY2) ) {
            if(UVC_VCD()==bcdVCD15) {
                yuv_on = MMP_TRUE ; 
            }    
        }
    }
    if(cur_pipe[1]) {
        pipe_cfg[1] = CUR_PIPE_CFG(GRA_DST_PIPE_2,cur_pipe[1]->pipe_cfg); 
        stream_cfg = cur_pipe[1] ;
    }
    
	//dbg_printf(3," #cur_pipe (%x,%x)\r\n", cur_pipe[0], cur_pipe[1]);
     
#if 0//(GRA_SRC_FIXED_SIZE==1)&&(SUPPORT_GRA_ZOOM==GRA_ZOOM_BY_CROP)
    gra_src_w = cur_pipe0->pipe_w[GRA_SRC_PIPE];
    gra_src_h = cur_pipe0->pipe_h[GRA_SRC_PIPE];
#endif    
    fitrange.fitmode = MMPF_SCALER_FITMODE_OUT;
    
    if(UVC_VCD()==bcdVCD15) {
        if(is_preview[0]) {
            start_pipe = 0;
        }
        else {
            start_pipe = 1;
        }
        if(is_preview[1]) {
            end_pipe = 2 ;
        }
        else {
            end_pipe = 1  ;
        }
    }
    else {
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE0_EN ) {
            start_pipe = 0;
        }
        else {
            start_pipe = 1;
        }
        if( PIPE_EN(stream_cfg->pipe_en) & PIPE1_EN ) {
            end_pipe = 2 ;
        }
        else {
            end_pipe = 1 ;
        }
        
    }
    
    grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;
    
    for(dst_pipe = start_pipe; dst_pipe < end_pipe;dst_pipe++ ) {
       // dbg_printf(3,"T(N,M,LN) : (%d,%d,%d)\r\n",gsZoomRatio,gsZoomRatioM,gsLastZoomRatio);
        if(gsZoomRatio[dst_pipe] != gsLastZoomRatio[dst_pipe]){
            dbg_printf(3,"zoomR,lastR [%d]: %d,%d\r\n",dst_pipe,gsZoomRatio[dst_pipe],gsLastZoomRatio[dst_pipe]);
            if(gbGRAEn[dst_pipe]) {// over 1X Zoom => trigger GRA
                 if( (gsZoomRatio[dst_pipe] > gsZoomRatioM[dst_pipe]) || ( (dst_pipe==0)&& ( gsZoomRatio[dst_pipe]== gsZoomRatioM[dst_pipe] ) ) ) {
                    MMP_USHORT zoomRatioN ,zoomRatioM = gsZoomRatioM[dst_pipe];
                    if(gbH264Encoding) {
                        dbg_printf(3,"--still-encoding\r\n") ;
                        return 1 ;
                    }
                 	
                    dbg_printf(GRA_DBG_LEVEL, "T:GRA-s p%d(%d,%d) crop( %d, %d). \r\n",dst_pipe, gsZoomRatio[dst_pipe],gsZoomRatioM[dst_pipe], usCropWidth, usCropHeight);
                    if(dst_pipe==GRA_DST_PIPE_1) {
                        // update grab range to fit the configs of destination pipes.
 retry:                    
                        zoomRatioN = MMPF_Display_ConfigGRADstScaler(dst_pipe, cur_pipe[dst_pipe], usCropWidth,usCropHeight ,zoomRatioM );
                        if(zoomRatioN > 127) {
                            zoomRatioM = zoomRatioM >> 1 ;
                            goto retry ;                  
                        }
                    }
                    else {
                        MMPF_Display_ConfigGRADstScaler(dst_pipe, cur_pipe[dst_pipe], usCropWidth,usCropHeight ,zoomRatioM );
                    }
                    //dbg_printf(3, "GRA->P%d(%d, %d) -> (%d,%d)\r\n", GRA_DST_PIPE_1,grabctl.usStartX, grabctl.usEndX,grabctl.usStartY, grabctl.usEndY);
                    // update grab range to fit the config of source pipe
                    MMPF_Display_ConfigGRASrcScaler(gra_src_w,gra_src_h);       
                    if(pipe_cfg[dst_pipe]==PIPE_CFG_MJPEG) {
                        AITPS_JPG  pJPG = AITC_BASE_JPG;
                        
                        if(UVCX_IsSimulcastH264()) {
                            MMPF_Display_ConfigGRADstIbc(dst_pipe,PIPE_CFG_MJPEG);
                        }
                            
                       // pJPG->JPG_CTL |= JPG_ENC_EN;
                        dbg_printf(GRA_DBG_LEVEL,"#T:GRA(J-Enc),FCTL:%d\r\n",scalinfo[dst_pipe]->flowctl) ;   
                    } 
                    if(pipe_cfg[dst_pipe]==PIPE_CFG_YUY2) {
                        if(UVCX_IsSimulcastH264()) {
                            MMPF_Display_ConfigGRADstIbc(dst_pipe,PIPE_CFG_YUY2);
                        }
                    
                    }
                    
                    gsLastZoomRatio[dst_pipe] = gsZoomRatio[dst_pipe] ;
                    #if SCALER_ZOOM_LEVEL==BEST_LEVEL  
                    MMPF_Scaler_DisableZoom(dst_pipe);
                    #endif      
                    skipOneFrame = 1;
                    
                    gsGraDstZoomRatioM[dst_pipe] = zoomRatioM ;//gsZoomRatioM[dst_pipe] ;
                    //gsGraDstZoomRatioN[dst_pipe] = zoomRatioN ;//gsZoomRatio[dst_pipe]  ;
                    //gsGraDstZoomRatioM[dst_pipe] = gsZoomRatioM[dst_pipe] ;
                    if(dst_pipe==GRA_DST_PIPE_1) {
                        gsGraDstZoomRatioN[dst_pipe] = zoomRatioN ;
                        gsGraDstZoomRatioM[dst_pipe] = zoomRatioM ;
                    }
                    else {
                        gsGraDstZoomRatioN[dst_pipe] = gsZoomRatio[dst_pipe]  ;
                        gsGraDstZoomRatioM[dst_pipe] = gsZoomRatioM[dst_pipe]  ;
                        
                        if( pcam_Osd_IsEnable() ) {
                            pcam_Osd_ConfigCurrentStream(MMP_FALSE,0,0);
                            USB_ScalingOSD(PCAM_OVERWR,1,gsGraDstZoomRatioN[dst_pipe],gsGraDstZoomRatioM[dst_pipe]);
                        }
                        
                    }    

                } else {
                    dbg_printf(GRA_DBG_LEVEL,"T:Unexpect N/M(%d/%d)\r\n",gsZoomRatio[dst_pipe],gsZoomRatioM[dst_pipe]);
                    dbg_printf(GRA_DBG_LEVEL,"Too fast Op[%d]....\r\n",dst_pipe);
                    if(scalinfo[dst_pipe]->set) {
                        if(scalinfo[dst_pipe]->flowctl==SCAL_WAIT_ENC_END) {
                            scalinfo[dst_pipe]->flowctl = SCAL_ENC_END ;
                        }
                    }
                    gsLastZoomRatio[dst_pipe] = ZOOM_RATIO_BASE ;
                }

            } else {
                 if(MMPF_Scaler_GetLineDelay(dst_pipe)) {
                    //MMPF_Display_SetGRAFps( 0 , 0 );
                    dbg_printf(GRA_DBG_LEVEL, "<T:ISP-s:%d,en(%d,%d),Z:%d\r\n",dst_pipe,gbGRAEn[0],gbGRAEn[1],usb_vc_cur_usof());
                    MMPF_Scaler_SetPixelLineDelay(dst_pipe, 0, 0);
                    
                    MMPF_Scaler_SetZoomSinglePipe(dst_pipe);  // OOXX ALIGN16 

                    #if SUPPORT_CHANGE_AE_WINDOW
                    // AE window change to GRA SRC pipe
                    MMPF_Sensor_UpdateAEWindow((1 << dst_pipe),dst_pipe);
                    #endif            

                    #if SCALER_ZOOM_LEVEL==BEST_LEVEL  
                    MMPF_Scaler_DisableZoom(dst_pipe);
                    #endif
                    skipOneFrame = 1;      
                    if(pipe_cfg[dst_pipe]==PIPE_CFG_MJPEG) {
                        MMPF_Video_EnableJpeg();
                        dbg_printf(GRA_DBG_LEVEL,"#T:ISP(J-Enc)\r\n") ;   
                    } 
                    // MMPF_IBC_SetStoreEnable();
                    gsLastZoomRatio[dst_pipe] = ZOOM_RATIO_BASE;
                    //sean@2013_02_16 ADD
                    //gsZoomRatio[dst_pipe] = ZOOM_RATIO_BASE;
                    if(scalinfo[dst_pipe]->flowctl==SCAL_ENC_END) {
                        scalinfo[dst_pipe]->flowctl = SCAL_READY_TO_SWITCH;
                        dbg_printf(GRA_DBG_LEVEL, ">T :set S_R_T_S%d <<\r\n",dst_pipe); 
                        stop_gra = MMP_TRUE ;		
                    }
                    
                    gsGraDstZoomRatioM[dst_pipe] = ZOOM_RATIO_BASE ;
                    gsGraDstZoomRatioN[dst_pipe] = ZOOM_RATIO_BASE ;
                }    	
            } 		
        }
	}
	
#endif	//SUPPORT_GRA_ZOOM
	return skipOneFrame ; 
}

MMP_USHORT gsJPEGLineDly = 0x15; 

MMP_ERR MMPF_Display_DoGRA(MMP_UBYTE eid/*yuv_pipe*/, MMP_ULONG buf_addr, MMP_USHORT usCropWidth, MMP_USHORT usCropHeight,MMP_BOOL bSkipGraLoop)
{
#if SUPPORT_GRA_ZOOM
    AITPS_SCAL  pSCAL   = AITC_BASE_SCAL;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    AITPS_JPG   pJPG    = AITC_BASE_JPG;
    STREAM_CFG *cur_pipe[2] = {0,0};
    STREAM_CFG *stream_cfg = 0;
    
    MMP_UBYTE pipe_cfg[2] = {0xFF,0xFF};
    MMP_USHORT gra_src_w = usCropWidth ,gra_src_h = usCropHeight;
    MMPF_SCALER_SWITCH_PATH *scalinfo[2] = {0,0};
    //MMP_BOOL   stop_gra = MMP_FALSE ;
    MMP_UBYTE  dst_pipe = 0,grapath;
    MMP_BOOL  is_preview[2] = {0,0};
    MMP_BOOL  yuv_on = MMP_FALSE ;
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[0],STREAM_EP_MJPEG);
    MMPS_3GPRECD_GetPreviewDisplayByEpId(&is_preview[1],STREAM_EP_H264 );
    
  	cur_pipe[0] = usb_get_cur_image_pipe(GRA_DST_PIPE_1) ;
  	cur_pipe[1] = usb_get_cur_image_pipe(GRA_DST_PIPE_2) ;
    scalinfo[0] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_1) ;
    scalinfo[1] = MMPF_Scaler_GetPathInfo(GRA_DST_PIPE_2) ;
    if(cur_pipe[0]) {
        pipe_cfg[0] = CUR_PIPE_CFG(GRA_DST_PIPE_1,cur_pipe[0]->pipe_cfg);
        stream_cfg = cur_pipe[0];
        if( is_preview[0] && ( pipe_cfg[0] == PIPE_CFG_YUY2) ) {
            if(UVC_VCD()==bcdVCD15) {
                yuv_on = MMP_TRUE ; 
            }    
        }
    }
    if(cur_pipe[1]) {
        pipe_cfg[1] = CUR_PIPE_CFG(GRA_DST_PIPE_2,cur_pipe[1]->pipe_cfg); 
        stream_cfg = cur_pipe[1] ;
    }
    
	//dbg_printf(3," #cur_pipe (%x,%x)\r\n", cur_pipe[0], cur_pipe[1]);
    grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;
    
	//else  // removed it
	//if(!skipOneFrame)
	if( !bSkipGraLoop )
	{
    	//if(gScalSrc == MMPF_SCALER_SOURCE_GRA){	// patrick: GRA grab zoom in
    	gsLineDly[0] = gsLineDly[1] = 0;
    	gsPixlDly[0] = gsPixlDly[1] = 2;
    	
    	if(gbGRAEn[GRA_DST_PIPE_1] || gbGRAEn[GRA_DST_PIPE_2]){
            MMPF_GRAPHICS_BUFATTRIBUTE src, dst;
            AITPS_GRA	pGRA = AITC_BASE_GRA;
            MMP_USHORT usPixlDly = gsPixlDly[0], usLineDly = gsLineDly[0];
    		#if 0
    		dbg_printf(3,"do_gra %d( %d/%d,%d/%d)-(%x,%d,%d)\r\n",eid,gsGraDstZoomRatioN[GRA_DST_PIPE_1],gsGraDstZoomRatioM[GRA_DST_PIPE_1],
    		                                                       gsGraDstZoomRatioN[GRA_DST_PIPE_2],gsGraDstZoomRatioM[GRA_DST_PIPE_2],
    		                                                       buf_addr,usCropWidth,usCropHeight);
    		#endif
    		
            dst.ulBaseAddr 		= (dst.ulBaseUAddr = (dst.ulBaseVAddr = 0));
            src.ulBaseAddr 		= buf_addr /*+ FRAME_PAYLOAD_HEADER_SZ*/;
            src.ulBaseUAddr 	= src.ulBaseAddr + gra_src_w * gra_src_h;
            src.usLineOffset 	= gra_src_w;//usCropWidth;
    		src.colordepth 		= MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;			        
            src.usWidth 		= gra_src_w;
            src.usHeight 		= gra_src_h;
            MMPF_Graphics_SetCrop(  0, \
            						0, \
            						usCropWidth, usCropHeight,
            						src.ulBaseAddr,src.ulBaseUAddr,src.ulBaseUAddr);
            #if SUPPORT_OSD_FUNC            						
    	    if(pcam_Osd_IsEnable()) {
    	        MMP_USHORT orig_w,orig_h;
    	        pcam_Osd_GetCurrentStreamRes(&orig_w,&orig_h);
    	        pcam_Osd_ConfigCurrentStream(MMP_TRUE,usCropWidth,usCropHeight);
    	        //pcam_Osd_DrawStr(__OSD_STR, _strlen(__OSD_STR), src.ulBaseAddr,src.ulBaseUAddr);
    	        pcam_Osd_DrawBuf(1,src.ulBaseAddr,src.ulBaseUAddr);
    	        pcam_Osd_ConfigCurrentStream(MMP_TRUE,orig_w,orig_h);
    	    }
            #endif
            				
                        					
            if (gbGRAEn[GRA_DST_PIPE_1]) {
            
                if(eid==0 ) {
                    MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_1, cur_pipe[GRA_DST_PIPE_1], usCropWidth,usCropHeight,gsGraDstZoomRatioM[GRA_DST_PIPE_1] );
                } 
                else {
                // 2nd GRA loop didn't need local stream
                // pipe 0 do scaling down to save loop time
                    if(gsGraDstZoomRatioN[GRA_DST_PIPE_1] >= gsGraDstZoomRatioM[GRA_DST_PIPE_1] ) {
                        STREAM_CFG dummy_pipe = *cur_pipe[GRA_DST_PIPE_1] ;
                        dummy_pipe.pipe_w[GRA_DST_PIPE_1] = usCropWidth >> 1 ;
                        dummy_pipe.pipe_h[GRA_DST_PIPE_1] = usCropHeight >> 1 ;
                        MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_1, /*cur_pipe[GRA_DST_PIPE_1]*/ &dummy_pipe, usCropWidth,usCropHeight,gsGraDstZoomRatioM[GRA_DST_PIPE_1] );
                    }
                    else {
                        MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_1, cur_pipe[GRA_DST_PIPE_1], usCropWidth,usCropHeight,gsGraDstZoomRatioM[GRA_DST_PIPE_1] );
                    }
                }
                // The scaler line delay is depend on format.
         		if(pipe_cfg[GRA_DST_PIPE_1] == PIPE_CFG_MJPEG) {
                    //usLineDly = 0x40;//0x30;
                    gsLineDly[GRA_DST_PIPE_1] = gsJPEGLineDly;
                    if(eid==0) {
                        MMPF_Video_EnableJpeg();
                    }
                    else {
                    }
        		}
        		else if(pipe_cfg[GRA_DST_PIPE_1]==PIPE_CFG_YUY2) {
        		    //usLineDly = 0xc0;
        		    gsLineDly[GRA_DST_PIPE_1] = 0xc0;
                    if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 1600*1200) {
                        //usPixlDly = 0x06;
                        gsPixlDly[GRA_DST_PIPE_1] = 0x06;
                    }
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 1280*720) {
                        //usPixlDly = 0x05; // 3->4 for yuy2 720p + h264 720p
                        gsPixlDly[GRA_DST_PIPE_1] = 0x05;
                    } 
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 864*480) {
                        gsPixlDly[GRA_DST_PIPE_1] = 0x04;
                    }
                    else if( (cur_pipe[GRA_DST_PIPE_1]->pipe_w[GRA_DST_PIPE_1] * cur_pipe[GRA_DST_PIPE_1]->pipe_h[GRA_DST_PIPE_1]) >= 864*480) {
                        //usPixlDly = 0x02;
                        gsPixlDly[GRA_DST_PIPE_1] = 0x03;
                        //usLineDly = 0xf0;
                    }   
                    else {
                        gsPixlDly[GRA_DST_PIPE_1] = 0x02;
                    } 
                    
         		}
                else if(pipe_cfg[GRA_DST_PIPE_1]==PIPE_CFG_H264) {
                    if(gsGraDstZoomRatioN[GRA_DST_PIPE_1] >= 2 * gsGraDstZoomRatioM[GRA_DST_PIPE_1]) {
                        //usLineDly = 0x88 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x88 ;
                    }
                    else if(gsGraDstZoomRatioN[GRA_DST_PIPE_1] >= gsGraDstZoomRatioM[GRA_DST_PIPE_1] * 6 / 5) {
                        //usLineDly = 0x60 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x60 ;
                    }
                    else {
                        //usLineDly = 0x30 ;
                        gsLineDly[GRA_DST_PIPE_1] = 0x30 ;
                    }
                }
                usLineDly = gsLineDly[GRA_DST_PIPE_1] ;
    		    MMPF_Scaler_SetPixelLineDelay(GRA_DST_PIPE_1, 0, usLineDly);
            }
            if (gbGRAEn[GRA_DST_PIPE_2]){
                MMPF_Display_ConfigGRADstScaler(GRA_DST_PIPE_2, cur_pipe[GRA_DST_PIPE_2], usCropWidth,usCropHeight,gsGraDstZoomRatioM[GRA_DST_PIPE_2]  );
           
                // h264 endpoint only
                if(pipe_cfg[GRA_DST_PIPE_2]==PIPE_CFG_H264) {
                    if(gsGraDstZoomRatioN[GRA_DST_PIPE_2] >= 2 * gsGraDstZoomRatioM[GRA_DST_PIPE_2]) {
                        //usLineDly = 0x88 ;
                        gsLineDly[GRA_DST_PIPE_2] =  (yuv_on) ? 0x98:0x88;
                    }
                    else if(gsGraDstZoomRatioN[GRA_DST_PIPE_2] >= gsGraDstZoomRatioM[GRA_DST_PIPE_2] * 6 / 5) {
                        //usLineDly = 0x80;// 0x60 ;
                        gsLineDly[GRA_DST_PIPE_2] =  (yuv_on) ? 0x80:0x60;
                    }
                    else {
                        //usLineDly = 0x30;
                        gsLineDly[GRA_DST_PIPE_2] = (yuv_on) ? 0x60:0x30;
                    }  
                    //
                    // When simulcast, H264 is up to 720p only.
                    // Reduce half of delay to tune up the performance to 30 fps
                    usLineDly = gsLineDly[GRA_DST_PIPE_2];// >> 1;
                    //dbg_printf(0,"usLineDly : %x\r\n",usLineDly);
                                    
                }          
                MMPF_Scaler_SetPixelLineDelay(GRA_DST_PIPE_2, 0, usLineDly);
			    
            }
            usPixlDly = VR_MAX(gsPixlDly[GRA_DST_PIPE_1],gsPixlDly[GRA_DST_PIPE_2]);
            //dbg_printf(3,"G21:%d\r\n",usb_vc_cur_usof());
            #if H264_SIMULCAST_EN
            {
            
                MMPF_GRAPHICS_RECT rect ;
                rect.usWidth           = src.usWidth;
                rect.usHeight          = src.usHeight;
                rect.usLeft            = 0;
                rect.usTop             = 0;
                MMPF_Graphics_SetScaleAttribute(&src, 0, MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE,
                                                &rect, 1, MMPF_GRAPHICS_SCAL_FB);
           
            }
            MMPF_Graphics_Scale_P0(src, dst, MMPF_Display_CallBackGRA, usPixlDly, 0);
            #else
            MMPF_Graphics_Scale(src, dst, MMPF_Display_CallBackGRA, usPixlDly, 0);
            #endif
        }	
        else {
           // dbg_printf(3,"OUT of Handle path\r\n");

        }
    }
#endif	//SUPPORT_GRA_ZOOM
	return MMP_ERR_NONE ; 
}


#endif

static void MMPF_Display_ConfigGRADstIbc(MMP_UBYTE usIBCPipe,MMP_UBYTE format)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    volatile AITPS_IBCP pIbcPipeCtl;

    if (usIBCPipe == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (usIBCPipe == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (usIBCPipe == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    pIbcPipeCtl->IBC_SRC_SEL &= ~(IBC_SRC_SEL_MASK|IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(usIBCPipe);
    switch(format) {
    case PIPE_CFG_H264:
        MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
        pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420 | IBC_420_STORE_CBR );
        pIbcPipeCtl->IBC_ADDR_Y_ST = glPreviewBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
        pIbcPipeCtl->IBC_ADDR_U_ST = glPreviewUBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
        pIbcPipeCtl->IBC_ADDR_V_ST = glPreviewVBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
        //dbg_printf(3,"ibc[%d].src_sel : %x\r\n",usIBCPipe,pIbcPipeCtl->IBC_SRC_SEL);
        
        break;
    case PIPE_CFG_MJPEG:
        //dbg_printf(3,"@JPG.loop(%d,%d)\r\n",dst_w,dst_h);
        pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
        pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
        pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_STORE_EN;
        pIBC->IBC_JPEG_PIPE_SEL = (pIBC->IBC_JPEG_PIPE_SEL & ~IBC_JPEG_SRC_SEL_MASK) | IBC_JPEG_SRC_SEL(usIBCPipe);
        MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_FALSE, MMPF_SCALER_COLRMTX_FULLRANGE);
        MMPF_IBC_SetInterruptEnable(usIBCPipe,MMPF_IBC_EVENT_FRM_RDY,MMP_FALSE);
        //pJPG->JPG_CTL |= JPG_ENC_EN; 
        break;
    case PIPE_CFG_YUY2:
        {
            MMP_ULONG buf_addr;
            buf_addr = (MMP_ULONG)MMPF_Video_CurWrPtr(usIBCPipe);
            pIbcPipeCtl->IBC_ADDR_Y_ST = buf_addr + FRAME_PAYLOAD_HEADER_SZ  ;
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_YUYV;
            MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
            //MMPF_IBC_SetInterruptEnable(usIBCPipe,MMPF_IBC_EVENT_FRM_RDY,MMP_TRUE);
            //dbg_printf(3,"yuy2.pipe0 :%x\r\n",buf_addr);
            break;
        }
    }
}

static MMP_USHORT MMPF_Display_ConfigGRADstScaler(MMP_UBYTE dst_pipe,STREAM_CFG *cur_pipe,MMP_USHORT usCropWidth,MMP_USHORT usCropHeight,MMP_USHORT usRatioM)
{
extern MMP_UBYTE gbCurH264EncId ;

    MMP_UBYTE pipe_cfg = 0xFF ;
    MMPF_SCALER_FIT_RANGE fitrange;
    MMPF_SCALER_GRABCONTROL grabctl;
    
    
    pipe_cfg = CUR_PIPE_CFG(dst_pipe,cur_pipe->pipe_cfg);
    
    fitrange.fitmode = MMPF_SCALER_FITMODE_OUT;

    fitrange.usFitResol  = usRatioM ; //gsZoomRatioM[dst_pipe];	
    fitrange.usInWidth   = usCropWidth;
    fitrange.usInHeight  = usCropHeight;
    
    fitrange.usOutWidth  = cur_pipe->pipe_w[dst_pipe];
    if(pipe_cfg==PIPE_CFG_H264) {
        fitrange.usOutHeight = ALIGN16(cur_pipe->pipe_h[dst_pipe] );
        fitrange.usOutWidth  = ALIGN16(cur_pipe->pipe_w[dst_pipe] );
        if( UVCX_IsSimulcastH264()  &&(gbCurH264EncId==1) ) {
            fitrange.usOutHeight = ALIGN16(gGRAStreamingCfg.frm_buf_h[GRA_STREAMING1]);
            fitrange.usOutWidth  = ALIGN16(gGRAStreamingCfg.frm_buf_w[GRA_STREAMING1]);
        }    
    }
    else if(pipe_cfg==PIPE_CFG_MJPEG) {
        fitrange.usOutHeight = ALIGN8(cur_pipe->pipe_h[dst_pipe] );
        fitrange.usOutWidth  = ALIGN16(cur_pipe->pipe_w[dst_pipe] );
    }
    else {
        fitrange.usOutHeight = cur_pipe->pipe_h[dst_pipe];
    }
    MMPF_Scaler_SetEngine(MMP_FALSE, dst_pipe, &fitrange, &grabctl);
    MMPF_Scaler_SetLPF(dst_pipe, &fitrange, &grabctl);
    return grabctl.usScaleN ;                    
}


static void MMPF_Display_ConfigGRASrcScaler(MMP_USHORT gra_src_w,MMP_USHORT gra_src_h)
{
    MMP_LONG            pan_off,tilt_off;
    MMPF_SCALER_FIT_RANGE fitrange; 
    MMPF_SCALER_GRABCONTROL grabctl;
    fitrange.fitmode = MMPF_SCALER_FITMODE_OUT ;
    #if 0
    {
        MMP_USHORT in_w = GRA_SRC_FIXED_SIZE?cur_pipe->pipe_w[GRA_SRC_PIPE]:gsSensorLCModeWidth ;
        MMP_USHORT in_h = GRA_SRC_FIXED_SIZE?cur_pipe->pipe_h[GRA_SRC_PIPE]:gsSensorLCModeHeight;
        MMP_USHORT target_w = ((cur_pipe->pipe_w[GRA_DST_PIPE_1] * gsZoomRatioM / gsZoomRatio + 1) >> 1) << 1;
        MMP_USHORT target_h = ((cur_pipe->pipe_h[GRA_DST_PIPE_1] * gsZoomRatioM / gsZoomRatio + 1) >> 1) << 1;
        MMP_USHORT rate;
                        
        rate = VR_MAX(64 * gra_src_w / target_w, 64 * gra_src_h / target_h);                    
        dbg_printf(3,"T:in_w,h, rate = %d,%d, %d\r\n",in_w,in_h, rate);
        fitrange.usInWidth   = in_w;
        fitrange.usInHeight  = in_h;
        fitrange.usOutWidth  = gra_src_w;//usCropWidth;//cur_pipe->pipe_w[0];
        fitrange.usOutHeight = gra_src_h;//usCropHeight;//cur_pipe->pipe_h[0];
        
        grabctl.usScaleN = rate;
        grabctl.usScaleM = 64;
        grabctl.usStartX= ( in_w - fitrange.usOutWidth) / 2 + 1 ;
        grabctl.usStartY= ( in_h -fitrange.usOutHeight) / 2 + 1 ;
        grabctl.usEndX=  grabctl.usStartX + fitrange.usOutWidth - 1 ;
        grabctl.usEndY=  grabctl.usStartY + fitrange.usOutHeight- 1 ;
        
        MMPF_Scaler_SetEngine(MMP_TRUE, GRA_SRC_PIPE, &fitrange, &grabctl);
        MMPF_Scaler_SetLPF(GRA_SRC_PIPE, &fitrange, &grabctl);
        dbg_printf(3, "IN(W,H)=(%d,%d) ,OUT(W,H)=(%d,%d)\r\n",fitrange.usInWidth,fitrange.usInHeight,fitrange.usOutWidth,fitrange.usOutHeight);    
    }
    #elif SUPPORT_GRA_ZOOM==GRA_ZOOM_BY_FOV// scaling down and grab
    {
    grabctl.usStartX= (cur_pipe->pipe_w[GRA_SRC_PIPE] - gra_src_w/*usCropWidth*/) / 2 + 1 ;
    grabctl.usStartY= (cur_pipe->pipe_h[GRA_SRC_PIPE] - gra_src_h/*usCropHeight*/) /2 + 1 ;
    grabctl.usEndX=  grabctl.usStartX + gra_src_w/*usCropWidth*/ - 1 ;
    grabctl.usEndY=  grabctl.usStartY + gra_src_h/*usCropHeight*/- 1 ;
    MMPF_Scaler_SetEngine(MMP_TRUE, GRA_SRC_PIPE, 0, &grabctl);
    }
    #else // bypass and grab
    {
        MMP_USHORT in_w ;
        MMP_USHORT in_h ;
        MMPF_SCALER_ZOOM_INFO       *scalinfo = MMPF_Scaler_GetCurInfo(GRA_SRC_PIPE) ;
        MMPF_SCALER_GRABCONTROL     tmpgrabCtl;
        //dbg_printf(3,"S:%d\r\n",usb_vc_cur_usof());
        #if SENSOR_IN_H > SENSOR_16_9_H
        in_w = gsSensorLCModeWidth  ;
        in_h = GetScalerRefInHeight() ;
        #else
        in_w = gsSensorLCModeWidth ;
        in_h = gsSensorLCModeHeight ;
        dbg_printf(GRA_DBG_LEVEL,"T:in_w,h = %d,%d\r\n",in_w,in_h);
        #endif        
        fitrange.usInWidth   = in_w;
        fitrange.usInHeight  = in_h;
        fitrange.usOutWidth  = gra_src_w;
        fitrange.usOutHeight = gra_src_h;
        grabctl.usScaleN = 64;
        grabctl.usScaleM = 64;
        grabctl.usStartX= ( in_w - fitrange.usOutWidth) / 2 + 1 ;
        grabctl.usStartY= ( in_h - fitrange.usOutHeight) / 2 + 1 ;
        grabctl.usEndX=  grabctl.usStartX + fitrange.usOutWidth - 1 ;
        grabctl.usEndY=  grabctl.usStartY + fitrange.usOutHeight- 1 ;
        scalinfo->usBaseN = grabctl.usScaleN ;
        scalinfo->grabCtl = grabctl ;
        MMPF_Scaler_SetEngine(MMP_TRUE, GRA_SRC_PIPE, &fitrange, &grabctl);
        MMPF_Scaler_SetLPF(GRA_SRC_PIPE, &fitrange, &grabctl);
        if(MMPF_Scaler_SetPanTiltParams(GRA_SRC_PIPE,0,0,0,&pan_off,&tilt_off)==MMP_ERR_NONE) {
            MMPF_SCALER_PANTILT_INFO *paninfo = MMPF_Scaler_GetCurPanTiltInfo(GRA_SRC_PIPE) ;
            
            if(pan_off || tilt_off) {
                tmpgrabCtl = paninfo->grabCtlIn;
                MMPF_Scaler_AccessGrabArea(MMP_TRUE,MMPF_SCALER_PATH_2,&tmpgrabCtl,0);
            } 
            else {
                if( (paninfo->usPanEnd==2 ) || (paninfo->usTiltEnd==2)) {
                    tmpgrabCtl =  paninfo->grabCtlIn; 
                    MMPF_Scaler_AccessGrabArea(MMP_TRUE,MMPF_SCALER_PATH_2,&tmpgrabCtl,0);
                }
            }
        }
        
        dbg_printf(GRA_DBG_LEVEL, "T:IN(W,H)=(%d,%d) ,OUT(W,H)=(%d,%d)\r\n",fitrange.usInWidth,fitrange.usInHeight,fitrange.usOutWidth,fitrange.usOutHeight);
    }
    #endif
    // dbg_printf(3, "P%d->GRA(%d, %d) -> (%d,%d)\r\n", GRA_SRC_PIPE,grabctl.usStartX, grabctl.usEndX,grabctl.usStartY, grabctl.usEndY);

    #if SUPPORT_CHANGE_AE_WINDOW
    // AE window change to GRA SRC pipe
    MMPF_Sensor_UpdateAEWindow((1 << GRA_SRC_PIPE),GRA_SRC_PIPE);
    #endif
}


static MMP_BOOL MMPF_Display_ReConfigGRASrcScaler(MMP_USHORT new_crop_w,MMP_USHORT new_crop_h)
{
    MMP_USHORT old_crop_w,old_crop_h ;
    
    MMPF_SCALER_GRABCONTROL grabout;
    MMPF_Scaler_AccessGrabArea(MMP_FALSE,MMPF_SCALER_PATH_2,0,&grabout);
    
    old_crop_w = grabout.usEndX - grabout.usStartX + 1 ;
    old_crop_h = grabout.usEndY - grabout.usStartY + 1 ;
    //dbg_printf(3,"---old crop (%d,%d)\r\n",old_crop_w,old_crop_h);
    if( (old_crop_w != new_crop_w) || (old_crop_h != new_crop_h) ) {
        return MMP_TRUE;
    }
    
    return MMP_FALSE ;
}

MMP_ERR MMPF_Display_SkipGraLoop(MMP_BOOL skip)
{
    gbSkipGraLoop = skip ;
    dbg_printf(0,"#Skip GraLoop:%d\r\n",gbSkipGraLoop);
    return MMP_ERR_NONE ;
}

MMP_SHORT gsSkipFrame ;
MMP_ERR MMPF_Display_SkipFrame(MMP_SHORT frame_n)
{
    dbg_printf(3,"#Skip Frame#:%d\r\n",frame_n);
    gsSkipFrame  = frame_n ;
    return MMP_ERR_NONE ; 
}


MMP_ERR MMPF_Display_WaitFrame(MMP_USHORT frame_n,MMP_UBYTE pipe)
{
    MMP_USHORT i;
    if(pipe==0) {
        pipe=1 ;
    } 
    else if (pipe==1){
        pipe=2;
    } 
    else {
    #if (CHIP==P_V2)
        pipe = 1;
    #endif
    #if (CHIP==VSN_V2)||(CHIP==VSN_V3)
        pipe = 3;
    #endif
    }
    for(i=0;i<frame_n;i++) {
        
        m_bStartPreviewFrameEndSig = pipe;
		if (MMPF_OS_AcquireSem(m_StartPreviewFrameEndSem, 500)) {
			RTNA_DBG_Str(0, "Wait IBC frame end :Fail\r\n");
			m_bStartPreviewFrameEndSig = 0;
			return MMP_DISPLAY_ERR_FRAME_END;
		}	
    }
    return MMP_ERR_NONE;
}


#if PCAM_EN==0
//------------------------------------------------------------------------------
//  Function    : MMPF_Display_SetRefresh
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_SetRefresh(MMPF_DISPLAY_CONTROLLER controller)
{
	AITPS_DSPY   pDSPY    = AITC_BASE_DSPY;
	AITPS_GBL    pGBL     = AITC_BASE_GBL;

    if (controller == MMPF_DISPLAY_SCD_CONTROL) {
		
    	switch ((pDSPY->DSPY_CTL_2 & DSPY_SCD_SEL_MASK) >> DSPY_SCD_SEL_SHIFT) {
    	case DSPY_TYPE_PL_LCD:
    		if (!(pDSPY->DSPY_CTL_0 & LCD_BUSY_STS)) {
				pDSPY->DSPY_CTL_0 |= SCD_DSPY_REG_READY;
				pDSPY->DSPY_SCD_CTL |= LCD_FRAME_TX;
    		} 
    		else {
		    	return	MMP_DISPLAY_ERR_LCD_BUSY;
			}
			break;
    	case DSPY_TYPE_SPI_LCD:
    		if (!(pDSPY->DSPY_CTL_0 & LCD_BUSY_STS)) {
				pDSPY->DSPY_CTL_0 |= SCD_DSPY_REG_READY;
				pDSPY->DSPY_SCD_CTL |= LCD_FRAME_TX;
    		}
    		else {
		    	return	MMP_DISPLAY_ERR_LCD_BUSY;
			}
			break;
    	case DSPY_TYPE_RGB_LCD:
    	case DSPY_TYPE_TV:
    		pDSPY->DSPY_CTL_0 |= SCD_DSPY_REG_READY;
    		break;
		}
		
    }
    else {
        if(pGBL->GBL_CCIR_PIN_CTL & GBL_CCIR_OUTPUT){
            #if (CHIP == P_V2)
            AITPS_CCIR  pCCIR = AITC_BASE_CCIR;
            if(!(pCCIR->CCIR_CAPTURE_FRAME_EN & CCIR_PREVIEW_FRAME_ST)){
                pCCIR->CCIR_CAPTURE_FRAME_EN |= CCIR_PREVIEW_FRAME_ST;
            } else {
                DBG_S3("CCIR is busy\r\n");
                return	MMP_DISPLAY_ERR_LCD_BUSY;
            }
            #endif
        } else {
    	switch ((pDSPY->DSPY_CTL_2 & DSPY_PRM_SEL_MASK) >> DSPY_PRM_SEL_SHIFT) {
    	case DSPY_TYPE_PL_LCD:
    		if (!(pDSPY->DSPY_CTL_0 & LCD_BUSY_STS)) {
				pDSPY->DSPY_CTL_2 |= PRM_DSPY_REG_READY;
				pDSPY->DSPY_CTL_0 |= LCD_FRAME_TX;
    		}
    		else {
		    	return	MMP_DISPLAY_ERR_LCD_BUSY;
			}
			break;
    	case DSPY_TYPE_SPI_LCD:
    		if (!(pDSPY->DSPY_CTL_0 & LCD_BUSY_STS)) {
				pDSPY->DSPY_CTL_2 |= PRM_DSPY_REG_READY;
				pDSPY->DSPY_CTL_0 |= LCD_FRAME_TX;
    		}
    		else {
		    	return	MMP_DISPLAY_ERR_LCD_BUSY;
			}
			break;
    	case DSPY_TYPE_RGB_LCD:
    	case DSPY_TYPE_TV:
    		pDSPY->DSPY_CTL_2 |= PRM_DSPY_REG_READY;
    		break;
		}
		}    		
    }
    return MMP_ERR_NONE;
}
#endif // PCAM_EN==0

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_StartPreview
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_StartPreview(MMP_USHORT usIBCPipe, MMP_BOOL bCheckFrameEnd)
{
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    volatile AITPS_IBCP  pIbcPipeCtl;
    MMP_BOOL    bVifEn = MMP_FALSE;

	gsPreviewPath |= (1 << usIBCPipe);
	
    if (usIBCPipe == 0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (usIBCPipe == 1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
	
    
    m_bReceiveStopPreviewSig[usIBCPipe] = MMP_FALSE;

    dbg_printf(3,"MMPF_Display_StartPreview : %d,%d\r\n",usIBCPipe,gbPreviewBufferCount[usIBCPipe]);
    
    if (gbPreviewBufferCount[usIBCPipe]) {
        MMPF_Display_StartSoftwareRefresh(usIBCPipe); 
        pIbcPipeCtl->IBC_ADDR_Y_ST = glPreviewBufAddr[usIBCPipe][0];
    	pIbcPipeCtl->IBC_ADDR_U_ST = glPreviewUBufAddr[usIBCPipe][0];
        pIbcPipeCtl->IBC_ADDR_V_ST = glPreviewVBufAddr[usIBCPipe][0];
        MMPF_IBC_SetInterruptEnable(usIBCPipe, MMPF_IBC_EVENT_FRM_RDY, MMP_TRUE);
        MMPF_IBC_SetInterruptEnable(usIBCPipe, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
#if USB_SPEEDUP_PREVIEW_TIME==0
        MMPF_IBC_SetStoreEnable(usIBCPipe, MMP_TRUE);
#endif        
    }
    else {
        MMPF_IBC_SetInterruptEnable(usIBCPipe, MMPF_IBC_EVENT_FRM_END, MMP_TRUE);
    }
    
	#if (SENSOR_EN)
	gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_TRUE);
	#endif		

	// Enable VIF input
	MMPF_VIF_IsInterfaceEnable (&bVifEn);
	if (bVifEn == MMP_FALSE) {
	    MMPF_VIF_EnableInputInterface(MMP_TRUE);
	}

    #if 0
	if (bCheckFrameEnd) {
		m_bStartPreviewFrameEndSig = MMP_TRUE;
		if (MMPF_OS_AcquireSem(m_StartPreviewFrameEndSem, 10000)) {
			RTNA_DBG_Str(0, "Wait IBC frame end :Fail\r\n");
			return MMP_DISPLAY_ERR_FRAME_END;
		}	
	}
    #endif
	
    return MMP_ERR_NONE;
 }

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_StopPreview
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_StopPreview(MMP_USHORT usIBCPipe)
{
#define STOP_PREVIEW_TIMEOUT 0//1000
	AITPS_IBC   pIBC    = AITC_BASE_IBC;
    dbg_printf(3,"stoppreview,gsPreviewPath : %x\r\n",gsPreviewPath);
	if (gsPreviewPath == 0)
		return MMP_ERR_NONE;

    if (usIBCPipe == 0) {
        pIBC->IBC_P0_INT_CPU_EN &= ~IBC_INT_FRM_RDY;
    }
    else if (usIBCPipe == 1) {
        pIBC->IBC_P1_INT_CPU_EN &= ~IBC_INT_FRM_RDY;
    }
    else  {
        pIBC->IBC_P2_INT_CPU_EN &= ~IBC_INT_FRM_RDY;
    }
    gsPreviewPath &= ~(1 << usIBCPipe);
    
    if (m_bReceiveStopPreviewSig[usIBCPipe] == MMP_TRUE)
		return MMP_ERR_NONE;
    else {
        MMP_UBYTE ret ;
        dbg_printf(3,"#Waiting preview end,pipe:%d,gsPreviewPath:%x\r\n",usIBCPipe,gsPreviewPath);
        m_bReceiveStopPreviewSig[usIBCPipe] = MMP_TRUE;
        ret = MMPF_OS_AcquireSem(m_PreviewControlSem, STOP_PREVIEW_TIMEOUT);
        if(ret==OS_TIMEOUT) {
            dbg_printf(3,"< stop 1 secs >\r\n");
            m_bWaitVIFEndSig = MMP_TRUE ;
            ret = MMPF_OS_AcquireSem(m_PreviewControlSem, STOP_PREVIEW_TIMEOUT);
            if(ret==OS_TIMEOUT) {
                dbg_printf(3,"< stop 1 secs again>\r\n");
            }
        }
        dbg_printf(3,"#Wait end:%d\r\n",usIBCPipe);
	}
	
#if INIT_SCALE_PATH_EN==0
{
    MMP_UBYTE grapath ;
    STREAM_CFG *stream_cfg = usb_get_cur_image_pipe(usIBCPipe) ;
    int workss = MMPF_Video_GetWorkingStreamSession();
    if(0/* !IS_FIRST_STREAM(workss)*/ ) {
        grapath = MMPF_Video_IsLoop2GRA(stream_cfg) ;   
        if(grapath & (0x02 | 0x04) ) {
            MMPF_Scaler_SwitchPath(MMP_TRUE, usIBCPipe,MMPF_SCALER_SOURCE_ISP,MMP_TRUE); 
            dbg_printf(0,"GRA status:%x\r\n",grapath);   
        } else {     
            MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(usIBCPipe)->scalerpath);
        }
    } else {
        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(usIBCPipe)->scalerpath);
    }
}
#endif
	//RTNA_DBG_Str(0, "=== MMPF_Display_StopPreview end ===\r\n");

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_StartSoftwareRefresh
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_StartSoftwareRefresh(MMP_USHORT usIBCPipe)
{
	// even odd toggle bit
	if (gbPreviewBufferCount[usIBCPipe]) {
	    gbExposureDoneFrame[usIBCPipe] = gbPreviewBufferCount[usIBCPipe] - 1;
	}
	else {
	    gbExposureDoneFrame[usIBCPipe] = 0;
	}
	
	gbCurIBCBuf[usIBCPipe] = 0;

	return MMP_ERR_NONE;
}
#if LCD_DISPLAY_ON==1

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_StartFLMRefresh
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_StartFLMRefresh(void)
{
	AITPS_GPIO	pGPIO   = AITC_BASE_GPIO;
    AITPS_AIC   pAIC 	= AITC_BASE_AIC;
    
	MMP_ULONG	gpio_shift, gpio_group;

    gbExposureDone = 0;

#if defined(TARGET_ID_170201)
   	ait81x_tc_open(&TC2_DESC, TC_CPCTRG | TC_CLKS_MCK8);
	ait81x_tc_write(&TC2_DESC, RC, 21000);

	ait81x_irq_open(TC2_DESC.periph_id, AIC_DIRECT_TO_IRQ, 0,
					AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE,timer2_asm_irq_handler);
#endif

	// Enable FLM <-> GPIO interrupt.
	gpio_shift = FLM_GPIO_NUM;
	gpio_group = 0;
	while (gpio_shift >= 32) {
		gpio_shift -= 32;
		gpio_group += 1;
	}
			
    pGPIO->GPIO_EN[gpio_group] &= ~(1 << gpio_shift);
    pGPIO->GPIO_INT_L2H_EN[gpio_group] |= (1 << gpio_shift);
   	pGPIO->GPIO_INT_H2L_EN[gpio_group] &= ~(1 << gpio_shift);

    pGPIO->GPIO_INT_L2H_SR[gpio_group] = (1 << gpio_shift);
   	pGPIO->GPIO_INT_CPU_EN[gpio_group] |= (1 << gpio_shift);

    RTNA_AIC_Open(pAIC, AIC_SRC_GPIO, gpio_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
	RTNA_AIC_IRQ_En(pAIC, AIC_SRC_GPIO);

    // clear exposure done flag
    gbExposureDone = 0;

	return	MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_StopFLMRefresh
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_StopFLMRefresh(void)
{
    AITPS_AIC   pAIC 	= AITC_BASE_AIC;

#if defined(TARGET_ID_170201)
	ait81x_irq_close(TC2_DESC.periph_id);
	ait81x_tc_close(&TC2_DESC);
#endif

	RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_GPIO);

	// Check if LCD is busy.
	return	MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPF_Display_FrameDoneTrigger
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Display_FrameDoneTrigger(MMP_USHORT usIBCPipe)
{
static char __osd_str[12] ;
static int __osd_fnum ;

    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    volatile AITPS_IBCP pIbcPipeCtl;
    MMP_USHORT encid = 0 ;
    
    if (usIBCPipe == 0) { 
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (usIBCPipe == 1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
     
	if (gIBCLinkType[usIBCPipe] & MMPF_IBC_LINK_VIDEO) {
	   // dbg_printf(3,"H0_s : %d\r\n",usb_vc_cur_usof());
	    if(pcam_Osd_IsEnable()) {
	        //_sprintf(__osd_str,"%d",__osd_fnum);
	        pcam_Osd_SetPos(40,120);
	        
	        //pcam_Osd_CopySrcBuf(0,BACKUP_IMAGE,pIbcPipeCtl->IBC_ADDR_Y_ST,pIbcPipeCtl->IBC_ADDR_U_ST);
	        pcam_Osd_DrawBuf(0,pIbcPipeCtl->IBC_ADDR_Y_ST,pIbcPipeCtl->IBC_ADDR_U_ST);
	        
	        //pcam_Osd_DrawStr(__osd_str, _strlen(__osd_str),
	        //                            pIbcPipeCtl->IBC_ADDR_Y_ST,
	        //                            pIbcPipeCtl->IBC_ADDR_U_ST);
	        
	        // __osd_fnum++;
	    }
	    //dbg_printf(3,"WDone.Y : %x\r\n",pIbcPipeCtl->IBC_ADDR_Y_ST);
        MMPF_VIDENC_TriggerFrameDone(gbIBCLinkEncId[usIBCPipe], //main enc trig ,orig = 0 
                                    &(gbExposureDoneFrame[usIBCPipe]),
                                    &(gbCurIBCBuf[usIBCPipe]),
                                    gbPreviewBufferCount[usIBCPipe],
                                    glPreviewBufAddr[usIBCPipe],
                                    glPreviewUBufAddr[usIBCPipe],
                                    glPreviewVBufAddr[usIBCPipe]);
                                    
        #if H264_SIMULCAST_EN==1
        if( UVCX_IsSimulcastH264()  ) {
            gGRAStreamingCfg.work_streaming = GRA_STREAMING0 ;
            if ( MMPF_VIDENC_GetTotalPipeNum() > 1) {
                glGRASrcYAddr = pIbcPipeCtl->IBC_ADDR_Y_ST ;
                glGRASrcUAddr = pIbcPipeCtl->IBC_ADDR_U_ST ;
                glGRASrcVAddr = pIbcPipeCtl->IBC_ADDR_V_ST ;
                MMPF_Display_Simulcast();
            }
        }
        #endif
                                   
	}
	else {
    	gbExposureDoneFrame[usIBCPipe] += 1;
    	if (gbExposureDoneFrame[usIBCPipe] >= gbPreviewBufferCount[usIBCPipe]) {
    		gbExposureDoneFrame[usIBCPipe] = 0;
        }
    	gbCurIBCBuf[usIBCPipe] += 1;
    	if (gbCurIBCBuf[usIBCPipe] >= gbPreviewBufferCount[usIBCPipe]) {
    		gbCurIBCBuf[usIBCPipe] = 0;
        }
    }
    #if PCAM_EN==0
    if (gIBCLinkType[usIBCPipe] & MMPF_IBC_LINK_ROTATE) {
        gsIBCtoDMAPipe = usIBCPipe;
        if (gbInFDTCMode && gbFWDrawFaceRect) {
            MMPF_FDTC_SetCurPreviewBufAddr(glPreviewBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
            				               glPreviewUBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
            				               glPreviewVBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_FDTC_DRAWRECT, MMPF_OS_FLAG_SET);
        }
        else
        // set os flag to sensor task, which is responsible to operate rotate DMA
        MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_ROTDMA, MMPF_OS_FLAG_SET);
    }
	if (gIBCLinkType[usIBCPipe] & MMPF_IBC_LINK_DISPLAY) {
        if (gbInFDTCMode && gbFWDrawFaceRect) {
            MMPF_FDTC_SetCurPreviewBufAddr(glPreviewBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
            				               glPreviewUBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
            				               glPreviewVBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
            MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_FDTC_DRAWRECT, MMPF_OS_FLAG_SET);
        }
        else
       	if (gPreviewMode[usIBCPipe] == MMPF_DISPLAY_P_LCD || 
       		gPreviewMode[usIBCPipe] == MMPF_DISPLAY_TV || 
       		gPreviewMode[usIBCPipe] == MMPF_DISPLAY_RGB_LCD ||
       		gPreviewMode[usIBCPipe] == MMPF_DISPLAY_HDMI ||
       		gPreviewMode[usIBCPipe] == MMPF_DISPLAY_CCIR) {
    		if (!(pDSPY->DSPY_CTL_0 & LCD_BUSY_STS) || gPreviewMode[usIBCPipe] == MMPF_DISPLAY_CCIR) {
	        	MMPF_Display_SetWinAddr(gPreviewWinID[usIBCPipe], 
       					glPreviewBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
    					glPreviewUBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]],
   						glPreviewVBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
				MMPF_Display_SetWinActive(gPreviewWinID[usIBCPipe], MMP_TRUE);	        						

   	        	if (gPreviewWinID[usIBCPipe] == MMPF_DISPLAY_WIN_SCD_WIN) {
   		    	    MMPF_Display_SetRefresh(MMPF_DISPLAY_SCD_CONTROL);
        	    }
               	else {
   	    		    MMPF_Display_SetRefresh(MMPF_DISPLAY_PRM_CONTROL);
           	    }
	        }
        }
	   	else if (gPreviewMode[usIBCPipe] == MMPF_DISPLAY_FLM) {
    		gbExposureDone = 1;
		}
	}
    
    if (gIBCLinkType[usIBCPipe] & MMPF_IBC_LINK_GRAPHIC) {
        MMPF_GRAPHICS_BUFATTRIBUTE src, dst;

        dst.ulBaseAddr = (dst.ulBaseUAddr = (dst.ulBaseVAddr = 0));
        src.ulBaseAddr = glPreviewBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]];
        src.ulBaseUAddr = (src.ulBaseVAddr = 0);

        if (pIbcPipeCtl->IBC_SRC_SEL & IBC_SRC_YUV420) {
            src.colordepth = MMPF_GRAPHICS_COLORDEPTH_YUV420;
            src.ulBaseUAddr = glPreviewUBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]];
            src.ulBaseVAddr = glPreviewVBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]];
        }
        else {
            src.colordepth = MMPF_GRAPHICS_COLORDEPTH_YUV422;
        }
        MMPF_Graphics_Scale(src, dst, NULL);
    }
    #endif // PCAM_EN==0
    
    if( UVCX_IsSimulcastH264()  ) {
        gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0] += 1 ;
        if (gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0]>= gbPreviewBufferCount[usIBCPipe]) {
            gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0] = 0;
        }
     	pIbcPipeCtl->IBC_ADDR_Y_ST = glPreviewBufAddr[usIBCPipe ][gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0]];
    	pIbcPipeCtl->IBC_ADDR_U_ST = glPreviewUBufAddr[usIBCPipe][gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0]];
    	pIbcPipeCtl->IBC_ADDR_V_ST = glPreviewVBufAddr[usIBCPipe][gGRAStreamingCfg.frm_buf_index[GRA_STREAMING0]];
   
    } else {

    	pIbcPipeCtl->IBC_ADDR_Y_ST = glPreviewBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
    	pIbcPipeCtl->IBC_ADDR_U_ST = glPreviewUBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
    	pIbcPipeCtl->IBC_ADDR_V_ST = glPreviewVBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
	}
	
    return MMP_ERR_NONE;
}

MMPF_GRAPHICS_BUFATTRIBUTE  m_GraSrcBufAttr = {
    0, 0,   //width, height
    0,      //line offset
    MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE,
    0, 0, 0 //YUV buf addr
};
MMPF_GRAPHICS_RECT  m_GraRect = {
    0, 0,   //top, left
    0, 0    //width, height
};

void MMPF_Display_CallBackGRA_P0(void);

void MMPF_Display_CallBackGRA_P0(void)
{
static MMP_ULONG enc_jpg_time = 0 ;
    MMP_UBYTE ws = gGRAStreamingCfg.work_streaming ;
    MMP_UBYTE format = gGRAStreamingCfg.frm_buf_format[ws] ;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    MMP_UBYTE usIBCPipe = 0 ; // pipe 0 
   // dbg_printf(3,"#GRA.done:%d\r\n",format);
    volatile AITPS_IBCP pIbcPipeCtl = &(pIBC->IBCP_0);
    switch(format) {
        case PIPE_CFG_H264:
	        //dbg_printf(3,"IBC%d(%x,%x,%x)\r\n",usIBCPipe,pIbcPipeCtl->IBC_ADDR_Y_ST,pIbcPipeCtl->IBC_ADDR_U_ST,pIbcPipeCtl->IBC_ADDR_V_ST);
	        //dbg_printf(3,"G10eh:%d\r\n",usb_vc_cur_usof());
            MMPF_VIDENC_TriggerFrameDone(gbIBCLinkEncId[usIBCPipe], //main enc trig ,orig = 0 
                                    &(gbExposureDoneFrame[usIBCPipe]),
                                    &(gbCurIBCBuf[usIBCPipe]),
                                    gbPreviewBufferCount[usIBCPipe],
                                    glPreviewBufAddr[usIBCPipe],
                                    glPreviewUBufAddr[usIBCPipe],
                                    glPreviewVBufAddr[usIBCPipe]);
                                    
            //dbg_printf(3,"s%d->%d\r\n",gGRAStreamingCfg.work_streaming,gGRAStreamingCfg.frm_buf_index[gGRAStreamingCfg.work_streaming] );
            break;
        case PIPE_CFG_YUY2:
        {
        
            //dbg_printf(3,"G10ey:%d\r\n",usb_vc_cur_usof());
            break;
        }
        case PIPE_CFG_MJPEG:
        {
            MMP_ULONG dt ; 
            //dbg_printf(3,"JPG gra.done\r\n");
            //dbg_printf(3,"G10ej:%d\r\n",usb_vc_cur_usof());
            dt = usb_vc_cur_usof() - enc_jpg_time ;
            //dbg_printf(3,"G10ej:%d\r\n",dt);
            enc_jpg_time = usb_vc_cur_usof() ;
        }
        break;
    }
    MMPF_Display_Simulcast();
}


void MMPF_Display_LoobackFrame(MMPF_IBC_PIPEID SrcPipe, MMPF_IBC_PIPEID DstPipe,
                               MMP_USHORT usSrcWidth, MMP_USHORT usSrcHeight, 
                               MMP_USHORT usWidth, MMP_USHORT usHeight,
                               MMP_ULONG  ulSrcYAddr,
                               MMP_ULONG  ulSrcUAddr,
                               MMP_ULONG  ulSrcVAddr,
                               MMPF_GRAPHICS_RECT *Rect,
                               GraphicRotDMACallBackFunc *GraEndCallback)
{
    MMP_ERR     ret;
    MMP_USHORT  usPxlDly = 2 , usLineDly = 0;
    MMPF_GRAPHICS_BUFATTRIBUTE dummy;
    MMPF_SCALER_PATH        scalp = (MMPF_SCALER_PATH)DstPipe; //TBD
    MMPF_SCALER_FIT_RANGE   ftrge;
    MMPF_SCALER_GRABCONTROL grctl;
    MMP_UBYTE ws = gGRAStreamingCfg.work_streaming ;
    MMP_UBYTE format = gGRAStreamingCfg.frm_buf_format[ws] ;
    
    MMP_USHORT ratio_x ,ratio_y ,ratio;
    
    #if 0
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
	volatile AITPS_IBCP pIbcPipeCtl;

    if (SrcPipe == MMPF_IBC_PIPE_0) {
        pIbcPipeCtl = &(pIBC->IBCP_0);
    }
    else if (SrcPipe == MMPF_IBC_PIPE_1) {
        pIbcPipeCtl = &(pIBC->IBCP_1);
    }
    else if (SrcPipe == MMPF_IBC_PIPE_2) {
        pIbcPipeCtl = &(pIBC->IBCP_2);
    }
    #endif
    m_GraSrcBufAttr.usWidth     = usSrcWidth;//gsPreviewBufWidth[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.usHeight    = usSrcHeight;//gsPreviewBufHeight[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.usLineOffset = m_GraSrcBufAttr.usWidth;
    m_GraSrcBufAttr.colordepth  = MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE;
    m_GraSrcBufAttr.ulBaseAddr  = ulSrcYAddr;//glPreviewBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.ulBaseUAddr = ulSrcUAddr;//glPreviewUBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    m_GraSrcBufAttr.ulBaseVAddr = ulSrcVAddr;//glPreviewVBufAddr[SrcPipe][gbExposureDoneFrame[SrcPipe]];
    
    
    #if 0
    dbg_printf(3,"[Y,U,V] = [%x,%x,%x] -- [%x,%x,%x]\r\n",m_GraSrcBufAttr.ulBaseAddr,m_GraSrcBufAttr.ulBaseUAddr,m_GraSrcBufAttr.ulBaseVAddr,
                                                         pIbcPipeCtl->IBC_ADDR_Y_ST,pIbcPipeCtl->IBC_ADDR_U_ST,pIbcPipeCtl->IBC_ADDR_V_ST);
    
    #endif
    
    if (Rect == NULL) {
        m_GraRect.usWidth           = m_GraSrcBufAttr.usWidth;
        m_GraRect.usHeight          = m_GraSrcBufAttr.usHeight;
        m_GraRect.usLeft            = 0;
        m_GraRect.usTop             = 0;
    }
    else {
        m_GraRect = *Rect;
    }
    //dbg_printf(3,"GRA(%x,%x,%x)\r\n",m_GraSrcBufAttr.ulBaseAddr,m_GraSrcBufAttr.ulBaseUAddr,m_GraSrcBufAttr.ulBaseVAddr);

    ret = MMPF_Graphics_SetScaleAttribute (&m_GraSrcBufAttr, 0, MMPF_GRAPHICS_COLORDEPTH_YUV420_INTERLEAVE,
                                        &m_GraRect, 1, MMPF_GRAPHICS_SCAL_FB);
    if (ret != MMP_ERR_NONE) {
        return;
    }
    dummy.ulBaseAddr = 0;

    
    ftrge.fitmode   = MMPF_SCALER_FITMODE_OUT;
    ftrge.usInWidth = m_GraRect.usWidth;
    ftrge.usInHeight = m_GraRect.usHeight;
    ftrge.usOutWidth = usWidth;
    ftrge.usOutHeight = usHeight;
    ratio_x = (ftrge.usInWidth  * 10 / ftrge.usOutWidth   ) ;
    ratio_y = (ftrge.usInHeight * 10 / ftrge.usOutHeight  )  ;
    
    ratio = VR_MAX(ratio_x,ratio_y);
    if(ratio < 10 ) {
        ftrge.usFitResol = (120 * ratio) / 10 ;
    }
    else {
        ftrge.usFitResol = 1200 / ratio;
	}
	
    MMPF_Scaler_SetEngine(MMP_FALSE, scalp, &ftrge, &grctl);
    MMPF_Scaler_SetLPF(scalp, &ftrge, &grctl);   
	//dbg_printf(3,"N/M:%d/%d\r\n",grctl.usScaleN,grctl.usScaleM);
    if (grctl.usScaleN >= grctl.usScaleM) {
        switch(format) {
        case PIPE_CFG_MJPEG:
        
            if(grctl.usScaleN >= 2 * grctl.usScaleM) {
                usLineDly = 0x50;//0x40 ;
            }
            else {
                usLineDly = 0x20;// JPEG 720p -> 1080p : 20.25 ms
            }
            break ;
        case PIPE_CFG_H264:
            if(grctl.usScaleN >= 2 * grctl.usScaleM) {
                usLineDly = 0x88 ;
            }
            else if(grctl.usScaleN >=  grctl.usScaleM * 6 / 5) { 
                usLineDly = 0x60 ;
            }
            else {
                usLineDly = 0x30 ;
            }
            break ;
            
        case PIPE_CFG_YUY2:
            usLineDly = 0xc0 ;
            break ;
        
        }
        usPxlDly = 2 ;
    } else {
        usPxlDly  = 2 ;
        usLineDly = 0 ;
    }
    MMPF_Scaler_SetPixelLineDelay(scalp, 0, usLineDly);
    //dbg_printf(3,"G10:%d\r\n",usb_vc_cur_usof());
    ret = MMPF_Graphics_Scale_P0(m_GraSrcBufAttr, dummy, MMPF_Display_CallBackGRA_P0, usPxlDly, 0xFFFF);
}


void MMPF_Display_Simulcast()
{
//extern MMP_ULONG glGraphicCropWidth,glGraphicCropHeight;
//extern MMP_ULONG glGraphicYAddr,glGraphicUAddr,glGraphicVAddr;

    MMP_ULONG  src_yaddr ,src_uaddr,src_vaddr ; 
    MMP_USHORT dst_w,dst_h,src_w,src_h ;
    MMPF_H264ENC_ENC_INFO   *pEnc = MMPF_H264ENC_GetHandle(1); // enc id = 1 for 2nd stream
    MMP_UBYTE format ;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    MMP_UBYTE usIBCPipe = 0 ,src_pipe,dst_pipe;
    volatile AITPS_IBCP pIbcPipeCtl = &(pIBC->IBCP_0);
    
    if(gGRAStreamingCfg.gra_loop_state==GRA_LOOP_STOP) {
        return ;
    }
    
	gGRAStreamingCfg.work_streaming++;
	if(gGRAStreamingCfg.work_streaming >= gGRAStreamingCfg.streaming_num) {
		gGRAStreamingCfg.work_streaming = GRA_STREAMING0;
		if(gGRAStreamingCfg.gra_loop_stop) {
    		gGRAStreamingCfg.gra_loop_state = GRA_LOOP_STOP ;
    		//gGRAStreamingCfg.gra_loop_stop = MMP_FALSE ;
    		dbg_printf(3,"--stop sim/h264 loop\r\n");
		}
		//DBG_S(3, "S End\r\n");
		return ;
	}	
	
    dst_w  = gGRAStreamingCfg.frm_buf_w[gGRAStreamingCfg.work_streaming] ; 
    dst_h  = gGRAStreamingCfg.frm_buf_h[gGRAStreamingCfg.work_streaming] ;
    format = gGRAStreamingCfg.frm_buf_format[gGRAStreamingCfg.work_streaming] ;
    if((format==PIPE_CFG_MJPEG)&&(UVCX_GetH264EncodeBufferMode()!=FRAME_MODE)) {
       // dbg_printf(3,"jpg in rt mode\r\n");
        gGRAStreamingCfg.work_streaming = GRA_STREAMING0;
        return ;
    }

    MMPF_IBC_SetStoreEnable(usIBCPipe,MMP_TRUE);

    pIbcPipeCtl->IBC_SRC_SEL &= ~(IBC_SRC_SEL_MASK|IBC_SRC_YUV420|IBC_420_STORE_CBR|IBC_NV12_EN);
    pIbcPipeCtl->IBC_SRC_SEL |= IBC_SRC_SEL_ICO(usIBCPipe);

    switch(format) {
        case PIPE_CFG_H264:
        {
            MMP_BOOL ibc_en[3] ;
            MMPF_IBC_Reset(ibc_en);
            MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
            pIbcPipeCtl->IBC_SRC_SEL |= (IBC_SRC_YUV420 | IBC_420_STORE_CBR | IBC_NV12_EN);
            pIbcPipeCtl->IBC_ADDR_Y_ST = glPreviewBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
            pIbcPipeCtl->IBC_ADDR_U_ST = glPreviewUBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
            pIbcPipeCtl->IBC_ADDR_V_ST = glPreviewVBufAddr[usIBCPipe][gbCurIBCBuf[usIBCPipe]];
            MMPF_IBC_SetInterruptEnable(usIBCPipe,MMPF_IBC_EVENT_FRM_RDY,MMP_FALSE);
            MMPF_IBC_Restore(ibc_en);
            //dbg_printf(3,"G10Hs:%d\r\n",usb_vc_cur_usof());
            break;
        }    
        case PIPE_CFG_MJPEG:
        {
            AITPS_JPG pJPG = AITC_BASE_JPG ;
            //dbg_printf(3,"@JPG.loop(%d,%d)\r\n",dst_w,dst_h);
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_UYVY;
            pIbcPipeCtl->IBC_BUF_CFG &= ~IBC_STORE_EN;
            pIBC->IBC_JPEG_PIPE_SEL = (pIBC->IBC_JPEG_PIPE_SEL & ~IBC_JPEG_SRC_SEL_MASK) | IBC_JPEG_SRC_SEL(usIBCPipe);
            MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_FALSE, MMPF_SCALER_COLRMTX_FULLRANGE);
            MMPF_IBC_SetInterruptEnable(usIBCPipe,MMPF_IBC_EVENT_FRM_RDY,MMP_FALSE);
            MMPF_Video_EnableJpeg(); 
            //dbg_printf(3,"G10Js:%d\r\n",usb_vc_cur_usof());
            break;
        }
        case PIPE_CFG_YUY2:
        {
            MMP_ULONG buf_addr;
            buf_addr = (MMP_ULONG)MMPF_Video_CurWrPtr(usIBCPipe);
            pIbcPipeCtl->IBC_ADDR_Y_ST = buf_addr + FRAME_PAYLOAD_HEADER_SZ  ;
            pIbcPipeCtl->IBC_SRC_SEL &= ~IBC_SRC_YUV420;
            pIbcPipeCtl->IBC_SRC_SEL = (pIbcPipeCtl->IBC_SRC_SEL & ~IBC_422_SEQ_MASK) | IBC_422_SEQ_YUYV;
            MMPF_Scaler_SetOutputColor(usIBCPipe, MMP_TRUE, MMPF_SCALER_COLRMTX_BT601);
            MMPF_IBC_SetInterruptEnable(usIBCPipe,MMPF_IBC_EVENT_FRM_RDY,MMP_TRUE);
            //dbg_printf(3,"yuy2.pipe0 :%x\r\n",buf_addr);
            break;
        
        }
    }
    src_pipe = MMPF_IBC_PIPE_1 ;
    dst_pipe = usIBCPipe ;
    src_w = gsPreviewBufWidth[MMPF_IBC_PIPE_1][gbExposureDoneFrame[MMPF_IBC_PIPE_1]];
    src_h = gsPreviewBufHeight[MMPF_IBC_PIPE_1][gbExposureDoneFrame[MMPF_IBC_PIPE_1]];
	src_yaddr = glGRASrcYAddr;
	src_uaddr = glGRASrcUAddr;
	src_vaddr = glGRASrcVAddr;
   // dbg_printf(3,"[%d,%d]->[%d,%d][y,u,v]:[%x,%x,%x]\r\n", src_w,src_h,dst_w, dst_h,src_yaddr ,src_uaddr ,src_vaddr );
    MMPF_Display_LoobackFrame(src_pipe, dst_pipe ,src_w,src_h,dst_w, dst_h, src_yaddr, src_uaddr,src_vaddr,NULL, NULL);
}

void MMPF_Display_PauseSimulcast(void)
{
    if(0/*UVCX_IsSimulcastH264()*/) {
        if(gGRAStreamingCfg.gra_loop_stop==MMP_FALSE) {
            if(gGRAStreamingCfg.gra_loop_state == GRA_LOOP_RUN) {
                gGRAStreamingCfg.gra_loop_stop= MMP_TRUE ;    
                dbg_printf(3,"S:Pause:%d\r\n",gGRAStreamingCfg.gra_loop_state );
            }
        }
    }
}


void MMPF_Display_ResumeSimulcast(void)
{
extern MMP_BOOL gbSimulcastH264WaitEncDone ;
    if(0/*UVCX_IsSimulcastH264()*/) {

        if(gGRAStreamingCfg.gra_loop_state == GRA_LOOP_STOP) {
            //MMPF_OS_Sleep(20*1000);
            gGRAStreamingCfg.gra_loop_state= GRA_LOOP_RUN ;    
            gGRAStreamingCfg.gra_loop_stop= MMP_FALSE ; 
            dbg_printf(3,"S:Resume:%d\r\n",gGRAStreamingCfg.gra_loop_state );
            gbSimulcastH264WaitEncDone = 1;
        }
    }
}

MMP_UBYTE MMPF_Display_StateSimulcast(void)
{
    return gGRAStreamingCfg.gra_loop_state ;
}

void MMPF_Display_SyncH264Ratio(void)
{
    gbGRAEn[0] = gbGRAEn[1] ;
    gsZoomRatio[0] = gsZoomRatioM[0]  = gsZoomRatioM[1] ;
    gsLastZoomRatio[0]  = ZOOM_RATIO_BASE ; 
    gbH264Encoding = MMP_FALSE ;// OOXX TEST
    dbg_printf(3,"--sync h264 ratio\r\n");
}

MMP_UBYTE gbCurH264EncId = 0 ;
void rt_enc_start_init_callback_cust(void* pEncHandle)
{
    MMPF_H264ENC_ENC_INFO *pEnc = (MMPF_H264ENC_ENC_INFO *)pEncHandle;
    if (pEnc->CurBufMode == MMPF_VIDENC_CURBUF_RT) {
        MMPF_IBC_SetStoreEnable (pEnc->CurRTSrcPipeId, MMP_TRUE);
        // H264 RTCTL active mode
        if (pEnc->CurRTFctlMode == MMPF_VIDENC_RTFCTL_ACTIVE) {
            //dbg_printf(0,"h264 eid : %d start\r\n",pEnc->eid);
            gbCurH264EncId = pEnc->eid ;
            MMPF_Display_DoGRA(pEnc->eid,glCurGraSrcAddr, gsCurGraSrcWidth, gsCurGraSrcHeight,MMP_FALSE) ;
        }
    }
}

