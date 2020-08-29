#include "includes_fw.h"
#if (USB_EN)

#include "lib_retina.h"
#include "reg_retina.h"
#include "mmpf_hif.h"
#include "mmpf_usbvend.h"
#include "mmpf_usbextrn.h"
#include "mmpf_usbpccam.h"
//#include "mmpf_usbmtp.h"
//#include "mmpf_usbdps.h"
//#include "mmpf_usbmsdc.h"
//#include "mmpf_memdev.h"
#include "mmpf_pll.h"
#include "mmpf_audio_ctl.h"
#include "mmps_3gprecd.h"
#include "mmp_reg_ibc.h"
#include "mmpf_usbuvc.h"
#include "mmpf_usbuac.h"
#include "mmpd_system.h"
#include "isp_if.h"
//#include "3A_ctl.h"
//#include "sensor_ctl.h"
#include "mmp_reg_jpeg.h"
#include "mmpf_sensor.h"
#include "mmpd_sensor.h"
#include "pcam_usb.h"
#include "pcam_msg.h"
#include "mmp_reg_usb.h"
#include "mmpf_dram.h"
#include "mmpf_boot.h"
//#include "mmpf_usbpccam.h"
#include "mmpf_usbuvc_vs.h"

/** @addtogroup MMPF_USB
@{
*/
extern MMPF_OS_FLAGID   SYS_Flag_Hif;

#if 1//(SUPPORT_PCCAM_FUNC == 1)
extern MMP_ULONG    glPCCAM_VIDEO_BUF_ADDR;
extern MMP_ULONG   glPCCamCompressBufAddr;
extern MMP_ULONG   glPCCamCompressBufSize;
#endif /* (SUPPORT_PCCAM_FUNC==1) */


#if (SUPPORT_UVC_FUNC==1)    
extern MMP_UBYTE gbUVCDSCCommand;            
extern MMP_UBYTE gbUVCPara[];
extern MMPS_3GPRECD_VIDEO_FORMAT m_VideoFmt;
extern MMP_ULONG   glPccamResolution;
MMP_UBYTE gbOrgVideoResolution;  //for still capture
MMPS_3GPRECD_VIDEO_FORMAT gbOrgVideoFormat;
MMP_ULONG glSystemEvent = 0;

#if USB_SUSPEND_TEST
extern MMP_UBYTE gbUSBSuspendEvent,  gbUSBSuspendFlag;
extern void USB_SuspendProcess(void);
#endif

extern void A810L_InitUSB(void);
extern MMP_ERR MMPS_3GPRECD_GetPreviewDisplay(MMP_BOOL *bEnable);
extern MMP_UBYTE gbDevicePowerSavingStatus;
extern MMP_BOOL m_bVidRecdPreviewStatus[];
extern MMP_UBYTE glAudioEnable;
#endif


#if USB_UVC_SKYPE
extern MMP_UBYTE gbSkypeMode;
extern MMP_UBYTE gbSkypeEncRes;
#endif

extern MMP_SHORT   gsAudioSamplesPerFrame ;

extern STREAM_CFG gsYUY2Stream_Cfg ;

void MMPF_SetUSB(void);
void MMPF_AdjustUSBSquelchLevel(MMP_USHORT squelch_level);
extern void SPI_Write(MMP_UBYTE addr, MMP_USHORT data);

void MMPF_USB_PauseStreaming(MMP_UBYTE ep_id,MMP_BOOL pause);
MMP_BOOL MMPF_USB_IsPauseStreaming(MMP_UBYTE ep_id);


