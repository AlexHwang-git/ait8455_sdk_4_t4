//==============================================================================
//
//  File        : mmpf_sensor.c
//  Description : Sensor Control function
//  Author      : Penguin Torng
//  Revision    : 1.0
//
//==============================================================================

#include "includes_fw.h"
#include "lib_retina.h"

#include "mmp_reg_vif.h"
#include "mmp_reg_ibc.h"
#include "mmp_reg_i2cm.h"
#include "mmpf_display.h"
#include "mmpf_scaler.h"
#include "mmpf_sensor.h"
#include "mmpf_ibc.h"
#include "mmpf_hif.h"
#include "mmpf_3gpmgr.h"
#include "mmpf_mp4venc.h"
#include "mmpf_rawproc.h"
#include "config_fw.h"
#include "mmpf_vif.h"
#include "mmp_reg_gpio.h"
#include "hdm_ctl.h"
#include "mmp_reg_scaler.h"
#include "mmp_reg_jpeg.h"
#include "mmps_3gprecd.h"
#include "isp_if.h"
#include "mmpf_usbuvc_vs.h"
#include "mmpf_usbuvc.h"
#include "mmpf_system.h"
#include "mmpf_usbuvc_event.h"
#include "mmp_reg_graphics.h"
#include "mmpf_usbuvch264.h"

//==============================================================================
//
//                              VARIABLES
//
//==============================================================================
/** @addtogroup MMPF_Sensor
@{
*/

extern 	MMPF_OS_FLAGID   SYS_Flag_Hif;
//MMP_UBYTE gbAFEnable;
extern	MMP_UBYTE	gbExposureDoneFrame[];
//extern	MMP_ULONG   glPreviewBufAddr[MMPF_IBC_PIPE_MAX][4];
//extern	MMP_ULONG   glPreviewUBufAddr[MMPF_IBC_PIPE_MAX][4];
//extern	MMP_ULONG   glPreviewVBufAddr[MMPF_IBC_PIPE_MAX][4];
extern  MMP_UBYTE   gbPreviewBufferCount[];

#if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW)) 
extern  MMP_USHORT  gsPreviewBufWidth[MMPF_IBC_PIPE_MAX][4];
extern  MMP_USHORT  gsPreviewBufHeight[MMPF_IBC_PIPE_MAX][4];
extern  MMP_ULONG   glRotateBufAddr[];
extern  MMP_ULONG   glRotateUBufAddr[];
extern  MMP_ULONG   glRotateVBufAddr[];
extern  MMP_UBYTE   gbRotateBufferCount;
extern  MMP_UBYTE   gbRotateCurBufIdx;
extern  MMP_UBYTE   gbRotateDoneBufIdx;
#endif

extern 	MMPF_DISPLAY_PREVIEW_MODE   gPreviewMode[];
extern  MMPF_DISPLAY_WINID  gPreviewWinID[];
extern	MMPF_IBC_LINK_TYPE	gIBCLinkType[];

static  MMP_BOOL    gbInStableMode = MMP_FALSE;

#if ((defined(DSC_MP3_P_FW)||(DSC_R_EN))&&(FDTC_SUPPORT == 1))
extern  MMPF_FDTC_ATTRIBUTE gFDTCAttribute;
extern  void MMPF_MMU_ConfigFDTCWorkingBuffer(unsigned int startAddr, unsigned int endAddr, char cacheable);
#endif
MMP_BOOL    gbInFDTCMode = MMP_FALSE;

MMP_USHORT  gsSensorLCModeWidth, gsSensorLCModeHeight;
MMP_USHORT  gsSensorMCModeWidth, gsSensorMCModeHeight;

MMP_USHORT  gsCurPreviewMode, gsCurPhaseCount;
MMPF_SENSOR_FUNCTION *gsSensorFunction;
MMP_USHORT gsSensorMode;

MMP_BOOL    m_bISP3AStatus = MMP_FALSE;

extern	MMPF_OS_SEMID   m_StartPreviewFrameEndSem;
extern	MMPF_OS_SEMID	m_PreviewControlSem;
extern	MMP_BOOL		m_bReceiveStopPreviewSig[MMPF_IBC_PIPE_MAX];
extern	MMP_BOOL		m_bWaitVIFEndSig;

extern MMPF_SENSOR_FUNCTION *SensorFunction_Module0;
#if (TOTAL_SENSOR_NUMBER >= 2)
extern MMPF_SENSOR_FUNCTION *SensorFunction_Module1;
#endif
static MMP_BOOL m_bRawPathPreview = MMP_FALSE;
MMP_BOOL m_bZeroShutterLagPath   = MMP_FALSE;        //Ted ZSL
static MMP_BOOL m_bRawStoreDownsample   = MMP_FALSE;
//static MMP_BOOL m_bDVSPathPreview       = MMP_FALSE;
MMP_ULONG glFrameStartSOF ;
#if PCAM_EN==1
//MMP_BOOL gbDVSStartSig = MMP_FALSE;
#else
extern MMP_BOOL gbDVSStartSig;
#endif
extern MMP_BOOL		m_bReceiveStopPreviewSig[];
//extern MMP_USHORT	gsPreviewPath;

extern	MMP_USHORT	m_gsISPCoreID;

extern MMP_BOOL    gbAutoFocus;

MMP_ULONG	m_glISPBufferStartAddr;
MMP_ULONG   m_glISPDmaBufferStartAddr ;
MMPF_SENSOR_3A_STATE    gb3AInitState = MMPF_SENSOR_3A_RESET ;

MMP_UBYTE   gbSensorPowerOffMode = 0;

VENUS_SYSTEM_MODE gSystemMode;

//------------------------------------------------------------------------------
//  Function    : MMPF_VIF_ISR
//  Description :
//------------------------------------------------------------------------------
//MMP_ULONG trycount = 0;
//MMP_ULONG glFrameStartTicks = 0 ;
#include "mmpf_usbpccam.h"
//extern MMP_ULONG   glPccamResolution;
extern MMP_ULONG glPCCamCnt0;
//extern volatile MMP_ULONG STC;
extern MMP_BOOL     m_bVidRecdPreviewStatus[] ;
extern MMP_UBYTE	m_bStartPreviewFrameEndSig;
//extern MMP_USHORT   gsKeepCurSlotNum ;
//extern PCAM_USB_VIDEO_RES gsVidRes;
extern MMP_SHORT gsSkipFrame ;
extern volatile MMP_ULONG PTS;
extern MMP_ULONG glCPUFreq ;
extern MMP_BYTE	gbFDTCUpdate[2];
extern MMPF_SCALER_FIT_RANGE	gFitRange[2];
extern MMPF_SCALER_GRABCONTROL  gGrabCtl[2];

MMP_BOOL gbSensorTaskIdle ;

static MMP_ULONG glSensorHeartBeat = 1 ;
static MMP_ULONG glSensorTimeTick ;

MMP_BOOL MMPF_Detect_Sensor(void)
{
static MMP_ULONG glSensorTicks = 0  ;
    MMP_BOOL en;
    MMPS_3GPRECD_GetPreviewDisplay(&en) ;
    if(en) {
        glSensorTimeTick++ ;
        if(glSensorTimeTick >= 1000) {
            glSensorTimeTick = 0 ;
            if(glSensorHeartBeat == glSensorTicks) {
                dbg_printf(0,"--ReSnr\r\n");
                USB_RestartSensor(0);
                glSensorHeartBeat = 1 ;
                glSensorTicks = 0 ;
            }
            else {
                glSensorTicks =  glSensorHeartBeat ;
            }
            //dbg_printf(0,"%d %d %d\r\n",glSensorTimeTick,glSensorHeartBeat,glSensorTicks);
        }        
    }  
    else {
        glSensorTimeTick = 0 ;
        glSensorHeartBeat = 1 ;
        glSensorTicks = 0;
    }  
}