void MMPF_SetUSB(void)
{
extern volatile MMPF_BOOTEXINFO *gsBootExInfo ;

	AITPS_USB_DMA pUSB_DMA = AITC_BASE_USBDMA;
	AITPS_USB_CTL pUSB_CTL = AITC_BASE_USBCTL;
    AITPS_GBL   pGBL = AITC_BASE_GBL;
    
    if( gsBootExInfo->loadfw & BC_USB_OFF ) {  
        pUSB_CTL->USB_ADT_LINK_INFO = (pUSB_CTL->USB_ADT_LINK_INFO & 0xF0 ) | 0x00 ;
          
        pUSB_CTL->USB_POWER = 0x61;
            
        #if(USB_CLK_INPUT == USB_CLKINPUT_PLL)
        pGBL->GBL_CLK_PATH_CTL |= USB_PHY_PLL_INPUT_EN;
        #endif
        #if(USB_CLK_INPUT == USB_CLKINPUT_CRYSTAL)
        pGBL->GBL_CLK_PATH_CTL &= (~USB_PHY_PLL_INPUT_EN);
        #endif 

        #if(USB_CLK_FREQ == USB_CLKFREQ_30MHZ)
        pUSB_DMA->USB_FARADAY_PHY_CTL1 |= 0x04;
        #elif (USB_CLK_FREQ == USB_CLKFREQ_12MHZ)
        pUSB_DMA->USB_FARADAY_PHY_CTL1 &= 0xFB;
        #endif
        
        dbg_printf(0,"--usb on\r\n");
    }
}
// 0x08 -> 0x0D17 for 1.8m
// For Rx sensitivity 
/*static */void MMPF_AdjustUSBSquelchLevel(MMP_USHORT squelch_level)
{
    MMP_USHORT usbphy_reg02 = 0x194F ;
    dbg_printf(3,"USB PHY Reg08 : %x\r\n",squelch_level);
    dbg_printf(3,"USB PHY Reg02 : %x\r\n",usbphy_reg02);
    SPI_Write(0x08,squelch_level);
    SPI_Write(0x02,usbphy_reg02);
   // #if (CHIP==VSN_V2) || (CHIP==VSN_V3) // ????
   // SPI_Write(0x06, 0x0100);
   // #endif
}
void MMPF_SetUSBChangeMode(unsigned char mode)
{

    AITPS_AIC   pAIC = AITC_BASE_AIC;
    AITPS_GBL   pGBL = AITC_BASE_GBL;

    //    RTNA_DBG_Str(0,"mode =  ");
    //    RTNA_DBG_Long(0,mode);
    //    RTNA_DBG_Str(0,"\r\n");

    // 0: Still mode, 1: MSDC mode  2: PCSync mode;
   // glUsbApplicationMode = mode;
     {
        /*==========================
        //====USB PHY Power Up======
        ============================*/
        pGBL->GBL_CLK_DIS1 &= (~GBL_CLK_USB_DIS); //Enable USB clock
    }

#if (SUPPORT_UVC_FUNC==1)    
     {
// sean@2012_09_12, move to bootcode
#if USB_POWER_EN==1 // move to bootloader to fine turning RESET -> D+ pull up
        MMPF_SetUSB();
#endif 
 
#if USB_SQUELCH_LEVEL > 0
        MMPF_AdjustUSBSquelchLevel(USB_SQUELCH_LEVEL);
#endif  
        A810L_InitUSB();
    }    

#endif
    RTNA_AIC_Open(pAIC, AIC_SRC_USB, usb_isr_a, AIC_INT_TO_IRQ | AIC_SRCTYPE_HIGH_LEVEL_SENSITIVE | 5/*3*/);//copychou change for webcam
    RTNA_AIC_IRQ_En(pAIC, AIC_SRC_USB);


    RTNA_DBG_Str(0, "===USB D+D- OK\r\n");
}



void MMPF_SetPCCAMCompressBuf(unsigned int buf_addr, unsigned int buf_size)
{
    glPCCamCompressBufAddr = buf_addr;
    glPCCamCompressBufSize = buf_size;
    dbg_printf(3,"jpg buf : %x, size : %d\r\n",buf_addr,buf_size);
}




#if defined(UPDATER_FW)
void MMPF_USB_StopUSB(void)
{
	AITPS_GBL   pGBL = AITC_BASE_GBL;
	
	pGBL->GBL_RST_REG_EN |= GBL_REG_USB_RST;
	pGBL->GBL_RST_CTL01 |= GBL_USB_RST;
}
#endif




MMP_USHORT MMPF_USBCTL_SendMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret,err ;
    msg_t *msg;

    msg = allocate_msg(src_id);
    if(!msg ){
        return MSG_ERROR_NONE ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = send_msg(msg,&err,src_id);
    /*destroy message*/
    free_msg( (void *)msg, src_id);
    if(ret) {
        return ret ;
    }
    return err;
}

MMP_USHORT MMPF_USBCTL_PostMsg(MMP_ULONG src_id,MMP_ULONG dst_id,MMP_ULONG msg_id,MMP_ULONG msg_sub_id,void *data)
{
    MMP_USHORT ret=MSG_ERROR_NONE ;
    msg_t *msg;
    msg = allocate_msg(src_id);
    if(!msg ){
        dbg_printf(0, "allocate !msg :%d\r\n", src_id);
        return MSG_ERROR_NONE ;
    }
    msg->src_id = src_id ; /*CallerID*/
    msg->dst_id = dst_id ; /*CalledID*/
    msg->msg_id = msg_id ; /*MSG ID*/
    msg->msg_sub_id = msg_sub_id ; /* MSG Sub Id */
    msg->msg_data = data ; /*API parameter*/
    ret = post_msg(msg, src_id);
    return ret;
}