void MMPF_VIF_ISR(void)
{
extern volatile MMP_ULONG uSOFNUM;
    AITPS_VIF   pVIF = AITC_BASE_VIF;
    MMP_UBYTE   intsrc1,intsrc, pipe, vid = VIF_CTL;
    static int run_frames = 30,run_grab = 30, idx=0 ;
    int jpegSize=0, buf_addr=0;
    intsrc = pVIF->VIF_INT_CPU_SR[vid] & pVIF->VIF_INT_CPU_EN[vid];
    pVIF->VIF_INT_CPU_SR[vid] = intsrc;
	intsrc1 = pVIF->VIF_INT_HOST_SR[vid];
    
    //if(intsrc!=VIF_INT_FRM_ST)
    //dbg_printf(0,"---------intsrc1:%x------------\r\n",intsrc1);
    	
	if (intsrc & VIF_INT_GRAB_END) {
	    glSensorHeartBeat++ ;
	    
	    // sean@2011_03_25, changed from Frame start to Grab End
	    glFrameStartSOF = usb_vc_cur_sof() ;  
	    //dbg_printf(3,"G");
		if (m_bWaitVIFEndSig == TRUE) {
			m_bWaitVIFEndSig = MMP_FALSE;
	    	MMPF_OS_ReleaseSem(m_PreviewControlSem);
	    	gsSensorFunction->MMPF_Sensor_Set3AStatus(MMP_FALSE);
			#if (defined(ALL_FW))
			MMPF_RAWPROC_CheckRawStoreDone();
			MMPF_VIF_EnableInputInterface(MMP_FALSE);
			#endif
	    }
        else {
            if (m_bRawPathPreview) {
                idx =( idx +1 )%run_frames;
                if( (idx%5)){
                    MMP_UBYTE release = 1,ep;			
                    //RES_TYPE_CFG *cur_res = GetResCfg(glPccamResolution) ;
                    //jpegSize = ( cur_res->res_w * cur_res->res_h) * 2;
                    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe(PIPE_0);
                    //ep = usb_get_cur_image_ep(PIPE_0);
                    jpegSize = cur_pipe->pipe_w[PIPE_0] * cur_pipe->pipe_h[PIPE_0] * 2;
                    //MMPF_RAWPROC_CheckRawStoreDone();
                    glPCCamCnt0++;
                    buf_addr = (MMP_ULONG)MMPF_Video_CurWrPtr(PIPE_0);//use pipe 0   				
                    usb_uvc_fill_payload_header((MMP_UBYTE *)buf_addr,jpegSize,glPCCamCnt0,0,PTS/*STC*/,cur_pipe->pipe_w[0],cur_pipe->pipe_h[0] ,41,5);		        
                    // Advance to next slot 
                    MMPF_Video_UpdateWrPtr(PIPE_0);
                    MMPF_RAWPROC_StoreSensorData(MMP_FALSE,MMP_FALSE);	
                    MMPF_USB_ReleaseDm(PIPE_0);
                }
            }

        }
	}
	
	if(m_bRawPathPreview==0) {
    	if ((intsrc & VIF_INT_FRM_ST/* VIF_INT_GRAB_END*/)&& (gsSkipFrame<0 )) {
    	    MMP_UBYTE update_scaler = MMPF_Scaler_CheckUpdatePath();
            MMPF_Scaler_SetZoomOP();
    #if SUPPORT_DIGITAL_PAN || SUPPORT_DIGITAL_ZOOM
            MMPF_Scaler_SetPanTiltOP();
    #endif
    #if SUPPORT_CHANGE_AE_WINDOW
            if(update_scaler) {
                MMPF_Sensor_UpdateAEWindow(update_scaler,MMPF_SCALER_PATH_1);
            }
    #endif            
    
    #if OMRON_FDTC_SUPPORT
    	for(pipe = 0; pipe < 2; pipe++) {	
    		if (gbFDTCUpdate[pipe]){	    		
	    		dbg_printf(0, "# Main_%d: ScalM=%d, ScalN=%d\r\n", pipe, gGrabCtl[pipe].usScaleM, gGrabCtl[pipe].usScaleN);
	    		MMPF_Scaler_SetEngine(MMP_TRUE, pipe, &gFitRange[pipe], &gGrabCtl[pipe]);
				MMPF_Scaler_SetLPF(pipe, &gFitRange[pipe], &gGrabCtl[pipe]);
				gbFDTCUpdate[pipe] = 0;
			}			
		}	
    #endif
    	}
    	
    	
        if (intsrc & VIF_INT_FRM_ST) {
            PIPE_PATH pipe; 
            STREAM_CFG *cur_stream;
            //dbg_printf(3,"  vs.f:%d\r\n",usb_vc_cur_sof() );
            //PTS =  MMPF_BSP_GetTick() * glCPUFreq + MMPF_BSP_GetTickClock()  ;
            #if LYNC_CERT_TEST_EN==0
            PTS = MMPF_BSP_GetTick() * USB_STC_CLK_KHZ + MMPF_BSP_GetTickClock();
            #endif
            #if INIT_SCALE_PATH_EN==1
                #if USB_SPEEDUP_PREVIEW_TIME==1	    
                if(MMPF_Sensor_CheckSkipFrame()==0) {
                    #if OMRON_FDTC_SUPPORT
                    if( usb_uvc_is_fdtc_on() && usb_uvc_is_fdtc_idle() ) {
                        AITPS_IBC   pIBC    = AITC_BASE_IBC;
                        volatile AITPS_IBCP pIbcPipeCtl;
                        pIbcPipeCtl = &(pIBC->IBCP_2);
                        if( !(pIbcPipeCtl->IBC_BUF_CFG & IBC_STORE_EN) ) {
                            pIbcPipeCtl->IBC_BUF_CFG |= IBC_STORE_EN;
                            //dbg_printf(3,"FDTC enable IBC 2\r\n");
                        }   
                        
                    }
                    #endif
                
                }
                #endif	
            #endif
            
                
        }	
    }    
}

MMP_USHORT MMPF_Sensor_CheckSkipFrame(void)
{
extern GRA_STREAMING_CFG gGRAStreamingCfg;
extern MMP_ULONG           glGRAPreviewFormat;
extern MMP_BYTE gbGRAEn[];
    STREAM_SESSION *ss ;
    MMP_UBYTE  i;
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_IBC   pIBC    = AITC_BASE_IBC;
    STREAM_CFG *cur_pipe,*fdtc_pipe;
    MMP_UBYTE pipe0_cfg ;
    MMP_UBYTE pipe1_cfg ;
    MMP_UBYTE pipe2_cfg ;
    MMP_BOOL  fdtc_inited = FALSE ;
#if INIT_SCALE_PATH_EN==0 
     int workss = MMPF_Video_GetWorkingStreamSession();
#endif    
    if(gsSkipFrame > 0) {
        gsSkipFrame-- ;
        return 1 ;
    }
    
    if(gsSkipFrame <= 0) {
        for(i=0;i< VIDEO_EP_END();i++) {
            MMP_BOOL config_end = MMP_FALSE ;
            ss = MMPF_Video_GetStreamSessionByEp(i);
            //dbg_printf(3,"ss->tx_flag : %x\r\n",ss->tx_flag);
            if(ss->tx_flag & SS_TX_STREAMING) {
                cur_pipe = usb_get_cur_image_pipe_by_epid(i);
                fdtc_pipe = usb_get_cur_fdtc_pipe();
            } else {
                cur_pipe = 0;
                fdtc_pipe = 0 ;
            }
            
            if(fdtc_pipe) {
                if (PIPE_EN(fdtc_pipe->pipe_en) &PIPE2_EN) {
                     if(ss->tx_flag & SS_TX_CONFIG_END) {
                     #if INIT_SCALE_PATH_EN==0 
                        MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_2)->scalerpath);
                     #endif   
                        MMPF_IBC_SetStoreEnable( PIPE_2,MMP_TRUE);
                        dbg_printf(3,"#ibc.pipe2-en\r\n");
                     }
                }    
            }
            
            if(cur_pipe){
                MMP_UBYTE   grapath = MMPF_Video_IsLoop2GRA(cur_pipe);
                
                if( PIPE_EN(cur_pipe->pipe_en) & PIPE0_EN) {
                    pipe0_cfg = CUR_PIPE_CFG(PIPE_0,cur_pipe->pipe_cfg);
                    if(ss->tx_flag & SS_TX_CONFIG_END) {
                        MMP_BOOL en_jpg = MMP_TRUE ;
                        #if INIT_SCALE_PATH_EN==0 
                        if( !UVCX_IsSimulcastH264()) {
                            if(gbRestartPreview) {
                                MMPF_SCALER_SOURCE scal_src = gbRestartPreview >> 4 ;
                                
                                if(scal_src==MMPF_SCALER_SOURCE_GRA) {
                                    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                                    en_jpg = MMP_FALSE ;
                                }
                                else {
                                    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                                }
                            }
                            else {
                                MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                            }
                        }
                        else {
                            if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                                MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                            }
                            else {
                                if(UVC_VCD()==bcdVCD10) {
                                    MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath);
                                }
                                else {
                                
                                }
                            }
                            // simulcast h264 is start first, 
                            if(UVC_VCD()==bcdVCD15) {
                                gGRAStreamingCfg.streaming_num = MMPF_VIDENC_GetTotalPipeNum()+((glGRAPreviewFormat==0xFF)?0:1);
                                gGRAStreamingCfg.work_streaming = GRA_STREAMING0 ;
                                //MMPF_Display_SyncH264Ratio();
                            }       
                            
                        }
                        #endif   
                        if(pipe0_cfg== PIPE_CFG_MJPEG){
                            if( !UVCX_IsSimulcastH264()) {
                                if(en_jpg) {
                                    MMPF_Video_EnableJpeg(); 
                                    dbg_printf(0,"#jpg.pipe0-en\r\n");  
                                }    
                            }
                            else {
                                //if( (UVCX_GetH264EncodeBufferMode()!=FRAME_MODE) &&(UVC_VCD()==bcdVCD10)) {
                                //    MMPF_Video_EnableJpeg(); 
                                //    dbg_printf(3,"#jpg.pipe0-en-uvc10\r\n"); 
                                //}
                            
                            }
                        } 
                        else if(pipe0_cfg==PIPE_CFG_H264) {
                            if(grapath) {
                                // Real-time H264 trigger line
                                pIBC->IBCP_0.IBC_INT_LINE_CNT = 1;
                                dbg_printf(3,"RT H264 Line = 1\r\n");
                            }    
                        }
                        else {
                            // Fixed bayer mode preview can't run 2nd times
                            if(!USB_IsBayerRawPreview()) {
                                if(pipe0_cfg!=PIPE_CFG_NV12) {
                                    MMPF_IBC_SetStoreEnable( PIPE_0,MMP_TRUE);
                                    dbg_printf(3,"#ibc.pipe0-en\r\n");
                                }
                            }    
                        } 
                        
                        if(UVC_VCD()==bcdVCD15) {
                            config_end = MMP_TRUE ;
                        } else {
                            if ( (PIPE_EN(cur_pipe->pipe_en) & PIPE1_EN) ==0 ) {
                                config_end = MMP_TRUE ;
                            }    
                        }
                    }
                }
                #if SUPPORT_GRA_ZOOM
                if (PIPE_EN(cur_pipe->pipe_en) &PIPE2_EN) {
                    pipe2_cfg = CUR_PIPE_CFG(PIPE_2,cur_pipe->pipe_cfg); 
                    if(ss->tx_flag & SS_TX_CONFIG_END) {
                        #if INIT_SCALE_PATH_EN==0 
                            MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_2)->scalerpath);
                        #endif   
                    
                        if(pipe2_cfg== PIPE_CFG_MJPEG){
                            MMPF_Video_EnableJpeg();    
                            dbg_printf(3,"#jpg.pipe2-en\r\n");  
                        } else {
                            if( UVCX_IsSimulcastH264() ) {
                                if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                                    MMPF_IBC_SetStoreEnable( PIPE_2,MMP_FALSE);
                                    dbg_printf(3,"#ibc.pipe2-dis-sim-fr\r\n");
                                }
                                else {
                                    MMPF_IBC_SetStoreEnable( PIPE_2,MMP_TRUE);
                                    dbg_printf(3,"#ibc.pipe2-en-sim-en\r\n");
                                }
                            }
                            else {
                                MMPF_IBC_SetStoreEnable( PIPE_2,MMP_TRUE);
                                dbg_printf(3,"#ibc.pipe2-en\r\n");
                            }
                        }
                        //ss->tx_flag &= ~SS_TX_CONFIG_END;
                    }
                }
                #endif
                
                if (PIPE_EN(cur_pipe->pipe_en) &PIPE1_EN) {
                    pipe1_cfg = CUR_PIPE_CFG(PIPE_1,cur_pipe->pipe_cfg); 
                    if(ss->tx_flag & SS_TX_CONFIG_END) {
                        MMP_BOOL pipe_store_en[MMPF_IBC_PIPE_MAX] = { 0, 0 ,0} ;
                        #if INIT_SCALE_PATH_EN==0 
                        // (ISP->SCALER)
                        if( UVCX_IsSimulcastH264()) {
                        
                            if(UVCX_GetH264EncodeBufferMode()==FRAME_MODE) {
                                MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_1)->scalerpath);
                            } else {
                                MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_1)->scalerpath);
                            }
                        }
                        else {
                            MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_ISP, usb_uvc_get_preview_fctl_link(PIPE_1)->scalerpath);
                        }
                        if( UVCX_IsSimulcastH264()) {
                        // switch pipe 0 to GRA....
                            if(MMPF_Scaler_GetPath(PIPE_0)==MMPF_SCALER_SOURCE_ISP) {
                                MMPF_Scaler_SetPath(MMPF_SCALER_SOURCE_GRA, usb_uvc_get_preview_fctl_link(PIPE_0)->scalerpath); 
                                
                            } 
                            else {
                                dbg_printf(3,"--pipe0-in-GRA\r\n");   
                            }
                            if(UVC_VCD()==bcdVCD15) {
                                gGRAStreamingCfg.streaming_num = MMPF_VIDENC_GetTotalPipeNum()+((glGRAPreviewFormat==0xFF)?0:1);
                                gGRAStreamingCfg.work_streaming = GRA_STREAMING0 ;
                            }       
                        }
                        #endif   
                        if(pipe1_cfg== PIPE_CFG_MJPEG){
                            MMPF_Video_EnableJpeg();    
                            dbg_printf(0,"#jpg.pipe1-en\r\n");  
                        } 
                        else if (pipe1_cfg==PIPE_CFG_H264) {
                            if(grapath) {
                            #if SUPPORT_OSD_FUNC==0
                                // Real-time H264 trigger line
                                pIBC->IBCP_1.IBC_INT_LINE_CNT = 1;
                            #endif    
                            }    
                        } else {
                            if(pipe1_cfg!=PIPE_CFG_NV12) {
                                pipe_store_en[PIPE_1] = MMP_TRUE ;
                                dbg_printf(3,"#ibc.pipe1-en(%d,%d,%d)\r\n",pipe_store_en[0],pipe_store_en[1],pipe_store_en[2]);
                            }
                        }
                        if(pipe1_cfg!=PIPE_CFG_H264) {
                            MMPF_IBC_SetStoreEnable( pipe_store_en[PIPE_1] ,MMP_TRUE);
                        }
                        #if H264_SIMULCAST_EN
                        else {
                            MMPF_VIDENC_CURBUF_MODE_CTL h264_mode ;
                            MMPF_VIDENC_GetParameter(0, MMPF_VIDENC_ATTRIBUTE_CURBUF_MODE, (void*)&h264_mode);
                            if(h264_mode.InitCurBufMode == MMPF_VIDENC_CURBUF_FRAME) {
                                #if SUPPORT_PCSYNC
                                MMPF_IBC_SetStoreEnable( PIPE_1,pcsync_is_preview()?MMP_FALSE:MMP_TRUE);
                                dbg_printf(0,"#ibc.pipe1-en\r\n");
                                #else
                                MMPF_IBC_SetStoreEnable( PIPE_1,MMP_TRUE);
                                dbg_printf(0,"#ibc.pipe1-en\r\n");
                                #endif
                            }
                        }
                        #endif
                        config_end = MMP_TRUE ;
                        //ss->tx_flag &= ~SS_TX_CONFIG_END;
                    }
                }
                
                
                // Put tx_flag update together
                if(config_end) {
                    ss->tx_flag &= ~SS_TX_CONFIG_END;
                    //dbg_printf(3,"<SS-TX CONFIG-END\r\n");
                }
            }
        }
        gsSkipFrame = -1 ;
       // dbg_printf(3,"#VIF_ISR.Enable Pipe\r\n");
    }
    return 0 ;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_ISP_ISR