msg_t *MMPF_USBCTL_GetMsg(void)
{
    MMPF_OS_FLAGS flags;
    msg_t *msg=0;
    /*
    Get message first in case there has multiple messages
    */
    #if 1
    msg = (msg_t *)get_msg(2);
    if(msg) {
        return msg ;
    }
    #endif
    /*
    Wait mesage event
    */
    MMPF_OS_WaitFlags(SYS_Flag_Hif, SYS_FLAG_USB_EP1,
                    MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME, 0, &flags);

    if(flags & SYS_FLAG_USB_EP1) {
        msg = (msg_t *)get_msg(2);
        if(msg) {
            return msg ;
        }
    }
    return (msg_t *)0;
}
//extern MMP_ULONG glFrameLength  ;
extern H264_FORMAT_TYPE gbCurH264Type ;
//extern MMP_UBYTE gbdrawflag; 
extern MMP_UBYTE gbStillCaptureEvent;
extern MMP_UBYTE gbCaptureBayerRawFlag;
//extern MMP_UBYTE gbCurFormatIndex;
extern STILL_PROBE sc;	//still commit
extern MMP_ULONG   dbg_level ;
extern MMP_UBYTE eu1_get_mmp_result_val[];

#if 0//SUPPORT_AUTO_FOCUS

#include "mmpf_pwm.h"
#include "mmp_reg_gpio.h"
#if (CHIP == P_V2)
MMPF_PWM_ATTRIBUTE pulseA = {MMPF_PWM_ID_1, MMPF_PWM_PULSE_ID_A, 1, 2, 3, 4, 0xFFFF, 0x1};
//sean@2011_01_24 from Gason, change PWM freq.
MMPF_PWM_ATTRIBUTE pulseB = {MMPF_PWM_ID_0, MMPF_PWM_PULSE_ID_A, 1, 2, 3, 4, 120, 0x1};
#define MOTOR_USED_PWM	MMPF_PWM_ID_0
#endif

#if (CHIP == VSN_V2)	// PWM depends on external clock
#define MOTOR_USED_PWM	MMPF_PWM_ID_1	// PWM1 => PBGPIO11
MMPF_PWM_ATTRIBUTE pulseB = {MOTOR_USED_PWM, MMPF_PWM_PULSE_ID_A, 1, 2, 3, 4, 120, 0x1};
#endif

#if (CHIP == VSN_V3)	// PWM depends on slow clock
#define MOTOR_USED_PWM	MMPF_PWM_ID_2	// PWM2 => PBGPIO13
//MMPF_PWM_ATTRIBUTE pulseB = {MOTOR_USED_PWM, MMPF_PWM_PULSE_ID_A, 8, 16, 24, 32, 2080, 0x1};
MMPF_PWM_ATTRIBUTE pulseB = {MOTOR_USED_PWM, MMPF_PWM_PULSE_ID_A, 10, 20, 30, 40, 2080, 0x1};
#endif

void InitializeMotor(void){
//	MMP_UBYTE* ptr = (MMP_UBYTE*)0x800009c0;
	MMPF_PWM_SetAttribe(&pulseB);	
	//MMPF_PWM_EnableInterrupt(MMPF_PWM_ID_0,MMP_TRUE,NULL,2);
	MMPF_PWM_ControlSet(MOTOR_USED_PWM, (PWM_PULSE_A_FIRST|PWM_AUTO_CYC|PWM_PULSE_A_NEG|PWM_EN));
	//MMPF_PWM_ControlSet(MOTOR_USED_PWM, (PWM_PULSE_A_FIRST|PWM_PULSE_A_NEG|PWM_EN));
	
	//Add for AIT auto focus
	//ISP_IF_AF_SetSearchRange(0x0, 0x60, 0x50, 0x78);
	// 2011_03_24 HB suggest search rang
	//ISP_IF_AF_SetSearchRange(0x10, 0x18, 0x50, 0x48);  //suggest=>0x10 = inf. 0x48 = 3cm, 0x60  = 2cm 0x10- & 0x60+: focus no change.
	ISP_IF_AF_SetSearchRange(0x10 << 2, 0x30 << 2, 0x28 << 2, 0x50 << 2, 10);  	//20120618 test range
	 
	ISP_IF_AF_SetPos(0x00, 10);
	
//	dbg_printf(0, "InitializeMotor......PWM=%x,   ", *ptr);
//	ptr = (MMP_UBYTE*)0x80006927;
//	dbg_printf(0, "GPIO=%x\r\n", *ptr);
}

void SetMotorPos(MMP_USHORT pos){
#if (CHIP == VSN_V3)
	MMPF_PWM_ATTRIBUTE pulseTmp = {MOTOR_USED_PWM, MMPF_PWM_PULSE_ID_A, 8, 16, 24, 32, 2080, 0x1};
#else
	MMPF_PWM_ATTRIBUTE pulseTmp = {MOTOR_USED_PWM, MMPF_PWM_PULSE_ID_A, 1, 2, 3, 4, 120, 0x1};
#endif
	
	//RTNA_DBG_PrintShort(0,pos);//remove by gason
	
	if(pos>255)
		return;

#if (CHIP == VSN_V3)
	if(pos==0)
		pulseTmp.usClkDuty_T3 = 32;
	else{
		//pulseTmp.usClkDuty_T3 += (pos*8);
		pulseTmp.usClkDuty_T3 = 32 + (pos << 2);	
	}
#else
	if(pos==0)
		pulseTmp.usClkDuty_T3 = 4;
	else{
		pulseTmp.usClkDuty_T3 += pos;	
	}
#endif

	//RTNA_DBG_PrintShort(0,pulseTmp.usClkDuty_T3);	//remove by gason
	MMPF_PWM_SetAttribe(&pulseTmp);	
}

void StopMotor(MMP_USHORT pos){
	//MMPF_PWM_SetAttribe(&pulseB);	
	//MMPF_PWM_EnableInterrupt(MMPF_PWM_ID_0,MMP_TRUE,NULL,2);
	SetMotorPos(0);
	MMPF_PWM_ControlSet(MOTOR_USED_PWM, 0);
}

#endif

void USB_Task_EP1(void)
{
static MMP_UBYTE sti_image_mode_ep1 = 0;
    MMPF_OS_FLAGS flags,wait_flags = SYS_FLAG_USB_EP1 ;
    MMP_BOOL IsPreviewEnable;

	#if USING_EXT_USB_DESC==1
 	Dummy_Init();
 	#endif
    MMPF_PWM_Initialize(); // PWM init , don't put in here	???    

#if SINGLE_STREAMING_TASK==1
    wait_flags |= SYS_FLAG_USB_EP2 ;
#endif
    while(1) {
        MMPF_OS_WaitFlags(SYS_Flag_Hif, wait_flags  ,MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME, 0, &flags); 
        if(flags & SYS_FLAG_USB_EP1 ) {
            STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(0);
            if( (ss->tx_flag & SS_TX_STREAMING) && !(ss->tx_flag & SS_TX_PAUSE) ){
                if(ss->tx_flag & SS_TX_NEXT_PACKET) {	
                    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe_by_epid(ss->ep_id);
                 	ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                 	ss->tx_flag &= ~SS_TX_EOS ;
                   	if( usb_vs_send_image(&sti_image_mode_ep1,ss->ep_id)== UVC_SEND_IMG_RET_END_FRAME) {
                   	    if(!(ss->tx_flag & SS_TX_RT_MODE)) {
                   	        MMPF_Video_UpdateRdPtrByPayloadLength(ss->ep_id);
                   	        // For H264,we will not wait H264 encoding done to trigger next frame
                   	        // But will check if slot buffer has frames to send.
                   	        usb_vs_next_frame(ss->ep_id,cur_pipe,PIPE_CFG_H264) ;
                   	    }
                   	    if( MMPF_USB_IsPauseStreaming(ss->ep_id) ) {
                   	        ss->tx_flag |= SS_TX_PAUSE ;
                   	        //dbg_printf(0,"--EP0.Tx.Pause\r\n");
                   	    }
                   	} else {
                   	    if( MMPF_USB_IsPauseStreaming(ss->ep_id) ) {
                   	        ss->tx_flag |= SS_TX_PAUSE ;
                   	        //dbg_printf(0,"--EP0.Tx.Pause 1\r\n");
                   	    }
                   	    else {
                       	    if(ss->tx_flag & SS_TX_BY_ONE_SHOT) {
                       	    #if YUY2_848_480_30FPS==FIRE_AT_FRAME_END
                       	        MMPF_USB_ReleaseDm(ss->pipe_id); 
                       	    #endif    
                       	    } else {
                       	        usb_vs_next_packet(ss->ep_id);
                       	    }     
                   	    }             	
                    }
                   	ss->tx_flag |= SS_TX_EOS ;
                } else {
                        usb_vs_next_packet(ss->ep_id);
                }
            }
        }
#if SINGLE_STREAMING_TASK==1
        if(flags & SYS_FLAG_USB_EP2 ) {
            STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(1);
            if(ss->tx_flag & SS_TX_STREAMING) {
                if(ss->tx_flag & SS_TX_NEXT_PACKET) {	
                    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe_by_epid(ss->ep_id);
                 	ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                 	ss->tx_flag &= ~SS_TX_EOS ;
                   	if( usb_vs_send_image(0,ss->ep_id)== UVC_SEND_IMG_RET_END_FRAME) {
                   	    //MMP_ULONG cpu_sr;
                   	    //MMP_UBYTE pipe0_cfg,pipe1_cfg;
                   	    MMPF_Video_UpdateRdPtrByPayloadLength(ss->ep_id);
                   	    // For H264,we will not wait H264 encoding done to trigger next frame
                   	    // But will check if slot buffer has frames to send.
                   	    usb_vs_next_frame(ss->ep_id,cur_pipe,PIPE_CFG_H264) ;
                   	}
                   	ss->tx_flag |= SS_TX_EOS ;
                } else {
                    usb_vs_next_packet(ss->ep_id);
                }
            }
        }
#endif
       
       
    }
}