//  Description :
//------------------------------------------------------------------------------
//sean@2011_01_24.
// Revised to fix AE hunting, horizontal lines
// 
extern ISP_UINT8 gDrawTextEn; 
#if SUPPORT_GRA_ZOOM
extern MMPF_SCALER_SOURCE gScalSrc;
#endif
MMP_BOOL gbResetIBC ;

void MMPF_ISP_ISR(void)
{
static MMP_UBYTE  isp_int_overlap = 0;
static MMP_ULONG s_f,e_f ;
#if (CHIP==P_V2)
#define ISP_DBG_REG (0x80000452)
#endif
#if (CHIP==VSN_V2)||(CHIP==VSN_V3)
#define ISP_DBG_REG (0x80006516)//(0x80007052)
#endif

    AITPS_ISP   pISP = AITC_BASE_ISP;
    MMP_UBYTE   intsrc,intsrc1;
    AITPS_JPG   pJPG = AITC_BASE_JPG;
    AITPS_GRA   pGRA = AITC_BASE_GRA;
    AITPS_GBL	pGBL = AITC_BASE_GBL;
    AITPS_SCAL	pSCAL = AITC_BASE_SCAL;
    STREAM_CFG  *cur_pipe;
    MMP_UBYTE 	pipe1_cfg ;
    
    intsrc = pISP->ISP_INT_CPU_SR & pISP->ISP_INT_CPU_EN;
    pISP->ISP_INT_CPU_SR = intsrc;


    if(m_bRawPathPreview) {
        return ;
    }
    
    if (intsrc & ISP_INT_FRAME_START) {
		#if (USE_TASK_DO3A == 1)
		MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_ISP_AE_START, MMPF_OS_FLAG_SET); // Change to task handle the 3A.
		#else
		gsSensorFunction->MMPF_Sensor_DoAEOperation_ST();
		#endif
		if(isp_int_overlap) {
		    //dbg_printf(3,"<%x>\r\n",isp_int_overlap );
		    isp_int_overlap = 0 ;
		}
    }

    if (intsrc & ISP_INT_FRAME_END) {
        //MMPF_SCALER_ZOOM_INFO* scalinfo = MMPF_Scaler_GetCurInfo(GRA_DST_PIPE_1);
        // Frame sync switch scaler path(GRA->SCALER)
        //dbg_printf(3,"# %d #\r\n",isp_fe_c );
    #if SUPPORT_GRA_ZOOM
        MMPF_Scaler_SwitchPath(MMP_FALSE,MMPF_SCALER_PATH_0,0,0); 
        MMPF_Scaler_SwitchPath(MMP_FALSE,MMPF_SCALER_PATH_1,0,0); 
    #endif    
#if 1       
        gsSensorFunction->MMPF_Sensor_DoAEOperation_END();
        ISP_IF_AWB_GetHWAcc(1);//20121106
	
        #if DRAW_TEXT_FEATURE_EN==ON
            gDrawTextEn = (*(MMP_UBYTE*)0x800070c4) & 0x01;
        #endif
        #if IQ_OPR_DMA_ON
            ISP_IF_IQ_UpdateOprtoHW(ISP_IQ_SWITCH_ALL, 0);
        #else
            if (((*(MMP_UBYTE*) 0x800070c4) & 0x02) == 0)
                gsSensorFunction->MMPF_Sensor_DoIQOperation();
        #endif		
#endif		
        //dbg_printf(3,"[ISP_INT_FRAME_END] \r\n");
    }
    
    if (intsrc & ISP_INT_AF_CALC_DONE) {
    #if SUPPORT_AUTO_FOCUS
    	ISP_IF_AF_GetHWAcc(1);
    	//RTNA_DBG_Str(0, "Get AF ACC......\r\n");
    #endif	
    }
    
    #if INIT_SCALE_PATH_EN==0
    if (intsrc & ISP_INT_FRAME_END) {
        MMP_BOOL check = MMP_TRUE ;
        e_f = usb_vc_cur_usof();
        
        intsrc1 = pISP->ISP_INT_CPU_SR & pISP->ISP_INT_CPU_EN;
        if ( intsrc & ISP_INT_FRAME_START ) {
            check = FALSE ;
            isp_int_overlap |= 1;
            //dbg_printf(3,"X");
        }
        if ( intsrc1 & ISP_INT_FRAME_START ) {
            check = FALSE ;
            //dbg_printf(3,"Z");
           isp_int_overlap |= 2;
        }
        if(check) {
            if( MMPF_Sensor_CheckSkipFrame()==0 ) {
                #if OMRON_FDTC_SUPPORT
                if( usb_uvc_is_fdtc_on() && usb_uvc_is_fdtc_idle() ) {
                    AITPS_IBC   pIBC    = AITC_BASE_IBC;
                    volatile AITPS_IBCP pIbcPipeCtl;
                    pIbcPipeCtl = &(pIBC->IBCP_2);
                    if( !(pIbcPipeCtl->IBC_BUF_CFG & IBC_STORE_EN) ) {
                        pIbcPipeCtl->IBC_BUF_CFG |= IBC_STORE_EN;
                        //dbg_printf(3,"FDTC enable IBC 2\r\n");
                    }   
                    
                }
                #endif
            } 
        }
        if( UVC_VCD()==bcdVCD15 ) {
            if(gsSkipFrame<=0) {
               usb_vs_timer_1stenable(MMPF_TIMER_1);
            }
        }
    }                
    #endif
    
}