#if SINGLE_STREAMING_TASK==0
void USB_Task_EP2(void)
{
static MMP_UBYTE sti_image_mode_ep2 = 0;
    MMPF_OS_FLAGS flags;
    MMP_UBYTE err;
    while(1) {
        err = MMPF_OS_WaitFlags(SYS_Flag_Hif, SYS_FLAG_USB_EP2,MMPF_OS_FLAG_WAIT_SET_ANY | MMPF_OS_FLAG_CONSUME, 0, &flags);
        if(err) {
            dbg_printf(3,"USBFIFO err:%d\r\n",err);
        } 
        if(flags&SYS_FLAG_USB_EP2) {
            STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(1);
             if(ss->tx_flag & SS_TX_STREAMING) {
                if(ss->tx_flag & SS_TX_NEXT_PACKET){
                    STREAM_CFG *cur_pipe = usb_get_cur_image_pipe_by_epid(ss->ep_id);
                    ss->tx_flag &= ~SS_TX_NEXT_PACKET ;
                    ss->tx_flag &= ~SS_TX_EOS ;
                   	if( usb_vs_send_image(&sti_image_mode_ep2,ss->ep_id)== UVC_SEND_IMG_RET_END_FRAME) {
                        MMPF_Video_UpdateRdPtrByPayloadLength(ss->ep_id);
           	            // For H264,we will not wait H264 encoding done to trigger next frame
                   	    // But will check if slot buffer has frames to send.
                   	    usb_vs_next_frame(ss->ep_id,cur_pipe,PIPE_CFG_H264) ;
                   	} else {
                   	    if(ss->tx_flag & SS_TX_BY_ONE_SHOT) {
                   	        MMPF_USB_ReleaseDm(ss->pipe_id);    
                   	    } else {
                   	        usb_vs_next_packet(ss->ep_id);
                   	    }                  	
                    }
                   	ss->tx_flag |= SS_TX_EOS ;
                } else {
                    usb_vs_next_packet(ss->ep_id);
                }
            }
        } // if(flags&SYS_FLAG_USBFIFO)       
    }                 
}
#endif

static MMP_BOOL gbPauseStreaming[2] ;

void MMPF_USB_PauseStreaming(MMP_UBYTE ep_id,MMP_BOOL pause)
{
    STREAM_SESSION *ss = MMPF_Video_GetStreamSessionByEp(ep_id);
    gbPauseStreaming[ep_id] = pause ;
    if(!pause) {
        ss->tx_flag &= ~SS_TX_PAUSE ;
    }
    else {
        while((ss->tx_flag & SS_TX_PAUSE)==0) {
            MMPF_OS_Sleep(1);
           // dbg_printf(0,"P,");
        }
    }
}

MMP_BOOL MMPF_USB_IsPauseStreaming(MMP_UBYTE ep_id)
{
    return gbPauseStreaming[ep_id] ;
}

#endif
/// @}