MMP_ERR MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_STATE state)
{
    gb3AInitState = state ;
    return MMP_ERR_NONE;
}

MMPF_SENSOR_3A_STATE MMPF_Sensor_Get3AState(void)
{
    return gb3AInitState ;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_GetParam
//  Description :
//------------------------------------------------------------------------------
/** @brief Get sesor related variables

Get sesor related variables

@param[in]  param_type  parameter type
@param[out] param       result

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_Sensor_GetParam(MMPF_SENSOR_PARAM param_type, void* param)
{
    switch(param_type)
    {
    case MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE:
       *((MMP_BOOL*)param) = m_bRawPathPreview;
        break;
    case MMPF_SENSOR_RAWPATH_STORE_DOWNSAMPLE:
        *((MMP_BOOL*)param) = m_bRawStoreDownsample;
        break;

    case MMPF_SENSOR_RAWPATH_ZEROSHUTTERLAG_ENABLE:
       *((MMP_BOOL*)param) = m_bZeroShutterLagPath;
        break;
    }
    
    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetParam
//  Description :
//------------------------------------------------------------------------------
/** @brief Set sesor related variables

Set sesor related variables

@param[in]  param_type  parameter type
@param[out] param       result

@return MMP_ERR_NONE
*/
MMP_ERR MMPF_Sensor_SetParam(MMPF_SENSOR_PARAM param_type, void* param)
{
    switch (param_type) {
    case MMPF_SENSOR_RAWPATH_PREVIEW_ENABLE:
		m_bRawPathPreview = *((MMP_BOOL*)param);
        break;
    }
    
    return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_IsStableMode
//  Description :
//------------------------------------------------------------------------------

MMP_BOOL MMPF_Sensor_IsStableMode(void)
{
    return gbInStableMode;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_IsStableMode
//  Description :
//------------------------------------------------------------------------------

MMP_BOOL MMPF_Sensor_IsFDTCMode(void)
{
    return gbInFDTCMode;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_GetPreviewEffectNum
//  Description :
//------------------------------------------------------------------------------

MMP_ERR MMPF_Sensor_GetPreviewEffectNum(MMP_UBYTE *ubEffectNum)
{
    *ubEffectNum = 38;

    return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_WaitFrame
//  Description :
//------------------------------------------------------------------------------

MMP_ERR MMPF_Sensor_WaitFrame(MMP_USHORT usFrameCount)
{
	MMP_USHORT i;
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_UBYTE   vid = VIF_CTL;

	for (i = 0; i < usFrameCount; i++) {
		pVIF->VIF_INT_CPU_SR[vid] = VIF_INT_FRM_ST;
	    while (!(pVIF->VIF_INT_CPU_SR[vid] & VIF_INT_FRM_ST));
	}

	return MMP_ERR_NONE;
}


//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_CheckFrameStart
//  Description : porting function for venus driver
//------------------------------------------------------------------------------

MMP_ERR MMPF_Sensor_CheckFrameStart(MMP_USHORT usFrameCount)
{
#define VIF_WAIT_MS (1000)
	MMP_USHORT i;
	int wait_count = VIF_WAIT_MS ;
	MMP_BOOL    bEnable;
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_UBYTE   vid = VIF_CTL;

    MMPF_VIF_IsInterfaceEnable(&bEnable);

	for (i = 0; i < usFrameCount; i++) {
	    if ( bEnable ) {
                pVIF->VIF_INT_HOST_SR[vid] = VIF_INT_FRM_ST;
	        while (!(pVIF->VIF_INT_HOST_SR[vid] & VIF_INT_FRM_ST)){
 		    MMPF_OS_Sleep(1);
    	            wait_count-- ;
    	            if(wait_count <= 0) {
    	                return MMP_SENSOR_ERR_INITIALIZE ;
    	            }    
	    	}
	    }
	    else {
	        MMPF_OS_Sleep(100);
	    }
	}

	return MMP_ERR_NONE;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_CheckFrameEnd
//  Description : porting function for venus driver
//------------------------------------------------------------------------------

MMP_ERR MMPF_Sensor_CheckFrameEnd(MMP_USHORT usFrameCount)
{
    MMP_USHORT i;
    AITPS_VIF  pVIF = AITC_BASE_VIF;
    MMP_UBYTE   vid = VIF_CTL;

    for (i = 0; i < usFrameCount; i++) {
        pVIF->VIF_INT_HOST_SR[vid] = VIF_INT_FRM_END;
        while (!(pVIF->VIF_INT_HOST_SR[vid] & VIF_INT_FRM_END));
    }

	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetVIFInterrupt
//  Description : porting function for venus driver
//------------------------------------------------------------------------------
MMP_ERR MMPF_Sensor_SetVIFInterrupt(MMP_UBYTE ubFlag, MMP_BOOL bEnable)
{
	AITPS_VIF  pVIF = AITC_BASE_VIF;
	MMP_UBYTE   vid = VIF_CTL;

	//RTNA_DBG_Str(0, "Venus Interrupt Enable function : ");
	//RTNA_DBG_Byte(0, ubFlag);
	//if (bEnable)
	//	RTNA_DBG_Str(0, " ON\r\n");
	//else
	//	RTNA_DBG_Str(0, " OFF\r\n");

	if (bEnable) {
		pVIF->VIF_INT_CPU_SR[vid] = ubFlag;
		pVIF->VIF_INT_CPU_EN[vid] |= ubFlag;
	} 
	else {
		pVIF->VIF_INT_CPU_EN[vid] &= ~ubFlag;
	}

	return MMP_ERR_NONE;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_SetISPInterrupt
//  Description : porting function for venus driver
//------------------------------------------------------------------------------
MMP_ERR MMPF_Sensor_SetISPInterrupt(MMP_UBYTE ubFlag, MMP_BOOL bEnable)
{
    AITPS_ISP   pISP = AITC_BASE_ISP;

	if (bEnable) {
		pISP->ISP_INT_CPU_SR = ubFlag;
		pISP->ISP_INT_CPU_EN |= ubFlag;
	} 
	else {
		pISP->ISP_INT_CPU_EN &= ~ubFlag;
	}

	return MMP_ERR_NONE;
}

MMP_ERR MMPF_Sensor_3A_SetInterrupt(MMP_BOOL bEnable)
{
    m_bISP3AStatus = bEnable;
   // dbg_printf(3,"m_bISP3AStatus:%d\r\n",m_bISP3AStatus);
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
	MMPF_Sensor_SetVIFInterrupt(VIF_INT_GRAB_END, bEnable);
	//MMPF_Sensor_SetVIFInterrupt(VIF_INT_LINE_0, bEnable);
	MMPF_Sensor_SetVIFInterrupt(VIF_INT_FRM_ST, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_AF_CALC_DONE, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_FRAME_START, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_FRAME_END, bEnable);
    #endif
    #if (CHIP == P_V2)
 	MMPF_Sensor_SetVIFInterrupt(VIF_INT_GRAB_END, bEnable);
	MMPF_Sensor_SetVIFInterrupt(VIF_INT_LINE_NO, bEnable);
	MMPF_Sensor_SetVIFInterrupt(VIF_INT_FRM_ST, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_AF_CALC_DONE, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_FRAME_START, bEnable);
	MMPF_Sensor_SetISPInterrupt(ISP_INT_FRAME_END, bEnable);
    #endif

    return MMP_ERR_NONE;
}

MMP_BOOL MMPF_Sensor_3A_GetInterrupt(void)
{
    return m_bISP3AStatus ;
}
//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ConfigFunctions
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Sensor_ConfigFunctions(MMP_USHORT usSensorID)
{
    switch(usSensorID){
    case 0:
        gsSensorFunction = SensorFunction_Module0;
        break;
    #if (TOTAL_SENSOR_NUMBER >= 2)        
    case 1:
        gsSensorFunction = SensorFunction_Module1;
        break;
    #endif        
    default:
        break;
    }

	return MMP_ERR_NONE;
}
#if ISP_BUF_IN_SRAM==1
MMP_ERR MMPF_Sensor_CalculateBufferSize (MMP_ULONG *ulBufSize, MMP_ULONG *ulDmaSize)
{
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    *ulBufSize = ISP_BUFFER_SIZE;
    *ulDmaSize = IQ_OPR_DMA_SIZE;
    #endif
    dbg_printf(3,"<*>ISP buf size : %d,DMA opr size:%d\r\n",*ulBufSize,*ulDmaSize); 
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr, MMP_ULONG ulDmaBufAddr)
{
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
	m_glISPBufferStartAddr = ulStartAddr;
	m_glISPDmaBufferStartAddr = ulDmaBufAddr;
	#endif
    dbg_printf(3,"<*>ISP buf addr : %x,DMA opr addr:%x\r\n",m_glISPBufferStartAddr,m_glISPDmaBufferStartAddr); 

	return MMP_ERR_NONE;
}

#else

MMP_ULONG MMPF_Sensor_CalculateBufferSize (void)
{
    MMP_ULONG   total_size = 0;

    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    total_size += ISP_BUFFER_SIZE;
    #endif
    //dbg_printf(3,"ISP HW BUF size:%d\r\n",total_size);
    return total_size;
}

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_AllocateBuffer
//  Description :
//------------------------------------------------------------------------------
MMP_ERR MMPF_Sensor_AllocateBuffer(MMP_ULONG ulStartAddr)
{
    #if (CHIP == VSN_V2) || (CHIP == VSN_V3)
    m_glISPBufferStartAddr = ulStartAddr;
    //dbg_printf(3,"ISP HW BUF Addr:%x\r\n",m_glISPBufferStartAddr);
	#endif

	return MMP_ERR_NONE;
}
#endif

//------------------------------------------------------------------------------
//  Function    : MMPF_Sensor_ProcessCmd
//  Description :
//------------------------------------------------------------------------------
#if PCAM_EN==0
MMP_ERR MMPF_Sensor_ProcessCmd(void)
{
    MMP_ULONG   ulParameter[6], i;
    MMP_USHORT  usData, usBaseN, usCurrentN;
    MMP_USHORT	usCommand;
    MMP_USHORT  usIBCPipe;
    MMP_ERR		err;
    MMP_UBYTE	ubEffectNum;
    MMP_ULONG	ulSize;

	usCommand = MMPF_HIF_GetCmd();
    //GBL_HOST_PARM only has 24 bytes (12 words or 6 ulong int)
	for (i = 0; i < 24; i += 4)
		ulParameter[i >> 2] = MMPF_HIF_CmdGetParameterL(i);
	
	MMPF_HIF_SetCmdStatus(SENSOR_CMD_IN_EXEC);	
	MMPF_HIF_ClearCmd();

    switch (usCommand & (FUNC_MASK|GRP_MASK)) {
    
    case HIF_CMD_SENSOR_CONTROL:
        switch (usCommand & SUB_MASK) {
        case SET_REGISTER:
            gsSensorFunction->MMPF_Sensor_SetReg((MMP_USHORT)ulParameter[0], (MMP_USHORT)(ulParameter[0] >> 16));
            break;
        case GET_REGISTER:
            gsSensorFunction->MMPF_Sensor_GetReg((MMP_USHORT)ulParameter[0], &usData);
			MMPF_HIF_CmdSetParameterW(0, usData);
            break;
        }
        break;
    case HIF_CMD_SET_PREVIEW_MODE:
        if ((usCommand & ENABLE_PREVIEW) == ENABLE_PREVIEW) {
			MMPF_HIF_CmdSetParameterL(0, (MMP_ULONG)MMPF_Display_StartPreview((MMP_USHORT)ulParameter[0], (MMP_BOOL)ulParameter[1]));
        }
        else {
            MMPF_Display_StopPreview((MMP_USHORT)ulParameter[0]);
		}
        break;
    case HIF_CMD_INIT_SENSOR:
        MMPF_Sensor_ConfigFunctions((MMP_USHORT)ulParameter[0]);
        gsSensorFunction->MMPF_Sensor_SetFrameRate((MMP_USHORT)((ulParameter[0] & 0xFFFF0000) >> 16));
        gsSensorFunction->MMPF_Sensor_InitializeVIF();
		gsSensorFunction->MMPF_Sensor_InitializeISP();
		err = gsSensorFunction->MMPF_Sensor_Initialize();

		MMPF_HIF_CmdSetParameterL(0, err);	//Return return code [0:success, 1:fail]		
		if (err) {		
			return err;
		}
        gsCurPreviewMode = (MMP_USHORT)ulParameter[1];
        gsCurPhaseCount = (MMP_USHORT)((ulParameter[1] & 0xFFFF0000) >> 16);
        gsSensorFunction->MMPF_Sensor_SetPreviewMode(gsCurPreviewMode);
        break;
    case HIF_CMD_SET_SENSOR_MODE:
    	gsSensorMode = (MMP_USHORT)ulParameter[0];
    	break;
    case HIF_CMD_POWERDOWN_SENSOR:
        MMPF_Sensor_ConfigFunctions((MMP_USHORT)ulParameter[0]);
        gsSensorFunction->MMPF_Sensor_PowerDown();
        break;
    case HIF_CMD_SET_ZOOM_PARAM:
        switch (usCommand & SUB_MASK) {
        case ZOOM_PARAMTER:
            MMPF_Scaler_SetZoomParams((MMP_USHORT)ulParameter[5],  
                                      (MMPF_SCALER_GRABCONTROL *)&ulParameter[2],
                                      (MMP_USHORT)ulParameter[1],
                                      (MMP_USHORT)(ulParameter[1] >> 16),
                                      (MMP_USHORT)ulParameter[0],
                                      (MMP_USHORT)(ulParameter[0] >> 16));
            break;
        case SINGLE_ZOOM_RANGE:
            MMPF_Scaler_SetZoomRange((MMP_USHORT)ulParameter[0],
                                     (MMP_USHORT)(ulParameter[0] >> 16),
                                     (MMP_USHORT)ulParameter[1]);
            break;
        case ZOOM_DIRECTION:
            MMPF_Scaler_SetDigitalZoom((MMP_USHORT)ulParameter[0],
                                       (MMP_USHORT)(ulParameter[0] >> 16), 
                                       (MMPF_SCALER_PATH)ulParameter[1], 
                                        (MMP_USHORT)(ulParameter[1] >> 16));
            break;
        }
        break;
    case HIF_CMD_GET_ZOOM_PARAM:
        switch (usCommand & SUB_MASK) {
        case ZOOM_INFO:
            MMPF_Scaler_GetZoomInfo((MMP_USHORT)ulParameter[0], &usBaseN, &usCurrentN);
            MMPF_HIF_CmdSetParameterW(0, usBaseN);
            MMPF_HIF_CmdSetParameterW(2, usCurrentN);
            break;
        }
        break;
    case HIF_CMD_SET_ZOOMOP:
        MMPF_Scaler_SetDigitalZoomOP((MMP_USHORT)ulParameter[0],
                                       (MMP_USHORT)(ulParameter[0] >> 16), 
                                       (MMPF_SCALER_PATH)ulParameter[1]);
        break;
    case HIF_CMD_3A_FUNCTION:
        switch (usCommand & SUB_MASK) {
        	case SET_AF_WIN:
               	gsSensorFunction->SetAFWin((MMP_USHORT)ulParameter[0], 
					(MMP_USHORT)(ulParameter[0] >> 16), (MMP_USHORT)ulParameter[1],
					(MMP_USHORT)(ulParameter[1] >> 16), (MMP_USHORT)ulParameter[2],
					(MMP_USHORT)(ulParameter[2] >> 16), (MMP_USHORT)ulParameter[3]);
                break;
            case AUTO_FOCUS:
			//gbInFDTCMode  = MMP_FALSE; 
               	gsSensorFunction->SetAFEnable(1);
                break;
            case FOCUS_INFINITY:
             	if (gsSensorFunction->GetAFEnable() == 1) {
                    RTNA_DBG_Str(3, "ERROR: MMPF_SENSOR_ProcessCmd()->FOCUS_INFINITY --> gbAFEnable ==1 \r\n");
                }
                gsSensorFunction->MMPF_Sensor_SetAFPosition(0);
                break;
            case SET_LENS_POSITION:
             	if (gsSensorFunction->GetAFEnable() == 1) {
                    RTNA_DBG_Str(3, "ERROR: MMPF_SENSOR_ProcessCmd()->SET_LENS_POS --> gbAFEnable ==1 \r\n");
                }
                gsSensorFunction->MMPF_Sensor_SetAFPosition((MMP_USHORT)ulParameter[0]);
                break;
            case LIGHT_FREQ:
                gsSensorFunction->MMPF_SetLightFreq((MMP_USHORT)ulParameter[0]);
                break;
            case AWB_TYPE:
                gsSensorFunction->MMPF_Sensor_SetAWBType((MMP_UBYTE)ulParameter[0] );
                break;
            case AE_TYPE:
                gsSensorFunction->MMPF_Sensor_SetAEEnable((MMP_UBYTE)ulParameter[0]);
                break;
            case NIGHT_MODE:
            	if ((ulParameter[0] & 0xFFFF) == 0)
                	gsSensorFunction->MMPF_Sensor_SetImageScene(0);
                else
                	gsSensorFunction->MMPF_Sensor_SetImageScene(7);	
                break;
            case COLOR_CONTRAST:
                gsSensorFunction->MMPF_Sensor_SetContrast((MMP_UBYTE)(ulParameter[0]&0xFF));
                break;
            case COLOR_SATURATION:
				gsSensorFunction->MMPF_Sensor_SetSaturation((MMP_UBYTE)(ulParameter[0]&0xFF));
                break;
            case COLOR_SHARPNESS:
                gsSensorFunction->MMPF_Sensor_SetSharpness((MMP_UBYTE)(ulParameter[0]&0xFF));
                break;
            case SET_AF_MODE:
	            gsSensorFunction->MMPF_Sensor_SetAFMode((MMP_UBYTE)(ulParameter[0]&0xFF));
	            break;
	        case GET_AF_STATUS:
	        	MMPF_HIF_CmdSetParameterB(0, gsSensorFunction->GetAFEnable());
	        	break;
			case SET_HW_BUFFER:
				MMPF_HIF_CmdSetParameterL(0, MMPF_Sensor_AllocateBuffer(ulParameter[0]));
	        	break;
         }
        break;
    case HIF_CMD_GET_EFFECT_NUM:
    	MMPF_Sensor_GetPreviewEffectNum(&ubEffectNum);
    	MMPF_HIF_CmdSetParameterW(0, ubEffectNum);
        break;
    case HIF_CMD_SET_PREVIEW_EFFECT:
    	gsSensorFunction->MMPF_Sensor_SetImageEffect((MMP_UBYTE)ulParameter[0]);
        break;
    case HIF_CMD_SET_EXPOSURE_VALUE:
    	gsSensorFunction->MMPF_Sensor_SetExposureValue((MMP_USHORT)ulParameter[0]);
        break;		        
    case HIF_CMD_SET_PREVIEW_BUF:
        switch (usCommand & SUB_MASK) {
		case BUFFER_ADDRESS:
			glPreviewBufAddr[(MMP_USHORT)ulParameter[0]][(ulParameter[0] >> 16) & 0xFF] = ulParameter[1];
			glPreviewUBufAddr[(MMP_USHORT)ulParameter[0]][(ulParameter[0] >> 16) & 0xFF] = ulParameter[2];
			glPreviewVBufAddr[(MMP_USHORT)ulParameter[0]][(ulParameter[0] >> 16) & 0xFF] = ulParameter[3];
			break;
		case BUFFER_COUNT:
			gbPreviewBufferCount[(MMP_USHORT)ulParameter[0]] = (MMP_UBYTE)(ulParameter[0] >> 16);
			break;
        #if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW))
        case BUFFER_WIDTH:
            gsPreviewBufWidth = (MMP_USHORT)ulParameter[0];
            break;
        case BUFFER_HEIGHT:
            gsPreviewBufHeight = (MMP_USHORT)ulParameter[0];
            break;
        #endif
        }
        break;
        
    #if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW))
    case HIF_CMD_SET_ROTATE_BUF:
        switch (usCommand & SUB_MASK) {
		case BUFFER_ADDRESS:
			glRotateBufAddr[(MMP_USHORT)ulParameter[0]] = ulParameter[1];
			glRotateUBufAddr[(MMP_USHORT)ulParameter[0]] = ulParameter[2];
			glRotateVBufAddr[(MMP_USHORT)ulParameter[0]] = ulParameter[3];
			break;
		case BUFFER_COUNT:
			gbRotateCurBufIdx = 0;
			gbRotateDoneBufIdx = 0;
			gbRotateBufferCount = (MMP_UBYTE)ulParameter[0];
			break;
        }
        break;
    #endif

    case HIF_CMD_SET_IBC_LINK_MODE:
        switch (usCommand & SUB_MASK) {
		case LINK_NONE:
			gIBCLinkType[(MMP_USHORT)ulParameter[0]] = MMPF_IBC_LINK_NONE;
			break;
		case LINK_DISPLAY:
			gIBCLinkType[(MMP_USHORT)ulParameter[0]] |= MMPF_IBC_LINK_DISPLAY;
			gPreviewMode[(MMP_USHORT)ulParameter[0]] = (MMP_USHORT)(ulParameter[0] >> 16);
			gPreviewWinID[(MMP_USHORT)ulParameter[0]] = (MMPF_DISPLAY_WINID)ulParameter[1];
			break;
		case LINK_VIDEO:
			gIBCLinkType[(MMP_USHORT)ulParameter[0]] |= MMPF_IBC_LINK_VIDEO;
        	break;
        #if (LANDSCAPE_SUPPORT)&&(defined(ALL_FW))
		case LINK_DMA:
            gIBCLinkType[(MMP_USHORT)ulParameter[0]] |= MMPF_IBC_LINK_ROTATE;
            gPreviewMode[(MMP_USHORT)ulParameter[0]] = (MMP_USHORT)(ulParameter[0] >> 16);
            gPreviewWinID[(MMP_USHORT)ulParameter[0]] = (MMPF_DISPLAY_WINID)ulParameter[1];
            break;
        #endif
        #if ((defined(DSC_MP3_P_FW)||(DSC_R_EN))&&(FDTC_SUPPORT == 1))
        case LINK_FDTC:
            gFDTCAttribute.usFdtcIBCPipe = (MMP_USHORT)ulParameter[0];
			gIBCLinkType[gFDTCAttribute.usFdtcIBCPipe] |= MMPF_IBC_LINK_FDTC;
        	break;
        #endif
        case LINK_GRAPHIC:
            gIBCLinkType[(MMP_USHORT)ulParameter[0]] |= MMPF_IBC_LINK_GRAPHIC;
            break;
        case UNLINK_GRAPHIC:
            gIBCLinkType[(MMP_USHORT)ulParameter[0]] &= ~MMPF_IBC_LINK_GRAPHIC;
            break;
        }
        break;
    case HIF_CMD_SET_STABLE_STATE: 
        gsSensorFunction->MMPF_SetStableState((MMP_BOOL)ulParameter[0]);
        gbInStableMode = (MMP_BOOL)ulParameter[1];
        break;

    case HIF_CMD_SET_HISTEQ_STATE: 
        gsSensorFunction->MMPF_SetHistEQState((MMP_BOOL)ulParameter[0]);
        break;
    
    case HIF_CMD_SENSOR_PREVIEW_MODE:
        if(gsCurPreviewMode != (MMP_USHORT)ulParameter[0]) {
            gsSensorFunction->MMPF_Sensor_SetPreviewMode((MMP_USHORT)ulParameter[0]);
        }
        break;
    case HIF_CMD_SET_AE_MODE:
        gsSensorFunction->MMPF_Sensor_SetAEMode((MMP_USHORT)ulParameter[0], (MMP_USHORT)((ulParameter[0] & 0xFFFF0000) >> 16));
        break;
    case HIF_CMD_GET_PREVIEW_BUF:
        usIBCPipe = (MMP_USHORT)ulParameter[0];
        MMPF_HIF_CmdSetParameterL(0, glPreviewBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
        MMPF_HIF_CmdSetParameterL(4, glPreviewUBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
        MMPF_HIF_CmdSetParameterL(8, glPreviewVBufAddr[usIBCPipe][gbExposureDoneFrame[usIBCPipe]]);
        break;
    case HIF_CMD_SET_RAW_PREVIEW:
        switch (usCommand & SUB_MASK) {
        case RESET_PREVIEW_ATTRIBUTE:
            MMPF_RAWPROC_ResetPreviewAttribue();
            break;
        case ENABLE_RAWPREVIEW:
            MMPF_RAWPROC_GetVIFSetting();
            m_bRawPathPreview = ulParameter[0];
            if(!m_bRawPathPreview) {
                MMPF_RAWPROC_ResetZoomOP();
            }
            
            break;
        case ENABLE_ZEROSHUTTERLAG_PATH:        //Ted ZSL
            m_bZeroShutterLagPath = ulParameter[0];
            break;
        #if (defined(ALL_FW))
        case ENABLE_DVSPREVIEW:
            m_bDVSPathPreview = ulParameter[0];
            break;
        case SET_STORE_RANGE:
            MMPF_RAWPROC_SetStoreRange(ulParameter[0], ulParameter[1]);
            break;
        case SET_FETCH_RANGE:
            MMPF_RAWPROC_SetFetchRange(ulParameter[0], ulParameter[1]);
            break;
        #endif
        case ENABLE_RAWSTORE:
            MMPF_RAWPROC_EnableRawPath(ulParameter[0]);
            break;
		case RAWSTORE_ADDR:
		    MMPF_RAWPROC_ConfigRawPathMemory(ulParameter[0],ulParameter[1],ulParameter[2]);
		    break;
		case RAWSTORE_BAYER_ADDR:
            for (i = 0; i < (MMP_UBYTE)(ulParameter[0] & 0x000000FF); i++) {
                MMPF_RAWPROC_SetRawBuffer(i, ulParameter[i+1]);
		}
        break;        
        case RAWSTORE_LUMA_ADDR:
            for (i = 0; i < (MMP_UBYTE)(ulParameter[0] & 0x000000FF); i++) {
                MMPF_RAWPROC_SetLumaBuffer(i, ulParameter[i+1]);
            }
        break;        
        case START_DVS_ME:
            gbDVSStartSig = (MMP_BOOL)(ulParameter[0] & 0xFF);
            break;
		case ENABLE_RAW_DOWNSAMPLE:
		    MMPF_RAWPROC_EnableDownsample(ulParameter[0],ulParameter[1]);
		    
		    m_bRawStoreDownsample = ulParameter[0];
		    break;
		}
        break;  
    }

    MMPF_HIF_ClearCmdStatus(SENSOR_CMD_IN_EXEC);
    return 0;
}
#endif

#if PCAM_EN==1
MMP_ERR MMPF_Sensor_Initialize(MMP_USHORT usSensorID, MMP_UBYTE ubFrameRate,MMP_UBYTE ubPreviewMode)
{
    MMP_ERR err = MMP_ERR_NONE ;
    MMPF_Sensor_ConfigFunctions((MMP_USHORT)usSensorID);

    gsSensorFunction->MMPF_Sensor_SetFrameRate(ubFrameRate);
    gsSensorFunction->MMPF_Sensor_InitializeVIF();

    if(MMPF_Sensor_Get3AState()==MMPF_SENSOR_3A_RESET) {
	    gsSensorFunction->MMPF_Sensor_InitializeISP();
	    MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_SET);
	}

	err = gsSensorFunction->MMPF_Sensor_Initialize();

    if(err) {

        dbg_printf(0,"@ Sensor Init Err: :x%x\r\n",err);
        RTNA_DBG_Str3("Sensor Init Err:");
        RTNA_DBG_Long3(err);
        RTNA_DBG_Str3("\r\n");
        return err;
    }

    // Sensor Mirror Flip
    #if	0	//(CUSTOMER == RAZ)				//ALEXH@20170728
    gsSensorFunction->MMPF_Sensor_SetColorID(MMPF_SENSOR_ROTATE_RIGHT_180);	//For Old sensor module only
    #else
    gsSensorFunction->MMPF_Sensor_SetColorID(MMPF_SENSOR_ROTATE_NO_ROTATE);
    #endif

    gbSensorTaskIdle = MMP_FALSE ;
    gsCurPreviewMode = (MMP_USHORT)ubPreviewMode;
    // gsCurPhaseCount = (MMP_USHORT)ubPhaseCount ;
    // gsSensorFunction->MMPF_Sensor_SetPreviewMode(gsCurPreviewMode);

    return err ;
}

//MMP_ERR MMPF_Sensor_SetPreviewEffect(MMP_UBYTE ubEffectID)
//{
//    gsSensorFunction->MMPF_Sensor_SetImageEffect(ubEffectID);
//    return MMP_ERR_NONE ;
//}

MMP_ERR MMPF_Sensor_PreviewMode(MMP_BOOL ubEnable,MMP_USHORT usIBCPipe,MMP_BOOL bCheckFrameEnd)
{
    if (ubEnable) {
		return MMPF_Display_StartPreview(usIBCPipe, bCheckFrameEnd);
    }
    else {
        return MMPF_Display_StopPreview(usIBCPipe );
	}
}



MMP_ERR MMPF_Sensor_AutoFocus(void)
{
    if(gbAutoFocus == MMP_TRUE){
        gsSensorFunction->SetAFEnable(1);
    }
    return MMP_ERR_NONE;
}

MMP_ERR MMPF_Sensor_SetSensorReg(MMP_USHORT usAddr, MMP_USHORT usData)
{
    return gsSensorFunction->MMPF_Sensor_SetReg(usAddr, usData);
}

MMP_ERR MMPF_Sensor_GetSensorReg(MMP_USHORT usAddr,MMP_USHORT usData)
{
    return gsSensorFunction->MMPF_Sensor_GetReg(usAddr,&usData); 
}

MMP_ERR MMPF_Sensor_PowerDown(MMP_USHORT usSensorID,MMP_UBYTE off_mode)
{
    gbSensorPowerOffMode = off_mode ;
    MMPF_Sensor_ConfigFunctions(usSensorID);
    return gsSensorFunction->MMPF_Sensor_PowerDown();
}

MMP_ERR MMPF_Sensor_InitPad(void)
{
    AITPS_PAD   pPAD = AITC_BASE_PAD;
    AITPS_GBL   pGBL = AITC_BASE_GBL;

    #if(CHIP==VSN_V2)||(CHIP==VSN_V3)
    dbg_printf(3,"Pull Low SEN/SCK/SDA:%x\r\n",pGBL->GBL_IO_CFG_PSNR2);
    // Pull SCK/SDA/SEN to Low
    pGBL->GBL_IO_CFG_PSNR2[0] = PAD_E8_CURRENT | PAD_E4_CURRENT | PAD_PULL_LOW;
    pGBL->GBL_IO_CFG_PSNR2[1] = PAD_E4_CURRENT | PAD_PULL_LOW;
    pGBL->GBL_IO_CFG_PSNR2[2] = PAD_E4_CURRENT | PAD_PULL_LOW;
    // Disable I2C to SNR
    pGBL->GBL_IO_CTL1 &= ~GBL_I2C_0_PAD_SEL_SNR;
    #endif
    
    return MMP_ERR_NONE ;

}

MMP_ERR gbSensorErr = MMP_ERR_NONE ;
MMP_ERR MMPF_Sensor_SetErr(MMP_ERR err)
{
    gbSensorErr = err ;
    return gbSensorErr ;
}

MMP_ERR MMPF_Sensor_GetErr(void)
{
    return  gbSensorErr ;   
}

MMP_ERR MMPF_Sensor_WaitTaskIdle(void)
{
    MMPF_OS_SetFlags(SYS_Flag_Hif, SYS_FLAG_ISP_AE_STOP, MMPF_OS_FLAG_SET); 
    while(gbSensorTaskIdle==MMP_FALSE) {
        MMPF_OS_Sleep(1);
        dbg_printf(3,"@");
    }
}

#endif

//------------------------------------------------------------------------------
//  Function    : DSC_Task
//  Description : DSC main function
//------------------------------------------------------------------------------
extern MMP_UBYTE   gbDramID;

void SENSOR_Task()
{
    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_GPIO  pGPIO = AITC_BASE_GPIO;
    AITPS_PAD   pPAD = AITC_BASE_PAD;
    AITPS_GBL   pGBL = AITC_BASE_GBL; 

    MMPF_OS_FLAGS flags;
    MMPF_OS_FLAGS waitFlags;
	//MMP_UBYTE   i;

   // RTNA_DBG_Str(0, "SENSOR_Task()\r\n"); // Rename to MMPF ?

    // Open VIF interrupt for C3A operation
    RTNA_AIC_Open(pAIC, AIC_SRC_VIF, vif_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_VIF);                
    //RTNA_AIC_IRQ_En(pAIC, AIC_SRC_VIF);

    // Open ISP interrupt for digital zoom operation
    RTNA_AIC_Open(pAIC, AIC_SRC_ISP, isp_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3); //Patrick
    //RTNA_AIC_IRQ_En(pAIC, AIC_SRC_ISP);
    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_ISP);

    RTNA_AIC_Open(pAIC, AIC_SRC_IBC, ibc_isr_a,
                AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 3);
	//RTNA_AIC_IRQ_En(pAIC, AIC_SRC_IBC);   
    RTNA_AIC_IRQ_Dis(pAIC, AIC_SRC_IBC); 

#if (JTAG_DBG_EN==0)
     //RTNA_DBG_Str(0,"Disable JTAG interface\r\n");	
    pGBL->GBL_CHIP_CFG &= ~(ARM_JTAG_MODE_EN);  
#else
    // Reset USB phy here so that we can connect usb by ICE no matter which FW load
    // in Flash.
    MMPF_SYS_ResetHModule(MMPF_SYS_HMODULETYPE_USB_PHY, TRUE);
#endif
    // Bug Fixed : Init I2CM semaphore 
    MMPF_I2cm_InitializeDriver();

    MMPF_Sensor_InitPad();
    
    // Init GPIO
    MMPF_PIO_Initialize();
    
    #if (TEST_PIN1!=255)
    MMPF_PIO_SetData(TEST_PIN1,FALSE);
    MMPF_PIO_EnableOutputMode(TEST_PIN1,TRUE);
    #endif
    
    
    #if (TEST_PIN2!=255)
    MMPF_PIO_SetData(TEST_PIN2,FALSE);
    MMPF_PIO_EnableOutputMode(TEST_PIN2,TRUE);
    #endif

    #if (TEST_PIN3!=255)
    MMPF_PIO_SetData(TEST_PIN3,FALSE);
    MMPF_PIO_EnableOutputMode(TEST_PIN3,TRUE);
    #endif

    #if (LED_READY!=255)
    MMPF_PIO_SetData(LED_READY,TRUE);
    MMPF_PIO_EnableOutputMode(LED_READY,TRUE);
    #endif
    
    #if (AF_EN!=255)
    MMPF_PIO_SetData(AF_EN,TRUE);
    MMPF_PIO_EnableOutputMode(AF_EN,TRUE);
    #endif

  
  
	m_PreviewControlSem = MMPF_OS_CreateSem(0); 
	m_StartPreviewFrameEndSem = MMPF_OS_CreateSem(0); 
	MMPF_Sensor_ConfigFunctions(0);

    waitFlags = SYS_FLAG_SENSOR | SYS_FLAG_ISP_AE_STOP;
    
    #if (USE_TASK_DO3A == 1)
    waitFlags |= SYS_FLAG_ISP_AE_START;
    #endif
    #if ((DSC_R_EN) && (FDTC_SUPPORT == 1))
    waitFlags |= SYS_FLAG_FDTC_DRAWRECT;
    #endif
    
    while (TRUE) {
        MMPF_OS_WaitFlags(SYS_Flag_Hif, waitFlags, MMPF_OS_FLAG_WAIT_SET_ANY | OS_FLAG_CONSUME, 0, &flags);
        if(flags & SYS_FLAG_ISP_AE_START) {
            if( (MMPF_Sensor_Get3AState()==MMPF_SENSOR_3A_SET) && (!gbSensorTaskIdle) ) {      
            	#if (USE_TASK_DO3A == 1)        
                if (flags & SYS_FLAG_ISP_AE_START) {       	
            	#if IQ_OPR_DMA_ON	
            		gsSensorFunction->MMPF_Sensor_DoIQOperation();
            	#endif
        		    gsSensorFunction->MMPF_Sensor_DoAFOperation_FrameStart();
        			gsSensorFunction->MMPF_Sensor_DoAEOperation_ST();
        			gsSensorFunction->MMPF_Sensor_DoAWBOperation(); //20121106
//            	#if IQ_OPR_DMA_ON	
//            		gsSensorFunction->MMPF_Sensor_DoIQOperation();
//            	#endif
                }
            	#endif
            }
        }
        
        if(flags & SYS_FLAG_ISP_AE_STOP) {
            //MMPF_Sensor_Set3AState(MMPF_SENSOR_3A_RESET);  
            //ISP_IF_3A_Control(ISP_3A_DISABLE);
            ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AE, 0);
            ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AWB, 0);
            ISP_IF_3A_SetSwitch(ISP_3A_ALGO_AF, 0);
            gbSensorTaskIdle = TRUE ;
        }
    }
}

#if 0 // Controls for color trans move to scaler
//Steven 20111107 ADD
const MMP_UBYTE gColorTransform[] = {
	0xFF, 0x91, 0xB8, 0x01, 0x80, 0x80, // Y:  0-255, U:  0-255, V:  0-255
//	0xDC, 0x91, 0xB8, 0x10, 0x80, 0x80, // Y: 16-235, U:  0-255, V:  0-255
	0xDC, 0x7E, 0xA0, 0x10, 0x80, 0x80, // Y: 16-235, U:  0-224, V:  0-224
//	0xFF, 0x91, 0xB8, 0x01, 0x80, 0x80, //	0xF0, 0x91, 0xB8, 0x10, 0x80, 0x80, // Y: 16-255, U:  0-255, V:  0-255
	255, 0x91, 0xB8, 7, 0x80, 0x80, // Y: 16-255, U:  0-255, V:  0-255
};

MMP_ERR MMPF_Sensor_SetColorTransform(MMP_UBYTE range)
{
	MMP_USHORT i;
	MMP_USHORT offset 	= range * 6;
	MMP_UBYTE *ISP_XBYTE = (MMP_UBYTE *)AITC_BASE_ISP;
	for (i = 0; i < 6; i++) {
		ISP_XBYTE[0x75 + i] = gColorTransform[offset + i];
	}
	return MMP_ERR_NONE;
}
#endif

//
// select_path is only used when 2 pipes are running.
//
#if SUPPORT_CHANGE_AE_WINDOW
MMP_ERR MMPF_Sensor_UpdateAEWindow(MMP_UBYTE working_path,MMPF_SCALER_PATH select_path)
{
#define AE_WIN_DBG_LEVEL (4)
extern MMP_ULONG  VIFGrab_H_Length, VIFGrab_V_Length;
extern MMP_UBYTE      gbZoomPathSel; 
//MMPF_SCALER_PANTILT_INFO *paninfo = MMPF_Scaler_GetCurPanTiltInfo(MMPF_SCALER_PATH_1);
    MMP_USHORT  imgHST[MMPF_SCALER_PATH_MAX] ;
    MMP_USHORT  imgHED[MMPF_SCALER_PATH_MAX] ;
    MMP_USHORT  imgVST[MMPF_SCALER_PATH_MAX] ;
    MMP_USHORT  imgVED[MMPF_SCALER_PATH_MAX] ;
    MMP_USHORT  imgOffsetX, imgOffsetY;
    MMP_USHORT  Dnsamp,i;
    MMP_USHORT  scal_Hx256, scal_Vx256 ;
    MMPF_SCALER_PATH update_path = MMPF_SCALER_PATH_0 ;
    
    AITPS_SCAL  pSCAL = AITC_BASE_SCAL;
    if(!working_path) {
        return MMP_ERR_NONE ;
    }
    
    for(i=0;i< MMPF_SCALER_PATH_MAX ;i++) {
        imgHST[i] = imgHED[i] = imgVST[i] = imgVED[i] = 0 ;
    }
    
    
    if(working_path & (1 << MMPF_SCALER_PATH_0) ) {
        Dnsamp = (pSCAL->SCAL_LPF_SCAL_CTL & 0x01) ? 4 : ((pSCAL->SCAL_LPF_SCAL_CTL & 0x02) ? 2 : 1); 
        scal_Hx256 =(MMP_USHORT)pSCAL->SCAL_SCAL_0_H_M * 256 * Dnsamp / (MMP_USHORT)pSCAL->SCAL_SCAL_0_H_N;
        scal_Vx256 =(MMP_USHORT)pSCAL->SCAL_SCAL_0_V_M * 256 * Dnsamp / (MMP_USHORT)pSCAL->SCAL_SCAL_0_V_N;
        imgHST[MMPF_SCALER_PATH_0] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_0_H_ST * scal_Hx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_0_H_ST - 1);
        imgHED[MMPF_SCALER_PATH_0] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_0_H_ED * scal_Hx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_0_H_ST - 1);
        imgVST[MMPF_SCALER_PATH_0] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_0_V_ST * scal_Vx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_0_V_ST - 1);
        imgVED[MMPF_SCALER_PATH_0] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_0_V_ED * scal_Vx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_0_V_ST - 1);                
    }
    
    if(working_path & (1 << MMPF_SCALER_PATH_1) ) {
        Dnsamp = (pSCAL->SCAL_LPF1_SCAL_CTL & 0x01) ? 4 : ((pSCAL->SCAL_LPF1_SCAL_CTL & 0x02) ? 2 : 1); 
        scal_Hx256 =(MMP_USHORT)pSCAL->SCAL_SCAL_1_H_M * 256 * Dnsamp / (MMP_USHORT)pSCAL->SCAL_SCAL_1_H_N;
        scal_Vx256 =(MMP_USHORT)pSCAL->SCAL_SCAL_1_V_M * 256 * Dnsamp / (MMP_USHORT)pSCAL->SCAL_SCAL_1_V_N;
        imgHST[MMPF_SCALER_PATH_1] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_1_H_ST * scal_Hx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_1_H_ST - 1);
        imgHED[MMPF_SCALER_PATH_1] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_1_H_ED * scal_Hx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_1_H_ST - 1);
        imgVST[MMPF_SCALER_PATH_1] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_1_V_ST * scal_Vx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_1_V_ST - 1);
        imgVED[MMPF_SCALER_PATH_1] = (((MMP_ULONG)pSCAL->SCAL_GRAB_OUT_1_V_ED * scal_Vx256) >> 8) + (pSCAL->SCAL_GRAB_SCAL_1_V_ST - 1);                
    }
    
    if( working_path == (1 << MMPF_SCALER_PATH_0) ) {
        update_path = MMPF_SCALER_PATH_0 ;
    	imgOffsetX = 0;
    	imgOffsetY = 0;        
    }
    else if( working_path & (1 << MMPF_SCALER_PATH_1) ) {
        update_path = MMPF_SCALER_PATH_1 ;
    	imgOffsetX = 0;
    	imgOffsetY = 0;        
    }
    else{// if( working_path == (1 << MMPF_SCALER_PATH_2) ) {
        update_path = MMPF_SCALER_PATH_2 ;
    	imgOffsetX = 0;//pSCAL->SCAL_GRAB_SCAL_2_H_ST;
    	imgOffsetY = 0;//pSCAL->SCAL_GRAB_SCAL_2_V_ST;     
        imgHST[MMPF_SCALER_PATH_1] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_H_ST;
        imgHED[MMPF_SCALER_PATH_1] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_H_ED;
        imgVST[MMPF_SCALER_PATH_1] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_V_ST;
        imgVED[MMPF_SCALER_PATH_1] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_V_ED;    
        imgHST[MMPF_SCALER_PATH_0] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_H_ST;
        imgHED[MMPF_SCALER_PATH_0] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_H_ED;
        imgVST[MMPF_SCALER_PATH_0] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_V_ST;
        imgVED[MMPF_SCALER_PATH_0] = (MMP_ULONG)pSCAL->SCAL_GRAB_SCAL_2_V_ED;                       
    	   
    }
//    else {
//        update_path = select_path ;
//    }
    
    dbg_printf(AE_WIN_DBG_LEVEL,"AE_Win(%x,%x)\r\n",working_path,update_path);

    RTNA_DBG_Str(AE_WIN_DBG_LEVEL,"MMPF_Sensor_UpdateAEWindow");
    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,working_path);       // 1: yuv/mjpg; 2: h264; 3.yuv/mjpg + h264 4. graphic
	    
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgHST[MMPF_SCALER_PATH_0]);
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgHED[MMPF_SCALER_PATH_0]);    
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgVST[MMPF_SCALER_PATH_0]);
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgVED[MMPF_SCALER_PATH_0]);  
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgHST[MMPF_SCALER_PATH_1]);
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgHED[MMPF_SCALER_PATH_1]);    
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgVST[MMPF_SCALER_PATH_1]);
//    RTNA_DBG_Short(AE_WIN_DBG_LEVEL,imgVED[MMPF_SCALER_PATH_1]);
//    RTNA_DBG_Str(AE_WIN_DBG_LEVEL,"\r\n");    
    

    
    if(update_path == GRA_SRC_PIPE) {
        	ISP_IF_IQ_SetScalerOutputLength(imgHED[MMPF_SCALER_PATH_0] - imgHST[MMPF_SCALER_PATH_0] + 1 , imgVED[MMPF_SCALER_PATH_0] - imgVST[MMPF_SCALER_PATH_0] + 1, imgHST[MMPF_SCALER_PATH_0] + imgOffsetX, imgVST[MMPF_SCALER_PATH_0] + imgOffsetY); 
	
    } else {
        ISP_IF_IQ_SetScalerOutputLength(imgHED[update_path] - imgHST[update_path] + 1, imgVED[update_path] - imgVST[update_path] + 1, imgHST[update_path] + imgOffsetX, imgVST[update_path] + imgOffsetY); 
    }
    ISP_IF_AE_SetMinAccLength(0,0);

    ISP_IF_AE_SetDZTol(0, 0);
    ISP_IF_AE_SetAccWins();                
}
#endif


/// @}
